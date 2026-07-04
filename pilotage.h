#ifndef PILOTAGE_H
#define PILOTAGE_H

#include "PID.h"
#include "Motor.h"

class Pilotage {
    public:

        enum class MODE : byte {
            PWM,
            POSITION,
            VITESSE,
            DOUBLE // Asservissement en vitesse et position
        };

    MODE mode;

    float consignePosition;
    float consigneVitesse;
    int16_t consignePWM;

    PID pidPosition;
    PID pidVitesse;

    Pilotage();
    void setMode(Pilotage::MODE mode);
    void setConsignePosition(float angle);
    void setConsigneVitesse(float vitesse);
    void setPWM(int16_t pwm);
    void update(Motor& moteur);
    void reset();

};

#endif