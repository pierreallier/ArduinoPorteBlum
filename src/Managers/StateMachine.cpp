#include "StateMachine.h"

StateMachine::StateMachine(SensorsManager& s, Motor& m, Buzzer& b) : capteurs(s), moteur(m), buzzer(b),
                                                                     calibration(s,m), consigne() {
}

void StateMachine::init() {
    etat = ETAT::INIT;
    modePilotage = StateMachine::MODE_PILOTAGE::PWM;
    time_etat = millis();

    ledCalibration.init();
    ledError.init();
    ledPilotage.init();

    calibration.init();
}

void StateMachine::changerEtat(ETAT etat_demande) {
    if (!capteurs.hasPower()) {
        sendError(MSG::ERR_PUISSANCE, capteurs.getTension());
        etat_demande = ETAT::NO_POWER;
    } 
    if (etat == etat_demande)
        return;
    if ((etat == ETAT::CALIBRATION) && calibration.isNotCalibrated()) {
        sendError(MSG::ERR_CALIBRATION_ANNULEE);
        calibration.changerEtat(ETAT_CALIBRATION::ERREUR);
    }
    if (calibration.isNotCalibrated() && (etat_demande == ETAT::PILOTAGE || etat_demande == ETAT::OUVERTURE || etat_demande == ETAT::FERMETURE)) {
        sendError(MSG::ERR_CALIBRATION_REQUISE);
        etat_demande = ETAT::CALIBRATION_REQUIS;
    }
    etat = etat_demande;
    time_etat = millis();
    sendEtat(MSG::ETAT_PROD,(uint32_t)etat);

    // Initiliations des états
    switch (etat) {
        case ETAT::DEBRAYAGE: {
            moteur.enable();
            moteur.startDebrayage();
            ledCalibration.off();
            ledPilotage.off();
            break;
        }
        case ETAT::OUVERTURE:
        case ETAT::FERMETURE:
            moteur.enable();
            ledError.off();
            break;

        case ETAT::PILOTAGE: {
            pidPosition.reset();
            pidVitesse.reset();
            consigne.init(time_etat);
            moteur.enable();
            ledError.off();
            ledPilotage.on();
            butee_desactivated = true;
            break;
        }
        case ETAT::CALIBRATION_REQUIS: {
            moteur.disable();
            ledCalibration.blink(500); // Blink the calibration LED to indicate calibration is required
            break;
        }
        case ETAT::CALIBRATION: {
            calibration.changerEtat(ETAT_CALIBRATION::DEBUT);
            ledCalibration.on(); // Turn on the calibration LED to indicate calibration is in progress
            ledError.off();
            break;
        }
        case ETAT::ERREUR: {
            moteur.disable();
            ledError.on();
            buzzer.sequenceErreur();
            break;
        }
        case ETAT::NO_POWER: {
            moteur.disable();
            ledCalibration.off();
            ledPilotage.off();
            ledError.on();
            buzzer.sequenceErreur();
            break;
        }
        default: {
            moteur.disable();
            ledCalibration.off();
            ledPilotage.off();
            break;
        } 
    }   
}

void StateMachine::setMode(StateMachine::MODE_PILOTAGE mode) {
    switch (mode) {
        case StateMachine::MODE_PILOTAGE::PWM:
            sendReponseOK(MSG::DATA_MODE_PWM);
            break;
        case StateMachine::MODE_PILOTAGE::VITESSE:
            sendReponseOK(MSG::DATA_MODE_VITESSE);
            break;
        case StateMachine::MODE_PILOTAGE::POSITION:
            sendReponseOK(MSG::DATA_MODE_POSITION);
            break;
        case StateMachine::MODE_PILOTAGE::POSITION_VITESSE:
            sendReponseOK(MSG::DATA_MODE_PV);
            break;
        default:
            sendReponseNOK(MSG::ERR_PILOTAGE_SET);
            return;
    }
    modePilotage = mode;
}

