#ifndef COMSERIE_H
#define COMSERIE_H

#include <Arduino.h>
#include "Sensors.h"
#include "Motor.h"
#include "StateMachine.h"

class ComSerie {
    public:
        ComSerie(Motor& m, Sensors& c, StateMachine& s);
        void init();
        void task();
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

        void _SET(String commande);
        void _GET(String commande);
        void _DO(String commande);
};


#endif