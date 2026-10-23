#include <Arduino.h>
#include <Bounce2.h>

#include "Config/Constantes.h"

#include "Inputs/SensorsManager.h"
#include "Managers/TestManager.h"
#include "Managers/BlumMachine.h"

#include "Outputs/Motor.h"
#include "Outputs/Buzzer.h"

#include "Serial/Messages.h"
#include "Serial/SerialManager.h"

Bounce2::Button btTest;
Bounce2::Button btWireless;
Bounce2::Button btCalibration;
Bounce2::Button btPilotage;

SensorsManager capteurs;

Buzzer buzzer;
Motor moteur(capteurs);

BlumMachine machine(capteurs, moteur, buzzer);
SerialManager portserie(capteurs, machine, buzzer);

uint32_t tVerif = 0;    constexpr uint32_t PERIODE_SECURITE = 1*1000UL;
uint32_t tBt = 0;       constexpr uint32_t PERIODE_BOUTONS = 50UL*1000UL;
uint32_t tMesure = 0;   constexpr uint32_t PERIODE_MESURES = 25UL*1000UL;
uint32_t tEnvoi = 0;
uint32_t tBno = 0;

bool is_time_securite = false;
bool has_send_accelero = false;

void lancerTest();
void ordonnanceur();

void configButton(Bounce2::Button& button, uint8_t pin) {
    button.attach(pin, INPUT_PULLUP);
    button.setPressedState(LOW);
}

void setup() {
    Led ledError = Led(LED_ERROR_PIN);
    
    portserie.init(); // Initialisation du port série
    buzzer.init(); // Initialiation du buzzer

    sendDirect(MSGTYPE::INFO, MSG::INFO_INIT_DEBUT);
    sendDirect(MSGTYPE::INFO, MSG::INFO_VERSION_SOFT, VERSION_SOFT);

    // Configuration des boutons
    configButton(btTest, TEST_BT);
    configButton(btWireless, WIRELESS_BT);
    configButton(btCalibration, CALIBRATION_BT);
    configButton(btPilotage, PILOTAGE_BT);

    capteurs.init(); // Initialisation des capteurs
    moteur.init(); // Initialisation du moteur et du driver
    machine.init(); // Initialisation de la machine à états
    ledError.init(); // Initialisation de la LED d'erreur

    // Mode DEV (message et envoi des mesures) sinon on n'envoit pas les mesures (à activer via le SET)
    #if VERSION_DEV
        sendDirect(MSGTYPE::WARNING, MSG::INFO_DEV);
        portserie.setMesurePeriode(1000); // Envoi des mesures toutes les secondes
    #else
        portserie.setMesurePeriode(0);
    #endif

    // Vérification codeur porte I2C
    bool send_error = false;  // Flag pour éviter d'afficher plusieurs fois l'erreur
    while (!capteurs.checkCodeurPorte()) {
        if (!send_error) {
            sendDirect(MSGTYPE::ERROR, MSG::ERR_CAPTEUR_ABSENT);
            ledError.on(); // Allume la LED d'erreur pour indiquer un problème avec le capteur 
            send_error = true;         
        }
        delay(1000);  // Petite pause pour éviter de saturer le CPU
    }
    sendDirect(MSGTYPE::INFO, MSG::INFO_CAPTEUR_PRESENT);
    ledError.off();

    // Vérification type de montage 
    capteurs.onMeuble() ? sendDirect(MSGTYPE::INFO, MSG::INFO_SUR_MEUBLE) : sendDirect(MSGTYPE::INFO, MSG::INFO_DEMONTE);

    // Vérification capteur BN0055
    capteurs.checkBNO() ? sendDirect(MSGTYPE::INFO, MSG::INFO_ACCEL_PRESENT): sendDirect(MSGTYPE::INFO, MSG::ERR_ACCEL_ABSENT);
    capteurs.bno().setEnabled(false); // Désactive le capteur pour économiser de l'énergie (il sera activé si nécessaire par la suite)
    
    // Vérification buzzer
    buzzer.isEnabled() ? sendDirect(MSGTYPE::INFO, MSG::INFO_BUZZER_ACTIVE) : sendDirect(MSGTYPE::INFO, MSG::INFO_BUZZER_DESACTIVE);
    
    sendDirect(MSGTYPE::INFO, MSG::INFO_INIT_FIN);
    buzzer.sequenceInit();
    delay(1000);

    // Initialisation des timers pour l'ordonnanceur
    tVerif = tMesure = tBt  = tEnvoi = tBno = micros();
    capteurs.updateSecurities(0); // Initialisation des sécurités
    capteurs.mesures(); // Initialise la 1er mesure
    if (capteurs.bno().isEnabled()) {
        capteurs.bno().requestData();
    }
    machine.start(); // Démarre la machine à états
}

