#include "TestManager.h"

namespace {
    struct Broche { MSG nom; uint8_t pin; };
    
    const Broche LEDS[] = {
        {MSG::TEST_LED_MOTOR,       LED_MOTOR_PIN},
        {MSG::TEST_LED_CALIBRATION, LED_CALIBRATION_PIN},
        {MSG::TEST_LED_PILOTAGE,    LED_PILOTAGE_PIN},
        {MSG::TEST_LED_ERROR,       LED_ERROR_PIN}
    };
    const Broche BOUTONS[] = {
        {MSG::TEST_BT_MOUVEMENT,    TEST_BT},
        {MSG::TEST_BT_WIRELESS,     WIRELESS_BT},
        {MSG::TEST_BT_CALIBRATION,  CALIBRATION_BT},
        {MSG::TEST_BT_PILOTAGE,     PILOTAGE_BT}
    };
    const Broche CAPTEURS[] = {
        {MSG::TEST_CAPTEUR_TENSION_MOTEUR,   MOTOR_VOLTAGE_PIN},
        {MSG::TEST_CAPTEUR_COURANT_DRIVER,   DRIVER_CURRENT_PIN},
        {MSG::TEST_CAPTEUR_COURANT_MOTEUR,   MOTOR_CURRENT_PIN},
        {MSG::TEST_CAPTEUR_CODEUR_PORTE,     CODEUR_PORTE_PIN},
        {MSG::TEST_CAPTEUR_MEUBLE,           DETECTEUR_MEUBLE},
        {MSG::TEST_CAPTEUR_POTENTIOMETRE,    POTENTIOMETRE_PIN}
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

    sendDirect(MSGTYPE::INFO, MSG::INFO_TEST_DEBUT);

    if (!buzzer.isEnabled()) {
        buzzer.enable();
        disabled_buzzer = true;
    }

    testSorties();
    if (!quitter) testCapteurs();
    if (!quitter) testI2C();
    if (!quitter) {
        sendTest(MSG::INFO_TEST_CAPTEURS_FIN);
        while (!quitter) service();
    }

    // Retour au repos
    for (uint8_t i = 0; i < NB_LEDS; i++)
        digitalWrite(LEDS[i].pin, LOW);
    buzzer.sequenceFinPilotage();

    if (disabled_buzzer) {
        buzzer.disable();
        disabled_buzzer = false;
    }

    sendDirect(MSGTYPE::INFO, MSG::INFO_TEST_FIN);
}

void TestManager::testSorties() {
    for (uint8_t i = 0; i < NB_LEDS && !quitter; i++) {
        sendTest(LEDS[i].nom);
        digitalWrite(LEDS[i].pin, HIGH);
        attendre(DUREE_SORTIE);
        digitalWrite(LEDS[i].pin, LOW);
    }
    if (quitter) return;
    sendTest(MSG::TEST_BUZZER);
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
            sendTest(BOUTONS[i].nom, presse ? 1:0);
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
        sendDirect(MSGTYPE::INFO, MSG::INFO_TEST_ACTIF);
}

void TestManager::testI2C() {
    if (quitter) return;

    sendTest(MSG::TEST_I2C_CODEUR_PORTE, verifierCodeur() ? 1 : 0);
    
    attendre(PERIODE_LECTURE);        // laisse partir le message (service() vide le buffer TX)

    if (quitter) return;

    sendTest(MSG::TEST_I2C_ACCELEROMETRE, verifierBNO() ? 1 : 0);
    
    attendre(PERIODE_LECTURE);
}

bool TestManager::verifierBNO() {
    if (!capteurs.bno().isEnabled())
        return false;
    capteurs.bno().update();
    if (!capteurs.bno().isConnected())
        return capteurs.bno().checkPresence();

    uint32_t t0 = millis();
    bool demande = false;
    while ((uint32_t)(millis() - t0) < 50) {
        capteurs.bno().update();
        if (!demande)
            demande = capteurs.bno().requestData();   // réessaie tant qu'une lecture précédente est en cours
        else if (capteurs.bno().available())
            break;
    }
    return demande && capteurs.bno().available() && capteurs.bno().isConnected();
}

bool TestManager::verifierCodeur() {
    return capteurs.codeurPorte().checkPresence(100);
}