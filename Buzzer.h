#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>
#include "Constantes.h"

class Buzzer {
public:
    void init();
    void task();

    void stop();
    void play(uint8_t nb, uint16_t duree, uint16_t pauseMs);

    void enable();
    void disable();
    bool isEnabled() const;

    void bip(uint16_t duree = 100);
    void sequenceInit();
    void sequenceErreur();  

private:

    bool enabled = false;
    bool actif = false;
    bool etatSortie = false;

    uint8_t nbBips = 0;
    uint8_t bipCourant = 0;

    uint16_t dureeBip = 100;
    uint16_t pause = 100;

    unsigned long t0 = 0;
};

#endif