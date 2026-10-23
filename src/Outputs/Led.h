#ifndef LED_H
#define LED_H

#include <Arduino.h>

/* Classe représentant une LED
 * Permet de gérer l'état d'une LED (allumée ou éteinte) et de la contrôler via un pin Arduino.
 * 
 * Exemple d'utilisation :
 * Led led(LED_PIN);
 * led.init(); // Initialise la LED (pin en sortie et éteinte)
 * led.on(); // Allume la LED
 * led.toggle(); // Change l'état de la LED (allumée -> éteinte ou éteinte -> allumée)
 * led.getState(); // Retourne l'état actuel de la LED (true si allumée, false si éteinte)
 * 
 * La méthode update() doit être appelée régulièrement dans la boucle principale pour gérer le clignotement.
 */

class Led {
    public:
        Led(uint8_t pin) : pin(pin), state(false) {}
        void init() { pinMode(pin, OUTPUT); digitalWrite(pin, LOW); state = false; }
        void setState(bool state) { this->state = state; digitalWrite(pin, state ? HIGH : LOW); }
        void on() { setState(true); }
        void off() { setState(false); }
        void toggle() { setState(!state); }
        bool getState() const { return state; }
        
        void blink(uint16_t interval_ms, uint16_t duration_ms=0) { 
            // Fait clignoter la LED pendant une durée donnée avec un intervalle donné
            blink_start_time = millis();
            blink_duration = duration_ms;
            blink_interval = interval_ms;
            last_toggle_time = blink_start_time;
            blinking = true;
            blink_prev_state = state; // Sauvegarde l'état précédent pour le restaurer à la fin du clignotement
        }
        void stopBlinking() {
            blinking = false;
            setState(blink_prev_state); // Restaure l'état précédent à la fin du clignotement
        }

        void update() {
            if (blinking) {
                uint32_t now = millis();
                if (blink_duration != 0 && now - blink_start_time >= blink_duration) {
                    // Fin du clignotement
                    blinking = false;
                    setState(false); // Éteint la LED à la fin du clignotement
                } else if (now - last_toggle_time >= blink_interval) {
                    // Change l'état de la LED
                    toggle();
                    last_toggle_time = now;
                }
            }
        }

    private:
        uint8_t pin;
        bool state;

        // Pour le clignotement
        bool blinking = false;
        uint32_t blink_start_time = 0;
        uint16_t blink_duration = 0;
        uint16_t blink_interval = 0;
        uint32_t last_toggle_time = 0;
        bool blink_prev_state = false; // État précédent avant le clignotement
};

#endif