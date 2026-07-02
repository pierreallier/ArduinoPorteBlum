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
    Serial.print(mesures.time_mesures);
    Serial.print(" , U (V):");
    Serial.print(mesures.tension);
    Serial.print(" , PWM:");
    Serial.print(moteur.pwm);
    Serial.print(" , I (A):");
    Serial.print(mesures.courant_moyen);
    Serial.print(" , Am (deg):");
    Serial.print(mesures.angle_moteur);
    Serial.print(" , Wm (imp/10ms):");
    Serial.print(mesures.vitesse_moteur);
    Serial.print(" , Ap (deg):");
    Serial.print(mesures.angle_porte);
    Serial.print(" , P:");
    Serial.print(mesures.potentiometre);
    Serial.print(" , E:");
    switch(etat) {
        case EtatMachine::INIT:
            Serial.print("INIT");
            break;
        case EtatMachine::REPOS:
            Serial.print("REPOS");
            break;
        case EtatMachine::OUVERTURE:
            Serial.print("OUVERTURE");
            break;
        case EtatMachine::FERMETURE:
            Serial.print("FERMETURE");
            break;
        case EtatMachine::PILOTE:
            Serial.print("PILOTE");
            break;
        case EtatMachine::CALIBRATION:
            Serial.print("CALIBRATION");
            break;
        case EtatMachine::DEBRAYAGE:
            Serial.print("DEBRAYAGE");
            break;
        case EtatMachine::ERREUR:
            Serial.print("ERREUR");
            break;
    }
    Serial.print(" , M:");
    Serial.print(moteur.enabled ? "ON" : "OFF");
    Serial.print(" , D:");
    switch(moteur.direction) {
        case MotorDir::OUVERTURE:
            Serial.print("OUVERTURE");
            break;
        case MotorDir::FERMETURE:
            Serial.print("FERMETURE");
            break;
    }
    Serial.print(" , L:");
    Serial.print(mesures.limite_haute ? "H" : "N");
    Serial.println(mesures.limite_basse ? "B" : "N");
}

void comSerie_sendMesures() {
    Serial.print("M;");
    Serial.print(mesures.time_mesures);
    Serial.print(";");
    Serial.print(mesures.tension);
    Serial.print(";");
    Serial.print(moteur.pwm);
    Serial.print(";");
    Serial.print(mesures.courant_moyen);
    Serial.print(";");
    Serial.print(mesures.angle_moteur);
    Serial.print(";");
    Serial.print(mesures.vitesse_moteur);
    Serial.print(";");
    Serial.print(mesures.angle_porte);
    Serial.print(";");
    Serial.print(mesures.limite_haute);
    Serial.print(";");
    Serial.println(mesures.limite_basse);  
}

void comSerie_SendEtat(EtatMachine etat){
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
}

void comSerie_GET(String commande) {
    commande.trim(); // Supprime les espaces et les retours à la ligne
    commande.toUpperCase(); // Convertit la commande
}

void comSerie_DO(String commande) {
    commande.trim(); // Supprime les espaces et les retours à la ligne
    commande.toUpperCase(); // Convertit la commande
    if (commande == "OUVRIR")
        changerEtat(EtatMachine::OUVERTURE);
    else if (commande == "FERMER")
        changerEtat(EtatMachine::FERMETURE);
    else if (commande == "PILOTER")
        changerEtat(EtatMachine::PILOTE);
    else if (commande == "DEBRAYER")
        changerEtat(EtatMachine::DEBRAYAGE);
    else if (commande == "STOP")
        changerEtat(EtatMachine::DEBRAYAGE);
    else 
        Serial.println("E;Commande DO inconnue {OUVRIR,FERMER,PILOTER,DEBRAYER,STOP}.");
}