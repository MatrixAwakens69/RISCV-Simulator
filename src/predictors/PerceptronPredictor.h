#ifndef PERCEPTRON_PREDICTOR_H
#define PERCEPTRON_PREDICTOR_H

#include <cstdint>
#include <vector>

class PerceptronPredictor {
public:
    PerceptronPredictor(int historyLength = 32, int numPerceptrons = 1024, int threshold = 128);
    ~PerceptronPredictor();

    bool predict(uint32_t pc);
    void update(uint32_t pc, bool taken);
    int confidence(uint32_t pc) const;
    void allocateResources(uint32_t pc);

private:
    int historyLength;
    int numPerceptrons;
    int threshold;
    std::vector<std::vector<int>> weights;
    std::vector<int> bias;
    std::vector<bool> globalHistory;

    int index(uint32_t pc) const;
    int dotProduct(int idx) const;
    void train(int idx, bool taken);
};

#endif // PERCEPTRON_PREDICTOR_H
