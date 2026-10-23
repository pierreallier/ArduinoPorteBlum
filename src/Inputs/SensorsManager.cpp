#include "SensorsManager.h"

void SensorsManager::init() {
    _meuble.init(); // Initialisation du détecteur de meuble
    _codeurPorte.init(); // Initialisation du codeur absolu de la porte
    _bno.init(); // Initialisation du capteur BN0055
    _codeurMoteur.init(); // Initialisation du codeur du moteur
    _courant.init(); // Initialisation du capteur de courant
    _courantFilter.reset();
    _mesures.time = millis();
}

void SensorsManager::updateSecurities(int16_t pwm) {
    // Mets à jours les sécurité pour le fonctionnement du système : 
    //  - Limites extrémales de la porte
    //  - Limite de courant du moteur
    //  - Vérifie les blocages

    // Détection des limites extrémales de la porte
    const int32_t angle = _codeurPorte.readRaw(); // lit la valeur brute du codeur absolu pour comparaisons limites
    _mesures.angle_porte = angle * _codeurPorte.getCoeff(); // Converti en °*100
    limite_haute = (angle >= calibrationData.highLimit);
    limite_basse = (angle <= calibrationData.lowLimit);

    // Limite de courant
    _courant.read();
    limite_courant_atteinte = _courant.isLimitReached();

    // PWM
    _pwm = pwm; // Valeur PWM actuelle (+/-255)

    // Détection blocage
    int pwm_abs = abs(pwm);
    if (pwm_abs < PWM_MIN) { // PWM trop faible pour détecter un blocage mécanique
        compteur_blocage = 0;
        blocage_detecte = false;
        return;
    }
    if (abs(_codeurMoteur.getDeltaTicks()) > TICKS_MIN) { // Le moteur tourne suffisamment
        compteur_blocage = 0;
        blocage_detecte = false;
        return;
    }
    compteur_blocage++;
    uint8_t seuil = (pwm_abs >= PWM_RAPIDE) ? NB_CYCLES_BLOCAGE_RAPIDE : NB_CYCLES_BLOCAGE_LENT;
    blocage_detecte = (compteur_blocage >= seuil);
}

bool SensorsManager::isBlocage() {
    if (blocage_detecte) {
        blocage_detecte = false;
        compteur_blocage = 0;
        return true;
    }
    return false;
}

bool SensorsManager::isLimiteCourant() {
    if (limite_courant_atteinte) {
        limite_courant_atteinte = false;
        return true;
    }
    return false;
}

bool SensorsManager::isLimiteAngle() {
    if (limite_haute || limite_basse) {
        limite_haute = false;
        limite_basse = false;
        return true;
    }
    return false;
}

bool SensorsManager::resetSecurities() {
    blocage_detecte = false;
    compteur_blocage = 0;
    limite_courant_atteinte = false;
    limite_haute = false;
    limite_basse = false;
    return true;
}


void SensorsManager::mesures() {
    _codeurMoteur.read();
    _mesures.time = millis();
    _mesures.angle_moteur = _codeurMoteur.getAngle();
    _mesures.vitesse_moteur = _codeurMoteur.getSpeed();
    // Multiplication sur 32 bits pour eviter le debordement 16 bits sur AVR.
    _mesures.tension = static_cast<int16_t>(static_cast<int32_t>(_tension.read()) * _pwm / 255);
    _mesures.courant_moyen = static_cast<int>(_courantFilter.get(_courant.getMoy()) * 100.0f); // A * 100
    _potentiometre.read();
    _meuble.update();
}


void SensorsManager::setCalibration(CalibrationData c) {
    calibrationData = c;
    _codeurPorte.setOffset(calibrationData.offset);
}


bool SensorsManager::setLimitCourant(float limite) {
    if (limite > 0.0f && limite < 2.5f) {
        _courant.setLimit(limite);
        return true;
    }
    return false;
}

bool SensorsManager::checkBNO() {
    _bno.setEnabled(true);
    if (!_bno.checkPresence()) {
        _bno.setEnabled(false);  // Désactivation du capteur pour éviter les erreurs de lecture
        return false;
    } else {
        _bno.requestData();  // Demande de lecture initiale
        return true;
    }
}
