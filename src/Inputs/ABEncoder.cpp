#include "ABEncoder.h"

volatile int32_t ABEncoder::ticks = 0;
volatile uint8_t ABEncoder::last_state = 0;

int8_t ABEncoder::QUADRATURE[16] = {
    0, -1, +1,  0,
    +1,  0,  0, -1,
    -1,  0,  0, +1,
    0, +1, -1,  0
};