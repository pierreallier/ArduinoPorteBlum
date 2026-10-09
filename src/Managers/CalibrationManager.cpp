#include "CalibrationManager.h"
#include <EEPROM.h>
#include "../Serial/Messages.h"
#include "../Config/Constantes.h"

CalibrationManager::CalibrationManager(SensorsManager& c, Motor& m)
  : capteurs(c), moteur(m) {
}

void CalibrationManager::init() {
  
  pinMode(LED_CALIBRATION_PIN, OUTPUT);
  digitalWrite(LED_CALIBRATION_PIN, LOW);

  etat = ETAT_CALIBRATION::NONE;
  loadFromEeprom(false);  // Initialiser les données à partir de la RAM
  updateCapteursLimits(getConfig());

  time_etat = millis();
  time_capteur = time_etat;
  time_led = time_etat;
}

void CalibrationManager::task() {
  const uint32_t now = millis();
  // Clignotement LED toutes les 1s si calibration requise
  if (isNotCalibrated() && (etat == ETAT_CALIBRATION::NONE) && (now - time_led >= 1000)) {
    time_led += 1000;
    ledState = !ledState;
    digitalWrite(LED_CALIBRATION_PIN, ledState);
  }
}


void CalibrationManager::setEepromActive(bool active) {  // Activer/désactiver l'utilisation de l'EEPROM
  if (eepromActive == active) return;                    // Pas de changement
  eepromActive = active;
  uint8_t flag = eepromActive ? 1 : 0;
  EEPROM.update(EEPROM_ADDR_ACTIVE_FLAG, flag);
  if (eepromActive) {
    loadFromEeprom();  // Charger les données depuis l'EEPROM
  }
}

void CalibrationManager::clearEeprom() {  // Efface les valeurs stockées dans l'EEPROM
  setDefaultValues(ON_FURNITURE);
  EEPROM.put(EEPROM_ADDR_ON_FURNITURE, calibrationData[ON_FURNITURE]);
  setDefaultValues(OFF_FURNITURE);
  EEPROM.put(EEPROM_ADDR_OFF_FURNITURE, calibrationData[OFF_FURNITURE]);
  updateCapteursLimits(getConfig());
}

