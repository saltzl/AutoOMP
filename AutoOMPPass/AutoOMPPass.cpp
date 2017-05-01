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


        std::vector<Loop*> getParallelizableLoops(LoopInfo &LI){
			std::vector<Loop*> parallelizableLoops;

			for(LoopInfo::iterator iter = LI.begin(), itend = LI.end(); iter != itend; ++iter){
				if(iter->getUniqueExitBlock() == nullptr) continue;
				//add other checks here
				parallelizableLoops.push_back(static_cast<Loop*>(iter));
			}


		}

		//loop extractor: modified from "LoopExtractor.cpp" in the llvm source
        Function* extractLoop(Loop *L, DominatorTree &DT, LoopInfo &LI) {
            if (skipOptnoneFunction(L))
                return false;
 
  			 // Only visit top-level loops.
   			if (L->getParentLoop())
    			return false;
 
   			// If LoopSimplify form is not available, stay out of trouble.
   			if (!L->isLoopSimplifyForm())
    			return false;
 
   			bool Changed = false;
 
		   // If there is more than one top-level loop in this function, extract all of
   		 // the loops. Otherwise there is exactly one top-level loop; in this case if
  		 // this function is more than a minimal wrapper around the loop, extract
  		 // the loop.
   			bool ShouldExtractLoop = false;
 
   		// Extract the loop if the entry block doesn't branch to the loop header.
	   		TerminatorInst *EntryTI = L->getHeader()->getParent()->getEntryBlock().getTerminator();
  		
			 if (!isa<BranchInst>(EntryTI) ||  !cast<BranchInst>(EntryTI)->isUnconditional() || EntryTI->getSuccessor(0) != L->getHeader()) {
    	 		ShouldExtractLoop = true;
   			} else {
     		// Check to see if any exits from the loop are more than just return
     		// blocks.
    	 		SmallVector<BasicBlock*, 8> ExitBlocks;
     			L->getExitBlocks(ExitBlocks);
     			for (unsigned i = 0, e = ExitBlocks.size(); i != e; ++i)
     			    if (!isa<ReturnInst>(ExitBlocks[i]->getTerminator())) {
        	 			ShouldExtractLoop = true;
        	 			break;
       				}
   			}
 
   			if (ShouldExtractLoop) {
    		 // We must omit EH pads. EH pads must accompany the invoke
    		 // instruction. But this would result in a loop in the extracted
     		// function. An infinite cycle occurs when it tries to extract that loop as
     		// well.
   	  			SmallVector<BasicBlock*, 8> ExitBlocks;
    	 		L->getExitBlocks(ExitBlocks);
     			for (unsigned i = 0, e = ExitBlocks.size(); i != e; ++i)
     				if (ExitBlocks[i]->isEHPad()) {
         				ShouldExtractLoop = false;
        	 			break;
       				}
   			}
 			Function* extracted = nullptr;
   			if (ShouldExtractLoop) {
    			 if (NumLoops == 0) return Changed;
    			 --NumLoops;
     			CodeExtractor Extractor(DT, *L);
				extracted = Extractor.extractCodeRegion();
     			if (extracted != nullptr) {
       				Changed = true;
       			// After extraction, the loop is replaced by a function call, so
      			 // we shouldn't try to run any more loop passes on it.
       				LI.markAsRemoved(L);
     			}
     			++NumExtracted;
   			}
 			return extracted;
		}

		
        bool runOnModule(Module &M) {
            LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
			std::vector<Loop*> loops = getParallelizableLoops(LI);
			
			for(std::vector<Loop*>::iterator iter = loops.begin(), iend = loops.end(); iter != iend; ++iter){
				Function* extracted = extractLoop(iter);
				if(!extracted) continue;
				//create header, function call for loop;
			}


            return false;
        }
        void getAnalysisUsage(AnalysisUsage &AU) const {
            AU.addRequired<LoopInfoWrapperPass>();
        }
    };
}

char AutoOMPPass::ID = 0;

static void AutoOMPPass(const PassManagerBuilder &, legacy::PassManagerBase &PM) {
    PM.add(new AutoOMPPass());
}
h
static RegisterPass<AutoOMPPass> X("reverseInst" , "Inst Reverse Pass", false, false);
