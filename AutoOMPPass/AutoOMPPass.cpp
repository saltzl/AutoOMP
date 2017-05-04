#include "llvm/Pass.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"
#include "llvm/IR/IRBuilder.h"
#include <iostream>
#include <string>
#include <vector>

using namespace llvm;

namespace {

    struct AutoOMPPass : public ModulePass {
        static char ID;
        AutoOMPPass() : ModulePass(ID) {}


      std::vector<Loop*> getParallelizableLoops(LoopInfo &LI){
  			std::vector<Loop*> parallelizableLoops;

  			for(LoopInfo::iterator iter = LI.begin(), itend = LI.end(); iter != itend; ++iter){
  				if((*iter)->getUniqueExitBlock() == nullptr) continue;
          bool shouldParallelize = true;
          for(Loop::block_iterator BI = (*iter)->block_begin(), BE = (*iter)->block_end(); BI != BE; ++BI){
            for(BasicBlock::iterator II = (*BI)->begin(), IE = (*BI)->end(); II != IE; ++II){
              if(isa<CallInst>(II)){
                shouldParallelize = false;
                break;
              }
            }
            if(!shouldParallelize) break;
          }
  				//add other checks here
  				parallelizableLoops.push_back(static_cast<Loop*>(*iter));
  			}
        return parallelizableLoops;
    }
		//loop extractor: modified from "LoopExtractor.cpp" in the llvm source
    Function* extractLoop(Loop *L, DominatorTree &DT, LoopInfo &LI) {
			 // Only visit top-level loops.
 			if (L->getParentLoop())
  			return nullptr;

 			// If LoopSimplify form is not available, stay out of trouble.
 			if (!L->isLoopSimplifyForm())
  			return nullptr;

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
  			
 			  CodeExtractor Extractor(DT, *L);
			  extracted = Extractor.extractCodeRegion();
   			if (extracted != nullptr) {
     				Changed = true;
     			// After extraction, the loop is replaced by a function call, so
    			 // we shouldn't try to run any more loop passes on it.
     				LI.markAsRemoved(L);
   			}
 			}
 			return extracted;
		}

    void createHeader(Function* extracted){
      BasicBlock* entry = &extracted->getEntryBlock();

      BasicBlock* header = BasicBlock::Create(entry->getContext(), "", extracted, entry);
      IRBuilder<> builder(header);
      Type* i32type = Type::getInt32PtrTy(header->getContext());
      auto global_tid = builder.createAlloca(i32type);
      auto bound_tid = builder.createAlloca(i32type);
      auto c_addr = builder.createAlloca(i32type);
      std::vector<Instruction*> arrays;
      int num_iterations= 0;
      for(Function::iterator BB = extracted->begin(); BB != extracted->end(); ++BB){
        for(BasicBlock::iterator Inst = BB->begin(); Inst != BB->end(); ++Inst){
          if(isa<GetElementPtrInst>(Inst)){
            arrays.push_back(builder.createAlloca(i32type, cast<GetElementPtrInst>(Inst)->getNumIndices());
              num_iterations = cast<GetElementPtrInst>(Inst)->getNumIndices();
          }
        }
      }

      auto iv = builder.createAlloca(i32type);
      auto lb =builder.createAlloca(i32type);
      auto ub = builder.createAlloca(i32type);
      auto stride = builder.createAlloca(i32type);
      auto islast = builder.createAlloca(i32type);

      //extract function made sure the loop was cannonicallized
      builder.createStore(0, lb);
      builder.createStore(num_iterations - 1, ub);
      builder.createStore(1, stride);
      builder.createStore(0, islast);

      //

    }
		
    bool runOnModule(Module &M) {
      std::cerr << "AUTO OMP PASS" << std::endl;


      for(Module::iterator fiter = M.getFunctionList().begin(); fiter != M.getFunctionList().end(); ++fiter){

            LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>(*fiter).getLoopInfo();
      			std::vector<Loop*> loops = getParallelizableLoops(LI);
      			DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>(*fiter).getDomTree();
            M.dump();

      			for(std::vector<Loop*>::iterator iter = loops.begin(), iend = loops.end(); iter != iend; ++iter){
              (*iter)->dump();
              BasicBlock pred = (*iter)->getLoopPredecessor();


      				Function* extracted = extractLoop(*iter, DT, LI);
      				if(!extracted) continue;



              IRBuilder<> builder(pred);

              builder.CreateCall(extracted);


              createHeader(extracted);

      				//create header, function call for loop;
      			}
            M.dump();
          }
      std::cerr << "END AUTO OMP PASS" << std::endl;

      return false;
    }
    void getAnalysisUsage(AnalysisUsage &AU) const {
        AU.addRequired<LoopInfoWrapperPass>();
        AU.addRequired<DominatorTreeWrapperPass>();
    }
  };
}

char AutoOMPPass::ID = 0;

static void registerAutoOMPPass(const PassManagerBuilder &, legacy::PassManagerBase &PM) {
    PM.add(new AutoOMPPass());
}
static RegisterPass<AutoOMPPass> X("autoOMP" , "Auto OMP Pass", false, false);

