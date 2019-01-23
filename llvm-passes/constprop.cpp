#define DEBUG_TYPE "ConstPropPass"
#include "utils.h"

#define ADD 11
#define SUB 13
#define MUL 15
#define ASHR 25
#define SHL 23

namespace {
    class ConstPropPass : public FunctionPass {
    public:
        static char ID;
        ConstPropPass() : FunctionPass(ID) {}
        virtual bool runOnFunction(Function &F) override;
    };
}

APInt processOperation(SmallVector<ConstantInt*, 1> intList, int op){
    APInt result, 
        op1 = intList.pop_back_val()->getValue(), 
        op2 = intList.pop_back_val()->getValue();

    switch(op){
        case ADD: result = op1 + op2; break;
        case MUL: result = op1 * op2; break;
        case SUB: result = op1 - op2; break;
        case ASHR: result = op2.ashr(op1) ; break;
        case SHL: result = op2 << op1; break;
    }

    return result;
}

bool ConstPropPass::runOnFunction(Function &F) {
    SmallVector<ConstantInt*, 1> intList;
    SmallVector<Instruction*, 1> varList;
    u_int opcode;
    llvm::LLVMContext &context = F.getContext();

    for (BasicBlock &BB : F) {
        for (Instruction &II : BB) {
            Instruction *I = &II;
            bool isFirst = false;
            int c = 0;
            if (dyn_cast<BinaryOperator>(I) != nullptr) {
                opcode = I->getOpcode();
                std::string parent = "";
                for(Use &U : I->operands()){
                    if (ConstantInt *CI = dyn_cast<ConstantInt>(U)){
                        intList.push_back(CI);
                        if (c == 0){
                            isFirst = true;
                        }
                    } else if (Instruction *V = dyn_cast<Instruction>(U)){
                        varList.push_back(V);
                    }
                    c++;
                }

                if(intList.size() == 2){
                    APInt result = processOperation(intList, opcode);
                    I->replaceAllUsesWith(ConstantInt::get(context, result));
                } else if(intList.size() == 1){
                    ConstantInt *constant = intList.pop_back_val();
                    if((I->getOpcode() == ADD && constant->getValue() == 0) || 
                    (I->getOpcode() == MUL && constant->getValue() == 1) ||
                    (I->getOpcode() == SUB && constant->getValue() == 0 && !isFirst)){
                        I->replaceAllUsesWith(varList.pop_back_val());
                    }
                }
                intList.clear();
                varList.clear();
            }
        }
    }
    return false;
}

char ConstPropPass::ID = 0;
static RegisterPass<ConstPropPass> X("coco-constprop", "LLVM pass propagating constants");