#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <Arduino.h>
#include <EEPROM.h>
#include "SerialTxBuffer.h"
#include "Messages.h"

#include "Sensors.h"
#include "Motor.h"
#include "StateMachine.h"

#define EEPROM_ADDR_TENVOIS 15

constexpr float RADS_TO_RPM = 60.0f / (2.0f * PI);
constexpr float RAD_TO_TURN = 1.0f / (2.0f * PI);

class SerialManager {
    public:
        SerialManager(Motor& m, Sensors& s, StateMachine& ma, CalibrationManager& c);
        void init();
        void task();

        inline void printStart() { 
            Serial.println(F("\n==== Pilotage Porte Blum ====\n"));
            Serial.flush();
            delay(500);
        }

        //void printMesures();
        void readSerial();

        uint16_t getMesurePeriode();
    
    private:
        Motor& moteur;
        Sensors& capteurs;
        StateMachine& machine;
        CalibrationManager& calibration;

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