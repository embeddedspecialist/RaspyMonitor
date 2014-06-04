#include "BlocksCommonData.h"



const char* Block_Config_Strings[] = {
    "NONE",
   "TotalSubSystems",
    "NofBlocks",
    "UpdateTime"
};
/////////////////////////////////////////////////////////////////
const char * Block_strings [] =
{
    "NODEV",
    "IF",
    "OPERAZIONE",
    "DELAY",
    "LOGICO",
    "PID",
    "SATURAZIONE",
    "ISTERESI",
    "CONTATORE",
    "COSTANTE",
    "GATE",
    "TIMER",
    "MUX",
    "TRIGGER",
    "BINARYENCDEC",
    "C3POINT",
    "CLOCK",
    "CLIMATICA"
};
//////////////////////////////////////////////////////////////////////////
const char * PID_Type_Strings[] =
{
    "PF",
    "LMD"
};
/////////////////////////////////////////////////////////////////////////////
const char * IF_Type_Strings[] =
{
    "EQ",
    "GT",
    "LT",
    "GET",
    "LET",
    "NE"
};
/////////////////////////////////////////////////////////////////////////////
const char * OP_Type_Strings[] =
{
    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "REM"
};
/////////////////////////////////////////////////////////////////////////////
const char * LOG_Type_Strings[] =
{
    "AND",
    "NAND",
    "OR",
    "NOR",
    "XOR",
    "XNOR",
    "NOT"
};
////////////////////////////////////////////////////////////////////////////
const char * SAT_Type_Strings[] =
{
    "SAT",
    "SAT_HI",
    "SAT_LOW"
};
////////////////////////////////////////////////////////////////////////////
const char * HYST_Type_Strings[] =
{
    "HYST"
};

const char * GATE_Type_Strings[] =
{
    "JK",
    "TOG"
};

const char* TRIGGER_Type_Strings[]=
{
    "LEV_LOW",
    "LEV_HI",
    "RISE",
    "FALL",
};