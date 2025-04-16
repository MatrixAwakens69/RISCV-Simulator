#include "BranchPredictor.h"
#include "predictors/BimodalPredictor.h"
#include "predictors/TAGEPredictor.h"
#include "predictors/PerceptronPredictor.h"
#include "predictors/ContextualLLBPredictor.h"
#include "predictors/DynamicAllocator.h"
#include "Debug.h"

BranchPredictor::BranchPredictor() {
    for (int i = 0; i < PRED_BUF_SIZE; ++i) {
        this->predbuf[i] = WEAK_TAKEN;
    }
    
    l0 = new BimodalPredictor(4096); // 4K entries
    l1 = new TAGEPredictor(1024, 12, 0xFFFFFFFFu); // tagbits=8, histMask=0xFFFF
    l2 = new PerceptronPredictor(128, 32); // numPerceptrons=128, histLen=32
    l3 = new ContextualLLBPredictor(512 * 1024); // 512KB backing store
    allocator = new DynamicAllocator(0.01); // track top 1% mis-predicted branches
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
        if (l0->confidence(pc) >= 2) {
            lastPredictedTaken = p0;
            return p0;
        }
    
        bool tageP = l1->predict(pc);
        bool percP = l2->predict(pc);

        lastTagePred = tageP;
        lastPercPred = percP;
    
        bool useTage = l0->chooseTage(pc);
        bool finalP = useTage ? tageP : percP;
    
        lastPredictedTaken = finalP;
        return finalP;
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
          bool predicted = lastPredictedTaken;
          allocator->record(pc, predicted != taken);
      
          l0->updateChooser(pc, lastTagePred, lastPercPred, taken);
      
          l0->update(pc, taken);
          l1->update(pc, taken);
          l2->update(pc, taken);
          l3->update(pc, taken);
      
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