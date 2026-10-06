#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <Arduino.h>
#include <EEPROM.h>
#include "Messages.h"
#include "../Config/Codes.h"

#include "../Inputs/SensorsManager.h"
#include "../Outputs/Motor.h"
#include "../MachineEtats/StateMachine.h"

#define EEPROM_ADDR_TENVOIS 15

constexpr float RADS_TO_RPM = 60.0f / (2.0f * PI);
constexpr float RAD_TO_TURN = 1.0f / (2.0f * PI);

class SerialManager {
    public:
        SerialManager(SensorsManager& s, StateMachine& ma);
        void init();
        void task();

        inline void printStart() { 
            Serial.println(F("\n==== Pilotage Porte Blum ====\n"));
        }
        inline void printFinInit() { 
            Serial.println(F("\n== Initialisation terminée ==\n"));
        }

        //void printMesures();
        void readSerial();

        uint32_t getMesurePeriode();

        bool demandeTest = false;                          // Levé par DO TEST, consommé par loop()
        inline void resync() { time_precedent = millis(); } // Resynchronise le timer après le test
    
    private:
        SensorsManager& capteurs;
        StateMachine& machine;

        uint32_t time_precedent = 0;    
        uint16_t periode_echantillonnage_mesures = 25;

        void _SET(String commande);
        void _GET(String commande);
        void _DO(String commande);
        int splitCommande(const String& commande, String items[], int maxItems);

        void loadMesurePeriode();
        void setMesurePeriode(uint16_t);
};


#endif