void CalibrationManager::loadFromEeprom(bool forced) {  // Charger toutes les données depuis l'EEPROM (si faux, ne charge pas)
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
  } else if (!forced || storedActiveFlag == 0) {
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


void CalibrationManager::setCalibration(CalibrationManager::Config config, uint16_t highLimit, uint16_t lowLimit, uint16_t offset) {  // Définir les valeurs d'étalonnage pour une configuration
  calibrationData[config].highLimit = highLimit;
  calibrationData[config].lowLimit = lowLimit;
  calibrationData[config].offset = offset;
  updateCapteursLimits(config);
  if (eepromActive) {
    if (config == ON_FURNITURE) {
      EEPROM.put(EEPROM_ADDR_ON_FURNITURE, calibrationData[ON_FURNITURE]);
    } else if (config == OFF_FURNITURE) {
      EEPROM.put(EEPROM_ADDR_OFF_FURNITURE, calibrationData[OFF_FURNITURE]);
    }
  }
}


bool CalibrationManager::getCalibration(CalibrationManager::Config config, int& highLimit, int& lowLimit, int& offset) {
  if (isCalibrationUninitialized(config)) {  // Valeurs non initialisées
    return false;
  }
  highLimit = calibrationData[config].highLimit;
  lowLimit = calibrationData[config].lowLimit;
  offset = calibrationData[config].offset;
  return true;
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


void CalibrationManager::changerEtat(ETAT_CALIBRATION nouvelle_etape) {
  time_etat = millis();
  etat = nouvelle_etape;
  sendEtat(MSG::ETAT_CALIBRATION,(uint8_t)etat);
}

bool CalibrationManager::exec() {
  /* Gestion de l'état calibration */
  // Vérification des sécurités (car non gérées au niveau supérieur)
  bool blocage = capteurs.isBlocage();
  // if (false && !blocage && capteurs.isLimiteCourant()) {
  //     if (etat == ETAT_CALIBRATION::OUVERTURE || etat == ETAT_CALIBRATION::BUTEE_HAUTE) {
  //         sendError("Courant trop important en ouverture, augmenter la raideur du ressort");
  //     } else if (etat == ETAT_CALIBRATION::BUTEE_BASSE) {
  //         sendError("Courant trop important en fermeture, diminuer la raideur du ressort");
  //     }
  //     changerEtat(ETAT_CALIBRATION::ERREUR);
  //     moteur.stop();
  // }
  // Execution des états
  switch (etat) {
    case ETAT_CALIBRATION::DEBUT:
      {
        sendInfo(MSG::INFO_CALIBRATION_DEBUT);
        ledState = HIGH;
        digitalWrite(LED_CALIBRATION_PIN, HIGH);
        angle_butee_basse = 0;
        angle_butee_haute = 0;
        moteur.enable();
        changerEtat(ETAT_CALIBRATION::OUVERTURE);
        moteur.ouvrir(PWM_CALIBRATION);
        break;
      }
    case ETAT_CALIBRATION::OUVERTURE:
      {
        if (blocage) {
          moteur.stop();
          changerEtat(ETAT_CALIBRATION::ATTENTE_HAUT);
        }
        break;
      }
    case ETAT_CALIBRATION::ATTENTE_HAUT:
      {
        if (millis() - time_etat >= 1000) {
          changerEtat(ETAT_CALIBRATION::BUTEE_BASSE);
          moteur.fermer(PWM_CALIBRATION);
        }
        break;
      }
    case ETAT_CALIBRATION::BUTEE_BASSE:
      {
        if (blocage) {
          moteur.stop();
          angle_butee_basse = analogRead(CODEUR_PORTE);
          changerEtat(ETAT_CALIBRATION::ATTENTE_BAS);
        }
        break;
      }
    case ETAT_CALIBRATION::ATTENTE_BAS:
      {
        if (millis() - time_etat >= 1000) {
          changerEtat(ETAT_CALIBRATION::BUTEE_HAUTE);
          moteur.ouvrir(PWM_CALIBRATION);
        }
        break;
      }
    case ETAT_CALIBRATION::BUTEE_HAUTE:
      {
        if (blocage) {
          moteur.stop();
          angle_butee_haute = analogRead(CODEUR_PORTE);
          changerEtat(ETAT_CALIBRATION::ENREGISTREMENT);
        }
        break;
      }
    case ETAT_CALIBRATION::ENREGISTREMENT:
      {
        if (millis() - time_etat >= 1000) {
          uint16_t course = angle_butee_haute <= angle_butee_basse ? angle_butee_basse - angle_butee_haute : ADC_MAX - angle_butee_haute + angle_butee_basse;
          if (course < COURSE_MIN_ADC || course > COURSE_MAX_ADC) {
            changerEtat(ETAT_CALIBRATION::ERREUR);
            break;
          }
          sendReponseOK(MSG::INFO_CALIBRATION_ACHEVEE);
          setCalibration(getConfig(), course-2, 0, angle_butee_basse);
          changerEtat(ETAT_CALIBRATION::NONE);
        }
        break;
      }
    case ETAT_CALIBRATION::ERREUR:
      {
        if (millis() - time_etat >= 1000) {
          sendReponseNOK(MSG::INFO_CALIBRATION_ECHEC);
          changerEtat(ETAT_CALIBRATION::NONE);
        }
        break;
      }
    case ETAT_CALIBRATION::NONE:
      {
        ledState = LOW;
        digitalWrite(LED_CALIBRATION_PIN, LOW);
        return true;
      }
    default:
      break;
  }
  return false;
}
