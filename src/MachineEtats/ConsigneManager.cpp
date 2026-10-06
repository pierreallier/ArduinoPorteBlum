#include "ConsigneManager.h"

bool ConsigneManager::setConsigne( const String& type, const String* params, int nbParams) {
    if (type == "POTENTIOMETRE") {
        consigneActive = &consignePotentiometre; // On ignore s'il y a des paramètres en plus
        return true;
    }
    if (type == "ECHELON") {
        if (nbParams < 2 || nbParams > 3)
            return false;
        float valeur = params[0].toFloat();
        unsigned long duree = params[1].toInt();
        unsigned long delai = (nbParams == 3) ? params[2].toInt() : 0;
        consigneEchelon.configure( valeur, duree, delai);
        consigneActive = &consigneEchelon;
        return true;
    }
    if (type == "RAMPE") {
        if (nbParams < 3 || nbParams > 4)
            return false;
        float valeurInitiale = params[0].toFloat();
        float valeurFinale = params[1].toFloat();
        unsigned long duree = params[2].toInt();
        unsigned long delai = (nbParams == 4) ? params[3].toInt() : 0;
        consigneRampe.configure( valeurInitiale, valeurFinale, duree, delai);
        consigneActive = &consigneRampe;
        return true;
    }
    if (type == "TRAPEZE") {
        if (nbParams < 4 || nbParams > 5)
            return false;
        float valeur = params[0].toFloat();
        unsigned long dureeMontee = params[1].toInt();
        unsigned long dureePlateau = params[2].toInt();
        unsigned long dureeDescente = params[3].toInt();
        unsigned long delai = (nbParams == 5) ? params[4].toInt() : 0;
        consigneTrapeze.configure( valeur, dureeMontee, dureePlateau, dureeDescente, delai);
        consigneActive = &consigneTrapeze;
        return true;
    }
    if (type == "SINUS") {
        if (nbParams < 3 || nbParams > 4)
            return false;
        float amplitude = params[0].toFloat();
        unsigned long periode = params[1].toInt();
        unsigned long duree = params[2].toInt();
        float offset = (nbParams == 4) ? params[3].toFloat() : 0.0f;
        consigneSinus.configure( amplitude, periode, duree, offset);
        consigneActive = &consigneSinus;
        return true;
    }
    // Type inconnu
    return false;
}

void ConsigneManager::init(unsigned long t) {
    if (consigneActive != nullptr)
        consigneActive->init(t);
}

float ConsigneManager::get(unsigned long t) {
    return consigneActive->get(t);
}

bool ConsigneManager::ended(unsigned long t) const{
    return consigneActive->ended(t);
}

const char* ConsigneManager::getName() const {
    return consigneActive->getName();
}