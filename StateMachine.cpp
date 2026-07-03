#include "StateMachine.h"
#include "Sensors.h"
#include "ComSerie.h"
#include "Pilotage.h"

EtatMachine etat;
Moteur moteur;

uint32_t time_etat;

void machineEtat_Init(Moteur m) {
    etat = EtatMachine::INIT;
    moteur = m;
    time_etat = millis();
}

void changerEtat(EtatMachine etat_demande) {
    if (etat != etat_demande) {
        time_etat = millis();
        comSerie_SendEtat(etat_demande);
    }
    etat = etat_demande;
    if (etat == EtatMachine::PILOTE || 
        etat == EtatMachine::OUVERTURE || 
        etat == EtatMachine::FERMETURE || 
        etat == EtatMachine::CALIBRATION || 
        etat == EtatMachine::DEBRAYAGE) 
    {
        moteur.enable();
    } else {
        moteur.disable();
    }
    if (etat == EtatMachine::DEBRAYAGE) 
        moteur.debrayage();
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
            moteur.setDirection(MotorDir::OUVERTURE);
            moteur.setSpeed(abs(mesures.potentiometre));
            if (mesures.limite_haute || detection_Butee()) {
                moteur.stop();
                changerEtat(EtatMachine::DEBRAYAGE);
            }
            break;

        case EtatMachine::FERMETURE:
            moteur.setDirection(MotorDir::FERMETURE);
            moteur.setSpeed(abs(mesures.potentiometre));
            if (mesures.limite_basse || detection_Butee()) {
                moteur.stop();
                changerEtat(EtatMachine::DEBRAYAGE);
            }
            break;

        case EtatMachine::PILOTE:
            if (!detection_Butee()) {
                pilotage_Update();
            } else {
                moteur.stop();
                changerEtat(EtatMachine::DEBRAYAGE);
            }
            break;

        case EtatMachine::CALIBRATION:

            // Calibration_Run();

            break;

        case EtatMachine::DEBRAYAGE: // ou Arrêt
            
            int32_t delta_angle = abs(mesures.angle_moteur - moteur.codeur_avant_debrayage);
            float delta_courant = abs(mesures.courant_moyen - moteur.courant_avant_debrayage)/mesures.courant_moyen;

            if (millis() - time_etat >= 100 || delta_angle > 100 || delta_courant > 0.5) {
                Serial.println(F("Debrayage terminé"));
                moteur.stop();
                changerEtat(EtatMachine::REPOS);
            }
            break;

        case EtatMachine::ERREUR:
            changerEtat(EtatMachine::DEBRAYAGE);
            break;

        default:
            moteur.stop();
            break;
    }
    moteur.update();
}
