#ifndef BRANCH_PREDICTOR_H
#define BRANCH_PREDICTOR_H

#include <cstdint>
#include <string>

const int PRED_BUF_SIZE = 4096;

class BimodalPredictor;
class TAGEPredictor;
class PerceptronPredictor;
class ContextualLLBPredictor;
class DynamicAllocator;

class BranchPredictor {
public:
  enum Strategy {
    AT, // Always Taken
    NT, // Always Not Taken
    BTFNT, // Backward Taken, Forward Not Taken
    BPB, // Branch Prediction Buffer with 2bit history information
    HCNP,
  } strategy;

  BranchPredictor();
  ~BranchPredictor();

  bool predict(uint32_t pc, uint32_t insttype, int64_t op1, int64_t op2,
               int64_t offset);

  // For Branch Prediction Buffer 
  void update(uint32_t pc, bool branch);

  std::string strategyName();
  
private:
  enum PredictorState {
    STRONG_TAKEN = 0, WEAK_TAKEN = 1,
    STRONG_NOT_TAKEN = 3, WEAK_NOT_TAKEN = 2,
  } predbuf[PRED_BUF_SIZE]; // initial state: WEAK_TAKEN

  BimodalPredictor*         l0;
  TAGEPredictor*            l1;
  PerceptronPredictor*      l2;
  ContextualLLBPredictor*   l3;
  DynamicAllocator*         allocator;

  bool lastPredictedTaken; // for HCNP
};

#endif