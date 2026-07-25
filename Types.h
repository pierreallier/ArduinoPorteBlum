#ifndef TYPES_H
#define TYPES_H

// Structure pour stocker les 3 valeurs d'étalonnage
struct CalibrationData {
    uint16_t  highLimit = 0xFFFF;   
    uint16_t  lowLimit = 0; 
    uint16_t  offset = 0;
};

#endif