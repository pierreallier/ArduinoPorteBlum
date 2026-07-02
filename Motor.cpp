#include "Motor.h"
#include "Sensors.h"

Motor moteur;

void moteur_Init() {
    // Initialisation du moteur et du driver
    pinMode(PWM_REV_PIN, OUTPUT);
    pinMode(PWM_FOR_PIN, OUTPUT);
    pinMode(STBY_PIN, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    moteur_Disable();
    moteur.enabled = false;
    moteur.pwm = 0;
    moteur.direction = MotorDir::OUVERTURE;
}

void moteur_SetDirection(MotorDir dir) {
    // Définit la direction du moteur
    moteur.direction = dir;
}

void moteur_Start() {
    // Démarre le moteur
    moteur_Enable();
}

void moteur_SetSpeed(int speed) {
    // Définit la vitesse du moteur (0 - 255)
    speed = constrain(speed, 0, 255);
    //if (moteur.direction == MotorDir::FERMETURE) {
        //analogWrite(PWM_REV_PIN, speed);
        //analogWrite(PWM_FOR_PIN, 0);
    //} else if (moteur.direction == MotorDir::OUVERTURE) {
        //analogWrite(PWM_FOR_PIN, speed);
        //analogWrite(PWM_REV_PIN, 0);
    //}
    moteur.pwm = speed;
}

void moteur_SetSpeedDir(int speed) {
    speed = constrain(speed, -255, 255);
    if (speed < 0) {
        moteur_SetDirection(MotorDir::FERMETURE);
        //analogWrite(PWM_REV_PIN, -speed);
        //analogWrite(PWM_FOR_PIN, 0);
        moteur.pwm = -speed;
    } else {
        moteur_SetDirection(MotorDir::OUVERTURE);
        //analogWrite(PWM_FOR_PIN, speed);
        //analogWrite(PWM_REV_PIN, 0);
        moteur.pwm = speed;
    }
}

void moteur_Enable() {
    // Active le moteur
    //digitalWrite(STBY_PIN, HIGH); // Enable the motor driver
    //digitalWrite(LED_BUILTIN, HIGH); // Turn on the built-in LED to indicate that the motor is enabled
    moteur.enabled = true;
}

void moteur_Disable() {
    // Désactive le moteur
    //digitalWrite(STBY_PIN, LOW); // Disable the motor driver
    //digitalWrite(PWM_REV_PIN, LOW);
    //digitalWrite(PWM_FOR_PIN, LOW);
    //digitalWrite(LED_BUILTIN, LOW); // Turn off the built-in LED to indicate that the motor is disabled
    moteur.enabled = false;
}

void moteur_Debrayage() {
    // Arrête le moteur
    moteur.codeur_avant_debrayage = mesures.angle_moteur;
    moteur.courant_avant_debrayage = mesures.courant_moyen;
    if (moteur.direction == MotorDir::OUVERTURE) {
        moteur_SetSpeed(-50); // Apply a small reverse speed to stop the motor
    } else if (moteur.direction == MotorDir::FERMETURE) {
        moteur_SetSpeed(50); // Apply a small forward speed to stop the motor
    }
}

void moteur_Stop(){
    // Arrête le moteur
    moteur_SetSpeed(0);
    moteur_Disable();
}

void moteur_Task() {
    // Met à jour l'état du moteur en fonction de la consigne et des capteurs
    if (moteur.enabled == true) {
        digitalWrite(STBY_PIN, HIGH); // Ensure the motor driver is enabled
        digitalWrite(LED_BUILTIN, HIGH); // Turn on the built-in LED to indicate that the motor is enabled
    } else {
        digitalWrite(STBY_PIN, LOW); // Ensure the motor driver is disabled
        digitalWrite(LED_BUILTIN, LOW); // Turn off the built-in LED to indicate that the motor is disabled
    }
    if (moteur.direction == MotorDir::OUVERTURE && !mesures.limite_haute) {
        analogWrite(PWM_FOR_PIN, moteur.pwm);
        analogWrite(PWM_REV_PIN, 0);
    } else if (moteur.direction == MotorDir::FERMETURE && !mesures.limite_basse) {
        analogWrite(PWM_REV_PIN, moteur.pwm);
        analogWrite(PWM_FOR_PIN, 0);
    } else {
        analogWrite(PWM_FOR_PIN, 0);
        analogWrite(PWM_REV_PIN, 0);
    }
}