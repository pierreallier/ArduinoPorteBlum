#include "StateMachine.h"

StateMachine::StateMachine(Motor& m, SensorsManager& s) : moteur(m), capteurs(s), calibration(m,s), consigne() {
}

void StateMachine::init() {
    etat = StateMachine::ETAT::INIT;
    modePilotage = StateMachine::MODE_PILOTAGE::PWM;
    time_etat = millis();

    pinMode(LED_PILOTAGE_PIN, OUTPUT);
    pinMode(LED_ERROR_PIN, OUTPUT);

    calibration.init();
}

void StateMachine::changerEtat(StateMachine::ETAT etat_demande) {
    if (etat == etat_demande)
        return;
    if ((etat == StateMachine::ETAT::CALIBRATION) && calibration.isNotCalibrated()) {
        sendError(MSG::ERR_CALIBRATION_ANNULEE);
        calibration.changerEtat(CalibrationManager::ETAT::ERREUR);
    }
    if (calibration.isNotCalibrated() && (etat_demande == StateMachine::ETAT::PILOTAGE || etat_demande == StateMachine::ETAT::OUVERTURE ||etat_demande == StateMachine::ETAT::FERMETURE)) {
        sendError(MSG::ERR_CALIBRATION_REQUISE);
        etat_demande = StateMachine::ETAT::ERREUR;
    }
    etat = etat_demande;
    time_etat = millis();
    sendEtat(MSG::ETAT_PROD,(uint32_t)etat);

    // Initiliations des états
    switch (etat) {
        case StateMachine::ETAT::DEBRAYAGE: {
            moteur.enable();
            moteur.debrayage();
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
            digitalWrite(LED_ERROR_PIN, LOW);
            digitalWrite(LED_PILOTAGE_PIN, HIGH);
            butee_desactivated = true;
            break;
        }
        case StateMachine::ETAT::CALIBRATION: {
            calibration.changerEtat(CalibrationManager::ETAT::DEBUT);
            digitalWrite(LED_ERROR_PIN, LOW);
            break;
        }
        case StateMachine::ETAT::ERREUR: {
            digitalWrite(LED_ERROR_PIN, HIGH);
            break;
        }
        default: {
            moteur.disable();
            digitalWrite(LED_MOTOR_PIN, LOW);
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
             if (capteurs.getAnglePorte() < 50)
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
                changerEtat(StateMachine::ETAT::DEBRAYAGE);
            }
            capteurs.setConsigne(moteur.getPWM());
            break;
        }
        case StateMachine::ETAT::ERREUR: {
            changerEtat(StateMachine::ETAT::DEBRAYAGE);
            break;
        }
        default: {
            changerEtat(StateMachine::ETAT::DEBRAYAGE);
            break;
        }
    }
    calibration.task();
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

bool StateMachine::etatDebrayage() {
    /* Gestion du debrayage du moteur */
    int32_t delta_angle = abs(capteurs.getAngleMoteur() - moteur.codeur_avant_debrayage);
    //float delta_courant = abs(1 - moteur.courant_avant_debrayage/capteurs.getCourant());
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
    if (etat == StateMachine::ETAT::PILOTAGE) {
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

    if (etat == StateMachine::ETAT::CALIBRATION) {
        // Calibration interrompue : retour à REPOS avec calibration requise (LED clignotante)
        calibration.changerEtat(CalibrationManager::ETAT::DEBUT);
        digitalWrite(LED_CALIBRATION_PIN, LOW);
        time_etat = millis();
        sendEtat(MSG::ETAT_PROD,(uint32_t)etat);
        sendInfo(MSG::ERR_CALIBRATION_ANNULEE);
        return;
    }
    time_etat = millis();
}