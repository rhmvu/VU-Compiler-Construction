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
    // Tell LLVM we need some analysis info which we use for analyzing the
    // DominatorTree.
    AU.setPreservesCFG();
    AU.addRequired<LoopInfoWrapperPass>();
    getLoopAnalysisUsage(AU);
}

bool LICMPass::runOnLoop(Loop *L, LPPassManager &LPM) {
    BasicBlock *Header = L->getHeader();
    DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();

    for (BasicBlock *BB : L->blocks()) {
        if (DT->dominates(Header, BB)) {
            LOG_LINE("The loop header dominates this Basic Block");
        } else {
            LOG_LINE("There is something terribly wrong with " << *BB);
        }
    }

    return false;
}

char LICMPass::ID = 0;
RegisterPass<LICMPass> X("coco-licm", "CoCo Dominator Tree Example");