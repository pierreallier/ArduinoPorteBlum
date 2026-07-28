#include "StateMachine.h"

const char* const StateMachine::ETAT_NAMES[] ={
    "INIT",
    "REPOS",
    "FONCTIONNEMENT",
    "OUVERTURE",
    "FERMETURE",
    "PILOTAGE",
    "CALIBRATION",
    "DEBRAYAGE",
    "ERREUR",
    "STOP"
};


StateMachine::StateMachine(Motor& m, Sensors& s, CalibrationManager& c) : moteur(m), capteurs(s), consigne(), calibration(c) {
}

void StateMachine::init() {
    etat = StateMachine::ETAT::INIT;
    modePilotage = StateMachine::MODE_PILOTAGE::PWM;
    time_etat = millis();

    //pinMode(LED_CALIBRATION_PIN, OUTPUT);
    pinMode(LED_PILOTAGE_PIN, OUTPUT);
    pinMode(LED_ERROR_PIN, OUTPUT);
}

void StateMachine::changerEtat(StateMachine::ETAT etat_demande) {
    if (etat == etat_demande)
        return;
    if ((etat == StateMachine::ETAT::CALIBRATION) && calibration.isNotCalibrated()) {
        sendError("Calibration annulé");
        calibration.changerEtat(CalibrationManager::ETAT::ERREUR);
    }
    if (calibration.isNotCalibrated() && (etat_demande == StateMachine::ETAT::PILOTAGE || etat_demande == StateMachine::ETAT::OUVERTURE ||etat_demande == StateMachine::ETAT::FERMETURE)) {
        sendError("Calibration requise");
        etat_demande = StateMachine::ETAT::ERREUR;
    }
    etat = etat_demande;
    time_etat = millis();
    sendEtat(ETAT_NAMES[static_cast<size_t>(etat)]);

    // Initiliations des états
    switch (etat) {
        case StateMachine::ETAT::DEBRAYAGE: {
            moteur.enable();
            moteur.debrayage();
            //digitalWrite(LED_CALIBRATION_PIN, LOW);
            digitalWrite(LED_PILOTAGE_PIN, LOW);
            break;
        }
        case StateMachine::ETAT::OUVERTURE:
        case StateMachine::ETAT::FERMETURE:
            moteur.enable();
            digitalWrite(LED_ERROR_PIN, LOW);
            break;

        case StateMachine::ETAT::PILOTAGE: {
            pidPosition.reset();
            pidVitesse.reset();
            consigne.init(time_etat);
            moteur.enable();
            //digitalWrite(LED_PILOTAGE_PIN, HIGH);
            digitalWrite(LED_ERROR_PIN, LOW);
            butee_desactivated = true;
            break;
        }
        case StateMachine::ETAT::CALIBRATION: {
            calibration.changerEtat(CalibrationManager::ETAT::DEBUT);
            //digitalWrite(LED_CALIBRATION_PIN, HIGH);
            digitalWrite(LED_ERROR_PIN, LOW);
            break;
        }
        case StateMachine::ETAT::ERREUR: {
            digitalWrite(LED_ERROR_PIN, HIGH);
            break;
        }
        default: {
            moteur.disable();
            //digitalWrite(LED_CALIBRATION_PIN, LOW);
            digitalWrite(LED_MOTOR_PIN, LOW);
            break;
        } 
    }   
}

void StateMachine::setMode(StateMachine::MODE_PILOTAGE mode) {
    modePilotage = mode;
    String message = "basculé en ";
    switch (mode) {
        case StateMachine::MODE_PILOTAGE::PWM:
            message += "PWM";
            break;
        case StateMachine::MODE_PILOTAGE::VITESSE:
            message += "asservissement vitesse";
            break;
        case StateMachine::MODE_PILOTAGE::POSITION:
            message += "asservissement position";
            break;
        case StateMachine::MODE_PILOTAGE::POSITION_VITESSE:
            message += "asservissement position et vitesse";
            break;
        default:
            sendReponseNOK("SET","MODE","Inconnu");
    }
    sendReponseOK("SET","MODE",message);
}

