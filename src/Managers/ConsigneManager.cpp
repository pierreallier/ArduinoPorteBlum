#include "ConsigneManager.h"

bool ConsigneManager::setConsigne( const String& type, const String* params, int nbParams) {
    if (type == "POTENTIOMETRE") {
        consigneActive = &consignePotentiometre; // On ignore s'il y a des paramètres en plus
        return true;
    }
    if (type == "ECHELON") {
        if (nbParams < 2 || nbParams > 3) {
            sendMessage(MSG::ERREUR_CONSIGNE_PARAMETRES);
            return false;
        }
        // Lecture des paramètres
        float valeur = params[0].toFloat();
        long duree = params[1].toInt();
        long delai = (nbParams == 3) ? params[2].toInt() : 0;
        // Validation des paramètres
        if (delai < 0L) delai = 0L;
        if (duree < 100L || duree > 10000L || delai > 10000L - duree) {// Durée totale maximale de 10 seconde
            sendMessage(MSG::ERREUR_CONSIGNE_DUREE);
            return false;
        }
        if (!isfinite(valeur)) {
            sendMessage(MSG::ERREUR_CONSIGNE_VALEUR);
            return false;
        }
        consigneEchelon.configure( valeur, duree, delai);
        consigneActive = &consigneEchelon;
        return true;
    }
    if (type == "RAMPE") {
        if (nbParams < 3 || nbParams > 4){
            sendMessage(MSG::ERREUR_CONSIGNE_PARAMETRES);
            return false;
        }
        // Lecture des paramètres
        float valeurInitiale = params[0].toFloat();
        float valeurFinale = params[1].toFloat();
        long duree = params[2].toInt();
        long delai = (nbParams == 4) ? params[3].toInt() : 0;
        // Validation des paramètres
        if (delai < 0L) delai = 0L;
        if (duree < 100L || duree > 10000L || delai > 10000L - duree) {// Durée totale maximale de 10 seconde
            sendMessage(MSG::ERREUR_CONSIGNE_DUREE);
            return false;
        }
        if (!isfinite(valeurInitiale) || !isfinite(valeurFinale)) {
            sendMessage(MSG::ERREUR_CONSIGNE_VALEUR);
            return false;
        }
        consigneRampe.configure( valeurInitiale, valeurFinale, duree, delai);
        consigneActive = &consigneRampe;
        return true;
    }
    if (type == "TRAPEZE") {
        if (nbParams < 4 || nbParams > 5) {
            sendMessage(MSG::ERREUR_CONSIGNE_PARAMETRES);
            return false;
        }
        // Lecture des paramètres
        float valeur = params[0].toFloat();
        long dureeMontee = params[1].toInt();
        long dureePlateau = params[2].toInt();
        long dureeDescente = params[3].toInt();
        long delai = (nbParams == 5) ? params[4].toInt() : 0;
        // Validation des paramètres
        if (dureeMontee < 0L) dureeMontee = 0L;
        if (dureePlateau < 0L) dureePlateau = 0L;
        if (dureeDescente < 0L) dureeDescente = 0L;
        if (delai < 0L) delai = 0L;
        if (dureeMontee > 10000L || dureePlateau > 10000L || dureeDescente > 10000L || dureeMontee + dureePlateau + dureeDescente > 10000L - delai) {// Durée totale maximale de 10 seconde
            sendMessage(MSG::ERREUR_CONSIGNE_DUREE);
            return false;
        }
        if (!isfinite(valeur)) {
            sendMessage(MSG::ERREUR_CONSIGNE_VALEUR);
            return false;
        }
        consigneTrapeze.configure( valeur, dureeMontee, dureePlateau, dureeDescente, delai);
        consigneActive = &consigneTrapeze;
        return true;
    }
    if (type == "SINUS") {
        if (nbParams < 3 || nbParams > 4) {
            sendMessage(MSG::ERREUR_CONSIGNE_PARAMETRES);
            return false;
        }
        // Lecture des paramètres
        float amplitude = params[0].toFloat();
        long periode = params[1].toInt();
        long duree = params[2].toInt();
        float offset = (nbParams == 4) ? params[3].toFloat() : 0.0f;
        // Validation des paramètres
        if (duree < 100L || duree > 10000L) {// Durée totale maximale de 10 seconde
            sendMessage(MSG::ERREUR_CONSIGNE_DUREE);
            return false;
        }
        if (periode <= 0L) {
            sendMessage(MSG::ERREUR_CONSIGNE_PERIODE);
            return false;   
        }
        if (!isfinite(amplitude) || !isfinite(offset)) {
            sendMessage(MSG::ERREUR_CONSIGNE_VALEUR);
            return false;
        }
        consigneSinus.configure( amplitude, periode, duree, offset);
        consigneActive = &consigneSinus;
        return true;
    }
    if (type == "TABLEAU") {
        if (nbParams < 1 || nbParams > 2) {
            sendMessage(MSG::ERREUR_CONSIGNE_PARAMETRES);
            return false;
        }
        // Initialisation: SET CONSIGNE TABLEAU <nbPoints>
        if (nbParams == 1) {
            long nbPointsLong = params[0].toInt();
            if (nbPointsLong <= 0L || nbPointsLong > 100L) {
                sendMessage(MSG::ERREUR_CONSIGNE_NBPOINTS);
                return false;
            }
            unsigned int nbPoints = static_cast<unsigned int>(nbPointsLong);
            // Reconstruction complète de l'objet (reset état + nouveau buffer)
            ConsigneTableau consigne;
            if (!consigne.reserve(nbPoints)) {
                sendMessage(MSG::ERREUR_CONSIGNE_MEMOIRE);
                return false;
            }
            consigneTableau = static_cast<ConsigneTableau&&>(consigne);
            consigneActive = &consigneTableau;
            return true;
        }
        // Ajout de point: SET CONSIGNE TABLEAU <time> <valeur>
        if (consigneActive != &consigneTableau) {
            sendMessage(MSG::ERREUR_CONSIGNE_INIT);
            return false;
        }
        if (consigneTableau.size() >= 100) {
            sendMessage(MSG::ERREUR_CONSIGNE_MEMOIRE);
            return false;
        }
        long time = params[0].toInt();
        float valeur = params[1].toFloat();
        if (time < 100L || time > 10000L) {// Durée maximale de 10 seconde
            sendMessage(MSG::ERREUR_CONSIGNE_DUREE);
            return false;
        }
        if (!isfinite(valeur)) {
            sendMessage(MSG::ERREUR_CONSIGNE_VALEUR);
            return false;
        }
        return consigneTableau.append(time, valeur);
    }
    if (type == "TEST") {
        consigneActive = &consigneTest; // On ignore s'il y a des paramètres en plus
        return true;
    }
    // Type inconnu
    return false;
}

void ConsigneManager::init(unsigned long t) { // time en microseconds
    if (consigneActive != nullptr)
        consigneActive->init(t);
}

float ConsigneManager::get(unsigned long t) { // time en microseconds
    return consigneActive->get(t);
}

bool ConsigneManager::ended(unsigned long t) const{ // time en microseconds
    return consigneActive->ended(t);
}

const char* ConsigneManager::getName() const {
    return consigneActive->getName();
}