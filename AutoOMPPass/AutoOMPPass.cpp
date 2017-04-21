#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

namespace {

    struct AutoOMPPass : public ModulePass {
        static char ID;
        AutoOMPPass() : ModulePass(ID) {}

        virtual bool runOnModule(Module &M) {
            return false;
        }
        virtual void getAnalysisUsage(AnalysisUsage &Info) const {
            
        }
    };
}

char AutoOMPPass::ID = 0;

static void AutoOMPPass(const PassManagerBuilder &, legacy::PassManagerBase &PM) {
    PM.add(new AutoOMPPass());
}
h
static RegisterPass<AutoOMPPass> X("reverseInst" , "Inst Reverse Pass", false, false);
