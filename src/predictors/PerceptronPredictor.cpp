#include "PerceptronPredictor.h"
#include <cmath>

PerceptronPredictor::PerceptronPredictor(int historyLength, int numPerceptrons, int threshold)
    : historyLength(historyLength),
      numPerceptrons(numPerceptrons),
      threshold(threshold),
      weights(numPerceptrons, std::vector<int>(historyLength, 0)),
      bias(numPerceptrons, 0),
      globalHistory(historyLength, false) {}

PerceptronPredictor::~PerceptronPredictor() {}

int PerceptronPredictor::index(uint32_t pc) const {
    return pc % numPerceptrons;
}

int PerceptronPredictor::dotProduct(int idx) const {
    int sum = bias[idx];
    for (int i = 0; i < historyLength; ++i) {
        sum += weights[idx][i] * (globalHistory[i] ? 1 : -1);
    }
    return sum;
}

bool PerceptronPredictor::predict(uint32_t pc) {
    int idx = index(pc);
    int sum = dotProduct(idx);
    return sum >= 0;
}

void PerceptronPredictor::update(uint32_t pc, bool taken) {
    int idx = index(pc);
    int sum = dotProduct(idx);
    bool prediction = sum >= 0;
    if (prediction != taken || std::abs(sum) <= threshold) {
        train(idx, taken);
    }
    globalHistory.pop_back();
    globalHistory.insert(globalHistory.begin(), taken);
}

void PerceptronPredictor::train(int idx, bool taken) {
    int t = taken ? 1 : -1;
    bias[idx] += t;
    for (int i = 0; i < historyLength; ++i) {
        weights[idx][i] += t * (globalHistory[i] ? 1 : -1);
    }
}

int PerceptronPredictor::confidence(uint32_t pc) const {
    int idx = index(pc);
    return std::abs(dotProduct(idx));
}

void PerceptronPredictor::allocateResources(uint32_t pc) {
    // Compute the index for the perceptron table using the PC
    int idx = index(pc);

    // Adjust the weights of the perceptron based on its misprediction history
    // For example, increase the weight for bits that are strongly correlated with the branch outcome
    for (int i = 0; i < historyLength; ++i) {
        if (globalHistory[i] == 1) {
            weights[idx][i] += (weights[idx][i] < threshold) ? 1 : 0;
        } else {
            weights[idx][i] -= (weights[idx][i] > -threshold) ? 1 : 0;
        }
    }

    // Optionally, adjust the bias term to influence the prediction threshold
    bias[idx] += 1;
}
