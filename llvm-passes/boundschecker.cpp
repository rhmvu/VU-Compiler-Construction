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


Value *accumulatedOffsets(Value *Val, IRBuilder<> builder)
{
    GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(Val);
    Value * operand2;
    if (GEP->getOperand(1) != nullptr) {
         operand2 = dyn_cast<Value>(GEP->getOperand(1));    
    }else{
        return nullptr;
    }

    
    Value *result = accumulatedOffsets(operand2, builder);
    if (result == nullptr)
    {
        return (operand2);
    }
    return builder.CreateAdd(operand2, accumulatedOffsets(operand2, builder));
}

Value *getBasePointer(Instruction *GEP, IRBuilder<> builder)
{
    
    GetElementPtrInst *operand1 = dyn_cast<GetElementPtrInst>(GEP->getOperand(0));
    if (operand1 == nullptr)
    {
        return GEP->getOperand(0);
    }
    else
    {
        return getBasePointer(operand1, builder);
    }
}



    bool BoundsCheckerPass::runOnModule(Module & M)
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
            IRBuilder<> builder(&F.getEntryBlock()); //probably not get entry block
            // We want to skip instrumenting certain functions, like declarations
            // and helper functions (e.g., our dummy_print_allocation)
            if (!shouldInstrument(&F))
                continue;

            LOG_LINE("Visiting function " << F.getName());
        
                for (Instruction &II :instructions(F))
                {
                    Instruction *I = &II;
                    //if it is a GEP and it has an index in addition to the base pointer, we process
                    if (isa<GetElementPtrInst>(I) &&
                        dyn_cast<GetElementPtrInst>(I)->getOperand(1) != nullptr)
                    {
                        //CallInst *call =
                        builder.CreateCall(PrintAllocFunc, {accumulatedOffsets(I,builder), getBasePointer(I,builder)});
                        //builder.Insert(call);
                    }
                }
            
            Changed |= instrumentAllocations(F);
        }

        return Changed;
    }

    char BoundsCheckerPass::ID = 0;
    static RegisterPass<BoundsCheckerPass> X("coco-boundscheck", "Example LLVM module pass that inserts prints for every allocation.");