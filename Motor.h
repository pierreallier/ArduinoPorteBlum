#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

#define PWM_REV_PIN  6 // Pin de commande PWM pour la rotation inverse du moteur
#define PWM_FOR_PIN  7 // Pin de commande PWM pour la rotation avant du moteur
#define STBY_PIN 8  // Driver du moteur : Standby input

enum class MotorDir :bool {
    OUVERTURE = true,
    FERMETURE = false
};

class Moteur {
public:
    Moteur();
    void init(); // Initialisation du moteur et du driver
    void setDirection(MotorDir dir); // Définit la direction du moteur
    void update(); // Met à jour l'état du moteur en fonction de la consigne et des capteurs
    void setSpeed(int speed); // Définit la vitesse du moteur (0-255)
    void setSpeedDir(int speed); // Définit la vitesse et la direction du moteur (-255 à 255)
    void enable(); // Active le moteur
    void disable(); // Désactive le moteur
    void start(); // Démarre le moteur
    void debrayage(); // Arrête le moteur en appliquant une petite vitesse inverse pour le débrayer
    void stop(); // Arrête le moteur

    // Accesseurs si besoin
    bool isEnabled() const { return enabled; }
    MotorDir getDirection() const { return direction; }
    int getPWM() const { return pwm; }

    int32_t codeur_avant_debrayage = 0; // valeur du codeur avant le débrayage
    float courant_avant_debrayage = 0.0; // valeur du courant avant le débrayage
    
private:
    int pwm = 0; // valeur PWM pour la vitesse du moteur (0-255)
    bool enabled = false; // état du moteur (activé ou désactivé)
    MotorDir direction = MotorDir::OUVERTURE; // direction du moteur (OUVERTURE ou FERMETURE)
};

#endif