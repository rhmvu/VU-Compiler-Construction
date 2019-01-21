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

APInt processOperation(SmallVector<ConstantInt*, 2> intList, int op){
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
    bool modifiedCode = false;

    SmallVector<ConstantInt*, 2> intList;
    u_int opcode;
    llvm::LLVMContext &context = F.getContext();

    for (BasicBlock &BB : F) {
        for (Instruction &II : BB) {
            Instruction *I = &II;
            if (dyn_cast<BinaryOperator>(I) != nullptr) {
                opcode = I->getOpcode();
                std::string parent = "";
                for(Use &U : I->operands()){
                    if (ConstantInt *CI = dyn_cast<ConstantInt>(U)){
                        intList.push_back(CI);
                    } else if (dyn_cast<Instruction>(U) != nullptr){

                    }
                }

                if(intList.size() == 2){
                    APInt result = processOperation(intList, opcode);
                    I->replaceAllUsesWith(ConstantInt::get(context, result));
                }
                intList.clear();
            }
        }
    }
    return modifiedCode;
}

char ConstPropPass::ID = 0;
static RegisterPass<ConstPropPass> X("coco-constprop", "LLVM pass propagating constants");
