//指令级插桩，计数

#include <iostream>
#include <fstream>
#include "pin.H"

ofstream OutFile;

static UINT64 icount = 0;

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "inscount.out", "specify output file name");

VOID docount() { icount++; }

VOID Instruction(INS ins, VOID *v)
{
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_END);
}

VOID Fini(INT32 code, VOID *v)
{
    OutFile.setf(ios::showbase);
    OutFile << "Count " << icount << endl;
    OutFile.close();
}

int main(int argc, char * argv[])
{
    PIN_Init(argc, argv);
    OutFile.open(KnobOutputFile.Value().c_str());
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);
    PIN_StartProgram();
    return 0;
}