void StateMachine::exec() {
    switch(etat) {
        case StateMachine::ETAT::INIT: {
            capteurs.setConsigne(0);
            changerEtat(StateMachine::ETAT::REPOS);
            break;
        }
        case StateMachine::ETAT::REPOS: {
            capteurs.setConsigne(0);
            break;
        }
        case StateMachine::ETAT::FONCTIONNEMENT: {
             if (capteurs.getCodeurPorte() < 50)
                changerEtat(StateMachine::ETAT::OUVERTURE);
            else
                changerEtat(StateMachine::ETAT::FERMETURE);
            break;
        }
        case StateMachine::ETAT::OUVERTURE: {
            if (etatOuverture(255)) {
                changerEtat(StateMachine::ETAT::DEBRAYAGE);
            }
            break;
        }
        case StateMachine::ETAT::FERMETURE: {
            if (etatFermeture(255)) {
                changerEtat(StateMachine::ETAT::DEBRAYAGE);
            }
            break;
        }
        case StateMachine::ETAT::PILOTAGE: {
            if (etatPilote()) {
                digitalWrite(LED_PILOTAGE_PIN, LOW);
                changerEtat(StateMachine::ETAT::DEBRAYAGE);
            }
            break;
        }
        case StateMachine::ETAT::DEBRAYAGE: { // ou Arrêt
            if (etatDebrayage()) {
                changerEtat(StateMachine::ETAT::REPOS);
            }
            capteurs.setConsigne(moteur.getPWM());
            break;
        }
        case StateMachine::ETAT::CALIBRATION: {
            if (calibration.exec()) {
                //digitalWrite(LED_CALIBRATION_PIN, LOW);
                changerEtat(StateMachine::ETAT::DEBRAYAGE);
            }
            capteurs.setConsigne(moteur.getPWM());
            break;
        }
        case StateMachine::ETAT::ERREUR: {
            changerEtat(StateMachine::ETAT::REPOS);
            break;
        }
        default: {
            changerEtat(StateMachine::ETAT::DEBRAYAGE);
            break;
        }
    }
    moteur.update();
}

bool StateMachine::etatOuverture(uint16_t speed) {
    /* Gestion de l'ouverture de la porte en BO, retourne false si en cours, true si fini */
    moteur.setDirection(Motor::DIR::OUVERTURE);
    moteur.setSpeed(speed);
    capteurs.setConsigne(speed);
    if (capteurs.limite_haute) {
        moteur.stop();
        return true;
    }
    return false;
}

bool StateMachine::etatFermeture(uint16_t speed) {
    /* Gestion de la fermeture de la porte en BO, retourne false si en cours, true si fini */
    moteur.setDirection(Motor::DIR::FERMETURE);
    moteur.setSpeed(speed);
    capteurs.setConsigne(speed);
    if (capteurs.limite_basse) {
        moteur.stop();
        return true;
    }
    return false;
}

bool StateMachine::etatDebrayage() {
    /* Gestion du debrayage du moteur */
    int32_t delta_angle = abs(capteurs.angle_moteur - moteur.codeur_avant_debrayage);
    //float delta_courant = abs(1 - moteur.courant_avant_debrayage/capteurs.courant_moyen);
    if (millis() - time_etat >= 200 || delta_angle > 100 ) { //|| delta_courant > 0.5) {
        moteur.stop();
        return true;
    }
    return false;
}

bool StateMachine::etatPilote() {
    /* Gestion du pilotage de la porte via consigne */
    unsigned long time = millis();
    if (consigne.ended(time)) {
        moteur.stop();
        return true;
    }
    float consigne_value = consigne.get(time);
    float pwm = 0.0f;
    switch (modePilotage) {
        case StateMachine::MODE_PILOTAGE::PWM: {
            pwm = constrain(consigne_value,-255,255);
            capteurs.setConsigne(pwm);
            break;
        }
        case StateMachine::MODE_PILOTAGE::VITESSE: {
            capteurs.setConsigne(consigne_value);
            pwm = pidVitesse.compute(consigne_value,capteurs.vitesse_moteur,time);
            break;
        }
        case StateMachine::MODE_PILOTAGE::POSITION: {
            consigne_value = constrain(consigne_value,-150,150);
            capteurs.setConsigne(consigne_value);
            pwm = pidPosition.compute(consigne_value,capteurs.angle_porte,time);
            break;
        }
        case StateMachine::MODE_PILOTAGE::POSITION_VITESSE: {
            consigne_value = constrain(consigne_value,-120,180);
            capteurs.setConsigne(consigne_value);
            float consigneVitesse = pidPosition.compute(consigne_value,capteurs.angle_porte,time);
            pwm = pidVitesse.compute(consigneVitesse,capteurs.vitesse_moteur, time);
            break;
        }
    }
    moteur.setSpeedDir(pwm);
    return false;
}

// bool StateMachine::is_calibre() {
//     return calibration.isCalibrationInitialized();
// }

