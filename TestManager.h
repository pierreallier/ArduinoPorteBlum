#ifndef TESTMANAGER_H
#define TESTMANAGER_H

#include <Arduino.h>
#include "Constantes.h"
#include "Messages.h"
#include "Buzzer.h"
#include "BN0055.h"
#include "MT6701.h"

class TestManager {
    // Teste les entrées et sorties du système, en mode bloquant (pas de multitâche)
    // Le test est interrompu si un message "DO STOP" ou "DO FIN_TEST" est reçu
    // Le test est lancé par un message "DO TEST" 
    public:
        TestManager(Buzzer& b, MT6701& codeurPorte, BN0055& bno) : buzzer(b), codeurPorte(codeurPorte), bno(bno) {}
        void run();   // Bloquant : retourne quand "DO STOP" (ou "DO FIN_TEST") est reçu

    private:
        Buzzer& buzzer;
        MT6701& codeurPorte;
        BN0055& bno;

        bool quitter = false;
        bool etatBoutons[4] = {false, false, false, false};
        uint32_t tEntrees = 0;

        bool attendre(uint32_t ms);   // Attente active avec service(), false si sortie demandée
        void service();               // Buzzer + entrées + TX série + commande de sortie
        void surveillerEntrees();
        void lireCommande();
        void testSorties();
        void testCapteurs();
        void testI2C();
        bool verifierBNO();
        bool verifierCodeur();
};

#endif