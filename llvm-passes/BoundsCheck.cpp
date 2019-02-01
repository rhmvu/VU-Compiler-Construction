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
    bool instrumentGetElementPointers(Function &F, Module &M);
    bool runOnFunction(Function &F);
    Value *accumulatedOffset(GetElementPtrInst *gep, IRBuilder<> builder);
    Value *getBasePointer(GetElementPtrInst *gep);
    Value *getArraySize(Value *basePointer, Module &M, IRBuilder<> builder);
    Value *PrepareCloneFunc(Argument *A, Module &M);
    Value *cloneFunc(Argument *A, Module &M, IRBuilder<> builder);
    Value *argumentAndSizeExist(GetElementPtrInst *G, Function &F, Module &M);
};
} // namespace

bool functionCloned = false;
SmallVector<Function *, 2> deadList;

bool BoundsCheckerPass::instrumentGetElementPointers(Function &F, Module &M)
{
    bool Changed = false;
    if (!shouldInstrument(&F))
        return Changed;

    IRBuilder<> builder(&F.getEntryBlock());

    for (Instruction &II : instructions(F))
    {
        Instruction *I = &II;

        if (isa<GetElementPtrInst>(I) &&
            dyn_cast<GetElementPtrInst>(I)->getOperand(1) != nullptr)
        {
            GetElementPtrInst *G = dyn_cast<GetElementPtrInst>(I);
            builder.SetInsertPoint(I);
            Value *offset = accumulatedOffset(G, builder);
            Value *basePointer = getBasePointer(G);
            Value *arraySize;

            Value *arraysize = argumentAndSizeExist(G, F, M);
            if (arraysize == nullptr)
            {
                arraySize = getArraySize(basePointer, M, builder);
                if (functionCloned)
                {
                    break;
                }
            }
            else
            {
                arraySize = arraysize;
            }

            builder.SetInsertPoint(G);
            builder.CreateCall(BoundsCheckHelper, {offset, arraySize});
            Changed = true;
        }
    }
    return Changed;
}

Value *BoundsCheckerPass::argumentAndSizeExist(GetElementPtrInst *G, Function &F, Module &M)
{
    if (Value *bp = dyn_cast<Argument>(G->getOperand(0)))
    {
        Function *newFunc = &F;
        Type *Int32Type = Type::getInt32Ty(M.getContext());
        Argument *temp = new Argument(Int32Type);
        temp->setName(bp->getName() + "value");
        for (Function::arg_iterator AIT = newFunc->arg_begin(); AIT != newFunc->arg_end(); AIT++)
        {
            if (AIT->getName() == temp->getName())
            {
                functionCloned = false;
                return AIT;
            }
        }
        return nullptr;
    }
    else
    {
        return nullptr;
    }
}

Value *BoundsCheckerPass::cloneFunc(Argument *A, Module &M, IRBuilder<> builder)
{
    Type *Int32Ty = Type::getInt32Ty(M.getContext());
    Type *Int32PtrTy = Type::getInt32PtrTy(M.getContext());
    Value *newArg, *oldArg;
    Argument *arg;
    Function *parentFunction = A->getParent();

    for (User *U : parentFunction->users())
    {
        if (CallInst *Ins = dyn_cast<CallInst>(U))
        {
            newArg = getArraySize(Ins, M, builder);
            oldArg = Ins->getArgOperand(0);
            ArrayRef<Type *> newParamTypes = llvm::ArrayRef<Type *>(Int32Ty);
            SmallVector<Argument *, 8> newArgs;
            Function *newFunc = addParamsToFunction(parentFunction, newParamTypes, newArgs);
            arg = newArgs.pop_back_val();
            arg->setName(A->getName() + "value");
            for (Function::arg_iterator AIT = newFunc->arg_begin(); AIT != newFunc->arg_end(); AIT++)
            {
                if (StringRef(AIT->getName()) == StringRef(""))
                {
                    AIT->setName(arg->getName());
                }
            }
            Function *cloneFunc = cast<Function>(M.getOrInsertFunction(newFunc->getName(), newFunc->getReturnType(), Int32PtrTy, Int32Ty));
            builder.SetInsertPoint(Ins);
            builder.CreateCall(newFunc, {oldArg, newArg});
            Ins->eraseFromParent();
            deadList.push_back(parentFunction);

            instrumentGetElementPointers(*cloneFunc, M);
            functionCloned = true;
        }
    }
    return arg;
}

Value *BoundsCheckerPass::getArraySize(Value *basePointer, Module &M, IRBuilder<> builder)
{
    if (AllocaInst *alloca = dyn_cast<AllocaInst>(basePointer))
    {
        functionCloned = false;
        return alloca->getArraySize();
    }
    else if (Argument *A = dyn_cast<Argument>(basePointer))
    {
        functionCloned = true;
        return cloneFunc(A, M, builder);
    }
    else if (CallInst *call = dyn_cast<CallInst>(basePointer))
    {
        return getArraySize(call->getOperand(0), M, builder);
    }
    else
    {
        functionCloned = false;
        Type *ty = basePointer->getType();
        ArrayType *AT = dyn_cast<ArrayType>(ty->getPointerElementType());
        if (AT == nullptr)
        {
            return basePointer;
        }

        return ConstantInt::get(Type::getInt32Ty(ty->getContext()), AT->getNumElements(), true);
    }
}

Value *BoundsCheckerPass::accumulatedOffset(GetElementPtrInst *gep, IRBuilder<> builder)
{
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
        Changed |= instrumentGetElementPointers(F, M);
    }

    for (Function *item : deadList)
    {
        item->eraseFromParent();
    }

    return Changed;
}

char BoundsCheckerPass::ID = 0;
static RegisterPass<BoundsCheckerPass> X("coco-boundscheck", "Example LLVM module pass that inserts prints for every allocation.");