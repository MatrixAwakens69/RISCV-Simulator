#ifndef BIMODAL_PREDICTOR_H
#define BIMODAL_PREDICTOR_H

#include <cstdint>
#include <vector>

class BimodalPredictor {
public:
    BimodalPredictor(uint32_t size = 4096);
    ~BimodalPredictor();

    bool predict(uint32_t pc);
    void update(uint32_t pc, bool taken);
    int confidence(uint32_t pc) const;

private:
    std::vector<uint8_t> table;
    uint32_t mask;
};

#endif // BIMODAL_PREDICTOR_H
