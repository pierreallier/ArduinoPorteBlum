#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <Arduino.h>
#include "StateMachine.h"
#include "PilotageMachine.h"
#include "CalibrationManager.h"

#include "../Serial/Messages.h"
#include "../Config/Etats.h"

#include "../Outputs/Led.h"

class BlumMachine : public StateMachine<ETAT> {
    public:

        Led ledError = Led(LED_ERROR_PIN);

        BlumMachine(SensorsManager& s, Motor& m, Buzzer& b);
        void init() override;
        void start();
        ETAT checkTransition(ETAT etat_demande) override;
        void execStateExit(ETAT etat) override;
        void execStateEntry(ETAT etat) override;
        void execState(ETAT etat) override;
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

        bool enCalibration() { return _etat == ETAT::CALIBRATION; }
        bool enPilotage() { return _etat == ETAT::PILOTAGE; }
        bool enMouvement() { return _etat == ETAT::FONCTIONNEMENT || _etat == ETAT::OUVERTURE || _etat == ETAT::FERMETURE || _etat == ETAT::PILOTAGE || _etat == ETAT::CALIBRATION; }

        inline bool isCalibrated() {return calibration.isCalibrated();}
        CalibrationManager& calibrationMachine() {return calibration;}
        PilotageMachine& pilotageMachine() {return pilotage;}
    
    private:
        Led ledCalibration = Led(LED_CALIBRATION_PIN);

        CalibrationManager calibration;
        ConsigneManager consigne;
        PilotageMachine pilotage;     
};

#endif
