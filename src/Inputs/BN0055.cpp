#include "BN0055.h"

// Instance utilisée par les callbacks statiques de nI2C.
BN0055 *BN0055::_instance = nullptr;

BN0055::BN0055() {
    _enabled = false;
    _connected = false;
    _state = DISABLED;
    _dataAvailable = false;
    _lastI2CStatus = CI2C::STATUS_OK;
    _lastDetectionTime = 0;
    _modeStartTime = 0;
}

void BN0055::init() {
    _i2cHandle = nI2C->RegisterDevice(ADDRESS,1,CI2C::Speed::FAST);
    _instance = this;
}


void BN0055::setEnabled(bool enabled) {
    if (enabled == _enabled)
        return;
    _enabled = enabled;

    if (!_enabled) {
        _connected = false;
        _dataAvailable = false;
        _state = DISABLED;
        return;
    }

    // Activation
    _connected = false;
    _dataAvailable = false;

    // On pourra lancer immédiatement une recherche.
    _lastDetectionTime = millis() - DETECTION_INTERVAL_MS;
    _state = SEARCHING;
}


bool BN0055::isEnabled() const {
    return _enabled;
}


bool BN0055::isConnected() const {
    return _connected;
}


void BN0055::update() {
    if (!_enabled)
        return;
    // Recherche du BN0055
    if (_state == SEARCHING) {
        uint32_t now = millis();
        if ((uint32_t)(now - _lastDetectionTime) >= DETECTION_INTERVAL_MS) {
            _lastDetectionTime = now;
            startDetection();
        }
        return;
    }
    // Démarrage du mode NDOF
    if (_state == STARTING) {
        // Après l'écriture du mode NDOF, on laisse au BN0055
        if ((uint32_t)(millis() - _modeStartTime) >= MODE_START_DELAY_MS) {
            _state = READY;
        }
        return;
    }
}


void BN0055::startDetection() {
    if (!_enabled)
        return;
    // Lecture du CHIP_ID.
    uint8_t status = nI2C->Read(_i2cHandle,REG_CHIP_ID,_dataBuffer,1,chipIdCallback);
    if (status != CI2C::STATUS_OK) {
        // Erreur : on réessaiera lors du prochain passage.
        _lastI2CStatus = status;
        return;
    }
    _detectionPending = true;
}


void BN0055::chipIdCallback(uint8_t status) {
    if (_instance == nullptr)
        return;
    _instance->onChipId(status);
}


void BN0055::onChipId(uint8_t status) {
    _detectionPending = false;
    _lastI2CStatus = status;
    if (!_enabled)
        return;
    // Erreur I2C
    if (status != CI2C::STATUS_OK) {
        // Le capteur est probablement absent : la recherche sera faite dans 1 seconde.
        _connected = false;
        _state = SEARCHING;
        return;
    }
    // Vérification CHIP_ID
    if (_dataBuffer[0] != CHIP_ID) {
        // Quelque chose répond à l'adresse mais ce n'est probablement pas un BN0055.
        _connected = false;
        _state = SEARCHING;
        return;
    }
    // BN0055 trouvé
    _connected = true;
    startMode();
}


bool BN0055::startMode() {
    // Passage en mode NDOF
    if (!_enabled)
        return false;

    _mode = OPERATION_MODE_NDOF;
    uint8_t status = nI2C->Write(_i2cHandle,REG_OPR_MODE,&_mode,1);

    if (status != CI2C::STATUS_OK) {
        _lastI2CStatus = status;
        _connected = false;
        _state = SEARCHING;
        return false;
    }
    _modeStartTime = millis();
    _state = STARTING;
    return true;
}

bool BN0055::checkPresence(uint16_t timeoutMs) {
    if (!_enabled)
        return false;
    uint32_t t0 = millis();

    _lastDetectionTime = t0;
    startDetection();                    // tentative immédiate

    while ((uint32_t)(millis() - t0) < timeoutMs) {
        update();                        // fait passer STARTING → READY après 20 ms
        if (_state == READY)
            return true;
        // Tentative terminée sans succès : on s'arrête, update() réessaiera en tâche de fond
        if (!_detectionPending && _state == SEARCHING)
            return false;
    }
    _detectionPending = false;           // timeout : on abandonne l'attente, pas la détection
    return false;
}


bool BN0055::requestData(uint32_t now) {
    if (!_enabled || !_connected || _state == READING || _state == STARTING)
        return false;
    _dataAvailable = false; // Si une ancienne mesure n'a pas été lue, on la perd.
    uint8_t status = nI2C->Read(_i2cHandle,REG_GYRO_DATA,_dataBuffer,DATA_SIZE,dataCallback);
    if (status != CI2C::STATUS_OK) {
        _lastI2CStatus = status;
        // STATUS_BUSY signifie qu'une autre transaction utilise actuellement le bus.
        // Dans ce cas on ne considère PAS le BN0055 comme déconnecté.
        if (status == CI2C::STATUS_BUSY) {
            return false;
        }
        handleDisconnection(status);
        return false;
    }
    _state = READING;
    _mesures.time = now;
    return true;
}


void BN0055::dataCallback(uint8_t status) {
    if (_instance == nullptr)
        return;
    _instance->onData(status);
}


void BN0055::onData(uint8_t status) {
    _lastI2CStatus = status;
    if (!_enabled) { // Capteur désactivé : on ignore la mesure.
        _state = DISABLED;
        return;
    }
    
    if (status != CI2C::STATUS_OK) { // Erreur I2C
        handleDisconnection(status);
        return;
    }
    _mesures.time = millis();
    _dataAvailable = true;
    _state = READY;
}


bool BN0055::available() {
    return _dataAvailable;
}


bool BN0055::readData() {
    if (!_dataAvailable)
        return false;
    // Conversion du buffer brut
    // Gyroscope
    _mesures.gyroX = ((int32_t)makeInt16(_dataBuffer, 0) * 100) >> 4; // Conversion en centi-degrés/s
    _mesures.gyroY = ((int32_t)makeInt16(_dataBuffer, 2) * 100) >> 4;
    _mesures.gyroZ = ((int32_t)makeInt16(_dataBuffer, 4) * 100) >> 4;
    // Euler
    _mesures.heading = ((int32_t)makeInt16(_dataBuffer, 6) * 100) >> 4; // Conversion en centi-degrés
    _mesures.roll    = ((int32_t)makeInt16(_dataBuffer, 8) * 100) >> 4;
    _mesures.pitch   = ((int32_t)makeInt16(_dataBuffer, 10) * 100) >> 4;
    // Accélération linéaire
    _mesures.accelX = makeInt16(_dataBuffer, 20); // Conversion en centi-degrés/s²
    _mesures.accelY = makeInt16(_dataBuffer, 22);
    _mesures.accelZ = makeInt16(_dataBuffer, 24);
    // Mesure consommée
    _dataAvailable = false;
    return true;
}


const uint8_t* BN0055::getDataBuffer() const {
    return _dataBuffer;
}


uint8_t BN0055::getDataBufferSize() const {
    return DATA_SIZE;
}


void BN0055::handleDisconnection(uint8_t status) {
    _lastI2CStatus = status;
    _connected = false;
    _dataAvailable = false;
    _lastDetectionTime = millis();
    _state = SEARCHING;
}


int16_t BN0055::makeInt16(const uint8_t *buffer, uint8_t offset) {
    return (int16_t)( ((uint16_t)buffer[offset + 1] << 8) | buffer[offset] );
}