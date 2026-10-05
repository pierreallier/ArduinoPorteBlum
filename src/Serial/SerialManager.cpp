#include "SerialManager.h"
#include <avr/wdt.h>

void resetArduino() {
    wdt_enable(WDTO_15MS); // Redémarrage dans ~15 ms
    while (true) {
        // Attendre le reset
    }
}

SerialManager::SerialManager(Motor& m, Sensors& s, StateMachine& ma, CalibrationManager& c) : moteur(m),capteurs(s),machine(ma),calibration(c) {
}

void SerialManager::init() {
    Serial.begin(115200);
    Serial.flush();
    loadMesurePeriode();
    printStart();
}

void SerialManager::task() {
    if (millis() - time_precedent >= 100) {
        time_precedent += 100;
        readSerial(); // Traitement des données reçues
        SerialTxBuffer::instance().send(Serial); // Envoi des données en attente
    }
}

void SerialManager::readSerial() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim(); // Supprime les espaces et les retours à la ligne

        if (command.startsWith("SET")) {
            _SET(command.substring(3));
        } else if (command.startsWith("GET")) {
            _GET(command.substring(3));
        } else if (command.startsWith("DO")) {
            _DO(command.substring(2));
        } else {
            sendError("Commande inconnue {SET,GET,DO}.");
        }
    }
}

void SerialManager::_SET(String commande) {
    commande.trim(); // Supprime les espaces et les retours à la ligne
    commande.toUpperCase(); // Convertit la commande

    if (commande.startsWith("CALIBRATION") && machine.etat == StateMachine::ETAT::REPOS) {
        String valeurs = commande.substring(11);
        valeurs.trim();
        valeurs.toUpperCase();
        // Lecture du statut de l'activation
        bool active;
        if (valeurs == "ON") {
            active = true;
        }
        else if (valeurs == "OFF") {
            active = false;
        }
        else if (valeurs == "EFFACER") {
            calibration.clearEeprom();
        }
        else {
            sendReponseNOK("SET","CALIBRATION","Valeur attendue : ON / OFF / EFFACER");
            return;
        }
        calibration.setEepromActive(active);
        sendReponseOK("SET", "CALIBRATION", calibration.getCalibrationString());
        return;
    }
    else if (commande.startsWith("COURANT") && machine.etat == StateMachine::ETAT::REPOS) {
        String valeurs = commande.substring(8);
        valeurs.trim();
        float limite = valeurs.substring(0).toFloat();
        if (capteurs.setLimitCourant(limite))
            sendReponseOK("SET","COURANT","Limite de courant modifiée à la valeur " + String(capteurs.getLimitCourant(), 2));
        else
            sendReponseNOK("SET","COURANT","Limite de courant incompatible (entre 0 et 2.0). Valeur non modifiée");
    }
    // Configuration du mode de pilotage
    else if (commande.startsWith("MODE") && machine.etat != StateMachine::ETAT::PILOTAGE) {
        String valeur = commande.substring(5);
        valeur.trim();
        if (valeur == "PWM")
            machine.setModePilotage(StateMachine::MODE_PILOTAGE::PWM);
        else if (valeur == "VITESSE")
            machine.setModePilotage(StateMachine::MODE_PILOTAGE::VITESSE);
        else if (valeur == "POSITION")
            machine.setModePilotage(StateMachine::MODE_PILOTAGE::POSITION);
        else if (valeur == "POSITION_VITESSE")
            machine.setModePilotage(StateMachine::MODE_PILOTAGE::POSITION_VITESSE);
        else  {
            sendReponseNOK("SET","MODE","Mode de pilotage inconnu : " + valeur);
            return;
        }
        // C'est le stateMachine qui renvoie le message de la bonne execution du changement
    }
    // Configuration des Consignes
    else if (commande.startsWith("CONSIGNE")) {
        String items[10];
        int nbItems = splitCommande(commande, items, 10);
        if (nbItems < 2) {
            sendReponseNOK("SET","CONSIGNE","Type de consigne manquant {POTENTIOMETRE,ECHELON,RAMPE,TRAPEZE,SINUS}");
            return;
        }
        if (machine.setConsigne(items[1], &items[2], nbItems - 2)) {
            sendReponseOK("SET","CONSIGNE","Consigne mise à jour");
        }
    }
    // Configuration des PID
    else if (commande.startsWith("PID") && machine.etat != StateMachine::ETAT::PILOTAGE) {
        String parametres = commande.substring(4);
        parametres.trim();
        // Recherche des séparateurs
        int sep1 = parametres.indexOf(' ');
        int sep2 = parametres.indexOf(' ', sep1 + 1);
        int sep3 = parametres.indexOf(' ', sep2 + 1);
        if (sep1 == -1 || sep2 == -1 || sep3 == -1) {
            sendReponseNOK("SET","PID","TYPE [VITESSE|POSITION] KP KI KD attendus");
            return;
        }
        String type_pid = parametres.substring(0, sep1);
        float kp = parametres.substring(sep1 + 1,sep2).toFloat();
        float ki = parametres.substring(sep2 + 1,sep3).toFloat();
        float kd = parametres.substring(sep3 + 1).toFloat();
        if (type_pid == "VITESSE")
            machine.setPIDVitesse(kp, ki, kd);
        else if (type_pid == "POSITION")
            machine.setPIDPosition(kp, ki, kd);
        else {
            sendReponseNOK("SET","PID","inconnu : " + type_pid);
            return;
        }
        sendReponseOK("SET","PID","Configuration du PID effectuée");
    }
    // Gestion de la fréquence d'envoie des mesures
    else if (commande.startsWith("MESURES")) {
        String valeur = commande.substring(8);
        valeur.trim();
        // Vérification : chaîne non vide
        if (valeur.length() == 0) {
            sendReponseNOK("SET","MESURES","periode manquante");
            return;
        }
        // Vérification : uniquement des chiffres
        for (unsigned int i = 0; i < valeur.length(); i++) {
            if (!isDigit(valeur[i])) {
                sendReponseNOK("SET","MESURES","periode invalide");
                return;
            }
        }
        uint16_t periode = valeur.toInt();
        periode = (periode / 5)*5;
        if (periode > 1275) {
            sendReponseNOK("SET","MESURES","Periode trop grande " + String(periode));
            return;
        }
        if (periode >= 5) {
            setMesurePeriode(periode);
            sendReponseOK("SET","MESURES","Envoi des mesures toutes les " + String(periode) + " ms");
        } else {
            setMesurePeriode(1275);
            sendReponseOK("SET","MESURES","Envoi des mesures désactivé");
        }
    }
    else {
        sendError("Commande SET inconnue {LIMITES,COURANT,MODE,CONSIGNE,PID,MESURES}");
        return;
    }

}

