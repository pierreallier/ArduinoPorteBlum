#include "Pilotage.h"

Pilotage pilotage;

void pilotage_INIT() {
    pilotage.mode = ModePilotage::PWM;
}

void pilotage_SetMode(ModePilotage mode) {

}

void pilotage_SetConsignePosition(float angle) {

}

void pilotage_SetConsigneVitesse(float vitesse) {

}

void pilotage_SetPWM(int16_t pwm) {

}

void pilotage_Update() {
    switch(pilotage.mode)
    {
        case ModePilotage::PWM:
            //moteur_SetSpeedDir(mesures.potentiometre);
            break;

        case ModePilotage::POSITION:

            break;

        case ModePilotage::VITESSE:

            break;
    }
}

void pilotage_Reset() {

}