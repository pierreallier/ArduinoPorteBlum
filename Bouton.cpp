#include "Boutons.h"

volatile unsigned prev_time_bt = 0; // Pour éviter l'effet bouncing du bouton
volatile bool demandeBoutonTest = false;
volatile bool demandeBoutonSansFil = false;

void boutons_Init() {
    pinMode(TEST_BT, INPUT_PULLUP);
    pinMode(WIRELESS_BT, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(TEST_BT), toogleBt, FALLING);
    attachInterrupt(digitalPinToInterrupt(WIRELESS_BT), wirelessBt, FALLING);
}

void toogleBt() {
    uint32_t maintenant = millis();
    if (maintenant - prev_time_bt >= 50) {
        prev_time_bt = maintenant;
        demandeBoutonTest = true;
    }
}

void wirelessBt() {
    uint32_t maintenant = millis();
    if (maintenant - prev_time_bt >= 50) {
        prev_time_bt = maintenant;
        demandeBoutonSansFil = true;
    }
}

bool get_BoutonTest() {
    noInterrupts();
    bool demande = demandeBoutonTest;
    demandeBoutonTest = false;
    interrupts();
    return demande;
}

bool get_BoutonSansFil() {
    noInterrupts();
    bool demande = demandeBoutonSansFil;
    demandeBoutonSansFil = false;
    interrupts();
    return demande;
}