#define DEBUG_TYPE "BoundsCheckerPass"
#include "utils.h"

namespace
{
class BoundsCheckerPass : public ModulePass
{
  public:
    static char ID;
    BoundsCheckerPass() : ModulePass(ID) {}
    virtual bool runOnModule(Module &M) override;

  private:
    Function *BoundsCheckHelper;
    //bool instrumentAllocations(Function &F);
    bool instrumentGetElementPointers(Function &F, Module &M);
    bool runOnFunction(Function &F);
    Value *accumulatedOffset(GetElementPtrInst *gep, IRBuilder<> builder);
    Value *getBasePointer(GetElementPtrInst *gep);
    Value *getArraySize(Value *basePointer, Module &M, IRBuilder<> builder);
    Value *PrepareCloneFunc(Argument *A, Module &M);
    Value *cloneFunc(Argument *A, Module &M, IRBuilder<> builder);
};
} // namespace

bool BoundsCheckerPass::instrumentGetElementPointers(Function &F, Module &M)
{
    bool Changed = false;

    // We want to skip instrumenting certain functions, like declarations
    // and helper functions (e.g., our dummy_print_allocation)
    if (!shouldInstrument(&F))
        return Changed;

    // Construct an IRBuilder (at a random insertion point) so we can reuse it
    // every time we need it.
    IRBuilder<> builder(&F.getEntryBlock());
    //funcClone_t func;
    //func.funcPtr = &F;

    // Iterate over all instructions in this function.
    for (Instruction &II : instructions(F))
    {
        Instruction *I = &II;

        if (isa<GetElementPtrInst>(I) &&
            dyn_cast<GetElementPtrInst>(I)->getOperand(1) != nullptr)
        {
            GetElementPtrInst *G = dyn_cast<GetElementPtrInst>(I);
            LOG_LINE(*G);
            builder.SetInsertPoint(I);
            Value *offset = accumulatedOffset(G, builder);
            LOG_LINE("offsets = " << *offset);
            Value *basePointer = getBasePointer(G);
            Value *arraySize;
            LOG_LINE("arraysize = " << *arraySize);

            if (basePointer == arraySize)
            {
                LOG_LINE("ARRAYSIZE ERROR, unhandled case");
            }

            if (argumentAndSizeExist(G))
            {
                arraySize = getArraySize(basePointer, M, builder);
                break;
            }

            // CallInst *call =
            builder.CreateCall(BoundsCheckHelper, {offset, arraySize});
            //builder.Insert(call);
            Changed = true;
        }
    }
    return Changed;
}

bool argumentAndSizeExist(GetElementPtrInst *G)
{
    if (Value *bp = dyn_cast<Argument>(G->getOperand(0)))
    {
        Function *newFunc = dyn_cast<Function>(G->getParent());

        LOG_LINE("operand name: " << dyn_cast<Argument>(G->getOperand(0))->getName());
        for (Function::arg_iterator AIT = newFunc->arg_begin(); AIT != newFunc->arg_end(); AIT++)
        {
            if (StringRef(AIT->getName()) == StringRef(bp->getName() + "cas"))
            {
                return true;
            }
        }
        return false;
    }
    else
    {
        return false;cloneFu
    }
}
/*
Value *BoundsCheckerPass::PrepareCloneFunc(Argument *A, Module &M){
    //must be argumnet
    
    Type* Int32Ty = Type::getInt32Ty(M.getContext());
    Value *newArg;
    SmallVector<Value*,2> oldArgs;
    SmallVector<Type*,2> argTypes;
    Function *parentFunction = A->getParent();
    for(User *U : parentFunction->users()){
        if(CallInst *Ins = dyn_cast<CallInst>(U)){
            int counter = 1;
            LOG_LINE("USER instruction" << *Ins);
            newArg = getArraySize(Ins, M);
            for(Value *O :Ins->operands()){
                oldArgs.push_back(O);
                argTypes.push_back(Int32Ty);
                counter++;
            }
            ArrayRef<Type*> newTypes = llvm::ArrayRef<Type*>(Int32Ty);
            SmallVector<Argument*,2> newArgs;
            Function *newFunc = addParamsToFunction(parentFunction,argTypes,newArgs);
            Argument *nca = newArgs.pop_back_val();
            nca->setName(A->getName());
            newFunc->eraseFromParent();
            return nca;
            
        }
    }

}*/

