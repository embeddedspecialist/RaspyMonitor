/***************************************************************************
 *   Copyright (C) 2005 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it                                           *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 ***************************************************************************/
//This file contains all the common definitions for the OneWire Application

#include "cstring.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

#ifndef __COMMONDECLARATIONS_H__
#define __COMMONDECLARATIONS_H__



#define MAX_NUM_NET 50              //Maximum number of nets manageable
#define MAX_NUM_DEV_PER_NET  5000   //Maximum number of devices per each net
#define WL_RETRIES_COUNT 10         //Maximum number of retries to acquire the master if NET is wireless

#define DS18B20_FN 0x28
#define DS18S20_FN 0x10
#define DS2405_FN  0x05
#define DS2438_FN  0x26
#define DS2890_FN  0x2C
#define DS2408_FN  0x29
#define DS2751_FN  0x51
#define DS1990A_FN  0x01

//Guard Time per i moduli XBee PRO, in ms
#define XBEE_GUARDTIME 10

//ROM Commands Specific defines
#define SKIPROM  0xCC
#define MATCHROM 0x55
#define CONVERT_T 0x44
#define CONVERT_V 0xB4

//Generic Numbers
#define MINVAL -100000000.0

//Error values for the sensors
#define TEMP_ERRVAL 85.0
#define ANALOG_ERRVAL -1.0

#define CONFIG_FILE "./config.ini"

//////////////////////////////////////////
//Debug/Error variables
//////////////////////////////////////////
#define DEBUG_NONE      0
#define DEBUG_ESSENTIAL 1
#define DEBUG_PRIMARY   2
#define DEBUG_ALL       4

//////////////////////////////////////////
//Tempistiche
//////////////////////////////////////////
#define TEMP_CONVERSION_TIME_MS 1000
#define DIGITAL_OUT_CHECK_INTERVAL 5000
#define ANALOG_OUT_CHECK_INTERVAL 7500


///////////////////////////////////////////////////
//Definizione dato per driver
///////////////////////////////////////////////////
typedef enum dataType
{
    DATA_INVALID_TYPE,
    DATA_FLOAT,
    DATA_REGISTER
}e_DataType;

typedef struct dataVal
{
    unsigned char regData[32];
    float floatData[32];
    e_DataType type;
    bool isValid;

    inline void init() { memset(this,0,sizeof(dataVal)); }

    dataVal() { init(); }
}t_DataVal;
 ////////////////////////////////////////////
 //Enumerations and strings for the devices
 ////////////////////////////////////////////

typedef enum deviceTypes
{
    DEV_NONE,
    DEV_DS18B20,
    DEV_DS18S20,
    DEV_DS18X20,
    DEV_DS2438,
    DEV_DS2890,
    DEV_DS2408,
    DEV_DS2405,
    DEV_DS2751,
    DEV_DIDO,
    DEV_DIGITALIN2OUT,
    DEV_PIDSIMPLE,
    DEV_PIDLMD,
    DEV_HUMIDITY,
    DEV_AIAO,
    DEV_PUMPCTRL,
    DEV_TEMPCTRL,
    DEV_TEMPCTRLHYST,
    DEV_REMOTEDIDO,
    DEV_BUTTONCTRL,
    DEV_VLVCTRL,
    DEV_STEPDIGITALOUT,
    DEV_TAGCTRL,
    DEV_UTACTRL,
    DEV_DI2AO,
    DEV_CNT,
    DEV_MGC,
    DEV_ACC,
    DEV_FULLUTACTRL,
    DEV_AFOVLV,
    DEV_ALARMCTRL,
    DEV_DIGITALCTRL,
    DEV_CHOVER,
    DEV_TIMEMARKER,
    DEV_CLIMATICOORD,
    DEV_C3POINT,
    DEV_ACCESSCOORD,
    DEV_AFOVLV_VAV,
    DEV_IBUTT_RDR,
    DEV_FULLUTACTRL_2,
    DEV_FLOORCOORD_2,
    DEV_BLOCKCOORD,
    DEV_THU,
    DEV_NUMTOT,     //!< total number of Devices Recognized
} e_DeviceType;

////////////////////////////////////////////////////
// Tipi, defines e macros per CDigitalCtrl        //
////////////////////////////////////////////////////

//Enumeratore sottotipi di CDigitalIN2OUT
typedef enum deviceSubType
{
    SUBDEV_NONE, //Errore
    SUBDEV_BUTTONCTRL,
    SUBDEV_REMOTEDIDO,
    SUBDEV_TAGCTRL,
    SUBDEV_STEPDIGITALIO,
} e_DeviceSubType;

