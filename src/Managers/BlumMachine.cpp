#include "BlumMachine.h"

BlumMachine::BlumMachine(SensorsManager& s, Motor& m, Buzzer& b) 
        : StateMachine<ETAT>(s,m,b), calibration(s,m,b), pilotage(s,m,b) {
    name = MSG::ETAT_PROD;
}

void BlumMachine::init() {
    ledCalibration.init();
    ledError.init();
    calibration.init();
    pilotage.init();
}

void BlumMachine::start() {
    setEtat(ETAT::INIT);
    if (calibration.isNotCalibrated()) {
        sendDirect(MSGTYPE::WARNING, MSG::ERR_CALIBRATION_REQUISE);
        changerEtat(ETAT::CALIBRATION_REQUIS);
    }
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
            pilotage.stop();
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
            moteur.enable();
            ledError.off();
            pilotage.start();
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

void BlumMachine::execState(ETAT etat) {
    switch(etat) {
        case ETAT::INIT: {
            //capteurs.setConsigne(0);
            changerEtat(ETAT::REPOS);
            break;
        }
        case ETAT::REPOS: {
            //capteurs.setConsigne(0);
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
            if (!pilotage.exec()) {
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
            //capteurs.setConsigne(moteur.getPWM());
            break;
        }
        case ETAT::CALIBRATION_REQUIS: {
            ledCalibration.update(); // Update the calibration LED blinking
            //capteurs.setConsigne(0);
            break;
        }
        case ETAT::CALIBRATION: {
            if (!calibration.exec()) {
                changerEtat(ETAT::DEBRAYAGE);
            }
            //capteurs.setConsigne(moteur.getPWM());
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

void BlumMachine::suspendre() {
    moteur.stop();
    moteur.disable();
    moteur.update();   // PWM à 0 et STBY à LOW
    StateMachine<ETAT>::suspendre();
}

void BlumMachine::reprendre() {
    capteurs.isBlocage();   // Efface un éventuel blocage mémorisé avant le test
    if (getEtat() == ETAT::CALIBRATION) {
        // Calibration interrompue : retour à REPOS avec calibration requise (LED clignotante)
        changerEtat(ETAT::CALIBRATION_REQUIS);
        sendInfo(MSG::ERR_CALIBRATION_ANNULEE);
    }
    if (getEtat() == ETAT::PILOTAGE) {
        // Pilotage interrompu : retour à REPOS
        changerEtat(ETAT::REPOS);
        sendInfo(MSG::ERR_PILOTAGE_STOPPE);
    }
    StateMachine<ETAT>::reprendre();
}