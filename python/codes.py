"""
Fichier généré automatiquement.
Ne pas modifier manuellement.

Source : protocol.csv + etats.csv + Constantes.h
"""

MSG = {
    0: 'none',    # 'NONE'
    1: 'commande inconnue (ou mal formée)',    # 'ERR_CMD_INCONNUE'
    2: 'mode ${PROD|DEV}',    # 'INFO_VERSION_DEV'
    3: 'version de la carte $',    # 'INFO_VERSION_CARTE'
    4: 'version du soft $',    # 'INFO_VERSION_SOFT'
    5: '$ message(s) perdu(s)',    # 'ERR_MESSAGE_PERDU'
    10: 'limite de courant atteinte',    # 'ERR_COURANT'
    11: 'blocage mécanique détecté',    # 'ERR_BLOCAGE'
    12: 'limite de la porte atteinte $.2°',    # 'ERR_LIMITE_PORTE'
    13: 'limite basse de sécurité $',    # 'DATA_LIMITE_BASSE'
    14: 'limite haute de sécurité $',    # 'DATA_LIMITE_HAUTE'
    15: "absence de puissance : vérifier l'alimentation électrique ($.2V)",    # 'ERR_PUISSANCE'
    20: 'calibration requise',    # 'ERR_CALIBRATION_REQUISE'
    21: 'calibration interrompue',    # 'ERR_CALIBRATION_ANNULEE'
    22: 'système non calibré',    # 'ERR_NON_CALIBRE'
    23: 'début de la calibration',    # 'INFO_CALIBRATION_DEBUT'
    24: 'fin de calibration',    # 'INFO_CALIBRATION_ACHEVEE'
    25: 'calibration échouée',    # 'INFO_CALIBRATION_ECHEC'
    26: 'système calibré',    # 'INFO_CALIBRE'
    27: 'course du capteur $',    # 'DATA_COURSE'
    28: 'offset du capteur $',    # 'DATA_OFFSET'
    30: 'commande set inconnue : valeur attendue : COURANT / CALIBRATION / MODE / CONSIGNE / PID / MESURES / ACCEL / BUZZER',    # 'ERR_CMD_SET_INCONNUE'
    31: 'limite de courant incompatible (entre 0 et 2.5A) : valeur non modifiée ',    # 'ERR_COURANT_SET'
    32: 'commande set calibration inconnue : valeur attendue : ON / OFF / EFFACER',    # 'ERR_CALIBRATION_SET'
    33: 'mode de pilotage inconnu : valeur attendue : PWM / POSITION / VITESSE / POSITIOB_VITESSE',    # 'ERR_PILOTAGE_SET'
    34: 'type de consigne inconnu : valeur attendue : POTENTIOMETRE / ECHELON / RAMPE / TRAPEZE / SINUS',    # 'ERR_CONSIGNE_SET'
    35: 'type de PID inconnu : valeur attendue VITESSE / POSITION',    # 'ERR_PID_TYPE_SET'
    36: 'réglage PID erronné : TYPE [VITESSE|POSITION] KP KI KD attendu',    # 'ERR_PID_SET'
    37: 'commande set accéléromètre inconnue : valeur attendue : ON / OFF',    # 'ERR_ACCEL_SET'
    38: 'commande set buzzer inconnue : valeur attendue : ON / OFF',    # 'ERR_BUZZER_SET'
    39: 'réglage de la fréquence de mesures : période manquante ou invalide (0 à 1000)',    # 'ERR_FREQ_MESURES_SET'
    40: 'limite de courant modifiée',    # 'INFO_COURANT_SET'
    41: 'commande set calibration faite',    # 'INFO_CALIBRATION_SET'
    42: 'consigne mise à jour',    # 'INFO_CONSIGNE_SET'
    43: 'configuration du PID effectuée',    # 'INFO_PID_SET'
    44: "${désactivation|activation} de l'accéléromètre",    # 'INFO_ACCEL_SET'
    50: 'commande get inconnue : valeur attendue : LIMITS / CALIBRATION / MODE / PID / MESURES / ACCEL / BUZZER',    # 'ERR_CMD_GET_INCONNUE'
    51: 'mode de pilotage erroné',    # 'ERR_MODE_GET'
    52: 'commande get mesures : capteur inconnu',    # 'ERR_MESURE_GET'
    53: 'accéléromètre ${désactivé|activé}',    # 'INFO_ACCEL_GET'
    54: 'accéléromètre non connecté',    # 'ERR_ACCEL_ABSENT'
    55: 'buzzer ${désactivé|activé}',    # 'INFO_BUZZER'
    60: 'commande do inconnue : valeur attendue : RESET / INIT / OUVRIR / FERMER / STOP / PILOTER / CALIBRATION / TEST',    # 'ERR_CMD_DO_INCONNUE'
    61: 'demande reset carte',    # 'INFO_DO_RESET'
    62: 'demande état INIT',    # 'INFO_DO_INIT'
    63: 'demande état OUVRIR',    # 'INFO_DO_OUVRIR'
    64: 'demande état FERMER',    # 'INFO_DO_FERMER'
    65: 'demande état STOP',    # 'INFO_DO_STOP'
    66: 'demande état PILOTER',    # 'INFO_DO_PILOTER'
    67: 'demande état CALIBRATION',    # 'INFO_DO_CALIBRATION'
    68: 'test de la carte demandé',    # 'INFO_DO_TEST'
    69: 'demande de test impossible : uniquement au repos ou en calibration',    # 'ERR_TEST_IMPOSSIBLE'
    70: '\\n==== Pilotage Porte Blum ====\\n',    # 'INFO_INIT_DEBUT'
    71: '\\n== Initialisation terminée ==\\n',    # 'INFO_INIT_FIN'
    72: "erreur d'initialisation",    # 'ERR_INIT'
    73: 'version de développement',    # 'INFO_DEV'
    74: 'codeur absolu de la porte non détecté',    # 'ERR_CAPTEUR_ABSENT'
    75: 'codeur absolu de la porte détecté',    # 'INFO_CAPTEUR_PRESENT'
    76: 'buzzer activé',    # 'INFO_BUZZER_ACTIVE'
    77: 'buzzer désactivé',    # 'INFO_BUZZER_DESACTIVE'
    80: 'impossible de changer de consigne pendant le pilotage : le système doit être au repos',    # 'ERR_CONSIGNE_EN_PILOTAGE'
    81: 'mode de pilotage PWM',    # 'DATA_MODE_PWM'
    82: 'mode de pilotage en position',    # 'DATA_MODE_POSITION'
    83: 'mode de pilotage en vitesse',    # 'DATA_MODE_VITESSE'
    84: 'mode de pilotage en vitesse et position',    # 'DATA_MODE_PV'
    90: 'PID vitesse :  Kp = $.3',    # 'DATA_PID_VITESSE_KP'
    91: 'PID vitesse :  Ki = $.3',    # 'DATA_PID_VITESSE_KI'
    92: 'PID vitesse :  Kd = $.3',    # 'DATA_PID_VITESSE_KD'
    93: 'PID position : Kp = $.3',    # 'DATA_PID_POSITION_KP'
    94: 'PID position : Ki = $.3',    # 'DATA_PID_POSITION_KI'
    95: 'PID position : Kd = $.3',    # 'DATA_PID_POSITION_KD'
    100: 'debut du mode test (DO STOP pour quitter)',    # 'INFO_TEST_DEBUT'
    101: 'sequence test terminée, entrées toujours surveillées (DO STOP pour quitter)',    # 'INFO_TEST_CAPTEURS_FIN'
    102: 'fin du mode test',    # 'INFO_TEST_FIN'
    103: 'mode test toujours actif : DO STOP pour quitter',    # 'INFO_TEST_ACTIF'
    104: 'état des entrées : $',    # 'DATA_TEST_ENTREES'
    105: 'état des sorties : $',    # 'DATA_TEST_SORTIES'
    110: 'envoi des mesures désactivé',    # 'INFO_MESURES_DESACTIVE'
    111: 'envoi des mesures toutes les $ ms',    # 'DATA_FREQ_MESURES'
    112: 'angle de la porte $.2°',    # 'DATA_ANGLE_PORTE'
    113: "tension d'alimentation $.2V",    # 'DATA_TENSION'
    114: 'courant consommé $.2A',    # 'DATA_COURANT'
    115: 'angle moteur $.2°',    # 'DATA_ANGLE_MOTEUR'
    116: 'vitesse moteur $.2 rad/s',    # 'DATA_VITESSE_MOTEUR'
    117: 'PWM moteur $.2',    # 'DATA_PWM'
    118: 'consigne moteur $.2 (PWM,°,rad/s)',    # 'DATA_CONSIGNE'
    119: 'servodrive monté sur le meuble',    # 'INFO_SUR_MEUBLE'
    120: 'servodrive non monté sur le meuble',    # 'INFO_DEMONTE'
    121: 'capteur BN0055 absent',    # 'INFO_BN0055_ABSENT'
    122: 'capteur BN0055 détecté',    # 'INFO_BN0055_PRESENT'
    250: 'etat de prod',    # 'ETAT_PROD'
    251: 'etat de calibration',    # 'ETAT_CALIBRATION'
}

ETAT_PROD = {
    0: "INIT",
    1: "REPOS",
    2: "FONCTIONNEMENT",
    3: "OUVERTURE",
    4: "FERMETURE",
    5: "PILOTAGE",
    6: "CALIBRATION",
    7: "DEBRAYAGE",
    8: "ERREUR",
    9: "STOP",
}

ETAT_CALIBRATION = {
    20: "INIT",
    21: "OUVERTURE",
    22: "ATTENTE_HAUT",
    23: "BUTEE_BASSE",
    24: "ATTENTE_BAS",
    25: "BUTEE_HAUTE",
    26: "ENREGISTREMENT",
    27: "ERREUR",
    28: "NONE",
}
