#pragma once
#include "../Inputs/SensorsManager.h"
#include "../Outputs/Motor.h"
#include "../Outputs/Buzzer.h"

template<typename Etat>
class StateMachine {
    public: 
        StateMachine(SensorsManager& s, Motor& m, Buzzer& b) : capteurs(s), moteur(m), buzzer(b) {};
        virtual ~StateMachine() = default;
        virtual void init() {};
        void start() { _time_etat = millis(); _activated = true;};
        void stop() { _activated = false;};
        void suspendre() {_activated = false;};  // Stoppe le fonctionnement de la machine à états
        void reprendre() {_activated = true;};   // Restaure le fonctionnement de la machine à états

        /* @brief vérifie la possibilité de changer d'état
         * @return False si aucun changement d'état effectué, True si l'état a été changé
         */
        bool changerEtat(Etat etat_demande) { 
            if (!_activated || _etat == etat_demande)
                return false;
            etat_demande = checkTransition(etat_demande);
            execStateExit(_etat);
            _etat = etat_demande;
            _time_etat = millis();
            execStateEntry(_etat);
            return true;
        };

        bool exec() { // Execution de la machine à états (actions de l'état courant)
            if (!_activated) return false;
            else return execState(_etat);
        };

        virtual Etat checkTransition(Etat etat_demande) { // Vérifie si la transition vers l'état demandé est possible
            return etat_demande;
        };
        virtual void execStateEntry(Etat etat) {}; // Actions à l'entrée de l'état courant
        virtual void execStateExit(Etat etat) {}; // Actions à la sortie de l'état courant
        virtual bool execState(Etat etat) = 0; // Actions à l'intérieur de l'état courant
    
        Etat getEtat() const { return _etat; };
    
    protected:
        SensorsManager& capteurs;
        Motor& moteur;
        Buzzer& buzzer;

        Etat _etat;
        uint32_t _time_etat = 0;
        bool _activated = false;
};
