#include <Arduino.h>
#include <Bounce2.h>
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
StateMachine machine(moteur, capteurs);
SerialManager portserie(moteur, capteurs, machine);
Buzzer buzzer;

bool erreurBlocage = false;
bool erreurCourant = false;
bool erreurLimitePorte = false;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
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

    portserie.printFinInit();
    buzzer.sequenceInit();
}

void loop() {
    portserie.task();
    machine.exec();
    ordonnanceur();
    buzzer.task();
}

uint32_t tVerif = 0;
uint32_t tAcq = 0;
uint32_t tMesure = 0;

void ordonnanceur() {
    // Tâches périodiques
    uint32_t maintenant = millis();

    // Sécurités (toutes les 5 ms)
    if (maintenant - tVerif >= 5) {
        tVerif += 5;   
        // Vérifications des sécurités
        capteurs.checkSecurites(moteur.getPWM());
        if (capteurs.limite_courant_atteinte && !erreurCourant) {
            erreurCourant = true;
            machine.changerEtat(StateMachine::ETAT::DEBRAYAGE);
            sendError("Limite de courant atteinte");
            buzzer.sequenceErreur();
        }
        if ((capteurs.limite_haute || capteurs.limite_basse) && !erreurLimitePorte) {
            erreurLimitePorte = true;
            machine.changerEtat(StateMachine::ETAT::DEBRAYAGE);
            if (machine.etat == StateMachine::ETAT::PILOTAGE) {
                sendError("Limite de la porte atteinte");
                buzzer.sequenceErreur();
            }
        }
        if (capteurs.isBlocage(false) && !erreurBlocage && not(machine.butee_desactivated)){
            erreurBlocage = true;
            machine.changerEtat(StateMachine::ETAT::DEBRAYAGE);
            sendError("Blocage détecté");
            buzzer.sequenceErreur();
        }
    }
    
    if (maintenant - tAcq >= 10) {
        tAcq += 10;
        // Vérification des boutons de commande
        btTest.update();
        btWireless.update();
        btPilotage.update();
        if (btPilotage.pressed()) {
            //Serial.println("I;Bouton Pilotage pressé");
            if (machine.etat == StateMachine::ETAT::REPOS) {
                machine.changerEtat(StateMachine::ETAT::PILOTAGE);
            }
            else
                machine.changerEtat(StateMachine::ETAT::DEBRAYAGE);
        }
        if (btWireless.pressed() || btTest.pressed()) {
            //Serial.println("I;Bouton Radio pressé");
            if (machine.etat == StateMachine::ETAT::REPOS)
                machine.changerEtat(StateMachine::ETAT::FONCTIONNEMENT);
            else
                machine.changerEtat(StateMachine::ETAT::DEBRAYAGE);
        }
        // Vérification des sécurités
        if(machine.etat == StateMachine::ETAT::REPOS) {
            erreurBlocage = false;
            erreurCourant = false;
            erreurLimitePorte = false;
        }
    }

    // Mesures des grandeurs
    int periode = portserie.mesureEnable();
    if (periode != 0 && maintenant - tMesure >= periode) {
        tMesure += periode;
        float pwm = moteur.getPWM();
        capteurs.mesures(pwm);
        sendMesures(capteurs.time_mesures, capteurs.tension, pwm, capteurs.courant_moyen, capteurs.angle_moteur, 
                    capteurs.vitesse_moteur, capteurs.angle_porte, capteurs.consigne);
    } 
}