/*
 * gdb klee
 * start Hello.bc
 */

/*-------------------------------------------------
 * klee/tools/klee/main.cpp
  -------------------------------------------------*/

int main(int argc, char **argv, char **envp) {
	//argc = 2, argv[0]="/home/user/Documents/klee/build/Release+Debug+Asserts/bin/klee", argv[1]="/home/user/Documents/test/LLVM_Pass_test/Hello.bc"
	atexit(llvm_shutdown);	//注册终止函数，函数退出时调用llvm_shutdown
	llvm::InitializeNativeTarget();
	//这个函数主要是用来初始化相应于主机的目标机，JIT应用程序通过这个函数调用来确保目标机的相应库已经链接到程序中
	parseArguments(argc, argv);	//解析命令行
	sys::PrintStackTraceOnErrorSignal();
	//如果程序收到错误信号(如SIBABRT或SIGSEGV)，打印stack trace并退出
	if (Watchdog) {...}
	sys::SetInterruptFunction(interrupt_handle);
	//注册中断函数，当按下ctrl+c时调用
	
	// Load the bytecode...
	std::string ErrorMsg;
	Module *mainModule = 0;
#if LLVM_VERSION_CODE < LLVM_VERSION(3, 5)
	OwningPtr<MemoryBuffer> BufferPtr;
	//智能指针，反正就是指针
	//MemoryBuffer类(include/llvm/Support/MemoryBuffer.h)，对内存块的只读访问，将文件或输入流写入内存，访问文件
	error_code ec=MemoryBuffer::getFileOrSTDIN(InputFile.c_str(), BufferPtr);	//打开指定文件或输入流
	//error_code类(include/llvm/Support/System_error.h)
	if (ec) {...}
	mainModule = getLazyBitcodeModule(BufferPtr.get(), getGlobalContext(), &ErrorMsg);	//getLazyBitcodeModule函数(include/llvm/bitcode/ReadWriter.h)，读取BufferPtr的头部，对函数体延迟反序列化，返回buffer的拥有者模块
	if (mainModule) {
		if (mainModule->MaterializeAllPermanently(&ErrorMsg)) {...}
	}
	if (!mainModule) ...
#else
	...
#endif
	if (WithPOSIXRuntime){...}
	std::string LibraryDir = KleeHandler::getRunTimeLibraryPath(argv[0]);
	//LibraryDir.c_str() = "/home/user/Documents/klee/build/Debug+Asserts/lib"
	Interpreter::ModuleOptions Opts(LibraryDir.c_str(),
									/*Optimize=*/OptimizeModule, 
                                  /*CheckDivZero=*/CheckDivZero,
                                  /*CheckOvershift=*/CheckOvershift);
	/*
	 *	Opt{
	 *		std::string LibraryDir = "/home/user/Documents/klee/build/Debug+Asserts/lib;
	 *		bool Optimize = false;
	 *		bool CheckDivZero = true;
	 *		bool CheckOvershift = true;
	 *	}
	 */
	switch (Libc) {
	case NoLibc: /* silence compiler warning */
		break;
	...
	}
	if (WithPOSIXRuntime) {...}
	
	// Get the desired main function.  klee_main initializes uClibc locale and other data and then calls main.
	// 准备调用被测试程序的main函数
	Function *mainFn = mainModule->getFunction("main");
	if (!mainFn) {...}
	// FIXME: Change me to std types.
	int pArgc;
	char **pArgv;
	char **pEnvp;
	if (Environ != "") {...}
	else {
		pEnvp = envp;
	}
	pArgc = InputArgv.size() + 1; 	//1
	pArgv = new char *[pArgc];
	//下面循环将 $ klee proc proc_args中proc_args复制到pArgv(包括proc名)
	for (unsigned i=0; i<InputArgv.size()+1; i++) {
		//执行了一次
		std::string &arg = (i==0 ? InputFile : InputArgv[i-1]);
		//arg.c_str()="Hello.bc"
		unsigned size = arg.size() + 1;	//9
		char *pArg = new char[size];
		std::copy(arg.begin(), arg.end(), pArg);
		pArg[size - 1] = 0;
		pArgv[i] = pArg;
	}
	// pArgv[0] = "/home/user/Documents/test/LLVM_Pass_test/Hello.bc"
	std::vector<bool> replayPath;
	if (ReplayPathFile != "") {...}
	Interpreter::InterpreterOptions IOpts;
	IOpts.MakeConcreteSymbolic = MakeConcreteSymbolic;	// 0
	KleeHandler *handler = new KleeHandler(pArgc, pArgv);	//创建了输出目录
	//输出：KLEE: output directory is "/home/user/Documents/test/test_LLVM_Pass/klee-out-0"
	Interpreter *interpreter = theInterpreter = Interpreter::create(IOpts, handler);	//创建一个Executor，lib/core/Executor.cpp
	handler->setInterpreter(interpreter);	// m_interpreter = interpreter;
	llvm::raw_ostream &infoFile = handler->getInfoStream(); // infoFile = m_infoFile
	for (int i=0; i<argc; i++) {
		infoFile << argv[i] << (i+1<argc ? " ":"\n");
	}
	infoFile << "PID: " << getpid() << "\n";
	const Module *finalModule = 
	interpreter->setModule(mainModule, Opts);
	externalsAndGlobalsCheck(finalModule);
	//输出文件assembly.ll, run.istats, tun.stats(没注意是上面两个函数的哪一个输出的)
	//输出：KLEE: WARNING: undefined reference to function: printf
	if (ReplayPathFile != "") {...}
	char buf[256];
	time_t t[2];
	t[0] = time(NULL);
	strftime(buf, sizeof(buf), "Started: %Y-%m-%d %H:%M:%S\n", localtime(&t[0]));
	// buf = "Started: 2015-04-09 11:02:12\n"
	infoFile << buf;
	infoFile.flush();
	if (!ReplayOutDir.empty() || !ReplayOutFile.empty()) {...}
	else {
		std::vector<KTest *> seeds;
		for (std::vector<std::string>::iterator it = SeedOutFile.begin(), ie = SeedOutFile.end(); it != ie; ++it) {...}
		for (std::vector<std::string>::iterator it = SeedOutDir.begin(), ie = SeedOutDir.end(); it != ie; ++it) {...}
		if (!seeds.empty()) {...}
		if (RunInDir != "") {...}
/*!!*/	interpreter->runFunctionAsMain(mainFn, pArgc, pArgv, pEnvp);
		//输出：KLEE: WARNING ONCE: calling external: printf(32027328)
		//输出：Hello LLVM
		while (!seeds.empty()) {...}
	}
	t[1] = time(NULL);
	strftime(buf, sizeof(buf), "Finished: %Y-%m-%d %H:%M:%S\n", localtime(&t[1]));
	infoFile << buf;
	strcpy(buf, "Elapsed: ");
	strcpy(format_tdiff(buf, t[1] - t[0]), "\n");
	infoFile << buf;
	// Free all the args.
	for (unsigned i=0; i<InputArgv.size()+1; i++)
		delete[] pArgv[i];
	delete[] pArgv;

	delete interpreter;
	uint64_t queries = *theStatisticManager->getStatisticByName("Queries");	//0
	uint64_t queriesValid = *theStatisticManager->getStatisticByName("QueriesValid");	//0
	uint64_t queriesInvalid = *theStatisticManager->getStatisticByName("QueriesInvalid");	//0
	uint64_t queryCounterexamples = *theStatisticManager->getStatisticByName("QueriesCEX");	//0
	uint64_t queryConstructs = *theStatisticManager->getStatisticByName("QueriesConstructs");	//0
	uint64_t instructions = *theStatisticManager->getStatisticByName("Instructions");	//4
	uint64_t forks = *theStatisticManager->getStatisticByName("Forks");	//0

	handler->getInfoStream() << "KLEE: done: explored paths = " << 1 + forks << "\n";
	
	// Write some extra information in the info file which users won't
	// necessarily care about or understand.
	if (queries) ...
	handler->getInfoStream()
	<< "KLEE: done: total queries = " << queries << "\n"
	<< "KLEE: done: valid queries = " << queriesValid << "\n"
	<< "KLEE: done: invalid queries = " << queriesInvalid << "\n"
	<< "KLEE: done: query cex = " << queryCounterexamples << "\n";

	std::stringstream stats;
	stats	<< "\n";
	stats	<< "KLEE: done: total instructions = " 
			<< instructions << "\n";
	stats	<< "KLEE: done: completed paths = " 
			<< handler->getNumPathsExplored() << "\n";
	stats	<< "KLEE: done: generated tests = " 
			<< handler->getNumTestCases() << "\n";

	bool useColors = llvm::errs().is_displayed();
	if (useColors)
		llvm::errs().changeColor(llvm::raw_ostream::GREEN, /*bold=*/true, /*bg=*/false);
	//显示变绿色
	llvm::errs() << stats.str();
	/* 输出：
	 * KLEE: done: total instructions = 4
	 * KLEE: done: completed paths = 1
	 * KLEE: done: generated tests = 1
	 */
	if (useColors)
		llvm::errs().resetColor();
	//显示变回白色

	handler->getInfoStream() << stats.str();

#if LLVM_VERSION_CODE < LLVM_VERSION(3, 5)
	// FIXME: This really doesn't look right
	// This is preventing the module from being
	// deleted automatically
	BufferPtr.take();
#endif
	delete handler;

	return 0;
}
	
