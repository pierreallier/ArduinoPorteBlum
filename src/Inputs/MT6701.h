#ifndef MT6701_H
#define MT6701_H

#include <Arduino.h>
#include <nI2C.h>
#include "Sensor.h"

#ifndef ADC_MAX
    #define ADC_MAX 1024
#endif

// Codeur magnétique absolu MT6701 (14 bits) lu en I2C asynchrone via nI2C ou analogique.
//
// Utilisation I2C :
//   - setup()  : init() puis checkPresence() (bloquant, une seule fois)
//   - loop()   : requestData() périodiquement ; dès que available() est vrai,
//                readAngle() récupère la mesure.
//
// Utilisation analogique :
//   - setup()  : rien
//   - loop()   : read() récupère la mesure
//
// Ne pas utiliser Wire dans le projet : nI2C et Wire se partagent le même TWI.

class MT6701 : public Sensor<uint16_t> {
public:
    MT6701(uint8_t pin, float coeff, uint8_t address = 0x06);

    // Enregistre le composant auprès de nI2C (à appeler dans setup()).
    void init();

    // Lecture via la sortie analogique du codeur
    uint16_t read() {_value = ((int32_t)ADC_MAX + _offset - analogRead(_pin)) % ADC_MAX * _coeff; return _value;}

    // Commandes via protocole I2C
    // BLOQUANT (setup uniquement). Lit l'angle une fois et attend la fin de la
    // transaction. Retourne true si le codeur répond avant le timeout.
    bool checkPresence(uint16_t timeoutMs = 100);

    // Lance une lecture asynchrone. Retourne false si une lecture est déjà
    // en cours ou si le bus est occupé.
    bool requestData();

    bool available() const { return _dataAvailable; }
    bool isConnected() const { return _connected; }

    // Dernière mesure en degrés, [0 ; 360[, offset appliqué.
    // Retourne false si aucune nouvelle mesure n'est disponible.
    uint16_t readAngle(float &angle);

    // Valeur brute 14 bits de la dernière mesure (0..16383).
    uint16_t getRaw() const;

    uint8_t getLastStatus() const { return _lastI2CStatus; }

private:
    uint8_t _address;   // Adresse I2C 7 bits
    static const uint8_t REG_ANGLE = 0x03; // ANGLE[13:6] ; 0x04 : ANGLE[5:0] dans les bits 7:2
    static const uint8_t DATA_SIZE = 2;
    static const uint16_t I2C_MAX = 16384;

    CI2C::Handle _i2cHandle;
    uint8_t _dataBuffer[DATA_SIZE];

    volatile bool _connected;
    volatile bool _busy;
    volatile bool _dataAvailable;
    volatile uint8_t _lastI2CStatus;
    volatile uint16_t _raw;

    static MT6701 *_instance;

    static void dataCallback(uint8_t status);
    void onData(uint8_t status);

    uint8_t startRead();
};

#endif