void SerialManager::_GET(String commande) {
    commande.trim(); // Supprime les espaces et les retours à la ligne
    commande.toUpperCase(); // Convertit la commande

    if (commande.startsWith("CALIBRATION")) {
        if (calibration.isCalibrated())
            sendReponseOK("GET","CALIBRATION","Système calibré;" + calibration.getCalibrationString());
        else 
            sendReponseNOK("GET","CALIBRATION","Système non calibré;" + calibration.getCalibrationString());
    }
    else if (commande.startsWith("LIMITS")) {
        if (calibration.isCalibrated())
            sendReponseOK("GET","LIMITS","Système calibré : limite basse=" + String(capteurs.getLimiteBasse()) + " limite haute=" + String(capteurs.getLimiteHaute()));
        else 
            sendReponseNOK("GET","LIMITS","Système non calibré : limite basse=" + String(capteurs.getLimiteBasse()) + " limite haute=" + String(capteurs.getLimiteHaute()));

    }
    else if (commande.startsWith("MESURES")) {
        String valeur = commande.substring(7);
        valeur.trim();
        // Vérification : chaîne non vide
        if (valeur.length() == 0) {
            sendReponseNOK("GET","MESURES","nom du capteur manquant");
            return;
        }
        if (valeur == "PERIODE") {
            if (getMesurePeriode() > 1000) {
                sendReponseOK("GET","MESURES PERIODE","Désactivé");
            } else {
                sendReponseOK("GET","MESURES PERIODE",String(getMesurePeriode()));
            }
        }
        if (valeur == "MEUBLE") {
            sendReponseOK("GET","MESURES MEUBLE",calibration.getEtat() ? "Sur meuble": "Hors meuble");
        }
        if (valeur == "PORTE") {
            sendReponseOK("GET","MESURES PORTE",String(capteurs.getRawCodeurPorte()) + " Brut, " + String(capteurs.angle_porte) + "CAN et " + String(capteurs.getCodeurPorte()) + "°");
        }
    }
    else if (commande.startsWith("MODE")) {
        switch (machine.modePilotage) {
            case StateMachine::MODE_PILOTAGE::PWM: {
                sendReponseOK("GET","MODE","PWM");
                break;
            }
            case StateMachine::MODE_PILOTAGE::VITESSE: {
                sendReponseOK("GET","MODE","VITESSE");
                break;
            }
            case StateMachine::MODE_PILOTAGE::POSITION: {
                sendReponseOK("GET","MODE","POSITION");
                break;
            }
            case StateMachine::MODE_PILOTAGE::POSITION_VITESSE: {
                sendReponseOK("GET","MODE","POSITION_VITESSE");
                break;
            }
            default: {
                sendReponseNOK("GET","MODE","Invalide");
                break;
            }
        }
    }
    else if (commande.startsWith("PID")) {
        sendReponseOK("GET","PID VITESSE",(String)machine.pidVitesse.kp + ";" + (String)machine.pidVitesse.ki + ";" + (String)machine.pidVitesse.kd);
        sendReponseOK("GET","PID POSITION",(String)machine.pidPosition.kp + ";" + (String)machine.pidPosition.ki + ";" + (String)machine.pidPosition.kd);
    }
    else
        sendError("Commande GET inconnue {CALIBRATION,MODE,PID}.");
}

