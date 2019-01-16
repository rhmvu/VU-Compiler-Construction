/*
 * Dummy (and minimal) function pass to serve as a starting point for your
 * Assignment 2 passes. It simply visits every function and prints every call
 * instruction it finds.
 */

#define DEBUG_TYPE "ADCEPass"
#include "utils.h"

namespace {
    class ADCEPass : public FunctionPass {
    public:
        df_iterator_default_set<BasicBlock*> Reachable;
        std::set<Instruction*> live_instructions;
        static char ID;
        ADCEPass() : FunctionPass(ID) {}
        virtual bool runOnFunction(Function &F) override;
    };
}

bool ADCEPass::runOnFunction(Function &F) {
    LOG_LINE("Visiting function " << F.getName());
    for (BasicBlock *BB : depth_first_ext(&F, Reachable) ) {
        for (Instruction &II : *BB) {
            Instruction *I = &II;
            if(!isInstructionTriviallyDead(I)){
                LOG_LINE(" LIVE: " << *I);
                live_instructions.insert(I);
            } else if(I->use_empty()){
                //remove I from BB
                I->dropAllReferences();
            }
        }
    }
    

    return false;  // We did not alter the IR
}

// Register the pass with LLVM so we can invoke it with opt. The first argument
// to RegisterPass is the commandline switch to run this pass (e.g., opt
// -coco-dummypass, the second argument is a description shown in the help text
// about this pass.
char ADCEPass::ID = 0;
static RegisterPass<ADCEPass> X("coco-adce", "Example LLVM pass printing each function it visits, and every call instruction it finds");
