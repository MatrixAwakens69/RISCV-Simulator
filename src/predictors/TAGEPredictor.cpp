#include "TAGEPredictor.h"
#include <cstdlib>
#include <cmath>

TAGEPredictor::TAGEPredictor(const std::vector<int>& history_lengths, int tag_bits, uint32_t hist_mask)
    : history_lengths(history_lengths), tag_bits(tag_bits), hist_mask(hist_mask), global_history(0) {
    for (int i = 0; i < NUM_TABLES; ++i) {
        tables[i].resize(TABLE_SIZE);
        for (auto& entry : tables[i]) {
            entry.tag = 0;
            entry.counter = 0;
            entry.u = 0;
        }
    }
}

TAGEPredictor::~TAGEPredictor() {}

int TAGEPredictor::computeIndex(uint32_t pc, int table) {
    return (pc ^ (global_history >> (table * 4))) % TABLE_SIZE;
}

uint32_t TAGEPredictor::computeTag(uint32_t pc, int table) {
    return (pc ^ (global_history >> (table * 4))) & ((1 << tag_bits) - 1);
}

bool TAGEPredictor::predict(uint32_t pc) {
    for (int i = NUM_TABLES - 1; i >= 0; --i) {
        int index = computeIndex(pc, i);
        uint32_t tag = computeTag(pc, i);
        if (tables[i][index].tag == tag) {
            return tables[i][index].counter >= 0;
        }
    }
    return true;
}

void TAGEPredictor::update(uint32_t pc, bool taken) {
    global_history = (global_history << 1) | (taken ? 1 : 0);
    for (int i = NUM_TABLES - 1; i >= 0; --i) {
        int index = computeIndex(pc, i);
        uint32_t tag = computeTag(pc, i);
        if (tables[i][index].tag == tag) {
            if (taken) {
                if (tables[i][index].counter < MAX_COUNTER)
                    tables[i][index].counter++;
            } else {
                if (tables[i][index].counter > MIN_COUNTER)
                    tables[i][index].counter--;
            }
            return;
        }
    }
    int alloc_index = rand() % NUM_TABLES;
    int index = computeIndex(pc, alloc_index);
    tables[alloc_index][index].tag = computeTag(pc, alloc_index);
    tables[alloc_index][index].counter = taken ? 0 : -1;
    tables[alloc_index][index].u = 0;
}

int TAGEPredictor::confidence(uint32_t pc) {
    for (int i = NUM_TABLES - 1; i >= 0; --i) {
        int index = computeIndex(pc, i);
        uint32_t tag = computeTag(pc, i);
        if (tables[i][index].tag == tag) {
            return std::abs(tables[i][index].counter);
        }
    }
    return 0;
}
