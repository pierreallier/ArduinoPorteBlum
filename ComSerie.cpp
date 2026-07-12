#include "HardwareSerial.h"
#include "ComSerie.h"

#include <avr/wdt.h>

void resetArduino() {
    wdt_enable(WDTO_15MS); // Redémarrage dans ~15 ms
    while (true) {
        // Attendre le reset
    }
}

ComSerie::ComSerie(Motor& m, Sensors& c, StateMachine& s) : moteur(m),capteurs(c),machine(s) {
}

void ComSerie::init() {
    Serial.begin(115200);
    Serial.flush();
    buzzer.init();
}

void ComSerie::task() {
    if (millis() - time_precedent >= 100) {
        time_precedent += 100;
        sendMessages();
        readSerial();
        buzzer.task();
    }
}

void ComSerie::printFinInit() {
    Serial.println("");
    Serial.println(F("==== Pilotage Porte Blum ===="));
    Serial.println("");
    Serial.println("Initialisation terminée");
    Serial.println("");
    Serial.flush();
    buzzer.sequenceInit();
    delay(1000);
}

void ComSerie::printMesures() {
    Serial.print(capteurs.time_mesures);
    Serial.print(" , Tension:"); // Tension en Volt
    Serial.print(capteurs.tension);
    Serial.print(" , PWM:");
    Serial.print(moteur.getPWM());
    Serial.print(" , Intensité:"); // Courant moteur en Ampère
    Serial.print(capteurs.courant_moyen);
    Serial.print(" , AngleMoteur:"); // Angle moteur en degré
    Serial.print(capteurs.angle_moteur * RAD_TO_TURN);
    Serial.print(" , VitesseMoteur:"); // Vitesse rotation moteur en rad/s
    Serial.print(capteurs.vitesse_moteur * RADS_TO_RPM);
    Serial.print(" , AnglePorte:"); // Angle porte en degré
    Serial.print(capteurs.angle_porte);
    Serial.print(" , Potentiomètre:"); // Consigne du potentiomètre en -255/255
    Serial.print(capteurs.potentiometre);
    Serial.print(" , Moteur:");
    Serial.print(moteur.isEnabled() ? "ON" : "OFF");
    Serial.print(" , Direction:");
    switch(moteur.getDirection()) {
        case Motor::DIR::OUVERTURE:
            Serial.print("OUVERTURE");
            break;
        case Motor::DIR::FERMETURE:
            Serial.print("FERMETURE");
            break;
    }
    Serial.print(" , Limites:");
    if (capteurs.blocage_detecte) {
        Serial.println("BB");
    } else {
        Serial.print(capteurs.limite_haute ? "H" : "N");
        Serial.println(capteurs.limite_basse ? "B" : "N");
    }
    
}

void ComSerie::sendMesures() {
    Serial.print("M;");
    Serial.print(capteurs.time_mesures,0);
    Serial.print(";");
    Serial.print(capteurs.tension,2);
    Serial.print(";");
    Serial.print(moteur.getPWM());
    Serial.print(";");
    Serial.print(capteurs.courant_moyen,2);
    Serial.print(";");
    Serial.print(capteurs.angle_moteur,2);
    Serial.print(";");
    Serial.print(capteurs.vitesse_moteur,2);
    Serial.print(";");
    Serial.print(capteurs.angle_porte,2);
    Serial.print(";");
    Serial.println(capteurs.consigne,2);
}

void ComSerie::sendMessages() {
    while (machine.hasMessage()) {
        Message msg = machine.getMessage();

        switch (msg.type) {
            case Message::TYPE::ETAT:
                sendEtat(msg.valeur);
                break;

            case Message::TYPE::INFO:
                sendInfo(msg.valeur);
                break;

            case Message::TYPE::ERREUR:
                sendError(msg.valeur);
                break;
        }
    }
}

