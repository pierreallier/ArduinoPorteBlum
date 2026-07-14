#ifndef SERIALTXBUFFER_H
#define SERIALTXBUFFER_H

#include <Arduino.h>

/**
 * @brief Buffer circulaire pour l'envoi non bloquant de messages série.
 *
 * Les messages sont stockés sous la forme :
 *
 *     TYPE;VALEUR\n
 *
 * Exemple :
 *
 *     I;Debut calibration\n
 *
 * La méthode send() transmet uniquement les octets pouvant être
 * immédiatement placés dans le buffer TX du port série.
 */
class SerialTxBuffer{
  public:
    /**
     * @brief 
     *
     * @return Référence vers l'instance du singleton.
     */
    static SerialTxBuffer& instance();


    /**
     * @brief Ajoute un message dans le buffer.
     *
     * Le type, le séparateur ';' et le caractère de fin de ligne '\n'
     * sont automatiquement ajoutés.
     *
     * Exemple :
     *
     *     push(INFO, "Debut calibration");
     *
     * ajoute dans le buffer :
     *
     *     I;Debut calibration\n
     *
     * @param type Type du message.
     * @param value Chaîne de caractères à transmettre.
     *
     * @return true si le message a été ajouté.
     * @return false si le buffer ne contient pas suffisamment d'espace.
     */
    bool push(char type, const char* value);

    /**
    * @brief Ajoute un message composé de trois champs au buffer.
    *
    * Format :
    *     TYPE;COMMANDE;CIBLE;MESSAGE\n
    *
    * @param type Type du message.
    * @param commande Commande concernée.
    * @param cible Cible de la commande.
    * @param message Message associé.
    *
    * @return true si le message a été ajouté au buffer.
    * @return false si l'espace disponible est insuffisant.
    */
    bool push(char type, const char* commande, const char* cible, const char* message);


    /**
     * @brief Transmet les données disponibles vers le port série.
     *
     * Seul le nombre d'octets pouvant être immédiatement placés
     * dans le buffer TX du port série est transmis.
     *
     * Les données restantes seront transmises lors des appels suivants.
     *
     * @param serial Port série utilisé pour la transmission.
     */
    void send(HardwareSerial& serial);


    /**
     * @brief Indique si le buffer est vide.
     *
     * @return true si aucune donnée n'est en attente.
     */
    bool isEmpty() const;


    /**
     * @brief Retourne le nombre d'octets actuellement stockés.
     *
     * @return Nombre d'octets utilisés dans le buffer.
     */
    size_t size() const;


    /**
     * @brief Retourne l'espace disponible dans le buffer.
     *
     * @return Nombre d'octets pouvant encore être ajoutés.
     */
    size_t available() const;


  private:

    /**
     * @brief Taille du buffer circulaire.
     *
     * Un octet est toujours conservé libre afin de distinguer
     * un buffer vide d'un buffer plein.
     */
    static constexpr size_t BUFFER_SIZE = 512;


    /**
     * @brief Buffer contenant les données en attente de transmission.
     */
    char buffer[BUFFER_SIZE];


    /**
     * @brief Position du prochain octet à transmettre.
     */
    size_t readIndex = 0;


    /**
     * @brief Position du prochain octet à écrire.
     */
    size_t writeIndex = 0;


    /**
     * @brief Constructeur privé du singleton.
     */
    SerialTxBuffer() = default;


    /**
     * @brief Ajoute un octet dans le buffer circulaire.
     *
     * L'espace disponible doit avoir été vérifié avant l'appel.
     *
     * @param value Octet à ajouter.
     */
    void pushByte(char value);


    // Interdiction de copier le singleton.

    SerialTxBuffer(const SerialTxBuffer&) = delete;

    SerialTxBuffer& operator=(const SerialTxBuffer&) = delete;
};

#endif