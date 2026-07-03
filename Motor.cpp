#include "Motor.h"
#include "Sensors.h"

Moteur::Moteur() {
    pwm = 0;
    codeur_avant_debrayage = 0;
    courant_avant_debrayage = 0.0;
    enabled = false;
    direction = MotorDir::OUVERTURE;
}

void Moteur::init() {
    TCCR4B = (TCCR4B & 0b11111000) | 0x01;
    // Initialisation du moteur et du driver
    pinMode(PWM_REV_PIN, OUTPUT);
    pinMode(PWM_FOR_PIN, OUTPUT);
    pinMode(STBY_PIN, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    disable();
}

void Moteur::setDirection(MotorDir dir) {
    // Définit la direction du moteur
    direction = dir;
}

void Moteur::start() {
    // Démarre le moteur
    enable();
}

void Moteur::setSpeed(int speed) {
    // Définit la vitesse du moteur (0 - 255)
    speed = constrain(speed, 0, 255);
    pwm = speed;
}

void Moteur::setSpeedDir(int speed) {
    speed = constrain(speed, -255, 255);
    if (speed < 0) {
        setDirection(MotorDir::FERMETURE);
        pwm = -speed;
    } else {
        setDirection(MotorDir::OUVERTURE);
        pwm = speed;
    }
}

void Moteur::enable() {
    // Active le moteur
    enabled = true;
}

void Moteur::disable() {
    // Désactive le moteur
    enabled = false;
}

void Moteur::debrayage(Capteurs capteurs) {
    // Arrête le moteur en enregistrant quelques mesures
    codeur_avant_debrayage = capteurs.angle_moteur;
    courant_avant_debrayage = capteurs.courant_moyen;
    if (direction == MotorDir::OUVERTURE) {
       setSpeedDir(-150); // Apply a small reverse speed to stop the motor
    } else if (direction == MotorDir::FERMETURE) {
        setSpeedDir(150); // Apply a small forward speed to stop the motor
    }
}

void Moteur::stop(){
    // Arrête le moteur
    setSpeed(0);
    disable();
}

void Moteur::update(Capteurs capteurs) {
    // Met à jour l'état du moteur en fonction de la consigne et des capteurs
    if (enabled == true) {
        digitalWrite(STBY_PIN, HIGH); // Ensure the motor driver is enabled
        digitalWrite(LED_BUILTIN, HIGH); // Indicate motor enabled
    } else {
        digitalWrite(STBY_PIN, LOW); // Ensure the motor driver is disabled
        digitalWrite(LED_BUILTIN, LOW); // Indicate motor disabled
    }
    if (direction == MotorDir::OUVERTURE && !capteurs.limite_haute) {
        analogWrite(PWM_FOR_PIN, pwm);
        analogWrite(PWM_REV_PIN, 0);
    } else if (direction == MotorDir::FERMETURE && !capteurs.limite_basse) {
        analogWrite(PWM_REV_PIN, pwm);
        analogWrite(PWM_FOR_PIN, 0);
    } else {
        analogWrite(PWM_FOR_PIN, 0);
        analogWrite(PWM_REV_PIN, 0);
    }
}