//VALORI RESTITUITI//
typedef short HRET;
#define HFALSE 0
#define HTRUE 1
#define HINVALID -1
inline HRET toHRET(bool value) { return ( static_cast<HRET>(value) ); } //short: 0 o 1
inline bool toBOOL(HRET value) { if( value<=0 ) { return false; } else { return true; } } //il -1 diventa false comunque.
//-----------------//


extern const char * Device_strings [];

//////////////////////////////////////////////////////
//Enumerations and strings for the configuration file
//Sections COMMON and Common parameters for the NETS
//////////////////////////////////////////////////////
typedef enum ConfigVars
{
    CONF_IBUTTONREADERPORT,
    CONF_NOFINTERFACEPORT,
    CONF_INTERFACEPORT,
    CONF_NOFCOMMPORTIN,
    CONF_COMMPORTIN,
    CONF_NOFCOMMPORTOUT,
    CONF_COMMPORTOUT,
    CONF_NUMBEROFNETS,
    CONF_DEFAULTTEMPALARMS,
    CONF_ENABLELOG,
    CONF_LOGINTERVAL,
    CONF_LOGERRORS,
    CONF_MAXDIMERR,
    CONF_FILENAME,
    CONF_MAXFILEDIM,
    CONF_DODEBUG,
    CONF_DOSETUP,
    CONF_POLLDIGITAL,
    CONF_POLLACTIVITY,
    CONF_POLLTEMP,
    CONF_POLLTEMPALARMS,
    CONF_POLLPID,
    CONF_POLLHUMID,
    CONF_POLLANALOG,
    CONF_WAITONSTARTUP,
    CONF_MAXNOFERRORS,
    CONF_BLACKLISTTIMEOUT,
    CONF_USEWATCHDOG,
    CONF_NETCOMPORT,
    CONF_NETWL,
    CONF_NETDELAY,
    CONF_NETDEFAULTTEMPALARMS,
    CONF_NETSWTEMPALARMS,
    CONF_NETTIMERID,
    CONF_NETDIGOUTDELAY,
    CONF_NETNUMBEROFDEVICES,
    CONF_GENERALALARM,
    CONF_NETISOVERIP,
    CONF_HOTELFILE,
    CONF_REPEATCOMMAND,
    CONF_VIRTUALNET,
    CONF_UPDATETIME,
    CONF_SAVEDIGITALSTATE,
    CONF_TOTALNUMBEROFPARAM
} e_ConfigVars;

extern const char* Config_Strings[];


//////////////////////////////////////////////////////
//Enumerations and strings for the commands, choices are:
//////////////////////////////////////////////////////

//!XML Message Struct
typedef struct msgStruct {
    CString command;
    CString content;
} t_Msg;

//Commands recognized by the onewireengine
typedef enum CommandTypes
{
    COMM_NONE,                          //0
    COMM_SETTALARM,                     //1
    COMM_GETTEMP,                       //2
    COMM_ENABLETIMER,                   //3
    COMM_SETTIMERID,                    //4
    COMM_CHANGETIMERSETTINGS,           //5
    COMM_CHANGETIMERLEVELS,             //6
    COMM_SETDIGITALOUTPUT,              //7
    COMM_GETDIDO,                       //8
    COMM_GETAIAO,                       //9
    COMM_SETANALOGOUTPUT,               //10
    COMM_SETPIDPARAM,                   //11
    COMM_SETPIDSETPOINT,                //12
    COMM_GETPIDINFO,                    //13
    COMM_SETSUMMER,                     //14
    COMM_GETHUMIDITY,                   //15
    COMM_SETHUMIDITYSETTINGS,           //16
    COMM_GETHUMIDITYSETTINGS,           //17
    COMM_SETPOLLMODE,                   //18
    COMM_GETTIMERSETTINGS,              //19
    COMM_GETTEMPALSETTINGS,             //20
    COMM_GETHYSTTEMPCTRLSETTINGS,       //21
    COMM_SETHYSTTEMPCTRLSETTINGS,       //22
    COMM_ENABLETIMERBYID,               //23
    COMM_GETDATEANDTIME,                //24
    COMM_SETDATEANDTIME,                //25
    COMM_CHANGEDOSTATE,                 //26
    COMM_GETPUMPCTRLSETTINGS,           //27
    COMM_SETPUMPCONTROLLERSWAPTIME,     //28
    COMM_GETVOWCTRLSETTINGS,            //29
    COMM_SETVOWCTRLSETTINGS,            //30
    COMM_ENABLETEMPALARMS,              //31
    COMM_GETTEMPALARMSTATE,             //32
    COMM_GETROOMDATA,                   //33
    COMM_SETBUTTONCODE,                 //34
    COMM_GETPERSONNELLIST,              //35
    COMM_CHECKOUTROOM,                  //36
    COMM_SETROOMSTATUS,                 //37
    COMM_SETAIRCOND,                    //38
    COMM_GETPLANTSTATE,                 //39
    COMM_SETPLANTSTATE,                 //40
    COMM_SETALARMSTATE,                 //41
    COMM_SETSETPOINT,                   //42
    COMM_SETFLOORSETTINGS,              //43
    COMM_GETFLOORSETTINGS,              //44
    COMM_GETVLV2SETTINGS,               //45
    COMM_SETVLV2SETTINGS,               //46
    COMM_SETZONESTATE,                  //47
    COMM_ACTIVATECONTROLLER,            //48
    COMM_GETSPONTANEOUSDATA,            //49
    COMM_ENABLEFARNETO,                 //50
    COMM_TURNON,                        //51
    COMM_GETCLIMATICCURVE,              //52
    COMM_SETCLIMATICURVE,               //53
    COMM_CHANGEDRIVERDATA,              //54
    COMM_SETIPADDR,                     //55
    COMM_CHANGEINIFILE,                 //56
    COMM_BLOCKCOMMAND,                  //57
    COMM_NUMTOT                         //58
} e_CommandTypes;

