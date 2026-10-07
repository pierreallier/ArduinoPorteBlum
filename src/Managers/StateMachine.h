#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <Arduino.h>
#include "../Serial/Messages.h"

#include "../Outputs/Motor.h"
#include "../Inputs/SensorsManager.h"
#include "CalibrationManager.h"
#include "PID.h"
#include "ConsigneManager.h"


//const uint16_t PWM_CALIBRATION = 150; 

class StateMachine {
    public:
        enum class ETAT : uint8_t {
            INIT,
            REPOS,
            FONCTIONNEMENT,
            OUVERTURE,
            FERMETURE,
            PILOTAGE,
            CALIBRATION,
            DEBRAYAGE,
            ERREUR,
            STOP
        };
        
        enum class MODE_PILOTAGE : uint8_t {
            PWM,
            POSITION,
            VITESSE,
            POSITION_VITESSE, // Asservissement en vitesse et position
        };

        ETAT etat;
        MODE_PILOTAGE modePilotage;
        uint32_t time_etat = 0;

        bool butee_desactivated = false; // Variable qui spécifie si on doit désactiver la détection des butées
        bool is_calibre();

        PID pidPosition;
        PID pidVitesse;

        StateMachine(Motor& m, SensorsManager& s);
        void init();
        void changerEtat(StateMachine::ETAT etat_demande);
        void setMode(StateMachine::MODE_PILOTAGE mode);
        void exec();

        void setPIDPosition(float kp, float ki, float kd) {
            pidPosition.setGains(kp, ki, kd);
        };
        void setPIDVitesse(float kp, float ki, float kd) {
            pidVitesse.setGains(kp, ki, kd);
        };
        bool setConsigne(const String& type, const String* params, int nbParams);

        void suspendre();   // Stoppe le fonctionnement avant le test
        void reprendre();   // Restaure le fonctionnement après le test

        CalibrationManager calibration;
    
    private:
        Motor& moteur;
        SensorsManager& capteurs;
        ConsigneManager consigne;
 
        bool etatOuverture(uint16_t speed);
        bool etatFermeture(uint16_t speed);
        bool etatDebrayage();
        bool etatPilote();        
};

#endif