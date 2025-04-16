#include "TAGEPredictor.h"
#include <cstdlib>
#include <algorithm>

TAGEPredictor::TAGEPredictor(int tableSize_,
                             int tagBits_,
                             uint32_t histMask_)
  : tableSize(tableSize_),
    tagBits(tagBits_),
    histMask(histMask_),
    globalHistory(0),
    historyLengths{2,4,8,16,32,64,128,256},
    tables(NUM_TABLES, std::vector<Entry>(tableSize_))
{
    for (auto &tbl : tables)
        for (auto &e : tbl)
            e = {0, 0, 0};
}

TAGEPredictor::~TAGEPredictor() {}

int TAGEPredictor::computeIndex(uint32_t pc, int t) const {
    uint64_t hist = (globalHistory & histMask) >> (t * 4);
    return (pc ^ hist) & (tableSize - 1);
}

uint32_t TAGEPredictor::computeTag(uint32_t pc, int t) const {
    uint64_t hist = (globalHistory & histMask) >> (t * 4);
    return (pc ^ hist) & ((1u << tagBits) - 1);
}

bool TAGEPredictor::predict(uint32_t pc) {
    int provider = -1, alt = -1;
    for (int t = NUM_TABLES-1; t >= 0; --t) {
        int idx = computeIndex(pc, t);
        if (tables[t][idx].tag == computeTag(pc,t)) {
            provider = t;
            break;
        }
    }
    for (int t = provider-1; t >= 0; --t) {
        int idx = computeIndex(pc, t);
        if (tables[t][idx].tag == computeTag(pc,t)) {
            alt = t;
            break;
        }
    }

    bool predProvider = true;
    if (provider >= 0) {
        auto &e = tables[provider][ computeIndex(pc,provider) ];
        predProvider = e.counter >= 0;
        if (e.u == 0 || std::abs(e.counter) <= 1) {
            if (alt >= 0)
                predProvider = tables[alt][ computeIndex(pc,alt) ].counter >= 0;
        }
    }
    return predProvider;
}

void TAGEPredictor::update(uint32_t pc, bool taken) {
    // 1) Update global history
    globalHistory = (globalHistory << 1) | (taken ? 1 : 0);

    // 2) Find provider and alternate
    int provider = -1, alt = -1;
    for (int t = NUM_TABLES-1; t >= 0; --t) {
        int idx = computeIndex(pc, t);
        if (tables[t][idx].tag == computeTag(pc,t)) {
            provider = t;
            break;
        }
    }
    for (int t = provider-1; t >= 0; --t) {
        int idx = computeIndex(pc, t);
        if (tables[t][idx].tag == computeTag(pc,t)) {
            alt = t;
            break;
        }
    }

    // 3) Determine final prediction (same as in predict)
    bool finalPred = true;
    if (provider >= 0) {
        auto &e = tables[provider][ computeIndex(pc,provider) ];
        finalPred = e.counter >= 0;
        if (e.u == 0 || std::abs(e.counter) <= 1) {
            if (alt >= 0)
                finalPred = tables[alt][ computeIndex(pc,alt) ].counter >= 0;
        }
    }

    // 4) Update provider counter
    if (provider >= 0) {
        auto &e = tables[provider][ computeIndex(pc,provider) ];
        e.counter = saturate(e.counter + (taken ? 1 : -1),
                             MIN_COUNTER, MAX_COUNTER);
        // Update usefulness: if provider was correct and alt wrong, inc u; if opposite, dec u
        bool altPred = (alt>=0)
          ? tables[alt][ computeIndex(pc,alt) ].counter >= 0
          : true;
        if (finalPred == taken && altPred != taken)
            e.u = std::min<uint8_t>(e.u+1, 1);
        else if (finalPred != taken && altPred == taken)
            e.u = 0;
    } else {
        // 5) Allocate a new entry if mispredicted
        if (finalPred != taken)
            allocateEntry(pc, taken, provider);
    }
}

int TAGEPredictor::confidence(uint32_t pc) {
    for (int t = NUM_TABLES-1; t >= 0; --t) {
        int idx = computeIndex(pc, t);
        if (tables[t][idx].tag == computeTag(pc,t))
            return std::abs(tables[t][idx].counter);
    }
    return 0;
}

void TAGEPredictor::allocateEntry(uint32_t pc, bool taken, int provider) {
    // Try to allocate in the first table above provider with u==0
    for (int t = provider+1; t < NUM_TABLES; ++t) {
        int idx = computeIndex(pc, t);
        auto &e = tables[t][idx];
        if (e.u == 0) {
            e.tag     = computeTag(pc, t);
            e.counter = (taken ? 0 : -1);
            e.u       = 0;
            return;
        }
    }
    // If none free, randomly pick one to replace
    int t = provider+1 + (std::rand() % (NUM_TABLES - (provider+1)));
    int idx = computeIndex(pc, t);
    auto &e = tables[t][idx];
    e.tag     = computeTag(pc, t);
    e.counter = (taken ? 0 : -1);
    e.u       = 0;
}

int TAGEPredictor::saturate(int v, int lo, int hi) const {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}
