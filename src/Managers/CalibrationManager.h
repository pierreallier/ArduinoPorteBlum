#ifndef CALIBRATIONMANAGER_H
#define CALIBRATIONMANAGER_H

#include "../Config/Types.h"
#include "../Config/Etats.h"
#include "../Outputs/Motor.h"
#include "../Inputs/SensorsManager.h"
#include "../Config/Constantes.h"

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

    CalibrationManager(SensorsManager& c, Motor& m); // Constructeur

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
        return capteurs.onMeuble() ? CalibrationManager::Config::ON_FURNITURE : CalibrationManager::Config::OFF_FURNITURE;
    }
    bool getEtat() const {
        return capteurs.onMeuble();
    }
    bool hasChanged() {
        if (capteurs.meubleChanged()) {
            updateCapteursLimits();
            return true;
        }
        return false;
    }
    
    void updateCapteursLimits(CalibrationManager::Config config) { capteurs.setCalibration(calibrationData[config]);}
    void updateCapteursLimits() { capteurs.setCalibration(calibrationData[getConfig()]);}

    // Gestion machine à état
    void changerEtat(ETAT_CALIBRATION nouvelle_etape); // Changer d'état 
    bool exec(); // Execution de la machine à états

    uint16_t getCourse() { return calibrationData[getConfig()].highLimit;}
    uint16_t getOffset() { return calibrationData[getConfig()].offset;}

private:
    // Gestion EEPROM
    void loadFromEeprom(bool forced = true); // Charger toutes les données depuis l'EEPROM
    void saveToEeprom(); // Sauvegarder toutes les données en EEPROM

    bool isCalibrationUninitialized(CalibrationManager::Config config); // Vérifier si une configuration est non initialisée
    void setDefaultValues(CalibrationManager::Config config); // Définir des valeurs par défaut pour une configuration

    // Données de calibrations
    bool eepromActive;
    CalibrationData calibrationData[2]; // 0: ON_FURNITURE, 1: OFF_FURNITURE

    SensorsManager& capteurs;
    Motor& moteur;

    bool ledState = false;

    // Variables pour la machine à états
    ETAT_CALIBRATION etat;
    uint32_t time_etat = 0;
    uint32_t time_capteur = 0;
    uint32_t time_led = 0;
    uint16_t angle_butee_basse;
    uint16_t angle_butee_haute;
};

#endif