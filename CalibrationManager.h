#ifndef CALIBRATIONMANAGER_H
#define CALIBRATIONMANAGER_H

#include <EEPROM.h>
#include "Types.h"
#include "Motor.h"
#include "Sensors.h"

// Adresses EEPROM
#define EEPROM_ADDR_ACTIVE_FLAG   0   // 1 octet (bool) + 1 octet CRC
#define EEPROM_ADDR_ON_FURNITURE  2   // 6 octets (3x uint16_t) + 1 octet CRC
#define EEPROM_ADDR_OFF_FURNITURE 8 // 6 octets (3x uint16_t) + 1 octet CRC

// Valeur pour indiquer qu'une valeur n'est pas initialisée
#define UNINITIALIZED_VALUE 0xFFFF

const uint16_t PWM_CALIBRATION = 250; // Vitesse moteur pour la calibration

class CalibrationManager {
public:
    // Enum pour les deux configurations
    enum Config {
        ON_FURNITURE,
        OFF_FURNITURE
    };

    // Les différents états pour la calibration
    enum class ETAT : uint8_t {
        DEBUT,
        OUVERTURE_INITIALE,
        ATTENTE_HAUT,
        DEBRAYAGE_HAUT,
        RECHERCHE_BUTEE_BASSE,
        ATTENTE_BAS,
        DEBRAYAGE_BAS,
        RECHERCHE_BUTEE_HAUTE,
        ATTENTE_ENREGISTREMENT,
        DEBRAYAGE_FINAL,
        ERREUR,
        NONE,
    };

    
    CalibrationManager(Motor& m, Sensors& c); // Constructeur
    
    void setEepromActive(bool active); // Activer/désactiver l'utilisation de l'EEPROM
    void setCalibration(CalibrationManager::Config config, uint16_t highLimit, uint16_t lowLimit, uint16_t offset); // Définir les valeurs d'étalonnage pour une configuration
    bool getCalibration(CalibrationManager::Config config, int& highLimit, int& lowLimit, int& offset); // Obtenir les valeurs d'étalonnage pour une configuration. Retourne true si les valeurs sont valides, false sinon

    bool isCalibrationInitialized() { // Vérifier si les données sont initialisées
        return !isCalibrationUninitialized(getConfig());
    }

    bool isCalibrated() {
        return !isCalibrationUninitialized(getConfig());
    }
    bool isNotCalibrated() {
        return isCalibrationUninitialized(getConfig());
    }

    bool isEepromActive() const { // Vérifier si l'EEPROM est active
        return eepromActive;
    }

    Config getConfig() {
        return capteurs.getMeuble() ? CalibrationManager::Config::ON_FURNITURE : CalibrationManager::Config::OFF_FURNITURE;
    }

    String getCalibrationString();
    void clearEeprom();

    void updateCapteurs(CalibrationManager::Config config) {
        capteurs.setLimits(calibrationData[config]);
    }
    void updateCapteurs() {
        capteurs.setLimits(calibrationData[getConfig()]);
    }

    // Gestion machine à état
    void changerEtat(CalibrationManager::ETAT nouvelle_etape); // Changer d'état 
    bool exec(); // Execution de la machine à états

private:
    // Gestion EEPROM
    void loadFromEeprom(); // Charger toutes les données depuis l'EEPROM
    void saveToEeprom(); // Sauvegarder toutes les données en EEPROM

    bool isCalibrationUninitialized(CalibrationManager::Config config); // Vérifier si une configuration est non initialisée
    void setDefaultValues(CalibrationManager::Config config); // Définir des valeurs par défaut pour une configuration
    
    bool ouverture(uint16_t speed);
    bool fermeture(uint16_t speed);

    CalibrationData calibrationData[2]; // 0: ON_FURNITURE, 1: OFF_FURNITURE
    bool eepromActive;

    ETAT etat;
    uint32_t time_etat = 0;

    bool is_calibre;
    uint16_t angle_butee_basse;
    uint16_t angle_butee_haute;

    Motor& moteur;
    Sensors& capteurs;
};

#endif