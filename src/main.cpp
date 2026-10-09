#include <Arduino.h>
#include <Bounce2.h>

#include "Config/Constantes.h"

#include "Inputs/SensorsManager.h"
#include "Managers/TestManager.h"
#include "Managers/StateMachine.h"

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

StateMachine machine(capteurs, moteur, buzzer);
SerialManager portserie(capteurs, machine, buzzer);

uint32_t tVerif = 0;
uint32_t tBt = 0;
uint32_t tMesure = 0;
uint32_t tEnvoi = 0;
uint32_t tBno = 0;

bool is_time_securite = false;
bool has_send_accelero = false;

void lancerTest();
void ordonnanceur();

void setup() {
    portserie.init(); // Initialisation du port série
    buzzer.init(); // Initialiation du buzzer

    sendDirect(MSGTYPE::INFO, MSG::INFO_INIT_DEBUT);

    // Configuration des boutons avec résistance de pull-up interne et état actif à LOW
    btTest.attach(TEST_BT,INPUT_PULLUP);
    btTest.setPressedState(LOW); 
    btWireless.attach(WIRELESS_BT,INPUT_PULLUP);
    btWireless.setPressedState(LOW);
    btCalibration.attach(CALIBRATION_BT,INPUT_PULLUP);
    btCalibration.setPressedState(LOW);
    btPilotage.attach(PILOTAGE_BT,INPUT_PULLUP);
    btPilotage.setPressedState(LOW);

    capteurs.init(); // Initialisation des capteurs
    moteur.init(); // Initialisation du moteur et du driver
    machine.init(); // Initialisation de la machine à états

    // Mode DEV (message et envoi des mesures) sinon on n'envoi pas les mesures (à activer via le SET)
    #if VERSION_DEV
        sendDirect(MSGTYPE::WARNING, MSG::INFO_DEV);
    #else
        portserie.setMesurePeriode(0);
    #endif

    // Vérification codeur porte I2C
    bool send_error = false;  // Flag pour éviter d'afficher plusieurs fois l'erreur
    while (!capteurs.checkCodeurPorte()) {
        if (!send_error) {
            sendDirect(MSGTYPE::ERROR, MSG::ERR_CAPTEUR_ABSENT);
            digitalWrite(LED_ERROR_PIN, HIGH);  
            send_error = true;         
        }
        delay(1000);  // Petite pause pour éviter de saturer le CPU
    }
    sendDirect(MSGTYPE::INFO, MSG::INFO_CAPTEUR_PRESENT);
    digitalWrite(LED_ERROR_PIN, LOW);

    // Vérification type de montage 
    capteurs.onMeuble() ? sendDirect(MSGTYPE::INFO, MSG::INFO_SUR_MEUBLE) : sendDirect(MSGTYPE::INFO, MSG::INFO_DEMONTE);
    // Vérification système calibré
    if (!machine.isCalibrated()) {
        sendDirect(MSGTYPE::WARNING, MSG::ERR_CALIBRATION_REQUISE);
    }

    // Vérification capteur BN0055
    capteurs.checkBNO() ? sendDirect(MSGTYPE::INFO, MSG::INFO_CAPTEUR_PRESENT): sendDirect(MSGTYPE::ERROR, MSG::ERR_CAPTEUR_ABSENT);
    
    // Vérification buzzer
    buzzer.isEnabled() ? sendDirect(MSGTYPE::INFO, MSG::INFO_BUZZER_ACTIVE) : sendDirect(MSGTYPE::INFO, MSG::INFO_BUZZER_DESACTIVE);
    
    sendDirect(MSGTYPE::INFO, MSG::INFO_INIT_FIN);
    buzzer.sequenceInit();
    delay(1000);

    capteurs.mesures(0); // Initialise la 1er mesure
}

