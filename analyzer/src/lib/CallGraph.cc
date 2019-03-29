//===-- CallGraph.cc - Build global call-graph------------------===//
// 
// This pass builds a global call-graph. The targets of an indirect
// call are identified based on type-analysis, i.e., matching the
// number and type of function parameters.
//
//===-----------------------------------------------------------===//

#include <llvm/IR/DebugInfo.h>
#include <llvm/Pass.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/Debug.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Constants.h>
#include <llvm/ADT/StringExtras.h>
#include <llvm/Analysis/CallGraph.h>

#include "CallGraph.h"
#include "Config.h"

using namespace llvm;


// Find targets of indirect calls based on type analysis: as long as
// the number and type of parameters of a function matches with the
// ones of the callsite, we say the function is a possible target of
// this call.
void CallGraphPass::findCalleesByType(CallInst *CI, FuncSet &S) {

	if (CI->isInlineAsm())
		return;

	CallSite CS(CI);
	for (Function *F : Ctx->AddressTakenFuncs) {

		// VarArg
		if (F->getFunctionType()->isVarArg()) {
			// Compare only known args in VarArg.
		}
		// otherwise, the numbers of args should be equal.
		else if (F->arg_size() != CS.arg_size()) {
			continue;
		}

		if (F->isIntrinsic()) {
			continue;
		}

		// Type matching on args.
		bool Matched = true;
		CallSite::arg_iterator AI = CS.arg_begin();
		for (Function::arg_iterator FI = F->arg_begin(), 
				FE = F->arg_end();
				FI != FE; ++FI, ++AI) {
			// Check type mis-matches.
			// Get defined type on callee side.
			Type *DefinedTy = FI->getType();
			// Get actual type on caller side.
			Type *ActualTy = (*AI)->getType();

      if (DefinedTy == ActualTy)
        continue;

      // FIXME: this is a tricky solution for disjoint
      // types in different modules. A more reliable
      // solution is required to evaluate the equality
      // of two types from two different modules.
      // Since each module has its own type table, same
      // types are duplicated in different modules. This
      // makes the equality evaluation of two types from
      // two modules very hard, which is actually done
      // at link time by the linker.
      while (DefinedTy->isPointerTy() && ActualTy->isPointerTy()) {
        DefinedTy = DefinedTy->getPointerElementType();
        ActualTy = ActualTy->getPointerElementType();
      }
      if (DefinedTy->isStructTy() && ActualTy->isStructTy() &&
          (DefinedTy->getStructName().equals(ActualTy->getStructName())))
        continue;
      if (DefinedTy->isIntegerTy() && ActualTy->isIntegerTy() &&
          DefinedTy->getIntegerBitWidth() == ActualTy->getIntegerBitWidth())
        continue;
      // TODO: more types to be supported.

			// Make the type analysis conservative: assume universal
			// pointers, i.e., "void *" and "char *", are equivalent to 
			// any pointer type and integer type.
			if (
					(DefinedTy == Int8PtrTy &&
					 (ActualTy->isPointerTy() || ActualTy == IntPtrTy)) 
					||
					(ActualTy == Int8PtrTy &&
					 (DefinedTy->isPointerTy() || DefinedTy == IntPtrTy))
					)
				continue;
			else {
        #if 0 // for debugging
        if (F->getName().compare("nfs_fs_mount") == 0) {
          OP << "DefinedTy @ " << DefinedTy << ": " << *DefinedTy << "\n";
          OP << "ActualTy @ " << ActualTy << ": " << *ActualTy << "\n";
        }
        #endif
				Matched = false;
				break;
			}
		}

		if (Matched)
			S.insert(F);
	}
}

bool CallGraphPass::doInitialization(Module *M) {

	DL = &(M->getDataLayout());
	Int8PtrTy = Type::getInt8PtrTy(M->getContext());
	IntPtrTy = DL->getIntPtrType(M->getContext());

	for (Function &F : *M) { 
		// Collect address-taken functions.
		if (F.hasAddressTaken())
			Ctx->AddressTakenFuncs.insert(&F);

		// Collect global function definitions.
		if (F.hasExternalLinkage() && !F.empty()) {
			// External linkage always ends up with the function name.
			StringRef FName = F.getName();
			// Special case: make the names of syscalls consistent.
			if (FName.startswith("SyS_"))
				FName = StringRef("sys_" + FName.str().substr(4));

			// Map functions to their names.
			Ctx->Funcs[FName] = &F;
		}
	}

	return false;
}

bool CallGraphPass::doFinalization(Module *M) {

	// Do nothing here.
	return false;
}

bool CallGraphPass::doModulePass(Module *M, StringRef mname) {

	// Use type-analysis to concervatively find possible targets of 
	// indirect calls.
	CallInst *myCI;
	for (Module::iterator f = M->begin(), fe = M->end(); 
			f != fe; ++f) {

		Function *F = &*f;
		for (inst_iterator i = inst_begin(F), e = inst_end(F); 
				i != e; ++i) {
			// Map callsite to possible callees.
			if (CallInst *CI = dyn_cast<CallInst>(&*i)) {
				FuncSet FS;
				Function *CF = CI->getCalledFunction();
				if (!CF) {
#ifdef SOUND_MODE
					findCalleesByType(CI, FS);
#endif
					Ctx->Callees[CI] = FS;

					for (Function *Callee : FS)
						Ctx->Callers[Callee].insert(CI);

					// Save called values for future uses.
					Ctx->IndirectCallInsts.push_back(CI);
				}
				else {
          if (CF->empty()) {
            StringRef FName = CF->getName();
		//std::cout << "FName: " << FName.Data << "\n";
            if (FName.startswith("SyS_"))
              FName = StringRef("sys_" + FName.str().substr(4));
		//std::cout << "FName: " << FName.str() << "\n";
            CF = Ctx->Funcs[FName];
          }
					FS.insert(CF);
					Ctx->Callees[CI] = FS;
					myCI = CI;
					Ctx->Callers[CF].insert(CI);
				}
			}
		}
	}
//	for (Function *myCallee : Ctx->Callees[myCI])
//	{
//		std::cout << "Size: " <<  myCallee->getFunctionType() << "\n";
//	}

  return false;
}
