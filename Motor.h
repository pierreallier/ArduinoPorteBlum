#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

#define PWM_REV_PIN  4 // Pin de commande PWM pour la rotation inverse du moteur
#define PWM_FOR_PIN  5 // Pin de commande PWM pour la rotation avant du moteur
#define STBY_PIN 8  // Driver du moteur : Standby input

enum class MotorDir :bool {
    OUVERTURE = true,
    FERMETURE = false
};

struct Motor {
    int pwm; // valeur PWM pour la vitesse du moteur (0-255)
    int32_t codeur_avant_debrayage = 0; // valeur du codeur avant le débrayage
    float courant_avant_debrayage = 0.0; // valeur du courant avant le débrayage
    bool enabled; // état du moteur (activé ou désactivé)
    MotorDir direction = MotorDir::OUVERTURE; // direction du moteur (OUVERTURE ou FERMETURE)
};
extern Motor moteur;


void moteur_Init(); // Initialisation du moteur et du driver
void moteur_SetDirection(MotorDir dir); // Définit la direction du moteur
void moteur_Update(); // Met à jour l'état du moteur en fonction de la consigne et des capteurs
void moteur_SetSpeed(int speed); // Définit la vitesse du moteur (0-255)
void moteur_SetSpeedDir(int speed); // Définit la vitesse et la direction du moteur (-255 à 255)
void moteur_Enable(); // Active le moteur
void moteur_Disable(); // Désactive le moteur
void moteur_Start(); // Démarre le moteur
void moteur_Debrayage(); // Arrête le moteur en appliquant une petite vitesse inverse pour le débrayer
void moteur_Stop(); // Arrête le moteur
void moteur_Task(); // Met à jour l'état du moteur en fonction de la consigne et des capteurs

#endif