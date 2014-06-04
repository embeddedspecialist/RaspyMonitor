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
    "OROLOGIO",
    "LOGICO",
    "PID",
    "SATURAZIONE",
    "ISTERESI",
    "CONTATORE",
    "COSTANTE",
    "GATE",
    "TIMER"
};
//////////////////////////////////////////////////////////////////////////
const char * PID_Type_Strings[] =
{
    "PF",
    "LMD",
    "IST",
    "COMPEXT"
};
/////////////////////////////////////////////////////////////////////////////
const char * IF_Type_Strings[] =
{
    "EQ",
    "GT",
    "LT",
    "GE",
    "LE"
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

const char * CONT_Type_Strings[] =
{
    "CONT"
};

const char * GATE_Type_Strings[] =
{
    "JK",
    "TOG"
};