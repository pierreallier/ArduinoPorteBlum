#include "Sensors.h"

volatile int32_t ticks_codeur=0;
void isr_Codeur() {
    // Gestion interruption du codeur
    ticks_codeur += (PIND & _BV(PD3)) ? -1 : +1;
}

void Sensors::init() {
    pinMode(CODEUR_PORTE, INPUT);
    pinMode(MOTOR_VOLTAGE, INPUT);
    pinMode(MOTOR_CURRENT, INPUT);
    pinMode(DRIVER_CURRENT, INPUT);
    pinMode(POTENTIOMETRE, INPUT);

    // Initialisation des limites
    limite_courant = LIMITE_COURANT;

    // Initialisation du tableau du courant
    _mesures.courant_moyen = 0;
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

    // Initialisation du codeur du moteur
    pinMode(CODEUR_A_PIN, INPUT_PULLUP);
    pinMode(CODEUR_B_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(CODEUR_A_PIN), isr_Codeur, RISING);

    _mesures.time = millis();
}

void Sensors::updateSecurities(int pwm) {
    // Mets à jours les sécurité pour le fonctionnement du système : 
    //  - Limites extrémales de la porte
    //  - Limite de courant du moteur
    //  - Vérifie les blocages

    // Détection des limites extrémales de la porte
    readCodeurPorte();
    float angle = getAnglePorte();
    limite_haute = (angle >= calibrationData.highLimit);
    limite_basse = (angle <= calibrationData.lowLimit);

    // Limite de courant
    readCourant();
    limite_courant_atteinte = (getCourant() >= limite_courant);

    // Détection blocage
    uint8_t pwm_abs = abs(pwm);
    if (pwm_abs < PWM_MIN) { // PWM trop faible pour détecter un blocage mécanique
        compteur_blocage = 0;
        blocage_detecte = false;
        return;
    }
    if (abs(codeur_Delta_Pos) > TICKS_MIN) { // Le moteur tourne suffisamment
        compteur_blocage = 0;
        blocage_detecte = false;
        return;
    }
    compteur_blocage++;
    uint8_t seuil = (pwm_abs >= PWM_RAPIDE) ? NB_CYCLES_BLOCAGE_RAPIDE : NB_CYCLES_BLOCAGE_LENT;
    blocage_detecte = (compteur_blocage >= seuil);
}

bool Sensors::isBlocage() {
    if (blocage_detecte) {
        blocage_detecte = false;
        compteur_blocage = 0;
        return true;
    }
    return false;
}

bool Sensors::isLimiteCourant() {
    if (limite_courant_atteinte) {
        limite_courant_atteinte = false;
        return true;
    }
    return false;
}

bool Sensors::isLimiteAngle() {
    if (limite_haute || limite_basse) {
        limite_haute = false;
        limite_basse = false;
        return true;
    }
    return false;
}


bool Sensors::resetSecurities() {
    blocage_detecte = false;
    compteur_blocage = 0;
    limite_courant_atteinte = false;
    limite_haute = false;
    limite_basse = false;
    return true;
}


void Sensors::mesures(int p) {
    readCodeurMoteur();
    readTension();
    readPotentiometre();
    _mesures.pwm = p * 100;
}

float Sensors::addCourant(float current) {
    // Fonction qui calcule la moyenne glissante du courant
    courantSomme -= courant_tab[courant_idx];
    courantSomme += current;

    courant_tab[courant_idx] = current;
    courant_idx = (courant_idx + 1) % NB_MOY_COURANT;

    return courantSomme / NB_MOY_COURANT;
}


void Sensors::readCourant() {
    int adc = analogRead(MOTOR_CURRENT);
    int diff = adc - courant_offset;
    float current = diff * CURRENT_COEF;

    _mesures.courant_moyen = addCourant(current);
}


void Sensors::readCodeurPorte() {
    // Lecture du codeur
    _mesures.angle_porte = ((int32_t)ADC_MAX + calibrationData.offset - analogRead(CODEUR_PORTE)) % ADC_MAX * CODEURPORTE_COEF;
}


void Sensors::readTension() {
    _mesures.tension = analogRead(MOTOR_VOLTAGE)*VOLTAGE_COEF; // en V*100
}


void Sensors::readCodeurMoteur() {
    codeur_Delta_Pos = encoderGetTicks();
    encoderResetTicks();

    float current_time = millis();
    _mesures.vitesse_moteur = K_VITESSE * codeur_Delta_Pos / (millis() - _mesures.time);   // rad/s en 128
    _mesures.angle_moteur += RAD_PER_TICK * codeur_Delta_Pos; // en rad
    _mesures.time = current_time;
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

void Sensors::readPotentiometre() {
    potentiometre = (analogRead(POTENTIOMETRE)-500)*0.5;
}

void Sensors::setConsigne(int c) {
    _mesures.consigne = c * 100;
}

void Sensors::setLimits(CalibrationData c) {
    calibrationData = c;
}


bool Sensors::setLimitCourant(float limite) {
    if (limite > 0.0f && limite < 2.5f) {
        limite_courant = limite;
        return true;
    }
    return false;
}