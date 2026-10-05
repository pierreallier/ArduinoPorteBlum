#include <Arduino.h>
#include <Bounce2.h>
#include "Serial/Messages.h"
#include "Outputs/Buzzer.h"
#include "Inputs/BN0055.h"
#include "Inputs/MT6701.h"
#include "MachineEtats/TestManager.h"

#include "MachineEtats/StateMachine.h"
#include "Inputs/Sensors.h"
#include "Serial/SerialManager.h"
#include "Outputs/Motor.h"
#include "Config/Constantes.h"

Bounce2::Button btTest;
Bounce2::Button btWireless;
Bounce2::Button btCalibration;
Bounce2::Button btPilotage;

Sensors capteurs;
BN0055 bno;
MT6701 codeurPorte;
Buzzer buzzer;

TestManager testeur(buzzer, codeurPorte, bno);
Motor moteur(capteurs);
CalibrationManager calibration(moteur, capteurs);
StateMachine machine(moteur, capteurs, calibration);
SerialManager portserie(moteur, capteurs, machine, calibration);

uint32_t tVerif = 0;
uint32_t tBt = 0;
uint32_t tMesure = 0;
uint32_t tEnvoi = 0;
uint32_t tBno = 0;

bool is_time_securite = false;

void lancerTest();
void ordonnanceur();

void setup() {
    portserie.init(); // Initialisation du port série
    buzzer.init(); // Initialiation du buzzer
    buzzer.disable();

    btTest.attach(TEST_BT,INPUT_PULLUP);
    btTest.setPressedState(LOW); 
    btWireless.attach(WIRELESS_BT,INPUT_PULLUP);
    btWireless.setPressedState(LOW);
    btCalibration.attach(CALIBRATION_BT,INPUT_PULLUP);
    btCalibration.setPressedState(LOW);
    btPilotage.attach(PILOTAGE_BT,INPUT_PULLUP);
    btPilotage.setPressedState(LOW);

    moteur.init(); // Initialisation du moteur et du driver
    codeurPorte.init(); // Initialisation du codeur absolu de la porte
    bno.init(); // Initialisation du capteur BN0055
    capteurs.init(); // Initialisation des capteurs
    machine.init(); // Initialisation de la machine à états
    calibration.init(); // Initialisation de la calibration

    // Vérification codeur porte I2C
    bool send_error = false;  // Flag pour éviter d'afficher plusieurs fois l'erreur
    while (!codeurPorte.checkPresence()) {
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

    // Vérification capteur BN0055
    bno.setEnabled(true);
    if (!bno.checkPresence()) {
        sendWarning("Capteur BN0055 non détecté");
        bno.setEnabled(false);  // Désactivation du capteur pour éviter les erreurs de lecture
    } else {
        sendInfo("Capteur BN0055 détecté");
        bno.requestData(0);  // Demande de lecture initiale
    }
    portserie.task();

    sendInfo("Initialisation terminée");
    buzzer.sequenceInit();
    portserie.task();
    delay(1000);
}

void loop() {
    portserie.task();
    machine.exec();
    if (portserie.demandeTest) {
        portserie.demandeTest = false;
        lancerTest();
    } else {
        calibration.task();
        bno.update();
        ordonnanceur();
    }
    buzzer.task();
}

void lancerTest() {
    machine.suspendre();
    testeur.run();                    // Bloquant jusqu'à DO STOP
    machine.reprendre();

    // Resynchronisation des timers pour éviter le rattrapage des cycles manqués
    tVerif = tMesure = tBt  = tEnvoi = tBno = millis();
    portserie.resync();

    // Évite qu'un appui pendant le test soit pris pour une nouvelle commande
    btTest.update(); btWireless.update(); btCalibration.update(); btPilotage.update();
}

void ordonnanceur() {
    uint32_t PERIODE_ENVOI = portserie.getMesurePeriode();
    uint32_t PERIODE_MESURE = min(PERIODE_ENVOI,25);
    float pwm = moteur.getPWM();

    // Tâches périodiques
    uint32_t maintenant = millis();

    // Mesures - Sécurités (toutes les 5 ms)
    if (maintenant - tVerif >= 5) {
        tVerif += 5;   
        // Vérifications des sécurités
        capteurs.updateSecurities(pwm);
        is_time_securite = true;
    }
    // Mesures des autres grandeurs (au maximum toutes les 25 ms) pour conserver le même timestamps que les sécurités
    if (maintenant - tMesure >= PERIODE_MESURE) {
        tMesure += PERIODE_MESURE;
        capteurs.mesures(pwm);
    }

    // Envoies des mesures sur le port série
    if (PERIODE_ENVOI <= 1000 && maintenant - tEnvoi >= PERIODE_ENVOI) {
        tEnvoi += PERIODE_ENVOI;
        if (maintenant - tBno >= 5 * PERIODE_ENVOI) {
            tBno += 5 * PERIODE_ENVOI;
            sendMesures(bno.getTime(),bno.getAcceleration(0),bno.getAcceleration(1), bno.getAcceleration(2), 
                        bno.getGyroscope(0), bno.getGyroscope(1), bno.getGyroscope(2), 
                        bno.getEulerAngle(0), bno.getEulerAngle(1), bno.getEulerAngle(2));
            bno.requestData(maintenant);
        } else {
            sendMesures(capteurs.time_mesures, capteurs.tension, capteurs.pwm, capteurs.courant_moyen, capteurs.angle_moteur, 
                        capteurs.vitesse_moteur, capteurs.getCodeurPorte(), capteurs.consigne);
        }
    } 

    // Vérifications des sécurités 
    if (is_time_securite) {
        is_time_securite = false;
        if (machine.etat != StateMachine::ETAT::CALIBRATION) {
            if (capteurs.isLimiteAngle() && (machine.etat != StateMachine::ETAT::DEBRAYAGE && machine.etat != StateMachine::ETAT::REPOS)) {
                if (machine.etat == StateMachine::ETAT::PILOTAGE) {
                    sendError("Limite de la porte atteinte");
                    machine.changerEtat(StateMachine::ETAT::ERREUR);
                    buzzer.sequenceErreur();
                } else {
                    sendInfo(("Limite de la porte atteinte " + String(capteurs.getCodeurPorte())).c_str());
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

    if (maintenant - tBt >= 50) {
        tBt += 50;
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
}