#include "ContextualLLBPredictor.h"

ContextualLLBPredictor::ContextualLLBPredictor(size_t historyLength)
    : historyLength(historyLength) {}

ContextualLLBPredictor::~ContextualLLBPredictor() {}

bool ContextualLLBPredictor::predict(uint32_t pc) {
    auto it = historyTable.find(pc);
    if (it == historyTable.end()) {
        return true;
    }
    return it->second.counter >= 2;
}

void ContextualLLBPredictor::update(uint32_t pc, bool taken) {
    updateHistory(pc, taken);
    updateCounter(pc, taken);
}

int ContextualLLBPredictor::confidence(uint32_t pc) const {
    auto it = historyTable.find(pc);
    if (it == historyTable.end()) {
        return 0;
    }
    return it->second.counter;
}

void ContextualLLBPredictor::allocateResources(uint32_t pc) {
    // Retrieve the branch history for the given PC
    auto& history = historyTable[pc];

    // Implement a strategy to adjust the counter based on misprediction history
    // For example, increase the counter for branches that are frequently mispredicted
    if (history.counter < 3) {
        history.counter++;
    } else {
        history.counter = 3; // Saturate the counter
    }

    // Optionally, adjust the history length to capture longer branch patterns
    if (historyLength < 32) {
        historyLength++;
    }
}

uint8_t ContextualLLBPredictor::getHistory(uint32_t pc) const {
    auto it = historyTable.find(pc);
    if (it == historyTable.end()) {
        return 0;
    }
    return it->second.history;
}

void ContextualLLBPredictor::updateHistory(uint32_t pc, bool taken) {
    auto& entry = historyTable[pc];
    entry.history = ((entry.history << 1) | static_cast<uint8_t>(taken)) & ((1 << historyLength) - 1);
}

void ContextualLLBPredictor::updateCounter(uint32_t pc, bool taken) {
    auto& entry = historyTable[pc];
    if (taken) {
        if (entry.counter < 3) {
            entry.counter++;
        }
    } else {
        if (entry.counter > 0) {
            entry.counter--;
        }
    }
}