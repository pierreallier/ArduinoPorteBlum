#include <Arduino.h>
#include "StateMachine.h"
#include "Sensors.h"
#include "Boutons.h"
#include "ComSerie.h"
#include "Motor.h"

extern Moteur moteur;
extern Capteurs capteurs;
//Serial portserie;

void setup() {
    moteur.init(); // Initialisation du moteur et du driver
    capteurs.init(); // Initialisation des capteurs
    boutons_Init(); // Initialisation des boutons de contrôle
    
    pinMode(LED_BUILTIN, OUTPUT);

    comSerie_Init(); // Initialisation du port série
    
    // Initialisation de la machine à états
    machineEtat_Init(moteur);
}

void loop() {
    ordonnanceur(capteurs);
    machineEtat();
    comSerie_Task();
}

uint32_t tVerif = 0;
uint32_t tAcq = 0;

void ordonnanceur(Capteurs capteurs) {
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
            if (etat == EtatMachine::REPOS)
                changerEtat(EtatMachine::PILOTE);
            else
                changerEtat(EtatMachine::DEBRAYAGE);
        }

        if (get_BoutonSansFil()) {
            if (etat == EtatMachine::REPOS) {
                if (capteurs.angle_porte < -100) {
                    changerEtat(EtatMachine::OUVERTURE);
                }
                else {
                    changerEtat(EtatMachine::FERMETURE);
                }
            }
            else {
                changerEtat(EtatMachine::DEBRAYAGE);
            }
        }
    }
}