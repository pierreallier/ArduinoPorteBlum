#include "BlumMachine.h"

BlumMachine::BlumMachine(SensorsManager& s, Motor& m, Buzzer& b) 
        : StateMachine<ETAT>(s,m,b), calibration(s,m,b), consigne() {
    name = MSG::ETAT_PROD;
}

void BlumMachine::init() {
    
    ledCalibration.init();
    ledError.init();
    ledPilotage.init();

    calibration.init();

}

void BlumMachine::start() {
    setEtat(ETAT::INIT);
    if (calibration.isNotCalibrated()) {
        sendDirect(MSGTYPE::WARNING, MSG::ERR_CALIBRATION_REQUISE);
        changerEtat(ETAT::CALIBRATION_REQUIS);
    }
    modePilotage = BlumMachine::MODE_PILOTAGE::PWM;
    StateMachine<ETAT>::start();    
}

ETAT BlumMachine::checkTransition(ETAT etat_demande) { 
    if (calibration.isNotCalibrated()) {
        if (etat_demande == ETAT::CALIBRATION || 
            etat_demande == ETAT::CALIBRATION_REQUIS || 
            etat_demande == ETAT::NO_POWER) {
            return etat_demande;
        } else if (_etat == ETAT::CALIBRATION && etat_demande != ETAT::CALIBRATION) {
            sendError(MSG::ERR_CALIBRATION_ANNULEE);
            return ETAT::ERREUR;
        } else if (etat_demande != ETAT::CALIBRATION_REQUIS) {
            sendWarning(MSG::ERR_CALIBRATION_REQUISE);
            return ETAT::CALIBRATION_REQUIS;
        }
    }
    return etat_demande;
}

bool BlumMachine::changerEtat(ETAT etat_demande) {
    if (!capteurs.hasPower() && _etat != ETAT::NO_POWER) {
        sendError(MSG::ERR_PUISSANCE, capteurs.getTension()*100);
        etat_demande = ETAT::NO_POWER;
    }
    return StateMachine::changerEtat(etat_demande);
}

void BlumMachine::execStateExit(ETAT etat) {
    switch (etat) {
        case ETAT::PILOTAGE: {
            ledPilotage.off();
            butee_desactivated = false;
            break;
        }
        case ETAT::CALIBRATION: {
            calibration.stop();
            ledCalibration.off();
            break;
        }
        default:
            break;
    }
}

void BlumMachine::execStateEntry(ETAT etat) {
    // Initiliations des états
    switch (etat) {
        case ETAT::REPOS: {
            moteur.disable();
            break;
        }
        case ETAT::DEBRAYAGE: {
            moteur.enable();
            moteur.startDebrayage();
            break;
        }
        case ETAT::OUVERTURE: {
            moteur.enable();
            ledError.off();
            moteur.ouvrir(255);
            break;
        }
        case ETAT::FERMETURE: {
            moteur.enable();
            ledError.off();
            moteur.fermer(255);
            break;
        }
        case ETAT::PILOTAGE: {
            pidPosition.reset();
            pidVitesse.reset();
            consigne.init(_time_etat);
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
            calibration.start();
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
            ledError.on();
            buzzer.sequenceErreur();
            break;
        }
        default:
            break;
    }
}

void BlumMachine::setMode(BlumMachine::MODE_PILOTAGE mode) {
    Serial.println("Setting mode to " + String(static_cast<int>(mode)));
    switch (mode) {
        case BlumMachine::MODE_PILOTAGE::PWM:
            sendReponseOK(MSG::DATA_MODE_PWM);
            break;
        case BlumMachine::MODE_PILOTAGE::VITESSE:
            sendReponseOK(MSG::DATA_MODE_VITESSE);
            break;
        case BlumMachine::MODE_PILOTAGE::POSITION:
            sendReponseOK(MSG::DATA_MODE_POSITION);
            break;
        case BlumMachine::MODE_PILOTAGE::POSITION_VITESSE:
            sendReponseOK(MSG::DATA_MODE_PV);
            break;
        default:
            sendReponseNOK(MSG::ERR_PILOTAGE_SET);
            return;
    }
    modePilotage = mode;
}

void BlumMachine::execState(ETAT etat) {
    switch(etat) {
        case ETAT::INIT: {
            capteurs.setConsigne(0);
            changerEtat(ETAT::REPOS);
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
            if (capteurs.isLimiteHaute()) {
                moteur.stop();
                changerEtat(ETAT::DEBRAYAGE);
            }
            break;
        }
        case ETAT::FERMETURE: {
            if (capteurs.isLimiteBasse()) {
                moteur.stop();
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
            if (!calibration.exec()) {
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
}

bool BlumMachine::etatOuverture(uint16_t speed) {
    /* Gestion de l'ouverture de la porte en BO, retourne false si en cours, true si fini */
    moteur.ouvrir(speed);
    if (capteurs.isLimiteHaute()) {
        moteur.stop();
        return true;
    }
    return false;
}

bool BlumMachine::etatFermeture(uint16_t speed) {
    /* Gestion de la fermeture de la porte en BO, retourne false si en cours, true si fini */
    moteur.fermer(speed);
    if (capteurs.isLimiteBasse()) {
        moteur.stop();
        return true;
    }
    return false;
}


bool BlumMachine::etatPilote() {
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
        case BlumMachine::MODE_PILOTAGE::PWM: {
            pwm = constrain(consigne_value,-255,255);
            capteurs.setConsigne(pwm);
            break;
        }
        case BlumMachine::MODE_PILOTAGE::VITESSE: {
            capteurs.setConsigne(consigne_value);
            pwm = pidVitesse.compute(consigne_value,capteurs.getVitesseMoteur(),time);
            break;
        }
        case BlumMachine::MODE_PILOTAGE::POSITION: {
            consigne_value = constrain(consigne_value,-150,150);
            capteurs.setConsigne(consigne_value);
            pwm = pidPosition.compute(consigne_value,capteurs.getAnglePorte(),time);
            break;
        }
        case BlumMachine::MODE_PILOTAGE::POSITION_VITESSE: {
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

bool BlumMachine::setConsigne(const String& type, const String* params, int nbParams) {
    if (_etat == ETAT::PILOTAGE) {
        sendError(MSG::ERR_CONSIGNE_EN_PILOTAGE);
        return false;
    }
    return consigne.setConsigne(type, params, nbParams);
}

void BlumMachine::suspendre() {
    moteur.stop();
    moteur.disable();
    moteur.update();   // PWM à 0 et STBY à LOW
    StateMachine<ETAT>::suspendre();
}

void BlumMachine::reprendre() {
    capteurs.isBlocage();   // Efface un éventuel blocage mémorisé avant le test
    if (_etat == ETAT::CALIBRATION) {
        // Calibration interrompue : retour à REPOS avec calibration requise (LED clignotante)
        changerEtat(ETAT::CALIBRATION_REQUIS);
        sendInfo(MSG::ERR_CALIBRATION_ANNULEE);
        return;
    }
    StateMachine<ETAT>::reprendre();
}