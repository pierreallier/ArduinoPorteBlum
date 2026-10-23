#include "PID.h"

const float alpha = 0.15f; 

PID::PID(float kp, float ki, float kd) : kp(kp), ki(ki), kd(kd) {}

void PID::setGains(float kp, float ki, float kd) {
    this->kp = kp;
    this->ki = ki;
    this->kd = kd;
}

void PID::setOutputLimits(float min, float max) {
    this->min = min;
    this->max = max;
}

void PID::setIntegraleLimit(float limite) {
    integraleMax = abs(limite);
}

void PID::reset() {
    integrale = 0.0f;
    erreurPrecedente = 0.0f;
    tempsPrecedent = 0;
    mesurePrecedente = 0.0f;
    deriveeFiltree = 0.0f;
    premierCalcul = true;
}

float PID::compute(float consigne, float mesure, unsigned long time) {
    if (premierCalcul) {
        tempsPrecedent = time;
        mesurePrecedente = mesure;
        premierCalcul = false;
        return constrain(kp * (consigne - mesure),min,max);
    }
    unsigned long dt = time - tempsPrecedent;
    if (dt == 0)
        return 0.0f;
    
    float erreur = consigne - mesure; // Erreur
    float derivee = -(mesure - mesurePrecedente) / dt * 1000000.0f; // Dérivée sur la mesure (dt en microsecondes)
    deriveeFiltree += alpha * (derivee - deriveeFiltree); // Filtre passe-bas sur la dérivée
    float integraleCandidate = integrale + erreur * dt * 1e-6f; // Intégrale candidate (dt en microsecondes)
    
    // Bornage de l'intégrale
    if (ki != 0.0f) {
        if (ki * integraleCandidate > integraleMax)
            integraleCandidate = integraleMax;
        else if (ki != 0.0f && ki * integraleCandidate < -integraleMax)
            integraleCandidate = -integraleMax;
    }
    else {
        integraleCandidate = 0.0f; // Si ki est nul, on ne veut pas d'intégrale
    }
    if (kd == 0.0f) {
        deriveeFiltree = 0.0f; // Si kd est nul, on ne veut pas de dérivée
    }

    float sortie = kp * erreur + ki * integraleCandidate + kd * deriveeFiltree; // Calcul de la sortie
    
    // Saturation + anti-windup
    if (sortie > max)
        sortie = max;
    else if (sortie < min)
        sortie = min;
    else
        integrale = integraleCandidate;
    // Sauvegarde
    erreurPrecedente = erreur;
    mesurePrecedente = mesure;
    tempsPrecedent = time;
    return sortie;
}

