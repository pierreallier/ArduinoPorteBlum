#ifndef MESSAGE_H
#define MESSAGE_H

#include "SerialTxBuffer.h"

/**
 * @brief Envoie un message
 *
 * Format :
 *     R;MESSAGE
 *
 * @param message Message associé à la réponse.
 *
 * @return true si le message a été ajouté au buffer.
 * @return false si l'espace disponible est insuffisant.
 */
inline bool sendMessage(const char* message) {
    return SerialTxBuffer::instance().push('R',  message);
}

/**
 * @brief Envoie une réponse positive à une commande associée à une cible.
 *
 * Format :
 *     O;COMMANDE;CIBLE;MESSAGE
 *
 * Exemple :
 *     O;SET;COURANT;Limite modifiee
 *
 * @param commande Commande exécutée (SET, GET ou DO).
 * @param cible Cible de la commande.
 * @param message Message associé à la réponse.
 *
 * @return true si le message a été ajouté au buffer.
 * @return false si l'espace disponible est insuffisant.
 */
inline bool sendReponseOK(const char* commande, const char* cible, const char* message) {
    return SerialTxBuffer::instance().push('O', commande, cible, message);
}
inline bool sendReponseOK(const char* commande, const char* cible, const String& message){
    return sendReponseOK(commande, cible, message.c_str());
}


/**
 * @brief Envoie une réponse négative à une commande associée à une cible.
 *
 * Format :
 *     N;COMMANDE;CIBLE;MESSAGE
 *
 * Exemple :
 *     N;SET;COURANT;Valeur invalide
 *
 * @param commande Commande exécutée (SET, GET ou DO).
 * @param cible Cible de la commande.
 * @param message Message associé à la réponse.
 *
 * @return true si le message a été ajouté au buffer.
 * @return false si l'espace disponible est insuffisant.
 */
inline bool sendReponseNOK(const char* commande, const char* cible, const char* message){
    return SerialTxBuffer::instance().push('N', commande, cible, message);
}
inline bool sendReponseNOK(const char* commande, const char* cible, const String& message){
    return sendReponseNOK(commande, cible, message.c_str());
}

/**
 * @brief Envoie un message d'information.
 *
 * Format :
 *     I;MESSAGE
 *
 * @param value Message à transmettre.
 *
 * @return true si le message a été ajouté au buffer.
 * @return false si l'espace disponible est insuffisant.
 */
inline bool sendInfo(const char* value) {
    return SerialTxBuffer::instance().push('I', value);
}

/**
 * @brief Envoie un message d'avertissement.
 *
 * Format :
 *     W;MESSAGE
 *
 * @param value Message à transmettre.
 *
 * @return true si le message a été ajouté au buffer.
 * @return false si l'espace disponible est insuffisant.
 */
inline bool sendWarning(const char* value) {
    return SerialTxBuffer::instance().push('W', value);
}

/**
 * @brief Envoie un message d'erreur.
 *
 * Format :
 *     E;MESSAGE
 *
 * @param value Message à transmettre.
 *
 * @return true si le message a été ajouté au buffer.
 * @return false si l'espace disponible est insuffisant.
 */
inline bool sendError(const char* value){
    return SerialTxBuffer::instance().push('E', value);
}



/**
 * @brief Envoie un changement d'état.
 *
 * Format :
 *     S;ETAT;message (optionnel)
 *
 * @param value Nouvel état à transmettre.
 *
 * @return true si le message a été ajouté au buffer.
 * @return false si l'espace disponible est insuffisant.
 */
inline bool sendEtat(const char* value, const char* message=nullptr) {
    return SerialTxBuffer::instance().push('S', value, message);
}

/**
 * @brief Envoie les mesures.
 *
 * @param time le temps de la mesure (en ms)
 * @param tension la tension d'alimentation du système
 * @param pwm la commande moteur
 * @param courant le courant consommée par le moteur
 * @param angle_mooteur l'angle du moteur
 * @param vitesse_moteur la vitesse du moteur (rad/s)
 * @param angle_porte l'angle de la porte (°)
 * @param consigne la consigne du système
 *
 * ATTENTION : il y a un facteur 1000 sur toutes les valeurs sauf le temps (ms)
 *
 * @return true si le message a été envoyé.
 * @return false si non.
 */
inline bool sendMesures(uint32_t time, float tension, int pwm, float courant, float angle_moteur, 
                        float vitesse_moteur, float angle_porte, float consigne) {
    // Serial.print(F("M;"));
    // Serial.print(time);
    // Serial.write(';');
    // Serial.print((int16_t)(tension * 100));
    // Serial.write(';');
    // Serial.print(pwm*100);
    // Serial.write(';');
    // Serial.print((int16_t)(courant * 100));
    // Serial.write(';');
    // Serial.print((int32_t)(angle_moteur * 100));
    // Serial.write(';');
    // Serial.print((int16_t)(vitesse_moteur * 100));
    // Serial.write(';');
    // Serial.print((int16_t)(angle_porte * 100));
    // Serial.write(';');
    // Serial.println((int16_t)(consigne * 100));
    // return true;
    // Crée une chaîne pour les valeurs principales
    char values[128];
    snprintf(values, sizeof(values),
             "%lu;%d;%d;%d;%ld;%d;%d;%d",
             time,
             (int16_t)(tension * 100),
             pwm * 100,
             (int16_t)(courant * 100),
             (int32_t)(angle_moteur * 100),
             (int16_t)(vitesse_moteur * 100),
             (int16_t)(angle_porte * 100),
             (int16_t)(consigne * 100));

    // Envoie via le buffer
    return SerialTxBuffer::instance().push('M', values);
}

#endif