void StateMachine::exec() {
    switch(etat) {
        case ETAT::INIT: {
            capteurs.setConsigne(0);
            if (calibration.isNotCalibrated()) {
                sendDirect(MSGTYPE::WARNING, MSG::ERR_CALIBRATION_REQUISE);
                changerEtat(ETAT::CALIBRATION_REQUIS);
            } else {
                changerEtat(ETAT::REPOS);
            }
            break;
        }
        case ETAT::REPOS: {
            capteurs.setConsigne(0);
            break;
        }
        case ETAT::FONCTIONNEMENT: {
             if (capteurs.getAnglePorte() < 50)
                changerEtat(ETAT::OUVERTURE);
            else
                changerEtat(ETAT::FERMETURE);
            break;
        }
        case ETAT::OUVERTURE: {
            if (etatOuverture(255)) {
                changerEtat(ETAT::DEBRAYAGE);
            }
            break;
        }
        case ETAT::FERMETURE: {
            if (etatFermeture(255)) {
                changerEtat(ETAT::DEBRAYAGE);
            }
            break;
        }
        case ETAT::PILOTAGE: {
            if (etatPilote()) {
                changerEtat(ETAT::DEBRAYAGE);
            }
            break;
        }
        case ETAT::DEBRAYAGE: { // ou Arrêt
            if (moteur.isDebraye()) {
                if (calibration.isNotCalibrated()) {
                    sendWarning(MSG::ERR_CALIBRATION_REQUISE);
                    changerEtat(ETAT::CALIBRATION_REQUIS);
                } else {
                    changerEtat(ETAT::REPOS);
                }
            }
            capteurs.setConsigne(moteur.getPWM());
            break;
        }
        case ETAT::CALIBRATION_REQUIS: {
            ledCalibration.update(); // Update the calibration LED blinking
            capteurs.setConsigne(0);
            break;
        }
        case ETAT::CALIBRATION: {
            if (calibration.exec()) {
                changerEtat(ETAT::DEBRAYAGE);
            }
            capteurs.setConsigne(moteur.getPWM());
            break;
        }
        case ETAT::ERREUR: {
            if (capteurs.hasPower()) {
                if (calibration.isNotCalibrated()) {
                    changerEtat(ETAT::REPOS);
                } else {
                    changerEtat(ETAT::DEBRAYAGE);
                }
            } else {
                changerEtat(ETAT::NO_POWER);
            }
            break;
        }
        case ETAT::NO_POWER: {
            if (capteurs.hasPower()) {
               changerEtat(ETAT::INIT);
               ledError.off();
            }
            break;
        }
        default: {
            changerEtat(ETAT::ERREUR);
            break;
        }
    }
    //calibration.task();
}

bool StateMachine::etatOuverture(uint16_t speed) {
    /* Gestion de l'ouverture de la porte en BO, retourne false si en cours, true si fini */
    moteur.ouvrir(speed);
    if (capteurs.isLimiteHaute()) {
        moteur.stop();
        return true;
    }
    return false;
}

bool StateMachine::etatFermeture(uint16_t speed) {
    /* Gestion de la fermeture de la porte en BO, retourne false si en cours, true si fini */
    moteur.fermer(speed);
    if (capteurs.isLimiteBasse()) {
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
        buzzer.sequenceFinPilotage(); // Si mode Potentiomètre, la consigne ne se termine pas, donc pas de bip
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
            pwm = pidVitesse.compute(consigne_value,capteurs.getVitesseMoteur(),time);
            break;
        }
        case StateMachine::MODE_PILOTAGE::POSITION: {
            consigne_value = constrain(consigne_value,-150,150);
            capteurs.setConsigne(consigne_value);
            pwm = pidPosition.compute(consigne_value,capteurs.getAnglePorte(),time);
            break;
        }
        case StateMachine::MODE_PILOTAGE::POSITION_VITESSE: {
            consigne_value = constrain(consigne_value,-120,180);
            capteurs.setConsigne(consigne_value);
            float consigneVitesse = pidPosition.compute(consigne_value,capteurs.getAnglePorte(),time);
            pwm = pidVitesse.compute(consigneVitesse,capteurs.getVitesseMoteur(), time);
            break;
        }
    }
    moteur.setSpeedDir(pwm);
    return false;
}

bool StateMachine::setConsigne(const String& type, const String* params, int nbParams) {
    if (etat == ETAT::PILOTAGE) {
        sendError(MSG::ERR_CONSIGNE_EN_PILOTAGE);
        return false;
    }
    return consigne.setConsigne(type, params, nbParams);
}

void StateMachine::suspendre() {
    // exec() ne tourne pas pendant le test : on applique l'arrêt nous-mêmes
    moteur.stop();
    moteur.disable();
    moteur.update();   // PWM à 0 et STBY à LOW
}

void StateMachine::reprendre() {
    capteurs.isBlocage();   // Efface un éventuel blocage mémorisé avant le test

    if (etat == ETAT::CALIBRATION) {
        // Calibration interrompue : retour à REPOS avec calibration requise (LED clignotante)
        calibration.changerEtat(ETAT_CALIBRATION::DEBUT);
        time_etat = millis();
        sendEtat(MSG::ETAT_PROD,(uint32_t)etat);
        sendInfo(MSG::ERR_CALIBRATION_ANNULEE);
        return;
    }
    time_etat = millis();
}