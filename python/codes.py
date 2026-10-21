"""
Fichier généré automatiquement.
Ne pas modifier manuellement.

Source : etats.csv + codes.csv
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
    45: "période d'envoi des mesures enregistrée à : $ ms",    # 'INFO_MESURES_SAUVEGARDE'
    50: 'commande get inconnue : valeur attendue : LIMITS / CALIBRATION / MODE / PID / MESURES / ACCEL / BUZZER',    # 'ERR_CMD_GET_INCONNUE'
    51: 'mode de pilotage erroné',    # 'ERR_MODE_GET'
    52: 'commande get mesures : capteur inconnu',    # 'ERR_MESURE_GET'
    53: 'accéléromètre ${désactivé|activé}',    # 'INFO_ACCEL_GET'
    55: 'buzzer ${désactivé|activé}',    # 'INFO_BUZZER'
    56: 'limite de courant du moteur $.2A',    # 'DATA_LIMITE_COURANT'
    59: 'demande état ACTIONNER',    # 'INFO_DO_FONCTIONNEMENT'
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
    70: '==== Pilotage Porte Blum ====',    # 'INFO_INIT_DEBUT'
    71: '== Initialisation terminée ==',    # 'INFO_INIT_FIN'
    72: "erreur d'initialisation",    # 'ERR_INIT'
    73: 'version de développement',    # 'INFO_DEV'
    74: 'codeur absolu de la porte non détecté',    # 'ERR_CAPTEUR_ABSENT'
    75: 'codeur absolu de la porte détecté',    # 'INFO_CAPTEUR_PRESENT'
    76: 'accéléromètre détecté',    # 'INFO_ACCEL_PRESENT'
    77: 'accéléromètre non détecté',    # 'ERR_ACCEL_ABSENT'
    78: 'buzzer activé',    # 'INFO_BUZZER_ACTIVE'
    79: 'buzzer désactivé',    # 'INFO_BUZZER_DESACTIVE'
    80: 'impossible de changer de consigne pendant le pilotage : le système doit être au repos',    # 'ERR_CONSIGNE_EN_PILOTAGE'
    81: 'mode de pilotage PWM',    # 'DATA_MODE_PWM'
    82: 'mode de pilotage en position',    # 'DATA_MODE_POSITION'
    83: 'mode de pilotage en vitesse',    # 'DATA_MODE_VITESSE'
    84: 'mode de pilotage en vitesse et position',    # 'DATA_MODE_PV'
    85: 'pilotage démarré',    # 'INFO_PILOTAGE_DEMARRE'
    86: 'pilotage terminé',    # 'INFO_PILOTAGE_FINI'
    87: 'pilotage interrompu',    # 'ERR_PILOTAGE_STOPPE'
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
    106: 'test I2C du codeur absolu de la porte : ${KO|OK}',    # 'TEST_I2C_CODEUR_PORTE'
    107: "test I2C de l'accéléromètre : ${KO|OK}",    # 'TEST_I2C_ACCELEROMETRE'
    108: 'test bouton mouvement : ${RELACHE|PRESSE}',    # 'TEST_BT_MOUVEMENT'
    109: 'test bouton wireless : ${RELACHE|PRESSE}',    # 'TEST_BT_WIRELESS'
    110: 'test bouton calibration : ${RELACHE|PRESSE}',    # 'TEST_BT_CALIBRATION'
    111: 'test bouton pilotage : ${RELACHE|PRESSE}',    # 'TEST_BT_PILOTAGE'
    112: 'test LED ERREUR',    # 'TEST_LED_ERROR'
    113: 'test LED MOTEUR',    # 'TEST_LED_MOTOR'
    114: 'test LED CALIBRATION',    # 'TEST_LED_CALIBRATION'
    115: 'test LED PILOTAGE',    # 'TEST_LED_PILOTAGE'
    116: 'test BUZZER',    # 'TEST_BUZZER'
    117: 'test capteur TENSION MOTEUR : $',    # 'TEST_CAPTEUR_TENSION_MOTEUR'
    118: 'test capteur COURANT DRIVER : $',    # 'TEST_CAPTEUR_COURANT_DRIVER'
    119: 'test capteur COURANT MOTEUR : $',    # 'TEST_CAPTEUR_COURANT_MOTEUR'
    120: 'test capteur CODEUR PORTE : $',    # 'TEST_CAPTEUR_CODEUR_PORTE'
    121: 'test capteur DETECTEUR MEUBLE : $',    # 'TEST_CAPTEUR_MEUBLE'
    122: 'test capteur POTENTIOMETRE : $',    # 'TEST_CAPTEUR_POTENTIOMETRE'
    150: 'envoi des mesures désactivé',    # 'INFO_MESURES_DESACTIVE'
    151: 'envoi des mesures toutes les $ ms',    # 'DATA_FREQ_MESURES'
    152: 'angle de la porte $.2°',    # 'DATA_ANGLE_PORTE'
    153: "tension d'alimentation $.2V",    # 'DATA_TENSION'
    154: 'courant moyen consommé $.2A',    # 'DATA_COURANT'
    155: 'angle moteur $.2°',    # 'DATA_ANGLE_MOTEUR'
    156: 'vitesse moteur $.2 rad/s',    # 'DATA_VITESSE_MOTEUR'
    157: 'PWM moteur $.2',    # 'DATA_PWM'
    158: 'consigne moteur $.2 (PWM,°,rad/s)',    # 'DATA_CONSIGNE'
    159: 'servodrive monté sur le meuble',    # 'INFO_SUR_MEUBLE'
    160: 'servodrive non monté sur le meuble',    # 'INFO_DEMONTE'
    161: 'capteur BN0055 absent',    # 'INFO_BN0055_ABSENT'
    162: 'capteur BN0055 détecté',    # 'INFO_BN0055_PRESENT'
    163: 'tension moteur $.2V',    # 'DATA_TENSION_MOTEUR'
    164: 'dernier courant moteur mesuré $.2A',    # 'DATA_LAST_COURANT'
    250: 'etat de prod',    # 'ETAT_PROD'
    251: 'etat de calibration',    # 'ETAT_CALIBRATION'
    252: 'etat de pilotage',    # 'ETAT_PILOTAGE'
}

ETAT_PROD = {
    0: "INIT",
    1: "REPOS",
    2: "FONCTIONNEMENT",
    3: "OUVERTURE",
    4: "FERMETURE",
    5: "PILOTAGE",
    6: "CALIBRATION",
    7: "CALIBRATION_REQUIS",
    8: "DEBRAYAGE",
    9: "ERREUR",
    10: "NO_POWER",
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
    28: "FIN",
}

ETAT_PILOTAGE = {
    40: "INIT",
    41: "EN_COURS",
    42: "ERREUR",
    43: "FIN",
}