void loop() {
    portserie.task();
    if (portserie.testRequired()) {
        lancerTest();
    } else {
        machine.exec();
        moteur.update();
        capteurs.bno().update();
        ordonnanceur();
    }
    buzzer.task();
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
    uint32_t PERIODE_ENVOI = portserie.getMesurePeriode();
    uint32_t PERIODE_MESURE = min(PERIODE_ENVOI,25);
    float pwm = moteur.getPWM();

    // Tâches périodiques
    uint32_t maintenant = millis();

    // Mesures - Sécurités (toutes les 5 ms)
    if (maintenant - tVerif >= 5) {
        tVerif += 5;   
        // Vérifications des sécurités
        capteurs.updateSecurities(pwm);
        is_time_securite = true;
    }
    // Mesures des autres grandeurs (au maximum toutes les 25 ms) pour conserver le même timestamps que les sécurités
    if (maintenant - tMesure >= PERIODE_MESURE) {
        tMesure += PERIODE_MESURE;
        capteurs.mesures(pwm);
    }

    // Vérifications des sécurités 
    if (is_time_securite) {
        is_time_securite = false;
        if (machine.etat != ETAT::CALIBRATION) {
            if (capteurs.isLimiteAngle() && (machine.etat != ETAT::DEBRAYAGE && machine.etat != ETAT::REPOS)) {
                if (machine.etat == ETAT::PILOTAGE) {
                    sendError(MSG::ERR_LIMITE_PORTE, capteurs.getAnglePorte());
                    machine.changerEtat(ETAT::ERREUR);
                    buzzer.sequenceErreur();
                } else {
                    sendInfo(MSG::ERR_LIMITE_PORTE, capteurs.getAnglePorte());
                    machine.changerEtat(ETAT::DEBRAYAGE);
                }
                capteurs.resetSecurities();
            } else {
                if (capteurs.isLimiteCourant()) {
                    sendError(MSG::ERR_COURANT);
                    machine.changerEtat(ETAT::ERREUR);
                    buzzer.sequenceErreur();
                }
                if (capteurs.isBlocage()){
                    sendError(MSG::ERR_BLOCAGE);
                    machine.changerEtat(ETAT::ERREUR);
                    buzzer.sequenceErreur();
                }
                if (capteurs.meubleChanged()) {
                        sendWarning(capteurs.onMeuble() ? MSG::INFO_SUR_MEUBLE : MSG::INFO_DEMONTE);
                    if (!machine.isCalibrated()) {
                        sendWarning(MSG::ERR_CALIBRATION_REQUISE);
                    }
                    if (machine.etat != ETAT::REPOS) {
                        machine.changerEtat(ETAT::ERREUR);
                    }
                }
            }
        }
    }

    // Envoies des mesures sur le port série
    if (PERIODE_ENVOI <= 1000 && maintenant - tEnvoi >= PERIODE_ENVOI) {
        tEnvoi += PERIODE_ENVOI;
        sendMesures(capteurs.getMesures());
        if (capteurs.bno().isEnabled() && maintenant - tBno >= 5 * PERIODE_ENVOI) {
            tBno += 5 * PERIODE_ENVOI;
            capteurs.bno().readData();
            sendMesures(capteurs.bno().getMesures());
            capteurs.bno().requestData(maintenant);
            has_send_accelero = true;
        } else {
            has_send_accelero = false;
        }
    } 
    if (!has_send_accelero) {
        sendEvents(); // Envoi des messages si pas d'envois des données de l'accéléromètre (pour éviter de saturer le port série)
    }

    // Vérification des boutons de commande (50ms)
    if (maintenant - tBt >= 50) {
        tBt += 50;
        
        btTest.update();
        btWireless.update();
        btCalibration.update();
        btPilotage.update();
        if (btCalibration.pressed()) {
            if (machine.etat == ETAT::REPOS) {
                machine.changerEtat(ETAT::CALIBRATION);
            }
            else
                machine.changerEtat(ETAT::DEBRAYAGE);
        }
        if (btPilotage.pressed()) {
            if (machine.etat == ETAT::REPOS) {
                machine.changerEtat(ETAT::PILOTAGE);
            }
            else
                machine.changerEtat(ETAT::DEBRAYAGE);
        }
        if (btWireless.pressed() || btTest.pressed()) {
            if (machine.etat == ETAT::REPOS)
                machine.changerEtat(ETAT::FONCTIONNEMENT);
            else
                machine.changerEtat(ETAT::DEBRAYAGE);
        }
    }
}