#include <bits/stdint-uintn.h>
#include <llvm-10/llvm/IR/BasicBlock.h>
#include <llvm-10/llvm/IR/Constants.h>
#include <llvm-10/llvm/IR/DerivedTypes.h>
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

            // FunctionType *type = FunctionType::get(Type::getInt32Ty(context), {}, false);
            // FunctionCallee get_canay_func = F.getParent()->getOrInsertFunction("__kss_get_canary", type);
            // Value * canary = builder_entry.CreateCall(get_canay_func, {});
            IRBuilder<> builder_entry(&F.getEntryBlock().front());
            AllocaInst * canary_slot = builder_entry.CreateAlloca(Type::getInt32Ty(context));
            Value * t1 = builder_entry.CreateLoad(Type::getInt32Ty(context), gcanary, true);
            builder_entry.CreateStore(t1, canary_slot); 

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
                        Value *t2 = builder_epilogue.CreateLoad(Type::getInt32Ty(context), canary_slot, true);
                        builder_epilogue.CreateCall(canary_check, {t2});
                    }
                }
            }
        }
        if (F.getName().str() == "main")
        { 
            FunctionType *type = FunctionType::get(Type::getInt32Ty(context), {}, false);
            FunctionCallee get_canay_func = F.getParent()->getOrInsertFunction("__kss_get_canary", type);
            IRBuilder<> builder_init(&F.getEntryBlock().front());
            Value * canary = builder_init.CreateCall(get_canay_func, {});
            builder_init.CreateStore(canary, gcanary);
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