// bool StateMachine::etatCalibration() {
//     /* Gestion de l'état calibration */
//     switch (etape_calibration) {
//         case StateMachine::ETAPE_CALIBRATION::OUVERTURE_INITIALE: {
//             if (etatOuverture(250) || capteurs.isBlocage(true)) {
//                 moteur.stop();
//                 changerEtapeCalibration(StateMachine::ETAPE_CALIBRATION::ATTENTE_HAUT);
//             }
//             break;
//         }
//         case StateMachine::ETAPE_CALIBRATION::ATTENTE_HAUT: {
//             if (millis() - time_etape_calibration >= 1000) {
//                 moteur.debrayage();
//                 changerEtapeCalibration(ETAPE_CALIBRATION::DEBRAYAGE_HAUT);
//             }
//             break;
//         }
//         case StateMachine::ETAPE_CALIBRATION::DEBRAYAGE_HAUT: {
//             if (etatDebrayage()) {
//                 changerEtapeCalibration(StateMachine::ETAPE_CALIBRATION::RECHERCHE_BUTEE_BASSE);
//             }
//             break;
//         }
//         case StateMachine::ETAPE_CALIBRATION::RECHERCHE_BUTEE_BASSE: {
//             if (etatFermeture(250) || capteurs.isBlocage(true)) {
//                 angle_butee_basse = capteurs.angle_porte;
//                 moteur.stop();
//                 changerEtapeCalibration(StateMachine::ETAPE_CALIBRATION::ATTENTE_BAS);
//             }
//             break;
//         }
//         case StateMachine::ETAPE_CALIBRATION::ATTENTE_BAS: {
//             if (millis() - time_etape_calibration >= 1000) {
//                 moteur.debrayage();
//                 changerEtapeCalibration(ETAPE_CALIBRATION::DEBRAYAGE_BAS);
//             }
//             break;
//         }
//         case StateMachine::ETAPE_CALIBRATION::DEBRAYAGE_BAS: {
//             if (etatDebrayage()) {
//                 changerEtapeCalibration(StateMachine::ETAPE_CALIBRATION::RECHERCHE_BUTEE_HAUTE);
//             }
//             break;
//         }
//         case StateMachine::ETAPE_CALIBRATION::RECHERCHE_BUTEE_HAUTE: {
//             if (etatOuverture(250) || capteurs.isBlocage(true)) {
//                 angle_butee_haute = capteurs.angle_porte;
//                 is_calibre = true;
//                 changerEtapeCalibration(StateMachine::ETAPE_CALIBRATION::DEBRAYAGE_FINAL);
//                 moteur.debrayage();
//             }
//             break;
//         }
//         case StateMachine::ETAPE_CALIBRATION::DEBRAYAGE_FINAL: {
//             if (etatDebrayage()) {
//                 butee_desactivated = false;
//                 sendReponseOK("DO","CALIBRATION","Fin de calibration : limite haute=" + String(angle_butee_haute) + "° ; limite basse=" + String(angle_butee_basse) + "°");
//                 moteur.stop();
//                 changerEtapeCalibration(StateMachine::ETAPE_CALIBRATION::ATTENTE_ENREGISTREMENT);
//             }
//             break;
//         }
//         case StateMachine::ETAPE_CALIBRATION::ATTENTE_ENREGISTREMENT: {
//             if (millis() - time_etape_calibration >= 1000) {
//                 capteurs.setLimits(angle_butee_basse,angle_butee_haute);
//                 changerEtapeCalibration(ETAPE_CALIBRATION::NONE);
//                 changerEtat(StateMachine::ETAT::REPOS);
//             }
//             break;
//         }
//         case StateMachine::ETAPE_CALIBRATION::NONE:
//             return true;
//         default:
//             break;
//     }
//     return false;
// }

// void StateMachine::changerEtapeCalibration(StateMachine::ETAPE_CALIBRATION nouvelle_etape) {
//     time_etape_calibration = millis();
//     etape_calibration = nouvelle_etape;
// }



// bool StateMachine::pushMessage(Message::TYPE t, String message) {
//     if (nbElements >= TAILLE_FIFO)
//         return false;           // FIFO pleine
//     Message m = {t,message};
//     messages[queue] = m;
//     queue = (queue + 1) % TAILLE_FIFO;
//     nbElements++;

//     return true;
// }

// bool StateMachine::popMessage(Message &m) {
//     if (nbElements == 0)
//         return false;
//     m = messages[tete];
//     tete = (tete + 1) % TAILLE_FIFO;
//     nbElements--;

//     return true;
// }

// bool StateMachine::hasMessage() const {
//     return nbElements > 0;
// }

// Message StateMachine::getMessage() {
//     Message cmd;
//     if (!popMessage(cmd))
//         return {Message::TYPE::AUCUN,""}; 
//     return cmd;
// }

bool StateMachine::setConsigne(const String& type, const String* params, int nbParams) {
    if (etat == StateMachine::ETAT::PILOTAGE) {
        sendError("Impossible de changer de consigne pendant l'état pilotage. Le système doit être au repos.");
        return false;
    }
    return consigne.setConsigne(type, params, nbParams);
}