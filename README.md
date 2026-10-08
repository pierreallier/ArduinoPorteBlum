# ArduinoPorteBlum

Programme Arduino pour piloter et contrôler un servomoteur BLUM qui permet de motoriser un mécanisme d'ouverture de meubles haut de cuisine. Reproduit le fonctionnement complet du système réel, avec mesures de plusieurs grandeurs physiques et la possibilité de piloter le servomoteur selon des loi de commandes personnalisés avec asservissements.


Basé sur un graphe d'état, avec détection des obstacles.



## Grandeurs mesurées :

* Tension d'alimentation
* Commande PWM moteur
* Courant moyen consommé
* Angle absolu de la porte
* Angle relatif et vitesse de rotation du moteur
* Accéléromètre pour mesure le mouvement de la porte



## Commandé à partir de 4 boutons :

* un bouton sans fil et 1 bouton physique = fonctionnement normal du système
* 1 boutons physiques = étallonnage de la porte
* carte de dev uniquement : 1 bouton pilotage via un potentiomètre
* un bouton reset



## Retours visuels (LED) :

* Etalonnage (en cours : allumé constant, requis : clignotement, étalonné : éteint)
* Moteur en fonctionnement si allumé
* Mode piloté activé si allumé
* Erreurs (limite de courant, blocage)



## Communication port série :

Communication binaire avec l'ordinateur ou textuelle si en mode DEV


Toute ligne envoyée par la carte Arduino démarrent par une lettre et séparé par un point virgules et des informations spécifiques au type de messages (pour S,I,W,E,O,N c'est un code entier + valeur numérique optionnelle) :

* M : mesure
* A : mesure accéléromètre
* S : changement d'état
* I : information
* W : warning
* E : erreur
* O : réponse positive (OK) à une commande de l'ordinateur
* N : réponse négative (NOK) à une commande de l'ordinateur
* R : réponse à une commande système (souvent une réponse intermédiaire) qui sera finalisé via un message O ou N.



L'ordinateur peut envoyer des commandes commençant par :

* SET : définir et envoyer une/des variables à la carte Arduino
* GET : récupérer les valeurs de variables depuis la carte Arduino
* DO : réaliser une action par la carte Arduino

Suit cette commande, après un espace un mot clé \[type], suivi de paramètres séparé par des espace en fonction du \[type] de commande souhaitée.



### Commandes SET :
- COURANT $f : configuration la limite de courant à la valeur $f renseignée (flottant)
- CALIBRATION [ON|OFF|EFFACER] : Active | désactive | efface la mémorisation de la calibration en mémoire 
- MODE [PWM|VITESSE|POSITION|POSITION_VITESSE] : configure le mode de pilotage entre les 4 possibilités 
- CONSIGNE [POTENTIOMETRE|ECHELON|RAMPE|TRAPEZE|SINUS] $ : configure le type de consigne de pilotage parmis les possibiltés (avec des paramètres propres $ en flottant)
    - POTENTIOMETRE : aucune valeur à renseigner
    - ECHELON $val $tf $d : $val=valeur finale de l'échelon, $tf=duré totale, $d=décalage temporel démarrage
    - RAMPE $valInit $valFin $tf $d : $valInit=valeur initiale, $valFin=valeur finale, $tf=duré totale, $d=décalage temporel démarrage
    - TRAPEZE $val $tm $tp $td $d : $val=valeur plateau, $tm=durée montée, $tp=durée plateau, $td=durée descente, $d=décalage temporel démarrage
    - SINUS $amp $p $d $o : $amp=amplitude du sinus, $p=periode du sinus, $d=durée totale, $o=offset en amplitude du signal
- PID [VITESSE|POSITION] $kp $ki $ kd : $kp=gain proportionnel, $ki=gain intégrateur, $kd=gain dérivateur (flottant)
- MESURES [ON|OFF|$i] : active|désactive l'envoi des mesures (sans mémorisation fréquence) ou $i régle la période d'envoi tous les $i ms (arrondis à 5ms) et le mémorise en EEPROM
- ACCEL [ON|OFF] : active|désactive la mesure via l'accéléromètre
- BUZZER [ON|OFF] : active|desactive le buzzer (avec mémorisation du choix en EEPROM)


### Commandes GET :
- CALIBRATION : renvoi l'état de calibration du sytème et si c'est le cas, la course et l'offset en mémoire
- LIMITS : renvoi les limites hautes et basses configurées pour le système
- MESURES [PERIODE|MEUBLE|PORTE|TENSION|COURANT|MOTEUR|PWM|CONSIGNE] : renvoi la période d'envoi des mesures ou la valeur du capteur mesurée à l'instant
- MODE : renvoi le mode de pilotage en cours du système parmis [PWM|VITESSE|POSITION|POSITION_VITESSE]
- PID : renvoi la configuration des deux PID (kp, ki, kd)
- ACCEL : renvoi l'état d'activation ou non de la mesures de grandeur de l'accéléromètre
- BUZZER : renvoi l'état d'activation ou non du buzzer


### Commandes DO :
- RESET : reset la carte Arduino
- INIT : réinitialise la machine à état
- OUVRIR : demande l'ouverture de la porte (sans asservissement)
- FERMER : demande la fermeture de la porte (sans asservissement)
- STOP : demande l'arrêt du mouvement de la porte
- PILOTER : demande à basculer le système en pilotage
- CALIBRATION : demande la calibration du système
- TEST : demande de tester les I/O (à désactiver par DO STOP)


## TODO:

* Tester tous les asservissements.
* Tester tous les types de consignes type echelon, trapèze, sinus ...
* amélioration code et documentation

