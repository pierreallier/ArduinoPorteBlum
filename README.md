# ArduinoPorteBlum

Programme Arduino pour piloter et controler un servomoteur BLUM qui permet de motoriser un mécanisme d'ouverture de meubles haut de cuisine.

Basé sur un graphe d'état, avec détection des obstacles.

Grandeurs mesurées :
- Tension d'alimentation
- Commande PWM
- Courant moyen consommé
- Angle absolu de la porte
- Angle relatif et vitesse de rotation du moteur 

Commandé à partir de 4 boutons :
- un bouton sans fil et 1 bouton physique = fonctionnement normal du système
- 2 boutons physiques = étallonnage de la porte / pilotage via un ordinateur
- un bouton reset

Retours visuels (LED) :
- Moteur en fonctionnement
- Mode piloté activé
- Erreurs (limite de courant, blocage)
- Etalonnage (en cours : allumé constant, requis : clignotement, étalonné : éteint)

TODO:
- Tester tous les asservissements.
- Tester tous les types de consignes type echelon, trapèze, sinus ...
- amélioration code