#include "ComSerie.h"

void comSerie_Init() {
    Serial.begin(115200);
    Serial.flush();
}

void comSerie_Task() {
    static uint32_t precedent = 0;
    if (millis() - precedent >= 200) {
        precedent += 200;
        comSerie_PrintMesures();
        comSerie_GetCommandes();
    }
}

void comSerie_PrintMesures() {
    Serial.print("Time: ");
    Serial.print(mesures.time_mesures);
    Serial.print(" s, Tension: ");
    Serial.print(mesures.tension);
    Serial.print(" V, Courant moyen: ");
    Serial.print(mesures.courant_moyen);
    Serial.print(" A, Angle moteur: ");
    Serial.print(mesures.angle_moteur);
    Serial.print(" deg, Vitesse moteur: ");
    Serial.print(mesures.vitesse_moteur);
    Serial.print(" imp/10ms, Angle porte: ");
    Serial.print(mesures.angle_porte);
    Serial.print(" deg, Potentiomètre: ");
    Serial.print(mesures.potentiometre);
    Serial.print(", Sur meuble: ");
    Serial.println(mesures.sur_meuble ? "Oui" : "Non");
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
            Serial.println("Commande inconnue.");
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
        Serial.println("Commande DO inconnue.");
}