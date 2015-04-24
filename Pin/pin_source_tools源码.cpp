/*
Source/tools/MyPinTool/MyPinTool.cpp
这是块粒度的PinTool
$ ../../pin-2.14-67254-gcc.4.4.7-linux/pin -t obj-intel64/MyPinTool.so -- /bin/ls
*/
#include "pin.H"
#include <iostream>
#include <fstream>

/*
命令行参数解析，KNOB class定义在source/include/pin/gen.knob.PH
选项-o和-count是跟在MyPinTool.so后面的
构造函数	KNOB(
					KNOB_MODE mymode,			//Per knob mode - indicates how multiple appearances of the mode on the command line are handled
					const string & myfamily,	//Family to which the knob belongs to 
					const string & myname, 		//Name of the knob 
					const string & mydefault,	//The default value to which the knob is initialized with 
					const string & mypurpose,	//A string that explains the purpose of the knob 
					const string & myprefix = ""//不知道)
					
*/
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,  "pintool", "o", "", "specify file name for MyPinTool output");
KNOB<BOOL>   KnobCount(KNOB_MODE_WRITEONCE,  "pintool", "count", "1", "count instructions, basic blocks and threads in the application");

//用法
INT32 Usage()
{
	//打印用法
	return -1;
}

/*
块粒度分析回调函数
*/
VOID bbl_analysis_func(type1 arg1, type2 arg2, ...)
{...}

/*
trace粒度插桩回调函数。TRACE class是Container for a trace
ypedef VOID(*LEVEL_PINCLIENT::TRACE_INSTRUMENT_CALLBACK)(TRACE trace, VOID *v)
*/
VOID trace_instrument_func(TRACE trace, VOID *v)
{
	//遍历操作trace中的每一BBL
	for(BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
	{
		//注册块粒度分析回调函数
		BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)bbl_analysis_func, IARG_TYPE1, arg1, ..., IARG_END);
	}
}

/*
线程启动的通知回调函数
VOID(* LEVEL_PINCLIENT::THREAD_START_CALLBACK)(THREADID threadIndex, CONTEXT *ctxt, INT32 flags, VOID *v)
Parameters:
	[in]	threadIndex	The Pin thread ID of the new thread.
	[in]	ctxt		Initial register state for the new thread. The tool may change this.
	[in]	flags		O/S specific flags for the new thread.
	[in]	v			The tool's call-back value. 
*/
VOID thread_strat_func(THREADID threadIndex, CONTEXT *ctxt, INT32 flags, VOID *v)
{...}

/*
程序结束回调函数
typedef VOID(* LEVEL_PINCLIENT::FINI_CALLBACK)(INT32 code, VOID *v)
Parameters:
	[in]	code	O/S specific termination code for the application.
	[in]	v		The tool's call-back value. 
*/
VOID Fini_name(INT32 code, VOID *v)
{...}

int main(int argc, char *argv[])
{
	//初始化Pin
	if(PIN_Init(argc, argv))
	{
		return Usage();
	}
	//获取选项-o的参数字符串
	string fileName = KnobOutputFile.Value();
    if (!fileName.empty()) { out = new std::ofstream(fileName.c_str());}
	if (KnobCount)	//不知道这个判断(count选项)是干嘛的
    {
        //注册trace粒度分析回调函数
        TRACE_AddInstrumentFunction(Trace, 0);

        //注册线程启动通知回调函数
        PIN_AddThreadStartFunction(thread_strat_func, 0);

        //注册程序结束回调函数
        PIN_AddFiniFunction(Fini_name, 0);
    }
	...
	//启动程序
	PIN_StartProgram();
	
	return 0;
}

/*===========================================================================================================================*/

/*
source/tools/ManualExamples/inscount0.cpp
计算程序中的指令数量
*/
#include <iostream>
#include <fstream>
#include "pin.H"

//命令行参数解析
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "inscount.out", "specify output file name");

//用法
INT32 Usage()
{...}
	
//指令级分析回调函数
VOID ins_analysis_func(type1 arg1, ...)
{...}

/*
指令级插桩回调函数
typedef VOID(* LEVEL_PINCLIENT::INS_INSTRUMENT_CALLBACK)(INS ins, VOID *v)
*/
VOID ins_instrument_func(INS ins, VOID *v)
{
	//注册指令级分析回调函数
	INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)ins_analysis_func, IARG_TYPE1, arg1, ..., IARG_END);
}

//程序结束回调函数
VOID Fini_name(INT32 code, VOID *v)
{...}

int main(int argc, char *argv[])
{
	//初始化Pin
	if(PIN_Init(argc, argv))
		return Usage();
	...
	//注册指令级插桩回调函数
    INS_AddInstrumentFunction(Instruction, 0);
    //注册程序结束回调函数
    PIN_AddFiniFunction(Fini, 0);
    //启动程序
    PIN_StartProgram();
    return 0;
}

/*===========================================================================================================================*/

/*
source/tools/ManualExamples/itrace.cpp
列出程序中所有指令的地址
*/

#include <stdio.h>
#include "pin.H"
