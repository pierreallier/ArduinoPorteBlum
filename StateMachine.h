#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <Arduino.h>
#include "Motor.h"
#include "Sensors.h"
#include "PID.h"
#include "Consigne.h"

struct Message {
    enum TYPE : uint8_t
    {
        ETAT,
        MESURE,
        INFO,
        ERREUR,
        AUCUN,
    };

    TYPE type;
    String valeur;
};

class StateMachine {
    public:
        enum class ETAT : byte {
            INIT=0,
            REPOS=1,
            FONCTIONNEMENT=2,
            OUVERTURE=3,
            FERMETURE=4,
            PILOTAGE=5,
            CALIBRATION=6,
            DEBRAYAGE=7,
            ERREUR=8,
            STOP=9,
        };

        enum class MODE_PILOTAGE : byte {
            PWM,
            POSITION,
            VITESSE,
            POSITION_VITESSE, // Asservissement en vitesse et position
        };

        ETAT etat;
        MODE_PILOTAGE modePilotage;
        uint32_t time_etat = 0;

        PID pidPosition;
        PID pidVitesse;

        StateMachine(Motor& m, Sensors& c);
        void init();
        void changerEtat(StateMachine::ETAT etat_demande);
        void setMode(StateMachine::MODE_PILOTAGE mode);
        void setConsigne(Consigne& c);
        void exec();
        void setPIDPosition(float kp, float ki, float kd) {
            pidPosition.setGains(kp, ki, kd);
        };
        void setPIDVitesse(float kp, float ki, float kd) {
            pidVitesse.setGains(kp, ki, kd);
        };

        bool hasMessage() const;
        Message getMessage();
    
    private:
        Motor& moteur;
        Sensors& capteurs;
        ConsignePotentiometre consignePotentiometre;
        Consigne* consigne = nullptr;

        void etatPilote(float consigne, unsigned long time); 
        
        static constexpr uint8_t TAILLE_FIFO = 10;
        Message messages[TAILLE_FIFO];
        uint8_t tete = 0;
        uint8_t queue = 0;
        uint8_t nbElements = 0;

        bool pushMessage(Message::TYPE t, String message);
        bool popMessage(Message &cmd);

           
};

#endif