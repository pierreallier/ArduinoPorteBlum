"""
Fichier généré automatiquement.
Ne pas modifier manuellement.

Source : protocol.csv
"""

from enum import IntEnum


class MSG(IntEnum):
    NONE = 0
    ERR_CMD_INCONNUE = 1
    INFO_VERSION_DEV = 2
    INFO_VERSION_CARTE = 3
    INFO_VERSION_SOFT = 4
    ERR_MESSAGE_PERDU = 5
    ERR_COURANT = 10
    ERR_BLOCAGE = 11
    ERR_LIMITE_PORTE = 12
    DATA_LIMITE_BASSE = 13
    DATA_LIMITE_HAUTE = 14
    ERR_PUISSANCE = 15
    ERR_CALIBRATION_REQUISE = 20
    ERR_CALIBRATION_ANNULEE = 21
    ERR_NON_CALIBRE = 22
    INFO_CALIBRATION_DEBUT = 23
    INFO_CALIBRATION_ACHEVEE = 24
    INFO_CALIBRATION_ECHEC = 25
    INFO_CALIBRE = 26
    DATA_COURSE = 27
    DATA_OFFSET = 28
    ERR_CMD_SET_INCONNUE = 30
    ERR_COURANT_SET = 31
    ERR_CALIBRATION_SET = 32
    ERR_PILOTAGE_SET = 33
    ERR_CONSIGNE_SET = 34
    ERR_PID_TYPE_SET = 35
    ERR_PID_SET = 36
    ERR_ACCEL_SET = 37
    ERR_BUZZER_SET = 38
    ERR_FREQ_MESURES_SET = 39
    INFO_COURANT_SET = 50
    INFO_CALIBRATION_SET = 51
    INFO_CONSIGNE_SET = 52
    INFO_PID_SET = 53
    INFO_ACCEL_SET = 54
    INFO_BUZZER = 55
    ERR_CMD_GET_INCONNUE = 60
    ERR_MODE_GET = 61
    ERR_MESURE_GET = 62
    INFO_ACCEL_GET = 63
    ERR_ACCEL_ABSENT = 64
    INFO_BUZZER_GET = 65
    ERR_CMD_DO_INCONNUE = 70
    INFO_DO_RESET = 71
    INFO_DO_INIT = 72
    INFO_DO_OUVRIR = 73
    INFO_DO_FERMER = 74
    INFO_DO_STOP = 75
    INFO_DO_PILOTER = 76
    INFO_DO_CALIBRATION = 77
    ERR_TEST_IMPOSSIBLE = 78
    ERR_CONSIGNE_EN_PILOTAGE = 80
    DATA_MODE_PWM = 81
    DATA_MODE_POSITION = 82
    DATA_MODE_VITESSE = 83
    DATA_MODE_PV = 84
    DATA_PID_VITESSE_KP = 90
    DATA_PID_VITESSE_KI = 91
    DATA_PID_VITESSE_KD = 92
    DATA_PID_POSITION_KP = 93
    DATA_PID_POSITION_KI = 94
    DATA_PID_POSITION_KD = 95
    INFO_MESURES_DESACTIVE = 100
    DATA_FREQ_MESURES = 101
    DATA_ANGLE_PORTE = 102
    DATA_TENSION = 103
    DATA_COURANT = 104
    DATA_ANGLE_MOTEUR = 105
    DATA_VITESSE_MOTEUR = 106
    DATA_PWM = 107
    DATA_CONSIGNE = 108
    INFO_SUR_MEUBLE = 109
    INFO_DEMONTE = 110
    INFO_BN0055_ABSENT = 111
    INFO_BN0055_PRESENT = 112
    ETAT_PROD = 250
    ETAT_CALIBRATION = 251


