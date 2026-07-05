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
- bouton physique = étallonnage de la porte / pilotage via un ordinateur

TODO:
- Détection besoin étalonnage si moteur tourne et pas la porte dans la vérification des blocages.
- Etallonnage de la porte avec le mécanisme de détection des obstacles pour étalonner le capteur angulaire.
- Ajout des asservissements.
- Envoyer la consignes (différent du PWM) dans les mesures.
- Réfléchir à un format de consignes type echelon, trapèze, sinus ...
