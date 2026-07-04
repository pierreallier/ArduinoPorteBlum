#include "Pilotage.h"

Pilotage pilotage;

Pilotage::Pilotage() {
    mode = Pilotage::MODE::PWM;
}

void Pilotage::setMode(Pilotage::MODE mode) {
    mode = mode;
}

void Pilotage::setConsignePosition(float angle) {
    consignePosition = angle;
}

void Pilotage::setConsigneVitesse(float vitesse) {
    consigneVitesse = vitesse;
}

void Pilotage::setPWM(int16_t pwm) {
    consignePWM = pwm;
}

void Pilotage::update(Motor& moteur) {
    switch(mode)
    {
        case Pilotage::MODE::PWM:
            moteur.setSpeedDir(consignePWM);
            break;

        case Pilotage::MODE::POSITION:

            break;

        case Pilotage::MODE::VITESSE:

            break;
        
        case Pilotage::MODE::DOUBLE:

            break;
    }
}

void Pilotage::reset() {

}