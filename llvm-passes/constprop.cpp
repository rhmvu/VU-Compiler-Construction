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
    // for (BasicBlock &BB : F) {
    //     for (Instruction &II : BB) {
    //         Instruction *I = &II;
    //         for(Use &U : I->operands()){
    //             Value *v = U.get();
    //         }
    //     }
    // }

    return false;  // We did not alter the IR
}

char ConstPropPass::ID = 0;
static RegisterPass<ConstPropPass> X("coco-constprop", "Example LLVM pass printing each function it visits, and every call instruction it finds");