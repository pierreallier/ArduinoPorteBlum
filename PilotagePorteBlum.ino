#include <Arduino.h>
#include "Sensors.h"
#include "Boutons.h"
#include "Motor.h"

/* Définitions des états du système */
enum class EtatMachine : byte {
    INIT,
    REPOS,
    OUVERTURE,
    FERMETURE,
    PILOTE,
    CALIBRATION,
    DEBRAYAGE,
    ERREUR
};

EtatMachine etat = EtatMachine::INIT;

void ChangerEtat(EtatMachine etat_demande) {
    etat = etat_demande;
}

void setup() {
    Mesures_Init(); // Initialisation des capteurs
    Boutons_Init(); // Initialisation des boutons de contrôle
    
    pinMode(LED_BUILTIN, OUTPUT);

    // Initialisation du port série
    Serial.begin(9600);
    Serial.flush();

    // Initialisation de la machine à états
    etat = EtatMachine::INIT;
}

uint32_t tEtat = 0;
uint32_t tVerif = 0;
uint32_t tAcq = 0;

void ordonnanceur() {
    // Tâches périodiques
    uint32_t maintenant = millis();
    if (maintenant - tAcq >= 100) {
        tAcq += 100;
        Mesures_Update();
        tVerif = maintenant; // on saute volontairement le cycle 5 ms
    } else if (maintenant - tVerif >= 5) {
        tVerif += 5;   
        if (Check_Securites()) {
            etat = EtatMachine::ERREUR;
        }
        // Vérification des boutons de commande
        if (Get_BoutonTest()) {
            if (etat == EtatMachine::REPOS)
                ChangerEtat(EtatMachine::PILOTE);
            else
                ChangerEtat(EtatMachine::DEBRAYAGE);
        }

        if (Get_BoutonSansFil()) {
            if (etat == EtatMachine::REPOS) {
                if (mesures.angle_porte > 100) {
                    tEtat = millis(); 
                    ChangerEtat(EtatMachine::OUVERTURE);
                }
                else {
                    tEtat = millis(); 
                    ChangerEtat(EtatMachine::FERMETURE);
                }
            }
            else {
                ChangerEtat(EtatMachine::DEBRAYAGE);
            }
        }
    }
}

void loop() {
    ordonnanceur();
    machineEtat();
    //communicationSerie();
}

void machineEtat() {
    switch(etat) {
        case EtatMachine::INIT:
            Serial.println(F("Pilotage Porte Blum"));
            ChangerEtat(EtatMachine::REPOS);
            break;

        case EtatMachine::REPOS:
            Serial.println(F("En attente"));
            break;

        case EtatMachine::OUVERTURE:
            Serial.println("Ouverture en cours");
            if (millis() - tEtat >= 2000) {
                ChangerEtat(EtatMachine::REPOS);
            }
            break;

        case EtatMachine::FERMETURE:
            Serial.println("Fermeture en cours");
            if (millis() - tEtat >= 2000) {
                ChangerEtat(EtatMachine::REPOS);
            }
            break;

        case EtatMachine::PILOTE:
            Serial.println("Pilotage");
            digitalWrite(LED_BUILTIN, HIGH);
            break;

        case EtatMachine::CALIBRATION:

            // Calibration_Run();

            break;

        case EtatMachine::DEBRAYAGE: // ou Arrêt
            Serial.println("Arret demandé");
            digitalWrite(LED_BUILTIN, LOW);
            ChangerEtat(EtatMachine::REPOS);
            break;

        default: // EtatMachine::ERREUR
            Serial.println("En Erreur");
            digitalWrite(LED_BUILTIN, LOW);
            break;
    }

    //-----------------------------
    // Driver moteur
    //-----------------------------

    //Motor_Task();
}
