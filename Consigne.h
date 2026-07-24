#pragma once

#include <Arduino.h>
#include "Constantes.h"

/**
 * @brief Classe abstraite représentant une consigne de pilotage.
 *
 * Définit l'interface commune à tous les types de consignes.
 */
class Consigne {
    public:
        virtual ~Consigne() = default;

        virtual void init(unsigned long t) {
            t_init = t;
        }

        virtual float get(unsigned long t) = 0;
        virtual bool ended(unsigned long t) const = 0;
        virtual const char* getName() const = 0;

    protected:
        unsigned long t_init = 0;
};

/**
 * @brief Consigne définie par la position d'un potentiomètre.
 *
 * Cette consigne n'a pas de durée limitée et ne se termine jamais.
 */
class ConsignePotentiometre : public Consigne {
    public:
        void init(unsigned long t) override {
            Consigne::init(t);
            pinMode(POTENTIOMETRE, INPUT);
        }

        float get(unsigned long) override {
            return (analogRead(POTENTIOMETRE) - 500) * 0.5f;
        }

        bool ended(unsigned long) const override {
            return false;
        }

        const char* getName() const override {
            return "POTENTIOMETRE";
        }
};

/**
 * @brief Consigne de type échelon.
 *
 * Après un délai optionnel, la consigne prend une valeur constante
 * pendant une durée déterminée, puis revient à zéro.
 */
class ConsigneEchelon : public Consigne {
    public:
        void configure(float valeur, unsigned long duree, unsigned long delai = 0) {
            this->valeur = valeur;
            this->duree = duree;
            this->delai = delai;
        }

        float get(unsigned long t) override {
            unsigned long dt = t - t_init;
            if (dt < delai)
                return 0.0f;
            dt -= delai;
            if (dt < duree)
                return valeur;
            return 0.0f;
        }

        bool ended(unsigned long t) const override {
            return (t - t_init >= delai + duree);
        }

        const char* getName() const override {
            return "ECHELON";
        }

    private:
        float valeur = 0.0f;
        unsigned long duree = 0;
        unsigned long delai = 0;
};

/**
 * @brief Consigne de type rampe linéaire.
 *
 * Après un délai optionnel, la consigne évolue linéairement entre
 * une valeur initiale et une valeur finale pendant une durée déterminée.
 */
class ConsigneRampe : public Consigne {
    public:
        void configure(float valeurInitiale, float valeurFinale, unsigned long duree, unsigned long delai = 0) {
            this->valeurInitiale = valeurInitiale;
            this->valeurFinale = valeurFinale;
            this->duree = duree;
            this->delai = delai;
        }

        float get(unsigned long t) override {
            unsigned long dt = t - t_init;
            if (dt < delai)
                return valeurInitiale;
            dt -= delai;
            if (duree == 0 || dt >= duree)
                return valeurFinale;
            float progression = static_cast<float>(dt) / static_cast<float>(duree);
            return valeurInitiale + progression * (valeurFinale - valeurInitiale);
        }

        bool ended(unsigned long t) const override {
            return (t - t_init >= delai + duree);
        }

        const char* getName() const override {
            return "RAMPE";
        }

    private:
        float valeurInitiale = 0.0f;
        float valeurFinale = 0.0f;
        unsigned long duree = 0;
        unsigned long delai = 0;
};

/**
 * @brief Consigne de type trapèze.
 *
 * Après un délai optionnel, la consigne comporte une montée linéaire,
 * un plateau, puis une descente linéaire jusqu'à zéro.
 */
class ConsigneTrapeze : public Consigne {
    public:
        void configure(float valeur, unsigned long dureeMontee, unsigned long dureePlateau, unsigned long dureeDescente, unsigned long delai = 0) {
            this->valeur = valeur;
            this->dureeMontee = dureeMontee;
            this->dureePlateau = dureePlateau;
            this->dureeDescente = dureeDescente;
            this->delai = delai;
        }

        float get(unsigned long t) override {
            unsigned long dt = t - t_init;
            if (dt < delai)
                return 0.0f;
            dt -= delai;
            if (dt < dureeMontee) {
                if (dureeMontee == 0)
                    return valeur;
                float progression = static_cast<float>(dt) / static_cast<float>(dureeMontee);
                return valeur * progression;
            }
            dt -= dureeMontee;
            if (dt < dureePlateau)
                return valeur;
            dt -= dureePlateau;
            if (dt < dureeDescente) {
                if (dureeDescente == 0)
                    return 0.0f;
                float progression = static_cast<float>(dt) / static_cast<float>(dureeDescente);
                return valeur * (1.0f - progression);
            }
            return 0.0f;
        }

        bool ended(unsigned long t) const override {
            return (t - t_init >= delai + dureeMontee + dureePlateau + dureeDescente);
        }

        const char* getName() const override {
            return "TRAPEZE";
        }

    private:
        float valeur = 0.0f;
        unsigned long dureeMontee = 0;
        unsigned long dureePlateau = 0;
        unsigned long dureeDescente = 0;
        unsigned long delai = 0;
};

/**
 * @brief Consigne sinusoïdale.
 *
 * La consigne évolue suivant une fonction sinusoïdale définie par
 * son amplitude, sa période, sa durée totale et son offset.
 */
class ConsigneSinus : public Consigne {
    public:
        void configure(float amplitude, unsigned long periode, unsigned long duree, float offset = 0.0f) {
            this->amplitude = amplitude;
            this->periode = periode;
            this->duree = duree;
            this->offset = offset;
        }

        float get(unsigned long t) override {
            unsigned long dt = t - t_init;
            if (dt >= duree)
                return 0.0f;
            if (periode == 0)
                return offset;
            float angle = 2.0f * PI * static_cast<float>(dt) / static_cast<float>(periode);
            return offset + amplitude * sin(angle);
        }

        bool ended(unsigned long t) const override {
            return (t - t_init >= duree);
        }

        const char* getName() const override {
            return "SINUS";
        }

    private:
        float amplitude = 0.0f;
        float offset = 0.0f;
        unsigned long periode = 0;
        unsigned long duree = 0;
};