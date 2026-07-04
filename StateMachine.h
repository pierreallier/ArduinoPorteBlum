#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <Arduino.h>
#include "Motor.h"
#include "Sensors.h"
#include "Pilotage.h"

struct Message {
    enum Type : uint8_t
    {
        ETAT,
        MESURE,
        INFO,
        ERREUR,
        AUCUN
    };

    Type type;
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
            ERREUR=8
        };

        ETAT etat;
        uint32_t time_etat = 0;

        StateMachine(Motor& m, Sensors& c);
        void init();
        void changerEtat(StateMachine::ETAT etat_demande);
        void exec();

        bool hasMessage() const;
        Message getMessage();
    
    private:
        Motor& moteur;
        Sensors& capteurs;
        Pilotage pilote;

        static constexpr uint8_t TAILLE_FIFO = 10;
        Message messages[TAILLE_FIFO];
        uint8_t tete = 0;
        uint8_t queue = 0;
        uint8_t nbElements = 0;

        bool pushMessage(Message cmd);
        bool popMessage(Message &cmd);
    
};

#endif