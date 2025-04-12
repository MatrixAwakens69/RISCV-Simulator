#include "DynamicAllocator.h"
#include "PerceptronPredictor.h"
#include "ContextualLLBPredictor.h"
#include <algorithm>

DynamicAllocator::DynamicAllocator(double mispredThreshold)
    : totalBranches(0),
      thresholdFraction(mispredThreshold),
      reallocationThreshold(1000) {}

DynamicAllocator::~DynamicAllocator() {}

void DynamicAllocator::record(uint32_t pc, bool mispredicted) {
    if (mispredicted) {
        mispredictions[pc]++;
    }
    totalBranches++;
}

bool DynamicAllocator::shouldReallocate() {
    return totalBranches >= reallocationThreshold;
}

void DynamicAllocator::redistribute(PerceptronPredictor* l2, ContextualLLBPredictor* l3) {
    std::vector<std::pair<uint32_t, int>> sortedMispreds(mispredictions.begin(), mispredictions.end());
    std::sort(sortedMispreds.begin(), sortedMispreds.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    int topN = static_cast<int>(sortedMispreds.size() * thresholdFraction);
    for (int i = 0; i < topN && i < static_cast<int>(sortedMispreds.size()); ++i) {
        uint32_t pc = sortedMispreds[i].first;
        l2->allocateResources(pc);
        l3->allocateResources(pc);
    }

    mispredictions.clear();
    totalBranches = 0;
}
