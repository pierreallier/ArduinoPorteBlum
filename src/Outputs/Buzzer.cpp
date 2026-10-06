#include "Buzzer.h"


void Buzzer::init() {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    enabled = true;
}

void Buzzer::enable() {
    enabled = true;
}

void Buzzer::disable() {
    enabled = false;

    // Arrêt immédiat
    actif = false;
    etatSortie = false;

    nbBips = 0;
    bipCourant = 0;

    digitalWrite(BUZZER_PIN, LOW);
}

bool Buzzer::isEnabled() const {
    return enabled;
}

void Buzzer::task() {
    if (!actif && !enabled)
        return;

    unsigned long maintenant = millis();

    if (etatSortie) {
        // Bip en cours
        if (maintenant - t0 >= dureeBip) {

            digitalWrite(BUZZER_PIN, LOW);
            etatSortie = false;
            t0 = maintenant;

            bipCourant++;

            if (bipCourant >= nbBips) {
                actif = false;
            }
        }
    }
    else {
        // Pause entre deux bips
        if (bipCourant < nbBips &&
            maintenant - t0 >= pause) {

            digitalWrite(BUZZER_PIN, HIGH);
            etatSortie = true;
            t0 = maintenant;
        }
    }
}

void Buzzer::stop() {

    actif = false;
    etatSortie = false;

    nbBips = 0;
    bipCourant = 0;

    digitalWrite(BUZZER_PIN, LOW);
}

void Buzzer::play(uint8_t nb, uint16_t duree, uint16_t pauseMs) {
    if (!enabled)
        return;

    stop();                 // Annule la séquence précédente

    nbBips = nb;
    dureeBip = duree;
    pause = pauseMs;

    bipCourant = 0;
    actif = true;
    etatSortie = true;

    digitalWrite(BUZZER_PIN, HIGH);
    t0 = millis();
}

void Buzzer::sequenceErreur() {
    play(3,50,50);
}

void Buzzer::sequenceInit() {
    play(1,10,10);
}

void Buzzer::bip(uint16_t duree) {
    play(1,duree,duree);
}