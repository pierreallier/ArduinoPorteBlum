#ifndef MESSAGE_H
#define MESSAGE_H

#include "EventQueue.h"
#include "../Config/Constantes.h"
#include "../Config/Codes.h"
#include "../Config/Etats.h"
#include "../Config/Types.h"

enum class MSGTYPE : char {
    NONE = ' ',
    REQUETE = 'R',
    REPONSE_OK = 'O',
    REPONSE_NOK = 'N',
    INFO = 'I',
    WARNING = 'W',
    ERROR = 'E',
    ETAT = 'S',
};

#if VERSION_DEV
    constexpr uint8_t TAILLE_MESSAGE = 20;
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
    return EventQueue::instance().push(static_cast<char>(MSGTYPE::REQUETE), code, val); 
}

inline bool sendReponseOK(MSG code, int32_t val = 0) {
    return EventQueue::instance().push(static_cast<char>(MSGTYPE::REPONSE_OK), code, val);
}
inline bool sendReponseNOK(MSG code, int32_t val = 0) {
    return EventQueue::instance().push(static_cast<char>(MSGTYPE::REPONSE_NOK), code, val);
}

inline bool sendInfo(MSG code, int32_t val = 0) { 
    return EventQueue::instance().push(static_cast<char>(MSGTYPE::INFO), code, val); 
}

inline bool sendWarning(MSG code, int32_t val = 0) { 
    return EventQueue::instance().push(static_cast<char>(MSGTYPE::WARNING), code, val); 
}

inline bool sendError(MSG code, int32_t val = 0) { 
    return EventQueue::instance().push(static_cast<char>(MSGTYPE::ERROR), code, val); 
}

inline bool sendEtat(MSG code,int32_t val = 0) { 
    return EventQueue::instance().push(static_cast<char>(MSGTYPE::ETAT), code, val); 
}

/** 
 * @brief Envoie un message directement sur le port série.
 * Format : TYPE;MESSAGE
 */
inline void sendDirect(MSGTYPE type, const char* message) {
    Serial.write(static_cast<char>(type));
    Serial.write(';');
    Serial.println(message);
}

/** 
 * @brief Envoie un message directement sur le port série.
 * Format : TYPE;CODE;VALEUR
 */
inline void sendDirect(MSGTYPE type, MSG code,int32_t val = 0) {
    #if VERSION_DEV
    Serial.write(static_cast<char>(type));
    Serial.write(';');
    Serial.print(msgName(code));
    Serial.write(';');
    Serial.println(val);
    #else
        Event message;
        message.type = static_cast<char>(type);
        message.code = code;
        message.val  = val;
        Serial.write((uint8_t*)&message, TAILLE_MESSAGE);
    #endif
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
    return true;
    #else
    if (Serial.availableForWrite() >= TAILLE_MESURES) {
        Serial.write((uint8_t*)&message, TAILLE_MESURES);
        return true;
    }
    return false;
    #endif
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
    #if VERSION_DEV
    Serial.print(F("A;"));
    Serial.print(message.time);
    Serial.print(";");
    Serial.println(message.accelX);
    return true;
    #else
    if (Serial.availableForWrite() >= TAILLE_MESURES_BN0055) {
        Serial.write((uint8_t*)&message, TAILLE_MESURES_BN0055);
        return true;
    } 
    return false;
    #endif
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
            Serial.print(static_cast<char>(MSGTYPE::WARNING));
            Serial.print(";");
            Serial.print(msgName(MSG::ERR_MESSAGE_PERDU));
            Serial.print(";");
            Serial.println(queue.perdus());
        #else
            Event e ; 
            e.type = static_cast<char>(MSGTYPE::WARNING);
            e.code = MSG::ERR_MESSAGE_PERDU;
            e.val  = queue.perdus();
            Serial.write((uint8_t*)&e,TAILLE_MESSAGE);
        #endif
        queue.clearPerdu();
    }
    dispo = min(dispo, 3); // on ne vide jamais plus de 10 messages d'un coup
    if (dispo > 0) {
        for (int i=0;i<dispo;i++) {
            #if VERSION_DEV
                Event data = queue.peek();
                if (data.type == static_cast<char>(MSGTYPE::ETAT)) {
                    if (data.code == MSG::ETAT_CALIBRATION) {
                        Serial.print(static_cast<char>(MSGTYPE::ETAT));
                        Serial.print(";CALIBRATION;");
                        Serial.println(etatName(static_cast<ETAT_CALIBRATION>(data.val)));
                    } else if (data.code == MSG::ETAT_PROD) {
                        Serial.print(static_cast<char>(MSGTYPE::ETAT));
                        Serial.print(";ETAT;");
                        Serial.println(etatName(static_cast<ETAT>(data.val)));
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