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

TODO:
- Tester tous les asservissements.
- Réfléchir à un format de consignes type echelon, trapèze, sinus ...
- Etat arret
- Implémenter code butées à vide à l'aide du courant et stockage en dur ?
- Buzzer avec la fonction bip native
- Réorganiser le code et commenter
    - MessageBuffer général unique que tous classe peut accéderet qui est lu par ComSerie
    - Fichier de configuration (les constantes générales du programme)
    - Fichier avec tous les enums (types)
    - Harmoniser les noms de variables et de fonctions et privatiser ce qui doit être privé et ajouter des get/set
- Ajotuer accéléromètre et autres boutons
- Ajouter modification limite en courant 