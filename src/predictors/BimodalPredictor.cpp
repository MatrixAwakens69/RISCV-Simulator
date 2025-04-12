#include "BimodalPredictor.h"

BimodalPredictor::BimodalPredictor(uint32_t size) {
    table.resize(size, 2); // Initialize to weakly taken (2)
    mask = size - 1;
}

BimodalPredictor::~BimodalPredictor() {}

bool BimodalPredictor::predict(uint32_t pc) {
    uint32_t index = pc & mask;
    return table[index] >= 2;
}

void BimodalPredictor::update(uint32_t pc, bool taken) {
    uint32_t index = pc & mask;
    uint8_t &counter = table[index];
    if (taken) {
        if (counter < 3) counter++;
    } else {
        if (counter > 0) counter--;
    }
}

int BimodalPredictor::confidence(uint32_t pc) const {
    uint32_t index = pc & mask;
    return table[index];
}
