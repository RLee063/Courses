#include <llvm-10/llvm/IR/BasicBlock.h>
#include <string>
#include <system_error>
#include <iostream>

#include "llvm/Support/raw_ostream.h"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

struct MyPlacementPass : public FunctionPass
{
    static char ID;
    MyPlacementPass() : FunctionPass(ID){}

    bool runOnFunction(Function &F) override
    {
        if (F.getName().startswith("__kss"))
        {
            return false;
        }
        printf("[+] Implementation @ %s.\n", F.getName().str().c_str());
        LLVMContext &context = F.getParent()->getContext();

        FunctionType *type = FunctionType::get(Type::getInt32Ty(context), {}, false);
        FunctionCallee get_canay_func = F.getParent()->getOrInsertFunction("__kss_get_canary", type);
        IRBuilder<> builder_entry(&F.getEntryBlock().front());
        AllocaInst * canary_slot = builder_entry.CreateAlloca(Type::getInt32Ty(context));
        Value * canary = builder_entry.CreateCall(get_canay_func, {});
        builder_entry.CreateStore(canary, canary_slot); 

        for (Function::iterator I = F.begin(), E = F.end(); I != E; ++I)
        {
            BasicBlock &BB = *I;
            for (BasicBlock::iterator I = BB.begin(), E = BB.end(); I != E; ++I)
            {
                ReturnInst *IST = dyn_cast<ReturnInst>(I);
                if (IST)
                {
                    FunctionType *type = FunctionType::get(Type::getVoidTy(context), {Type::getInt32Ty(context)}, false);
                    FunctionCallee canary_check = BB.getModule()->getOrInsertFunction("__kss_stack_chk", type);
                    IRBuilder<> builder_epilogue(IST);
                    canary = builder_epilogue.CreateLoad(Type::getInt32Ty(context), canary_slot, true);
                    builder_epilogue.CreateCall(canary_check, {canary});
                }
            }
        }
        return false;
    }
};

char MyPlacementPass::ID = 0;

static void registerSkeletonPass(const PassManagerBuilder &, legacy::PassManagerBase &PM) 
{
    PM.add(new MyPlacementPass());
}

static RegisterStandardPasses RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible, registerSkeletonPass);
