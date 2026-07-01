#include "Sensors.h"

volatile int32_t ticks_codeur;
volatile uint8_t etat_codeur; // ancien état AB (2 bits)
const int8_t tableCodeur[16] = { // table de transition quadrature
  0, -1,  1,  0,
  1,  0,  0, -1,
  -1,  0,  0,  1,
  0,  1, -1,  0
};

unsigned int courant_idx = 0;
float courant_tab[NB_MOY_COURANT];
unsigned long time_detection_butee = 0;

Mesures mesures;

void Mesures_Init() {
    pinMode(CODEUR_PORTE, INPUT);
    pinMode(DETECTEUR_MEUBLE, INPUT);
    pinMode(MOTOR_VOLTAGE, INPUT);
    pinMode(MOTOR_CURRENT, INPUT);
    pinMode(DRIVER_CURRENT, INPUT);
    pinMode(POTENTIOMETRE, INPUT);

    // Initialisation du tableau du courant
    for (int i=0; i < NB_MOY_COURANT; i++) {
        courant_tab[i] = 0;
    }

    // Initialisation du codeur du moteur
    pinMode(CODEUR_A_PIN, INPUT_PULLUP);
    pinMode(CODEUR_B_PIN, INPUT_PULLUP);
    etat_codeur = (PIND >> 2) & 0x03;
    ticks_codeur = 0;
    attachInterrupt(digitalPinToInterrupt(CODEUR_A_PIN), ISR_Codeur, CHANGE);
    attachInterrupt(digitalPinToInterrupt(CODEUR_B_PIN), ISR_Codeur, CHANGE);

    mesures.time_mesures = millis();
}

bool Check_Securites() {
    // Vérifie les conditions de sécurité pour le fonctionnement du système : 
    //  - Limites extrémales de la porte
    //  - Limite de courant du moteur

    // Détection des limites extrémales de la porte
    Get_Codeur_Porte();
    mesures.limite_haute = (mesures.angle_porte >= 180) ? true : false;
    mesures.limite_basse = (mesures.angle_porte <= -135) ? true : false;

    // Limite de courant
    Get_Courant();
    mesures.limite_courant_atteinte = (mesures.courant_moyen >= LIMITE_COURANT) ? true : false;

    return mesures.limite_haute || mesures.limite_basse || mesures.limite_courant_atteinte;
}

void Mesures_Update() {
    Check_Securites();
    Get_Codeur_Moteur();
    Get_Tension();
    Get_Meuble();
    Get_Potentiometre();
}

float add_Courant(float current) {
  // Fonction qui calcule une moyenne glissante
  mesures.courant_moyen -= courant_tab[courant_idx];
  mesures.courant_moyen += current;
  courant_tab[courant_idx] = current;
  courant_idx = (courant_idx+1) % NB_MOY_COURANT;
  return (float) mesures.courant_moyen / NB_MOY_COURANT;
}

void Get_Codeur_Porte() {
    // Lecture du codeur
    // TODO (Détecter ces valeurs par une méthode d'étalonnage du codeur)
    int codeurValue = map(analogRead(CODEUR_PORTE),0,655,0,360.0)-12;
    if (codeurValue > 210)
        codeurValue -= 360;
    mesures.angle_porte = codeurValue;
}

void Get_Tension() {
    mesures.tension = analogRead(MOTOR_VOLTAGE)*(25/(255*1023.)); // en V
}

void Get_Courant() {
    float current = ((analogRead(MOTOR_CURRENT) / 1023.0) * 5.0 - (5.0/2)) / 0.185; // en A
    mesures.courant_moyen = add_Courant(current);
}

void Get_Meuble() {
    mesures.sur_meuble = (analogRead(DETECTEUR_MEUBLE) > 512) ? true : false;
}

void Get_Codeur_Moteur() {
    float codeur_Delta_Pos = Encoder_GetTicks();
    Encoder_ResetTicks();

    mesures.vitesse_moteur = ((3.141592*codeur_Delta_Pos)/128.)/(millis()-mesures.time_mesures)*1000.0; // en rad/s
    mesures.time_mesures = millis();
    mesures.angle_moteur += ((3.141592*codeur_Delta_Pos)/128.);
}

int32_t Encoder_GetTicks() {
    noInterrupts();
    int32_t ticks = ticks_codeur;
    interrupts();

    return ticks;
}

void Encoder_ResetTicks() {
    noInterrupts();
    ticks_codeur = 0;
    interrupts();
}

void ISR_Codeur() {
    // Gestion interruption du codeur
    uint8_t etat = (PIND >> 2) & 0x03;
    uint8_t index = (etat_codeur << 2) | etat;
    ticks_codeur += tableCodeur[index];
    etat_codeur = etat;
}

bool Detection_Butee() {
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

void Get_Potentiometre() {
    mesures.potentiometre = (analogRead(POTENTIOMETRE)-500)*0.5;
}

void Set_Consigne(int consigne) {
    mesures.consigne = constrain(consigne, -255, 255);
}