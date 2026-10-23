#pragma once

#include <Arduino.h>
#include "../Config/Constantes.h"

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
            pinMode(POTENTIOMETRE_PIN, INPUT);
        }

        float get(unsigned long) override {
            return (analogRead(POTENTIOMETRE_PIN) - 500) * 0.5f;
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
            if (dt < delai*1000UL)
                return 0.0f;
            dt -= delai*1000UL;
            if (dt < duree*1000UL)
                return valeur;
            return 0.0f;
        }

        bool ended(unsigned long t) const override {
            return (t - t_init >= (delai + duree)*1000UL);
        }

        const char* getName() const override {
            return "ECHELON";
        }

    private:
        float valeur = 0.0f; // valeur de l'échelon
        unsigned long duree = 0; // durée de l'échelon en millisecondes
        unsigned long delai = 0; // délai avant le début de l'échelon en millisecondes
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
            if (dt < delai*1000UL)
                return valeurInitiale;
            dt -= delai*1000UL;
            if (duree == 0 || dt >= duree*1000UL)
                return valeurFinale;
            float progression = static_cast<float>(dt) / static_cast<float>(duree) * 0.001f;
            return valeurInitiale + progression * (valeurFinale - valeurInitiale);
        }

        bool ended(unsigned long t) const override {
            return (t - t_init >= (delai + duree)*1000UL);
        }

        const char* getName() const override {
            return "RAMPE";
        }

    private:
        float valeurInitiale = 0.0f; // valeur initiale de la rampe
        float valeurFinale = 0.0f; // valeur finale de la rampe
        unsigned long duree = 0; // durée de la rampe en millisecondes
        unsigned long delai = 0; // délai avant le début de la rampe en millisecondes
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
            if (dt < delai*1000UL)
                return 0.0f;
            dt -= delai*1000UL;
            if (dt < dureeMontee*1000UL) {
                if (dureeMontee == 0)
                    return valeur;
                float progression = static_cast<float>(dt) / static_cast<float>(dureeMontee) * 0.001f;
                return valeur * progression;
            }
            dt -= dureeMontee*1000UL;
            if (dt < dureePlateau*1000UL)
                return valeur;
            dt -= dureePlateau*1000UL;
            if (dt < dureeDescente*1000UL) {
                if (dureeDescente == 0)
                    return 0.0f;
                float progression = static_cast<float>(dt) / static_cast<float>(dureeDescente) * 0.001f;
                return valeur * (1.0f - progression);
            }
            return 0.0f;
        }

        bool ended(unsigned long t) const override {
            return (t - t_init >= (delai + dureeMontee + dureePlateau + dureeDescente)*1000UL);
        }

        const char* getName() const override {
            return "TRAPEZE";
        }

    private:
        float valeur = 0.0f; // Valeur maximale du trapèze
        unsigned long dureeMontee = 0; // Durée de la montée en millisecondes
        unsigned long dureePlateau = 0; // Durée du plateau en millisecondes
        unsigned long dureeDescente = 0; // Durée de la descente en millisecondes
        unsigned long delai = 0; // Délai avant le début du trapèze en millisecondes
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
            if (dt >= duree*1000UL)
                return offset;
            if (periode == 0)
                return offset;
            float angle = 2.0f * PI * static_cast<float>(dt) / static_cast<float>(periode) * 0.001f;
            return offset + amplitude * sin(angle);
        }

        bool ended(unsigned long t) const override {
            return (t - t_init >= duree*1000UL);
        }

        const char* getName() const override {
            return "SINUS";
        }

    private:
        float amplitude = 0.0f; // Amplitude de la sinusoïde
        float offset = 0.0f; // Offset de la sinusoïde
        unsigned long periode = 0; // période de la sinusoïde en millisecondes
        unsigned long duree = 0; // durée totale de la consigne en millisecondes
};

