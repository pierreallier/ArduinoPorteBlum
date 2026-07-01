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
    bool enabled; // état du moteur (activé ou désactivé)
    MotorDir direction = MotorDir::OUVERTURE; // direction du moteur (OUVERTURE ou FERMETURE)
};
extern Motor moteur;

/*
void Moteur_Init(); // Initialisation du moteur et du driver
void Moteur_SetDirection(MotorDir dir); // Définit la direction du moteur
void Moteur_Update(); // Met à jour l'état du moteur en fonction de la consigne et des capteurs
void Moteur_SetSpeed(int speed); // Définit la vitesse du moteur (0-255)
void Moteur_Enable(); // Active le moteur
void Moteur_Disable(); // Désactive le moteur
void Moteur_Start(); // Démarre le moteur
void Moteur_Stop(); // Arrête le moteur
*/

#endif