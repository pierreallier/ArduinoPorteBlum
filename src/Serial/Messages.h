#ifndef MESSAGE_H
#define MESSAGE_H

#include "EventQueue.h"
#include "../Config/Constantes.h"
#include "../Config/Codes.h"
#include "../Config/Types.h"

#if VERSION_DEV
    constexpr uint8_t TAILLE_MESSAGE = 20;

    const char* const CALIBRATION_NAMES[] = {
        "CALIBRATION_DEBUT",
        "CALIBRATION_OUVERTURE",
        "CALIBRATION_PAUSEHAUT",
        "CALIBRATION_BUTEEBASSES",
        "CALIBRATION_PAUSEBAS",
        "CALIBRATION_BUTEEHAUTES",
        "CALIBRATION_ENREGISTREMENT",
        "CALIBRATION_ERREUR",
        "CALIBRATION_NONE"
    };

    const char* const ETAT_NAMES[] ={
        "INIT",
        "REPOS",
        "FONCTIONNEMENT",
        "OUVERTURE",
        "FERMETURE",
        "PILOTAGE",
        "CALIBRATION",
        "DEBRAYAGE",
        "ERREUR",
        "STOP"
    };
#else
    constexpr uint8_t TAILLE_MESSAGE = sizeof(Event);
#endif

constexpr uint8_t TAILLE_MESURES = sizeof(Mesures);
constexpr uint8_t TAILLE_MESURES_BN0055 = sizeof(MesuresBN0055);


/*
 * Principe :
 *  - Les messages (hors mesures) sont mis dans buffer circulaire EventQueue (jamais bloquant).
 *  - Chaque mesure est formatée au moment de l'envoi et écrite directement sur
 *    Serial, SEULEMENT si la ligne entière tient dans le buffer TX matériel
 *    (sinon elle est abandonnée : jamais de blocage, jamais de ligne coupée).
 *  - Une mesure envoyée est suivie d'UN message de la file, s'il y en a un.
 *  - Secours : si aucune mesure n'est partie depuis DELAI_SECOURS_MS (mesures
 *    désactivées par SET MESURES, mesures refusées faute de place...),
 *    serviceEvents() vide la file.
 *  - Setup et mode test : pas de mesures, envoi direct avec sendDirect().
 *
 * Contrainte : le buffer TX matériel (SERIAL_TX_BUFFER_SIZE, 64 octets par défaut)
 * doit contenir une ligne. Une ligne plus longue n'est écrite que si le buffer est
 * entièrement libre, et bloque alors quelques ms (voir ecrireLigne).
 */

/**
 * @brief Envoie un message au format TYPE;CODE;VAL.
 
 * @param code Code du message (voir Codes.h)
 * @param val Valeur associée au message (0 si sans objet)
 *
 * @return true si le message a été ajouté au buffer.
 * @return false si l'espace disponible est insuffisant.
 */
inline bool sendMessage(MSG code, int32_t val = 0) { 
    return EventQueue::instance().push('R', code, val); 
}

inline bool sendReponseOK(MSG code, int32_t val = 0) {
    return EventQueue::instance().push('O', code, val);
}
inline bool sendReponseNOK(MSG code, int32_t val = 0) {
    return EventQueue::instance().push('N', code, val);
}

inline bool sendInfo(MSG code, int32_t val = 0) { 
    return EventQueue::instance().push('I', code, val); 
}

inline bool sendWarning(MSG code, int32_t val = 0) { 
    return EventQueue::instance().push('W', code, val); 
}

inline bool sendError(MSG code, int32_t val = 0) { 
    return EventQueue::instance().push('E', code, val); 
}

inline bool sendEtat(MSG code,int32_t val = 0) { 
    return EventQueue::instance().push('S', code, val); 
}

/** 
 * @brief Envoie un message directement sur le port série.
 * Format : TYPE;MESSAGE
 */
inline void sendDirect(char type, const char* message) {
    Serial.write(type);
    Serial.write(';');
    Serial.println(message);
}

/** 
 * @brief Envoie un message directement sur le port série.
 * Format : TYPE;MESSAGE
 */
inline void sendDirect(char type, MSG code,int32_t val = 0) {
    Serial.write(type);
    Serial.write(';');
    Serial.print(msgName(code));
    Serial.write(';');
    Serial.println(val);
}

