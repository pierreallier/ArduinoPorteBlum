#include "Sensors.h"

volatile int32_t ticks_codeur=0;
void isr_Codeur() {
    // Gestion interruption du codeur
    ticks_codeur += (PIND & _BV(PD2)) ? -1 : +1;
}

Sensors::Sensors() {
    // Initialisation du tableau du courant
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

    // Calcul offset courant
    for (int i = 0; i < 500; i++) {
        courant_offset += analogRead(MOTOR_CURRENT);
    }
    courant_offset /= 500;

    // Initialisation du codeur du moteur
    pinMode(CODEUR_A_PIN, INPUT_PULLUP);
    pinMode(CODEUR_B_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(CODEUR_A_PIN), isr_Codeur, RISING);

    time_mesures = millis();
}

bool Sensors::checkSecurites() {
    // Vérifie les conditions de sécurité pour le fonctionnement du système : 
    //  - Limites extrémales de la porte
    //  - Limite de courant du moteur

    // Détection des limites extrémales de la porte
    getCodeurPorte();
    limite_haute = (angle_porte >= 180) ? true : false;
    limite_basse = (angle_porte <= -135) ? true : false;

    // Limite de courant
    getCourant();
    limite_courant_atteinte = (courant_moyen >= LIMITE_COURANT) ? true : false;

    return limite_haute || limite_basse || limite_courant_atteinte;
}

void Sensors::mesures() {
    checkSecurites();
    getCodeurMoteur();
    getTension();
    getMeuble();
    getPotentiometre();
}

float Sensors::addCourant(float current) {
  // Fonction qui calcule une moyenne glissante
  courant_moyen -= courant_tab[courant_idx];
  courant_moyen += current;
  courant_tab[courant_idx] = current;
  courant_idx = (courant_idx+1) % NB_MOY_COURANT;
  return (float) courant_moyen / NB_MOY_COURANT;
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
    float current = (analogRead(MOTOR_CURRENT) - courant_offset)*0.02641938126; // en A
    courant_moyen = addCourant(current);
}

void Sensors::getMeuble() {
    sur_meuble = (analogRead(DETECTEUR_MEUBLE) > 512) ? true : false;
}

void Sensors::getCodeurMoteur() {
    float codeur_Delta_Pos = encoderGetTicks();
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

bool Sensors::detectionButees() {
    // Détection des butées par la vitesse et le courant
    if (vitesse_moteur > VITESSE_BUTEE && courant_moyen > COURANT_BUTEE) {
        if (time_detection_butee == 0) {
            time_detection_butee = millis();
        } else if (millis() - time_detection_butee >= TEMPS_BUTEE) {
            time_detection_butee = 0;
            return true;
        }
    }
    else {
        time_detection_butee = 0;
    }
    return false;
}

void Sensors::getPotentiometre() {
    potentiometre = (analogRead(POTENTIOMETRE)-500)*0.5;
}

void Sensors::setConsigne(int consigne) {
    consigne = constrain(consigne, -255, 255);
}