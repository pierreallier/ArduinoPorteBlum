#include "PID.h"

const float alpha = 0.15f; 

PID::PID(float kp = 0.0f, float ki = 0.0f, float kd = 0.0f) : kp(kp), ki(ki), kd(kd) {}

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
    if (dt <= 0.0f)
        return 0.0f;
    
    float erreur = consigne - mesure; // Erreur
    float derivee = -(mesure - mesurePrecedente) / dt; // Dérivée sur la mesure
    deriveeFiltree += alpha * (derivee - deriveeFiltree); // Filtre passe-bas sur la dérivée
    float integraleCandidate = integrale + erreur * dt; // Intégrale candidate
    
    // Bornage de l'intégrale
     if (ki * integraleCandidate > integraleMax)
        integraleCandidate = integraleMax;
    else if (ki * integraleCandidate < -integraleMax)
        integraleCandidate = -integraleMax;

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

