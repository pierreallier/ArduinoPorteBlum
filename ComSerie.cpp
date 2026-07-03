#include "ComSerie.h"

void comSerie_Init() {
    Serial.begin(115200);
    Serial.flush();
}

void comSerie_Task() {
    static uint32_t precedent = 0;
    if (millis() - precedent >= 200) {
        precedent += 200;
        //comSerie_PrintMesures();
        comSerie_sendMesures();
        comSerie_GetCommandes();
    }
}

void comSerie_PrintMesures() {
    Serial.print(capteurs.time_mesures);
    Serial.print(" , U (V):");
    Serial.print(capteurs.tension);
    Serial.print(" , PWM:");
    Serial.print(moteur.getPWM());
    Serial.print(" , I (A):");
    Serial.print(capteurs.courant_moyen);
    Serial.print(" , Am (deg):");
    Serial.print(capteurs.angle_moteur);
    Serial.print(" , Wm (imp/10ms):");
    Serial.print(capteurs.vitesse_moteur);
    Serial.print(" , Ap (deg):");
    Serial.print(capteurs.angle_porte);
    Serial.print(" , P:");
    Serial.print(capteurs.potentiometre);
    // Serial.print(" , E:");
    // switch(etat) {
    //     case EtatMachine::INIT:
    //         Serial.print("INIT");
    //         break;
    //     case EtatMachine::REPOS:
    //         Serial.print("REPOS");
    //         break;
    //     case EtatMachine::OUVERTURE:
    //         Serial.print("OUVERTURE");
    //         break;
    //     case EtatMachine::FERMETURE:
    //         Serial.print("FERMETURE");
    //         break;
    //     case EtatMachine::PILOTE:
    //         Serial.print("PILOTE");
    //         break;
    //     case EtatMachine::CALIBRATION:
    //         Serial.print("CALIBRATION");
    //         break;
    //     case EtatMachine::DEBRAYAGE:
    //         Serial.print("DEBRAYAGE");
    //         break;
    //     case EtatMachine::ERREUR:
    //         Serial.print("ERREUR");
    //         break;
    // }
    Serial.print(" , M:");
    Serial.print(moteur.isEnabled() ? "ON" : "OFF");
    Serial.print(" , D:");
    switch(moteur.getDirection()) {
        case Motor::DIR::OUVERTURE:
            Serial.print("OUVERTURE");
            break;
        case Motor::DIR::FERMETURE:
            Serial.print("FERMETURE");
            break;
    }
    Serial.print(" , L:");
    Serial.print(capteurs.limite_haute ? "H" : "N");
    Serial.println(capteurs.limite_basse ? "B" : "N");
}

void comSerie_sendMesures() {
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
    Serial.println(capteurs.limite_basse);  
}

void comSerie_SendEtat(StateMachine::ETAT etat){
    Serial.print("S");
    Serial.println((int)etat);
}

void comSerie_GetCommandes() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim(); // Supprime les espaces et les retours à la ligne

        if (command.startsWith("SET")) {
            comSerie_SET(command.substring(3));
        } else if (command.startsWith("GET")) {
            comSerie_GET(command.substring(3));
        } else if (command.startsWith("DO")) {
            comSerie_DO(command.substring(2));
        } else {
            Serial.println("E;Commande inconnue {SET,GET,DO}.");
        }
    }
}

void comSerie_SET(String commande) {
    commande.trim(); // Supprime les espaces et les retours à la ligne
    commande.toUpperCase(); // Convertit la commande
    if (commande == "PILOTAGE") {

    }
    else 
        Serial.println("E;Commande SET inconnue");
}

void comSerie_GET(String commande) {
    commande.trim(); // Supprime les espaces et les retours à la ligne
    commande.toUpperCase(); // Convertit la commande
}

void comSerie_DO(String commande) {
    commande.trim(); // Supprime les espaces et les retours à la ligne
    commande.toUpperCase(); // Convertit la commande
    // if (commande == "OUVRIR")
    //     changerEtat(EtatMachine::OUVERTURE);
    // else if (commande == "FERMER")
    //     changerEtat(EtatMachine::FERMETURE);
    // else if (commande == "PILOTER")
    //     changerEtat(EtatMachine::PILOTE);
    // else if (commande == "DEBRAYER")
    //     changerEtat(EtatMachine::DEBRAYAGE);
    // else if (commande == "STOP")
    //     changerEtat(EtatMachine::DEBRAYAGE);
    // else 
    //     Serial.println("E;Commande DO inconnue {OUVRIR,FERMER,PILOTER,DEBRAYER,STOP}.");
}