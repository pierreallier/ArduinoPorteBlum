#include "StateMachine.h"
#include "Sensors.h"
#include "ComSerie.h"
#include "Pilotage.h"

EtatMachine etat;
Moteur moteur;
Capteurs capteurs;


StateMachine::StateMachine(Moteur m, Capteurs c) {
    etat = EtatMachine::INIT;
    moteur = m;
    capteurs = c;
    time_etat = millis();
}

void StateMachine::init() {
    etat = EtatMachine::INIT;
    time_etat = millis();
}

void StateMachine::changerEtat(EtatMachine etat_demande) {
    if (etat != etat_demande) {
        time_etat = millis();
        //comSerie_SendEtat(etat_demande);
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
        moteur.debrayage(capteurs);
}

void StateMachine::exec() {
    switch(etat) {
        case EtatMachine::INIT:
            Serial.println(F("Pilotage Porte Blum"));
            changerEtat(EtatMachine::REPOS);
            break;

        case EtatMachine::REPOS:
            break;

        case EtatMachine::OUVERTURE:
            moteur.setDirection(MotorDir::OUVERTURE);
            moteur.setSpeed(abs(capteurs.potentiometre));
            if (capteurs.limite_haute || capteurs.detectionButees()) {
                moteur.stop();
                changerEtat(EtatMachine::DEBRAYAGE);
            }
            break;

        case EtatMachine::FERMETURE:
            moteur.setDirection(MotorDir::FERMETURE);
            moteur.setSpeed(abs(capteurs.potentiometre));
            if (capteurs.limite_basse || capteurs.detectionButees()) {
                moteur.stop();
                changerEtat(EtatMachine::DEBRAYAGE);
            }
            break;

        case EtatMachine::PILOTE:
            if (!capteurs.detectionButees()) {
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
            
            int32_t delta_angle = abs(capteurs.angle_moteur - moteur.codeur_avant_debrayage);
            float delta_courant = abs(1 - moteur.courant_avant_debrayage/capteurs.courant_moyen);

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
    moteur.update(capteurs);
}
