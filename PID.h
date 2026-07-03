#ifndef PID_H
#define PID_H

class PID {
public:
    void setGains(float kp, float ki, float kd);

    void setOutputLimits(float min, float max);

    void reset();

    float compute(float consigne, float mesure, float dt);

private:
    float kp, ki, kd;

    float integrale;

    float erreurPrecedente;
};

#endif