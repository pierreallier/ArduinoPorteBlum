#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <Arduino.h>
#include "Motor.h"
#include "Sensors.h"
#include "Pilotage.h"



class StateMachine {
    public:
        enum class ETAT : byte {
            INIT=0,
            REPOS=1,
            OUVERTURE=2,
            FERMETURE=3,
            PILOTE=4,
            CALIBRATION=5,
            DEBRAYAGE=6,
            ERREUR=7
        };


        ETAT etat;
        uint32_t time_etat;

        StateMachine(Motor& m, Sensors& c);
        void init();
        void changerEtat(StateMachine::ETAT etat_demande);
        void exec();
    
    private:
        Motor& moteur;
        Sensors& capteurs;
        Pilotage pilote;
    
};

#endif