void ComSerie::sendEtat(String etat){
    if (etat != "") {
        Serial.print("S;");
        Serial.print(etat);
        if (etat == '0')
            Serial.println(";INIT");
        else if (etat.startsWith("1"))
            Serial.println(";REPOS");
        else if (etat.startsWith("2"))
            Serial.println(";FONCTIONNEMENT");
        else if (etat.startsWith("3"))
            Serial.println(";OUVERTURE");
        else if (etat.startsWith("4"))
            Serial.println(";FERMETURE");
        else if (etat.startsWith("5"))
            Serial.println(";PILOTE");
        else if (etat.startsWith("6"))
            Serial.println(";CALIBRATION");
        else if (etat.startsWith("7"))
            Serial.println(";DEBRAYAGE");
        else if (etat.startsWith("8"))
            Serial.println(";ERREUR");
        else
            Serial.println(";ETAT INCONNU");
    }
}

void ComSerie::sendError(String message, bool buz = false ) {
    Serial.print("E;");
    Serial.println(message);
    if (buz) {
        buzzer.sequenceErreur();
    }
}

void ComSerie::sendInfo(String message) {
    Serial.print("I;");
    Serial.println(message);
}


void ComSerie::readSerial() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim(); // Supprime les espaces et les retours à la ligne

        if (command.startsWith("SET")) {
            _SET(command.substring(3));
        } else if (command.startsWith("GET")) {
            _GET(command.substring(3));
        } else if (command.startsWith("DO")) {
            _DO(command.substring(2));
        } else {
            sendInfo("Commande inconnue {SET,GET,DO}.");
        }
    }
}

void ComSerie::_SET(String commande) {
    commande.trim(); // Supprime les espaces et les retours à la ligne
    commande.toUpperCase(); // Convertit la commande

    if (commande.startsWith("LIMITES")) {
        String valeurs = commande.substring(7);
        valeurs.trim();
        int separateur = valeurs.indexOf(' ');
        if (separateur == -1) {
            sendInfo("SET LIMITES : deux valeurs attendues");
            return;
        }
        float limite_basse = valeurs.substring(0, separateur).toFloat();
        float limite_haute = valeurs.substring(separateur + 1).toFloat();
        capteurs.setLimits(limite_basse, limite_haute);
        sendInfo("Limites modifiees : basse=" +String(limite_basse, 2) +" ; haute=" +String(limite_haute, 2));
        return;
    }
    // Configuration du mode de pilotage
    else if (commande.startsWith("MODE")) {
        String valeur = commande.substring(5);
        valeur.trim();
        if (valeur == "PWM")
            machine.setModePilotage(StateMachine::MODE_PILOTAGE::PWM);
        else if (valeur == "VITESSE")
            machine.setModePilotage(StateMachine::MODE_PILOTAGE::VITESSE);
        else if (valeur == "POSITION")
            machine.setModePilotage(StateMachine::MODE_PILOTAGE::POSITION);
        else if (valeur == "POSITION_VITESSE")
            machine.setModePilotage(StateMachine::MODE_PILOTAGE::POSITION_VITESSE);
        else 
            sendInfo("Mode de pilotage inconnu : " + valeur);
    }
    // Configuration des PID
    else if (commande.startsWith("CONSIGNE")) {

    }
    // Configuration des PID
    else if (commande.startsWith("PID")) {
        String parametres = commande.substring(4);
        parametres.trim();
        // Recherche des séparateurs
        int sep1 = parametres.indexOf(' ');
        int sep2 = parametres.indexOf(' ', sep1 + 1);
        int sep3 = parametres.indexOf(' ', sep2 + 1);
        if (sep1 == -1 || sep2 == -1 || sep3 == -1) {
            sendInfo("SET PID : TYPE [VITESSE|POSITION] KP KI KD attendus");
            return;
        }
        String type_pid = parametres.substring(0, sep1);
        float kp = parametres.substring(sep1 + 1,sep2).toFloat();
        float ki = parametres.substring(sep2 + 1,sep3).toFloat();
        float kd = parametres.substring(sep3 + 1).toFloat();
        if (type_pid == "VITESSE")
            machine.setPIDVitesse(kp, ki, kd);
        else if (type_pid == "POSITION")
            machine.setPIDPosition(kp, ki, kd);
        else {
            sendInfo("PID inconnu : " + type_pid);
            return;
        }
    }
    // Gestion de la fréquence d'envoie des mesures
    else if (commande.startsWith("MESURES")) {
        String valeur = commande.substring(8);
        valeur.trim();
        // Vérification : chaîne non vide
        if (valeur.length() == 0) {
            sendInfo("SET MESURES : periode manquante");
            return;
        }
        // Vérification : uniquement des chiffres
        for (unsigned int i = 0; i < valeur.length(); i++) {
            if (!isDigit(valeur[i])) {
                sendInfo("SET MESURES : periode invalide");
                return;
            }
        }
        unsigned long periode = valeur.toInt();
        if (periode >= 5) {
            periode_echantillonnage_mesures = periode;
            sendInfo("Envoi des mesures toutes les " + String(periode) + " ms");
        }
        else {
            sendInfo("SET MESURES : periode invalide");
        }
    }
    else 
        sendInfo("Commande SET inconnue {LIMITES,MODE,CONSIGNE,PID,MESURES}");
}

