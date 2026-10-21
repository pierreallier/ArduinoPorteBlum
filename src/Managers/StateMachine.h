#pragma once
#include "../Config/Codes.h"
#include "../Inputs/SensorsManager.h"
#include "../Outputs/Motor.h"
#include "../Outputs/Buzzer.h"
#include "../Serial/Messages.h"

template<typename Etat>
class StateMachine {
    public: 
        StateMachine(SensorsManager& s, Motor& m, Buzzer& b) : capteurs(s), moteur(m), buzzer(b) {};
        virtual ~StateMachine() = default;
        virtual void init() {};
        void start() { 
            _time_etat = millis(); 
            _activated = true;
            execStateEntry(getEtat());
        };
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
            sendEtat(name,(uint32_t)_etat);
            return true;
        };

        bool exec() { // Execution de la machine à états (actions de l'état courant) - faux si ne s'execute pas
            if (!_activated) return false;
            execState(_etat);
            return true;
        };

        virtual Etat checkTransition(Etat etat_demande) { // Vérifie si la transition vers l'état demandé est possible
            return etat_demande;
        };
        virtual void execStateEntry(Etat etat) = 0; // Actions à l'entrée de l'état courant
        virtual void execStateExit(Etat etat) = 0; // Actions à la sortie de l'état courant
        virtual void execState(Etat etat) = 0; // Actions à l'intérieur de l'état courant
    
        Etat getEtat() const { return _etat; };
        void setEtat(Etat etat) { _etat = etat;};
        uint32_t getTimeEtat() const { return _time_etat; };
    
    protected:
        MSG name;
        SensorsManager& capteurs;
        Motor& moteur;
        Buzzer& buzzer;

        Etat _etat;
        uint32_t _time_etat = 0;
        bool _activated = false;
};
