#include "CalibrationManager.h"
#include "Messages.h"
#include "Constantes.h"

CalibrationManager::CalibrationManager(Motor& m, Sensors& c) : moteur(m), capteurs(c) { 
    etat = CalibrationManager::ETAT::NONE;
    loadFromEeprom(); // Initialiser les données à partir de la RAM
    updateCapteurs(getConfig());
}


void CalibrationManager::setEepromActive(bool active) { // Activer/désactiver l'utilisation de l'EEPROM
    if (eepromActive == active) return; // Pas de changement
    eepromActive = active;
    uint8_t flag = eepromActive ? 1 : 0;
    EEPROM.update(EEPROM_ADDR_ACTIVE_FLAG, flag);
    if (eepromActive) {
        loadFromEeprom(); // Charger les données depuis l'EEPROM
    }
}

void CalibrationManager::clearEeprom() { // Efface les valeurs stockées dans l'EEPROM
    setDefaultValues(ON_FURNITURE);
    EEPROM.put(EEPROM_ADDR_ON_FURNITURE, calibrationData[ON_FURNITURE]);
    setDefaultValues(OFF_FURNITURE);
    EEPROM.put(EEPROM_ADDR_OFF_FURNITURE, calibrationData[OFF_FURNITURE]);
    updateCapteurs(getConfig());
}
   
void CalibrationManager::loadFromEeprom() { // Charger toutes les données depuis l'EEPROM
    // Charger le flag d'activation
    uint8_t storedActiveFlag;
    EEPROM.get(EEPROM_ADDR_ACTIVE_FLAG, storedActiveFlag);
    // Gestion EEPROM corrompue ou non initilisée
    if (storedActiveFlag != 0 && storedActiveFlag != 1) {
        // Flag invalide ou EEPROM non initialisée
        eepromActive = false;
        setDefaultValues(ON_FURNITURE);
        setDefaultValues(OFF_FURNITURE);
        // Réinitialisation du flag à une valeur valide
        EEPROM.update(EEPROM_ADDR_ACTIVE_FLAG, 0);
        return;
    } else if (storedActiveFlag == 0) {
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
            EEPROM.put(EEPROM_ADDR_ON_FURNITURE, calibrationData[ON_FURNITURE]);
        }
        if (isCalibrationUninitialized(OFF_FURNITURE)) {
            setDefaultValues(OFF_FURNITURE);
            EEPROM.put(EEPROM_ADDR_OFF_FURNITURE, calibrationData[OFF_FURNITURE]);
        }
    }
}

    
void CalibrationManager::setCalibration(CalibrationManager::Config config, uint16_t highLimit, uint16_t lowLimit, uint16_t offset) { // Définir les valeurs d'étalonnage pour une configuration
    calibrationData[config].highLimit = highLimit;
    calibrationData[config].lowLimit = lowLimit;
    calibrationData[config].offset = offset;
    updateCapteurs(config);
    if (eepromActive) {
        if (config == ON_FURNITURE) {
            EEPROM.put(EEPROM_ADDR_ON_FURNITURE, calibrationData[ON_FURNITURE]);
        } else if (config == OFF_FURNITURE) {
            EEPROM.put(EEPROM_ADDR_OFF_FURNITURE, calibrationData[OFF_FURNITURE]);
        }
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

String CalibrationManager::getCalibrationString() {
    String message;

    message = "Mémorisation EEPROM : ";
    message += eepromActive ? "ON" : "OFF";

    message += " ; Monté sur meuble : ";
    message += "limite haute=";
    message += String(calibrationData[ON_FURNITURE].highLimit);
    message += ", limite basse=";
    message += String(calibrationData[ON_FURNITURE].lowLimit);
    message += ", offset=";
    message += String(calibrationData[ON_FURNITURE].offset);

    message += " ; Démonté du meuble : ";
    message += "limite haute=";
    message += String(calibrationData[OFF_FURNITURE].highLimit);
    message += ", limite basse=";
    message += String(calibrationData[OFF_FURNITURE].lowLimit);
    message += ", offset=";
    message += String(calibrationData[OFF_FURNITURE].offset);

    return message;
}


bool CalibrationManager::isCalibrationUninitialized(CalibrationManager::Config config) {
    return (calibrationData[config].highLimit > 1024 || calibrationData[config].lowLimit > 1024 || calibrationData[config].offset > 1024);
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
    // Vérification des sécurités (car non gérées au niveau supérieur)
    bool blocage = capteurs.isBlocage();
    if (false && !blocage && capteurs.isLimiteCourant()) {
        changerEtat(CalibrationManager::ETAT::ERREUR);
        moteur.stop();
    }
    // Execution des états
    switch (etat) {
        case CalibrationManager::ETAT::DEBUT: {
            sendInfo("Debut de calibration");
            moteur.enable();
            changerEtat(CalibrationManager::ETAT::OUVERTURE_INITIALE);
        }
        case CalibrationManager::ETAT::OUVERTURE_INITIALE: {
            if (ouverture(PWM_CALIBRATION) || blocage) {
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
            if (fermeture(PWM_CALIBRATION) || blocage) {
                angle_butee_basse = analogRead(CODEUR_PORTE);
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
            if (ouverture(PWM_CALIBRATION) || blocage) {
                angle_butee_haute = analogRead(CODEUR_PORTE);
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
                setCalibration(getConfig(), (angle_butee_haute + (675 - angle_butee_basse)) % 675, 0, 675 - angle_butee_basse);
                changerEtat(CalibrationManager::ETAT::NONE);
            }
            break;
        }
        case CalibrationManager::ETAT::ERREUR: {
            if (millis() - time_etat >= 1000) {
                sendReponseNOK("DO","CALIBRATION","Calibration échouée - Limite de courant atteinte");
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

