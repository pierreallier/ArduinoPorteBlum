#ifndef COMSERIE_H
#define COMSERIE_H

#include <Arduino.h>
#include "Sensors.h"
#include "Motor.h"

class ComSerie {
    public:
        ComSerie(Motor& m, Sensors& c);
        void init();
        void task();
        void printMesures();
        void getCommandes();
        void sendMesures();
        void sendEtat(int etat);
        void sendError(String message);
    
    private:
        uint32_t time_precedent = 0;
        Motor& moteur;
        Sensors& capteurs;

        void _SET(String commande);
        void _GET(String commande);
        void _DO(String commande);
};


#endif