/*-------------------------------------------------
 * klee/lib/Core/Executor.cpp
  -------------------------------------------------*/

// 上面121行调用
// klee/lib/Core/Executor.cpp:3335
void Executor::runFunctionAsMain(Function *f, int argc, char **argv, char **envp) {
	// f表示main, argc=1, argc[0]=".../Hello.bc"
	std::vector<ref<Expr> > arguments;
	// force deterministic initialization of memory objects
	srand(1);
	srandom(1);
	MemoryObject *argvMO = 0;
	// In order to make uclibc happy and be closer to what the system is doing we lay out the environments at the end of the argv array (both are terminated by a null). There is also a final terminating null that uclibc seems to expect, possibly the ELF header?
	int envc;
	for (envc=0; envp[envc]; ++envc) ;	// envc = 74
	unsigned NumPtrBytes = Context::get().getPointerWidth() / 8;		// NumPtrBytes = 8
	KFunction *kf = kmodule->functionMap[f];
	//kmodule->functionMap, function到kfunction的映射
	/*
	 *	*kf {
	 *		llvm::Function* function = 0x1e12e20;
	 *		unsigned numArgs = 0;
	 *		unsigned numRegisters = 4;
	 *		numInstructions = 4;
	 *		KInstruction** instructions = 0x1e74c00;
	 *		std::map<llvm::BasicBlock*, unsigned> basicBlockEntry;
	 *		trackCoverage = true
	 *	};
	 */
	...
	Function::arg_iterator ai = f->arg_begin(), ae = f->arg_end();
	if (ai!=ae) {...}
	ExecutionState *state = new ExecutionState(kmodule->functionMap[f]);
	/*
		*state {
			std::map<std::string, std::string> fnAliases;
			bool fakeState = false;
			unsigned depth = 0;
			KInstIterator pc = {it = 0x100000000};
			KInstIterator prevPC = {it = 0x0};
			std::vector<StackFrame> stack;
			ConstraintManager constraints;
			mutable double queryCost = 6.9533490674397384e-310;
			double weight = 0;
			AddressSpace addressSpace;
			TreeOStream pathOS = {writer = 0x0, id = 1};
			TreeOStream symPathOS = {writer = 0x7ffff7fe04f8, id = 0};
			unsigned instsSinceCovNew = 4160617784;
			bool coveredNew = 255;
			bool forkDisabled = 127;
			std::map<const std::string*, std::set<unsigned>> coveredLines;
			PTreeNode* ptreeNode = 0xffff800000002541;
			std::vector< std::pair<const MemoryObject*, const Array*>> symbolics;
			std::set<std::string> arrayNames;
			MemoryMap shadowObjects;
			unsigned incomingBBIndex = 4294957759
		};
	 */
	if (pathWriter) ...
	if (symPathWriter) ...
	if (statsTracker)
    statsTracker->framePushed(*state, 0);
	...
	for (unsigned i = 0, e = f->arg_size(); i != e; ++i) ...

	if (argvMO) {...}
	
	initializeGlobals(*state);

	processTree = new PTree(state);
	state->ptreeNode = processTree->root;
	run(*state);
	// 输出"KLEE: WARNING ONCE: calling external: printf(32027440)"
	// 输出"Hello LLVM!"
	delete processTree;
	processTree = 0;

	// hack to clear memory objects
	delete memory;
	memory = new MemoryManager();
	
	globalObjects.clear();
	globalAddresses.clear();

	if (statsTracker)
	statsTracker->done();
}

