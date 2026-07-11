#define POTENTIOMETRE A2 // Potentiomètre réglage vitesse moteur

class Consigne {
    public:
        virtual void init(unsigned long t) {
            t_init = t;
        }
        virtual float get(unsigned long t) = 0;

        virtual bool ended(unsigned long t);

        virtual ~Consigne() = default;

        virtual String getName();

    protected:
        unsigned long t_init = 0;
};

class ConsignePotentiometre : public Consigne {
    public:
        ConsignePotentiometre(){}

        void init(unsigned long t) override {
            pinMode(POTENTIOMETRE, INPUT);
            t_init = t;
        }

        float get(unsigned long) override {
            int valeur = analogRead(POTENTIOMETRE);
            return (valeur-500)*0.5;
        }

        bool ended(unsigned long) override {
            return false;
        }

        String getName() {
            return "Consigne Potentiometre";
        }
};

class ConsigneEchelon : public Consigne {
    public:
        ConsigneEchelon(float valeur,unsigned long duree, unsigned long delai = 0)
            : valeur(valeur), duree(duree), delai(delai) {}
        
        float get(unsigned long t) override {
            unsigned long dt = t - t_init;
            if (dt < delai)
                return 0.0f;
            if (dt < delai + duree)
                return valeur;
            return 0.0f;
        }

        bool ended(unsigned long t) override {
            return (t - t_init >= delai + duree);
        }

        String getName() {
            return "Consigne Echelon";
        }

    private:
        float valeur;
        unsigned long duree;
        unsigned long delai;
};

class ConsigneRampe : public Consigne {
    public:
        ConsigneRampe(float valeur_initiale,float valeur_finale,unsigned long duree, unsigned long delai = 0)
            : valeur_initiale(valeur_initiale), valeur_finale(valeur_finale), duree(duree), delai(delai) {}
        
        float get(unsigned long t) override {
            unsigned long dt = t - t_init;
            if (dt < delai)
                return 0.0f;
            dt -= delai;
            if (dt >= duree)
                return valeur_finale;
            float progression = (float) dt / duree;
            return valeur_initiale + progression * (valeur_finale - valeur_initiale);
        }

        bool ended(unsigned long t) override {
            return (t - t_init >= delai + duree);
        }

        String getName() {
            return "Consigne Rampe";
        }

    private:
        float valeur_initiale;
        float valeur_finale;
        unsigned long duree;
        unsigned long delai;
};

class ConsigneTrapeze : public Consigne {
    public:
        ConsigneTrapeze(float valeur, unsigned long duree_montee, unsigned long duree_plateau, unsigned long duree_descente, unsigned long delai = 0) 
            : valeur(valeur), duree_montee(duree_montee), duree_plateau(duree_plateau), duree_descente(duree_descente), delai(delai) {}

        float get(unsigned long t) override {
            unsigned long dt = t - t_init;
            // Délai initial
            if (dt < delai)
                return 0.0f;
            dt -= delai;
            // Montée
            if (dt < duree_montee) {
                float progression = (float)dt / duree_montee;
                return valeur * progression;
            }
            dt -= duree_montee;
            // Plateau
            if (dt < duree_plateau)
                return valeur;
            dt -= duree_plateau;
            // Descente
            if (dt < duree_descente) {
                float progression = (float)dt / duree_descente;
                return valeur * (1.0f - progression);
            }
            return 0.0f;
        }

        bool ended(unsigned long t) override {
            return (t - t_init >= delai + duree_montee + duree_plateau + duree_descente);
        }

        String getName() {
            return "Consigne Trapeze";
        }

    private:
        float valeur;
        unsigned long duree_montee;
        unsigned long duree_plateau;
        unsigned long duree_descente;
        unsigned long delai;
};

class ConsigneSinus : public Consigne {
    public:
        ConsigneSinus(float amplitude, unsigned long periode, unsigned long duree, float offset = 0.0f) 
            : amplitude(amplitude), periode(periode), duree(duree), offset(offset) {}

        float get(unsigned long t) override {
            unsigned long dt = t - t_init;
             if (dt >= duree)
                return 0.0f;
            float angle = 2.0f * PI * (float)dt / periode;
            return (offset + amplitude * sin(angle));
        }

        bool ended(unsigned long t) override {
            return (t - t_init >= duree);
        }

        String getName() {
            return "Consigne Sinus";
        }

    private:
        float amplitude;
        unsigned long periode;
        unsigned long duree;
        float offset;
};

