#pragma once

#define VERSION_CARTE 3
#define VERSION_SOFT 0
#define VERSION_DEV 1

// Moteur et driver
#define PWM_REV_PIN  7 // Pin de commande PWM pour la rotation inverse du moteur
#define PWM_FOR_PIN  6 // Pin de commande PWM pour la rotation avant du moteur
#define STBY_PIN 5  // Driver du moteur : Standby input

// Codeur incrémental du moteur
#define CODEUR_A_PIN 19
#define CODEUR_B_PIN 18

#if VERSION_CARTE == 2
    // Capteurs
    #define MOTOR_VOLTAGE A0 // Driver du moteur : mesure de la tension
    #define DRIVER_CURRENT A1 // Driver du moteur : mesure du courant
    #define MOTOR_CURRENT A2 // Mesure du courant fournit au moteur
    #define DETECTEUR_MEUBLE A4 // Détecteur de présence du meuble
    #define POTENTIOMETRE A5 // Potentiomètre réglage vitesse moteur
    #define CODEUR_PORTE A3 // Codeur de position absolue de la porte

    // Boutons
    #define TEST_BT 33 // Bouton de test
    #define WIRELESS_BT 27 // Bouton sans fil
    #define CALIBRATION_BT 25 // Bouton de calibration
    #define PILOTAGE_BT 41 // Bouton de mise en fonctionnement / arrêt

    // Buzzer
    #define BUZZER_PIN 31

    // Led
    #define LED_MOTOR_PIN 35
    #define LED_CALIBRATION_PIN 29
    #define LED_PILOTAGE_PIN 43
    #define LED_ERROR_PIN 53

#elif VERSION_CARTE == 3
    // Capteurs
    #define MOTOR_VOLTAGE A1 // Driver du moteur : mesure de la tension
    #define DRIVER_CURRENT A0 // Driver du moteur : mesure du courant
    #define MOTOR_CURRENT A2 // Mesure du courant fournit au moteur
    #define DETECTEUR_MEUBLE A10 // Détecteur de présence du meuble
    #define POTENTIOMETRE A12 // Potentiomètre réglage vitesse moteur
    #define CODEUR_PORTE A11 // Codeur de position absolue de la porte

    // Boutons
    #define TEST_BT 37 // Bouton de test
    #define WIRELESS_BT 25 // Bouton sans fil
    #define CALIBRATION_BT 33 // Bouton de calibration
    #define PILOTAGE_BT 41 // Bouton de mise en fonctionnement / arrêt

    // Buzzer
    #define BUZZER_PIN 35

    // Led
    #define LED_MOTOR_PIN 39
    #define LED_CALIBRATION_PIN 31
    #define LED_PILOTAGE_PIN 43
    #define LED_ERROR_PIN 45 // à configurer entre 43,45,47 selon la couleur

#else
    #error "VERSION_CARTE non définie ou incorrecte. Veuillez définir VERSION_CARTE à 2 ou 3."
#endif

// Adresses I2C 
#define MT6701_ADDRESS  0x06
#define BN0055_ADDRESS  0x28

// Arduino - constantes
#define ADC_MAX 1024 // Résolution du CAN

// EEPROM
// Valeurs des calibrations (TODO à stocker sur l'eeprom I2C)
#define EEPROM_ADDR_ACTIVE_FLAG   0   // 1 octet (bool)
#define EEPROM_ADDR_ON_FURNITURE  2   // 6 octets (3x uint16_t) 
#define EEPROM_ADDR_OFF_FURNITURE 8 // 6 octets (3x uint16_t)
// Autres valeurs (a stocker sur l'eeprom de l'arduino)
#define EEPROM_ADDR_TENVOIS 15 // 1 octet (uint8_t)
#define EEPROM_ADDR_BUZZER 16 // 1 octet (bool)
