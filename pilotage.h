#ifndef PILOTAGE_H
#define PILOTAGE_H

#include "pid.h"
#include "motor.h"

enum class ModePilotage {
    PWM,
    POSITION,
    VITESSE,
    DOUBLE // Asservissement en vitesse et position
};

struct Pilotage {
    ModePilotage mode;

    float consignePosition;
    float consigneVitesse;
    int16_t consignePWM;

    PID pidPosition;
    PID pidVitesse;
};

void pilotage_INIT();
void pilotage_SetMode(ModePilotage mode);
void pilotage_SetConsignePosition(float angle);
void pilotage_SetConsigneVitesse(float vitesse);
void pilotage_SetPWM(int16_t pwm);
void pilotage_Update(float dt);
void pilotage_Reset();

#endif