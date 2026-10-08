#include "SerialManager.h"
#include <EEPROM.h>
#include <avr/wdt.h>

void resetArduino() {
    wdt_enable(WDTO_15MS); // Redémarrage dans ~15 ms
    while (true) {
        // Attendre le reset
    }
}

SerialManager::SerialManager(SensorsManager& s, StateMachine& ma, Buzzer& b) : capteurs(s),machine(ma),buzzer(b) {
}

void SerialManager::init() {
    Serial.begin(DEBIT);
    Serial.flush();
    loadMesurePeriode();
    printStart();
}

void SerialManager::task() {
    if (millis() - time_precedent >= 100) {
        time_precedent += 100;
        readSerial(); // Traitement des données reçues
        sendEvents();
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
            sendError(MSG::ERR_CMD_INCONNUE);
        }
    }
}

bool SerialManager::testRequired() {
    bool val = demandeTest;
    demandeTest=0;
    return val;
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
            machine.calibrationMachine().clearEeprom();
            active = machine.calibrationMachine().isEepromActive();
        }
        else {
            sendReponseNOK(MSG::ERR_CALIBRATION_SET);
            return;
        }
        machine.calibrationMachine().setEepromActive(active);
        sendReponseOK(MSG::INFO_CALIBRATION_SET);
        return;
    }
    else if (commande.startsWith("COURANT") && machine.etat == StateMachine::ETAT::REPOS) {
        String valeurs = commande.substring(8);
        valeurs.trim();
        float limite = valeurs.substring(0).toFloat();
        if (capteurs.setLimitCourant(limite))
            sendReponseOK(MSG::INFO_COURANT_SET);
        else
            sendReponseNOK(MSG::ERR_COURANT_SET);
    }
    // Configuration du mode de pilotage
    else if (commande.startsWith("MODE") && machine.etat != StateMachine::ETAT::PILOTAGE) {
        String valeur = commande.substring(5);
        valeur.trim();
        if (valeur == "PWM")
            machine.setMode(StateMachine::MODE_PILOTAGE::PWM);
        else if (valeur == "VITESSE")
            machine.setMode(StateMachine::MODE_PILOTAGE::VITESSE);
        else if (valeur == "POSITION")
            machine.setMode(StateMachine::MODE_PILOTAGE::POSITION);
        else if (valeur == "POSITION_VITESSE")
            machine.setMode(StateMachine::MODE_PILOTAGE::POSITION_VITESSE);
        // C'est le stateMachine qui renvoie le message de la bonne execution du changement
    }
    // Configuration des Consignes
    else if (commande.startsWith("CONSIGNE")) {
        String items[10];
        int nbItems = splitCommande(commande, items, 10);
        if (nbItems < 2) {
            sendReponseNOK(MSG::ERR_CONSIGNE_SET);
            return;
        }
        if (machine.setConsigne(items[1], &items[2], nbItems - 2)) {
            sendReponseOK(MSG::INFO_CONSIGNE_SET);
        } else {
            sendReponseNOK(MSG::ERR_CONSIGNE_SET);
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
            sendReponseNOK(MSG::ERR_PID_SET);
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
            sendReponseNOK(MSG::ERR_PID_TYPE_SET);
            return;
        }
        sendReponseOK(MSG::INFO_PID_SET);
    }
    // Gestion de la fréquence d'envoie des mesures
    else if (commande.startsWith("MESURES")) {
        String valeur = commande.substring(8);
        valeur.trim();
        // Vérification : chaîne non vide
        if (valeur.length() == 0) {
            sendReponseNOK(MSG::ERR_FREQ_MESURES_SET);
            return;
        }
        // Activation (sans sauvegarde valeur)
        if (valeur == "ON") {
            loadMesurePeriode();
            sendReponseOK(MSG::DATA_FREQ_MESURES, periode_echantillonnage_mesures);
            return;
        }
        // Désactivation (sans sauvegarde valeur)
        if (valeur == "OFF") {
            setMesurePeriode(0);
            sendReponseOK(MSG::INFO_MESURES_DESACTIVE);
            return;
        }
        // Vérification : uniquement des chiffres
        for (unsigned int i = 0; i < valeur.length(); i++) {
            if (!isDigit(valeur[i])) {
                sendReponseNOK(MSG::ERR_FREQ_MESURES_SET);
                return;
            }
        }
        uint16_t periode = valeur.toInt();
        periode = (periode / 5)*5;
        if (periode > 1275) {
            sendReponseNOK(MSG::ERR_FREQ_MESURES_SET);
            return;
        }
        if (periode >= 5) {
            setMesurePeriode(periode, true);
            sendReponseOK(MSG::DATA_FREQ_MESURES, periode_echantillonnage_mesures);
        } else {
            setMesurePeriode(0, true);
            sendReponseOK(MSG::INFO_MESURES_DESACTIVE);
        }

    }
    // Gestion de l'accéléromètre
    else if (commande.startsWith("ACCEL")) {
        String valeur = commande.substring(6);
        valeur.trim();
        valeur.toUpperCase();
        // Lecture du statut de l'activation
        bool active;
        if (valeur == "ON") {
            if (capteurs.bno().isConnected()) {
                active = true;
            } else {
                sendReponseNOK(MSG::ERR_ACCEL_ABSENT);
                return;
            }
        }
        else if (valeur == "OFF") {
            active = false;
        } else {
            sendReponseNOK(MSG::ERR_ACCEL_SET);
            return;
        }
        capteurs.setBNO(active);
        sendReponseOK(MSG::INFO_ACCEL_SET, capteurs.bno().isEnabled());
    }
    // Gestion du buzzer
    else if (commande.startsWith("BUZZER")) {
        String valeur = commande.substring(7);
        valeur.trim();
        valeur.toUpperCase();
        // Lecture du statut de l'activation
        if (valeur == "ON") {
            buzzer.enable();
        } else if (valeur == "OFF") {
            buzzer.disable();
        } else {
            sendReponseNOK(MSG::ERR_BUZZER_SET);
            return;
        }
        sendReponseOK(MSG::INFO_BUZZER,buzzer.isEnabled());
    }
    else {
        sendError(MSG::ERR_CMD_SET_INCONNUE);
        return;
    }

}

