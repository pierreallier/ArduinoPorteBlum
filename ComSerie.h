#ifndef COMSERIE_H
#define COMSERIE_H

#include <Arduino.h>
#include "Sensors.h"
#include "StateMachine.h"
#include "Motor.h"

class ComSerie {
    public:
        ComSerie(StateMachine& s, Motor& m, Sensors& c);
        void init();
        void task();
        void printMesures();
        void getCommandes();
        void sendMesures();
        void sendEtat(StateMachine::ETAT etat);
    
    private:
        uint32_t time_precedent = 0;
        StateMachine& machine;
        Motor& moteur;
        Sensors& capteurs;

        void _SET(String commande);
        void _GET(String commande);
        void _DO(String commande);
};


#endif