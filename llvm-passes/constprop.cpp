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
    std::map<std::string, Use*>::iterator variableMapIt;
    //Map for variable values we calculated/are constants
    std::string opCodeName;

    for (BasicBlock &BB : F) {
        for (Instruction &II : BB) {
            Instruction *I = &II;
            if (BinaryOperator *BO = dyn_cast<BinaryOperator>(I)) {
                LOG_LINE("Binary op: " << *BO);
                opCodeName = I->getOpcodeName();
                        LOG_LINE("Parent Name: " << I->getName());
                for(Use &U : I->operands()){
                    //check if ConstantInt
                    if (ConstantInt *CI = dyn_cast<ConstantInt>(U)){
                        LOG_LINE("     INT CONST: " << *CI);
                        intList.push_back(CI);
                    }else if (ConstantFP *CF = dyn_cast<ConstantFP>(U)){
                        LOG_LINE("     FP CONST: " << *CF);
                        floatList.push_back(CF);
                    } else{
                        if (Instruction *V = dyn_cast<Instruction>(U)){
                            LOG_LINE("     Parent var: " << V->getName());

                            variableMapIt = variableMap.find(V->getName());
                            if(variableMapIt != variableMap.end()) {

                                Use* varPointer = variableMapIt->second;
                                if (ConstantInt *intVar = dyn_cast<ConstantInt>(varPointer)) {
                                    intList.push_back(intVar);
                                }else if(ConstantFP *floatVar = dyn_cast<ConstantFP>(varPointer)){
                                    floatList.push_back(floatVar);
                                }
                            }
                        }

                    }
                }
                //if 2 operands and we can do calculation, then save the outcome in the variable map
                if (floatList.size() == 2){
                    float result;
                    switch(opCodeName){
                        case "add": result = processfAdd(floatlist); break;
                        case "mul": result = processfMul(floatlist); break;
                        case "sub": result = processfSub(floatlist); break;
                        //case "div": result = processfDiv(floatlist); break;
                        case "ashr": result = processfshift(floatlist); break;
                        default: LOG_LINE("ERRORROROROROROR, opcode unknown: "<< opCodeName); break;
                    }

                    /* get (Type *Ty, uint64_t V, bool isSigned=false) and, if you don't already have an integer type, IntegerType::get (LLVMContext &C, unsigned NumBits).*/
                    ConstantFP* resultFP = new ConstantFP.get(FloatType.get(nullptr,64),result,true);
                    ConstantFP *CF = dyn_cast<Use>(U)){

                    variableMapIt->second = resultFP;


                } else if(intList.size() == 2){
                    LOG_LINE(opCodeName);

                    switch(opCodeName){
                        case "add": result = processAdd(intlist); break;
                        case "mul": result = processMul(intlist); break;
                        case "sub": result = processSub(intlist); break;
                        //case "div": result = processDiv(intlist); break;
                        case "ashr": result = processshift(intlist); break;
                        default: LOG_LINE("ERRORROROROROROR, opcode unknown: "<< opCodeName); break;
                    }




                    for(ConstantInt* ptr : intList){
                        LOG_LINE(*ptr);
                    }
                }
                intList.clear();
                floatList.clear();
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
