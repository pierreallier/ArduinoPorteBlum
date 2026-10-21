#include "PilotageMachine.h"

PilotageMachine::PilotageMachine(SensorsManager& s, Motor& m, Buzzer& b) 
        : StateMachine<ETAT_PILOTAGE>(s,m,b), pidPosition(), pidVitesse(), consigne() {
    name = MSG::ETAT_PILOTAGE;
}

void PilotageMachine::init() {
    ledPilotage.init();
}

void PilotageMachine::start() {
    setEtat(ETAT_PILOTAGE::INIT);
    StateMachine<ETAT_PILOTAGE>::start();
}

void PilotageMachine::stop() {
    setEtat(ETAT_PILOTAGE::FIN);
    StateMachine<ETAT_PILOTAGE>::stop();
    ledPilotage.off();
}

ETAT_PILOTAGE PilotageMachine::checkTransition(ETAT_PILOTAGE etat_demande) {
    return etat_demande;
}

void PilotageMachine::execStateEntry(ETAT_PILOTAGE etat) {
    switch (etat) {
        case ETAT_PILOTAGE::EN_COURS: {
            sendDirect(MSGTYPE::INFO, MSG::INFO_PILOTAGE_DEMARRE);
            break;
        }
        case ETAT_PILOTAGE::ERREUR: {
            stop();
            break;
        }
        case ETAT_PILOTAGE::FIN: {
            sendInfo(MSG::INFO_PILOTAGE_FINI);
            stop();
            break;
        }
        default:
            break;
    }
}

void PilotageMachine::execStateExit(ETAT_PILOTAGE etat) {
    if (etat == ETAT_PILOTAGE::INIT) {
        pidPosition.reset();
        pidVitesse.reset();
        consigne.init(millis());
    }
}

void PilotageMachine::execState(ETAT_PILOTAGE etat) {
    switch (etat) {
        case ETAT_PILOTAGE::INIT: {
            ledPilotage.on();
            changerEtat(ETAT_PILOTAGE::EN_COURS);
            break;
        }
        case ETAT_PILOTAGE::EN_COURS: {
            if (!etatPilote()) {
                changerEtat(ETAT_PILOTAGE::FIN);
            }
            break;
        }
        case ETAT_PILOTAGE::ERREUR: {
            ledPilotage.off();
            break;
        }
        case ETAT_PILOTAGE::FIN: {
            ledPilotage.off();
            break;
        }
    }
}

void PilotageMachine::setMode(PilotageMachine::MODE mode) {
    switch (mode) {
        case PilotageMachine::MODE::PWM:
            sendReponseOK(MSG::DATA_MODE_PWM);
            break;
        case PilotageMachine::MODE::VITESSE:
            sendReponseOK(MSG::DATA_MODE_VITESSE);
            break;
        case PilotageMachine::MODE::POSITION:
            sendReponseOK(MSG::DATA_MODE_POSITION);
            break;
        case PilotageMachine::MODE::POSITION_VITESSE:
            sendReponseOK(MSG::DATA_MODE_PV);
            break;
        default:
            sendReponseNOK(MSG::ERR_PILOTAGE_SET);
            return;
    }
    modePilotage = mode;
}

void PilotageMachine::setMode(const String& modeStr) {
    if (getEtat() != ETAT_PILOTAGE::EN_COURS) {
        if (modeStr == "PWM") {
            setMode(PilotageMachine::MODE::PWM);
            sendReponseOK(MSG::DATA_MODE_PWM);
            return;
        } else if (modeStr == "VITESSE") {
            setMode(PilotageMachine::MODE::VITESSE);
            sendReponseOK(MSG::DATA_MODE_VITESSE);
            return;
        } else if (modeStr == "POSITION") {
            setMode(PilotageMachine::MODE::POSITION);
            sendReponseOK(MSG::DATA_MODE_POSITION);
            return;
        } else if (modeStr == "POSITION_VITESSE") {
            setMode(PilotageMachine::MODE::POSITION_VITESSE);
            sendReponseOK(MSG::DATA_MODE_PV);
            return;
        } 
    }
    // Dans tous les autres cas, on renvoie un message d'erreur
    sendReponseNOK(MSG::ERR_PILOTAGE_SET);
}

bool PilotageMachine::setConsigne(const String& type, const String* params, int nbParams) {
    if (getEtat() == ETAT_PILOTAGE::EN_COURS) {
        sendError(MSG::ERR_CONSIGNE_EN_PILOTAGE);
        return false;
    }
    return consigne.setConsigne(type, params, nbParams);
}

bool PilotageMachine::etatPilote() {
    /* Gestion du pilotage de la porte via consigne */
    unsigned long time = millis();
    if (consigne.ended(time)) {
        moteur.stop();
        buzzer.sequenceFinPilotage(); // Si mode Potentiomètre, la consigne ne se termine pas, donc pas de bip
        return false;
    }
    float consigne_value = consigne.get(time);
    float pwm = 0.0f;
    switch (modePilotage) {
        case PilotageMachine::MODE::PWM: {
            pwm = constrain(consigne_value,-255,255);
            capteurs.setConsigne(pwm);
            break;
        }
        case PilotageMachine::MODE::VITESSE: {
            capteurs.setConsigne(consigne_value);
            pwm = pidVitesse.compute(consigne_value,capteurs.getVitesseMoteur(),time);
            break;
        }
        case PilotageMachine::MODE::POSITION: {
            consigne_value = constrain(consigne_value,-150,150);
            capteurs.setConsigne(consigne_value);
            pwm = pidPosition.compute(consigne_value,capteurs.getAnglePorte(),time);
            break;
        }
        case PilotageMachine::MODE::POSITION_VITESSE: {
            consigne_value = constrain(consigne_value,-120,180);
            capteurs.setConsigne(consigne_value);
            float consigneVitesse = pidPosition.compute(consigne_value,capteurs.getAnglePorte(),time);
            pwm = pidVitesse.compute(consigneVitesse,capteurs.getVitesseMoteur(), time);
            break;
        }
    }
    moteur.setSpeedDir(pwm);
    return true;
}