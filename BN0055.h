#ifndef BN0055_H
#define BN0055_H

#include <Arduino.h>
#include <nI2C.h>

// Capteur de position BNO055 (9 axes) lu en I2C asynchrone via nI2C.
//
// Utilisation :
//   - setup()  : begin()
//   - loop()   : update() périodiquement pour vérifier la présence du capteur.
//                requestData() pour demander une nouvelle mesure. Dès que available() est vrai,
//                readData() (ou get*) récupère la mesure.
// Pour activer/désactiver le capteur : setEnabled() (désactive la communication I2C et ignore les mesures).
//
// Ne pas utiliser Wire dans le projet : nI2C et Wire se partagent le même TWI.


class BN0055 {
public:

    explicit BN0055();
    void init(); // À appeler une seule fois dans setup() pour initialiser le capteur.

    // Active/désactive complètement le capteur. Quand disabled :
    //   - aucune communication I2C
    //   - aucune tentative de détection
    void setEnabled(bool enabled);

    bool isEnabled() const;
    bool isConnected() const;

    // Vérification de présence au démarrage : attend au plus timeoutMs que le capteur
    // soit détecté ET en mode NDOF. Ne bloque jamais plus que le timeout.
    bool checkPresence(uint16_t timeoutMs = 300);

    // À appeler très fréquemment dans loop(). Ne bloque pas.
    void update();

    // Demande une nouvelle acquisition.
    //
    // La fonction démarre une transaction I2C mais retourne immédiatement.
    //
    // Retourne false si :
    //   - capteur désactivé
    //   - capteur absent
    //   - acquisition déjà en cours
    //   - initialisation en cours
    bool requestData(uint32_t now);

    // Vrai lorsqu'une nouvelle mesure est disponible.
    bool available();

    // Récupère la dernière mesure disponible.
    // Retourne false si aucune nouvelle donnée n'est disponible.
    bool readData(float &accelX,float &accelY,float &accelZ,
                  float &gyroX,float &gyroY,float &gyroZ,
                  float &heading,float &roll,float &pitch);

    uint32_t getTime() const { return _lastReadTime; }
    float getAcceleration(int axis); // 0 = X, 1 = Y, 2 = Z
    float getGyroscope(int axis); // 0 = X, 1 = Y, 2 = Z
    float getEulerAngle(int axis); // 0 = heading, 1 = roll, 2 = pitch
    void clear(); // Efface le flag de disponibilité des données

    // Accès lecture seule au buffer brut
    const uint8_t* getDataBuffer() const;
    uint8_t getDataBufferSize() const;

    uint8_t getLastStatus() const { return _lastI2CStatus; }

private:

    // Registres BNO055
    static const uint8_t REG_CHIP_ID = 0x00;
    static const uint8_t REG_OPR_MODE = 0x3D;
    static const uint8_t REG_GYRO_DATA = 0x14;
    static const uint8_t REG_LINEAR_ACCEL = 0x28;
    static const uint8_t CHIP_ID = 0xA0;
    static const uint8_t OPERATION_MODE_NDOF = 0x0C;
    static const uint8_t ADDRESS = 0x28;

    static const uint8_t DATA_SIZE = 26;

    // Temporisations
    static const uint32_t DETECTION_INTERVAL_MS = 1000;
    static const uint32_t MODE_START_DELAY_MS = 20;

    // Machine d'état
    enum State {
        DISABLED,
        SEARCHING,
        STARTING,
        READY,
        READING
    };

    State _state;

    bool _enabled;
    bool _connected;

    CI2C::Handle _i2cHandle;

    uint8_t _dataBuffer[DATA_SIZE]; // Buffer I2C.

    volatile bool _dataAvailable; // Vrai lorsque de nouvelles données sont disponibles
    volatile uint8_t _lastI2CStatus; // Dernière erreur I2C
    volatile bool _detectionPending = false; // lecture CHIP_ID en cours

    uint32_t _lastDetectionTime;
    uint32_t _modeStartTime;
    uint8_t _mode;
    uint32_t _lastReadTime = 0;

    // Callbacks nI2C
    static BN0055 *_instance;

    static void chipIdCallback(uint8_t status);
    static void dataCallback(uint8_t status);

    void onChipId(uint8_t status);
    void onData(uint8_t status);

    // Fonctions internes
    void startDetection();
    bool startMode();
    void handleDisconnection(uint8_t status);
    int16_t makeInt16(const uint8_t *buffer, uint8_t offset);
};

#endif
