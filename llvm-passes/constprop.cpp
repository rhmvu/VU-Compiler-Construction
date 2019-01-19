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

    SmallVector<ConstantFP*, 2> floatList;
    SmallVector<ConstantInt*, 2> intList;
    std::map<std::string, Use*> variableMap;
    //Map for variable values we calculated/are constants

    for (BasicBlock &BB : F) {
        for (Instruction &II : BB) {
            Instruction *I = &II;
            if (BinaryOperator *BO = dyn_cast<BinaryOperator>(I)) {
                LOG_LINE("Binary op: " << *BO);
                LOG_LINE("Parent Name: " << I->getName());
                for(Use &U : I->operands()){

                    //check if ConstantInt
                    if (ConstantInt *CI = dyn_cast<ConstantInt>(U)){
                        LOG_LINE("     INT CONST: " << *CI);
                        intList.push_back(CI);
                    }else if (ConstantFP *CF = dyn_cast<ConstantFP>(U)){
                        LOG_LINE("     FP CONST: " << *CF);
                        floatList.push_back(CF);
                    /*} else if (CallInst *V = dyn_cast<CallInst>(U)){
                        LOG_LINE("     VAR: " << *V);
                        //if variable check if it is in the map and it already has a value, if so put this value in int or float list. so we can calculate them later
*/
                    } else{
                      //LOG_LINE("     NOT recognised: " << *U);
                      if (Instruction *V = dyn_cast<Instruction>(U)){
                          LOG_LINE("     Parent var: " << V->getName());
                        }

                    }
                }
                //if 2 operands and we can do calculation, then save the outcome in the variable map
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
