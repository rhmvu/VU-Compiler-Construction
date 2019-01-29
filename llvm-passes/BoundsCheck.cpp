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
    bool instrumentAllocations(Function &F);
    bool runOnFunction(Function &F);
    Value* accumulatedOffset(GetElementPtrInst *gep, IRBuilder<> builder);
    //Value *getBasePointer(Instruction *GEP);
    Value *getArraySize(GetElementPtrInst *gep);
};
} // namespace

/*
 * Finds all allocations in a function and inserts a call that prints its size.
 */
bool BoundsCheckerPass::instrumentAllocations(Function &F)
{
    bool Changed = false;

    // We want to skip instrumenting certain functions, like declarations
    // and helper functions (e.g., our dummy_print_allocation)
    if (!shouldInstrument(&F))
        return Changed;

    // Construct an IRBuilder (at a random insertion point) so we can reuse it
    // every time we need it.
    IRBuilder<> builder(&F.getEntryBlock());

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
            Value *arraySize = getArraySize(G);
            LOG_LINE("arraysize = " << *arraySize);
        
            // CallInst *call = 
            builder.CreateCall(BoundsCheckHelper, {offset, arraySize});
            //builder.Insert(call);
            Changed = true;
        }
    }
    return Changed;
}

Value* BoundsCheckerPass::accumulatedOffset(GetElementPtrInst *gep, IRBuilder<> builder)
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

Value *getBasePointer(GetElementPtrInst *gep)
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

Value *BoundsCheckerPass::getArraySize(GetElementPtrInst *gep)
{
    Value *basePointer = getBasePointer(gep);
    if (AllocaInst *alloca = dyn_cast<AllocaInst>(basePointer))
    {
        return alloca->getArraySize();
    }
    else
    {   LOG_LINE("CONVERTING TO ARRAYTYPE");
        Type *ty = basePointer->getType();
        ArrayType *AT = dyn_cast<ArrayType>(ty->getPointerElementType());
        return ConstantInt::get(Type::getInt32Ty(ty->getContext()), AT->getNumElements(), true);
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
        Changed |= instrumentAllocations(F);
    }

    return Changed;
}

char BoundsCheckerPass::ID = 0;
static RegisterPass<BoundsCheckerPass> X("coco-boundscheck", "Example LLVM module pass that inserts prints for every allocation.");