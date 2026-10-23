#ifndef EXPFILTER_H
#define EXPFILTER_H

#include <Arduino.h>

// Filtre exponentiel adaptatif, independant du capteur et de son unite.
class ExpFilter {
    public:
        // Temps en secondes ; changeFloor dans l'unite du signal.
        // tauStable >= tauDynamic >= 0, changeFloor > 0, changeRatio >= 0.
        ExpFilter(float tauStable, float tauDynamic, float changeFloor, float changeRatio)
            : _tauStable(tauStable), _tauDynamic(tauDynamic),
              _tauRange(tauDynamic - tauStable),
              _changeFloor(changeFloor), _changeRatio(changeRatio) {}

        // Un appel par nouvelle mesure ; le filtre gere le temps ecoule.
        float get(float input) {
            const uint32_t now = micros();
            if (!_initialized) {
                _lastTime = now;
                _value = input;
                _initialized = true;
                _alpha = 1.0f;
                return _value;
            }
            const uint32_t dt = now - _lastTime;
            if (dt == 0) return _value;
            _lastTime = now;
            const float dtSeconds = dt * 0.000001f;
            
            // Calcul de l'alpha adaptatif en fonction de la variation du signal.
            const float error = input - _value;
            const float magnitude = _value < 0.0f ? -_value : _value;
            const float deviation = error < 0.0f ? -error : error;
            const float threshold = _changeFloor + _changeRatio * magnitude;
            // Eviter une division lorsque la variation depasse deja le seuil.
            const float tau = deviation >= threshold ? _tauDynamic
                : _tauStable + (deviation / threshold) * _tauRange;
            _alpha = dtSeconds / (tau + dtSeconds);
            _value += _alpha * error;
            return _value;
        }

        float getAlpha() const { return _alpha; }
        void reset() {
            _value = 0.0f;
            _alpha = 0.0f;
            _initialized = false;
            _lastTime = 0;
        }

    private:
        const float _tauStable;
        const float _tauDynamic;
        const float _tauRange;
        const float _changeFloor;
        const float _changeRatio;
        float _value = 0.0f;
        float _alpha = 0.0f;
        bool _initialized = false;
        uint32_t _lastTime = 0;
};

#endif
