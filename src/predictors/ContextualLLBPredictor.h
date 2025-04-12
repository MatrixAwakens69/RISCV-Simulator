#ifndef CONTEXTUAL_LLBP_PREDICTOR_H
#define CONTEXTUAL_LLBP_PREDICTOR_H

#include <cstdint>
#include <cstddef>
#include <unordered_map>
#include <vector>

class ContextualLLBPredictor {
public:
    ContextualLLBPredictor(size_t historyLength = 16);
    ~ContextualLLBPredictor();

    bool predict(uint32_t pc);
    void update(uint32_t pc, bool taken);
    int confidence(uint32_t pc) const;
    void allocateResources(uint32_t pc);

private:
    struct BranchHistory {
        uint8_t history;
        uint8_t counter;
    };

    size_t historyLength;
    std::unordered_map<uint32_t, BranchHistory> historyTable;

    uint8_t getHistory(uint32_t pc) const;
    void updateHistory(uint32_t pc, bool taken);
    void updateCounter(uint32_t pc, bool taken);
};

#endif // CONTEXTUAL_LLBP_PREDICTOR_H
