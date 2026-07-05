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
    if (millis() - time_precedent >= 50) {
        time_precedent += 50;
        //printMesures();
        sendMesures();
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
    buzzer.sequenceInit();
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
    Serial.print(capteurs.angle_moteur);
    Serial.print(" , VitesseMoteur:"); // Vitesse rotation moteur en rad/s
    Serial.print(capteurs.vitesse_moteur);
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
    Serial.print(capteurs.time_mesures);
    Serial.print(";");
    Serial.print(capteurs.tension);
    Serial.print(";");
    Serial.print(moteur.getPWM());
    Serial.print(";");
    Serial.print(capteurs.courant_moyen);
    Serial.print(";");
    Serial.print(capteurs.angle_moteur);
    Serial.print(";");
    Serial.print(capteurs.vitesse_moteur);
    Serial.print(";");
    Serial.print(capteurs.angle_porte);
    Serial.print(";");
    Serial.print(capteurs.limite_haute);
    Serial.print(";");
    Serial.print(capteurs.limite_basse); 
    Serial.print(";");
    Serial.println(capteurs.blocage_detecte);
}

void ComSerie::sendMessages() {
    while (machine.hasMessage()) {
        Message msg = machine.getMessage();

        switch (msg.type) {
            case Message::Type::ETAT:
                sendEtat(msg.valeur);
                break;

            case Message::Type::INFO:
                Serial.print("I;");
                Serial.println(msg.valeur);
                break;

            case Message::Type::ERREUR:
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

void ComSerie::sendError(String message) {
    Serial.print("E;");
    Serial.println(message);
    buzzer.sequenceErreur();
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
            Serial.println("E;Commande inconnue {SET,GET,DO}.");
        }
    }
}

void ComSerie::_SET(String commande) {
    commande.trim(); // Supprime les espaces et les retours à la ligne
    commande.toUpperCase(); // Convertit la commande
    if (commande == "PILOTAGE") {

    }
    else 
        Serial.println("E;Commande SET inconnue");
}

void ComSerie::_GET(String commande) {
    commande.trim(); // Supprime les espaces et les retours à la ligne
    commande.toUpperCase(); // Convertit la commande
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
    else 
        Serial.println("E;Commande DO inconnue {RESET,INIT,OUVRIR,FERMER,STOP,PILOTER}.");
}