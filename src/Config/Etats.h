#ifndef ETATS_H
#define ETATS_H

#include <stdint.h>
#include "Constantes.h"

/*
 * Fichier généré automatiquement.
 * Ne pas modifier manuellement.
 *
 * Source : etats.csv
 */

enum class ETAT : uint8_t {
    INIT = 0,    // "INIT"
    REPOS = 1,    // "REPOS"
    FONCTIONNEMENT = 2,    // "FONCTIONNEMENT"
    OUVERTURE = 3,    // "OUVERTURE"
    FERMETURE = 4,    // "FERMETURE"
    PILOTAGE = 5,    // "PILOTAGE"
    CALIBRATION = 6,    // "CALIBRATION"
    DEBRAYAGE = 7,    // "DEBRAYAGE"
    ERREUR = 8,    // "ERREUR"
    STOP = 9,    // "STOP"
};

enum class ETAT_CALIBRATION : uint8_t {
    DEBUT = 20,    // "INIT"
    OUVERTURE = 21,    // "OUVERTURE"
    ATTENTE_HAUT = 22,    // "ATTENTE_HAUT"
    BUTEE_BASSE = 23,    // "BUTEE_BASSE"
    ATTENTE_BAS = 24,    // "ATTENTE_BAS"
    BUTEE_HAUTE = 25,    // "BUTEE_HAUTE"
    ENREGISTREMENT = 26,    // "ENREGISTREMENT"
    ERREUR = 27,    // "ERREUR"
    NONE = 28,    // "NONE"
};

inline const __FlashStringHelper* etatName(ETAT etat)
{
    switch (etat) {
        case ETAT::INIT: return F("INIT");
        case ETAT::REPOS: return F("REPOS");
        case ETAT::FONCTIONNEMENT: return F("FONCTIONNEMENT");
        case ETAT::OUVERTURE: return F("OUVERTURE");
        case ETAT::FERMETURE: return F("FERMETURE");
        case ETAT::PILOTAGE: return F("PILOTAGE");
        case ETAT::CALIBRATION: return F("CALIBRATION");
        case ETAT::DEBRAYAGE: return F("DEBRAYAGE");
        case ETAT::ERREUR: return F("ERREUR");
        case ETAT::STOP: return F("STOP");

        default:
            return F("UNKNOWN");
    }
}

inline const __FlashStringHelper* etatName(ETAT_CALIBRATION etat)
{
    switch (etat) {
        case ETAT_CALIBRATION::DEBUT: return F("INIT");
        case ETAT_CALIBRATION::OUVERTURE: return F("OUVERTURE");
        case ETAT_CALIBRATION::ATTENTE_HAUT: return F("ATTENTE_HAUT");
        case ETAT_CALIBRATION::BUTEE_BASSE: return F("BUTEE_BASSE");
        case ETAT_CALIBRATION::ATTENTE_BAS: return F("ATTENTE_BAS");
        case ETAT_CALIBRATION::BUTEE_HAUTE: return F("BUTEE_HAUTE");
        case ETAT_CALIBRATION::ENREGISTREMENT: return F("ENREGISTREMENT");
        case ETAT_CALIBRATION::ERREUR: return F("ERREUR");
        case ETAT_CALIBRATION::NONE: return F("NONE");

        default:
            return F("UNKNOWN");
    }
}

#endif
