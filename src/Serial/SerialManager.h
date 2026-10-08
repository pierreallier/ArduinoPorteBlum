#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <Arduino.h>
#include "Messages.h"
#include "../Config/Codes.h"

#include "../Inputs/SensorsManager.h"
#include "../Outputs/Motor.h"
#include "../Outputs/Buzzer.h"
#include "../Managers/StateMachine.h"

constexpr float RADS_TO_RPM = 60.0f / (2.0f * PI);
constexpr float RAD_TO_TURN = 1.0f / (2.0f * PI);

class SerialManager {
    public:
        SerialManager(SensorsManager& s, StateMachine& ma, Buzzer& b);
        void init();
        void task();

        inline void printStart() { 
            Serial.println(F("\n==== Pilotage Porte Blum ====\n"));
        }
        inline void printFinInit() { 
            Serial.println(F("\n== Initialisation terminée ==\n"));
        }

        void readSerial();

        uint32_t getMesurePeriode();
        void loadMesurePeriode();
        void setMesurePeriode(uint16_t periode, bool save = false);

        bool testRequired();
        inline void resync() { time_precedent = millis(); } // Resynchronise le timer après le test

        
    
    private:
        SensorsManager& capteurs;
        StateMachine& machine;
        Buzzer& buzzer;

        uint32_t time_precedent = 0;    
        uint16_t periode_echantillonnage_mesures = T_ENVOI;
        bool demandeTest = false;

        void _SET(String commande);
        void _GET(String commande);
        void _DO(String commande);
        int splitCommande(const String& commande, String items[], int maxItems);
  
};


#endif