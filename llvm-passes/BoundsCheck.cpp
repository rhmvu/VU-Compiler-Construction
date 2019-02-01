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
    Value *argumentAndSizeExist(GetElementPtrInst *G, Function &F, Module &M);
};
} // namespace

bool functionCloned = false;
Function *latestFunction;
SmallVector<Function*, 2> deadList;


bool BoundsCheckerPass::instrumentGetElementPointers(Function &F, Module &M)
{
    bool Changed = false;
    //latestFunction = &F;
    

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
            LOG_LINE("basepointer = " << *basePointer);
            Value *arraySize;

            
            LOG_LINE("before argument check");
            Value* arraysize =  argumentAndSizeExist(G,F, M);
            if (arraysize == nullptr)
            {
                LOG_LINE("ARGUMENT CHECK INSIDE");
                arraySize = getArraySize(basePointer, M, builder);
                if(functionCloned){
                    LOG_LINE("BREAKING");
                    break;
                }
            }else{
                arraySize = arraysize;
            }
            LOG_LINE("arraysize = " << *arraySize);

            // CallInst *call =
            /*IRBuilder builderClone(&latestFunction->getEntryBlock());
            builderClone.SetInsertPoint()*/
            builder.SetInsertPoint(G);
            builder.CreateCall(BoundsCheckHelper, {offset, arraySize});
            LOG_LINE("INSERTED CREATE CALL");
                        LOG_LINE("WHOLE MODULE JUST LIKE THIS:\n\n" << M);

            //builder.Insert(call);
            Changed = true;
        }
    }
    return Changed;
}

Value* BoundsCheckerPass::argumentAndSizeExist(GetElementPtrInst *G,Function &F, Module &M)
{
    if (Value *bp = dyn_cast<Argument>(G->getOperand(0)))
    {
        Function *newFunc = &F;
        Type *Int32Type = Type::getInt32Ty(M.getContext());
        Argument *temp = new Argument(Int32Type);
        temp->setName(bp->getName() + "cas");
        LOG_LINE("operand name we are lkooking to match for: " << dyn_cast<Argument>(G->getOperand(0))->getName());
        LOG_LINE("if match: " << temp->getName());
        for (Function::arg_iterator AIT = newFunc->arg_begin(); AIT != newFunc->arg_end(); AIT++)
        {
            if (AIT->getName() == temp->getName())
            {
                LOG_LINE("MATCHED, size found in params");
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
    Value *newArg, *oldArg;
    Argument *nca;
    SmallVector<Value *, 2> oldArgs;
    SmallVector<Type *, 2> argTypes;
    Function *parentFunction = A->getParent();
    for (User *U : parentFunction->users())
    {
        if (CallInst *Ins = dyn_cast<CallInst>(U))
        {
            LOG_LINE("USER instruction" << *Ins);
            newArg = getArraySize(Ins, M, builder);
            LOG_LINE("new arg is: " << *newArg);
            oldArg = Ins->getArgOperand(0);
            ArrayRef<Type *> newParamTypes = llvm::ArrayRef<Type *>(Int32Ty);
            SmallVector<Argument *, 8> newArgs;
            Function *newFunc = addParamsToFunction(parentFunction, newParamTypes, newArgs);
            nca = newArgs.pop_back_val();
            nca->setName(A->getName() + "cas");
            for (Function::arg_iterator AIT = newFunc->arg_begin(); AIT != newFunc->arg_end(); AIT++)
            {
                if (StringRef(AIT->getName()) == StringRef(""))
                {
                    AIT->setName(nca->getName());
                }
            }
            Function *cloneFunc = cast<Function>(M.getOrInsertFunction(newFunc->getName(), newFunc->getReturnType(), Int32PtrTy, Int32Ty));
            builder.SetInsertPoint(Ins);
            builder.CreateCall(newFunc, {oldArg,newArg}); //CallInst *replInst = 
            Ins->eraseFromParent();
            //first should be ptr type
            LOG_LINE("WHOLE MODULE JUST LIKE THIS:\n\n" << M);
/*
            if(CallInst *UCI = dyn_cast<CallInst>(U)){
                ReplaceInstWithInst(UCI,replInst);
            }else{
                LOG_LINE("ERROR CAST FAILED");
            }*/
            deadList.push_back(parentFunction);
            LOG_LINE("SUCESSFULLYCLONED FUNCITONSSDFDF");
            instrumentGetElementPointers(*cloneFunc,M);
            functionCloned = true;
        }
    }
    return nca; //later nca
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
    }else if (CallInst *call = dyn_cast<CallInst>(basePointer)){
        return getArraySize(call->getOperand(0), M, builder);
    }
    else
    {   
        LOG_LINE("PRocessing: " << *basePointer);
        LOG_LINE("WHOLE MODULE JUST LIKE THIS:\n\n" << M);
        functionCloned = false;
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
        LOG_LINE("STARTING WITH FUNCTION "<< F);
        //Changed |= instrumentAllocations(F);
        Changed |= instrumentGetElementPointers(F, M);
        LOG_LINE("DONE WITH FUNCTION "<< F);

        if (Changed)
        {
            //
        }
    }
    for(Function *item: deadList)
    {
        item->eraseFromParent();
    }

    return Changed;
}

char BoundsCheckerPass::ID = 0;
static RegisterPass<BoundsCheckerPass> X("coco-boundscheck", "Example LLVM module pass that inserts prints for every allocation.");