#ifndef SENSORSMANAGER_H
#define SENSORSMANAGER_H

#include <Arduino.h>
#include "Sensor.h"
#include "AnalogStateDetector.h"
#include "MT6701.h"
#include "BN0055.h"
#include "CurrentMoySensor.h"
#include "ABEncoder.h"
#include "ExpFilter.h"

#include "../Config/Constantes.h"
#include "../Config/Types.h"

// Coefficient de conversion pour les capteurs analogiques (tension, courant, potentiomètre)
const float COEFF_TENSION = 29.8f / ADC_MAX * 100; // Convertit la valeur brute du capteur de tension en V*100 (29.8V max)
const float COEFF_PWM = 1 / 255.0f; // Convertit le PWM en pourcentage de tension appliquée au moteur
const float COEFF_COURANT = 5.0f / ADC_MAX / 0.400f; // Convertit la valeur brute du capteur de courant en A (0.400V/A)
const float COEFF_POTENTIOMETRE = 0.5f * 100; // Convertit la valeur brute du potentiomètre en pourcentage (0 à 100)
const float COEFF_CODEUR_PORTE = 360.0f / ADC_MAX * 100; // Convertit la sortie analogique du codeur en degrés*100
const float COEFF_CODEUR_MOTEUR = 2.0f * PI / 128.0f * 100; // Convertit les ticks du codeur en rad*100


// Détections butées
const int PWM_MIN = 50;
constexpr int PWM_RAPIDE = 150;
constexpr int NB_CYCLES_BLOCAGE_RAPIDE = 50; // 50 x 1 ms = 50 ms
constexpr int NB_CYCLES_BLOCAGE_LENT = 250; // 250 x 1 ms = 250 ms
const int TICKS_MIN = 2;

class SensorsManager {
    public:

        void init();
        void mesures();

        // Sécurités 
        void updateSecurities(int16_t pwm);
        bool isBlocage();
        bool isLimiteCourant();
        bool isLimiteAngle();
        inline bool isLimiteHaute() {return limite_haute;}
        inline bool isLimiteBasse() {return limite_basse;}
        bool resetSecurities();
        inline bool onMeuble() const {return false;} // pour l'instant tjr hors meuble _meuble.etat()
        inline bool meubleChanged() {return _meuble.changed();}
        inline bool hasPower() { return _tension.get() > 500;}

        inline Mesures getMesures() const { return _mesures;}

        // Retourne les mesures converties en unités physiques (float)
        inline float getTension() const { return _tension.get() / 100.0f;}
        inline float getTensionMoteur() const { return _mesures.tension / 100.0f;}
        inline float getCourant() const { return _mesures.courant_moyen / 100.0f;}
        inline float getLastCourant() const { return _courant.get();}
        inline float getAnglePorte() const { return _mesures.angle_porte / 100.0f;}
        inline float getAngleMoteur() const { return _mesures.angle_moteur / 100.0f;}
        inline float getVitesseMoteur() const { return _mesures.vitesse_moteur / 100.0f;}
        inline float getPWM() const { return _pwm;}
        inline float getConsigne() const { return _mesures.consigne / 100.0f;}
        inline float getPotentiometre() const { return _potentiometre.get() / 100.0f;}
        inline float getLimiteBasse() const { return calibrationData.lowLimit;}
        inline float getLimiteHaute() const { return calibrationData.highLimit;}
        
        // Plusieurs surcharges pour définir la consigne en fonction du type de donnée (int, float, etc.)
        void setConsigne(uint16_t consigne) { _mesures.consigne = static_cast<int32_t>(consigne) * 100; }
        void setConsigne(int16_t consigne) { _mesures.consigne = static_cast<int32_t>(consigne) * 100; }
        void setConsigne(int32_t consigne) { _mesures.consigne = consigne * 100; }
        void setConsigne(float consigne) { _mesures.consigne = static_cast<int32_t>(consigne * 100.0f); }
        
        inline float getLimitCourant() const { return _courant.getLimit();}
        bool setLimitCourant(float limite);
        void setCalibration(CalibrationData c);
        CalibrationData& getCalibration() {return calibrationData;}

        inline BN0055& bno()          { return _bno; }
        bool checkBNO();
        inline void setBNO(bool etat) { _bno.setEnabled(etat);}

        inline MT6701& codeurPorte()  { return _codeurPorte; }
        inline bool checkCodeurPorte() { return _codeurPorte.checkPresence();}

        // Ensemble des capteurs gérés par le SensorsManager
        AnalogStateDetector _meuble = AnalogStateDetector(DETECTEUR_MEUBLE, 100, 50, 500); // Binaire 
        Sensor<uint16_t> _tension = Sensor<uint16_t>(MOTOR_VOLTAGE_PIN, COEFF_TENSION); // en V*100
        Sensor<float> _potentiometre = Sensor<float>(POTENTIOMETRE_PIN, COEFF_POTENTIOMETRE, 500); // BRUT
        CurrentMoySensor _courant = CurrentMoySensor(MOTOR_CURRENT_PIN, COEFF_COURANT, 2.0f); // en A, limite 2.0A
        MT6701 _codeurPorte = MT6701(CODEUR_PORTE_PIN, COEFF_CODEUR_PORTE, MT6701_ADDRESS); // en °*100
        BN0055 _bno = BN0055(BN0055_ADDRESS);
        ABEncoder _codeurMoteur = ABEncoder(COEFF_CODEUR_MOTEUR); // en rad*100 et rad/s*100

    private:
        Mesures _mesures;
        // Filtrage du courant moyen en A : tau stable/dynamique (s), seuil (A), ratio.
        ExpFilter _courantFilter{0.150f, 0.010f, 0.100f, 0.25f};

        bool limite_haute = false;
        bool limite_basse = false;
        bool blocage_detecte = false;
        bool limite_courant_atteinte = false;
        uint8_t compteur_blocage = 0;

        int16_t _pwm = 0; // Valeur PWM actuelle (+/-255)

        CalibrationData calibrationData;

};

#endif
