#include "StateMachine.h"

EtatMachine etat;

void machineEtat_Init() {
    etat = EtatMachine::INIT;
}

void changerEtat(EtatMachine etat_demande) {
    etat = etat_demande;
}

void machineEtat(uint32_t tEtat) {
    switch(etat) {
        case EtatMachine::INIT:
            Serial.println(F("Pilotage Porte Blum"));
            changerEtat(EtatMachine::REPOS);
            break;

        case EtatMachine::REPOS:
            Serial.println(F("En attente"));
            break;

        case EtatMachine::OUVERTURE:
            Serial.println("Ouverture en cours");
            if (millis() - tEtat >= 2000) {
                changerEtat(EtatMachine::REPOS);
            }
            break;

        case EtatMachine::FERMETURE:
            Serial.println("Fermeture en cours");
            if (millis() - tEtat >= 2000) {
                changerEtat(EtatMachine::REPOS);
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
            changerEtat(EtatMachine::REPOS);
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
