#include <Arduino.h>
#include <Bounce2.h>
#include "StateMachine.h"
#include "Sensors.h"
#include "ComSerie.h"
#include "Motor.h"

#define TEST_BT 2 // Bouton de mise en fonctionnement / arrêt
#define WIRELESS_BT 3 // Bouton sans fil 

Bounce2::Button btTest;
Bounce2::Button btWireless;

Sensors capteurs;
Motor moteur(capteurs);
ComSerie portserie(moteur, capteurs);
StateMachine machine(moteur, capteurs, portserie);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    btTest.attach(TEST_BT,INPUT_PULLUP);
    btTest.setPressedState(LOW); 
    btWireless.attach(WIRELESS_BT,INPUT_PULLUP);
    btWireless.setPressedState(LOW);

    moteur.init(); // Initialisation du moteur et du driver
    capteurs.init(); // Initialisation des capteurs
    portserie.init(); // Initialisation du port série
    machine.init(); // Initialisation de la machine à états
}

void loop() {
    ordonnanceur();
    machine.exec();
    portserie.task();
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
    
        // Vérification des boutons de commande
        btTest.update();
        btWireless.update();
        if (btTest.pressed()) {
            if (machine.etat == StateMachine::ETAT::REPOS)
                machine.changerEtat(StateMachine::ETAT::PILOTE);
            else
                machine.changerEtat(StateMachine::ETAT::DEBRAYAGE);
        }
        if (btWireless.pressed()) {
            if (machine.etat == StateMachine::ETAT::REPOS) {
                // TODO : la suite doit être géré par la machine à état !
                if (capteurs.angle_porte < -100) {
                    machine.changerEtat(StateMachine::ETAT::OUVERTURE);
                }
                else {
                    machine.changerEtat(StateMachine::ETAT::FERMETURE);
                }
            }
            else {
                machine.changerEtat(StateMachine::ETAT::DEBRAYAGE);
            }
        }
    }

    // Mesures des grandeurs (toutes les 100 ms)
    if (maintenant - tMesure >= 100) {
        tMesure += 100;
        capteurs.mesures(moteur.getPWM());
    } 
}