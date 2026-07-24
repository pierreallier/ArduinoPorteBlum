#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <Arduino.h>
#include "SerialTxBuffer.h"
#include "Messages.h"

#include "Sensors.h"
#include "Motor.h"
#include "StateMachine.h"

constexpr float RADS_TO_RPM = 60.0f / (2.0f * PI);
constexpr float RAD_TO_TURN = 1.0f / (2.0f * PI);

class SerialManager {
    public:
        SerialManager(Motor& m, Sensors& c, StateMachine& s);
        void init();
        void task();

        inline void printStart() { 
            Serial.println(F("\n==== Pilotage Porte Blum ====\n"));
            Serial.flush();
            delay(500);
        }

        //void printMesures();
        void readSerial();
        int mesureEnable();
    
    private:
        uint32_t time_precedent = 0;
        Motor& moteur;
        Sensors& capteurs;
        StateMachine& machine;
        int periode_echantillonnage_mesures = 25;

        void _SET(String commande);
        void _GET(String commande);
        void _DO(String commande);
        int splitCommande(const String& commande, String items[], int maxItems);
};


#endif