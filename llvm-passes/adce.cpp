#define DEBUG_TYPE "ADCEPass"
#include "utils.h"

namespace {
    class ADCEPass : public FunctionPass {
    public:
        df_iterator_default_set<BasicBlock*> Reachable;
        SmallVector<Instruction*, 128> worklist;
        std::set<Instruction*> eraselist;
        std::set<Instruction*> livelist;
        static char ID;
        ADCEPass() : FunctionPass(ID) {}
        virtual bool runOnFunction(Function &F) override;
    };
}

bool ADCEPass::runOnFunction(Function &F) {
    bool codeModified = false;
    for (BasicBlock *BB : depth_first_ext(&F, Reachable) ) {
        for (Instruction &II : *BB) {
            Instruction *I = &II;
            if(I->mayHaveSideEffects() || I->isTerminator() || (isa<LoadInst>(I) && dyn_cast<LoadInst>(I)->isVolatile())|| isa<StoreInst>(I) || isa<CallInst>(I)){
                livelist.insert(I);
                worklist.push_back(I);
            } else if(I->use_empty()){
                eraselist.insert(I);
            }
        }
    }

    for (Instruction *I : eraselist) {
        codeModified = true;
        I->dropAllReferences();
    }

    for(Instruction* I : eraselist){
        I->eraseFromParent();
    }

    eraselist.clear();

    while(!worklist.empty()){
        Instruction *I = worklist.pop_back_val();
            for(Use &U : I->operands()){
                if (Instruction *i = dyn_cast<Instruction>(U))
                if(livelist.insert(i).second){
                    worklist.push_back(i);
                }
            }
    }
    
    for (BasicBlock *BB : Reachable) {
        for (Instruction &II : *BB) {
            Instruction *I = &II;
            if(!livelist.count(I)){
                eraselist.insert(I);
            }
        }
    }

    for (Instruction *I : eraselist) {
        codeModified = true;
        I->dropAllReferences();
    }

    for(Instruction* I : eraselist){
        I->eraseFromParent();
    }

    return codeModified;
}

char ADCEPass::ID = 0;
static RegisterPass<ADCEPass> X("coco-adce", "Aggressive Dead-Code Elimination");