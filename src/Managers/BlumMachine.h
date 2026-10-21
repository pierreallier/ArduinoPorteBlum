#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <Arduino.h>
#include "StateMachine.h"

#include "../Serial/Messages.h"
#include "../Config/Etats.h"

#include "../Outputs/Led.h"

#include "CalibrationManager.h"
#include "PID.h"
#include "ConsigneManager.h"

class BlumMachine : public StateMachine<ETAT> {
    public:
        enum class MODE_PILOTAGE : uint8_t {
            PWM,
            POSITION,
            VITESSE,
            POSITION_VITESSE, // Asservissement en vitesse et position
        };

        MODE_PILOTAGE modePilotage;

        Led ledError = Led(LED_ERROR_PIN);

        bool butee_desactivated = false; // Variable qui spécifie si on doit désactiver la détection des butées
        
        PID pidPosition;
        PID pidVitesse;

        BlumMachine(SensorsManager& s, Motor& m, Buzzer& b);
        void init() override;
        void start();
        bool changerEtat(ETAT etat_demande);
        ETAT checkTransition(ETAT etat_demande) override;
        void execStateExit(ETAT etat) override;
        void execStateEntry(ETAT etat) override;
        bool execState(ETAT etat) override;
        void suspendre();   // Stoppe le fonctionnement avant le test
        void reprendre();   // Restaure le fonctionnement après le test

        void calibrationBtPressed() {
            if (_etat == ETAT::REPOS || _etat == ETAT::CALIBRATION_REQUIS) {
                changerEtat(ETAT::CALIBRATION);
            } else if (_etat == ETAT::CALIBRATION) {
                changerEtat(ETAT::DEBRAYAGE);
            }
        }
        void actionBtPressed() { (_etat == ETAT::REPOS) ? changerEtat(ETAT::FONCTIONNEMENT) : changerEtat(ETAT::DEBRAYAGE); }
        void pilotageBtPressed() { (_etat == ETAT::REPOS) ? changerEtat(ETAT::PILOTAGE) : changerEtat(ETAT::DEBRAYAGE);}

        void setMode(BlumMachine::MODE_PILOTAGE mode);

        void setPIDPosition(float kp, float ki, float kd) {
            pidPosition.setGains(kp, ki, kd);
        };
        void setPIDVitesse(float kp, float ki, float kd) {
            pidVitesse.setGains(kp, ki, kd);
        };
        bool setConsigne(const String& type, const String* params, int nbParams);

        

        inline bool isCalibrated() {return calibration.isCalibrated();}
        CalibrationManager& calibrationMachine() {return calibration;}
    
    private:
        
        Led ledPilotage = Led(LED_PILOTAGE_PIN);
        Led ledCalibration = Led(LED_CALIBRATION_PIN);

        CalibrationManager calibration;
        ConsigneManager consigne;
 
        bool etatOuverture(uint16_t speed);
        bool etatFermeture(uint16_t speed);
        bool etatPilote();        
};

#endif
