#include "Sensors.h"

volatile int32_t ticks_codeur=0;

int courant_offset = 0;
unsigned int courant_idx = 0;
float courant_tab[NB_MOY_COURANT];
unsigned long time_detection_butee = 0;

Mesures mesures;

void capteurs_Init() {
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

    // Initialisation du tableau du courant
    for (int i=0; i < NB_MOY_COURANT; i++) {
        courant_tab[i] = 0;
    }

    // Initialisation du codeur du moteur
    pinMode(CODEUR_A_PIN, INPUT_PULLUP);
    pinMode(CODEUR_B_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(CODEUR_A_PIN), isr_Codeur, RISING);

    mesures.time_mesures = millis();
}

bool check_Securites() {
    // Vérifie les conditions de sécurité pour le fonctionnement du système : 
    //  - Limites extrémales de la porte
    //  - Limite de courant du moteur

    // Détection des limites extrémales de la porte
    get_Codeur_Porte();
    mesures.limite_haute = (mesures.angle_porte >= 180) ? true : false;
    mesures.limite_basse = (mesures.angle_porte <= -135) ? true : false;

    // Limite de courant
    get_Courant();
    mesures.limite_courant_atteinte = (mesures.courant_moyen >= LIMITE_COURANT) ? true : false;

    return mesures.limite_haute || mesures.limite_basse || mesures.limite_courant_atteinte;
}

void mesures_Update() {
    check_Securites();
    get_Codeur_Moteur();
    get_Tension();
    get_Meuble();
    get_Potentiometre();
}

float add_Courant(float current) {
  // Fonction qui calcule une moyenne glissante
  mesures.courant_moyen -= courant_tab[courant_idx];
  mesures.courant_moyen += current;
  courant_tab[courant_idx] = current;
  courant_idx = (courant_idx+1) % NB_MOY_COURANT;
  return (float) mesures.courant_moyen / NB_MOY_COURANT;
}

void get_Codeur_Porte() {
    // Lecture du codeur
    // TODO (Détecter ces valeurs par une méthode d'étalonnage du codeur)
    int codeurValue = map(analogRead(CODEUR_PORTE),0,655,0,360.0)-12;
    if (codeurValue > 210)
        codeurValue -= 360;
    mesures.angle_porte = codeurValue;
}

void get_Tension() {
    mesures.tension = analogRead(MOTOR_VOLTAGE)*(25/1023.); // en V
}

void get_Courant() {
    float current = (analogRead(MOTOR_CURRENT) - courant_offset)*0.02641938126; // en A
    mesures.courant_moyen = add_Courant(current);
}

void get_Meuble() {
    mesures.sur_meuble = (analogRead(DETECTEUR_MEUBLE) > 512) ? true : false;
}

void get_Codeur_Moteur() {
    float codeur_Delta_Pos = encoder_GetTicks();
    encoder_ResetTicks();

    mesures.vitesse_moteur = ((3.141592*codeur_Delta_Pos)/512.)/(millis()-mesures.time_mesures)*1000.0; // en rad/s
    mesures.time_mesures = millis();
    mesures.angle_moteur += codeur_Delta_Pos*0.3515625;
}

int32_t encoder_GetTicks() {
    noInterrupts();
    int32_t ticks = ticks_codeur;
    interrupts();

    return ticks;
}

void encoder_ResetTicks() {
    noInterrupts();
    ticks_codeur = 0;
    interrupts();
}

void isr_Codeur() {
    // Gestion interruption du codeur
    ticks_codeur += (PIND & _BV(PD2)) ? -1 : +1;
}

bool detection_Butee() {
    // Détection des butées par la vitesse et le courant
    if (mesures.vitesse_moteur > VITESSE_BUTEE && mesures.courant_moyen > COURANT_BUTEE) {
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

void get_Potentiometre() {
    mesures.potentiometre = (analogRead(POTENTIOMETRE)-500)*0.5;
}

void set_Consigne(int consigne) {
    mesures.consigne = constrain(consigne, -255, 255);
}