Value *BoundsCheckerPass::cloneFunc(Argument *A, Module &M, IRBuilder<> builder)
{
    //must be argumnet

    Type *Int32Ty = Type::getInt32Ty(M.getContext());
    Type *Int32PtrTy = Type::getInt32PtrTy(M.getContext());
    Value *newArg;
    SmallVector<Value *, 2> oldArgs;
    SmallVector<Type *, 2> argTypes;
    Function *parentFunction = A->getParent();
    for (User *U : parentFunction->users())
    {
        if (CallInst *Ins = dyn_cast<CallInst>(U))
        {
            int counter = 1;
            LOG_LINE("USER instruction" << *Ins);
            newArg = getArraySize(Ins, M, builder);
            for (Value *O : Ins->operands())
            {
                oldArgs.push_back(O);
                argTypes.push_back(Int32Ty);
                counter++;
            }
            ArrayRef<Type *> newParamTypes = llvm::ArrayRef<Type *>(Int32Ty);
            SmallVector<Argument *, 8> newArgs;
            Function *newFunc = addParamsToFunction(parentFunction, newParamTypes, newArgs);
            Argument *nca = newArgs.pop_back_val();
            nca->setName(A->getName() + "cas");
            for (Function::arg_iterator AIT = newFunc->arg_begin(); AIT != newFunc->arg_end(); AIT++)
            {
                if (StringRef(AIT->getName()) == StringRef(""))
                {
                    AIT->setName(nca->getName());
                }
            }
            Function *cloneFunc = cast<Function>(M.getOrInsertFunction(newFunc->getName(), newFunc->getReturnType(), Int32PtrTy, Int32Ty));

            //first should be ptr type
        }
    }
    return nullptr; //later nca
}

Value *BoundsCheckerPass::getArraySize(Value *basePointer, Module &M, IRBuilder<> builder)
{
    if (AllocaInst *alloca = dyn_cast<AllocaInst>(basePointer))
    {
        return alloca->getArraySize();
    }
    else if (Argument *A = dyn_cast<Argument>(basePointer))
    {
        return cloneFunc(A, M, builder);
    }
    else
    {
        LOG_LINE("CONVERTING TO ARRAYTYPE");
        Type *ty = basePointer->getType();
        ArrayType *AT = dyn_cast<ArrayType>(ty->getPointerElementType());
        if (AT == nullptr)
        {
            return basePointer; //Argument contains value at runtime
        }
        return ConstantInt::get(Type::getInt32Ty(ty->getContext()), AT->getNumElements(), true);
    }
}

// class funcClone
// {
//   public:
//     Function* Func;
//     SmallVector<Argument*, 2> newArgs();
// };

Value *BoundsCheckerPass::accumulatedOffset(GetElementPtrInst *gep, IRBuilder<> builder)
{
    // GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(gep);
    GetElementPtrInst *operand1 = dyn_cast<GetElementPtrInst>(gep->getOperand(0));
    Value *operand2 = gep->getOperand(1);

    if (operand1 == nullptr)
    {
        return operand2;
    }
    else
    {
        return builder.CreateAdd(operand2, accumulatedOffset(operand1, builder));
    }
}

Value *BoundsCheckerPass::getBasePointer(GetElementPtrInst *gep)
{
    GetElementPtrInst *operand1 = dyn_cast<GetElementPtrInst>(gep->getOperand(0));
    if (operand1 == nullptr)
    {
        return gep->getOperand(0);
    }
    else
    {
        return getBasePointer(operand1);
    }
}

//first add all pointertypes args in functions to worklist
// for every arg in worklist with pointertype add count
// for every count add arg in function, with the list tpye and argname starting
// then add them to the function and loop through all USERS to add these arguments as wel.
// then when these arguments are added loop to these users each as wel adding them recursively
// start from allocas
//
//

/*void insertArgumentInFunction(Function &F, Value* basePointer){
    Function *newFuncPtr = addParamsToFunction();
}*/

bool BoundsCheckerPass::runOnModule(Module &M)
{
    // Retrieve a pointer to the helper function. The instrumentAllocations
    // function will insert calls to this function for every allocation. This
    // function is written in our runtime (runtime/dummy.c). To see its (LLVM)
    // type, you can check runtime/obj/dummy.ll)
    LLVMContext &C = M.getContext();
    Type *VoidTy = Type::getVoidTy(C);
    Type *Int32Ty = Type::getInt32Ty(C);
    //   void @__coco_dummy_print_allocation(i32 %elems)
    BoundsCheckHelper =
        cast<Function>(M.getOrInsertFunction("__coco_check_bounds",
                                             VoidTy, Int32Ty, Int32Ty));

    // LLVM wants to know whether we made any modifications to the IR, so we
    // keep track of this.
    bool Changed = false;

    for (Function &F : M)
    {
        //Changed |= instrumentAllocations(F);
        Changed |= instrumentGetElementPointers(F, M);
        if (Changed)
        {
            //dosomething
        }
    }

    return Changed;
}

char BoundsCheckerPass::ID = 0;
static RegisterPass<BoundsCheckerPass> X("coco-boundscheck", "Example LLVM module pass that inserts prints for every allocation.");