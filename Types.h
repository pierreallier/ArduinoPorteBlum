#ifndef TYPES_H
#define TYPES_H

// Structure pour stocker les 3 valeurs d'étalonnage
struct CalibrationData {
    uint16_t  highLimit;   
    uint16_t  lowLimit; 
    uint16_t  offset;
};

#endif