/**
 * @brief Envoie un message du mode test (directement sur le port série).
 * Format : T;CIBLE;VALEUR
 */
inline void sendTest(const char* cible, const char* valeur) {
    Serial.print(F("T;"));
    Serial.print(cible);
    Serial.write(';');
    Serial.println(valeur);
}
inline void sendTest(const char* cible, long valeur) {
    Serial.print(F("T;"));
    Serial.print(cible);
    Serial.write(';');
    Serial.println(valeur);
}

/**
 * @brief Envoie les mesures.
 *
 * @param Mesures via SensorsManager::getMesures() 
 *
 * @return true si le message a été envoyé.
 * @return false si non.
 */
inline bool sendMesures(Mesures message) {
    if (Serial.availableForWrite() >= TAILLE_MESURES) {
        #if VERSION_DEV
        Serial.print(F("M;"));
        Serial.print(message.time);
        Serial.print(";");
        Serial.print(message.tension / 100.0f);
        Serial.print(";");
        Serial.print(message.courant_moyen / 100.0f);
        Serial.print(";");
        Serial.print(message.angle_porte / 100.0f);
        Serial.print(";");
        Serial.print(message.angle_moteur / 100.0f);
        Serial.print(";");
        Serial.print(message.vitesse_moteur / 100.0f);
        Serial.print(";");
        Serial.print(message.pwm / 100.0f);
        Serial.print(";");
        Serial.println(message.consigne / 100.0f);
        #else
        Serial.write((uint8_t*)&message, TAILLE_MESURES);
        #endif
        return true;
    }
    return false;
}


/** 
 * @brief Envoie les mesures du capteur BN0055.
 * 
 * @param MesuresBN0055 via BN0055::getMesures() 
 * 
 * @return true si le message a été envoyé.
 * @return false si non.
 */
inline bool sendMesures(MesuresBN0055 message) {
    if (Serial.availableForWrite() >= TAILLE_MESURES_BN0055) {
        #if VERSION_DEV
        Serial.print(F("A;"));
        Serial.print(message.time);
        Serial.print(";");
        Serial.println(message.accelX);
        #else
        Serial.write((uint8_t*)&message, TAILLE_MESURES_BN0055);
        #endif
        return true;
    } 
    return false;
}

/**
 * @brief Envoi un ou plusieurs messages (si buffer disponible)
 * 
 * @return true si le message a été envoyé.
 * @return false si non.
 */
inline bool sendEvents() {
    auto& queue = EventQueue::instance();

    int dispo = Serial.availableForWrite() / TAILLE_MESSAGE;
    if (dispo > queue.size())
        dispo = queue.size();
    if (queue.perdus() > 0) {
        dispo -= 1;
        #if VERSION_DEV
            Serial.print(F("W;"));
            Serial.print(msgName(MSG::ERR_MESSAGE_PERDU));
            Serial.print(";");
            Serial.println(queue.perdus());
        #else
            Event e ; 
            e.type = 'W';
            e.code = MSG::ERR_MESSAGE_PERDU;
            e.val  = queue.perdus();
            Serial.write((uint8_t*)&e,TAILLE_MESSAGE);
        #endif
        queue.clearPerdu();
    }
    if (dispo > 0) {
        for (int i=0;i<dispo;i++) {
            #if VERSION_DEV
                Event data = queue.peek();
                if (data.type == 'S') {
                    if (data.code == MSG::ETAT_CALIBRATION) {
                        Serial.print(F("S;CALIBRATION;"));
                        Serial.println(CALIBRATION_NAMES[data.val]);
                    } else if (data.code == MSG::ETAT_PROD) {
                        Serial.print(F("S;ETAT;"));
                        Serial.println(ETAT_NAMES[data.val]);
                    }
                } else {
                    Serial.print(data.type);
                    Serial.print(";");
                    Serial.print(msgName(data.code));
                    Serial.print(";");
                    Serial.println(data.val);
                }
            #else
                Serial.write((uint8_t*)&queue.peek(),TAILLE_MESSAGE);
            #endif
            queue.pop();
        }
        return true;
    }
    return false;
}

#endif