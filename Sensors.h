#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

#define CODEUR_PORTE A4 // Codeur de position absolue de la porte
#define DETECTEUR_MEUBLE A5 // Détecteur de présence du meuble
#define MOTOR_VOLTAGE A0 // Driver du moteur : mesure de la tension
#define MOTOR_CURRENT A1 // Mesure du courant fournit au moteur
#define DRIVER_CURRENT A3 // Driver du moteur : mesure du courant
#define POTENTIOMETRE A2 // Potentiomètre réglage vitesse moteur

// Codeur incrémental du moteur
#define CODEUR_A_PIN 18
#define CODEUR_B_PIN 19

// Variables pour le calcul de la moyenne glissante du courant
const float LIMITE_COURANT = 2;
const unsigned int NB_MOY_COURANT = 5;

// Détections butées
const float COURANT_BUTEE = 0.5;      // A
const float VITESSE_BUTEE = 5;        // impulsions/10 ms
const uint16_t TEMPS_BUTEE = 50;      // ms

class Sensors {
    public:
        float time_mesures;
        float tension;
        float courant_moyen;
        float angle_moteur;
        float vitesse_moteur;
        float angle_porte;
        float potentiometre;
        bool sur_meuble;

        int consigne;

        bool limite_haute = false ;
        bool limite_basse = false ;
        bool limite_courant_atteinte = false;

        Sensors();
        void init();
        void mesures();
        bool checkSecurites();
        bool detectionButees();
        
        void getTension();
        void getCourant();
        void getMeuble();
        void getCodeurPorte();
        void getCodeurMoteur();
        void getPotentiometre();

        void setConsigne(int consigne);

    private:
        int courant_offset = 0;
        unsigned int courant_idx = 0;
        float courant_tab[NB_MOY_COURANT];
        unsigned long time_detection_butee = 0;

        int32_t encoderGetTicks();
        void encoderResetTicks();
        float addCourant(float current);
};

#endif