/*
 * SimpleExamples$ make obj-ia32/opcodemix.so DEBUG=1
 * SimpleExamples/obj-ia32$ ../../../../pin -pause_tool 20 -t opcodemix.so -- /bin/ls
 * SimpleExamples/obj-ia32$ gdb
 * (gdb) attach 3581
 * (gdb) add-symbol-file ./opcodemix.so 0xb6cd7df0 -s .data 0xb740a380 -s .bss 0xb740afa0
 * (gdb) b main
 * (gdb) c
 */


/*
 *	tools/SimpleExample/opcodemix.cpp
 */

int main(int argc, CHAR *argv[])
{
    PIN_InitSymbols();
	PIN_Init(argc,argv);
	//LOCALVAR CONTROL_MANAGER control;
    control.RegisterHandler(Handler, 0, FALSE);
    control.Activate();
    string filename =  KnobOutputFile.Value();
    out = new std::ofstream(filename.c_str());
    TRACE_AddInstrumentFunction(Trace, 0);
    PIN_AddFiniFunction(Fini, 0);
    PIN_StartProgram(); 
    return 0;
}

LOCALFUN VOID Handler(EVENT_TYPE ev, VOID *val, CONTEXT * ctxt, VOID *ip, THREADID tid, bool bcast)
{
    switch(ev)
    {
      case EVENT_START:
        enabled = 1;
        break;

      case EVENT_STOP:
        enabled = 0;
        break;

      default:
        ASSERTX(false);
    }
}

/*
 *	tools/InstLib/control_manager.cpp
 */

VOID CONTROL_MANAGER::RegisterHandler(CONTROL_HANDLER ch, VOID* val, BOOL passContext=FALSE) 
/*
 * 注册控制器处理函数
 * ch - 当事件发生时要调用的回调函数
 * val - 传递给回调函数的参数
 * passContext - 回调函数是否需要context
 */
{
    CONTROL_HANDLER_PARAMS params;
	/*
	 *	typedef struct {
	 *		CONTROL_HANDLER handler;
	 *		VOID* val;
	 *		bool  passContext;
	 *	} CONTROL_HANDLER_PARAMS;
	 */
    params.handler = ch;
    params.val = val;
    params.passContext = passContext;
    _control_handler.push_back(params);
	//list<CONTROL_HANDLER_PARAMS> _control_handler;
}

VOID CONTROL_MANAGER::Activate()
{   
    UINT32 num_chains = _control_knob->NumberOfValues();	//0

    if (_control_log_knob->Value()) ...

    //检查事件处理函数是否需要context，只要有一个函数需要就返回TRUE，否则返回FALSE
    _pass_context = ShouldPassContext();	//FALSE

    if (_control_interactive->Value() != "") ...

    //iterate all the chains and parse them
    for (UINT32 i=0; i<num_chains; i++) ...

    //add translation of old controller knobs
    AddOldKnobs();

    _iregions->Activate(_pass_context);
    
    //if non of the chains has start event we must add one
    if (_control_default_start->Value()){
        AddDefaultStart();
    }

    PIN_AddFiniFunction(Fini,this);
}

VOID CONTROL_MANAGER::AddOldKnobs(){
    // create a "chain" from old controller knobs
    // in case length knob is used we need to append it to the generated chain
    UINT32 added_length = 0;
    added_length += CreateOldOne(_control_skip->Value(),"start","icount",TRUE);
    added_length += CreateOld(_control_start_address,"start","address",TRUE);
    added_length += CreateOld(_control_stop_address,"stop","address");
    added_length += CreateOld(_control_start_ssc,"start","ssc",TRUE);
    added_length += CreateOld(_control_stop_ssc,"stop","ssc");
    added_length += CreateOld(_control_start_itext,"start","itext",TRUE);
    added_length += CreateOld(_control_stop_itext,"stop","itext");
    added_length += CreateOld(_control_start_int3,"start","int3",TRUE);
    added_length += CreateOld(_control_stop_int3,"stop","int3");
    added_length += CreateOld(_control_start_isa_ctg,"start","isa_category",
                              TRUE);
    added_length += CreateOld(_control_stop_isa_ctg,"stop","isa_category");
    added_length += CreateOld(_control_start_isa_ext,"start","isa_extension",
                              TRUE);
    added_length += CreateOld(_control_stop_isa_ext,"stop","isa_extension");
    //added_length = 0
	
    //length knob was used, but no start event was specified - so we did 
    //not add this stop event to any other chain.
    if (_control_length->Value() != "") ...
}

/*
 *	tools/InstLib/regions_control.H
 */

class CONTROL_IREGIONS
{
    INT32 Activate(BOOL passContext)
    {
        if (strcmp(_rFileKnob.Value().c_str(),"") == 0)
        {
            return 0;
        }
		...
    }
}

/*
 *	tools/InstLib/control_manager.cpp
 */

VOID CONTROL_MANAGER::AddDefaultStart(){   
    //no start event was defined.    
    if(HasStartEvent() == FALSE){
        _init_alarm.Activate(this);
    }
}

/*
 *	tools/InstLib/init_alarm.cpp
 */

