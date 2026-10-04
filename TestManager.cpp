#include "TestManager.h"

// TODO : 
// - ne pas utiliser la librairie de message mais Serial.print() directement

namespace {
    struct Broche { const char* nom; uint8_t pin; };
    
    const Broche LEDS[] = {
        {"LED_MOTOR",       LED_MOTOR_PIN},
        {"LED_CALIBRATION", LED_CALIBRATION_PIN},
        {"LED_PILOTAGE",    LED_PILOTAGE_PIN},
        {"LED_ERROR",       LED_ERROR_PIN}
    };
    const Broche BOUTONS[] = {
        {"BT_TEST",        TEST_BT},
        {"BT_WIRELESS",    WIRELESS_BT},
        {"BT_CALIBRATION", CALIBRATION_BT},
        {"BT_PILOTAGE",    PILOTAGE_BT}
    };
    const Broche CAPTEURS[] = {
        {"CAPTEUR_A0_TENSION_MOTEUR",   MOTOR_VOLTAGE},
        {"CAPTEUR_A1_COURANT_DRIVER",   DRIVER_CURRENT},
        {"CAPTEUR_A2_COURANT_MOTEUR",   MOTOR_CURRENT},
        {"CAPTEUR_A3_CODEUR_PORTE",     CODEUR_PORTE},
        {"CAPTEUR_A4_DETECTEUR_MEUBLE", DETECTEUR_MEUBLE},
        {"CAPTEUR_A5_POTENTIOMETRE",    POTENTIOMETRE}
    };

    const uint8_t NB_LEDS     = sizeof(LEDS)     / sizeof(LEDS[0]);
    const uint8_t NB_BOUTONS  = sizeof(BOUTONS)  / sizeof(BOUTONS[0]);
    const uint8_t NB_CAPTEURS = sizeof(CAPTEURS) / sizeof(CAPTEURS[0]);

    const uint32_t DUREE_SORTIE    = 1500; // ms par sortie
    const uint8_t  NB_LECTURES     = 5;   // par capteur
    const uint32_t PERIODE_LECTURE = 250;  // ms -> 5 s par capteur
    const uint32_t PERIODE_ENTREES = 20;   // ms (anti-rebond simple)
}

void TestManager::run() {
    quitter = false;

    for (uint8_t i = 0; i < NB_LEDS; i++)
        digitalWrite(LEDS[i].pin, LOW);
    for (uint8_t i = 0; i < NB_BOUTONS; i++)
        etatBoutons[i] = (digitalRead(BOUTONS[i].pin) == LOW);
    tEntrees = millis();

    sendInfo("Debut du mode test (DO STOP pour quitter)");

    testSorties();
    if (!quitter) testCapteurs();
    if (!quitter) testI2C();
    if (!quitter) {
        sendInfo("Sequence terminee, entrees toujours surveillees (DO STOP pour quitter)");
        while (!quitter) service();
    }

    // Retour au repos
    for (uint8_t i = 0; i < NB_LEDS; i++)
        digitalWrite(LEDS[i].pin, LOW);
    buzzer.stop();
    sendInfo("Fin du mode test");

    // Vide le buffer TX avant de rendre la main
    uint32_t t = millis();
    while (!SerialTxBuffer::instance().isEmpty() && millis() - t < 500)
        SerialTxBuffer::instance().send(Serial);
}

void TestManager::testSorties() {
    for (uint8_t i = 0; i < NB_LEDS && !quitter; i++) {
        sendTest("SORTIE", LEDS[i].nom);
        digitalWrite(LEDS[i].pin, HIGH);
        attendre(DUREE_SORTIE);
        digitalWrite(LEDS[i].pin, LOW);
    }
    if (quitter) return;
    sendTest("SORTIE", "BUZZER");
    buzzer.play(3, 150, 150);
    attendre(DUREE_SORTIE);
}

void TestManager::testCapteurs() {
    for (uint8_t i = 0; i < NB_CAPTEURS && !quitter; i++) {
        //sendTest("CAPTEUR", CAPTEURS[i].nom);
        for (uint8_t k = 0; k < NB_LECTURES && !quitter; k++) {
            sendTest(CAPTEURS[i].nom, (long)analogRead(CAPTEURS[i].pin)); // Valeur brute du CAN
            attendre(PERIODE_LECTURE);
        }
    }
}

bool TestManager::attendre(uint32_t ms) {
    uint32_t t0 = millis();
    while (!quitter && millis() - t0 < ms)
        service();
    return !quitter;
}

void TestManager::service() {
    buzzer.task();
    surveillerEntrees();
    lireCommande();
    SerialTxBuffer::instance().send(Serial);
}

void TestManager::surveillerEntrees() {
    uint32_t now = millis();
    if (now - tEntrees < PERIODE_ENTREES)
        return;
    tEntrees = now;

    // Boutons (actifs à l'état bas, INPUT_PULLUP déjà configuré dans setup)
    for (uint8_t i = 0; i < NB_BOUTONS; i++) {
        bool presse = (digitalRead(BOUTONS[i].pin) == LOW);
        if (presse != etatBoutons[i]) {
            etatBoutons[i] = presse;
            sendTest(BOUTONS[i].nom, presse ? "PRESSE" : "RELACHE");
        }
    }
}

void TestManager::lireCommande() {
    if (Serial.available() <= 0)
        return;
    String c = Serial.readStringUntil('\n');
    c.trim();
    c.toUpperCase();
    if (c.indexOf("STOP") >= 0 || c.indexOf("FIN_TEST") >= 0)
        quitter = true;
    else if (c.length() > 0)
        sendWarning("Mode test actif : DO STOP pour quitter");
}

void TestManager::testI2C() {
    if (quitter) return;

    if (!verifierCodeur()) {
        sendTest("I2C_CODEUR_PORTE", "NON_PRESENT");
    } else {
        sendTest("I2C_CODEUR_PORTE", "PRESENT");
    }
    attendre(PERIODE_LECTURE);        // laisse partir le message (service() vide le buffer TX)

    if (quitter) return;

    if (!verifierBNO()) {
        sendTest("I2C_ACCELEROMETRE", "NON_PRESENT");
    } else {
        sendTest("I2C_ACCELEROMETRE", "PRESENT");
    }
    attendre(PERIODE_LECTURE);
}

bool TestManager::verifierBNO() {
    if (!bno.isEnabled())
        return false;
    bno.update();
    if (!bno.isConnected())
        return bno.checkPresence();

    uint32_t t0 = millis();
    bool demande = false;
    while ((uint32_t)(millis() - t0) < 50) {
        bno.update();
        if (!demande)
            demande = bno.requestData(millis());   // réessaie tant qu'une lecture précédente est en cours
        else if (bno.available())
            break;
    }
    return demande && bno.available() && bno.isConnected();
}

bool TestManager::verifierCodeur() {
    return codeurPorte.checkPresence(100);
}