void Executor::run(ExecutionState &initialState) {
	bindModuleConstants();

	// Delay init till now so that ticks don't accrue during
	// optimization and such.
	initTimers();

	states.insert(&initialState);

	if (usingSeeds) {...}
	
	searcher = constructUserSearcher(*this);

	searcher->update(0, states, std::set<ExecutionState*>());

	while (!states.empty() && !haltExecution) {
		//逐条执行LLVM汇编
		ExecutionState &state = searcher->selectState();
		KInstruction *ki = state.pc;
		stepInstruction(state);
		executeInstruction(state, ki);
		//执行LLVM汇编
		processTimers(&state, MaxInstructionTime);

		if (MaxMemory) {
			if ((stats::instructions & 0xFFFF) == 0) {...}
		}
			updateStates(&state);
	}
	delete searcher;
	searcher = 0;

dump:
	if (DumpStatesOnHalt && !states.empty()) {...}
}

void Executor::stepInstruction(ExecutionState &state) {
	if (DebugPrintInstructions) {...}
	if (statsTracker)
		statsTracker->stepInstruction(state);
	++stats::instructions;
	state.prevPC = state.pc;
	++state.pc;
	if (stats::instructions==StopAfterNInstructions)
		haltExecution = true;
}

void Executor::executeInstruction(ExecutionState &state, KInstruction *ki) {
	Instruction *i = ki->inst;
	switch (i->getOpcode()) {
	// Control flow
	case Instruction::Ret: {...}
#if LLVM_VERSION_CODE < LLVM_VERSION(3, 1)
	...
#endif
  case Instruction::Br: {...}
  case Instruction::Switch: {...}
  case Instruction::Unreachable:
    ...
  case Instruction::Invoke:
  case Instruction::Call: {...}
  case Instruction::PHI: {...}
    // Special instructions
  case Instruction::Select: {...}
  case Instruction::VAArg:
    ...
    // Arithmetic / logical
  case Instruction::Add: {...}
  case Instruction::Sub: {...}
  case Instruction::Mul: {...}
  case Instruction::UDiv: {...}
  case Instruction::SDiv: {...}
  case Instruction::URem: {...}
  case Instruction::SRem: {...}
  case Instruction::And: {...}
  case Instruction::Or: {...}
  case Instruction::Xor: {...}
  case Instruction::Shl: {...}
  case Instruction::LShr: {...}
  case Instruction::AShr: {...}
    // Compare
  case Instruction::ICmp: {...}
    // Memory instructions...
  case Instruction::Alloca: {
    AllocaInst *ai = cast<AllocaInst>(i);
    unsigned elementSize = 
      kmodule->targetData->getTypeStoreSize(ai->getAllocatedType());
    ref<Expr> size = Expr::createPointer(elementSize);
    if (ai->isArrayAllocation()) {...}
    bool isLocal = i->getOpcode()==Instruction::Alloca;
    executeAlloc(state, size, isLocal, ki);
    break;
  }

  case Instruction::Load: {
    ref<Expr> base = eval(ki, 0, state).value;
    executeMemoryOperation(state, false, base, 0, ki);
    break;
  }
  case Instruction::Store: {
    ref<Expr> base = eval(ki, 1, state).value;
    ref<Expr> value = eval(ki, 0, state).value;
    executeMemoryOperation(state, true, base, value, 0);
    break;
  }

  case Instruction::GetElementPtr: {
    KGEPInstruction *kgepi = static_cast<KGEPInstruction*>(ki);
    ref<Expr> base = eval(ki, 0, state).value;

    for (std::vector< std::pair<unsigned, uint64_t> >::iterator 
           it = kgepi->indices.begin(), ie = kgepi->indices.end(); 
         it != ie; ++it) {
      uint64_t elementSize = it->second;
      ref<Expr> index = eval(ki, it->first, state).value;
      base = AddExpr::create(base,
                             MulExpr::create(Expr::createSExtToPointerWidth(index),
                                             Expr::createPointer(elementSize)));
    }
    if (kgepi->offset)
      base = AddExpr::create(base,
                             Expr::createPointer(kgepi->offset));
    bindLocal(ki, state, base);
    break;
  }

    // Conversion
  case Instruction::Trunc: {
    CastInst *ci = cast<CastInst>(i);
    ref<Expr> result = ExtractExpr::create(eval(ki, 0, state).value,
                                           0,
                                           getWidthForLLVMType(ci->getType()));
    bindLocal(ki, state, result);
    break;
  }
  case Instruction::ZExt: {
    CastInst *ci = cast<CastInst>(i);
    ref<Expr> result = ZExtExpr::create(eval(ki, 0, state).value,
                                        getWidthForLLVMType(ci->getType()));
    bindLocal(ki, state, result);
    break;
  }
  case Instruction::SExt: {
    CastInst *ci = cast<CastInst>(i);
    ref<Expr> result = SExtExpr::create(eval(ki, 0, state).value,
                                        getWidthForLLVMType(ci->getType()));
    bindLocal(ki, state, result);
    break;
  }

  case Instruction::IntToPtr: {
    CastInst *ci = cast<CastInst>(i);
    Expr::Width pType = getWidthForLLVMType(ci->getType());
    ref<Expr> arg = eval(ki, 0, state).value;
    bindLocal(ki, state, ZExtExpr::create(arg, pType));
    break;
  } 
  case Instruction::PtrToInt: {
    CastInst *ci = cast<CastInst>(i);
    Expr::Width iType = getWidthForLLVMType(ci->getType());
    ref<Expr> arg = eval(ki, 0, state).value;
    bindLocal(ki, state, ZExtExpr::create(arg, iType));
    break;
  }

  case Instruction::BitCast: {
    ref<Expr> result = eval(ki, 0, state).value;
    bindLocal(ki, state, result);
    break;
  }

    // Floating point instructions

  case Instruction::FAdd: {
    ref<ConstantExpr> left = toConstant(state, eval(ki, 0, state).value,
                                        "floating point");
    ref<ConstantExpr> right = toConstant(state, eval(ki, 1, state).value,
                                         "floating point");
    if (!fpWidthToSemantics(left->getWidth()) ||
        !fpWidthToSemantics(right->getWidth()))
      return terminateStateOnExecError(state, "Unsupported FAdd operation");

#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 3)
    llvm::APFloat Res(*fpWidthToSemantics(left->getWidth()), left->getAPValue());
    Res.add(APFloat(*fpWidthToSemantics(right->getWidth()),right->getAPValue()), APFloat::rmNearestTiesToEven);
#else
    llvm::APFloat Res(left->getAPValue());
    Res.add(APFloat(right->getAPValue()), APFloat::rmNearestTiesToEven);
#endif
    bindLocal(ki, state, ConstantExpr::alloc(Res.bitcastToAPInt()));
    break;
  }

  case Instruction::FSub: {
    ref<ConstantExpr> left = toConstant(state, eval(ki, 0, state).value,
                                        "floating point");
    ref<ConstantExpr> right = toConstant(state, eval(ki, 1, state).value,
                                         "floating point");
    if (!fpWidthToSemantics(left->getWidth()) ||
        !fpWidthToSemantics(right->getWidth()))
      return terminateStateOnExecError(state, "Unsupported FSub operation");
#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 3)
    llvm::APFloat Res(*fpWidthToSemantics(left->getWidth()), left->getAPValue());
    Res.subtract(APFloat(*fpWidthToSemantics(right->getWidth()), right->getAPValue()), APFloat::rmNearestTiesToEven);
#else
    llvm::APFloat Res(left->getAPValue());
    Res.subtract(APFloat(right->getAPValue()), APFloat::rmNearestTiesToEven);
#endif
    bindLocal(ki, state, ConstantExpr::alloc(Res.bitcastToAPInt()));
    break;
  }
 
  case Instruction::FMul: {
    ref<ConstantExpr> left = toConstant(state, eval(ki, 0, state).value,
                                        "floating point");
    ref<ConstantExpr> right = toConstant(state, eval(ki, 1, state).value,
                                         "floating point");
    if (!fpWidthToSemantics(left->getWidth()) ||
        !fpWidthToSemantics(right->getWidth()))
      return terminateStateOnExecError(state, "Unsupported FMul operation");

#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 3)
    llvm::APFloat Res(*fpWidthToSemantics(left->getWidth()), left->getAPValue());
    Res.multiply(APFloat(*fpWidthToSemantics(right->getWidth()), right->getAPValue()), APFloat::rmNearestTiesToEven);
#else
    llvm::APFloat Res(left->getAPValue());
    Res.multiply(APFloat(right->getAPValue()), APFloat::rmNearestTiesToEven);
#endif
    bindLocal(ki, state, ConstantExpr::alloc(Res.bitcastToAPInt()));
    break;
  }

  case Instruction::FDiv: {
    ref<ConstantExpr> left = toConstant(state, eval(ki, 0, state).value,
                                        "floating point");
    ref<ConstantExpr> right = toConstant(state, eval(ki, 1, state).value,
                                         "floating point");
    if (!fpWidthToSemantics(left->getWidth()) ||
        !fpWidthToSemantics(right->getWidth()))
      return terminateStateOnExecError(state, "Unsupported FDiv operation");

#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 3)
    llvm::APFloat Res(*fpWidthToSemantics(left->getWidth()), left->getAPValue());
    Res.divide(APFloat(*fpWidthToSemantics(right->getWidth()), right->getAPValue()), APFloat::rmNearestTiesToEven);
#else
    llvm::APFloat Res(left->getAPValue());
    Res.divide(APFloat(right->getAPValue()), APFloat::rmNearestTiesToEven);
#endif
    bindLocal(ki, state, ConstantExpr::alloc(Res.bitcastToAPInt()));
    break;
  }

  case Instruction::FRem: {
    ref<ConstantExpr> left = toConstant(state, eval(ki, 0, state).value,
                                        "floating point");
    ref<ConstantExpr> right = toConstant(state, eval(ki, 1, state).value,
                                         "floating point");
    if (!fpWidthToSemantics(left->getWidth()) ||
        !fpWidthToSemantics(right->getWidth()))
      return terminateStateOnExecError(state, "Unsupported FRem operation");
#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 3)
    llvm::APFloat Res(*fpWidthToSemantics(left->getWidth()), left->getAPValue());
    Res.mod(APFloat(*fpWidthToSemantics(right->getWidth()),right->getAPValue()),
            APFloat::rmNearestTiesToEven);
#else
    llvm::APFloat Res(left->getAPValue());
    Res.mod(APFloat(right->getAPValue()), APFloat::rmNearestTiesToEven);
#endif
    bindLocal(ki, state, ConstantExpr::alloc(Res.bitcastToAPInt()));
    break;
  }

  case Instruction::FPTrunc: {
    FPTruncInst *fi = cast<FPTruncInst>(i);
    Expr::Width resultType = getWidthForLLVMType(fi->getType());
    ref<ConstantExpr> arg = toConstant(state, eval(ki, 0, state).value,
                                       "floating point");
    if (!fpWidthToSemantics(arg->getWidth()) || resultType > arg->getWidth())
      return terminateStateOnExecError(state, "Unsupported FPTrunc operation");

#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 3)
    llvm::APFloat Res(*fpWidthToSemantics(arg->getWidth()), arg->getAPValue());
#else
    llvm::APFloat Res(arg->getAPValue());
#endif
    bool losesInfo = false;
    Res.convert(*fpWidthToSemantics(resultType),
                llvm::APFloat::rmNearestTiesToEven,
                &losesInfo);
    bindLocal(ki, state, ConstantExpr::alloc(Res));
    break;
  }

  case Instruction::FPExt: {
    FPExtInst *fi = cast<FPExtInst>(i);
    Expr::Width resultType = getWidthForLLVMType(fi->getType());
    ref<ConstantExpr> arg = toConstant(state, eval(ki, 0, state).value,
                                        "floating point");
    if (!fpWidthToSemantics(arg->getWidth()) || arg->getWidth() > resultType)
      return terminateStateOnExecError(state, "Unsupported FPExt operation");
#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 3)
    llvm::APFloat Res(*fpWidthToSemantics(arg->getWidth()), arg->getAPValue());
#else
    llvm::APFloat Res(arg->getAPValue());
#endif
    bool losesInfo = false;
    Res.convert(*fpWidthToSemantics(resultType),
                llvm::APFloat::rmNearestTiesToEven,
                &losesInfo);
    bindLocal(ki, state, ConstantExpr::alloc(Res));
    break;
  }

  case Instruction::FPToUI: {
    FPToUIInst *fi = cast<FPToUIInst>(i);
    Expr::Width resultType = getWidthForLLVMType(fi->getType());
    ref<ConstantExpr> arg = toConstant(state, eval(ki, 0, state).value,
                                       "floating point");
    if (!fpWidthToSemantics(arg->getWidth()) || resultType > 64)
      return terminateStateOnExecError(state, "Unsupported FPToUI operation");

#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 3)
    llvm::APFloat Arg(*fpWidthToSemantics(arg->getWidth()), arg->getAPValue());
#else
    llvm::APFloat Arg(arg->getAPValue());
#endif
    uint64_t value = 0;
    bool isExact = true;
    Arg.convertToInteger(&value, resultType, false,
                         llvm::APFloat::rmTowardZero, &isExact);
    bindLocal(ki, state, ConstantExpr::alloc(value, resultType));
    break;
  }

  case Instruction::FPToSI: {
    FPToSIInst *fi = cast<FPToSIInst>(i);
    Expr::Width resultType = getWidthForLLVMType(fi->getType());
    ref<ConstantExpr> arg = toConstant(state, eval(ki, 0, state).value,
                                       "floating point");
    if (!fpWidthToSemantics(arg->getWidth()) || resultType > 64)
      return terminateStateOnExecError(state, "Unsupported FPToSI operation");
#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 3)
    llvm::APFloat Arg(*fpWidthToSemantics(arg->getWidth()), arg->getAPValue());
#else
    llvm::APFloat Arg(arg->getAPValue());

#endif
    uint64_t value = 0;
    bool isExact = true;
    Arg.convertToInteger(&value, resultType, true,
                         llvm::APFloat::rmTowardZero, &isExact);
    bindLocal(ki, state, ConstantExpr::alloc(value, resultType));
    break;
  }

  case Instruction::UIToFP: {
    UIToFPInst *fi = cast<UIToFPInst>(i);
    Expr::Width resultType = getWidthForLLVMType(fi->getType());
    ref<ConstantExpr> arg = toConstant(state, eval(ki, 0, state).value,
                                       "floating point");
    const llvm::fltSemantics *semantics = fpWidthToSemantics(resultType);
    if (!semantics)
      return terminateStateOnExecError(state, "Unsupported UIToFP operation");
    llvm::APFloat f(*semantics, 0);
    f.convertFromAPInt(arg->getAPValue(), false,
                       llvm::APFloat::rmNearestTiesToEven);

    bindLocal(ki, state, ConstantExpr::alloc(f));
    break;
  }

  case Instruction::SIToFP: {
    SIToFPInst *fi = cast<SIToFPInst>(i);
    Expr::Width resultType = getWidthForLLVMType(fi->getType());
    ref<ConstantExpr> arg = toConstant(state, eval(ki, 0, state).value,
                                       "floating point");
    const llvm::fltSemantics *semantics = fpWidthToSemantics(resultType);
    if (!semantics)
      return terminateStateOnExecError(state, "Unsupported SIToFP operation");
    llvm::APFloat f(*semantics, 0);
    f.convertFromAPInt(arg->getAPValue(), true,
                       llvm::APFloat::rmNearestTiesToEven);

    bindLocal(ki, state, ConstantExpr::alloc(f));
    break;
  }

  case Instruction::FCmp: {
    FCmpInst *fi = cast<FCmpInst>(i);
    ref<ConstantExpr> left = toConstant(state, eval(ki, 0, state).value,
                                        "floating point");
    ref<ConstantExpr> right = toConstant(state, eval(ki, 1, state).value,
                                         "floating point");
    if (!fpWidthToSemantics(left->getWidth()) ||
        !fpWidthToSemantics(right->getWidth()))
      return terminateStateOnExecError(state, "Unsupported FCmp operation");

#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 3)
    APFloat LHS(*fpWidthToSemantics(left->getWidth()),left->getAPValue());
    APFloat RHS(*fpWidthToSemantics(right->getWidth()),right->getAPValue());
#else
    APFloat LHS(left->getAPValue());
    APFloat RHS(right->getAPValue());
#endif
    APFloat::cmpResult CmpRes = LHS.compare(RHS);

    bool Result = false;
    switch( fi->getPredicate() ) {
      // Predicates which only care about whether or not the operands are NaNs.
    case FCmpInst::FCMP_ORD:
      Result = CmpRes != APFloat::cmpUnordered;
      break;

    case FCmpInst::FCMP_UNO:
      Result = CmpRes == APFloat::cmpUnordered;
      break;

      // Ordered comparisons return false if either operand is NaN.  Unordered
      // comparisons return true if either operand is NaN.
    case FCmpInst::FCMP_UEQ:
      if (CmpRes == APFloat::cmpUnordered) {
        Result = true;
        break;
      }
    case FCmpInst::FCMP_OEQ:
      Result = CmpRes == APFloat::cmpEqual;
      break;

    case FCmpInst::FCMP_UGT:
      if (CmpRes == APFloat::cmpUnordered) {
        Result = true;
        break;
      }
    case FCmpInst::FCMP_OGT:
      Result = CmpRes == APFloat::cmpGreaterThan;
      break;

    case FCmpInst::FCMP_UGE:
      if (CmpRes == APFloat::cmpUnordered) {
        Result = true;
        break;
      }
    case FCmpInst::FCMP_OGE:
      Result = CmpRes == APFloat::cmpGreaterThan || CmpRes == APFloat::cmpEqual;
      break;

    case FCmpInst::FCMP_ULT:
      if (CmpRes == APFloat::cmpUnordered) {
        Result = true;
        break;
      }
    case FCmpInst::FCMP_OLT:
      Result = CmpRes == APFloat::cmpLessThan;
      break;

    case FCmpInst::FCMP_ULE:
      if (CmpRes == APFloat::cmpUnordered) {
        Result = true;
        break;
      }
    case FCmpInst::FCMP_OLE:
      Result = CmpRes == APFloat::cmpLessThan || CmpRes == APFloat::cmpEqual;
      break;

    case FCmpInst::FCMP_UNE:
      Result = CmpRes == APFloat::cmpUnordered || CmpRes != APFloat::cmpEqual;
      break;
    case FCmpInst::FCMP_ONE:
      Result = CmpRes != APFloat::cmpUnordered && CmpRes != APFloat::cmpEqual;
      break;

    default:
      assert(0 && "Invalid FCMP predicate!");
    case FCmpInst::FCMP_FALSE:
      Result = false;
      break;
    case FCmpInst::FCMP_TRUE:
      Result = true;
      break;
    }

    bindLocal(ki, state, ConstantExpr::alloc(Result, Expr::Bool));
    break;
  }
  case Instruction::InsertValue: {
    KGEPInstruction *kgepi = static_cast<KGEPInstruction*>(ki);

    ref<Expr> agg = eval(ki, 0, state).value;
    ref<Expr> val = eval(ki, 1, state).value;

    ref<Expr> l = NULL, r = NULL;
    unsigned lOffset = kgepi->offset*8, rOffset = kgepi->offset*8 + val->getWidth();

    if (lOffset > 0)
      l = ExtractExpr::create(agg, 0, lOffset);
    if (rOffset < agg->getWidth())
      r = ExtractExpr::create(agg, rOffset, agg->getWidth() - rOffset);

    ref<Expr> result;
    if (!l.isNull() && !r.isNull())
      result = ConcatExpr::create(r, ConcatExpr::create(val, l));
    else if (!l.isNull())
      result = ConcatExpr::create(val, l);
    else if (!r.isNull())
      result = ConcatExpr::create(r, val);
    else
      result = val;

    bindLocal(ki, state, result);
    break;
  }
  case Instruction::ExtractValue: {
    KGEPInstruction *kgepi = static_cast<KGEPInstruction*>(ki);

    ref<Expr> agg = eval(ki, 0, state).value;

    ref<Expr> result = ExtractExpr::create(agg, kgepi->offset*8, getWidthForLLVMType(i->getType()));

    bindLocal(ki, state, result);
    break;
  }
 
    // Other instructions...
    // Unhandled
  case Instruction::ExtractElement:
  case Instruction::InsertElement:
  case Instruction::ShuffleVector:
    terminateStateOnError(state, "XXX vector instructions unhandled",
                          "xxx.err");
    break;
 
  default:
    terminateStateOnExecError(state, "illegal instruction");
    break;
  }
}

