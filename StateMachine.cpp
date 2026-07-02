#include "StateMachine.h"
#include "Sensors.h"
#include "Motor.h"

EtatMachine etat;

uint32_t time_etat;

void machineEtat_Init() {
    etat = EtatMachine::INIT;
    time_etat = millis();
}

void changerEtat(EtatMachine etat_demande) {
    if (etat != etat_demande) {
        time_etat = millis();
    }
    etat = etat_demande;
    if (etat == EtatMachine::PILOTE || 
        etat == EtatMachine::OUVERTURE || 
        etat == EtatMachine::FERMETURE || 
        etat == EtatMachine::CALIBRATION || 
        etat == EtatMachine::DEBRAYAGE) 
    {
        moteur_Enable();
    } else {
        moteur_Disable();
    }
}

void machineEtat() {
    switch(etat) {
        case EtatMachine::INIT:
            Serial.println(F("Pilotage Porte Blum"));
            changerEtat(EtatMachine::REPOS);
            break;

        case EtatMachine::REPOS:
            break;

        case EtatMachine::OUVERTURE:
            moteur_SetDirection(MotorDir::OUVERTURE);
            moteur_SetSpeed(abs(mesures.potentiometre));
            if (mesures.limite_haute || detection_Butee()) {
                moteur_Stop();
                changerEtat(EtatMachine::DEBRAYAGE);
            }
            break;

        case EtatMachine::FERMETURE:
            moteur_SetDirection(MotorDir::FERMETURE);
            moteur_SetSpeed(abs(mesures.potentiometre));
            if (mesures.limite_basse || detection_Butee()) {
                moteur_Stop();
                changerEtat(EtatMachine::DEBRAYAGE);
            }
            break;

        case EtatMachine::PILOTE:
            if (!detection_Butee()) {
                moteur_SetSpeedDir(mesures.potentiometre);
            } else {
                moteur_Stop();
                changerEtat(EtatMachine::DEBRAYAGE);
            }
            break;

        case EtatMachine::CALIBRATION:

            // Calibration_Run();

            break;

        case EtatMachine::DEBRAYAGE: // ou Arrêt
            moteur_Debrayage();

            int32_t delta_angle = abs(mesures.angle_moteur - moteur.codeur_avant_debrayage);
            float delta_courant = abs(mesures.courant_moyen - moteur.courant_avant_debrayage)/mesures.courant_moyen;

            if (millis() - time_etat >= 200 || delta_angle > 20 || delta_courant > 0.05) {
                Serial.println(F("Debrayage terminé"));
                moteur_Stop();
                changerEtat(EtatMachine::REPOS);
            }
            break;

        case EtatMachine::ERREUR:
            Serial.println(F("ERREUR"));
            changerEtat(EtatMachine::DEBRAYAGE);
            break;

        default:
            moteur_Stop();
            break;
    }
    moteur_Task();
}
