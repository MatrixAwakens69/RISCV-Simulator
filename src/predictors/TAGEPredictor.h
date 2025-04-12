#ifndef TAGE_PREDICTOR_H
#define TAGE_PREDICTOR_H

#include <vector>
#include <cstdint>

class TAGEPredictor {
public:
    TAGEPredictor(const std::vector<int>& history_lengths, int tag_bits, uint32_t hist_mask);
    ~TAGEPredictor();

    bool predict(uint32_t pc);
    void update(uint32_t pc, bool taken);
    int confidence(uint32_t pc);

private:
    struct Entry {
        uint32_t tag;
        int counter;
        int u;
    };

    std::vector<int> history_lengths;
    int tag_bits;
    uint32_t hist_mask;
    uint64_t global_history;
    static const int NUM_TABLES = 5;
    static const int TABLE_SIZE = 1024;
    static const int MAX_COUNTER = 3;
    static const int MIN_COUNTER = -4;
    std::vector<Entry> tables[NUM_TABLES];

    int computeIndex(uint32_t pc, int table);
    uint32_t computeTag(uint32_t pc, int table);
};

#endif // TAGE_PREDICTOR_H
