#include "SerialTxBuffer.h"


SerialTxBuffer& SerialTxBuffer::instance() {
    static SerialTxBuffer instance;
    return instance;
}


bool SerialTxBuffer::push(char type,const char* value) {
    const size_t length = strlen(value);
    const size_t requiredSize = length + 3; // Espace nécessaire : type + ';' + valeur + '\n'
    // Le message est refusé dans son intégralité si le buffer ne contient pas suffisamment d'espace.
    if (requiredSize > available())
        return false;
    pushByte(type); // Ajout du type de message
    pushByte(';'); // Ajout du séparateur
    // Ajout du contenu du message
    for (size_t i = 0; i < length; ++i)
        pushByte(value[i]);
    pushByte('\n'); // Ajout de la fin de ligne
    return true;
}


bool SerialTxBuffer::push(char type, const char* commande, const char* cible, const char* message){
    const size_t commandeLength = strlen(commande);
    const size_t cibleLength = strlen(cible);
    const size_t messageLength = strlen(message);
    const size_t requiredSize = commandeLength + cibleLength + messageLength + 5; // Espace nécessaire : TYPE;COMMANDE;CIBLE;MESSAGE\n
    if (requiredSize > available()) // Le message est refusé dans son intégralité si le buffer ne contient pas suffisamment d'espace
        return false;
    pushByte(type); // Ajout du type de message
    pushByte(';'); 
    // Ajout de la commande
    for (size_t i = 0; i < commandeLength; ++i)
        pushByte(commande[i]);
    pushByte(';');
    // Ajout de la cible
    for (size_t i = 0; i < cibleLength; ++i)
        pushByte(cible[i]);
    pushByte(';');
    // Ajout du message
    for (size_t i = 0; i < messageLength; ++i)
        pushByte(message[i]);
    pushByte('\n'); // Ajout de la fin de ligne
    return true;
}


void SerialTxBuffer::send(HardwareSerial& serial) {
    if (isEmpty()) // Rien à transmettre
        return;
    const size_t serialAvailable = serial.availableForWrite(); // Nombre d'octets pouvant être immédiatement placés dans le buffer TX du port série
    if (serialAvailable == 0)
        return;
    // Calcul du nombre d'octets contigus disponibles à partir de readIndex
    size_t contiguousBytes; 
    if (readIndex < writeIndex)
        contiguousBytes = writeIndex - readIndex;
    else
        contiguousBytes = BUFFER_SIZE - readIndex;
    const size_t bytesToSend = min(serialAvailable, contiguousBytes); // Nombre d'octets pouvant être envoyés sans attendre
    const size_t bytesSent = serial.write( reinterpret_cast<const uint8_t*>(buffer + readIndex), bytesToSend); // Envoi du bloc contigu
    readIndex += bytesSent; // Déplacement de l'index de lecture
    if (readIndex == BUFFER_SIZE) //Gestion du rebouclage du buffer circulaire
        readIndex = 0;
}


bool SerialTxBuffer::isEmpty() const {
    return readIndex == writeIndex;
}


size_t SerialTxBuffer::size() const {
    if (writeIndex >= readIndex)
        return writeIndex - readIndex;
    return BUFFER_SIZE - readIndex + writeIndex;
}


size_t SerialTxBuffer::available() const {
    return BUFFER_SIZE - size() - 1; // Un octet reste toujours inutilisé afin de distinguer l'état vide de l'état plein
}


void SerialTxBuffer::pushByte(char value) {
    buffer[writeIndex] = value; // Écriture de l'octet
    ++writeIndex; // Avancement de l'index
    if (writeIndex == BUFFER_SIZE) // Gestion du rebouclage du buffer circulaire
        writeIndex = 0;
}