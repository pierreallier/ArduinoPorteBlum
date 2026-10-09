#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>
#include "Codes.h"

// @brief Structure pour stocker les 3 valeurs d'étalonnage
struct CalibrationData {
    uint16_t  highLimit = 0xFFFF;   
    uint16_t  lowLimit = 0; 
    uint16_t  offset = 0;
};

// @brief Structure pour stocker les mesures du capteur BN0055
struct MesuresBN0055 { // TAILLE = 35 octets
    uint8_t bin = 0x02;
    uint32_t time = 0; // ms
    int16_t accelX = 0; // degrés/s²*100
    int16_t accelY = 0; // degrés/s²*100
    int16_t accelZ = 0; // degrés/s²*100
    int32_t gyroX = 0; // degrés/s*100
    int32_t gyroY = 0; // degrés/s*100
    int32_t gyroZ = 0; // degrés/s*100
    int32_t heading = 0; // degrés*100
    int32_t roll = 0; // degrés*100
    int32_t pitch = 0; // degrés*100
};

// @brief Structure pour stocker les mesures du système
struct Mesures { // TAILLE = 25 octets
    uint8_t bin = 0x01;
    uint32_t time = 0; // ms
    uint16_t tension = 0; // V*100
    int16_t courant_moyen = 0; // A*100
    int32_t angle_moteur = 0; // °*100
    int32_t vitesse_moteur = 0; // rad/s*100
    uint16_t angle_porte = 0;  // °*100 
    int16_t pwm = 0; // *100
    int32_t consigne = 0; // °*100 ou rad/s*100
};

// @brief Message hors mesure : TYPE;CODE;VAL (6 octets en RAM).
struct Event { // TAILLE = 7 octets
    uint8_t bin = 0x00; // 0x00 pour message binaire
    char    type; // I, W, E, S, O, N, R, T
    MSG     code; //uint8_t
    int32_t val;
};

#endif