#include "Sensors.h"

volatile int32_t ticks_codeur=0;
void isr_Codeur() {
    // Gestion interruption du codeur
    ticks_codeur += (PIND & _BV(PD2)) ? -1 : +1;
}

void Sensors::init() {
    pinMode(CODEUR_PORTE, INPUT);
    pinMode(DETECTEUR_MEUBLE, INPUT);
    pinMode(MOTOR_VOLTAGE, INPUT);
    pinMode(MOTOR_CURRENT, INPUT);
    pinMode(DRIVER_CURRENT, INPUT);
    pinMode(POTENTIOMETRE, INPUT);

    // Initialisation des limites
    angle_haut_max = ANGLE_MAX;
    angle_bas_max = ANGLE_MIN; 

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
    limite_haute = (angle_porte >= angle_haut_max);
    limite_basse = (angle_porte <= angle_bas_max);

    // Limite de courant
    getCourant();
    limite_courant_atteinte = (courant_moyen >= LIMITE_COURANT);

    // Limites
    if (limite_haute || limite_basse || limite_courant_atteinte) {
        compteur_blocage = 0;
        blocage_detecte = false;
        return;
    }

    // Détection blocage
    uint8_t pwm_abs = abs(pwm);
    if (pwm_abs < PWM_MIN) { // PWM trop faible pour détecter un blocage mécanique
        compteur_blocage = 0;
        blocage_detecte = false;
        return;
    }
    if (abs(codeur_Delta_Pos) > TICKS_MIN && courant_moyen < I_BLOCAGE) { // Le moteur tourne suffisamment
        compteur_blocage = 0;
        blocage_detecte = false;
        return;
    }
    compteur_blocage++;
    uint8_t seuil = (pwm_abs >= PWM_RAPIDE) ? NB_CYCLES_BLOCAGE_RAPIDE : NB_CYCLES_BLOCAGE_LENT;
    blocage_detecte = (compteur_blocage >= seuil);

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
    float codeurValue = analogRead(CODEUR_PORTE) * (360.0f / 655.0f) - 12.0f;
    if (codeurValue > 210.0f)
        codeurValue -= 360.0f;
    angle_porte = codeurValue;
}

void Sensors::getTension() {
    tension = analogRead(MOTOR_VOLTAGE)*VOLTAGE_COEF; // en V
}

void Sensors::getCourant() {
    int adc = analogRead(MOTOR_CURRENT);
    int diff = adc - courant_offset;
    float current = diff * CURRENT_COEF;

    courant_moyen = addCourant(current);
}

void Sensors::getMeuble() {
    sur_meuble = (analogRead(DETECTEUR_MEUBLE) > 512) ? true : false;
}

void Sensors::getCodeurMoteur() {
    codeur_Delta_Pos = encoderGetTicks();
    encoderResetTicks();

    float current_time = millis();
    vitesse_moteur = K_VITESSE * codeur_Delta_Pos / (millis() - time_mesures);   // rad/s en 128
    angle_moteur += RAD_PER_TICK * codeur_Delta_Pos; // en rad
    time_mesures = current_time;
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

void Sensors::setLimits(float limite_basse, float limite_haute) {
    if (limite_basse > ANGLE_MIN){
        angle_bas_max = limite_basse;
    }
    if (limite_haute < ANGLE_MAX) {
        angle_haut_max = limite_haute;
    }
}

void Sensors::resetLimits() {
    angle_bas_max = ANGLE_MIN;
    angle_haut_max = ANGLE_MAX;
}