void loop() {
    portserie.task(); // Vérifie les commandes recues
    if (portserie.testRequired()) {
        lancerTest();
    } else {
        machine.exec(); // Exécute la machine à états
        moteur.update(); // Met à jour l'état du moteur en fonction de la consigne et des capteurs
        capteurs.bno().update(); // Met à jour l'état du capteur BN0055 (accéléromètre)
        ordonnanceur(); // Exécute les tâches périodiques (mesures, sécurités, envois, boutons)
    }
    buzzer.task(); // Met à jour l'état du buzzer
}

void lancerTest() {
    machine.suspendre();
    TestManager(capteurs,buzzer).run();    // Bloquant jusqu'à DO STOP
    machine.reprendre();

    // Resynchronisation des timers pour éviter le rattrapage des cycles manqués
    tVerif = tMesure = tBt  = tEnvoi = tBno = millis();
    portserie.resync();

    // Évite qu'un appui pendant le test soit pris pour une nouvelle commande
    btTest.update(); btWireless.update(); btCalibration.update(); btPilotage.update();
}

void ordonnanceur() {
    // Récupération de la période d'envoi des mesures si changée
    uint32_t PERIODE_ENVOI = portserie.getMesurePeriode()*1000;
    uint32_t PERIODE_MESURE = min(PERIODE_ENVOI,PERIODE_MESURES);
    uint16_t pwm = moteur.getPWM();

    uint32_t maintenant = micros();

    // Gestion des sécurités (toutes les PERIODE_SECURITE)
    if (maintenant - tVerif >= PERIODE_SECURITE) {
        tVerif += PERIODE_SECURITE;   
        // Acquisition des sécurités
        capteurs.updateSecurities(pwm);
        // Acquisition des autres grandeurs (au maximum toutes les PERIODE_MESURE)
        if (maintenant - tMesure >= PERIODE_MESURE) {
            tMesure += PERIODE_MESURE;
            capteurs.mesures();
        }
        // Vérification des sécurités
        if (!machine.enCalibration()) {
            // On vérifie les sécurités uniquement si on n'est pas en calibration (géré côté calibration)
            if (capteurs.isLimiteAngle() && machine.enMouvement()) {
                if (machine.enPilotage()) {
                    // En pilotage, on ne devrait pas atteindre les limites
                    sendError(MSG::ERR_LIMITE_PORTE, capteurs.getAnglePorte());
                    machine.changerEtat(ETAT::ERREUR);
                    buzzer.sequenceErreur();
                } else {
                    // En fonctionnement normal, on passe en débrayage pour éviter d'endommager le système
                    sendInfo(MSG::ERR_LIMITE_PORTE, capteurs.getAnglePorte());
                    machine.changerEtat(ETAT::DEBRAYAGE);
                }
                capteurs.resetSecurities(); // requis pour éviter de répéter le message d'erreur
            } else {
                if (capteurs.isLimiteCourant()) {
                    // Limite de courant atteinte, sans avoir atteint les limites
                    sendError(MSG::ERR_COURANT);
                    machine.changerEtat(ETAT::ERREUR);
                    buzzer.sequenceErreur();
                }
                if (capteurs.isBlocage()){
                    // Blocage mécanique détecté, sans avoir atteint les limites
                    sendError(MSG::ERR_BLOCAGE);
                    machine.changerEtat(ETAT::ERREUR);
                    buzzer.sequenceErreur();
                }
            }
        }
        if (capteurs.meubleChanged()) {
            sendWarning(capteurs.onMeuble() ? MSG::INFO_SUR_MEUBLE : MSG::INFO_DEMONTE);
            machine.changerEtat(ETAT::DEBRAYAGE); // C'est la machine à état qui basculera vers l'état requis (calibration ou repos) après le débrayage
        }
    }

    // Envoies des mesures sur le port série
    if (PERIODE_ENVOI <= 1000UL*1000UL && maintenant - tEnvoi >= PERIODE_ENVOI) {
        tEnvoi += PERIODE_ENVOI;
        sendMesures(capteurs.getMesures());
        if (capteurs.bno().isEnabled() && maintenant - tBno >= 5 * PERIODE_ENVOI) {
            tBno += 5 * PERIODE_ENVOI;
            if (capteurs.bno().available()) {
                capteurs.bno().readData();
                sendMesures(capteurs.bno().getMesures());
            }
            capteurs.bno().requestData();
            has_send_accelero = true;
        } else {
            has_send_accelero = false;
        }
    } 
    if (!has_send_accelero) {
        sendEvents(); // Envoi des messages si pas d'envois des données de l'accéléromètre (pour éviter de saturer le port série)
    }

    // Vérification des boutons de commande (50ms)
    if (maintenant - tBt >= PERIODE_BOUTONS) {
        tBt += PERIODE_BOUTONS;
        
        btTest.update();
        btWireless.update();
        btCalibration.update();
        btPilotage.update();
        
        if (btCalibration.pressed())
            machine.calibrationBtPressed();
        if (btPilotage.pressed())
            machine.pilotageBtPressed();
        if (btWireless.pressed() || btTest.pressed())
            machine.actionBtPressed();
    }
}
