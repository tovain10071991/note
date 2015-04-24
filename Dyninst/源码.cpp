/**************************************************
 * dyninst-8.0\dyninstAPI\h\BPatch_eventLock.h
 **************************************************/

// 所有类的父类，提供锁机制，全部会被派生类overrade
class BPATCH_DLL_EXPORT BPatch_eventLock {
public:
  static eventLock *getLock();
  unsigned long threadID() const;	// 获取线程id
  int _Lock(const char *__file__, unsigned int __line__) const; 
  int _Trylock(const char *__file__, unsigned int __line__) const; 
  int _Unlock(const char *__file__, unsigned int __line__) const; 
  int _Broadcast(const char *__file__, unsigned int __line__) const;
  int _WaitForSignal(const char *__file__, unsigned int __line__) const;
  unsigned int lockDepth() const;
};

/**************************************************
 * dyninst-8.0\dyninstAPI\h\BPatch.h
 **************************************************/

class BPATCH_DLL_EXPORT BPatch : public BPatch_eventLock {
public:
    static BPatch			*bpatch;
	static BPatch			*getBPatch();		//返回此BPatch句柄(就是上面这个)
    BPatch_builtInTypeCollection	*builtInTypes;
    BPatch_typeCollection	*stdTypes;
    BPatch_typeCollection	*APITypes;	//API/User defined types
    BPatch_type			 	*type_Error;
    BPatch_type			 	*type_Untyped;
public:
    BPatch();
    //  BPatch::~BPatch
    //  destructor
    ~BPatch(){BPatch_dtor();}

    static const char *getEnglishErrorString(int number);
    static void formatErrorString(char *dst, int size, const char *fmt, const char * const *params);
    // BPatch::isTypeChecked:
    // 类型检查
    bool isTypeChecked();
    // BPatch::parseDebugInfo:
    // 调试信息解析
	bool parseDebugInfo();
    // BPatch::baseTrampDeletion:
    // 删除base tramp
	bool baseTrampDeletion();
	
    // BPatch::setPrelinkCommand
    // sets the fully qualified path name of the prelink command
	void setPrelinkCommand(char *command);
    // BPatch::getPrelinkCommand
    // gets the fully qualified path name of the prelink command
	bool getPrelinkCommand();
    // BPatch::isTrampRecursive:
    // 蹦床代码递归
	bool isTrampRecursive();
    // BPatch::isMergeTramp:
    // base tramp和mini tramp合并
	bool isMergeTramp();   
    // BPatch::saveFPROn:
    // 保存FP
	bool isSaveFPROn();
    // BPatch::forceSaveFPROn:
    // 强制保存FP
    bool isForceSaveFPROn();        
    // BPatch::hasForcedRelocation_NP:
    // 重定位所有插桩函数
    bool hasForcedRelocation_NP();
    // BPatch::autoRelocationsOn:
    // 恰当时重定位插桩函数
    bool autoRelocationOn();
    // BPatch::delayedParsingOn:
    // 延迟解析
    bool delayedParsingOn();
	// Liveness...
    bool livenessAnalysisOn();
	int livenessAnalysisDepth();

    //  注册用户定义的回调函数

