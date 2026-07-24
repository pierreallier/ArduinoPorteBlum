#pragma once

// Capteurs
#define MOTOR_VOLTAGE A0 // Driver du moteur : mesure de la tension
#define DRIVER_CURRENT A1 // Driver du moteur : mesure du courant
#define MOTOR_CURRENT A2 // Mesure du courant fournit au moteur
#define DETECTEUR_MEUBLE A4 // Détecteur de présence du meuble
#define POTENTIOMETRE A5 // Potentiomètre réglage vitesse moteur

// Codeur de la porte
#define CODEUR_PORTE A3 // Codeur de position absolue de la porte
#define AS5600_ADDRESS 0x36  // Adresse I2C par défaut (vérifiez la datasheet)
#define AS5600_STATUS_REG 0x0B // Registre STATUS (à confirmer selon la datasheet)
#define MAGNET_TOO_WEAK   0x04  // Bit 2 : aimant trop faible
#define MAGNET_TOO_STRONG 0x08 // Bit 3 : aimant trop fort
#define MAGNET_DETECTED   0x10  // Bit 4 : aimant détecté (optionnel)

// Codeur incrémental du moteur
#define CODEUR_A_PIN 19
#define CODEUR_B_PIN 18

// Moteur et driver
#define PWM_REV_PIN  7 // Pin de commande PWM pour la rotation inverse du moteur
#define PWM_FOR_PIN  6 // Pin de commande PWM pour la rotation avant du moteur
#define STBY_PIN 5  // Driver du moteur : Standby input

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

