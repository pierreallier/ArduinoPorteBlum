#ifndef ANALOGSTATEDETECTOR_H
#define ANALOGSTATEDETECTOR_H

/**
 * @class AnalogStateDetector
 * @brief Détecte les changements d'état d'un capteur analogique avec temporisation.
 *
 * Cette classe convertit la valeur analogique d'un capteur en un état logique
 * actif ou inactif à partir de deux seuils :
 *
 * - un seuil d'activation ;
 * - un seuil de désactivation.
 *
 * L'utilisation de deux seuils permet d'appliquer une hystérésis et d'éviter
 * les changements d'état intempestifs lorsque la mesure oscille autour d'un
 * seuil.
 *
 * Un changement d'état n'est validé que si le nouvel état reste stable pendant
 * la durée de temporisation définie. La classe fonctionne sans utiliser
 * delay() et doit être mise à jour régulièrement à l'aide de la méthode update().
 *
 * Exemple :
 *
 * @code
 * AnalogStateDetector capteur(A0, 600, 500, 100);
 *
 * void setup() {
 *   capteur.init();
 * }
 *
 * void loop() {
 *   capteur.update();
 *
 *   if (capteur.changed()) {
 *     // Un changement d'état vient d'être validé
 *   }
 * }
 * @endcode
 */
class AnalogStateDetector {
    public:
    AnalogStateDetector(
        uint8_t pin,
        uint16_t seuilActivation,
        uint16_t seuilDesactivation,
        uint16_t temporisation
    )
        : _pin(pin),
        _seuilActivation(seuilActivation),
        _seuilDesactivation(seuilDesactivation),
        _temporisation(temporisation)
    {
    }

    /**
    * @brief Initialise le détecteur d’état analogique.
    *
    * Configure la broche analogique, lit la valeur initiale du capteur
    * et initialise l’état stable ainsi que les indicateurs internes.
    *
    * Cette fonction doit être appelée une seule fois dans setup().
    */
    void init() {
        pinMode(_pin, INPUT);
        _etatStable = mesure() >= _seuilActivation;
        _etatCandidat = _etatStable;
        _temporisationEnCours = false;
        _instantDebut = 0;
        _changementDetecte = false;
    }

    /**
    * @brief Met à jour l'état du capteur.
    *
    * Lit la valeur analogique, vérifie les seuils d'activation et de désactivation,
    * puis valide le changement après la temporisation définie.
    *
    * Cette fonction est non bloquante et doit être appelée régulièrement dans loop().
    */
    void update() {
        uint16_t m = mesure();
        // Détermination du nouvel état candidat avec hystérésis
        if (!_etatStable && m >= _seuilActivation)
            _etatCandidat = true;
        else if (_etatStable && m <= _seuilDesactivation)
            _etatCandidat = false;
        else
            _etatCandidat = _etatStable; // Entre les deux seuils : on conserve l'état candidat
        // Aucun changement potentiel
        if (_etatCandidat == _etatStable) {
            _temporisationEnCours = false;
            return;
        }
        // Début d'une nouvelle temporisation
        if (!_temporisationEnCours) {
            _temporisationEnCours = true;
            _instantDebut = millis();
            return;
        }
        // Vérification de la durée de stabilité
        if (millis() - _instantDebut >= _temporisation) {
            _etatStable = _etatCandidat;
            _temporisationEnCours = false;
            _changementDetecte = true;
        }
    }


    /**
    * @brief Retourne l'état stable actuel du capteur.
    *
    * @return true si le capteur est actif, sinon false.
    */
    bool etat() const { return _etatStable;}


    /**
    * @brief Indique si un changement d'état vient d'être validé.
    *
    * Cette fonction retourne true une seule fois par changement d'état,
    * puis remet automatiquement l'indicateur à false.
    *
    * @return true si un changement a été détecté, sinon false.
    */
    bool changed() { 
        if (_changementDetecte) {
            _changementDetecte = false;
            return true;
        }
        return false;
    }


    /**
    * @brief Lit la valeur analogique du capteur.
    *
    * Effectue une mesure sur la broche analogique associée au capteur
    * et retourne la valeur obtenue par le convertisseur analogique-numérique.
    *
    * @return Valeur mesurée, généralement comprise entre 0 et 1023.
    */
    uint16_t mesure() const {
        return analogRead(_pin);
    }

    private:
        uint8_t _pin;

        int _seuilActivation;
        int _seuilDesactivation;

        uint16_t _temporisation;
        uint16_t _instantDebut;

        bool _etatStable = false;
        bool _etatCandidat = false;
        bool _temporisationEnCours = false;
        bool _changementDetecte = false;
};

#endif