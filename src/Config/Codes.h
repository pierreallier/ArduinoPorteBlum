#ifndef CODES_H
#define CODES_H

#include <stdint.h>
#include "Constantes.h"

/*
 * Fichier généré automatiquement.
 * Ne pas modifier manuellement.
 *
 * Source : protocol.csv
 */

enum class MSG : uint8_t {
    NONE = 0,    // "none"
    ERR_CMD_INCONNUE = 1,    // "commande inconnue (ou mal formée)"
    INFO_VERSION_DEV = 2,    // "mode ${PROD|DEV}"
    INFO_VERSION_CARTE = 3,    // "version de la carte $"
    INFO_VERSION_SOFT = 4,    // "version du soft $"
    ERR_MESSAGE_PERDU = 5,    // "$ message(s) perdu(s)"
    ERR_COURANT = 10,    // "limite de courant atteinte"
    ERR_BLOCAGE = 11,    // "blocage mécanique détecté"
    ERR_LIMITE_PORTE = 12,    // "limite de la porte atteinte $.2°"
    DATA_LIMITE_BASSE = 13,    // "limite basse de sécurité $"
    DATA_LIMITE_HAUTE = 14,    // "limite haute de sécurité $"
    ERR_PUISSANCE = 15,    // "absence de puissance : vérifier l'alimentation électrique ($.2V)"
    ERR_CALIBRATION_REQUISE = 20,    // "calibration requise"
    ERR_CALIBRATION_ANNULEE = 21,    // "calibration interrompue"
    ERR_NON_CALIBRE = 22,    // "système non calibré"
    INFO_CALIBRATION_DEBUT = 23,    // "début de la calibration"
    INFO_CALIBRATION_ACHEVEE = 24,    // "fin de calibration"
    INFO_CALIBRATION_ECHEC = 25,    // "calibration échouée"
    INFO_CALIBRE = 26,    // "système calibré"
    DATA_COURSE = 27,    // "course du capteur $"
    DATA_OFFSET = 28,    // "offset du capteur $"
    ERR_CMD_SET_INCONNUE = 30,    // "commande set inconnue : valeur attendue : LIMITES / COURANT / CALIBRATION / MODE / CONSIGNE / PID / MESURES / ACCEL"
    ERR_COURANT_SET = 31,    // "limite de courant incompatible (entre 0 et 2.5A) : valeur non modifiée "
    ERR_CALIBRATION_SET = 32,    // "commande set calibration inconnue : valeur attendue : ON / OFF / EFFACER"
    ERR_PILOTAGE_SET = 33,    // "mode de pilotage inconnu : valeur attendue : PWM / POSITION / VITESSE / POSITIOB_VITESSE"
    ERR_CONSIGNE_SET = 34,    // "type de consigne inconnu : valeur attendue : POTENTIOMETRE / ECHELON / RAMPE / TRAPEZE / SINUS"
    ERR_PID_TYPE_SET = 35,    // "type de PID inconnu : valeur attendue VITESSE / POSITION"
    ERR_PID_SET = 36,    // "réglage PID erronné : TYPE [VITESSE|POSITION] KP KI KD attendu"
    ERR_ACCEL_SET = 37,    // "commande set accéléromètre inconnue : valeur attendue : ON / OFF"
    ERR_BUZZER_SET = 38,    // "commande set buzzer inconnue : valeur attendue : ON / OFF"
    ERR_FREQ_MESURES_SET = 39,    // "réglage de la fréquence de mesures : période manquante ou invalide (0 à 1000)"
    INFO_COURANT_SET = 50,    // "limite de courant modifiée"
    INFO_CALIBRATION_SET = 51,    // "commande set calibration faite"
    INFO_CONSIGNE_SET = 52,    // "consigne mise à jour"
    INFO_PID_SET = 53,    // "configuration du PID effectuée"
    INFO_ACCEL_SET = 54,    // "${désactivation|activation} de l'accéléromètre"
    INFO_BUZZER = 55,    // "Buzzer ${désactivé|activé}"
    ERR_CMD_GET_INCONNUE = 60,    // "commande get inconnue : valeur attendue : CALIBRATION / MODE / PID / MESURES"
    ERR_MODE_GET = 61,    // "mode de pilotage erroné"
    ERR_MESURE_GET = 62,    // "commande get mesures : capteur inconnu"
    INFO_ACCEL_GET = 63,    // "accéléromètre ${désactivé|activé}"
    ERR_ACCEL_ABSENT = 64,    // "accéléromètre non connecté"
    INFO_BUZZER_GET = 65,    // "buzzer ${désactivé|activé}"
    ERR_CMD_DO_INCONNUE = 70,    // "commande do inconnue : valeur attendue : RESET / INIT / OUVRIR / FERMER / STOP / PILOTER / CALIBRATION / TEST"
    INFO_DO_RESET = 71,    // "demande reset carte"
    INFO_DO_INIT = 72,    // "demande état INIT"
    INFO_DO_OUVRIR = 73,    // "demande état OUVRIR"
    INFO_DO_FERMER = 74,    // "demande état FERMER"
    INFO_DO_STOP = 75,    // "demande état STOP"
    INFO_DO_PILOTER = 76,    // "demande état PILOTER"
    INFO_DO_CALIBRATION = 77,    // "demande état CALIBRATION"
    ERR_TEST_IMPOSSIBLE = 78,    // "demande de test impossible : uniquement au repos ou en calibration"
    ERR_CONSIGNE_EN_PILOTAGE = 80,    // "impossible de changer de consigne pendant le pilotage : le système doit être au repos"
    DATA_MODE_PWM = 81,    // "mode de pilotage PWM"
    DATA_MODE_POSITION = 82,    // "mode de pilotage en position"
    DATA_MODE_VITESSE = 83,    // "mode de pilotage en vitesse"
    DATA_MODE_PV = 84,    // "mode de pilotage en vitesse et position"
    DATA_PID_VITESSE_KP = 90,    // "PID vitesse :  Kp = $.3"
    DATA_PID_VITESSE_KI = 91,    // "PID vitesse :  Ki = $.3"
    DATA_PID_VITESSE_KD = 92,    // "PID vitesse :  Kd = $.3"
    DATA_PID_POSITION_KP = 93,    // "PID position : Kp = $.3"
    DATA_PID_POSITION_KI = 94,    // "PID position : Ki = $.3"
    DATA_PID_POSITION_KD = 95,    // "PID position : Kd = $.3"
    INFO_MESURES_DESACTIVE = 100,    // "envoi des mesures désactivé"
    DATA_FREQ_MESURES = 101,    // "envoi des mesures toutes les $ ms"
    DATA_ANGLE_PORTE = 102,    // "angle de la porte $.2°"
    DATA_TENSION = 103,    // "tension d'alimentation $.2V"
    DATA_COURANT = 104,    // "courant consommé $.2A"
    DATA_ANGLE_MOTEUR = 105,    // "angle moteur $.2°"
    DATA_VITESSE_MOTEUR = 106,    // "vitesse moteur $.2 rad/s"
    DATA_PWM = 107,    // "PWM moteur $.2"
    DATA_CONSIGNE = 108,    // "consigne moteur $.2 (PWM,°,rad/s)"
    INFO_SUR_MEUBLE = 109,    // "servodrive monté sur le meuble"
    INFO_DEMONTE = 110,    // "servodrive non monté sur le meuble"
    INFO_BN0055_ABSENT = 111,    // "capteur BN0055 absent"
    INFO_BN0055_PRESENT = 112,    // "capteur BN0055 détecté"
    ETAT_PROD = 250,    // "etat en prod"
    ETAT_CALIBRATION = 251,    // "etat de calibration"
};

