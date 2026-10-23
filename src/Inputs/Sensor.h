#ifndef SENSOR_H
#define SENSOR_H

# include <Arduino.h>

template <class T>
class Sensor {
    
    public:
        Sensor(uint8_t pin, float coeff, T offset = 0) : _pin(pin), _coeff(coeff), _offset(offset), _value(0) {
            pinMode(_pin, INPUT);
        }
        
        void init() {};
        inline int readRaw() { return analogRead(_pin); }
        T read() {_value = static_cast<T>((static_cast<float>(readRaw()) - _offset) * _coeff ); return _value;}
        T get() const { return _value;}
        void reset() {_value = 0;}
        void setOffset(T offset) {_offset = offset;}
        T getOffset() const { return _offset;}
        float getCoeff() const { return _coeff;}
    
    protected:
        uint8_t _pin;
        float _coeff;
        T _offset;
        T _value;
};

#endif