void SerialManager::_GET(String commande) {
    commande.trim(); // Supprime les espaces et les retours à la ligne
    commande.toUpperCase(); // Convertit la commande

    if (commande.startsWith("CALIBRATION")) {
        if (machine.isCalibrated()) {
            sendMessage(MSG::DATA_COURSE, capteurs.getCalibration().highLimit);
            sendMessage(MSG::DATA_OFFSET, capteurs.getCalibration().offset);
            sendReponseOK(MSG::INFO_CALIBRE);
        }
        else 
            sendReponseNOK(MSG::ERR_NON_CALIBRE);
    }
    else if (commande.startsWith("LIMITS")) {
        sendMessage(MSG::DATA_LIMITE_BASSE,capteurs.getLimiteBasse());
        sendMessage(MSG::DATA_LIMITE_HAUTE,capteurs.getLimiteHaute());
        sendReponseOK(MSG::INFO_CALIBRE);
    }
    else if (commande.startsWith("MESURES")) {
        String valeur = commande.substring(7);
        valeur.trim();
        // Vérification : chaîne non vide
        if (valeur.length() == 0) {
            sendReponseNOK(MSG::ERR_MESURE_GET);
            return;
        }
        if (valeur == "PERIODE") {
            if (getMesurePeriode() > 1000) {
                sendReponseOK(MSG::INFO_MESURES_DESACTIVE);
            } else {
                sendReponseOK(MSG::DATA_FREQ_MESURES,getMesurePeriode());
            }
        }
        if (valeur == "MEUBLE") {
            if (machine.calibrationMachine().getEtat()) 
                sendReponseOK(MSG::INFO_SUR_MEUBLE);
            else
                sendReponseOK(MSG::INFO_DEMONTE);
        }
        if (valeur == "PORTE") {
            sendReponseOK(MSG::DATA_ANGLE_PORTE, capteurs.getAnglePorte()*100);
        }
        if (valeur == "TENSION") {
            sendReponseOK(MSG::DATA_TENSION, capteurs.getTension()*100);
        }
        if (valeur == "COURANT") {
            sendReponseOK(MSG::DATA_COURANT, capteurs.getCourant()*100);
        }
        if (valeur == "MOTEUR") {
            sendMessage(MSG::DATA_ANGLE_MOTEUR, capteurs.getAngleMoteur()*100);
            sendReponseOK(MSG::DATA_VITESSE_MOTEUR, capteurs.getVitesseMoteur()*100);
        }
        if (valeur == "PWM") {
            sendReponseOK(MSG::DATA_PWM, capteurs.getPWM()*100);
        }
        if (valeur == "CONSIGNE") {
            sendReponseOK(MSG::DATA_CONSIGNE, capteurs.getConsigne()*100);
        }
    }
    else if (commande.startsWith("MODE")) {
        switch (machine.modePilotage) {
            case StateMachine::MODE_PILOTAGE::PWM: {
                sendReponseOK(MSG::DATA_MODE_PWM);
                break;
            }
            case StateMachine::MODE_PILOTAGE::VITESSE: {
                sendReponseOK(MSG::DATA_MODE_VITESSE);
                break;
            }
            case StateMachine::MODE_PILOTAGE::POSITION: {
                sendReponseOK(MSG::DATA_MODE_POSITION);
                break;
            }
            case StateMachine::MODE_PILOTAGE::POSITION_VITESSE: {
                sendReponseOK(MSG::DATA_MODE_PV);
                break;
            }
            default: {
                sendReponseNOK(MSG::ERR_MODE_GET);
                break;
            }
        }
    }
    else if (commande.startsWith("PID")) {
        sendMessage(MSG::DATA_PID_VITESSE_KP);
        sendMessage(MSG::DATA_PID_VITESSE_KI);
        sendMessage(MSG::DATA_PID_VITESSE_KD);
        sendMessage(MSG::DATA_PID_POSITION_KP);
        sendMessage(MSG::DATA_PID_POSITION_KI);
        sendReponseOK(MSG::DATA_PID_POSITION_KD);
    }
    else if (commande.startsWith("VERSION")) {
        sendMessage(MSG::INFO_VERSION_CARTE,VERSION_CARTE);
        sendMessage(MSG::INFO_VERSION_SOFT,VERSION_SOFT);
        sendReponseOK(MSG::INFO_VERSION_DEV,VERSION_DEV);
    }
    else if (commande.startsWith("ACCEL")) {
        capteurs.bno().isConnected() ? sendReponseOK(MSG::INFO_ACCEL_GET,capteurs.bno().isEnabled()) 
                                     : sendReponseNOK(MSG::ERR_ACCEL_ABSENT);
    }
    else if (commande.startsWith("BUZZER")) {
        sendReponseOK(MSG::INFO_BUZZER, buzzer.isEnabled());
    }
    else
        sendError(MSG::ERR_CMD_GET_INCONNUE);
}