VOID INIT_ALARM::Activate(CONTROL_MANAGER* manager){
    _manager = manager;
    memset(_start_thread,0,sizeof(_start_thread));

    PIN_CALLBACK thread_start = PIN_AddThreadStartFunction(ThreadStart, this);
    // other tools working with the controller might do some initialization in their 
    // thread start callback.
    // need to make sure we are been call AFTER all thread start callbacks were called.
	//设置回调函数优先级
    CALLBACK_SetExecutionPriority(thread_start, CALL_ORDER_LAST);
    TRACE_AddInstrumentFunction(OnTrace, this);
}

/*
 * 第一个被调用的回调函数
 * tools/InstLib/init_alarm.cpp
 */

VOID INIT_ALARM::OnTrace(TRACE trace, VOID *vthis){
    INIT_ALARM *me = static_cast<INIT_ALARM*>(vthis);

    INS ins = BBL_InsHead(TRACE_BblHead(trace));
    ADDRINT first_ip = INS_Address(ins); 
    
    if (me->_thread_first_ip.find(first_ip) == me->_thread_first_ip.end()){
        // the first instruction of the TARCE is not the first instruction of
        // any new created thread 
        return;
    }
    INS_InsertIfCall(
        ins, IPOINT_BEFORE, AFUNPTR(ShouldStart),
        IARG_CALL_ORDER, me->_manager->GetInsOrder(),
        IARG_ADDRINT, me,
        IARG_THREAD_ID,
        IARG_END);

    if (me->_manager->PassContext())
    {
        INS_InsertThenCall(
            ins, IPOINT_BEFORE, AFUNPTR(Start),
            IARG_CALL_ORDER, me->_manager->GetInsOrder(),
            IARG_CONTEXT, 
            IARG_INST_PTR,
            IARG_THREAD_ID, 
            IARG_ADDRINT, me, 
            IARG_END);
    }
    else
    {
        INS_InsertThenCall(
            ins, IPOINT_BEFORE, AFUNPTR(Start),
            IARG_CALL_ORDER, me->_manager->GetInsOrder(),
            IARG_ADDRINT, static_cast<ADDRINT>(0),
            IARG_INST_PTR,
            IARG_THREAD_ID, 
            IARG_ADDRINT, me,
            IARG_END);
    }
}

/*
 * 第二个被调用的回调函数
 * tools/SimpleExample/opcodemix.cpp
 */

VOID Trace(TRACE trace, VOID *v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        const INS head = BBL_InsHead(bbl);
        if (! INS_Valid(head)) continue;

        const UINT32 n = IndexStringLength(bbl, 1);
		//基本块中的指令操作和访存总数

        UINT16 *const stats = new UINT16[ n + 1];
        UINT16 *const stats_end = stats + (n + 1);
        UINT16 *curr = stats;
		/*
		 *	class STATS
		 *	{
		 *	public:
		 *		COUNTER unpredicated[MAX_INDEX];
		 *		COUNTER predicated[MAX_INDEX];
		 *		COUNTER predicated_true[MAX_INDEX];
		 *		VOID Clear()
		 *		{
		 *			for ( UINT32 i = 0; i < MAX_INDEX; i++)
		 *			{
		 *				unpredicated[i] = 0;
		 *				predicated[i] = 0;
		 *				predicated_true[i] = 0;
		 *			}
		 *		}
		 *	};
		 */

        for (INS ins = head; INS_Valid(ins); ins = INS_Next(ins))
        {
            curr = INS_GenerateIndexString(ins,curr,1);
			//收集指令的操作类型和访存
        }
        
        *curr++ = 0;
        ASSERTX( curr == stats_end );

        BBLSTATS * bblstats = new BBLSTATS(stats);
		/*
		class BBLSTATS
		{
		public:
			COUNTER _counter;
			const UINT16 * const _stats;
		public:
			BBLSTATS(UINT16 * stats) : _counter(0), _stats(stats) {};
		};
		*/
        INS_InsertCall(head, IPOINT_BEFORE, AFUNPTR(docount), IARG_FAST_ANALYSIS_CALL, IARG_PTR, &(bblstats->_counter), IARG_END);

        statsList.push_back(bblstats);
		//LOCALVAR vector<const BBLSTATS*> statsList;
    }
}

VOID Fini(int, VOID * v)
{

    // static counts

    DumpStats(*out, GlobalStatsStatic, false, "$static-counts");
	//STATS GlobalStatsStatic;
    
    *out << endl;

    // dynamic Counts 

    statsList.push_back(0); // add terminator marker

    for (vector<const BBLSTATS*>::iterator bi = statsList.begin(); bi != statsList.end(); bi++)
    {
        const BBLSTATS *b = (*bi);

        if ( b == 0 ) continue;
        
        for (const UINT16 * stats = b->_stats; *stats; stats++)
        {
            GlobalStatsDynamic.unpredicated[*stats] += b->_counter;
        }
    }


    DumpStats(*out, GlobalStatsDynamic, KnobProfilePredicated, "$dynamic-counts");                

    *out << "# $eof" <<  endl;

    out->close();
}