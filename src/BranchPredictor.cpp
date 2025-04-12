#include "BranchPredictor.h"
#include "predictors/BimodalPredictor.h"
#include "predictors/TAGEPredictor.h"
#include "predictors/PerceptronPredictor.h"
#include "predictors/ContextualLLBPredictor.h"
#include "predictors/DynamicAllocator.h"
#include "Debug.h"

BranchPredictor::BranchPredictor() {
    // Initialize the prediction buffer to WEAK_TAKEN
    for (int i = 0; i < PRED_BUF_SIZE; ++i) {
        this->predbuf[i] = WEAK_TAKEN;
    }

    // Initialize components based on the selected strategy
    if (strategy == HCNP) {
        l0 = new BimodalPredictor(4096); // 4K entries
        l1 = new TAGEPredictor({4, 8, 16, 32, 64}, 8, 0xFFFF); // tagbits=8, histMask=0xFFFF
        l2 = new PerceptronPredictor(1024, 128); // numPerceptrons=1024, histLen=128
        l3 = new ContextualLLBPredictor(512 * 1024); // 512KB backing store
        allocator = new DynamicAllocator(0.01); // track top 1% mis-predicted branches
    } else {
        // For other strategies, set pointers to nullptr
        l0 = nullptr;
        l1 = nullptr;
        l2 = nullptr;
        l3 = nullptr;
        allocator = nullptr;
    }
}

BranchPredictor::~BranchPredictor() {
  if (strategy == HCNP) {
    delete l0;
    delete l1;
    delete l2;
    delete l3;
    delete allocator;
  }
}

bool BranchPredictor::predict(uint32_t pc, uint32_t insttype, int64_t op1,
                              int64_t op2, int64_t offset) {
  switch (this->strategy) {
    case NT:
      return false;
    case AT:
      return true;
    case BTFNT:
      if (offset >= 0) {
        return false;
      } else {
        return true;
      }
    case BPB: {
      PredictorState state = this->predbuf[pc % PRED_BUF_SIZE];
      if (state == STRONG_TAKEN || state == WEAK_TAKEN) {
        return true;
      } else if (state == STRONG_NOT_TAKEN || state == WEAK_NOT_TAKEN) {
        return false;
      } else {
        dbgprintf("Strange Prediction Buffer!\n");
      }
    }
    case HCNP: {
      bool p0 = l0->predict(pc);
      if (l0->confidence(pc) >= 2)
        return p0;

      bool p1 = l1->predict(pc);
      if (l1->confidence(pc) >= 1)
        return p1;

      bool p2 = l2->predict(pc);
      if (l2->confidence(pc) >= 1)
        return p2;

      bool p3 = l3->predict(pc);
      return p3;
    }
    default:
      dbgprintf("Unknown Branch Prediction Strategy!\n");
      break;
  }
  return false;
}

void BranchPredictor::update(uint32_t pc, bool taken) {
  switch (this->strategy) {
      case NT:
      case AT:
      case BTFNT:
          // static predictors: no state to update
          break;

      case BPB: {
          // 2‑bit Branch Prediction Buffer update (unchanged)
          int id = pc % PRED_BUF_SIZE;
          PredictorState &state = this->predbuf[id];
          if (taken) {
              if (state == STRONG_NOT_TAKEN)   state = WEAK_NOT_TAKEN;
              else if (state == WEAK_NOT_TAKEN) state = WEAK_TAKEN;
              else if (state == WEAK_TAKEN)     state = STRONG_TAKEN;
              // else STRONG_TAKEN stays
          } else {
              if (state == STRONG_TAKEN)     state = WEAK_TAKEN;
              else if (state == WEAK_TAKEN)   state = WEAK_NOT_TAKEN;
              else if (state == WEAK_NOT_TAKEN) state = STRONG_NOT_TAKEN;
              // else STRONG_NOT_TAKEN stays
          }
          break;
      }

      case HCNP: {
          // --- 1) Record misprediction for dynamic allocator ---
          bool predicted = this->lastPredictedTaken;  // must be set in predict()
          allocator->record(pc, predicted != taken);

          // --- 2) Train/update all levels in parallel (or selectively) ---
          l0->update(pc, taken);
          l1->update(pc, taken);
          l2->update(pc, taken);
          l3->update(pc, taken);

          // --- 3) Occasionally rebalance resources to the hardest branches ---
          if (allocator->shouldReallocate()) {
              allocator->redistribute(l2, l3);
          }
          break;
      }

      default:
          dbgprintf("Unknown Branch Prediction Strategy in update()!\n");
          break;
  }
}

std::string BranchPredictor::strategyName() {
  switch (this->strategy) {
  case NT:
    return "Always Not Taken";
  case AT:
    return "Always Taken";
  case BTFNT:
    return "Back Taken Forward Not Taken";
  case BPB:
    return "Branch Prediction Buffer";
  case HCNP:
    return "Hierarchical Contextual Neural Predictor";
  default:
    dbgprintf("Unknown Branch Perdiction Strategy!\n");
    break;
  }
  return "error";
}