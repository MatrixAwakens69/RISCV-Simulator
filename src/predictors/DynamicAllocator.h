#ifndef DYNAMIC_ALLOCATOR_H
#define DYNAMIC_ALLOCATOR_H

#include <unordered_map>
#include <vector>
#include <cstdint>

class PerceptronPredictor;
class ContextualLLBPredictor;

class DynamicAllocator {
public:
    DynamicAllocator(double mispredThreshold = 0.01); // NEW constructor
    ~DynamicAllocator();

    void record(uint32_t pc, bool mispredicted);
    bool shouldReallocate();
    void redistribute(PerceptronPredictor* l2, ContextualLLBPredictor* l3);

private:
    std::unordered_map<uint32_t, int> mispredictions;
    int totalBranches;
    double thresholdFraction; // e.g., 0.01 for top 1%
    int reallocationThreshold;
};

#endif // DYNAMIC_ALLOCATOR_H
