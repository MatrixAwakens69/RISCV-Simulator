#ifndef TAGE_PREDICTOR_H
#define TAGE_PREDICTOR_H

#include <vector>
#include <cstdint>

class TAGEPredictor {
public:
    TAGEPredictor(int tableSize = 1024,
                  int tagBits   = 12,
                  uint32_t histMask = 0xFFFFFFFFu);
    ~TAGEPredictor();

    bool   predict(uint32_t pc);
    void   update(uint32_t pc, bool taken);
    int    confidence(uint32_t pc);

private:
    struct Entry {
        uint32_t tag;
        int8_t   counter;  // saturating in [MIN_COUNTER..MAX_COUNTER]
        uint8_t u;         // useful bit
    };

    static const int NUM_TABLES    = 8;
    static const int MAX_COUNTER   =  3;
    static const int MIN_COUNTER   = -4;

    int            tableSize;
    int            tagBits;
    uint32_t       histMask;
    uint64_t       globalHistory;
    std::vector<int> historyLengths;                  // size = NUM_TABLES
    std::vector<std::vector<Entry>> tables;           // tables[i].size() = tableSize

    int    computeIndex(uint32_t pc, int t)    const;
    uint32_t computeTag(uint32_t pc, int t)    const;
    int    saturate(int v, int lo, int hi)     const;
    void   allocateEntry(uint32_t pc, bool taken, int provider);
};

#endif // TAGE_PREDICTOR_H