/*-------------------------------------------------
 * klee/lib/Core/StatsTracker.cpp:274
  -------------------------------------------------*/

void StatsTracker::stepInstruction(ExecutionState &es)
{
	if (OutputIStats) {
		if (TrackInstructionTime) {...}
		Instruction *inst = es.pc->inst;
		const InstructionInfo &ii = *es.pc->info;
		StackFrame &sf = es.stack.back();
		theStatisticManager->setIndex(ii.id);
		if (UseCallPaths)
			theStatisticManager->setContext(&sf.callPathNode->statistics);
		if (es.instsSinceCovNew) ...
		if (sf.kf->trackCoverage && instructionIsCoverable(inst)) {
			if (!theStatisticManager->getIndexedValue(stats::coveredInstructions, ii.id)) {
				// Checking for actual stoppoints avoids inconsistencies due to line number propogation.
				// FIXME: This trick no longer works, we should fix this in the line number propogation.
				es.coveredLines[&ii.file].insert(ii.line);
				es.coveredNew = true;
				es.instsSinceCovNew = 1;
				++stats::coveredInstructions;
				stats::uncoveredInstructions += (uint64_t)-1;
			}
		}
	}
}

//上面92行调用
KleeHandler::KleeHandler(int argc, char **argv) : m_interpreter(0), m_pathWriter(0), m_symPathWriter(0), m_infoFile(0), m_outputDirectory(), m_testIndex(0), m_pathsExplored(0), m_argc(argc), m_argv(argv) {
	// create output directory (OutputDir or "klee-out-<i>")
	bool dir_given = OutputDir != "";	// false
	SmallString<128> directory(dir_given ? OutputDir : InputFile);
	if (!dir_given) sys::path::remove_filename(directory);	//移除最后路径的最后一个文件到达目录
#if LLVM_VERSION_CODE < LLVM_VERSION(3, 5)
	error_code ec;
	if ((ec = sys::fs::make_absolute(directory)) != errc::success) {
#else
	...
#endif
		...
	}
	if (dir_given) {...}
	else {
		// "klee-out-<i>"
		int i = 0;
		for (; i <= INT_MAX; ++i) {
			SmallString<128> d(directory);
			llvm::sys::path::append(d, "klee-out-");
			raw_svector_ostream ds(d); ds << i; ds.flush();
			// create directory and try to link klee-last
			if (mkdir(d.c_str(), 0775) == 0) {
			//创建目录klee-out-i与Hello.bc在同目录下
				m_outputDirectory = d;
				// d表示目录klee-out-i
				SmallString<128> klee_last(directory);
				llvm::sys::path::append(klee_last, "klee-last");
				if (((unlink(klee_last.c_str()) < 0) && (errno != ENOENT)) ||
				symlink(m_outputDirectory.c_str(), klee_last.c_str()) < 0) {...}
				//创建符号链接klee-last
				break;
			}
			...
		}
		if (i == INT_MAX && m_outputDirectory.str().equals(""))
			...
	}
	klee_message("output directory is \"%s\"", m_outputDirectory.c_str());
	//输出：KLEE: output directory is "/home/user/Documents/test/test_LLVM_Pass/klee-out-0"
	// open warnings.txt(创建文件warning.txt)
	std::string file_path = getOutputFilename("warnings.txt");
	// file_path = "/home/user/Documents/test/LLVM_Pass_test/klee-out-0/warnings.txt"
	if ((klee_warning_file = fopen(file_path.c_str(), "w")) == NULL) ...
	// open messages.txt(创建文件messages.txt)
	file_path = getOutputFilename("messages.txt");
	if ((klee_message_file = fopen(file_path.c_str(), "w")) == NULL) ...
	// open info(创建文件info)
	m_infoFile = openOutputFile("info");
}

