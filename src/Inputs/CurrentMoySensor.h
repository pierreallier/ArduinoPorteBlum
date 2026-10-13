#ifndef CURRENTMOYSENSOR_H
#define CURRENTMOYSENSOR_H

#include <Arduino.h>
#include "Sensor.h"

/* 
 * Classe pour la lecture du courant moyen à partir d'un capteur de courant analogique.
 * La classe utilise une moyenne glissante pour lisser les variations de lecture.
 * 
 * Exemple d'utilisation :
 * CurrentMoySensor courantSensor(MOTOR_CURRENT_PIN);
 * courantSensor.setLimit(2.0f); // Limite de courant à 2.0A
 * int16_t current = courantSensor.read(); // Lecture du courant moyen en A*100
 * bool isLimitReached = courantSensor.isLimitReached(); // Vérifie si la limite est atteinte
 * 
 */

class CurrentMoySensor : public Sensor<uint16_t> {
    public:
        CurrentMoySensor(uint8_t pin, float coeff, float limit): Sensor<uint16_t>(pin, coeff), _limit(limit) {
        }

        void init() {
            // Calibrage de l'offset : moyenne de quelques lectures brutes
            uint32_t somme = 0;
            for (uint16_t i = 0; i < 500; i++) {
                somme += Sensor<uint16_t>::readRaw();
            }
            _offset = somme / 500.0f;

            // Initialiser la table de mesures avec la première lecture compensée
            uint16_t initial = Sensor<uint16_t>::readRaw();
            for (uint8_t i = 0; i < NB_MESURES; i++) {
                _table[i] = initial;
            }
            _currentSum = initial * NB_MESURES;

            setLimit(_limit); // Définir la limite de courant en valeur brute
        }

        float read() {
            _value = Sensor<uint16_t>::readRaw(); // Lecture du capteur brut
            add(_value); // Lecture du capteur brut et mise à jour de la moyenne glissante
            return get();
        }

        float get() const { // Retourne la dernière valeur du courant
            return (_value - _offset) * _coeff;
        }

        inline float getMoy() const { // Retourne le courant moyen
            return (_currentSum  / static_cast<float>(NB_MESURES) - _offset) * _coeff;
        } 

        inline bool isLimitReached() const { return _currentSum >= _limit; }
        inline void setLimit(float limit) { _limit = NB_MESURES * (_offset + limit / _coeff); }
        inline float getLimit() const { return (_limit / static_cast<float>(NB_MESURES) - _offset) * _coeff; }

        
    private:
        static constexpr uint8_t NB_MESURES = 5; // Nombre de mesures par défaut pour la moyenne glissante
        float _limit = 0; // Limite de courant en valeur brute (sans offset ni coeff)
        uint32_t _currentSum = 0; // Stocke la somme des dernière mesures brutes (sans offset ni coeff)
        uint8_t _idx = 0;
        uint16_t _table[NB_MESURES];

        void add(float current) {
            _currentSum -= _table[_idx];
            _currentSum += current;
            _table[_idx] = current;
            _idx = (_idx + 1) % NB_MESURES;
        }
};

#endif