MESSAGE_TEXT = {
    MSG.NONE: 'none',
    MSG.ERR_CMD_INCONNUE: 'commande inconnue (ou mal formée)',
    MSG.INFO_VERSION_DEV: 'mode ${PROD|DEV}',
    MSG.INFO_VERSION_CARTE: 'version de la carte $',
    MSG.INFO_VERSION_SOFT: 'version du soft $',
    MSG.ERR_MESSAGE_PERDU: '$ message(s) perdu(s)',
    MSG.ERR_COURANT: 'limite de courant atteinte',
    MSG.ERR_BLOCAGE: 'blocage mécanique détecté',
    MSG.ERR_LIMITE_PORTE: 'limite de la porte atteinte $.2°',
    MSG.DATA_LIMITE_BASSE: 'limite basse de sécurité $',
    MSG.DATA_LIMITE_HAUTE: 'limite haute de sécurité $',
    MSG.ERR_PUISSANCE: "absence de puissance : vérifier l'alimentation électrique ($.2V)",
    MSG.ERR_CALIBRATION_REQUISE: 'calibration requise',
    MSG.ERR_CALIBRATION_ANNULEE: 'calibration interrompue',
    MSG.ERR_NON_CALIBRE: 'système non calibré',
    MSG.INFO_CALIBRATION_DEBUT: 'début de la calibration',
    MSG.INFO_CALIBRATION_ACHEVEE: 'fin de calibration',
    MSG.INFO_CALIBRATION_ECHEC: 'calibration échouée',
    MSG.INFO_CALIBRE: 'système calibré',
    MSG.DATA_COURSE: 'course du capteur $',
    MSG.DATA_OFFSET: 'offset du capteur $',
    MSG.ERR_CMD_SET_INCONNUE: 'commande set inconnue : valeur attendue : LIMITES / COURANT / CALIBRATION / MODE / CONSIGNE / PID / MESURES / ACCEL',
    MSG.ERR_COURANT_SET: 'limite de courant incompatible (entre 0 et 2.5A) : valeur non modifiée ',
    MSG.ERR_CALIBRATION_SET: 'commande set calibration inconnue : valeur attendue : ON / OFF / EFFACER',
    MSG.ERR_PILOTAGE_SET: 'mode de pilotage inconnu : valeur attendue : PWM / POSITION / VITESSE / POSITIOB_VITESSE',
    MSG.ERR_CONSIGNE_SET: 'type de consigne inconnu : valeur attendue : POTENTIOMETRE / ECHELON / RAMPE / TRAPEZE / SINUS',
    MSG.ERR_PID_TYPE_SET: 'type de PID inconnu : valeur attendue VITESSE / POSITION',
    MSG.ERR_PID_SET: 'réglage PID erronné : TYPE [VITESSE|POSITION] KP KI KD attendu',
    MSG.ERR_ACCEL_SET: 'commande set accéléromètre inconnue : valeur attendue : ON / OFF',
    MSG.ERR_BUZZER_SET: 'commande set buzzer inconnue : valeur attendue : ON / OFF',
    MSG.ERR_FREQ_MESURES_SET: 'réglage de la fréquence de mesures : période manquante ou invalide (0 à 1000)',
    MSG.INFO_COURANT_SET: 'limite de courant modifiée',
    MSG.INFO_CALIBRATION_SET: 'commande set calibration faite',
    MSG.INFO_CONSIGNE_SET: 'consigne mise à jour',
    MSG.INFO_PID_SET: 'configuration du PID effectuée',
    MSG.INFO_ACCEL_SET: "${désactivation|activation} de l'accéléromètre",
    MSG.INFO_BUZZER: 'Buzzer ${désactivé|activé}',
    MSG.ERR_CMD_GET_INCONNUE: 'commande get inconnue : valeur attendue : CALIBRATION / MODE / PID / MESURES',
    MSG.ERR_MODE_GET: 'mode de pilotage erroné',
    MSG.ERR_MESURE_GET: 'commande get mesures : capteur inconnu',
    MSG.INFO_ACCEL_GET: 'accéléromètre ${désactivé|activé}',
    MSG.ERR_ACCEL_ABSENT: 'accéléromètre non connecté',
    MSG.INFO_BUZZER_GET: 'buzzer ${désactivé|activé}',
    MSG.ERR_CMD_DO_INCONNUE: 'commande do inconnue : valeur attendue : RESET / INIT / OUVRIR / FERMER / STOP / PILOTER / CALIBRATION / TEST',
    MSG.INFO_DO_RESET: 'demande reset carte',
    MSG.INFO_DO_INIT: 'demande état INIT',
    MSG.INFO_DO_OUVRIR: 'demande état OUVRIR',
    MSG.INFO_DO_FERMER: 'demande état FERMER',
    MSG.INFO_DO_STOP: 'demande état STOP',
    MSG.INFO_DO_PILOTER: 'demande état PILOTER',
    MSG.INFO_DO_CALIBRATION: 'demande état CALIBRATION',
    MSG.ERR_TEST_IMPOSSIBLE: 'demande de test impossible : uniquement au repos ou en calibration',
    MSG.ERR_CONSIGNE_EN_PILOTAGE: 'impossible de changer de consigne pendant le pilotage : le système doit être au repos',
    MSG.DATA_MODE_PWM: 'mode de pilotage PWM',
    MSG.DATA_MODE_POSITION: 'mode de pilotage en position',
    MSG.DATA_MODE_VITESSE: 'mode de pilotage en vitesse',
    MSG.DATA_MODE_PV: 'mode de pilotage en vitesse et position',
    MSG.DATA_PID_VITESSE_KP: 'PID vitesse :  Kp = $.3',
    MSG.DATA_PID_VITESSE_KI: 'PID vitesse :  Ki = $.3',
    MSG.DATA_PID_VITESSE_KD: 'PID vitesse :  Kd = $.3',
    MSG.DATA_PID_POSITION_KP: 'PID position : Kp = $.3',
    MSG.DATA_PID_POSITION_KI: 'PID position : Ki = $.3',
    MSG.DATA_PID_POSITION_KD: 'PID position : Kd = $.3',
    MSG.INFO_MESURES_DESACTIVE: 'envoi des mesures désactivé',
    MSG.DATA_FREQ_MESURES: 'envoi des mesures toutes les $ ms',
    MSG.DATA_ANGLE_PORTE: 'angle de la porte $.2°',
    MSG.DATA_TENSION: "tension d'alimentation $.2V",
    MSG.DATA_COURANT: 'courant consommé $.2A',
    MSG.DATA_ANGLE_MOTEUR: 'angle moteur $.2°',
    MSG.DATA_VITESSE_MOTEUR: 'vitesse moteur $.2 rad/s',
    MSG.DATA_PWM: 'PWM moteur $.2',
    MSG.DATA_CONSIGNE: 'consigne moteur $.2 (PWM,°,rad/s)',
    MSG.INFO_SUR_MEUBLE: 'servodrive monté sur le meuble',
    MSG.INFO_DEMONTE: 'servodrive non monté sur le meuble',
    MSG.INFO_BN0055_ABSENT: 'capteur BN0055 absent',
    MSG.INFO_BN0055_PRESENT: 'capteur BN0055 détecté',
    MSG.ETAT_PROD: 'etat en prod',
    MSG.ETAT_CALIBRATION: 'etat de calibration',
}
