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

// Constantes des capteurs
constexpr float RAD_PER_TICK = 2.0f * PI / 128.0f;
constexpr float K_VITESSE = 2.0f * PI / 128.0f / 0.1f;
constexpr float CURRENT_COEF = 5.0f / 1024.0f / 0.185f;
constexpr float VOLTAGE_COEF = 25.0f / 1023.0f;


// Variables pour le calcul de la moyenne glissante du courant
const float LIMITE_COURANT = 2.0f;
const unsigned int NB_MOY_COURANT = 5;

// Détections butées
const int PWM_MIN = 50;
constexpr int PWM_RAPIDE = 150;
constexpr int NB_CYCLES_BLOCAGE_RAPIDE = 20; // 10 x 5 ms = 50 ms
constexpr int NB_CYCLES_BLOCAGE_LENT = 50;
const int TICKS_MIN = 2;
const float I_BLOCAGE = 1.5f;
const float ANGLE_MAX = 180.0f;
const float ANGLE_MIN = -140.0f;

class Sensors {
    public:
        float time_mesures = 0.0f;
        float tension = 0.0f;
        float courant_moyen = 0.0f;
        float angle_moteur = 0.0f;
        float vitesse_moteur = 0.0f;
        float angle_porte = 0.0f;
        float potentiometre = 0.0f;
        bool sur_meuble = false;

        float consigne = 0.0f;

        bool limite_haute = false;
        bool limite_basse = false;
        bool blocage_detecte = false;
        bool limite_courant_atteinte = false;

        void init();
        void mesures(int pwm);
        void checkSecurites(int pwm);
        bool isBlocage(bool reset = false);
        
        void getTension();
        void getCourant();
        void getMeuble();
        void getCodeurPorte();
        void getCodeurMoteur();
        void getPotentiometre();

        void setConsigne(int consigne);
        void setLimits(float limite_basse, float limite_haute);
        void resetLimits();

    private:
        int courant_offset = 0;
        unsigned int courant_idx = 0;
        float courant_tab[NB_MOY_COURANT];
        float courantSomme = 0.0f;

        uint8_t compteur_blocage = 0;
        int32_t codeur_Delta_Pos = 0;

        int32_t encoderGetTicks();
        void encoderResetTicks();
        float addCourant(float current);

        float angle_bas_max = ANGLE_MIN;
        float angle_haut_max = ANGLE_MAX;
};

#endif