void ComSerie::_GET(String commande) {
    commande.trim(); // Supprime les espaces et les retours à la ligne
    commande.toUpperCase(); // Convertit la commande

    if (commande.startsWith("CALIBRATION")) {
        Serial.print("CALIBRATION;");
        Serial.print(machine.is_calibre);
        Serial.print(";");
        Serial.print(capteurs.getLimiteBasse(), 2);
        Serial.print(";");
        Serial.println(capteurs.getLimiteHaute(), 2);
    }
    if (commande.startsWith("MODE")) {
        Serial.print("MODE;");
        switch (machine.modePilotage) {
            case StateMachine::MODE_PILOTAGE::PWM:
                Serial.println("PWM");
                break;
            case StateMachine::MODE_PILOTAGE::VITESSE:
                Serial.println("VITESSE");
                break;
            case StateMachine::MODE_PILOTAGE::POSITION:
                Serial.println("POSITION");
                break;
            case StateMachine::MODE_PILOTAGE::POSITION_VITESSE:
                Serial.println("POSITION_VITESSE");
                break;
        }
    }
    if (commande.startsWith("PID")) {
        Serial.println("PID;VITESSE;" + (String)machine.pidVitesse.kp + ";" + (String)machine.pidVitesse.ki + ";" + (String)machine.pidVitesse.kd);
        Serial.println("PID;POSITION;" + (String)machine.pidPosition.kp + ";" + (String)machine.pidPosition.ki + ";" + (String)machine.pidPosition.kd);
    }
    else
        sendInfo("Commande GET inconnue {CALIBRATION,MODE,PID}.");
}

void ComSerie::_DO(String commande) {
    commande.trim(); // Supprime les espaces et les retours à la ligne
    commande.toUpperCase(); // Convertit la commande
    if (commande.startsWith("RESET"))
        resetArduino();
    if (commande.startsWith("INIT"))
        machine.changerEtat(StateMachine::ETAT::INIT);
    else if (commande.startsWith("OUVRIR"))
        machine.changerEtat(StateMachine::ETAT::OUVERTURE);
    else if (commande.startsWith("FERMER"))
        machine.changerEtat(StateMachine::ETAT::FERMETURE);
    else if (commande.startsWith("STOP"))
        machine.changerEtat(StateMachine::ETAT::DEBRAYAGE);
    else if (commande.startsWith("PILOTER"))
        machine.changerEtat(StateMachine::ETAT::PILOTAGE);
    else if (commande.startsWith("CALIBRATION"))
        machine.changerEtat(StateMachine::ETAT::CALIBRATION);
    else 
        sendInfo("Commande DO inconnue {RESET,INIT,OUVRIR,FERMER,STOP,PILOTER}.");
}

int ComSerie::mesureEnable() {
    return periode_echantillonnage_mesures;
}