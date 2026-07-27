#include <Arduino.h>
#include <Bounce2.h>
#include <Wire.h>
#include "Messages.h"
#include "Buzzer.h"

#include "StateMachine.h"
#include "Sensors.h"
#include "SerialManager.h"
#include "Motor.h"
#include "Constantes.h"

Bounce2::Button btTest;
Bounce2::Button btWireless;
Bounce2::Button btCalibration;
Bounce2::Button btPilotage;

Sensors capteurs;
Motor moteur(capteurs);
CalibrationManager calibration(moteur, capteurs);
StateMachine machine(moteur, capteurs, calibration);
SerialManager portserie(moteur, capteurs, machine, calibration);
Buzzer buzzer;

bool erreurBlocage = false;
bool erreurCourant = false;
bool erreurLimitePorte = false;
bool ledState = LOW;

uint32_t tVerif = 0;
uint32_t tAcq = 0;
uint32_t tMesure = 0;
uint32_t tLed = 0;

void setup() {
    portserie.init(); // Initialisation du port série
    buzzer.init(); // Initialiation du buzzer

    btTest.attach(TEST_BT,INPUT_PULLUP);
    btTest.setPressedState(LOW); 
    btWireless.attach(WIRELESS_BT,INPUT_PULLUP);
    btWireless.setPressedState(LOW);
    btCalibration.attach(CALIBRATION_BT,INPUT_PULLUP);
    btCalibration.setPressedState(LOW);
    btPilotage.attach(PILOTAGE_BT,INPUT_PULLUP);
    btPilotage.setPressedState(LOW);

    erreurBlocage = false;
    erreurCourant = false;
    erreurLimitePorte = false;

    moteur.init(); // Initialisation du moteur et du driver
    capteurs.init(); // Initialisation des capteurs
    machine.init(); // Initialisation de la machine à états
    calibration.init(); // Initialisation de la calibration

    // Vérification codeur porte I2C
    Wire.begin();
    unsigned long startTime = millis();
    bool send_error = false;  // Flag pour éviter d'afficher plusieurs fois l'erreur
    while (!capteurs.checkCodeurPorte()) {
        if (!send_error) {
            sendError("Erreur sur le capteur I2C de la porte");
            digitalWrite(LED_ERROR_PIN, HIGH);
            portserie.task();  
            send_error = true;         
        }
        delay(1000);  // Petite pause pour éviter de saturer le CPU
    }
    sendInfo("Capteur I2C de la porte fonctionnel");
    digitalWrite(LED_ERROR_PIN, LOW);

    // Vérification type de montage et calibration
    if (calibration.getEtat()) {
        sendInfo("Système monté sur un meuble");
    } else {
        sendInfo("Système non monté");
    }
    sendInfo(calibration.getCalibrationString().c_str());
    if (calibration.isNotCalibrated()) {
        sendWarning("Calibration requise");
    }

    sendInfo("Initialisation terminée");
    buzzer.sequenceInit();
    portserie.task();
    delay(1000);
}

void loop() {
    portserie.task();
    calibration.task();
    machine.exec();
    ordonnanceur();
    buzzer.task();
}

void ordonnanceur() {
    // Tâches périodiques
    uint32_t maintenant = millis();

    // Sécurités (toutes les 5 ms)
    if (maintenant - tVerif >= 5) {
        tVerif += 5;   
        // Vérifications des sécurités
        capteurs.updateSecurities(moteur.getPWM());
        if (machine.etat != StateMachine::ETAT::CALIBRATION) {
            if (capteurs.isLimiteAngle() & machine.etat != StateMachine::ETAT::DEBRAYAGE) {
                if (machine.etat == StateMachine::ETAT::PILOTAGE) {
                    sendError("Limite de la porte atteinte");
                    machine.changerEtat(StateMachine::ETAT::ERREUR);
                    buzzer.sequenceErreur();
                } else {
                    sendInfo("Limite de la porte atteinte");
                    machine.changerEtat(StateMachine::ETAT::DEBRAYAGE);
                }
                capteurs.resetSecurities();
            } else {
                if (capteurs.isLimiteCourant()) {
                    sendError("Limite de courant atteinte");
                    machine.changerEtat(StateMachine::ETAT::ERREUR);
                    buzzer.sequenceErreur();
                }
                if (capteurs.isBlocage()){
                    sendError("Blocage détecté");
                    machine.changerEtat(StateMachine::ETAT::ERREUR);
                    buzzer.sequenceErreur();
                }
                if (calibration.hasChanged()) {
                    if (calibration.getEtat()) {
                        sendWarning("ServoDrive monté sur un meuble");
                    } else {
                        sendWarning("ServoDrive démonté du meuble");
                    }
                    if (calibration.isNotCalibrated()) {
                        sendWarning("Calibration requise");
                    }
                    if (machine.etat != StateMachine::ETAT::REPOS) {
                        machine.changerEtat(StateMachine::ETAT::ERREUR);
                    }
                }
            }
        }
    }
    
    if (maintenant - tAcq >= 50) {
        tAcq += 50;
        // Vérification des boutons de commande
        btTest.update();
        btWireless.update();
        btCalibration.update();
        btPilotage.update();
        if (btCalibration.pressed()) {
            if (machine.etat == StateMachine::ETAT::REPOS) {
                machine.changerEtat(StateMachine::ETAT::CALIBRATION);
            }
            else
                machine.changerEtat(StateMachine::ETAT::DEBRAYAGE);
        }
        if (btPilotage.pressed()) {
            if (machine.etat == StateMachine::ETAT::REPOS) {
                machine.changerEtat(StateMachine::ETAT::PILOTAGE);
            }
            else
                machine.changerEtat(StateMachine::ETAT::DEBRAYAGE);
        }
        if (btWireless.pressed() || btTest.pressed()) {
            if (machine.etat == StateMachine::ETAT::REPOS)
                machine.changerEtat(StateMachine::ETAT::FONCTIONNEMENT);
            else
                machine.changerEtat(StateMachine::ETAT::DEBRAYAGE);
        }
    }

    // Mesures des grandeurs
    int periode = portserie.mesureEnable();
    //int periode = 100;
    if (periode != 0 && maintenant - tMesure >= periode) {
        tMesure += periode;
        float pwm = moteur.getPWM();
        capteurs.mesures(pwm);
        //sendMesures(capteurs.time_mesures, capteurs.tension, pwm, capteurs.courant_moyen, capteurs.angle_moteur, 
        //            capteurs.vitesse_moteur, capteurs.getCodeurPorte(), capteurs.consigne);
    } 
}