/*
 * Initial pass to 
 */

#define DEBUG_TYPE "ADCEPass"
#include "utils.h"

namespace {
    class ADCEPass : public FunctionPass {
    public:
        df_iterator_default_set<BasicBlock*> Reachable;
        std::set<Instruction*> worklist;
        std::set<Instruction*> deadlist;
        std::set<Instruction*> eraselist;
        static char ID;
        ADCEPass() : FunctionPass(ID) {}
        virtual bool runOnFunction(Function &F) override;
    };
}

bool ADCEPass::runOnFunction(Function &F) {
    for (BasicBlock *BB : depth_first_ext(&F, Reachable) ) {
        for (Instruction &II : *BB) {
            Instruction *I = &II;
            //BasicBlock *bb = I->getParent();
            //LOG_LINE(" Basic block: " << *bb);
            if(!isInstructionTriviallyDead(I)){
                worklist.insert(I);
            } else if(I->use_empty()){
                deadlist.insert(I);
            }
        }
    }

    //need to add the new ops in worklist too
    for(Instruction *I : worklist){
        BasicBlock *BB = I->getParent();
        if(Reachable.find(BB) != Reachable.end()){
            for(Use &U : I->operands()){
                LOG_LINE(" Operand: " << *U.get());
                Instruction *i = dyn_cast<Instruction>(U.get());
                if(i != nullptr && deadlist.find(i) != deadlist.end()){
                    deadlist.erase(dyn_cast<Instruction>(i));
                }
            }
        }
    }


    
    for (BasicBlock *BB : Reachable) {
        for (Instruction &II : *BB) {
            Instruction *I = &II;
            if(deadlist.find(I) != deadlist.end()){
                eraselist.insert(I);
                I->dropAllReferences();
            }
        }
    }

    for(Instruction* I : eraselist){
        I->eraseFromParent();
    }

    return true;
}

// Register the pass with LLVM so we can invoke it with opt. The first argument
// to RegisterPass is the commandline switch to run this pass (e.g., opt
// -coco-dummypass, the second argument is a description shown in the help text
// about this pass.
char ADCEPass::ID = 0;
static RegisterPass<ADCEPass> X("coco-adce", "Example LLVM pass printing each function it visits, and every call instruction it finds");
