#include <Arduino.h>
#include "StateMachine.h"
#include "Sensors.h"
#include "Boutons.h"
#include "ComSerie.h"
#include "Motor.h"


void setup() {
    mesures_Init(); // Initialisation des capteurs
    boutons_Init(); // Initialisation des boutons de contrôle
    
    pinMode(LED_BUILTIN, OUTPUT);

    comSerie_Init(); // Initialisation du port série
    
    // Initialisation de la machine à états
    machineEtat_Init();
}

uint32_t tEtat = 0;

void loop() {
    ordonnanceur();
    machineEtat(tEtat);
    comSerie_Task();
}

uint32_t tVerif = 0;
uint32_t tAcq = 0;

void ordonnanceur() {
    // Tâches périodiques
    uint32_t maintenant = millis();
    if (maintenant - tAcq >= 100) {
        tAcq += 100;
        mesures_Update();
        tVerif = maintenant; // on saute volontairement le cycle 10 ms
    } else if (maintenant - tVerif >= 10) {
        tVerif += 10;   
        if (check_Securites()) {
            etat = EtatMachine::ERREUR;
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
                if (mesures.angle_porte > 100) {
                    tEtat = millis(); 
                    changerEtat(EtatMachine::OUVERTURE);
                }
                else {
                    tEtat = millis(); 
                    changerEtat(EtatMachine::FERMETURE);
                }
            }
            else {
                changerEtat(EtatMachine::DEBRAYAGE);
            }
        }
    }
}