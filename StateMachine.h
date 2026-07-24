#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <Arduino.h>
#include "Messages.h"

#include "Motor.h"
#include "Sensors.h"
#include "PID.h"
#include "ConsigneManager.h"

const uint16_t PWM_CALIBRATION = 150; 

class StateMachine {
    public:
        enum class ETAT : byte {
            INIT,
            REPOS,
            FONCTIONNEMENT,
            OUVERTURE,
            FERMETURE,
            PILOTAGE,
            CALIBRATION,
            DEBRAYAGE,
            ERREUR,
            STOP,
            NB_ETATS
        };
        
        static const char* const ETAT_NAMES[static_cast<size_t>(ETAT::NB_ETATS)]; // Tableau des noms (même ordre que l'enum ETAT)

        enum class MODE_PILOTAGE : byte {
            PWM,
            POSITION,
            VITESSE,
            POSITION_VITESSE, // Asservissement en vitesse et position
        };

        ETAT etat;
        MODE_PILOTAGE modePilotage;
        uint32_t time_etat = 0;

        bool butee_desactivated = false; // Variable qui spécifie si on doit désactiver la détection des butées
        bool is_calibre;

        PID pidPosition;
        PID pidVitesse;

        StateMachine(Motor& m, Sensors& c);
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
        void setModePilotage(StateMachine::MODE_PILOTAGE mode) {
            modePilotage = mode;
        }
        bool setConsigne(const String& type, const String* params, int nbParams);

    
    private:
        Motor& moteur;
        Sensors& capteurs;
        ConsigneManager consigne;

        enum class ETAPE_CALIBRATION : uint8_t {
            OUVERTURE_INITIALE,
            ATTENTE_HAUT,
            DEBRAYAGE_HAUT,
            RECHERCHE_BUTEE_BASSE,
            ATTENTE_BAS,
            DEBRAYAGE_BAS,
            RECHERCHE_BUTEE_HAUTE,
            ATTENTE_ENREGISTREMENT,
            DEBRAYAGE_FINAL,
            NONE,
        };
        unsigned long time_etape_calibration = 0;
        ETAPE_CALIBRATION etape_calibration;
        float angle_butee_basse;
        float angle_butee_haute;
        
        void changerEtapeCalibration(StateMachine::ETAPE_CALIBRATION nouvelle_etape);
        
        bool etatOuverture(uint16_t speed);
        bool etatFermeture(uint16_t speed);
        bool etatDebrayage();
        bool etatPilote();
        bool etatCalibration();        
};

#endif