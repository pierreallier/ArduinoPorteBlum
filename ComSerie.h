#ifndef COMSERIE_H
#define COMSERIE_H

#include <Arduino.h>
#include "Sensors.h"
#include "Motor.h"
#include "StateMachine.h"
#include "Buzzer.h"

constexpr float RADS_TO_RPM = 60.0f / (2.0f * PI);
constexpr float RAD_TO_TURN = 1.0f / (2.0f * PI);

class ComSerie {
    public:
        ComSerie(Motor& m, Sensors& c, StateMachine& s);
        void init();
        void task();
        void printFinInit();
        void printMesures();
        void readSerial();
        void sendMesures();
        void sendEtat(String etat);
        void sendError(String message);
        void sendMessages();
    
    private:
        uint32_t time_precedent = 0;
        Motor& moteur;
        Sensors& capteurs;
        StateMachine& machine;
        Buzzer buzzer;

        void _SET(String commande);
        void _GET(String commande);
        void _DO(String commande);
};


#endif