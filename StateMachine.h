#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <Arduino.h>

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

enum class ModePilotage {
    PWM,
    POSITION,
    VITESSE
};

extern ModePilotage modePilotage;

extern EtatMachine etat;

void machineEtat_Init();
void changerEtat(EtatMachine etat_demande);
void machineEtat();

#endif