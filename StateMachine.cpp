#include "StateMachine.h"

StateMachine::StateMachine(Motor& m, Sensors& c, ComSerie& s) : moteur(m), capteurs(c), serial(s), pilote() {
    etat = StateMachine::ETAT::INIT;
    time_etat = millis();
}

void StateMachine::init() {
    etat = StateMachine::ETAT::INIT;
    time_etat = millis();
}

void StateMachine::changerEtat(StateMachine::ETAT etat_demande) {
    if (etat != etat_demande) {
        time_etat = millis();
    }
    etat = etat_demande;
    serial.sendEtat((int)etat);
    if (etat == StateMachine::ETAT::PILOTE || 
        etat == StateMachine::ETAT::OUVERTURE || 
        etat == StateMachine::ETAT::FERMETURE || 
        etat == StateMachine::ETAT::CALIBRATION || 
        etat == StateMachine::ETAT::DEBRAYAGE) {
        moteur.enable();
    } else {
        moteur.disable();
    }
    if (etat == StateMachine::ETAT::DEBRAYAGE) 
        moteur.debrayage();
}

void StateMachine::exec() {
    switch(etat) {
        case StateMachine::ETAT::INIT:
            Serial.println("");
            Serial.println(F("==== Pilotage Porte Blum ===="));
            Serial.println("");
            changerEtat(StateMachine::ETAT::REPOS);
            break;

        case StateMachine::ETAT::REPOS:
            break;

        case StateMachine::ETAT::OUVERTURE:
            moteur.setDirection(Motor::DIR::OUVERTURE);
            moteur.setSpeed(abs(capteurs.potentiometre));
            if (capteurs.limite_haute || capteurs.isBlocage(true)) {
                moteur.stop();
                changerEtat(StateMachine::ETAT::DEBRAYAGE);
            }
            break;

        case StateMachine::ETAT::FERMETURE:
            moteur.setDirection(Motor::DIR::FERMETURE);
            moteur.setSpeed(abs(capteurs.potentiometre));
            if (capteurs.limite_basse || capteurs.isBlocage(true)) {
                moteur.stop();
                changerEtat(StateMachine::ETAT::DEBRAYAGE);
            }
            break;

        case StateMachine::ETAT::PILOTE:
            if (!capteurs.isBlocage(true)) {
                pilote.setPWM(capteurs.potentiometre);
                pilote.setConsigneVitesse(capteurs.vitesse_moteur);
                pilote.setConsignePosition(capteurs.angle_porte);
                pilote.update(moteur);
            } else {
                moteur.stop();
                serial.sendError("Blocage detecté");
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
                moteur.stop();
                changerEtat(StateMachine::ETAT::REPOS);
            }
            break;

        default:
            changerEtat(StateMachine::ETAT::DEBRAYAGE);
            break;
    }
    moteur.update();
}
