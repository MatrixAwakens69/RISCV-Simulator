#ifndef CONTEXTUAL_LLBP_PREDICTOR_H
#define CONTEXTUAL_LLBP_PREDICTOR_H

#include <cstdint>
#include <cstddef>
#include <unordered_map>
#include <vector>

class ContextualLLBPredictor {
public:
    // historyLength: bits of local history per entry (e.g. 16)
    // tableSize: must be power of two (e.g. 65536)
    ContextualLLBPredictor(size_t historyLength = 16,
                           size_t tableSize     = 1 << 16);
    ~ContextualLLBPredictor();

    // contextHash: from your call/return shadow stack
    bool predict(uint32_t pc, uint64_t contextHash);
    void update (uint32_t pc, uint64_t contextHash, bool taken);
    int  confidence(uint32_t pc, uint64_t contextHash) const;
    void allocateResources(uint32_t pc);

private:
    struct Entry {
        uint32_t tag;      // low bits of PC
        uint8_t  history;  // local history shift register
        int8_t   counter;  // saturating in [-4..+3]
        uint32_t lru;      // for allocation recency
    };

    size_t       historyLength;
    size_t       tableSize;
    uint32_t     mask;
    uint32_t     globalLRU;
    std::vector<Entry> table;

    uint32_t getIndex(uint32_t pc, uint64_t contextHash) const {
        return (static_cast<uint32_t>(contextHash) ^ pc) & mask;
    }

    void allocateEntry(uint32_t idx, uint32_t tag, bool taken) {
        auto &e = table[idx];
        e.tag     = tag;
        e.counter = taken ? 0 : -1;
        e.history = static_cast<uint8_t>(taken);
        e.lru     = globalLRU;
    }
};

#endif // CONTEXTUAL_LLBP_PREDICTOR_H
