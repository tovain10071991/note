/*
 * $ gdb ~/Documents/llvm-3.4/build/Release+Debug+Asserts/bin/opt
 * (gdb)start -load ~/Documents/llvm-3.4/lib/Transforms/TestPass/TestPass.so -TestPass Hello.bc -o /dev/null
 * (gdb)break 853	//Passes.run(*M.get());
 */

/* ----------------------------------------------
 * llvm-3.4/tools/opt/opt.cpp:853
 ------------------------------------------------*/

int main(int argc, char **argv) {
	...
	// Now that we have all of the passes ready, run them.
	Passes.run(*M.get());
	...
}

/* ----------------------------------------------
 * llvm-3.4/lib/IR/LegacyPassManager.cpp:1746
 ------------------------------------------------*/

/// run - Execute all of the passes scheduled for execution.  Keep track of
/// whether any of the passes modifies the module, and if so, return true.
bool PassManager::run(Module &M) {
  return PM->run(M);
}

/* ----------------------------------------------
 * llvm-3.4/lib/IR/LegacyPassManager.cpp:1696
 ------------------------------------------------*/

/// run - Execute all of the passes scheduled for execution.  Keep track of
/// whether any of the passes modifies the module, and if so, return true.
bool PassManagerImpl::run(Module &M) {
  bool Changed = false;
  TimingInfo::createTheTimeInfo();

  dumpArguments();
  dumpPasses();

  SmallVectorImpl<ImmutablePass *>& IPV = getImmutablePasses();
  for (SmallVectorImpl<ImmutablePass *>::const_iterator I = IPV.begin(),
       E = IPV.end(); I != E; ++I) {
    Changed |= (*I)->doInitialization(M);
  }

  initializeAllAnalysisInfo();
  for (unsigned Index = 0; Index < getNumContainedManagers(); ++Index)
    Changed |= getContainedManager(Index)->runOnModule(M);	//此处调用了TestPass

  for (SmallVectorImpl<ImmutablePass *>::const_iterator I = IPV.begin(),
       E = IPV.end(); I != E; ++I) {
    Changed |= (*I)->doFinalization(M);
  }

  return Changed;
}