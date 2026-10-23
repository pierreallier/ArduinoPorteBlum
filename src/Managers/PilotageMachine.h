#pragma once

#include "../Config/Etats.h"
#include "StateMachine.h"
#include "PID.h"
#include "ConsigneManager.h"

class PilotageMachine : public StateMachine<ETAT_PILOTAGE> {
    public:
        enum class MODE : uint8_t {
            PWM,
            POSITION,
            VITESSE,
            POSITION_VITESSE, // Asservissement en vitesse et position
        };

        MODE modePilotage = MODE::PWM;

        PilotageMachine(SensorsManager& s, Motor& m, Buzzer& b);
        void init() override;
        void start();
        void stop();
        ETAT_PILOTAGE checkTransition(ETAT_PILOTAGE etat_demande) override;
        void execStateEntry(ETAT_PILOTAGE etat) override;
        void execStateExit(ETAT_PILOTAGE etat) override;
        void execState(ETAT_PILOTAGE etat) override;

        // Selection du mode de pilotage
        void setMode(PilotageMachine::MODE mode);
        void setMode(const String& modeStr);
        PilotageMachine::MODE getMode() const { return modePilotage; }

        // Correcteur et Consigne
        PID getPIDPosition() const { return pidPosition; }; 
        void setPIDPosition(float kp, float ki, float kd) {
            pidPosition.setGains(kp, ki, kd);
        };
        PID getPIDVitesse() const { return pidVitesse; };
        void setPIDVitesse(float kp, float ki, float kd) {
            pidVitesse.setGains(kp, ki, kd);
        };
        bool setConsigne(const String& type, const String* params, int nbParams);
        

    protected:
        PID pidPosition;
        PID pidVitesse;
        ConsigneManager consigne;
        Led ledPilotage = Led(LED_PILOTAGE_PIN);

        bool etatPilote(); // Gestion du pilotage de la porte via consigne

};