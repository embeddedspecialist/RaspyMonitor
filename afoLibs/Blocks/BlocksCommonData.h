/* 
 * File:   BlocksCommonData.h
 * Author: amirrix
 *
 * Created on 9 ottobre 2009, 16.20
 */

#ifndef _BLOCKSCOMMONDATA_H
#define	_BLOCKSCOMMONDATA_H

#define BLOCKS_FILE "./Blocks.ini"

typedef enum {
    BLOCK_NO_DATA,
    BLOCK_NOF_SUBSYSTEMS,
    BLOCK_NOF_BLOCKS,
    BLOCK_UPTIME,
    BLOCK_NUMTOT_CONF
}e_BlockConfig;

extern const char* Block_Config_Strings[];

 ////////////////////////////////////////////
 //Enumerations and strings for the blocks
 ////////////////////////////////////////////
typedef enum
{
    BLK_NONE,
    BLK_IF,
    BLK_ARITHMETIC,
    BLK_DELAY,
    BLK_LOGIC,
    BLK_PID,
    BLK_SATURATION,
    BLK_HYSTERESIS,
    BLK_COUNTER,
    BLK_COSTANT,
    BLK_GATE,
    BLK_TIMER,
    BLK_MUX,
    BLK_TRIGGER,
    BLK_BINARYENCDEC,
    BLK_C3POINT,
    BLK_CLOCK,
    BLK_CLIMATIC,
    BLK_NUMTOT,     //!< total number of Devices Recognized
} e_BlockType;

extern const char * Block_strings [];

///////////////////////////////////////////////////////
//         Various enumerations
///////////////////////////////////////////////////////
typedef enum
{
    PID_TYPE_PF,
    PID_TYPE_LMD,
    PID_TYPE_NUM_TOT
}e_PIDType;

extern const char * PID_Type_Strings[] ;

typedef enum
{
    IF_TYPE_EQUAL,
    IF_TYPE_GREATTHAN,
    IF_TYPE_LESSTHAN,
    IF_TYPE_GREAT_EQ,
    IF_TYPE_LESS_EQ,
    IF_TYPE_NOT_EQUAL,
    IF_TYPE_NUM_TOT
}e_IFType;

extern const char * IF_Type_Strings[];

typedef enum
{
    OP_TYPE_ADD,
    OP_TYPE_SUB,
    OP_TYPE_MUL,
    OP_TYPE_DIV,
    OP_TYPE_REM,
    OP_TYPE_NUM_TOT
} e_OPType;

extern const char * OP_Type_Strings[];

typedef enum
{
    LOG_TYPE_AND,
    LOG_TYPE_NAND,
    LOG_TYPE_OR,
    LOG_TYPE_NOR,
    LOG_TYPE_XOR,
    LOG_TYPE_XNOR,
    LOG_TYPE_NOT,
    LOG_TYPE_NUM_TOT
} e_LOGType;

extern const char * LOG_Type_Strings[] ;

typedef enum
{
    SAT_TYPE_SAT,
    SAT_TYPE_HI,
    SAT_TYPE_LOW,
    SAT_TYPE_NUM_TOT
} e_SATType;

extern const char * SAT_Type_Strings[];

typedef enum
{
    HYST_TYPE_HYST,
    HYST_TYPE_NUM_TOT
} e_HYSTType;

extern const char * HYST_Type_Strings[];

typedef enum
{
    GATE_TYPE_JK,
    GATE_TYPE_TOG,
    GATE_TYPE_NUM_TOT
} e_GATEType;

extern const char * GATE_Type_Strings[];

typedef enum {
    TRIGGER_LOW,
    TRIGGER_HI,
    TRIGGER_RISE,
    TRIGGER_FALL,
    TRIGGER_NUM_TOT
}e_TRIGGERType;

extern const char* TRIGGER_Type_Strings[];

typedef enum {
    BINARY_ENCODER,
    BINARY_DECODER
}e_BINARYENCDEC_Type;
////////////////////////////////////////////////////////////////////////////////

#endif	/* _BLOCKSCOMMONDATA_H */

