#include <Arduino.h>
#include "StateMachine.h"
#include "Sensors.h"
#include "Boutons.h"
#include "ComSerie.h"
#include "Motor.h"

Motor moteur;
Sensors capteurs;
StateMachine machine(moteur, capteurs);
ComSerie portserie(machine, moteur, capteurs);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    moteur.init(); // Initialisation du moteur et du driver
    capteurs.init(); // Initialisation des capteurs
    boutons_Init(); // Initialisation des boutons de contrôle
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

void ordonnanceur() {
    // Tâches périodiques
    uint32_t maintenant = millis();
    if (maintenant - tAcq >= 100) {
        tAcq += 100;
        capteurs.mesures();
        tVerif = maintenant; // on saute volontairement le cycle 10 ms
    } else if (maintenant - tVerif >= 10) {
        tVerif += 10;   
        if (capteurs.checkSecurites()) {
            // Vérification des limites angulaires et courant
        }
        // Vérification des boutons de commande
        if (get_BoutonTest()) {
            if (machine.etat == StateMachine::ETAT::REPOS)
                machine.changerEtat(StateMachine::ETAT::PILOTE);
            else
                machine.changerEtat(StateMachine::ETAT::DEBRAYAGE);
        }

        if (get_BoutonSansFil()) {
            if (machine.etat == StateMachine::ETAT::REPOS) {
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
}