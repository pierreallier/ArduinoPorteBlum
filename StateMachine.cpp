#include "StateMachine.h"

StateMachine::StateMachine(Motor& m, Sensors& c) : moteur(m), capteurs(c), consignePotentiometre(), consigne(&consignePotentiometre) {
    etat = StateMachine::ETAT::INIT;
    modePilotage = StateMachine::MODE_PILOTAGE::PWM;
    time_etat = millis();
}

void StateMachine::init() {
    etat = StateMachine::ETAT::INIT;
    modePilotage = StateMachine::MODE_PILOTAGE::PWM;
    time_etat = millis();
}

void StateMachine::changerEtat(StateMachine::ETAT etat_demande) {
    if (etat == etat_demande)
        return;
    if ((etat == StateMachine::ETAT::PILOTAGE) && (etat_demande!=StateMachine::ETAT::DEBRAYAGE)) {
        pushMessage(Message::TYPE::ERREUR,"Changement depuis piloté vers un état impossible");
        return;
    }
    etat = etat_demande;
    time_etat = millis();
    pushMessage(Message::TYPE::ETAT,(String)((int)etat));
    if (etat == StateMachine::ETAT::DEBRAYAGE) {
        moteur.enable();
        moteur.debrayage();
    } else if (etat == StateMachine::ETAT::OUVERTURE || 
               etat == StateMachine::ETAT::FERMETURE || 
               etat == StateMachine::ETAT::CALIBRATION) {
        moteur.enable();
    } else if (etat == StateMachine::ETAT::PILOTAGE) {
        pidPosition.reset();
        pidVitesse.reset();
        consigne->init(time_etat);
        moteur.enable();
    } else {
        moteur.disable();
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
    unsigned long time = millis();
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
            moteur.setDirection(Motor::DIR::OUVERTURE);
            moteur.setSpeed(abs(capteurs.potentiometre));
            if (capteurs.limite_haute) {
                moteur.stop();
                changerEtat(StateMachine::ETAT::DEBRAYAGE);
            }
            break;
        }

        case StateMachine::ETAT::FERMETURE: {
            moteur.setDirection(Motor::DIR::FERMETURE);
            moteur.setSpeed(abs(capteurs.potentiometre));
            if (capteurs.limite_basse) {
                moteur.stop();
                changerEtat(StateMachine::ETAT::DEBRAYAGE);
            }
            break;
        }

        case StateMachine::ETAT::PILOTAGE: {
            if (consigne == nullptr) {
                moteur.stop();
                pushMessage(Message::TYPE::ERREUR, "Consigne nulle en mode PILOTAGE");
                changerEtat(StateMachine::ETAT::DEBRAYAGE);
            }
            bool consigne_terminee = consigne->ended(time);
            if (consigne_terminee) {
                moteur.stop();
                changerEtat(StateMachine::ETAT::DEBRAYAGE);
            } else {
                etatPilote(consigne->get(time), time);
            }
            break;
        }

        case StateMachine::ETAT::CALIBRATION: {
            // Calibration_Run();
            break;
        }

        case StateMachine::ETAT::DEBRAYAGE: { // ou Arrêt
            int32_t delta_angle = abs(capteurs.angle_moteur - moteur.codeur_avant_debrayage);
            float delta_courant = abs(1 - moteur.courant_avant_debrayage/capteurs.courant_moyen);
            if (time - time_etat >= 100 || delta_angle > 100 || delta_courant > 0.5) {
                moteur.stop();
                changerEtat(StateMachine::ETAT::REPOS);
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

void StateMachine::etatPilote(float consigne, unsigned long time) {
    float pwm = 0.0f;
    switch (modePilotage) {
        case StateMachine::MODE_PILOTAGE::PWM:
            pwm = constrain(consigne,-255,255);
            
            break;
        case StateMachine::MODE_PILOTAGE::VITESSE:
            pwm = pidVitesse.compute(consigne,capteurs.vitesse_moteur,time);
            break;
        case StateMachine::MODE_PILOTAGE::POSITION:
            consigne = constrain(consigne,-120,180);
            pwm = pidPosition.compute(consigne,capteurs.angle_porte,time);
            break;
        case StateMachine::MODE_PILOTAGE::POSITION_VITESSE: {
            consigne = constrain(consigne,-120,180);
            float consigneVitesse = pidPosition.compute(consigne,capteurs.angle_porte,time);
            pwm = pidVitesse.compute(consigneVitesse,capteurs.vitesse_moteur, time);
            break;
        }
    }
    Serial.println(pwm);
    moteur.setSpeedDir(pwm);
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