#include "MT6701.h"

MT6701 *MT6701::_instance = nullptr;

MT6701::MT6701() {
    _connected = false;
    _busy = false;
    _dataAvailable = false;
    _lastI2CStatus = CI2C::STATUS_OK;
    _raw = 0;
    _offset = 0.0f;
}


void MT6701::init() {
    _i2cHandle = nI2C->RegisterDevice(ADDRESS, 1, CI2C::Speed::FAST);
    _instance = this;
}


// Démarre une lecture asynchrone des 2 octets d'angle.
uint8_t MT6701::startRead() {
    _busy = true; // avant l'appel : le callback peut arriver avant le retour de Read()
    uint8_t status = nI2C->Read(_i2cHandle, REG_ANGLE, _dataBuffer, DATA_SIZE, dataCallback);
    if (status != CI2C::STATUS_OK) {
        _busy = false;
        _lastI2CStatus = status;
    }
    return status;
}


bool MT6701::checkPresence(uint16_t timeoutMs) {
    uint32_t t0 = millis();
    uint8_t status;

    // Bus occupé (autre capteur en cours de lecture) : on réessaie jusqu'au timeout.
    do {
        status = startRead();
    } while (status == CI2C::STATUS_BUSY && (uint32_t)(millis() - t0) < timeoutMs);

    if (status != CI2C::STATUS_OK) {
        _connected = false;
        return false;
    }

    // Attente de la fin de la transaction (callback).
    while (_busy && (uint32_t)(millis() - t0) < timeoutMs) {
    }

    if (_busy) { // Timeout : pas de réponse
        _busy = false;
        _connected = false;
        return false;
    }
    return _connected;
}


bool MT6701::requestData() {
    if (_busy)
        return false;
    uint8_t status = startRead();
    // Bus occupé : ce n'est pas une déconnexion du codeur.
    if (status != CI2C::STATUS_OK && status != CI2C::STATUS_BUSY)
        _connected = false;
    return status == CI2C::STATUS_OK;
}


void MT6701::dataCallback(uint8_t status) {
    if (_instance == nullptr)
        return;
    _instance->onData(status);
}


void MT6701::onData(uint8_t status) {
    _lastI2CStatus = status;
    _busy = false;
    if (status != CI2C::STATUS_OK) {
        _connected = false;
        _dataAvailable = false;
        return;
    }
    // ANGLE[13:6] = octet 0 ; ANGLE[5:0] = bits 7:2 de l'octet 1
    _raw = ((uint16_t)_dataBuffer[0] << 6) | (_dataBuffer[1] >> 2);
    _connected = true;
    _dataAvailable = true;
}


uint16_t MT6701::getRaw() const {
    noInterrupts();
    uint16_t value = _raw;
    interrupts();
    return value;
}


bool MT6701::readAngle(float &angle) {
    if (!_dataAvailable)
        return false;
    float a = getRaw() * (360.0f / 16384.0f) + _offset;
    while (a < 0.0f)
        a += 360.0f;
    while (a >= 360.0f)
        a -= 360.0f;
    angle = a;
    _dataAvailable = false;
    return true;
}