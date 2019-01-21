#define DEBUG_TYPE "LICMPass"
#include "utils.h"

namespace {
    class LICMPass : public LoopPass {
    public:
        static char ID;
        LICMPass() : LoopPass(ID) {}
        virtual bool runOnLoop(Loop *L, LPPassManager &LPM) override;
        void getAnalysisUsage(AnalysisUsage &AU) const override;
    };
}

void LICMPass::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesCFG();
    AU.addRequired<LoopInfoWrapperPass>();
    getLoopAnalysisUsage(AU);
}

bool isLoopInvariant(Instruction *I){
    return I -> isBinaryOp() || I -> isShift() || isa<SelectInst>(I) || I -> isCast() || isa<GetElementPtrInst>(I);
}

bool dominatesExitBlocks(Instruction *I, Loop *L){
    bool result = false;

    if (isLoopInvariant(I)) {
        for (Use &U : I->operands()) {
            if (isa<Constant>(U) || L->hasLoopInvariantOperands(I)) {
                result = true;
            } else {
                return false;
            }
        }
    }

    return result;
}

bool LICMPass::runOnLoop(Loop *L, LPPassManager &LPM) {
    BasicBlock *Header = L->getHeader();
    DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();

    for (BasicBlock *BB : L->blocks()) {
        if (DT->dominates(Header, BB)) {
            BasicBlock::iterator it = BB->begin();
            while (it != BB->end()){
                Instruction *I = dyn_cast<Instruction>(it);
                it++;

                if (dominatesExitBlocks(I, L) && !(I -> mayHaveSideEffects())) {
                    BasicBlock *pre = L->getLoopPreheader();
                    Instruction *term = pre->getTerminator();
                    I->moveBefore(term);
                    I->dropUnknownNonDebugMetadata();
                }
            }
        }
    }

    return false;
}

char LICMPass::ID = 0;
RegisterPass<LICMPass> X("coco-licm", "CoCo Dominator Tree Example");