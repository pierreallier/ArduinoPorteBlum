#ifndef PID_H
#define PID_H

#include <Arduino.h>

class PID {
    public:
        PID(float kp = 1.0f, float ki = 0.0f, float kd = 0.0f);
        void setGains(float kp, float ki, float kd);
        void getGain();
        void setOutputLimits(float min, float max);
        void setIntegraleLimit(float limite);
        void reset();
        float compute(float consigne, float mesure, unsigned long time);

        float kp, ki, kd;

    private:
        float integrale = 0.0f;
        float erreurPrecedente = 0.0f;
        float mesurePrecedente = 0.0f;
        float deriveeFiltree = 0.0f;
        float min = -255.0f;
        float max = 255.0f;
        float integraleMax = 100.0f;
        unsigned long tempsPrecedent = 0;
        bool premierCalcul = true;
};

#endif