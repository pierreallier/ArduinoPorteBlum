#include "CalibrationManager.h"
#include "Messages.h"
#include "Constantes.h"

CalibrationManager::CalibrationManager(Motor& m, Sensors& c) : moteur(m), capteurs(c) { 
    //loadFromEeprom(); // Initialiser les données à partir de la RAM
    etat = CalibrationManager::ETAT::NONE;
    eepromActive = false;
    setDefaultValues(ON_FURNITURE);
    setDefaultValues(OFF_FURNITURE);
    updateConfig(getConfig());
}


void CalibrationManager::setEepromActive(bool active) { // Activer/désactiver l'utilisation de l'EEPROM
    if (eepromActive == active) return; // Pas de changement
    eepromActive = active;
    if (eepromActive) {
        loadFromEeprom(); // Charger les données depuis l'EEPROM
    }
    saveToEeprom(); // Sauvegarder le nouvel état
}


void CalibrationManager::saveToEeprom() { // Sauvegarder toutes les données en EEPROM
    if (!eepromActive) return;
    // Sauvegarder le flag d'activation
    EEPROM.put(EEPROM_ADDR_ACTIVE_FLAG, eepromActive);
    // Sauvegarder les données pour chaque configuration
    EEPROM.put(EEPROM_ADDR_ON_FURNITURE, calibrationData[ON_FURNITURE]);
    EEPROM.put(EEPROM_ADDR_OFF_FURNITURE, calibrationData[OFF_FURNITURE]);
}

    
void CalibrationManager::loadFromEeprom() { // Charger toutes les données depuis l'EEPROM
    // Charger le flag d'activation
    bool storedActiveFlag;
    EEPROM.get(EEPROM_ADDR_ACTIVE_FLAG, storedActiveFlag);
    if (!storedActiveFlag) {
        // Si l'EEPROM est désactivée, initialiser avec les valeurs par défaut
        setDefaultValues(ON_FURNITURE);
        setDefaultValues(OFF_FURNITURE);
        eepromActive = false;
    } else {
        // Si l'EEPROM est activée, charger les données
        eepromActive = true;
        EEPROM.get(EEPROM_ADDR_ON_FURNITURE, calibrationData[ON_FURNITURE]);
        EEPROM.get(EEPROM_ADDR_OFF_FURNITURE, calibrationData[OFF_FURNITURE]);
        // Vérifier si les données sont valides (non initialisées)
        if (isCalibrationUninitialized(ON_FURNITURE)) {
            setDefaultValues(ON_FURNITURE);
        }
        if (isCalibrationUninitialized(OFF_FURNITURE)) {
            setDefaultValues(OFF_FURNITURE);
        }
    }
}

    
void CalibrationManager::setCalibration(CalibrationManager::Config config, uint16_t highLimit, uint16_t lowLimit, uint16_t offset) { // Définir les valeurs d'étalonnage pour une configuration
    calibrationData[config].highLimit = highLimit;
    calibrationData[config].lowLimit = lowLimit;
    calibrationData[config].offset = offset;
    updateConfig(config);
    if (eepromActive) {
        saveToEeprom();
    }
}

   
bool CalibrationManager::getCalibration(CalibrationManager::Config config, int& highLimit, int& lowLimit, int& offset) {
    if (isCalibrationUninitialized(config)) { // Valeurs non initialisées
        return false;
    }
    highLimit = calibrationData[config].highLimit;
    lowLimit = calibrationData[config].lowLimit;
    offset = calibrationData[config].offset;
    return true;
}


bool CalibrationManager::isCalibrationUninitialized(CalibrationManager::Config config) {
    return (calibrationData[config].highLimit > 1024);
}

    // Définir des valeurs par défaut pour une configuration
void CalibrationManager::setDefaultValues(CalibrationManager::Config config) {
    // Valeurs par défaut
    calibrationData[config].highLimit = UNINITIALIZED_VALUE;
    calibrationData[config].lowLimit = 0;
    calibrationData[config].offset = 0;
}

bool CalibrationManager::ouverture(uint16_t speed) {
    /* Gestion de l'ouverture de la porte en BO, retourne false si en cours, true si fini */
    moteur.setDirection(Motor::DIR::OUVERTURE);
    moteur.setSpeed(speed);
    capteurs.setConsigne(speed);
    return false;
}

bool CalibrationManager::fermeture(uint16_t speed) {
    /* Gestion de la fermeture de la porte en BO, retourne false si en cours, true si fini */
    moteur.setDirection(Motor::DIR::FERMETURE);
    moteur.setSpeed(speed);
    capteurs.setConsigne(speed);
    return false;
}


void CalibrationManager::changerEtat(CalibrationManager::ETAT nouvelle_etape) {
    time_etat = millis();
    etat = nouvelle_etape;
}

bool CalibrationManager::exec() {
    /* Gestion de l'état calibration */
    switch (etat) {
        case CalibrationManager::ETAT::DEBUT: {
            sendInfo("Debut de calibration");
            moteur.enable();
            changerEtat(CalibrationManager::ETAT::OUVERTURE_INITIALE);
        }
        case CalibrationManager::ETAT::OUVERTURE_INITIALE: {
            if (ouverture(PWM_CALIBRATION) || capteurs.isBlocage()) {
                moteur.stop();
                changerEtat(CalibrationManager::ETAT::ATTENTE_HAUT);
            }
            break;
        }
        case CalibrationManager::ETAT::ATTENTE_HAUT: {
            if (millis() - time_etat >= 1000) {;
                changerEtat(CalibrationManager::ETAT::RECHERCHE_BUTEE_BASSE);
            }
            break;
        }
        case CalibrationManager::ETAT::RECHERCHE_BUTEE_BASSE: {
            if (fermeture(PWM_CALIBRATION) || capteurs.isBlocage()) { // Faire une gestion interne ?
                angle_butee_basse = analogRead(CODEUR_PORTE); // lire directement depuis le capteur
                moteur.stop();
                changerEtat(CalibrationManager::ETAT::ATTENTE_BAS);
            }
            break;
        }
        case CalibrationManager::ETAT::ATTENTE_BAS: {
            if (millis() - time_etat >= 1000) {
                changerEtat(CalibrationManager::ETAT::RECHERCHE_BUTEE_HAUTE);
            }
            break;
        }
        case CalibrationManager::ETAT::RECHERCHE_BUTEE_HAUTE: {
            if (ouverture(PWM_CALIBRATION) || capteurs.isBlocage()) {
                angle_butee_haute = analogRead(CODEUR_PORTE); // lire directement depuis le capteur
                moteur.stop();
                is_calibre = true;
                changerEtat(CalibrationManager::ETAT::ATTENTE_ENREGISTREMENT);
            }
            break;
        }
        case CalibrationManager::ETAT::DEBRAYAGE_FINAL: {
            changerEtat(CalibrationManager::ETAT::ATTENTE_ENREGISTREMENT);
            break;
        }
        case CalibrationManager::ETAT::ATTENTE_ENREGISTREMENT: {
            if (millis() - time_etat >= 1000) {
                sendReponseOK("DO","CALIBRATION","Fin de calibration : limite haute=" + String(angle_butee_haute) + " ; limite basse=" + String(angle_butee_basse));
                setCalibration(getConfig(), (angle_butee_haute + (1024 - angle_butee_basse)) % 1024, 0, 1024 - angle_butee_basse);
                changerEtat(CalibrationManager::ETAT::NONE);
            }
            break;
        }
        case CalibrationManager::ETAT::NONE:
            return true;
        default:
            break;
    }
    return false;
}

