/*
 * Dummy (and minimal) function pass to serve as a starting point for your
 * Assignment 2 passes. It simply visits every function and prints every call
 * instruction it finds.
 */

#define DEBUG_TYPE "ConstPropPass"
#include "utils.h"

namespace {
    class ConstPropPass : public FunctionPass {
    public:
        static char ID;
        ConstPropPass() : FunctionPass(ID) {}
        virtual bool runOnFunction(Function &F) override;
    };
}

bool ConstPropPass::runOnFunction(Function &F) {
    bool modifiedCode = false;

    for (BasicBlock &BB : F) {
        for (Instruction &II : BB) {
            Instruction *I = &II;
            if (BinaryOperator *BO = dyn_cast<BinaryOperator>(I)) {
                LOG_LINE(" Binary op: " << *BO);
            }
        }
    }

    return modifiedCode;
}

// Register the pass with LLVM so we can invoke it with opt. The first argument
// to RegisterPass is the commandline switch to run this pass (e.g., opt
// -coco-dummypass, the second argument is a description shown in the help text
// about this pass.
char ConstPropPass::ID = 0;
static RegisterPass<ConstPropPass> X("coco-constprop", "LLVM pass propagating constants");
