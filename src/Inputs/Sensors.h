#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "../Config/Constantes.h"
#include "../Config/Types.h"

// Constantes des capteurs
constexpr float RAD_PER_TICK = 2.0f * PI / 128.0f * 100;
constexpr float K_VITESSE = 2.0f * PI / 128.0f / 0.1f * 100;
constexpr float CURRENT_COEF = 5.0f / 1023.0f / 0.400f * 100;
constexpr float VOLTAGE_COEF = 29.8f / 1023.0f * 100;
constexpr float CODEURPORTE_COEF = 360.0f / ADC_MAX * 100;


// Variables pour le calcul de la moyenne glissante du courant
const float LIMITE_COURANT = 2.0f;
const unsigned int NB_MOY_COURANT = 5;

// Détections butées
const int PWM_MIN = 50;
constexpr int PWM_RAPIDE = 150;
constexpr int NB_CYCLES_BLOCAGE_RAPIDE = 10; // 10 x 5 ms = 50 ms
constexpr int NB_CYCLES_BLOCAGE_LENT = 50;
const int TICKS_MIN = 2;

class Sensors {
    public:
        //uint32_t time_mesures = 0;
        //float tension = 0.0f;
        //float courant_moyen = 0.0f;
        //float angle_moteur = 0.0f;
        //float vitesse_moteur = 0.0f;
        //int32_t angle_porte = 0;
        float potentiometre = 0.0f;    
        //float consigne = 0.0f;
        //int pwm = 0;

        bool limite_haute = false;
        bool limite_basse = false;
        bool blocage_detecte = false;
        bool limite_courant_atteinte = false;

        void init();
        void mesures(int pwm);

        void updateSecurities(int pwm);
        bool isBlocage();
        bool isLimiteCourant();
        bool isLimiteAngle();
        bool resetSecurities();

        inline Mesures getMesures() { return _mesures;}
        inline float getTension() { return _mesures.tension / 100.0f;}
        float getCourant() { return _mesures.courant_moyen / 100.0f;}
        float getAnglePorte() { return _mesures.angle_porte / 100.0f;}
        float getAngleMoteur() { return _mesures.angle_moteur / 100.0f;}
        float getVitesseMoteur() { return _mesures.vitesse_moteur / 100.0f;}
        float getPotentiometre() { return potentiometre;}

        void setConsigne(int consigne);
        float getLimitCourant() const { return limite_courant;}
        bool setLimitCourant(float limite);
        
        float getLimiteBasse() const { return calibrationData.lowLimit;}
        float getLimiteHaute() const { return calibrationData.highLimit;}
        void setLimits(CalibrationData c);

    private:
        Mesures _mesures;
        int courant_offset = 0;
        unsigned int courant_idx = 0;
        float courant_tab[NB_MOY_COURANT];
        float courantSomme = 0.0f;

        uint8_t compteur_blocage = 0;
        int32_t codeur_Delta_Pos = 0;

        int32_t encoderGetTicks();
        void encoderResetTicks();
        float addCourant(float current);

        CalibrationData calibrationData;
        float limite_courant;

        void readCodeurPorte();
        void readCourant();
        void readTension();
        void readCodeurMoteur();
        void readPotentiometre();
};

#endif