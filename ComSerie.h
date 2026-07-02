#ifndef COMSERIE_H
#define COMSERIE_H

#include <Arduino.h>
#include "Sensors.h"
#include "StateMachine.h"
#include "Motor.h"

extern Mesures mesures;

void comSerie_Init();
void comSerie_Task();
void comSerie_PrintMesures();
void comSerie_GetCommandes();

// interne
void comSerie_SET(String commande);
void comSerie_GET(String commande);
void comSerie_DO(String commande);

#endif