void SerialManager::_DO(String commande) {
    commande.trim(); // Supprime les espaces et les retours à la ligne
    commande.toUpperCase(); // Convertit la commande

    if (commande.startsWith("RESET")) {
        sendDirect('I',MSG::INFO_DO_RESET);
        resetArduino();
        return;
    }
    if (commande.startsWith("TEST")) {
        if (machine.etat == StateMachine::ETAT::REPOS ||
            machine.etat == StateMachine::ETAT::CALIBRATION) {
            demandeTest = true;
            sendDirect('I',"TEST Demandée");
        } else {
            sendReponseNOK(MSG::ERR_TEST_IMPOSSIBLE);
        }
        return;
    }
    if (commande.startsWith("INIT")) {
        machine.changerEtat(StateMachine::ETAT::INIT);
        sendReponseOK(MSG::INFO_DO_INIT);
    }
    else if (commande.startsWith("OUVRIR")) {
        machine.changerEtat(StateMachine::ETAT::OUVERTURE);
        sendReponseOK(MSG::INFO_DO_OUVRIR);
    }
    else if (commande.startsWith("FERMER")) {
        machine.changerEtat(StateMachine::ETAT::FERMETURE);
        sendReponseOK(MSG::INFO_DO_FERMER);
    }
    else if (commande.startsWith("STOP")) {
        machine.changerEtat(StateMachine::ETAT::DEBRAYAGE);
        sendReponseOK(MSG::INFO_DO_STOP);
    }
    else if (commande.startsWith("PILOTER")) {
        machine.changerEtat(StateMachine::ETAT::PILOTAGE);
        sendReponseOK(MSG::INFO_DO_PILOTER);
    }
    else if (commande.startsWith("CALIBRATION")) {
        machine.changerEtat(StateMachine::ETAT::CALIBRATION);
        sendReponseOK(MSG::INFO_DO_CALIBRATION);
    }
    else {
        sendError(MSG::ERR_CMD_DO_INCONNUE);
        return;
    }
    
}

void SerialManager::loadMesurePeriode() {
    periode_echantillonnage_mesures = static_cast<uint32_t>(EEPROM.read(EEPROM_ADDR_TENVOIS))*5;
}

void SerialManager::setMesurePeriode(uint16_t periode, bool save) {
    periode = (periode < 4) ? 1275 : periode;
    uint8_t periode_eeprom = periode / 5;
    if (save) {
        EEPROM.update(EEPROM_ADDR_TENVOIS, periode_eeprom);
    }
    periode_echantillonnage_mesures = periode_eeprom * 5;
}

uint32_t SerialManager::getMesurePeriode() {
    return periode_echantillonnage_mesures;
}

int SerialManager::splitCommande(const String& commande, String items[], int maxItems) {
    // Découpe un string de commande sur l'espace et renvoi un tableau contenant chaque items
    int nbItems = 0;
    unsigned int debut = 0;

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