extern const char *commandStrings[];
////////////////////////////////////////////////////////////
typedef enum HotelCommandTypes
{
    H_COMM_NONE,
    H_COMM_GETIBUTTONCODE,
    H_COMM_PROGRAMROOM,
    H_COMM_GETROOMDATA,
    H_COMM_GETROOMSETTINGS,
    H_COMM_SETROOMSETTINGS,
    H_COMM_ERASEROOMCODES,
    H_COMM_HNUMTOT
} e_HotelCommandTypes;

extern const char *HotelCommandStrings[];

//////////////////////////////////////////////////////
//Timers section
//////////////////////////////////////////////////////
//Timers defines
#define MAX_NUMBER_OF_TZ 10
#define MAX_NUMBER_OF_TIMERS 10
typedef enum TimerValues
{
    TIMERVAL_DIGITAL,
    TIMERVAL_ANALOG,
    TIMERVAL_PID,
    TIMERVAL_PIDH,
    TIMERVAL_PIDL,
    TIMERVAL_ALARM,
    TIMERVAL_HYST,
    TIMERVAL_NUMTOT,
} e_TimerValues;



typedef struct TimeZone
{
    int tz_Hour;
    int tz_Min;
    int tz_Level;
} t_TimeZone;

typedef struct TimerArray
{
    int nOfValues; //The total number of valid values in the timeZones array

    t_TimeZone timeZones[MAX_NUMBER_OF_TZ];

} t_TimerArray;

typedef struct TimerMx
{
    bool isTimerEnabled;
    int levelsMatrix[3][TIMERVAL_NUMTOT];
    t_TimerArray timerDay[7];

} t_TimerMx;

typedef enum daysInWeek
{
    SUN,
    MON,
    TUE,
    WED,
    THU,
    FRI,
    SAT,
    TOTALDAYS
} e_DaysInWeek;

extern const char *timerValsString[];
extern const char* daysStrings[];


//////////////////////////////////////////////////////
//Access Control Data
//////////////////////////////////////////////////////
typedef enum idCategories
{
    NONE,
    GUEST,
    PERSONNEL,
    ALL
}e_IDcategories;

typedef struct accessData {
    accessData(){roomNumber = -1;expireDateSec = 0; category = NONE; enablesAlarms = false; channel = 2;};
    int roomNumber;
    CString name;
    CString keySN;
    unsigned int expireDateSec;
    e_IDcategories category;
    bool enablesAlarms;
    unsigned int channel;   //E' il canale della scheda di controllo: 0, 1 o entrambi (2)
}t_AccessData;

#define ACCESS_CNTRL_NOF_RECORDS "NofRecords"

//////////////////////////////////////////////////////
//Pump controllers data
//////////////////////////////////////////////////////
typedef struct IntegerCouple
{
    int first;
    int second;
} t_IntCouple;

typedef struct FloatCouple
{
    int first;
    int second;
} t_FloatCouple;
///////////////////////////////////////////////////////////////
typedef struct pidVar
{
    float value;
    bool updated;
    bool isRemoted;
    pidVar(){value = TEMP_ERRVAL; updated = false;isRemoted = false;}
}t_PidVar;

#endif
