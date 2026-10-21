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
}

void SerialManager::task() {
    if (millis() - time_precedent >= 100) {
        time_precedent += 100;
        readSerial(); // Traitement des données reçues
        
    }
}

void SerialManager::readSerial() {
    if (Serial.available() > 0) {
        String commande = Serial.readStringUntil('\n');
        commande.trim(); // Supprime les espaces et les retours à la ligne
        commande.toUpperCase(); // Convertit la commande

        if (commande.startsWith("SET")) {
            _SET(commande.substring(4));
        } else if (commande.startsWith("GET")) {
            _GET(commande.substring(4));
        } else if (commande.startsWith("DO")) {
            _DO(commande.substring(3));
        } else {
            sendReponseNOK(MSG::ERR_CMD_INCONNUE);
        }
    }
}

bool SerialManager::testRequired() {
    bool val = demandeTest;
    demandeTest=0;
    return val;
}

void SerialManager::_SET(String commande) {
    
    if (commande.startsWith("CALIBRATION") && machine.etat == ETAT::REPOS) {
        String status = getNextItem(commande); // On ignore le premier mot "CALIBRATION"
        // Lecture du statut de l'activation
        bool active;
        if (status == "ON") {
            active = true;
        }
        else if (status == "OFF") {
            active = false;
        }
        else if (status == "EFFACER") {
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
    else if (commande.startsWith("COURANT") && machine.etat == ETAT::REPOS) {
        float limite = getNextItem(commande).toFloat();
        if (capteurs.setLimitCourant(limite))
            sendReponseOK(MSG::INFO_COURANT_SET);
        else
            sendReponseNOK(MSG::ERR_COURANT_SET);
    }
    // Configuration du mode de pilotage
    else if (commande.startsWith("MODE") && machine.etat != ETAT::PILOTAGE) {
        String mode = getNextItem(commande);
        if (mode == "PWM")
            machine.setMode(StateMachine::MODE_PILOTAGE::PWM);
        else if (mode == "VITESSE")
            machine.setMode(StateMachine::MODE_PILOTAGE::VITESSE);
        else if (mode == "POSITION")
            machine.setMode(StateMachine::MODE_PILOTAGE::POSITION);
        else if (mode == "POSITION_VITESSE")
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
    else if (commande.startsWith("PID") && machine.etat != ETAT::PILOTAGE) {
        String items[4];
        int nbItems = splitCommande(commande, items, 4);
        if (nbItems < 4) {
            sendReponseNOK(MSG::ERR_PID_SET);
            return;
        }
        String type_pid = items[1];
        float kp = items[2].toFloat();
        float ki = items[3].toFloat();
        float kd = items[4].toFloat();
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
        String param = getNextItem(commande);
        // Vérification : chaîne non vide
        if (param.length() == 0) {
            sendReponseNOK(MSG::ERR_FREQ_MESURES_SET);
            return;
        }
        // Activation (sans sauvegarde valeur)
        if (param == "ON") {
            loadMesurePeriode();
            sendReponseOK(MSG::DATA_FREQ_MESURES, periode_echantillonnage_mesures);
            return;
        }
        // Désactivation (sans sauvegarde valeur)
        if (param == "OFF") {
            setMesurePeriode(0);
            sendReponseOK(MSG::INFO_MESURES_DESACTIVE);
            return;
        }
        if (param == "SAVE") {
            saveMesurePeriode();
            sendReponseOK(MSG::INFO_MESURES_SAUVEGARDE, periode_echantillonnage_mesures);
            return;
        }
        // Vérification : uniquement des chiffres
        for (unsigned int i = 0; i < param.length(); i++) {
            if (!isDigit(param[i])) {
                sendReponseNOK(MSG::ERR_FREQ_MESURES_SET);
                return;
            }
        }
        uint16_t periode = param.toInt();
        periode = (periode / 5)*5;
        if (periode > 1275) {
            sendReponseNOK(MSG::ERR_FREQ_MESURES_SET);
            return;
        }
        if (periode >= 5) {
            setMesurePeriode(periode, false);
            sendReponseOK(MSG::DATA_FREQ_MESURES, periode_echantillonnage_mesures);
        } else {
            setMesurePeriode(0, false);
            sendReponseOK(MSG::INFO_MESURES_DESACTIVE);
        }

    }
    // Gestion de l'accéléromètre
    else if (commande.startsWith("ACCEL")) {
        String param = getNextItem(commande);
        // Lecture du statut de l'activation
        bool active;
        if (param == "ON") {
            if (capteurs.bno().isConnected()) {
                active = true;
            } else {
                sendReponseNOK(MSG::ERR_ACCEL_ABSENT);
                return;
            }
        }
        else if (param == "OFF") {
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
        String param = getNextItem(commande);
        // Lecture du statut de l'activation
        if (param == "ON") {
            buzzer.enable();
        } else if (param == "OFF") {
            buzzer.disable();
        } else {
            sendReponseNOK(MSG::ERR_BUZZER_SET);
            return;
        }
        sendReponseOK(MSG::INFO_BUZZER,buzzer.isEnabled());
    }
    else {
        sendReponseNOK(MSG::ERR_CMD_SET_INCONNUE);
        return;
    }

}

void SerialManager::_GET(String commande) {
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
        sendMessage(MSG::DATA_LIMITE_COURANT, capteurs.getLimitCourant()*100);
        sendMessage(MSG::DATA_LIMITE_BASSE,capteurs.getLimiteBasse());
        sendMessage(MSG::DATA_LIMITE_HAUTE,capteurs.getLimiteHaute());
        sendReponseOK(MSG::INFO_CALIBRE);
    }
    else if (commande.startsWith("MESURES")) {
        String capteur = getNextItem(commande); // On ignore le premier mot "MESURES"
        // Vérification : chaîne non vide
        if (capteur.length() == 0) {
            sendReponseNOK(MSG::ERR_MESURE_GET);
            return;
        }
        if (capteur == "PERIODE") {
            if (getMesurePeriode() > 1000) {
                sendReponseOK(MSG::INFO_MESURES_DESACTIVE);
            } else {
                sendReponseOK(MSG::DATA_FREQ_MESURES,getMesurePeriode());
            }
        }
        if (capteur == "MEUBLE") {
            if (machine.calibrationMachine().getEtat()) 
                sendReponseOK(MSG::INFO_SUR_MEUBLE);
            else
                sendReponseOK(MSG::INFO_DEMONTE);
        }
        if (capteur == "PORTE") {
            sendReponseOK(MSG::DATA_ANGLE_PORTE, capteurs.getAnglePorte()*100);
        }
        if (capteur == "TENSION") {
            sendMessage(MSG::DATA_TENSION, capteurs.getTension()*100);
            sendReponseOK(MSG::DATA_TENSION_MOTEUR, capteurs.getTensionMoteur()*100);
        }
        if (capteur == "COURANT") {
            sendMessage(MSG::DATA_COURANT, capteurs.getCourant()*100);
            sendReponseOK(MSG::DATA_LAST_COURANT, capteurs.getLastCourant()*100);
        }
        if (capteur == "MOTEUR") {
            sendMessage(MSG::DATA_ANGLE_MOTEUR, capteurs.getAngleMoteur()*100);
            sendReponseOK(MSG::DATA_VITESSE_MOTEUR, capteurs.getVitesseMoteur()*100);
        }
        if (capteur == "PWM") {
            sendReponseOK(MSG::DATA_PWM, capteurs.getPWM()*100);
        }
        if (capteur == "CONSIGNE") {
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
        sendReponseNOK(MSG::ERR_CMD_GET_INCONNUE);
}

void SerialManager::_DO(String commande) {

    if (commande.startsWith("RESET")) {
        sendDirect(MSGTYPE::REPONSE_OK,MSG::INFO_DO_RESET);
        resetArduino();
        return;
    }
    if (commande.startsWith("TEST")) {
        if (machine.etat == ETAT::REPOS ||
            machine.etat == ETAT::CALIBRATION) {
            demandeTest = true;
            sendDirect(MSGTYPE::REPONSE_OK,MSG::INFO_DO_TEST);
        } else {
            sendReponseNOK(MSG::ERR_TEST_IMPOSSIBLE);
        }
        return;
    }
    if (commande.startsWith("INIT")) {
        machine.changerEtat(ETAT::INIT);
        sendReponseOK(MSG::INFO_DO_INIT);
    }
    else if (commande.startsWith("ACTIONNER")) {
        machine.changerEtat(ETAT::FONCTIONNEMENT);
        sendReponseOK(MSG::INFO_DO_FONCTIONNEMENT);
    }
    else if (commande.startsWith("OUVRIR")) {
        machine.changerEtat(ETAT::OUVERTURE);
        sendReponseOK(MSG::INFO_DO_OUVRIR);
    }
    else if (commande.startsWith("FERMER")) {
        machine.changerEtat(ETAT::FERMETURE);
        sendReponseOK(MSG::INFO_DO_FERMER);
    }
    else if (commande.startsWith("STOP")) {
        machine.changerEtat(ETAT::DEBRAYAGE);
        sendReponseOK(MSG::INFO_DO_STOP);
    }
    else if (commande.startsWith("PILOTER")) {
        machine.changerEtat(ETAT::PILOTAGE);
        sendDirect(MSGTYPE::REPONSE_OK,MSG::INFO_DO_PILOTER); // On envoie le message tout de suite pour démarrer l'acquistion
    }
    else if (commande.startsWith("CALIBRATION")) {
        machine.changerEtat(ETAT::CALIBRATION);
        sendReponseOK(MSG::INFO_DO_CALIBRATION);
    }
    else {
        sendReponseNOK(MSG::ERR_CMD_DO_INCONNUE);
        return;
    }
    
}

void SerialManager::loadMesurePeriode() {
    periode_echantillonnage_mesures = static_cast<uint32_t>(EEPROM.read(EEPROM_ADDR_TENVOIS))*5;
}

void SerialManager::setMesurePeriode(uint16_t periode, bool save) {
    periode_echantillonnage_mesures = (periode < 4) ? 1275 : periode;
    if (save) {
        saveMesurePeriode();
    }
}

void SerialManager::saveMesurePeriode() {
    uint8_t periode_eeprom = periode_echantillonnage_mesures / 5;
    EEPROM.update(EEPROM_ADDR_TENVOIS, periode_eeprom);
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

String SerialManager::getNextItem(const String& commande) {
    // Découpe un message sur l'esapce et renvoie le 2nd item (après le premier mot)
    int debut = commande.indexOf(' ');
    if (debut == -1) {
        return "";
    }
    int fin = commande.indexOf(' ', debut + 1);
    if (fin == -1) {
        return commande.substring(debut + 1);
    }
    return commande.substring(debut + 1, fin);
}