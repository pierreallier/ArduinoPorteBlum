#include "CalibrationManager.h"
#include <EEPROM.h>
#include "../Serial/Messages.h"
#include "../Config/Constantes.h"

CalibrationManager::CalibrationManager(SensorsManager& c, Motor& m, Buzzer& b) 
                : StateMachine<ETAT_CALIBRATION>(c,m,b), eepromActive(false) {
  name = MSG::ETAT_CALIBRATION;
}

void CalibrationManager::init() {
  loadFromEeprom(false);  // Initialiser les données à partir de la RAM
  updateCapteursLimits(getConfig());
  setEtat(ETAT_CALIBRATION::FIN);
}

/* Gestion de l'eeprom */
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

/* Gestion des données de calibration */
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

bool CalibrationManager::hasChanged() {
  if (capteurs.meubleChanged()) {
    updateCapteursLimits();
    return true;
  }
  return false;
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

/* Machine à états */
void CalibrationManager::execStateEntry(ETAT_CALIBRATION etat) {
  // Actions à l'entrée de l'état courant
  switch (etat) {
    case ETAT_CALIBRATION::INIT: {
      sendInfo(MSG::INFO_CALIBRATION_DEBUT);
      angle_butee_basse = 0;
      angle_butee_haute = 0;
      moteur.enable();
      moteur.setSpeed(0);
      capteurs.setConsigne(0);
      break;
    }
    case ETAT_CALIBRATION::OUVERTURE: {
      moteur.ouvrir(PWM_CALIBRATION);
      capteurs.setConsigne(PWM_CALIBRATION);
      break;
    }
    case ETAT_CALIBRATION::BUTEE_BASSE: {
      moteur.fermer(PWM_CALIBRATION);
      capteurs.setConsigne(-PWM_CALIBRATION);
      break;
    }
    case ETAT_CALIBRATION::BUTEE_HAUTE: {
      moteur.ouvrir(PWM_CALIBRATION);
      capteurs.setConsigne(PWM_CALIBRATION);
      break;
    }
    case ETAT_CALIBRATION::ERREUR: {
      moteur.stop();
      capteurs.setConsigne(0);
      LedError.on();
      buzzer.sequenceErreur();
      break;
    }
    case ETAT_CALIBRATION::FIN: {
      moteur.stop();
      capteurs.setConsigne(0);
      break;
    }
    default:
      break;
  }
}

void CalibrationManager::execStateExit(ETAT_CALIBRATION etat) {
  // Actions à la sortie de l'état courant
  switch (etat) {
    case ETAT_CALIBRATION::OUVERTURE:
    case ETAT_CALIBRATION::BUTEE_BASSE:
    case ETAT_CALIBRATION::BUTEE_HAUTE: 
      moteur.stop();
      capteurs.setConsigne(0);
      break;
    default:
      break;
  }
}


void CalibrationManager::execState(ETAT_CALIBRATION etat) {
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
    case ETAT_CALIBRATION::INIT: {
      changerEtat(ETAT_CALIBRATION::OUVERTURE);
      break;
    }
    case ETAT_CALIBRATION::OUVERTURE: {
      if (blocage) changerEtat(ETAT_CALIBRATION::ATTENTE_HAUT);
      break;
    }
    case ETAT_CALIBRATION::ATTENTE_HAUT: {
      if (millis() - getTimeEtat() >= 1000) changerEtat(ETAT_CALIBRATION::BUTEE_BASSE);
      break;
    }
    case ETAT_CALIBRATION::BUTEE_BASSE: {
      if (blocage) {
        angle_butee_basse = analogRead(CODEUR_PORTE_PIN);
        changerEtat(ETAT_CALIBRATION::ATTENTE_BAS);
      }
      break;
    }
    case ETAT_CALIBRATION::ATTENTE_BAS: {
      if (millis() - getTimeEtat() >= 1000) {
        changerEtat(ETAT_CALIBRATION::BUTEE_HAUTE);
      }
      break;
    }
    case ETAT_CALIBRATION::BUTEE_HAUTE:{
      if (blocage) {
        angle_butee_haute = analogRead(CODEUR_PORTE_PIN);
        changerEtat(ETAT_CALIBRATION::ENREGISTREMENT);
      }
      break;
    }
    case ETAT_CALIBRATION::ENREGISTREMENT: {
      if (millis() - getTimeEtat() >= 1000) {
        uint16_t course = angle_butee_haute <= angle_butee_basse ? angle_butee_basse - angle_butee_haute : ADC_MAX - angle_butee_haute + angle_butee_basse;
        if (course < COURSE_MIN_ADC || course > COURSE_MAX_ADC) {
          changerEtat(ETAT_CALIBRATION::ERREUR);
          break;
        }
        sendReponseOK(MSG::INFO_CALIBRATION_ACHEVEE);
        setCalibration(getConfig(), course-2, 0, angle_butee_basse);
        changerEtat(ETAT_CALIBRATION::FIN);
        buzzer.sequenceFinPilotage();
      }
      break;
    }
    case ETAT_CALIBRATION::ERREUR: {
      if (millis() - getTimeEtat() >= 1000) {
        sendReponseNOK(MSG::INFO_CALIBRATION_ECHEC);
        changerEtat(ETAT_CALIBRATION::FIN);
      }
      break;
      }
    case ETAT_CALIBRATION::FIN: {
      stop();
    }
    default:
      break;
  }
}
