#include "ContextualLLBPredictor.h"
#include <algorithm>
#include <cstdlib>

ContextualLLBPredictor::ContextualLLBPredictor(size_t historyLength_,
                                               size_t tableSize_)
  : historyLength(historyLength_),
    tableSize(tableSize_),
    globalLRU(0)
{
    mask = static_cast<uint32_t>(tableSize - 1);
    table.resize(tableSize);
    for (auto &e : table) {
        e.tag     = 0;
        e.history = 0;
        e.counter = 0;
        e.lru     = 0;
    }
}

ContextualLLBPredictor::~ContextualLLBPredictor() {}

bool ContextualLLBPredictor::predict(uint32_t pc, uint64_t contextHash = 0) {
    uint32_t idx = getIndex(pc, contextHash);
    const auto &e = table[idx];
    if (e.tag == (pc & mask))
        return e.counter >= 0;
    return true;  // default taken
}

void ContextualLLBPredictor::update(uint32_t pc,
                                    uint64_t contextHash,
                                    bool taken)
{
    ++globalLRU;
    uint32_t idx = getIndex(pc, contextHash);
    auto &e = table[idx];
    uint32_t tag = pc & mask;

    if (e.tag == tag) {
        // update counter
        int c = e.counter + (taken ? 1 : -1);
        e.counter = static_cast<int8_t>(std::max(-4, std::min(3, c)));
        // update history
        e.history = static_cast<uint8_t>(
            ((e.history << 1) | static_cast<uint8_t>(taken))
            & ((1u << historyLength) - 1));
        e.lru = globalLRU;
    } else {
        // misprediction under default‐taken => allocate
        if (true != taken) {
            allocateEntry(idx, tag, taken);
        }
    }
}

int ContextualLLBPredictor::confidence(uint32_t pc,
                                       uint64_t contextHash) const
{
    uint32_t idx = getIndex(pc, contextHash);
    const auto &e = table[idx];
    if (e.tag == (pc & mask))
        return std::abs(e.counter);
    return 0;
}

void ContextualLLBPredictor::allocateResources(uint32_t pc) {
    uint32_t idx = getIndex(pc, 0);
    auto &e = table[idx];
    e.tag     = pc & mask;
    e.counter = 0;
    e.history = 0;
    e.lru     = globalLRU;
}