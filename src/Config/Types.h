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
struct MesuresBN0055 { // TAILLE = 25 octets
    uint8_t start = 0x01;
    uint8_t type = 'A';
    uint32_t time = 0; // ms
    int16_t accelX = 0; // degrés/s²*100
    int16_t accelY = 0; // degrés/s²*100
    int16_t accelZ = 0; // degrés/s²*100
    int16_t gyroX = 0; // 1/16 °/s
    int16_t gyroY = 0; // 1/16 °/s
    int16_t gyroZ = 0; // 1/16 °/s
    int16_t heading = 0; // degrés*100
    int16_t roll = 0; // degrés*100
    int16_t pitch = 0; // degrés*100
    uint8_t crc = 0; // CRC-8/SMBUS, calcule juste avant envoi
};

// @brief Structure pour stocker les mesures du système
struct Mesures { // TAILLE = 27 octets
    uint8_t start = 0x01;
    uint8_t type = 'M';
    uint32_t time = 0; // ms
    int16_t tension = 0; // V*100
    int16_t courant_moyen = 0; // A*100
    int32_t angle_moteur = 0; // °*100
    int32_t vitesse_moteur = 0; // rad/s*100
    int32_t angle_porte = 0;  // °*100 
    int32_t consigne = 0; // PWM*100 ou °*100 ou rad/s*100
    uint8_t crc = 0; // CRC-8/SMBUS, calcule juste avant envoi
};

// @brief Message hors mesure : TYPE;CODE;VAL (8 octets avec start et CRC).
struct Event { // TAILLE = 8 octets
    uint8_t start = 0x01; // octet de début
    char    type; // I, W, E, S, O, N, R, T
    MSG     code; //uint8_t
    int32_t val;
    uint8_t crc = 0; // CRC-8/SMBUS, calcule juste avant envoi
};

// CRC-8/SMBUS: poly=0x07, init=0x00, refin=false, refout=false, xorout=0x00.
// Check: CRC("123456789") = 0xF4.
inline uint8_t crc8(const uint8_t* data, size_t length) {
    uint8_t crc = 0;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x80) ? static_cast<uint8_t>((crc << 1) ^ 0x07)
                               : static_cast<uint8_t>(crc << 1);
        }
    }
    return crc;
}

// The wire structures must have no padding and end with the CRC byte.
template<typename T>
    inline void updateFrameCrc(T& frame) {
        frame.crc = crc8(reinterpret_cast<const uint8_t*>(&frame), sizeof(T) - 1);
}


// La transmission brute suppose la disposition AVR, sans padding.
static_assert(sizeof(Mesures) == 27, "Disposition binaire Mesures incompatible");
static_assert(sizeof(MesuresBN0055) == 25, "Disposition binaire MesuresBN0055 incompatible");
static_assert(sizeof(Event) == 8, "Disposition binaire Event incompatible");

#endif
