#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <Arduino.h>

enum class EtatMachine : byte {
    INIT,
    REPOS,
    OUVERTURE,
    FERMETURE,
    PILOTE,
    CALIBRATION,
    DEBRAYAGE,
    ERREUR
};

extern EtatMachine etat;

void machineEtat_Init();
void changerEtat(EtatMachine etat_demande);
void machineEtat(uint32_t tEtat);

#endif