    //  BPatch::registerErrorCallback:
    //  错误处理函数
    BPatchErrorCallback registerErrorCallback(BPatchErrorCallback function);
    //  BPatch::registerDynLibraryCallback:
    //  Register callback for new library events (eg. load)
    BPatchDynLibraryCallback registerDynLibraryCallback(BPatchDynLibraryCallback func);
    //  BPatch::registerPostForkCallback:
    //  mutateef调用fork后执行的回调函数
    BPatchForkCallback registerPostForkCallback(BPatchForkCallback func);
    //  BPatch::registerPreForkCallback:
    //  mutateef调用fork前执行的回调函数
    BPatchForkCallback registerPreForkCallback(BPatchForkCallback func));
    //  BPatch::registerExecCallback:
    //  Register callback to handle mutatee exec events 
    BPatchExecCallback registerExecCallback(BPatchExecCallback func);
    //  BPatch::registerExitCallback:
    //  mutatee退出时调用
    BPatchExitCallback registerExitCallback(BPatchExitCallback func);
    //  BPatch::registerOneTimeCodeCallback:
    //  oneTimeCode完成时调用 
    BPatchOneTimeCodeCallback registerOneTimeCodeCallback(BPatchOneTimeCodeCallback func);
    //  BPatch::registerThreadEventCallback
    //	线程被创建时调用
    bool registerThreadEventCallback(BPatch_asyncEventType type, BPatchAsyncThreadEventCallback cb));
    //  BPatch::removeThreadEventCallback
    //  线程销毁时调用
    bool removeThreadEventCallback(BPatch_asyncEventType type, BPatchAsyncThreadEventCallback cb));
    //  BPatch::registerDynamicCallCallback
    //  Specifies a user-supplied function to be called when a dynamic call is executed.
    bool registerDynamicCallCallback(BPatchDynamicCallSiteCallback cb);

    bool removeDynamicCallCallback(BPatchDynamicCallSiteCallback cb);

    //  BPatch::registerUserEventCallback
    //  
    //  Specifies a user defined function to call when a "user event" occurs, user events are trigger by calls to the function DYNINSTuserMessage(void *, int) in the runtime library.
    //  
    //  BPatchUserEventCallback is:
    //  void (*BPatchUserEventCallback)(void *msg, unsigned int msg_size);

    bool registerUserEventCallback(BPatchUserEventCallback cb); 

    bool removeUserEventCallback(BPatchUserEventCallback cb);

    // BPatch::registerSignalHandlerCallback 
    // 
    // If the mutator produces a signal matching an element of
    // signal_numbers, the callback is invoked, returning the point
    // that caused the exception, the signal number, and a Vector
    // representing the address of signal handler(s) in the mutatee
    // for the exception.  In Windows this is the handler stack, each
    // function of which is invoked until one is found that agrees to
    // handle the exception.  In Unix there will be at most one
    // handler for the signal number, the handler registered with
    // syscalls signal() or sigaction(), or the default system
    // handler, in which case we return an empty vector.
	//信号处理函数
	bool registerSignalHandlerCallback(BPatchSignalHandlerCallback cb, BPatch_Set<long>*signal_numbers)); 

    bool removeSignalHandlerCallback(BPatchSignalHandlerCallback cb); 

    bool registerCodeDiscoveryCallback(BPatchCodeDiscoveryCallback cb);

    bool removeCodeDiscoveryCallback(BPatchCodeDiscoveryCallback cb);

    // BPatch::registerCodeOverwriteCallbacks
    // 
    // 代码重写时调用
    bool registerCodeOverwriteCallbacks(BPatchCodeOverwriteBeginCallback cbBegin, BPatchCodeOverwriteEndCallback cbEnd));

    //  BPatch::getProcesses:
    //  获取当前被测进程的列表 
    BPatch_Vector<BPatch_process*>* getProcesses();

    //
    //  General BPatch parameter settings:
    //
    
    //  BPatch::setDebugParsing:
    //  Turn on/off parsing of debug section(s)
    void setDebugParsing(bool x);

    //  BPatch::setBaseTrampDeletion:
    //  Turn on/off deletion of base tramp
    void setBaseTrampDeletion(bool x);

    //  BPatch::setTypeChecking:
    //  Turn on/off type checking
    void setTypeChecking(bool x);

    void setInstrStackFrames(bool b);

    bool getInstrStackFrames,();

    //  BPatch::setTypeChecking:
    //  Turn on/off line info truncating
    void truncateLineInfoFilenames(bool x);

    //  BPatch::setTrampRecursive:
    //  Turn on/off recursive trampolines
    void setTrampRecursive(bool x);

    //  BPatch::setMergeTramp:
    //  Turn on/off merged base & mini-tramps
    void setMergeTramp(bool x);

    //  BPatch::setSaveFPR:
    //  Turn on/off merged base & mini-tramps
    void setSaveFPR,(bool x);

    //  BPatch::forceSaveFPR:
    //  Force Turn on/off merged base & mini-tramps - ignores isConservative
    void forceSaveFPR(bool x);


    //  BPatch::setForcedRelocation_NP:
    //  Turn on/off forced relocation of instrumted functions
    void setForcedRelocation_NP(bool x);

    //  BPatch::setAutoRelocation_NP:
    //  Turn on/off function relocations, performed when necessary
    void setAutoRelocation_NP(bool x);

    //  BPatch::setDelayedParsing:
    //  Turn on/off delayed parsing
    void setDelayedParsing(bool x);

    // Liveness...
    void setLivenessAnalysis(bool x);

    void setLivenessAnalysisDepth(int x);

    // BPatch::processCreate:
    // Create a new mutatee process
    BPatch_process* processCreate(const char *path,
									const char *argv[],
									const char **envp = NULL,
									int stdin_fd=0,
									int stdout_fd=1,
									int stderr_fd=2,
									BPatch_hybridMode mode=BPatch_normalMode);

    // BPatch::processAttach
    // Attach to mutatee process
    BPatch_process* processAttach(const char *path, int pid, BPatch_hybridMode mode=BPatch_normalMode);

    // BPatch::openBinary
    // Open a binary for static instrumentation
    //
    // The second parameter really should be a boolean, but the value
    // gets reset between the openBinary and openBinaryInt calls--is
    // this a gcc bug???
    // 
    BPatch_binaryEdit* openBinary(const char *path, bool openDependencies = false);

    // BPatch::createEnum:
    // Create Enum types. 
    BPatch_type* createEnum(const char* name, BPatch_Vector<char*> &elementNames, BPatch_Vector<int> &elementIds);

    // BPatch::createEnum:
    // API selects elementIds
    BPatch_type* createEnum(const char * name, BPatch_Vector<char *> &elementNames);

    // BPatch::createStruct:
    // Create Struct types. 
    BPatch_type* createStruct(const char * name, BPatch_Vector<char *> &fieldNames, BPatch_Vector<BPatch_type *> &fieldTypes);

    // BPatch::createUnion:
    // Create Union types. 
    BPatch_type* createUnion(const char* name, BPatch_Vector<char*> &fieldNames, BPatch_Vector<BPatch_type *> &fieldTypes);

    // BPatch::createArray:
    // Creates BPatch_array type or symtyperanges ( scalars with upper and
    //lower bound).
    BPatch_type* createArray(const char* name, BPatch_type* ptr, unsigned int low, unsigned int hi);

    // BPatch::createPointer:
    // Creates BPatch_pointer types	 
    BPatch_type* createPointer(const char* name, BPatch_type* ptr, int size = sizeof(void*));

    // BPatch::createScalar:
    // Creates BPatch_scalar types
    BPatch_type* createScalar(const char* name, int size);
    
    // BPatch::createTypedef:
    // Creates typedefs.
    BPatch_type* createTypedef(const char* name, BPatch_type* ptr);
	 
    // User programs are required to call pollForStatusChange or
    // waitForStatusChange before user-level callback functions
    // are executed (for example, fork, exit, or a library load). 

    // Non-blocking form; returns immediately if no callback is
    // ready, or executes callback(s) then returns.
    bool pollForStatusChange();

    // Blocks until a callback is ready.
    bool waitForStatusChange();

    // For user programs that block on other things as well,
    // we provide a (simulated) file descriptor that can be added
    // to a poll or select fdset. When a callback is prepared the BPatch
    // layer writes to this fd, thus making poll/select return. The user
    // program should then call pollForStatusChange. The BPatch layer
    // will handle clearing the file descriptor; all the program must do 
    // is call pollForStatusChange or waitForStatusChange.
    int getNotificationFD();

    //  BPatch:: waitUntilStopped:
    //  Block until specified process has stopped.
    bool waitUntilStopped(BPatch_thread *appThread);

    //  BPatch::getBPatchStatistics:
    //  Get Instrumentation statistics
    BPatch_stats &getBPatchStatistics();

	void getBPatchVersion(int &major, int &minor, int &subminor);

    // These three should probably be moved into their own BPatch_* class.
    // Perhaps BPatch_remoteDebug?
    bool isConnected();

    bool remoteConnect(BPatch_remoteHost &remote);

    bool getPidList(BPatch_remoteHost &remote, BPatch_Vector<unsigned int> &pidlist);

    bool getPidInfo(BPatch_remoteHost &remote, unsigned int pid, std::string &pidStr);

    bool remoteDisconnect(BPatch_remoteHost &remote);

    //  BPatch::addNonReturningFunc:
    //  Globally specify that any function with a given name will not return
    void addNonReturningFunc(std::string name);
};