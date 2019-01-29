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
    Function *PrintAllocFunc;
    bool instrumentAllocations(Function &F);
    bool runOnFunction(Function &F);
    // Value *accumulatedOffsets(GetElementPtrInst *gep, IRBuilder<> builder);
    // Value *getBasePointer(Instruction *GEP, IRBuilder<> builder)
};
} // namespace

/*
 * Finds all allocations in a function and inserts a call that prints its size.
 */
bool BoundsCheckerPass::instrumentAllocations(Function &F)
{
    bool Changed = false;

    // Construct an IRBuilder (at a random insertion point) so we can reuse it
    // every time we need it.
    IRBuilder<> B(&F.getEntryBlock());

    // Iterate over all instructions in this function.
    for (Instruction &II : instructions(F))
    {
        Instruction *I = &II;

        // To see if this instruction is an allocation, we try to cast it to an
        // AllocaInst. This returns a nullptr if this instruction is not an
        // AllocaInst.
        if (AllocaInst *AI = dyn_cast<AllocaInst>(I))
        {
            // Create a new call to our helper function, with the number of
            // allocated elements as argument.
            B.SetInsertPoint(AI);
            B.CreateCall(PrintAllocFunc, {AI->getArraySize()});
            Changed = true;
        }
    }
    return Changed;
}

Value *accumulatedOffsets(GetElementPtrInst *gep, IRBuilder<> builder)
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
        return builder.CreateAdd(operand2, accumulatedOffsets(operand1, builder));
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

Value *getArraySize(GetElementPtrInst *gep)
{
    Value *basePointer = getBasePointer(gep);
    if (AllocaInst *alloca = dyn_cast<AllocaInst>(basePointer))
    {
        return alloca->getArraySize();
    }
    else
    { 
        
        if(Argument *arg = dyn_cast<Argument>(basePointer)){
            LOG_LINE("IT'S An ARG!!>> " << *arg);
        }
        LOG_LINE("NOT AN ALLOCA, FAILING NOW");
        exit(0);
        // Constant *cons = dyn_cast<Constant>(basePointer);
        // Type *type = cons->getType();
        // /*PointerType *ptrType = PointerType::get(type,0);
        // ArrayType *arrType = ptrType->getArrayElementType();
        // uint64_t arraySize = arrType->getArrayNumElements();
        // */
        // LOG_LINE("arraysize is probably not: " << type->getArrayNumElements());
        //return basePointer; //should return actual value here
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
    PrintAllocFunc =
        cast<Function>(M.getOrInsertFunction("__coco_boundscheck_print_allocation",
                                             VoidTy, Int32Ty));

    // LLVM wants to know whether we made any modifications to the IR, so we
    // keep track of this.
    bool Changed = false;

    for (Function &F : M)
    {
        Changed |= runOnFunction(F);
    }

    return Changed;
}

bool BoundsCheckerPass::runOnFunction(Function &F)
{
    bool Changed = false;

    // We want to skip instrumenting certain functions, like declarations
    // and helper functions (e.g., our dummy_print_allocation)
    if (!shouldInstrument(&F))
        return Changed;

    IRBuilder<> builder(&F.getEntryBlock());
    LOG_LINE("Visiting function " << F.getName());

    for (Instruction &II : instructions(F))
    {
        Instruction *I = &II;
        //if it is a GEP and it has an index in addition to the base pointer, we process
        if (isa<GetElementPtrInst>(I) &&
            dyn_cast<GetElementPtrInst>(I)->getOperand(1) != nullptr)
        {
            GetElementPtrInst *G = dyn_cast<GetElementPtrInst>(I);
            LOG_LINE(*G);
            builder.SetInsertPoint(I);
            Value* offset = accumulatedOffsets(G,builder);
            LOG_LINE("offsets = " << *offset);
            Value* arraySize = getArraySize(G);
            LOG_LINE("arraysize = " << *arraySize);
            CallInst *call = builder.CreateCall(PrintAllocFunc, {offset, arraySize});
            builder.Insert(call);
            Changed = true;
        }
    }

    Changed |= instrumentAllocations(F);
    return Changed;
}

char BoundsCheckerPass::ID = 0;
static RegisterPass<BoundsCheckerPass> X("coco-boundscheck", "Example LLVM module pass that inserts prints for every allocation.");