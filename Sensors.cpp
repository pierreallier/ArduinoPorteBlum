#include "Sensors.h"

volatile int32_t ticks_codeur=0;
void isr_Codeur() {
    // Gestion interruption du codeur
    ticks_codeur += (PIND & _BV(PD2)) ? -1 : +1;
}

Sensors::Sensors() {
    // Initialisation du tableau du courant
    courant_moyen = 0.0f;
    courant_idx = 0;
    courantSomme = 0.0f;
    for (int i=0; i < NB_MOY_COURANT; i++) {
        courant_tab[i] = 0;
    }
}

void Sensors::init() {
    pinMode(CODEUR_PORTE, INPUT);
    pinMode(DETECTEUR_MEUBLE, INPUT);
    pinMode(MOTOR_VOLTAGE, INPUT);
    pinMode(MOTOR_CURRENT, INPUT);
    pinMode(DRIVER_CURRENT, INPUT);
    pinMode(POTENTIOMETRE, INPUT);

    // Initialisation du tableau du courant
    courant_moyen = 0.0f;
    courant_idx = 0;
    courantSomme = 0.0f;
    for (int i=0; i < NB_MOY_COURANT; i++) {
        courant_tab[i] = 0;
    }

    // Calcul offset courant
    uint32_t somme = 0;
    for (int i = 0; i < 500; i++) {
        somme += analogRead(MOTOR_CURRENT);
    }
    courant_offset = somme / 500;

    Serial.println(courant_offset);

    // Initialisation du codeur du moteur
    pinMode(CODEUR_A_PIN, INPUT_PULLUP);
    pinMode(CODEUR_B_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(CODEUR_A_PIN), isr_Codeur, RISING);

    time_mesures = millis();
}

void Sensors::checkSecurites(int pwm) {
    // Vérifie les conditions de sécurité pour le fonctionnement du système : 
    //  - Limites extrémales de la porte
    //  - Limite de courant du moteur
    //  - Vérifie les blocages

    // Détection des limites extrémales de la porte
    getCodeurPorte();
    limite_haute = (angle_porte >= ANGLE_MAX);
    limite_basse = (angle_porte <= ANGLE_MIN);

    // Limite de courant
    getCourant();
    limite_courant_atteinte = (courant_moyen >= LIMITE_COURANT);

    if (!(limite_haute || limite_basse || limite_courant_atteinte)) {
        // Détection blocage
        if (abs(pwm) > PWM_MIN && abs(codeur_Delta_Pos) <= TICKS_MIN || courant_moyen >= I_BLOCAGE)
            compteur_blocage++;
        else 
            compteur_blocage=0;
        blocage_detecte = (compteur_blocage >= NB_CYCLES_BLOCAGE);
    }
}

void Sensors::mesures(int pwm) {
    getCodeurMoteur();
    getTension();
    getMeuble();
    getPotentiometre();
    consigne = pwm;
}

float Sensors::addCourant(float current) {
    // Fonction qui calcule la moyenne glissante du courant
    courantSomme -= courant_tab[courant_idx];
    courantSomme += current;

    courant_tab[courant_idx] = current;
    courant_idx = (courant_idx + 1) % NB_MOY_COURANT;

    return courantSomme / NB_MOY_COURANT;
}

void Sensors::getCodeurPorte() {
    // Lecture du codeur
    // TODO (Détecter ces valeurs par une méthode d'étalonnage du codeur)
    int codeurValue = map(analogRead(CODEUR_PORTE),0,655,0,360.0)-12;
    if (codeurValue > 210)
        codeurValue -= 360;
    angle_porte = codeurValue;
}

void Sensors::getTension() {
    tension = analogRead(MOTOR_VOLTAGE)*(25/1023.); // en V
}

void Sensors::getCourant() {
    int adc = analogRead(MOTOR_CURRENT);
    int diff = adc - courant_offset;
    float current = diff * 0.02640625f;

    courant_moyen = addCourant(current);
}

void Sensors::getMeuble() {
    sur_meuble = (analogRead(DETECTEUR_MEUBLE) > 512) ? true : false;
}

void Sensors::getCodeurMoteur() {
    codeur_Delta_Pos = encoderGetTicks();
    encoderResetTicks();

    vitesse_moteur = ((3.141592*codeur_Delta_Pos)/512.)/(millis()-time_mesures)*1000.0; // en rad/s
    time_mesures = millis();
    angle_moteur += codeur_Delta_Pos*0.3515625;
}

int32_t Sensors::encoderGetTicks() {
    noInterrupts();
    int32_t ticks = ticks_codeur;
    interrupts();

    return ticks;
}

void Sensors::encoderResetTicks() {
    noInterrupts();
    ticks_codeur = 0;
    interrupts();
}

bool Sensors::isBlocage(bool reset=false) {
    if (blocage_detecte && reset) {
        blocage_detecte = false;
        return true;
    } else {
        return blocage_detecte;
    }
}

void Sensors::getPotentiometre() {
    potentiometre = (analogRead(POTENTIOMETRE)-500)*0.5;
}

void Sensors::setConsigne(int c) {
    consigne = constrain(c, -255, 255);
}