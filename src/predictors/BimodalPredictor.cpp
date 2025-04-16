#include "BimodalPredictor.h"

BimodalPredictor::BimodalPredictor(uint32_t size) {
    table.resize(size, 2);   // weakly taken
    chooser.resize(size, 2); // weakly favor TAGE
    mask = size - 1;
}

BimodalPredictor::~BimodalPredictor() {}

bool BimodalPredictor::predict(uint32_t pc) {
    uint32_t idx = pc & mask;
    return table[idx] >= 2;
}

void BimodalPredictor::update(uint32_t pc, bool taken) {
    uint32_t idx = pc & mask;
    uint8_t &ctr = table[idx];
    if (taken) {
        if (ctr < 3) ctr++;
    } else {
        if (ctr > 0) ctr--;
    }
}

int BimodalPredictor::confidence(uint32_t pc) const {
    uint32_t idx = pc & mask;
    return table[idx];
}

bool BimodalPredictor::chooseTage(uint32_t pc) const {
    uint32_t idx = pc & mask;
    return chooser[idx] >= 2;
}

void BimodalPredictor::updateChooser(uint32_t pc,
                                     bool tagePred,
                                     bool percPred,
                                     bool actual) {
    if (tagePred == percPred) return;
    uint32_t idx = pc & mask;
    uint8_t &c = chooser[idx];
    if (tagePred == actual) {
        if (c < 3) c++;
    } else if (percPred == actual) {
        if (c > 0) c--;
    }
}
