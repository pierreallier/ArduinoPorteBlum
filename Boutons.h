#ifndef BOUTONS_H
#define BOUTONS_H

#include <Arduino.h>

#define TEST_BT 2 // Bouton de mise en fonctionnement / arrêt
#define WIRELESS_BT 3 // Bouton sans fil 

void Boutons_Init();
bool Get_BoutonTest();
bool Get_BoutonSansFil();

void toogleBt();
void wirelessBt();


#endif