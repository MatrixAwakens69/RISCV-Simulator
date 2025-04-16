#ifndef BIMODAL_PREDICTOR_H
#define BIMODAL_PREDICTOR_H

#include <cstdint>
#include <vector>

class BimodalPredictor {
public:
    BimodalPredictor(uint32_t size);
    ~BimodalPredictor();

    bool predict(uint32_t pc);
    void update(uint32_t pc, bool taken);
    int confidence(uint32_t pc) const;

    bool chooseTage(uint32_t pc) const;
    void updateChooser(uint32_t pc,
                       bool tagePred,
                       bool percPred,
                       bool actual);

private:
    std::vector<uint8_t> table;   // 2‑bit outcome counters
    std::vector<uint8_t> chooser; // 2‑bit chooser: >=2→TAGE, <2→Perceptron
    uint32_t mask;
};

#endif // BIMODAL_PREDICTOR_H
