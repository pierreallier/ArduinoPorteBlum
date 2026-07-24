# ArduinoPorteBlum

Programme Arduino pour piloter et controler un servomoteur BLUM qui permet de motoriser un mécanisme d'ouverture de meubles haut de cuisine.

Basé sur un graphe d'état, avec détection des obstacles.

Grandeurs mesurées :
- Tension d'alimentation
- Commande PWM
- Courant moyen consommé
- Angle absolu de la porte
- Angle relatif et vitesse de rotation du moteur 

Commandé à partir de deux boutons :
- un bouton sans fil = fonctionnement normal du système
- 2 boutons physiques = étallonnage de la porte / pilotage via un ordinateur
- un bouton reset

Retours visuels :
- Moteur en fonctionnement
- Mode piloté activé
- Limite de courant activé
- Etalonnage (en cours, allumé constant, a faire : clignotement)

TODO:
- Tester tous les asservissements.
- Tester toues les types de consignes type echelon, trapèze, sinus ...
- Ajouter modification limite en courant 
- Etat arret
- Implémenter code butées à vide à l'aide du courant et stockage en dur ?
    - EEPROM : stocker les limites basses et hautes et autres variables pertinentes.
- Buzzer avec la fonction bip native
- Réorganiser le code et commenter
    - MessageBuffer général unique que toutes les classes peut accéder et qui est lu par ComSerie
     -> Sensors en SensorsManager qui stockes les mesures en entier (*1000) et Moyenne vitesse + structure last_mesures ?
    - Codeur moteur sur A et B (vu que les interruptions ne semblent pas bloquer l'Arduino) 
    - Fichier de configuration (les constantes générales du programme)
    - Fichier avec tous les enums (types)
    - Harmoniser les noms de variables et de fonctions et privatiser ce qui doit être privé et ajouter des get/set
- Ajotuer accéléromètre et autres boutons et led
