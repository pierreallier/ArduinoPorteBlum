/* Définitions des états du système */
enum class EtatMachine {
    INIT,
    REPOS,
    OUVERTURE,
    FERMETURE,
    PILOTE,
    CALIBRATION,
    DEBRAYAGE,
    ERREUR
};


struct SystemState {
    EtatMachine etat = EtatMachine::INIT;
    bool limiteHaute = false;
    bool limiteBasse = false;

    bool calibre = false;
    float angleMini = 416;
    float angleMaxi = 392;
    float angleZero = 13;
};

SystemState systemState;

/* Codeur porte */
#define CODEUR_PORTE A4

void CodeurPorte_Update() {
    // Lecture du codeur
    // TODO (Détecter ces valeurs par une méthode d'étalonnage du codeur)
    int codeurValue = map(analogRead(CODEUR_PORTE),0,655,0,360.0)-12;
    if (codeurValue > 210)
        codeurValue -= 360;
    systemState.limiteHaute = (codeurValue >= 180) ? HIGH : LOW;
    systemState.limiteBasse = (codeurValue <= -135) ? HIGH : LOW;
    // Serial.print(codeurValue);
    // Serial.print(", limite haute=");
    // Serial.print(systemState.limiteHaute);
    // Serial.print(", limite basse=");
    // Serial.println(systemState.limiteBasse);

    if (systemState.limiteHaute || systemState.limiteBasse) {
        systemState.etat = EtatMachine::ERREUR;
    }
}

/* Boutons de controle */
#define TEST_BT 2 // Bouton de mise en fonctionnement / arrêt
#define WIRELESS_BT 3 // Bouton sans fil 
volatile unsigned prev_time_bt = 0; // Pour éviter l'effot bouncing du bouton

void toogleBt() {
    // BOouton de test
    if (millis() - prev_time_bt >= 250){
        prev_time_bt = millis();
        if (systemState.etat == EtatMachine::REPOS) {
        systemState.etat = EtatMachine::PILOTE;
        } else {
        systemState.etat = EtatMachine::DEBRAYAGE;
        }
    }
}

void wirelessBt() {
    // Bouton sans fil du système réel
    if (millis() - prev_time_bt >= 250){
        prev_time_bt = millis();
        if (systemState.etat == EtatMachine::REPOS) {
            systemState.etat = EtatMachine::PILOTE;
        } else {
            systemState.etat = EtatMachine::DEBRAYAGE;
        }
    }
}


unsigned long timer5ms = 0;
unsigned long timer100ms = 0;

void setup() {
    // Bouton de mise en fonctionnement
    pinMode(TEST_BT, INPUT_PULLUP);
    pinMode(WIRELESS_BT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(TEST_BT), toogleBt, FALLING);
    attachInterrupt(digitalPinToInterrupt(WIRELESS_BT), wirelessBt, FALLING);
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(9600);
    Serial.flush();

    //Motor_Begin();

    systemState.etat = EtatMachine::INIT;

    Serial.println(F("Pilotage Porte Blum"));
}

void loop() {
    unsigned long maintenant = millis();

    //-----------------------------
    // Tâches rapides (5 ms)
    //-----------------------------

    if (maintenant - timer5ms >= 5)
    {
        timer5ms += 5;

        CodeurPorte_Update();
    }

    //-----------------------------
    // Tâches lentes (100 ms)
    //-----------------------------

    if (maintenant - timer100ms >= 100)
    {
        timer100ms += 100;

        //Sensors_UpdateSlow();
        //Communication_Task();
    }
    //-----------------------------
    // Machine d'état principale
    //-----------------------------

    switch(systemState.etat)
    {
        case EtatMachine::INIT:

            //Motor_Stop();
            systemState.etat = EtatMachine::REPOS;
            break;

        case EtatMachine::REPOS:

            break;

        case EtatMachine::OUVERTURE:

            break;

        case EtatMachine::FERMETURE:

            break;

        case EtatMachine::PILOTE:
            Serial.println("Pilotage demandé");
            break;

        case EtatMachine::CALIBRATION:

            // Calibration_Run();

            break;

        case EtatMachine::DEBRAYAGE:
            Serial.println("Arret demandé");
            systemState.etat = EtatMachine::REPOS;
            break;

        case EtatMachine::ERREUR:
            //Motor_Stop();
            break;
    }

    //-----------------------------
    // Driver moteur
    //-----------------------------

    //Motor_Task();
}