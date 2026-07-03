#include "StateMachine.h"
#include "Sensors.h"
#include "ComSerie.h"
#include "Pilotage.h"

Motor moteur;
Sensors capteurs;

StateMachine::StateMachine(Motor m, Sensors c) {
    etat = StateMachine::ETAT::INIT;
    moteur = m;
    capteurs = c;
    time_etat = millis();
}

void StateMachine::init() {
    etat = StateMachine::ETAT::INIT;
    time_etat = millis();
}

void StateMachine::changerEtat(StateMachine::ETAT etat_demande) {
    if (etat != etat_demande) {
        time_etat = millis();
        //comSerie_SendEtat(etat_demande);
    }
    etat = etat_demande;
    if (etat == StateMachine::ETAT::PILOTE || 
        etat == StateMachine::ETAT::OUVERTURE || 
        etat == StateMachine::ETAT::FERMETURE || 
        etat == StateMachine::ETAT::CALIBRATION || 
        etat == StateMachine::ETAT::DEBRAYAGE) 
    {
        moteur.enable();
    } else {
        moteur.disable();
    }
    if (etat == StateMachine::ETAT::DEBRAYAGE) 
        moteur.debrayage(capteurs);
}

void StateMachine::exec() {
    switch(etat) {
        case StateMachine::ETAT::INIT:
            Serial.println(F("Pilotage Porte Blum"));
            changerEtat(StateMachine::ETAT::REPOS);
            break;

        case StateMachine::ETAT::REPOS:
            break;

        case StateMachine::ETAT::OUVERTURE:
            moteur.setDirection(Motor::DIR::OUVERTURE);
            moteur.setSpeed(abs(capteurs.potentiometre));
            if (capteurs.limite_haute || capteurs.detectionButees()) {
                moteur.stop();
                changerEtat(StateMachine::ETAT::DEBRAYAGE);
            }
            break;

        case StateMachine::ETAT::FERMETURE:
            moteur.setDirection(Motor::DIR::FERMETURE);
            moteur.setSpeed(abs(capteurs.potentiometre));
            if (capteurs.limite_basse || capteurs.detectionButees()) {
                moteur.stop();
                changerEtat(StateMachine::ETAT::DEBRAYAGE);
            }
            break;

        case StateMachine::ETAT::PILOTE:
            if (!capteurs.detectionButees()) {
                pilotage_Update();
            } else {
                moteur.stop();
                changerEtat(StateMachine::ETAT::DEBRAYAGE);
            }
            break;

        case StateMachine::ETAT::CALIBRATION:

            // Calibration_Run();

            break;

        case StateMachine::ETAT::DEBRAYAGE: // ou Arrêt
            
            int32_t delta_angle = abs(capteurs.angle_moteur - moteur.codeur_avant_debrayage);
            float delta_courant = abs(1 - moteur.courant_avant_debrayage/capteurs.courant_moyen);

            if (millis() - time_etat >= 100 || delta_angle > 100 || delta_courant > 0.5) {
                Serial.println(F("Debrayage terminé"));
                moteur.stop();
                changerEtat(StateMachine::ETAT::REPOS);
            }
            break;

        case StateMachine::ETAT::ERREUR:
            changerEtat(StateMachine::ETAT::DEBRAYAGE);
            break;

        default:
            moteur.stop();
            break;
    }
    moteur.update(capteurs);
}
