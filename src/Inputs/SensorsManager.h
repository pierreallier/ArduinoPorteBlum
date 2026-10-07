#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "AnalogStateDetector.h"
#include "MT6701.h"
#include "BN0055.h"
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

class SensorsManager {
    public:

        void init();
        void mesures(int pwm);

        void updateSecurities(int pwm);
        bool isBlocage();
        bool isLimiteCourant();
        bool isLimiteAngle();
        inline bool isLimiteHaute() {return limite_haute;}
        inline bool isLimiteBasse() {return limite_basse;}
        bool resetSecurities();
        inline bool onMeuble() const {return _meuble.etat();}
        inline bool meubleChanged() {return _meuble.changed();}
        inline bool hasPower() { return _mesures.tension > 500;}

        inline Mesures getMesures() const { return _mesures;}
        inline float getTension() const { return _mesures.tension / 100.0f;}
        inline float getCourant() const { return _mesures.courant_moyen / 100.0f;}
        inline float getAnglePorte() const { return _mesures.angle_porte / 100.0f;}
        inline float getAngleMoteur() const { return _mesures.angle_moteur / 100.0f;}
        inline float getVitesseMoteur() const { return _mesures.vitesse_moteur / 100.0f;}
        inline float getPotentiometre() const { return potentiometre;}
        inline float getLimiteBasse() const { return calibrationData.lowLimit;}
        inline float getLimiteHaute() const { return calibrationData.highLimit;}
        
        void setConsigne(int consigne);
        inline float getLimitCourant() const { return limite_courant;}
        bool setLimitCourant(float limite);
        void setLimits(CalibrationData c);

        inline BN0055& bno()          { return _bno; }
        inline MT6701& codeurPorte()  { return _codeurPorte; }
        inline bool checkCodeurPorte() { return _codeurPorte.checkPresence();}
        bool checkBNO();
        inline void setBNO(bool etat) { _bno.setEnabled(etat);}

    private:
        AnalogStateDetector _meuble = AnalogStateDetector(DETECTEUR_MEUBLE, 100, 50, 500);
        MT6701 _codeurPorte = MT6701(MT6701_ADDRESS);
        BN0055 _bno = BN0055(BN0055_ADDRESS);
        Mesures _mesures;

        bool limite_haute = false;
        bool limite_basse = false;
        bool blocage_detecte = false;
        bool limite_courant_atteinte = false;

        int courant_offset = 0;
        unsigned int courant_idx = 0;
        float courant_tab[NB_MOY_COURANT];
        float courantSomme = 0.0f;
        float addCourant(float current);

        uint8_t compteur_blocage = 0;
        int32_t codeur_Delta_Pos = 0;
        int32_t encoderGetTicks();
        void encoderResetTicks();

        CalibrationData calibrationData;
        float limite_courant;

        float potentiometre = 0.0f;

        void readCodeurPorte();
        void readCourant();
        void readTension();
        void readCodeurMoteur();
        void readPotentiometre();
};

#endif