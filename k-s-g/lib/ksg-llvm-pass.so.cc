#include <bits/stdint-uintn.h>
#include <llvm-10/llvm/IR/BasicBlock.h>
#include <llvm-10/llvm/IR/Constants.h>
#include <llvm-10/llvm/IR/DerivedTypes.h>
#include <llvm-10/llvm/IR/GlobalValue.h>
#include <llvm-10/llvm/IR/GlobalVariable.h>
#include <llvm-10/llvm/Support/Casting.h>
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
    GlobalVariable * gcanary = NULL;

    bool containsArray(Function &F){
        for (const BasicBlock &BB : F){
            for (const Instruction &I : BB){
                if (const AllocaInst *AI = dyn_cast<AllocaInst>(&I)) {
                    if (ArrayType * AT = dyn_cast<ArrayType>(AI->getAllocatedType())){
                        return true;
                    } 
                }
            }
        }
        return false;
    }

    BasicBlock * getFailBB(Function &F){
        LLVMContext &context = F.getContext();
        BasicBlock *FailBB = BasicBlock::Create(context, "", &F);
        IRBuilder<> builder_fail(FailBB);
        FunctionType *type = FunctionType::get(Type::getVoidTy(context), {}, false);
        FunctionCallee canary_check_fail = F.getParent()->getOrInsertFunction("__kss_stack_chk_fail", type);
        builder_fail.CreateCall(canary_check_fail, {});
        builder_fail.CreateUnreachable();
        return FailBB;
    }

    bool runOnFunction(Function &F) override
    {
        LLVMContext &context = F.getParent()->getContext();
        if (!gcanary){
            gcanary = new GlobalVariable(*F.getParent(), Type::getInt32Ty(F.getParent()->getContext()), false, GlobalValue::ExternalLinkage, NULL, "__kss_canary");
        }
        if (F.getName().startswith("__kss"))
        {
            return false;
        }
        if (containsArray(F)){
            printf("[+] Implemente @ %s.\n", F.getName().str().c_str());

            IRBuilder<> builder_entry(&F.getEntryBlock().front());
            AllocaInst * canary_slot = builder_entry.CreateAlloca(Type::getInt32Ty(context));
            Value * t1 = builder_entry.CreateLoad(Type::getInt32Ty(context), gcanary, true);
            builder_entry.CreateStore(t1, canary_slot); 

            for (Function::iterator I = F.begin(), E = F.end(); I != E;)
            {
                BasicBlock &BB = *I++;
                ReturnInst *IST = dyn_cast<ReturnInst>(BB.getTerminator());
                if (IST)
                {
                    BasicBlock *FailBB = getFailBB(F);

                    BasicBlock *NewBB = BB.splitBasicBlock(IST->getIterator());
                    BB.getTerminator()->eraseFromParent();
                    NewBB->moveAfter(&BB);

                    IRBuilder<> builder_epilogue(&BB);
                    Value *canary_local = builder_epilogue.CreateLoad(Type::getInt32Ty(context), canary_slot, true);
                    Value *canary_global = builder_epilogue.CreateLoad(Type::getInt32Ty(context), gcanary, true);
                    Value *Cmp = builder_epilogue.CreateICmpEQ(canary_local, canary_global);
                    builder_epilogue.CreateCondBr(Cmp, NewBB, FailBB);
                }
            }
        }
        if (F.getName().str() == "main")
        { 
            FunctionType *type = FunctionType::get(Type::getVoidTy(context), {}, false);
            FunctionCallee get_canay_func = F.getParent()->getOrInsertFunction("__kss_get_canary", type);
            IRBuilder<> builder_init(&F.getEntryBlock().front());
            builder_init.CreateCall(get_canay_func, {});
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
