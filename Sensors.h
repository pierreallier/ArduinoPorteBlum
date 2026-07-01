#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

#define CODEUR_PORTE A4 // Codeur de position absolue de la porte
#define DETECTEUR_MEUBLE A5 // Détecteur de présence du meuble
#define MOTOR_VOLTAGE A0 // Driver du moteur : mesure de la tension
#define MOTOR_CURRENT A1 // Mesure du courant fournit au moteur
#define DRIVER_CURRENT A3 // Driver du moteur : mesure du courant
#define POTENTIOMETRE A2 // Potentiomètre réglage vitesse moteur

// Codeur incrémental du moteur
#define CODEUR_A_PIN 18
#define CODEUR_B_PIN 19

// Variables pour le calcul de la moyenne glissante du courant
const float LIMITE_COURANT = 2;
const unsigned int NB_MOY_COURANT = 5;

// Détections butées
const float COURANT_BUTEE = 2.8;      // A
const float VITESSE_BUTEE = 5;        // impulsions/10 ms
const uint16_t TEMPS_BUTEE = 50;      // ms

struct Mesures {
    float time_mesures;
    float tension;
    float courant_moyen;
    float angle_moteur;
    float vitesse_moteur;
    float angle_porte;
    float potentiometre;
    bool sur_meuble;

    int consigne;

    bool limite_haute = false ;
    bool limite_basse = false ;
    bool limite_courant_atteinte = false;
};
extern Mesures mesures;

void mesures_Init();
void mesures_Update();

bool check_Securites();
bool detection_Butee();

void get_Tension();
void get_Courant();
void get_Meuble();
void get_Codeur_Porte();
void get_Codeur_Moteur();
void get_Potentiometre();

void set_Consigne();

// Fonctions internes
void isr_Codeur();
int32_t encoder_GetTicks();
void encoder_ResetTicks();
float add_Courant(float current);

#endif