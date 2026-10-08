#pragma once

#include <Arduino.h>

#include "Consigne.h"

/**
 * @brief Gestionnaire des consignes de pilotage.
 *
 * La classe ConsigneManager assure la configuration, la sélection et
 * l'utilisation des différents types de consignes disponibles.
 *
 * Elle possède une instance de chaque type de consigne et maintient un
 * pointeur vers la consigne actuellement active. Ce mécanisme permet
 * d'utiliser les différentes consignes de manière polymorphique sans
 * recourir à l'allocation dynamique de mémoire.
 */

class ConsigneManager {
    public:
        ConsigneManager() : consigneActive(&consignePotentiometre) { } // Par défaut, la consigne active est la consigne potentiomètre
        bool setConsigne( const String& type, const String* params, int nbParams); // de configurer et sélectionner une consigne à partir de paramètres textuels
        void init(unsigned long t); // Initialiser la consigne active
        float get(unsigned long t); //Calculer sa valeur de la consigne à un instant donné
        bool ended(unsigned long t) const; // Déterminer si l'exécution de la consigne est terminée
        const char* getName() const; // Obtenir son nom
        bool hasConsigne() const { // Vérifie qu'une consigne est bien active
            return consigneActive != nullptr;
        }

    private:
        ConsignePotentiometre consignePotentiometre;
        ConsigneEchelon       consigneEchelon;
        ConsigneRampe         consigneRampe;
        ConsigneTrapeze       consigneTrapeze;
        ConsigneSinus         consigneSinus;

        Consigne* consigneActive = nullptr;
};