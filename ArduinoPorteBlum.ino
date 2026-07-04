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
StateMachine machine(moteur, capteurs);
ComSerie portserie(moteur, capteurs, machine);


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
    portserie.task();
    machine.exec();
    ordonnanceur();
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
        if (capteurs.limite_courant_atteinte) {
            portserie.sendError("Limite de courant atteinte");
        }
    }
    
    if (maintenant - tAcq >= 10) {
        tAcq += 10;
        // Vérification des boutons de commande
        btTest.update();
        btWireless.update();
        if (btTest.pressed()) {
            if (machine.etat == StateMachine::ETAT::REPOS)
                machine.changerEtat(StateMachine::ETAT::PILOTAGE);
            else
                machine.changerEtat(StateMachine::ETAT::DEBRAYAGE);
        }
        if (btWireless.pressed()) {
            if (machine.etat == StateMachine::ETAT::REPOS)
                machine.changerEtat(StateMachine::ETAT::FONCTIONNEMENT);
            else
                machine.changerEtat(StateMachine::ETAT::DEBRAYAGE);
        }
    }

    // Mesures des grandeurs (toutes les 50 ms)
    if (maintenant - tMesure >= 50) {
        tMesure += 50;
        capteurs.mesures(moteur.getPWM());
    } 
}