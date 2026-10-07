#include <Arduino.h>
#include <Bounce2.h>
#include "Serial/Messages.h"
#include "Outputs/Buzzer.h"
#include "Inputs/BN0055.h"
#include "Inputs/MT6701.h"
#include "Managers/TestManager.h"

#include "Managers/StateMachine.h"
#include "Inputs/SensorsManager.h"
#include "Serial/SerialManager.h"
#include "Outputs/Motor.h"

#include "Config/Constantes.h"

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

void lancerTest();
void ordonnanceur();

void setup() {
    portserie.init(); // Initialisation du port série
    buzzer.init(); // Initialiation du buzzer
    buzzer.disable();

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

    // Mode
    #if VERSION_DEV
        sendDirect('W', "version de développement");
    #endif

    // Vérification codeur porte I2C
    bool send_error = false;  // Flag pour éviter d'afficher plusieurs fois l'erreur
    while (!capteurs.checkCodeurPorte()) {
        if (!send_error) {
            sendDirect('E', "codeur absolu de la porte non détecté");
            digitalWrite(LED_ERROR_PIN, HIGH);  
            send_error = true;         
        }
        delay(1000);  // Petite pause pour éviter de saturer le CPU
    }
    sendDirect('I', "codeur absolu de la porte détecté");
    digitalWrite(LED_ERROR_PIN, LOW);

    // Vérification type de montage 
    capteurs.onMeuble() ? sendDirect('I', "servodrive monté sur le meuble") : sendDirect('I', "servodrive non monté sur le meuble");
    // Vérification système calibré
    if (!machine.isCalibrated()) {
        sendDirect('W', "calibration requise");
    }

    // Vérification capteur BN0055
    capteurs.checkBNO() ? sendDirect('I', "capteur BN0055 détecté"): sendDirect('I', "capteur BN0055 non détecté");
    
    // Vérificationb buzzer
    buzzer.isEnabled() ? sendDirect('I', "Buzzer activé") : sendDirect('I', "Buzzer désactivé");
    
    portserie.printFinInit();
    buzzer.sequenceInit();
    delay(1000);

    capteurs.mesures(0); // Initialise la 1er mesure
}

void loop() {
    portserie.task();
    if (portserie.demandeTest) {
        portserie.demandeTest = false;
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

    // Envoies des mesures sur le port série
    if (PERIODE_ENVOI <= 1000 && maintenant - tEnvoi >= PERIODE_ENVOI) {
        tEnvoi += PERIODE_ENVOI;
        sendMesures(capteurs.getMesures());
        if (capteurs.bno().isEnabled() && maintenant - tBno >= 5 * PERIODE_ENVOI) {
            tBno += 5 * PERIODE_ENVOI;
            capteurs.bno().readData();
            sendMesures(capteurs.bno().getMesures());
            capteurs.bno().requestData(maintenant);
        }
    } 

    // Vérifications des sécurités 
    if (is_time_securite) {
        is_time_securite = false;
        if (machine.etat != StateMachine::ETAT::CALIBRATION) {
            if (capteurs.isLimiteAngle() && (machine.etat != StateMachine::ETAT::DEBRAYAGE && machine.etat != StateMachine::ETAT::REPOS)) {
                if (machine.etat == StateMachine::ETAT::PILOTAGE) {
                    sendError(MSG::ERR_LIMITE_PORTE, capteurs.getAnglePorte());
                    machine.changerEtat(StateMachine::ETAT::ERREUR);
                    buzzer.sequenceErreur();
                } else {
                    sendInfo(MSG::ERR_LIMITE_PORTE, capteurs.getAnglePorte());
                    machine.changerEtat(StateMachine::ETAT::DEBRAYAGE);
                }
                capteurs.resetSecurities();
            } else {
                if (capteurs.isLimiteCourant()) {
                    sendError(MSG::ERR_COURANT);
                    machine.changerEtat(StateMachine::ETAT::ERREUR);
                    buzzer.sequenceErreur();
                }
                if (capteurs.isBlocage()){
                    sendError(MSG::ERR_BLOCAGE);
                    machine.changerEtat(StateMachine::ETAT::ERREUR);
                    buzzer.sequenceErreur();
                }
                if (capteurs.meubleChanged()) {
                        sendWarning(capteurs.onMeuble() ? MSG::INFO_SUR_MEUBLE : MSG::INFO_DEMONTE);
                    if (!machine.isCalibrated()) {
                        sendWarning(MSG::ERR_CALIBRATION_REQUISE);
                    }
                    if (machine.etat != StateMachine::ETAT::REPOS) {
                        machine.changerEtat(StateMachine::ETAT::ERREUR);
                    }
                }
            }
        }
    }

    // Vérification des boutons de commande (50ms)
    if (maintenant - tBt >= 50) {
        tBt += 50;
        
        btTest.update();
        btWireless.update();
        btCalibration.update();
        btPilotage.update();
        if (btCalibration.pressed()) {
            if (machine.etat == StateMachine::ETAT::REPOS) {
                machine.changerEtat(StateMachine::ETAT::CALIBRATION);
            }
            else
                machine.changerEtat(StateMachine::ETAT::DEBRAYAGE);
        }
        if (btPilotage.pressed()) {
            if (machine.etat == StateMachine::ETAT::REPOS) {
                machine.changerEtat(StateMachine::ETAT::PILOTAGE);
            }
            else
                machine.changerEtat(StateMachine::ETAT::DEBRAYAGE);
        }
        if (btWireless.pressed() || btTest.pressed()) {
            if (machine.etat == StateMachine::ETAT::REPOS)
                machine.changerEtat(StateMachine::ETAT::FONCTIONNEMENT);
            else
                machine.changerEtat(StateMachine::ETAT::DEBRAYAGE);
        }
    }
}