void SerialManager::_DO(String commande) {
    commande.trim(); // Supprime les espaces et les retours à la ligne
    commande.toUpperCase(); // Convertit la commande

    if (commande.startsWith("RESET")) {
        sendReponseOK("DO","RESET","Effectuée");
        resetArduino();
        return;
    }
    if (commande.startsWith("TEST")) {
        if (machine.etat == StateMachine::ETAT::REPOS ||
            machine.etat == StateMachine::ETAT::CALIBRATION) {
            demandeTest = true;
            sendReponseOK("DO", "TEST", "Demandée");
        } else {
            sendReponseNOK("DO", "TEST", "Possible uniquement au repos ou en calibration");
        }
        return;
    }
    if (commande.startsWith("INIT"))
        machine.changerEtat(StateMachine::ETAT::INIT);
    else if (commande.startsWith("OUVRIR"))
        machine.changerEtat(StateMachine::ETAT::OUVERTURE);
    else if (commande.startsWith("FERMER"))
        machine.changerEtat(StateMachine::ETAT::FERMETURE);
    else if (commande.startsWith("STOP"))
        machine.changerEtat(StateMachine::ETAT::DEBRAYAGE);
    else if (commande.startsWith("PILOTER"))
        machine.changerEtat(StateMachine::ETAT::PILOTAGE);
    else if (commande.startsWith("CALIBRATION")) {
        machine.changerEtat(StateMachine::ETAT::CALIBRATION);
        return;
    }
    else {
        sendError("Commande DO inconnue {RESET,INIT,OUVRIR,FERMER,STOP,PILOTER,CALIBRATION,TEST}.");
        return;
    }
    sendReponseOK("DO",commande.c_str(),"Effectuée");
}

void SerialManager::loadMesurePeriode() {
    periode_echantillonnage_mesures = static_cast<uint32_t>(EEPROM.read(EEPROM_ADDR_TENVOIS))*5;
}

void SerialManager::setMesurePeriode(uint16_t periode) {
    uint8_t periode_eeprom = periode / 5;
    EEPROM.update(EEPROM_ADDR_TENVOIS, periode_eeprom);
    periode_echantillonnage_mesures = periode_eeprom * 5;
}

uint32_t SerialManager::getMesurePeriode() {
    return periode_echantillonnage_mesures;
}

int SerialManager::splitCommande(const String& commande, String items[], int maxItems) {
    // Découpe un string de commande sur l'espace et renvoi un tableau contenant chaque items
    int nbItems = 0;
    int debut = 0;

    while (debut < commande.length() && nbItems < maxItems) {
        // Ignorer les espaces
        while (debut < commande.length() && commande[debut] == ' ')
            debut++;
        if (debut >= commande.length())
            break;
        int fin = commande.indexOf(' ', debut);
        if (fin == -1){
            items[nbItems++] = commande.substring(debut);
            break;
        }
        items[nbItems++] = commande.substring(debut, fin);
        debut = fin + 1;
    }
    return nbItems;
}