/* @brief Consigne à partir d'un tableau de valeurs (échantillons).
 *
 * La consigne est définie par un tableau de valeurs à deux entrée
 * La consigne évolue suivant les valeurs du tableau, avec interpolation 
 * linéaire entre les échantillons.
 * 
 * La durée totale de la consigne est déterminée par le nombre d'échantillons
 * et l'intervalle de temps entre eux.
 * 
 * 
 */
class ConsigneTableau : public Consigne {
    public:
        explicit ConsigneTableau(unsigned int nb_points = 0) {
            reserve(nb_points);
        }

        ConsigneTableau(const ConsigneTableau&) = delete;
        ConsigneTableau& operator=(const ConsigneTableau&) = delete;

        ConsigneTableau(ConsigneTableau&& other) noexcept {
            points = other.points;
            count = other.count;
            capacity = other.capacity;
            other.points = nullptr;
            other.count = 0;
            other.capacity = 0;
        }

        ConsigneTableau& operator=(ConsigneTableau&& other) noexcept {
            if (this == &other)
                return *this;

            if (points != nullptr)
                free(points);

            points = other.points;
            count = other.count;
            capacity = other.capacity;

            other.points = nullptr;
            other.count = 0;
            other.capacity = 0;
            return *this;
        }

        ~ConsigneTableau() override {
            if (points != nullptr) {
                free(points);
                points = nullptr;
            }
            capacity = 0;
            count = 0;
        }

        bool reserve(unsigned int nb_points) {
            if (nb_points <= capacity)
                return true;

            Point* nouveau = static_cast<Point*>(realloc(points, nb_points * sizeof(Point)));
            if (nouveau == nullptr)
                return false;

            points = nouveau;
            capacity = nb_points;
            return true;
        }

        void clear() {
            count = 0;
        }

        bool append(unsigned long time, float val) {
            if (count >= capacity) {
                unsigned int nouvelleCapacite = (capacity == 0) ? 8 : (capacity * 2);
                if (nouvelleCapacite >= 100 || !reserve(nouvelleCapacite))
                    return false;
            }

            if (count > 0 && time < points[count - 1].time)
                return false;

            points[count].time = time;
            points[count].valeur = val;
            count++;
            return true;
        }

        unsigned int size() const {
            return count;
        }

        float get(unsigned long t) override {
            if (count == 0 || points == nullptr)
                return 0.0f;
            if (count == 1)
                return points[0].valeur;

            unsigned long dt = t - t_init;
            if (dt >= points[count - 1].time*1000UL)
                return 0.0f;

            if (dt <= points[0].time*1000UL)
                return points[0].valeur;

            for (unsigned int i = 0; i + 1 < count; i++) {
                const unsigned long t0 = points[i].time*1000UL;
                const unsigned long t1 = points[i + 1].time*1000UL;

                if (dt < t1) {
                    if (t1 <= t0)
                        return points[i + 1].valeur;

                    float progression = static_cast<float>(dt - t0) / static_cast<float>(t1 - t0);
                    return points[i].valeur + progression * (points[i + 1].valeur - points[i].valeur);
                }
            }

            return 0.0f;
        }

        bool ended(unsigned long t) const override {
            if (count == 0 || points == nullptr)
                return true;
            return (t - t_init >= points[count - 1].time*1000UL);
        }

        const char* getName() const override {
            return "TABLEAU";
        }

    private:
        struct Point {
            unsigned long time = 0; // Temps en millisecondes depuis le début de la consigne
            float valeur = 0.0f; // Valeur du point
        };

        Point* points = nullptr;
        unsigned int count = 0; // Nombre de points actuellement stockés
        unsigned int capacity = 0;
};

/* @brief Consigne de test.
 *   
 * Consigne qui ne fait rien et se termine après 10 secondes.
 * Pour test du code sans mouvement moteur.
 */
class ConsigneTest : public Consigne {
    public:
        void init(unsigned long t) override {
            Consigne::init(t);
        }

        float get(unsigned long t) override {
            return 0.0f;
        }

        bool ended(unsigned long t) const override {
            if (t - t_init >= 10000UL*1000UL) {
                return true;
            }
            return false;
        }

        const char* getName() const override {
            return "TEST";
        }
};