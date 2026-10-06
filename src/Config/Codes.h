#ifndef CODES_H
#define CODES_H
 
#include <stdint.h>

/*
 * Protocole des messages hors mesures : une ligne  TYPE;CODE;VAL\n
 *
 *   TYPE : I info | W warning | E erreur | S état | O réponse OK | N réponse NOK | R donnée
 *   CODE : valeur de l'enum Msg ci-dessous.
 *          NE JAMAIS RENUMÉROTER : le programme PC utilise les mêmes valeurs.
 *          Pour ajouter un message, prendre un numéro libre de sa plage.
 *   VAL  : entier signé 32 bits, 0 si sans objet.
 *          Les grandeurs décimales sont multipliées (x100 ou x1000, voir commentaires).
 *
 * Plages : erreurs 1..99  | infos 100..199 
 *          données 200..254 
 *
 * Hors protocole codé (envoyés en texte avec sendDirect(), voir Messages.h) :
 *   - les messages du setup (capteurs I2C, BNO055, fin d'initialisation...)
 *   - les messages du mode test (type T)
 */

 enum class MSG : uint8_t {
    NONE = 0,

    // ---------------- Erreurs (E) ----------------
    ERR_COURANT = 1,                    // "limite de courant atteinte"
    ERR_COURANT_SET = 2,                // "limite de courant incompatible (entre 0 et 2.5). Valeur non modifiée "
    ERR_BLOCAGE = 3,                    // "blocage mécanique détecté"
    ERR_LIMITE_PORTE = 4,               // "limite de la porte atteinte $.2°"      val : angle de la porte x100 (°)
    ERR_CALIBRATION_REQUISE = 5,        // "calibration requise"
    ERR_CALIBRATION_ANNULEE = 6,        // "calibration interrompue"
    ERR_NON_CALIBRE = 7,                // "Système non calibré"
    ERR_CALIBRATION_SET = 8,            // "commande set calibration inconnue : valeur attendue : ON / OFF / EFFACER"
    ERR_CMD_SETMODE_ICONNU = 9,         // "commande set mode inconnue"
    ERR_CMD_INCONNUE = 10,              // "commande inconnue (ou mal formée)"
    ERR_CMD_SET_INCONNUE = 11,          // "Commande set inconnue : valeur possibles : LIMITES / COURANT / MODE / CONSIGNE / PID / MESURES"
    ERR_CONSIGNE_EN_PILOTAGE = 12,      // "impossible de changer de consigne pendant le pilotage : le système doit être au repos"
    ERR_PILOTAGE_SET = 13,              // "Mode de pilotage inconnu : valeur possibles : PWM / POSITION / VITESSE / POSITIOB_VITESSE"
    ERR_PID_SET = 14,                   // "Réglage PID erronné : TYPE [VITESSE|POSITION] KP KI KD attendus"
    ERR_PID_TYPE_SET = 15,              // "Type de PID inconnu : valeur possibles VITESSE / POSITION"
    ERR_CONSIGNE_SET = 16,              // "Type de consigne inconnu : valeur possibles : POTENTIOMETRE / ECHELON / RAMPE / TRAPEZE / SINUS"
    ERR_FREQ_MESURES_SET = 17,          // "réglage de la fréquence de mesures : période manquante ou invalide (0 à 1000)"
    ERR_MESURE_GET = 18,                // "commande get mesures : capteur inconnu"
    ERR_CMD_GET_INCONNUE = 19,          // "commande get inconnue : valeurs possibles : CALIBRATION / MODE / PID"
    ERR_CMD_DO_INCONNUE = 20,           // "commande do inconnue : valeurs possibles : RESET / INIT / OUVRIR / FERMER / STOP / PILOTER / CALIBRATION / TEST"
    ERR_MODE = 21,                      // "mode de pilotage erroné"
    ERR_TEST_IMPOSSIBLE = 22,           // "demande de test impossible : uniquement au repos ou en calibration"
    ERR_MESSAGE_PERDU = 23,             // "$ message(s) perdu(s)"
    
    // ---------------- Infos (I) ----------------
    INFO_CALIBRATION_DEBUT = 100,        // "début de la calibration"
    INFO_CALIBRATION_ACHEVEE = 101,      // "fin de calibration"
    INFO_CALIBRATION_ECHEC = 102,        // "calibration échouée"
    INFO_CALIBRE = 103,                  // "système calibré"
    INFO_COURSE = 104,                   // "Course du capteur $"
    INFO_OFFSET = 105,                   // "Offset du capteur $"
    INFO_LIMITE_BASSE = 106,             // "Limite basse de sécurité $"
    INFO_LIMITE_HAUTE = 107,             // "Limite haute de sécurité $"
    INFO_CALIBRATION_SET = 108,          // "commande set calibration faite"
    INFO_ANGLE_PORTE = 109,              // "Angle de la porte $.2"
    INFO_SUR_MEUBLE = 110,               // "servodrive monté sur le meuble"
    INFO_DEMONTE = 111,                  // "servodrive non monté sur le meuble"
    INFO_BN0055_ABSENT = 112,            // "capteur BN0055 absent"
    INFO_BN0055_PRESENT = 113,           // "capteur BN0055 détecté"
    INFO_MODE = 114,                     // "Mode de pilotage : ${PWM|POSITION|VITESSE|POSITION_VITESSE}"
    INFO_PID_SET = 115,                  // "Configuration du PID effectuée"
    INFO_CONSIGNE_SET = 116,             // "Consigne mise à jour"
    INFO_COURANT_SET = 117,              // "Limite de courant modifiée"
    INFO_MESURES_DESACTIVE = 118,        // "Envoi des mesures désactivé"
    INFO_FREQ_MESURES = 119,             // "Envoi des mesures toutes les $ ms"
    INFO_DO_INIT = 120,                  // "Demande état INIT"
    INFO_DO_OUVRIR = 121,                // "Demande état OUVRIR"
    INFO_DO_FERMER = 122,                // "Demande état FERMER"
    INFO_DO_STOP = 123,                  // "Demande état STOP"
    INFO_DO_PILOTER = 124,               // "Demande état PILOTER"
    INFO_DO_CALIBRATION = 125,           // "Demande état CALIBRATION"
    INFO_VERSION_DEV = 126,              // "Mode ${DEV|PROD}"
    INFO_VERSION_CARTE = 127,            // "Version de la carte $"
    INFO_VERSION_SOFT = 128,             // "Version du soft $"
  
    // ---------------- Données (R) ----------------
    DATA_PID_VITESSE_KP = 200,  // "PID vitesse : Kp=$.3"
    DATA_PID_VITESSE_KI = 201,  // "PID vitesse : Ki=$.3"
    DATA_PID_VITESSE_KD = 202,  // "PID vitesse : Kd=$.3"
    DATA_PID_POSITION_KP = 203, // "PID position : Kp=$.3"
    DATA_PID_POSITION_KI = 204, // "PID position : Ki=$.3"
    DATA_PID_POSITION_KD = 205, // "PID position : Kd=$.3"
    DATA_MODE_PWM = 206,        // "mode de pilotage PWM"
    DATA_MODE_POSITION = 207,   // "mode de pilotage en position"
    DATA_MODE_VITESSE = 208,    // "mode de pilotage en vitesse"
    DATA_MODE_PV = 209,         // "mode de pilotage en vitesse et position"
    
};

#endif