#if VERSION_DEV

inline const __FlashStringHelper* msgName(MSG msg)
{
    switch (msg) {
        case MSG::NONE: return F("NONE");
        case MSG::ERR_CMD_INCONNUE: return F("ERR_CMD_INCONNUE");
        case MSG::INFO_VERSION_DEV: return F("INFO_VERSION_DEV");
        case MSG::INFO_VERSION_CARTE: return F("INFO_VERSION_CARTE");
        case MSG::INFO_VERSION_SOFT: return F("INFO_VERSION_SOFT");
        case MSG::ERR_MESSAGE_PERDU: return F("ERR_MESSAGE_PERDU");
        case MSG::ERR_COURANT: return F("ERR_COURANT");
        case MSG::ERR_BLOCAGE: return F("ERR_BLOCAGE");
        case MSG::ERR_LIMITE_PORTE: return F("ERR_LIMITE_PORTE");
        case MSG::DATA_LIMITE_BASSE: return F("DATA_LIMITE_BASSE");
        case MSG::DATA_LIMITE_HAUTE: return F("DATA_LIMITE_HAUTE");
        case MSG::ERR_PUISSANCE: return F("ERR_PUISSANCE");
        case MSG::ERR_CALIBRATION_REQUISE: return F("ERR_CALIBRATION_REQUISE");
        case MSG::ERR_CALIBRATION_ANNULEE: return F("ERR_CALIBRATION_ANNULEE");
        case MSG::ERR_NON_CALIBRE: return F("ERR_NON_CALIBRE");
        case MSG::INFO_CALIBRATION_DEBUT: return F("INFO_CALIBRATION_DEBUT");
        case MSG::INFO_CALIBRATION_ACHEVEE: return F("INFO_CALIBRATION_ACHEVEE");
        case MSG::INFO_CALIBRATION_ECHEC: return F("INFO_CALIBRATION_ECHEC");
        case MSG::INFO_CALIBRE: return F("INFO_CALIBRE");
        case MSG::DATA_COURSE: return F("DATA_COURSE");
        case MSG::DATA_OFFSET: return F("DATA_OFFSET");
        case MSG::ERR_CMD_SET_INCONNUE: return F("ERR_CMD_SET_INCONNUE");
        case MSG::ERR_COURANT_SET: return F("ERR_COURANT_SET");
        case MSG::ERR_CALIBRATION_SET: return F("ERR_CALIBRATION_SET");
        case MSG::ERR_PILOTAGE_SET: return F("ERR_PILOTAGE_SET");
        case MSG::ERR_CONSIGNE_SET: return F("ERR_CONSIGNE_SET");
        case MSG::ERR_PID_TYPE_SET: return F("ERR_PID_TYPE_SET");
        case MSG::ERR_PID_SET: return F("ERR_PID_SET");
        case MSG::ERR_ACCEL_SET: return F("ERR_ACCEL_SET");
        case MSG::ERR_BUZZER_SET: return F("ERR_BUZZER_SET");
        case MSG::ERR_FREQ_MESURES_SET: return F("ERR_FREQ_MESURES_SET");
        case MSG::INFO_COURANT_SET: return F("INFO_COURANT_SET");
        case MSG::INFO_CALIBRATION_SET: return F("INFO_CALIBRATION_SET");
        case MSG::INFO_CONSIGNE_SET: return F("INFO_CONSIGNE_SET");
        case MSG::INFO_PID_SET: return F("INFO_PID_SET");
        case MSG::INFO_ACCEL_SET: return F("INFO_ACCEL_SET");
        case MSG::INFO_BUZZER: return F("INFO_BUZZER");
        case MSG::ERR_CMD_GET_INCONNUE: return F("ERR_CMD_GET_INCONNUE");
        case MSG::ERR_MODE_GET: return F("ERR_MODE_GET");
        case MSG::ERR_MESURE_GET: return F("ERR_MESURE_GET");
        case MSG::INFO_ACCEL_GET: return F("INFO_ACCEL_GET");
        case MSG::ERR_ACCEL_ABSENT: return F("ERR_ACCEL_ABSENT");
        case MSG::INFO_BUZZER_GET: return F("INFO_BUZZER_GET");
        case MSG::ERR_CMD_DO_INCONNUE: return F("ERR_CMD_DO_INCONNUE");
        case MSG::INFO_DO_RESET: return F("INFO_DO_RESET");
        case MSG::INFO_DO_INIT: return F("INFO_DO_INIT");
        case MSG::INFO_DO_OUVRIR: return F("INFO_DO_OUVRIR");
        case MSG::INFO_DO_FERMER: return F("INFO_DO_FERMER");
        case MSG::INFO_DO_STOP: return F("INFO_DO_STOP");
        case MSG::INFO_DO_PILOTER: return F("INFO_DO_PILOTER");
        case MSG::INFO_DO_CALIBRATION: return F("INFO_DO_CALIBRATION");
        case MSG::ERR_TEST_IMPOSSIBLE: return F("ERR_TEST_IMPOSSIBLE");
        case MSG::ERR_CONSIGNE_EN_PILOTAGE: return F("ERR_CONSIGNE_EN_PILOTAGE");
        case MSG::DATA_MODE_PWM: return F("DATA_MODE_PWM");
        case MSG::DATA_MODE_POSITION: return F("DATA_MODE_POSITION");
        case MSG::DATA_MODE_VITESSE: return F("DATA_MODE_VITESSE");
        case MSG::DATA_MODE_PV: return F("DATA_MODE_PV");
        case MSG::DATA_PID_VITESSE_KP: return F("DATA_PID_VITESSE_KP");
        case MSG::DATA_PID_VITESSE_KI: return F("DATA_PID_VITESSE_KI");
        case MSG::DATA_PID_VITESSE_KD: return F("DATA_PID_VITESSE_KD");
        case MSG::DATA_PID_POSITION_KP: return F("DATA_PID_POSITION_KP");
        case MSG::DATA_PID_POSITION_KI: return F("DATA_PID_POSITION_KI");
        case MSG::DATA_PID_POSITION_KD: return F("DATA_PID_POSITION_KD");
        case MSG::INFO_MESURES_DESACTIVE: return F("INFO_MESURES_DESACTIVE");
        case MSG::DATA_FREQ_MESURES: return F("DATA_FREQ_MESURES");
        case MSG::DATA_ANGLE_PORTE: return F("DATA_ANGLE_PORTE");
        case MSG::DATA_TENSION: return F("DATA_TENSION");
        case MSG::DATA_COURANT: return F("DATA_COURANT");
        case MSG::DATA_ANGLE_MOTEUR: return F("DATA_ANGLE_MOTEUR");
        case MSG::DATA_VITESSE_MOTEUR: return F("DATA_VITESSE_MOTEUR");
        case MSG::DATA_PWM: return F("DATA_PWM");
        case MSG::DATA_CONSIGNE: return F("DATA_CONSIGNE");
        case MSG::INFO_SUR_MEUBLE: return F("INFO_SUR_MEUBLE");
        case MSG::INFO_DEMONTE: return F("INFO_DEMONTE");
        case MSG::INFO_BN0055_ABSENT: return F("INFO_BN0055_ABSENT");
        case MSG::INFO_BN0055_PRESENT: return F("INFO_BN0055_PRESENT");
        case MSG::ETAT_PROD: return F("ETAT_PROD");
        case MSG::ETAT_CALIBRATION: return F("ETAT_CALIBRATION");

        default:
            return F("UNKNOWN");
    }
}

#endif

#endif
