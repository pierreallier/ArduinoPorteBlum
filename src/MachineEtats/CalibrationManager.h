#ifndef CALIBRATIONMANAGER_H
#define CALIBRATIONMANAGER_H

#include <EEPROM.h>
#include "../Config/Types.h"
#include "../Outputs/Motor.h"
#include "../Inputs/Sensors.h"
#include "../Inputs/AnalogStateDetector.h"

// Adresses EEPROM
#define EEPROM_ADDR_ACTIVE_FLAG   0   // 1 octet (bool)
#define EEPROM_ADDR_ON_FURNITURE  2   // 6 octets (3x uint16_t) 
#define EEPROM_ADDR_OFF_FURNITURE 8 // 6 octets (3x uint16_t)

// Valeur pour indiquer qu'une valeur n'est pas initialisée
#define UNINITIALIZED_VALUE 0xFFFF

constexpr uint16_t PWM_CALIBRATION = 250; // Vitesse moteur pour la calibration
constexpr uint16_t COURSE_MAX_ADC = (359UL * ADC_MAX) / 360; // Course maximale en ADC (359°) pour éviter le dépassement de 360°
constexpr uint16_t COURSE_MIN_ADC = (300UL * ADC_MAX) / 360; // Course minimale en ADC pour valider la calibration

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
        OUVERTURE,
        ATTENTE_HAUT,
        BUTEE_BASSE,
        ATTENTE_BAS,
        BUTEE_HAUTE,
        ENREGISTREMENT,
        ERREUR,
        NONE,
        NB_ETATS
    };

    static const char* const ETAT_NAMES[static_cast<size_t>(ETAT::NB_ETATS)]; // Tableau des noms (même ordre que l'enum ETAT)

    
    CalibrationManager(Motor& m, Sensors& c); // Constructeur

    void init(); // Initialise les données
    void task(); // Execute les taches périodiques
    
    void setEepromActive(bool active); // Activer/désactiver l'utilisation de l'EEPROM
    void clearEeprom(); // Efface les données sauvegardé en EEPROM
    void setCalibration(CalibrationManager::Config config, uint16_t highLimit, uint16_t lowLimit, uint16_t offset); // Définir les valeurs d'étalonnage pour une configuration
    bool getCalibration(CalibrationManager::Config config, int& highLimit, int& lowLimit, int& offset); // Obtenir les valeurs d'étalonnage pour une configuration. Retourne true si les valeurs sont valides, false sinon

    bool isNotCalibrated() { return isCalibrationUninitialized(getConfig());}
    bool isCalibrated() { return !isNotCalibrated();} // Vérifie que le système est calibré
    bool isEepromActive() const {  return eepromActive;} // Vérifier si l'EEPROM est active

    Config getConfig() {
        return meuble.etat() ? CalibrationManager::Config::ON_FURNITURE : CalibrationManager::Config::OFF_FURNITURE;
    }
    bool getEtat() const {
        return meuble.etat();
    }
    bool hasChanged() {
        if (meuble.changed()) {
            updateCapteurs();
            return true;
        }
        return false;
    }

    String getCalibrationString();
    
    void updateCapteurs(CalibrationManager::Config config) { capteurs.setLimits(calibrationData[config]);}
    void updateCapteurs() { capteurs.setLimits(calibrationData[getConfig()]);}

    // Gestion machine à état
    void changerEtat(CalibrationManager::ETAT nouvelle_etape); // Changer d'état 
    bool exec(); // Execution de la machine à états

    uint16_t getCourse() { return calibrationData[getConfig()].highLimit;}
    uint16_t getOffset() { return calibrationData[getConfig()].offset;}

private:
    // Gestion EEPROM
    void loadFromEeprom(bool forced = true); // Charger toutes les données depuis l'EEPROM
    void saveToEeprom(); // Sauvegarder toutes les données en EEPROM

    bool isCalibrationUninitialized(CalibrationManager::Config config); // Vérifier si une configuration est non initialisée
    void setDefaultValues(CalibrationManager::Config config); // Définir des valeurs par défaut pour une configuration
    
    bool ouverture(uint16_t speed);
    bool fermeture(uint16_t speed);

    // Données de calibrations
    bool eepromActive;
    CalibrationData calibrationData[2]; // 0: ON_FURNITURE, 1: OFF_FURNITURE

    Motor& moteur;
    Sensors& capteurs;
    AnalogStateDetector meuble = AnalogStateDetector(DETECTEUR_MEUBLE, 100, 50, 500);

    bool ledState = false;

    // Variables pour la machine à états
    ETAT etat;
    uint32_t time_etat = 0;
    uint32_t time_capteur = 0;
    uint32_t time_led = 0;
    uint16_t angle_butee_basse;
    uint16_t angle_butee_haute;
};

#endif