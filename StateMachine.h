#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <Arduino.h>
#include "Motor.h"
#include "Sensors.h"

enum class EtatMachine : byte {
    INIT=0,
    REPOS=1,
    OUVERTURE=2,
    FERMETURE=3,
    PILOTE=4,
    CALIBRATION=5,
    DEBRAYAGE=6,
    ERREUR=7
};

class StateMachine {
    public:
        EtatMachine etat;
        uint32_t time_etat;

        StateMachine(Moteur m, Capteurs c);
        void init();
        void changerEtat(EtatMachine etat_demande);
        void exec();
    
    private:
        Moteur moteur;
        Capteurs capteurs;
    
};

#endif