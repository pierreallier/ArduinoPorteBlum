#include "StateMachine.h"

StateMachine::StateMachine(Motor& m, Sensors& c) : moteur(m), capteurs(c), consignePotentiometre(), consigne(&consignePotentiometre) {
    etat = StateMachine::ETAT::INIT;
    modePilotage = StateMachine::MODE_PILOTAGE::PWM;
    etape_calibration = StateMachine::ETAPE_CALIBRATION::NONE;
    time_etat = millis();
    butee_desactivated = false;
    is_calibre = false;
}

void StateMachine::init() {
    etat = StateMachine::ETAT::INIT;
    modePilotage = StateMachine::MODE_PILOTAGE::PWM;
    etape_calibration = StateMachine::ETAPE_CALIBRATION::NONE;
    time_etat = millis();
    butee_desactivated = false;
    is_calibre = false;
}

void StateMachine::changerEtat(StateMachine::ETAT etat_demande) {
    if (etat == etat_demande)
        return;
    if ((etat == StateMachine::ETAT::CALIBRATION) && !is_calibre) {
        pushMessage(Message::TYPE::ERREUR,"Calibration échouée");
        return;
    }
    if (!is_calibre && etat_demande == StateMachine::ETAT::PILOTAGE) {
        pushMessage(Message::TYPE::ERREUR,"Calibration requise");
        return;
    }
    etat = etat_demande;
    time_etat = millis();
    pushMessage(Message::TYPE::ETAT,(String)((int)etat));

    // Initiliations des états
    switch (etat) {
        case StateMachine::ETAT::DEBRAYAGE: {
            butee_desactivated = false;
            moteur.enable();
            moteur.debrayage();
            break;
        }
        case StateMachine::ETAT::OUVERTURE:
        case StateMachine::ETAT::FERMETURE:
            moteur.enable();
            break;

        case StateMachine::ETAT::PILOTAGE: {
            pidPosition.reset();
            pidVitesse.reset();
            consigne->init(time_etat);
            moteur.enable();
            butee_desactivated = true;
            break;
        }
        case StateMachine::ETAT::CALIBRATION: {
            is_calibre = false;
            capteurs.resetLimits();
            butee_desactivated = true;
            changerEtapeCalibration(ETAPE_CALIBRATION::OUVERTURE_INITIALE);
            moteur.enable();
            pushMessage(Message::TYPE::INFO, "Debut de calibration");
            break;
        }
        default: {
            moteur.disable();
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
    }
    pushMessage(Message::TYPE::INFO,message);
}

void StateMachine::setConsigne(Consigne& c) {
    consigne = &c;
    pushMessage(Message::TYPE::INFO, "consigne " + c.getName());
}

void StateMachine::exec() {
    switch(etat) {
        case StateMachine::ETAT::INIT: {
            changerEtat(StateMachine::ETAT::REPOS);
            break;
        }
        case StateMachine::ETAT::REPOS: {
            break;
        }
        case StateMachine::ETAT::FONCTIONNEMENT: {
             if (capteurs.angle_porte < -100)
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
                changerEtat(StateMachine::ETAT::DEBRAYAGE);
            }
            break;
        }
        case StateMachine::ETAT::DEBRAYAGE: { // ou Arrêt
            if (etatDebrayage()) {
                changerEtat(StateMachine::ETAT::REPOS);
            }
            break;
        }
        case StateMachine::ETAT::CALIBRATION: {
            if (etatCalibration()) {
                changerEtat(StateMachine::ETAT::DEBRAYAGE);
            }
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
    if (capteurs.limite_basse) {
        moteur.stop();
        return true;
    }
    return false;
}

bool StateMachine::etatDebrayage() {
    /* Gestion du debrayage du moteur */
    int32_t delta_angle = abs(capteurs.angle_moteur - moteur.codeur_avant_debrayage);
    float delta_courant = abs(1 - moteur.courant_avant_debrayage/capteurs.courant_moyen);
    if (millis() - time_etat >= 100 || delta_angle > 100 || delta_courant > 0.5) {
        moteur.stop();
        return true;
    }
    return false;
}

bool StateMachine::etatPilote() {
    /* Gestion du pilotage de la porte via consigne */
    if (consigne == nullptr) {
        moteur.stop();
        pushMessage(Message::TYPE::ERREUR, "Consigne nulle en mode PILOTAGE");
        return true;
    }
    unsigned long time = millis();
    if (consigne->ended(time)) {
        moteur.stop();
        return true;
    }
    float consigne_value = consigne->get(time);
    float pwm = 0.0f;
    switch (modePilotage) {
        case StateMachine::MODE_PILOTAGE::PWM:
            pwm = constrain(consigne_value,-255,255);
            break;
        case StateMachine::MODE_PILOTAGE::VITESSE:
            pwm = pidVitesse.compute(consigne_value,capteurs.vitesse_moteur,time);
            break;
        case StateMachine::MODE_PILOTAGE::POSITION:
            consigne_value = constrain(consigne_value,-120,180);
            pwm = pidPosition.compute(consigne_value,capteurs.angle_porte,time);
            break;
        case StateMachine::MODE_PILOTAGE::POSITION_VITESSE: {
            consigne_value = constrain(consigne_value,-120,180);
            float consigneVitesse = pidPosition.compute(consigne_value,capteurs.angle_porte,time);
            pwm = pidVitesse.compute(consigneVitesse,capteurs.vitesse_moteur, time);
            break;
        }
    }
    moteur.setSpeedDir(pwm);
    return false;
}

bool StateMachine::etatCalibration() {
    /* Gestion de l'état calibration */
    switch (etape_calibration) {
        case StateMachine::ETAPE_CALIBRATION::OUVERTURE_INITIALE: {
            if (etatOuverture(250) || capteurs.isBlocage(true)) {
                moteur.stop();
                changerEtapeCalibration(StateMachine::ETAPE_CALIBRATION::ATTENTE_HAUT);
            }
            break;
        }
        case StateMachine::ETAPE_CALIBRATION::ATTENTE_HAUT: {
            if (millis() - time_etape_calibration >= 1000) {
                moteur.debrayage();
                changerEtapeCalibration(ETAPE_CALIBRATION::DEBRAYAGE_HAUT);
            }
            break;
        }
        case StateMachine::ETAPE_CALIBRATION::DEBRAYAGE_HAUT: {
            if (etatDebrayage()) {
                changerEtapeCalibration(StateMachine::ETAPE_CALIBRATION::RECHERCHE_BUTEE_BASSE);
            }
            break;
        }
        case StateMachine::ETAPE_CALIBRATION::RECHERCHE_BUTEE_BASSE: {
            if (etatFermeture(250) || capteurs.isBlocage(true)) {
                angle_butee_basse = capteurs.angle_porte;
                moteur.stop();
                changerEtapeCalibration(StateMachine::ETAPE_CALIBRATION::ATTENTE_BAS);
            }
            break;
        }
        case StateMachine::ETAPE_CALIBRATION::ATTENTE_BAS: {
            if (millis() - time_etape_calibration >= 1000) {
                moteur.debrayage();
                changerEtapeCalibration(ETAPE_CALIBRATION::DEBRAYAGE_BAS);
            }
            break;
        }
        case StateMachine::ETAPE_CALIBRATION::DEBRAYAGE_BAS: {
            if (etatDebrayage()) {
                changerEtapeCalibration(StateMachine::ETAPE_CALIBRATION::RECHERCHE_BUTEE_HAUTE);
            }
            break;
        }
        case StateMachine::ETAPE_CALIBRATION::RECHERCHE_BUTEE_HAUTE: {
            if (etatOuverture(250) || capteurs.isBlocage(true)) {
                angle_butee_haute = capteurs.angle_porte;
                is_calibre = true;
                changerEtapeCalibration(StateMachine::ETAPE_CALIBRATION::DEBRAYAGE_FINAL);
                moteur.debrayage();
            }
            break;
        }
        case StateMachine::ETAPE_CALIBRATION::DEBRAYAGE_FINAL: {
            if (etatDebrayage()) {
                butee_desactivated = false;
                pushMessage(Message::TYPE::INFO, "Fin de calibration : limite haute=" + String(angle_butee_haute) + "° ; limite basse=" + String(angle_butee_basse) + "°");
                moteur.stop();
                changerEtapeCalibration(StateMachine::ETAPE_CALIBRATION::ATTENTE_ENREGISTREMENT);
            }
            break;
        }
        case StateMachine::ETAPE_CALIBRATION::ATTENTE_ENREGISTREMENT: {
            if (millis() - time_etape_calibration >= 1000) {
                capteurs.setLimits(angle_butee_basse,angle_butee_haute);
                changerEtapeCalibration(ETAPE_CALIBRATION::NONE);
                changerEtat(StateMachine::ETAT::REPOS);
            }
            break;
        }
        case StateMachine::ETAPE_CALIBRATION::NONE:
            return true;
        default:
            break;
    }
    return false;
}

void StateMachine::changerEtapeCalibration(StateMachine::ETAPE_CALIBRATION nouvelle_etape) {
    time_etape_calibration = millis();
    etape_calibration = nouvelle_etape;
}



bool StateMachine::pushMessage(Message::TYPE t, String message) {
    if (nbElements >= TAILLE_FIFO)
        return false;           // FIFO pleine
    Message m = {t,message};
    messages[queue] = m;
    queue = (queue + 1) % TAILLE_FIFO;
    nbElements++;

    return true;
}

bool StateMachine::popMessage(Message &m) {
    if (nbElements == 0)
        return false;
    m = messages[tete];
    tete = (tete + 1) % TAILLE_FIFO;
    nbElements--;

    return true;
}

bool StateMachine::hasMessage() const {
    return nbElements > 0;
}

Message StateMachine::getMessage() {
    Message cmd;
    if (!popMessage(cmd))
        return {Message::TYPE::AUCUN,""}; 
    return cmd;
}