//上面94行调用
// klee/lib/Core/Executor.cpp:3565
Interpreter *Interpreter::create(const InterpreterOptions &opts, InterpreterHandler *ih) {
	return new Executor(opts, ih);
}

Executor::Executor(const InterpreterOptions &opts, InterpreterHandler *ih) : Interpreter(opts), kmodule(0), interpreterHandler(ih), searcher(0), externalDispatcher(new ExternalDispatcher()), statsTracker(0), pathWriter(0), symPathWriter(0), specialFunctionHandler(0), processTree(0), replayOut(0), replayPath(0), usingSeeds(0), atMemoryLimit(false), inhibitForking(false), haltExecution(false), ivcEnabled(false), coreSolverTimeout(MaxCoreSolverTime != 0 && MaxInstructionTime != 0 ? std::min(MaxCoreSolverTime,MaxInstructionTime) : std::max(MaxCoreSolverTime,MaxInstructionTime)) {
	
	if (coreSolverTimeout) ...
	Solver *coreSolver = NULL;
#ifdef SUPPORT_METASMT
	...
#else
  coreSolver = new STPSolver(UseForkedCoreSolver, CoreSolverOptimizeDivides);
#endif /* SUPPORT_METASMT */
	Solver *solver = constructSolverChain(coreSolver, interpreterHandler->getOutputFilename(ALL_QUERIES_SMT2_FILE_NAME), interpreterHandler->getOutputFilename(SOLVER_QUERIES_SMT2_FILE_NAME), interpreterHandler->getOutputFilename(ALL_QUERIES_PC_FILE_NAME), interpreterHandler->getOutputFilename(SOLVER_QUERIES_PC_FILE_NAME));

	this->solver = new TimingSolver(solver, EqualitySubstitution);
	
	memory = new MemoryManager();
}