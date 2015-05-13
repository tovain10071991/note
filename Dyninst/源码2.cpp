BPatch bpatch;
//------------------------------------------------
BPatch::BPatch()
  : info(NULL),
    typeCheckOn(true),
    lastError(0),
    debugParseOn(true),
    baseTrampDeletionOn(false),
    trampRecursiveOn(false),
    forceRelocation_NP(false),
    autoRelocation_NP(true),
    saveFloatingPointsOn(true),
    forceSaveFloatingPointsOn(false),
    livenessAnalysisOn_(true),
    livenessAnalysisDepth_(3),
    asyncActive(false),
    delayedParsing_(false),
    instrFrames(false),
    systemPrelinkCommand(NULL),
    notificationFDOutput_(-1),
    notificationFDInput_(-1),
    FDneedsPolling_(false),
    errorCallback(NULL),
    preForkCallback(NULL),
    postForkCallback(NULL),
    execCallback(NULL),
    exitCallback(NULL),
    oneTimeCodeCallback(NULL),
    dynLibraryCallback(NULL),
    threadCreateCallback(NULL),
    threadDestroyCallback(NULL),
    dynamicCallSiteCallback(NULL),
    signalHandlerCallback(NULL),
    codeOverwriteCallback(NULL),
    builtInTypes(NULL),
    stdTypes(NULL),
    type_Error(NULL),
    type_Untyped(NULL)
{
    init_debug();	//初始化调试，第一次什么也没做
    init_stats();	//初始化running_time和stats_instru(跟主线似乎没关系)
    memset(&stats, 0, sizeof(BPatch_stats));	//初始化this.stats
    extern bool init();	//不知道在哪

    //this.bpatch是指向自己的指针
    if (bpatch == NULL){
       bpatch = this;
    }
    
    BPatch::bpatch->registerErrorCallback(defaultErrorFunc);
    bpinfo("installed default error reporting function");
    
    /*
     * Create the list of processes.
     */
    info = new BPatch_libInfo();

    /*
     * Create the "error" and "untyped" types.
     */
    type_Error   = BPatch_type::createFake("<error>");
    type_Untyped = BPatch_type::createFake("<no type>");
    
    /*
     * Initialize hash table of API types.
     */
    APITypes = BPatch_typeCollection::getGlobalTypeCollection();

    stdTypes = BPatch_typeCollection::getGlobalTypeCollection();
    vector<Type *> *sTypes = Symtab::getAllstdTypes();
    for(unsigned i=0; i< sTypes->size(); i++)
        stdTypes->addType(new BPatch_type((*sTypes)[i]));

    builtInTypes = new BPatch_builtInTypeCollection;
    sTypes = Symtab::getAllbuiltInTypes();
    for(unsigned i=0; i< sTypes->size(); i++)
        builtInTypes->addBuiltInType(new BPatch_type((*sTypes)[i]));

    //loadNativeDemangler();

	// Start up the event handler thread
	PCEventMuxer::start();
}
//------------------------------------------------
BPatch_process* appProc = bpatch.processCreate("../test/main", NULL);
//------------------------------------------------
BPatch_process *BPatch::processCreateInt(const char *path, const char *argv[], const char **envp, int stdin_fd, int stdout_fd, int stderr_fd, BPatch_hybridMode mode)
{
   clearError();

   // User request: work on scripts by creating the interpreter instead
   char *pathToUse = NULL;
   char **argvToUse = NULL;
   buildPath(path, argv, pathToUse, argvToUse);
   BPatch_process *ret = 
      new BPatch_process((pathToUse ? pathToUse : path), 
                         (argvToUse ? (const_cast<const char **>(argvToUse)) : argv), 
                         mode, envp, stdin_fd,stdout_fd,stderr_fd);
   
   if (pathToUse) free(pathToUse);
   if (argvToUse) {

      int tmp = 0;
      while(argvToUse[tmp] != NULL) {
         free(argvToUse[tmp]);
         tmp++;
      }
      free(argvToUse);
   }

   if (!ret->llproc 
         ||  !ret->llproc->isStopped()
         ||  !ret->llproc->isBootstrapped()) {
      delete ret;
      reportError(BPatchFatal, 68, "create process failed bootstrap");
      return NULL;
   }

   ret->triggerInitialThreadEvents();

   if (ret->lowlevel_process()->isExploratoryModeOn()) {
       ret->getHybridAnalysis()->init();
   }

   return ret;
}
//------------------------------------------------
BPatch_process::BPatch_process(const char *path, const char *argv[], BPatch_hybridMode mode, const char **envp, int stdin_fd, int stdout_fd, int stderr_fd)
   : llproc(NULL), lastSignal(-1), exitCode(-1), 
     exitedNormally(false), exitedViaSignal(false), mutationsActive(true), 
     createdViaAttach(false), detached(false), 
     terminated(false), reportedExit(false),
     hybridAnalysis_(NULL)
{
   image = NULL;
   pendingInsertions = NULL;

   pdvector<std::string> argv_vec;
   pdvector<std::string> envp_vec;
   // Contruct a vector out of the contents of argv
   if (argv) {
      for(int i = 0; argv[i] != NULL; i++)
         argv_vec.push_back(argv[i]);
   }

   std::string directoryName = "";

 #if !defined(os_windows)
   // this fixes a problem on linux and alpha platforms where pathless
   // filenames are searched for either in a standard, predefined path, or
   // in $PATH by execvp.  thus paths that should resolve to "./" are
   // missed.  Note that the previous use of getcwd() here for the alpha
   // platform was dangerous since this is an API and we don't know where
   // the user's code will leave the cwd pointing.

   if (NULL == strchr(path, '/')) {
      const char *pathenv = getenv("PATH");
      char *pathenv_copy = strdup(pathenv);
      char *ptrptr;
      char *nextpath = strtok_r(pathenv_copy, ":", &ptrptr);
      while (nextpath) {
         struct stat statbuf;

         char *fullpath = new char[strlen(nextpath)+strlen(path)+2];
         strcpy(fullpath,nextpath);
         strcat(fullpath,"/");
         strcat(fullpath,path);

         if (!stat(fullpath,&statbuf)) {
            directoryName = nextpath;
            delete[] fullpath;
            break;
         }
         delete[] fullpath;
         nextpath = strtok_r(NULL,":", &ptrptr);
      }
      ::free(pathenv_copy);

      if (nextpath == NULL) {
         const char *dotslash = "./";
         directoryName = dotslash;
      }
   }
#endif

   std::string spath(path);
   llproc = PCProcess::createProcess(spath, argv_vec, mode, envp_vec,
                                     directoryName, 
                                     stdin_fd, stdout_fd, stderr_fd);
   if (llproc == NULL) {
      BPatch_reportError(BPatchFatal, 68,
           "Dyninst was unable to create the specified process");
      return;
   }

   startup_cerr << "Registering function callback..." << endl;
   llproc->registerFunctionCallback(createBPFuncCB);


   startup_cerr << "Registering instPoint callback..." << endl;
   llproc->registerInstPointCallback(createBPPointCB);
   llproc->set_up_ptr(this);

   assert(BPatch::bpatch != NULL);
   startup_cerr << "Registering process..." << endl;
   BPatch::bpatch->registerProcess(this);

   // Create an initial thread
   startup_cerr << "Getting initial thread..." << endl;
   PCThread *thr = llproc->getInitialThread();
   BPatch_thread *initial_thread = new BPatch_thread(this, thr);
   threads.push_back(initial_thread);

   startup_cerr << "Creating new BPatch_image..." << endl;
   image = new BPatch_image(this);

   assert(llproc->isBootstrapped());

   assert(BPatch_heuristicMode != llproc->getHybridMode());
   if ( BPatch_normalMode != mode ) {
       BPatch::bpatch->setInstrStackFrames(true);
       hybridAnalysis_ = new HybridAnalysis(llproc->getHybridMode(),this);
   }

   // Let's try to profile memory usage
#if defined(PROFILE_MEM_USAGE)
   void *mem_usage = sbrk(0);
   fprintf(stderr, "Post BPatch_process: sbrk %p\n", mem_usage);
#endif

   startup_cerr << "BPatch_process::BPatch_process, completed." << endl;
}