#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>
#include "../Inputs/SensorsManager.h"
#include "../Config/Constantes.h"

class Motor {
    public:
        enum class DIR :bool {
            OUVERTURE = true,
            FERMETURE = false
        };

        Motor(SensorsManager& c);
        void init(); // Initialisation du moteur et du driver
        void setDirection(Motor::DIR dir); // Définit la direction du moteur
        void update(); // Met à jour l'état du moteur en fonction de la consigne et des capteurs
        void setSpeed(int speed); // Définit la vitesse du moteur (0-255)
        void setSpeedDir(int speed); // Définit la vitesse et la direction du moteur (-255 à 255)
        
        void enable(); // Active le moteur
        void disable(); // Désactive le moteur
        void start(); // Démarre le moteur
        void startDebrayage(); // Arrête le moteur en appliquant une petite vitesse inverse pour le débrayer
        bool isDebraye(); // retourne l'état de debrayage du moteur
        void stop(); // Arrête le moteur
        void ouvrir(uint16_t speed);
        void fermer(uint16_t speed);

        // Accesseurs si besoin
        bool isEnabled() const { return enabled; }
        Motor::DIR getDirection() const { return direction; }
        int getPWM() const { return pwm; }

    private:
        int pwm = 0; // valeur PWM pour la vitesse du moteur (0-255)
        bool enabled = false; // état du moteur (activé ou désactivé)
        DIR direction ; // direction du moteur (OUVERTURE ou FERMETURE)
        SensorsManager& capteurs;

        int32_t codeur_avant_debrayage = 0; // valeur du codeur avant le débrayage
        float courant_avant_debrayage = 0.0; // valeur du courant avant le débrayage
        uint32_t time_avant_debrayage = 0; // début du debrayage
};

#endif