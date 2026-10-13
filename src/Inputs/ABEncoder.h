#ifndef ABENCODER_H
#define ABENCODER_H

#include <Arduino.h>

static constexpr float DEF_RAD_PER_TICK = 2.0f * PI / 128.0f * 100;

// Compile-time mode selection: define one of these (default QUADRATURE)
// - ABENC_MODE_UNIPOLAR : count only A rising edges
// - ABENC_MODE_BIPOLAR  : count A both edges
// - ABENC_MODE_QUADRATURE: count edges on both A and B (default)
#if !defined(ABENC_MODE_UNIPOLAR) && !defined(ABENC_MODE_BIPOLAR) && !defined(ABENC_MODE_QUADRATURE)
#define ABENC_MODE_QUADRATURE
#endif

class ABEncoder {
    public:
        ABEncoder(float coeff) {
            _coeff = coeff;
            pinMode(19, INPUT_PULLUP); // A = PD3
            pinMode(18, INPUT_PULLUP); // B = PD2
            #if defined(ABENC_MODE_UNIPOLAR)
                attachInterrupt(digitalPinToInterrupt(19), ABEncoder::isr, RISING);
            #elif defined(ABENC_MODE_BIPOLAR)
                attachInterrupt(digitalPinToInterrupt(19), ABEncoder::isr, CHANGE);
            #elif defined(ABENC_MODE_QUADRATURE)
                last_state = ((PIND & _BV(PD3)) ? 2 : 0) | ((PIND & _BV(PD2)) ? 1 : 0);
                attachInterrupt(digitalPinToInterrupt(18), ABEncoder::isr, CHANGE);
                attachInterrupt(digitalPinToInterrupt(19), ABEncoder::isr, CHANGE);
            #endif
        }

        void init() {
            ticks = 0;
            _delta_ticks = 0;
            _nb_ticks = 0;
            _time = micros();
            _last_time = _time;
        }

        void read() {
            _last_time = _time;
            noInterrupts();
            _delta_ticks = ticks;
            _time = micros();
            ticks = 0;
            interrupts();
            _nb_ticks += _delta_ticks;
        } 

        inline int32_t getDeltaTicks() { return _delta_ticks; }
        inline void reset() { _nb_ticks = 0; }
        
        inline uint32_t getTime() { return _time; }
        inline int32_t getAngle() { return static_cast<int32_t>(_nb_ticks * coeff());}
        inline int32_t getDeltaAngle() { return static_cast<int32_t>(_delta_ticks * coeff());} // Variation angulaire
        int32_t getSpeed() {
            uint32_t dt = _time - _last_time;
            if (dt < 10) return 0; // Evite la division par zéro
            int32_t angle = static_cast<int64_t>(getDeltaAngle()) * 1000000LL / dt;
            return angle;
        }
        
    private:
        float _coeff;

        int32_t _delta_ticks = 0;
        uint32_t _time = 0;
        uint32_t _last_time = 0;
        int32_t _nb_ticks = 0; 
        
        static int8_t QUADRATURE[16];
        
        // Gestion interruption du codeur
        static volatile int32_t ticks;
        static volatile uint8_t last_state;
        static void isr() {
            #if defined(ABENC_MODE_UNIPOLAR)
                // Front montant de A : B indique le sens
                ticks += (PIND & _BV(PD2)) ? -1 : +1;
            #elif defined(ABENC_MODE_BIPOLAR)
                ticks += ((PIND & _BV(PD3)) == (PIND & _BV(PD2)) ? -1 : +1);
            #elif defined(ABENC_MODE_QUADRATURE)
                uint8_t state = ((PIND & _BV(PD2)) ? 2 : 0) | ((PIND & _BV(PD3)) ? 1 : 0);
                uint8_t index = (last_state << 2) | state;
                ticks += QUADRATURE[index];
                last_state = state;
            #endif
        }

        #if defined(ABENC_MODE_UNIPOLAR)
            inline float coeff() { return _coeff;}
        #elif defined(ABENC_MODE_BIPOLAR)
            inline float coeff() { return _coeff / 2.0f;}
        #elif defined(ABENC_MODE_QUADRATURE)
            inline float coeff() { return _coeff / 4.0f;}            
        #endif
};

#endif