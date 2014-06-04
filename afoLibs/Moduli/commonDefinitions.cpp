/***************************************************************************
 *   Copyright (C) 2006 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it                                           *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 *************************************************************************/

#define __CONFIG_DEV_NAME_CLASSIC_

#ifndef __CONFIG_ITALIAN_
#define __CONFIG_ITALIAN_
#endif

//////////////////////////////////////////
//Debug variables
//////////////////////////////////////////
//static bool doDebug;


 //! Device String name definition
#ifdef __CONFIG_DEV_NAME_CLASSIC_
const char * Device_strings [] =
{
    "NODEV",
    "DS18B20",
    "DS18S20",
    "DS18X20",
    "DS2438",
    "DS2890",
    "DS2408",
    "DS2405",
    "DS2751",
    "DigitalINOUT",
    "DigitalIn2Out",
    "PIDSimple",
    "PIDLMD",
    "Humidity",
    "AnalogINOUT",
    "PumpController",
    "TempController",
    "TempCtrlHyst",
    "RemoteDIDO",
    "ButtonController",
    "ValveController",
    "StepDigitalOut",
    "TAGController",
    "UTAController",
    "DIn2AOut",
    "AFO-CNT",
    "AFO-MGC",
    "AFO-ACC",
    "FullUTACtrl",
    "AFO-VLV",
    "AlarmCtrl",
    "DiDo",
    "ChangeoverCoord",
    "TimeMarker",
    "ClimaticCurveCoord",
    "C3PointCtrl",
    "AccessCoord",
    "VLV-VAV",
    "IButtRdr",
    "FullUTA2_Ctrl",
    "Floor2_Coord",
    "BlockCoord",
    "Thu"

};
#endif

#ifdef __CONFIG_DEV_NAME_NTH_
extern const char * Device_strings [] =
{
    "NODEV",
    "NTH-AFO-TMP-B",
    "NTH-AFO-TMP-S",
    "DS18X20",
    "NTH-AFO-GP", //Sonda general purpose (AI, lux, hum)
    "NTH-AFO-AO",
    "NTH-AFO-DIDO-8",
    "NTH-AFO-DIDO-2",
    "NTH-AFO-VOW",  //Virtual one wire
    "DigitalINOUT",
    "PIDSimple",
    "PIDLMD",
    "Humidity",
    "AnalogINOUT",
    "PumpController",
    "TempController",
    "TempCtrlHyst",
    "RemoteDIDO",
    "ButtonController",
    "ValveController",
    "StepDigitalOut",
    "TAGController",
    "UTAController",
    "DIn2AOut",
    "AFO-CNT",
    "AFO-MGC",
    "AFO-ACC",
    "FullUTACtrl",
    "AFO-VLV",
    "AlarmCtrl",
    "DiDo",
    "C3Point"
};
#endif

#ifdef __CONFIG_ITALIAN_
const char* Config_Strings[] =
{
    "IButtonReader",
    "NPorteInterfaccia",
    "PortaInterfaccia",
    "NPorteComIn",
    "PortaComunicazioneIn",
    "NPorteComOut",
    "PortaComunicazioneOut",
    "TotalNets",
    "SoglieDiAllarme",
    "AbilitaLog",
    "IntervalloLog",
    "RegistraErrori",
    "DimensioneFileErrori",
    "File",
    "MAXDimensioneFile",
    "DODEBUG",
    "EseguiSetup",
    "GestioneDigitalIO",
    "GestioneAttivita",
    "GestioneTemperature",
    "GestioneAllarmiTemp",
    "GestionePID",
    "GestioneUmid",
    "GestioneAnalogIO",
    "AttendiInAvvio",
    "MaxNoErrori",
    "BlackListTimeout",
    "UsaWatchDog",
    "PortaComunicazione",
    "Wireless",
    "RitardoNet",
    "SoglieDiAllarme",
    "AllarmiTemperaturaSw",
    "TimerID",
    "TempoScatto",
    "NofDev",
    "AllarmeGenerale",
    "OverIP",
    "HotelFile",
    "RepeatCommand",
    "VirtualNet",
    "UpdateTime",
    "SaveDigitalState"
};
#endif

#ifdef __CONFIG_CLASSIC_
extern const char* Config_Strings[] =
{
    "IButtonReaderPort",
    "InterfacePort",
    "InCommPort",
    "OutCommPort",
    "TotalNets",
    "AllarmiDefault",
    "EnableLog",
    "LogTime",
    "FilePort",
    "MAXFileSize",
    "DODEBUG",
    "DoSetup",
    "PollDigitalIO",
    "PollTemp",
    "PollAlarms",
    "PollPID",
    "PollHum",
    "PollAnalogInput",
    "WaitOnStartup",
    "MaxNofErrors",
    "BlackListTimeout",
    "COMPORT",
    "WIRELESS",
    "NetDelay",
    "AllarmiDefault",
    "SwTempAlarms",
    "TimerID",
    "TempoScatto",
    "NofDev"
};
#endif

const char *commandStrings[] =
{
    "NoCommand",                    //0
    "SetTAlarm",                    //1
    "GetTemp",                      //2
    "EnableTimer",                  //3
    "SetTimerID",                   //4
    "ChangeTimerSettings",          //5
    "ChangeTimerLevels",            //6
    "SetDigitalOutput",             //7
    "GetDIDO",                      //8
    "GetAIAO",                      //9
    "SetAnalogOutput",              //10
    "SetPIDParam",                  //11
    "SetPIDSetPoint",               //12
    "GetPIDInfo",                   //13
    "SetSummer",                    //14
    "GetHumidity",                  //15
    "SetHumiditySettings",          //16
    "GetHumiditySettings",          //17
    "SetPollMode",                  //18
    "GetTimerSettings",             //19
    "GetTempAlarmsSettings",        //20
    "GetHystTempCtrlSettings",      //21
    "SetHystTempCtrlSettings",      //22
    "EnableTimerByID",              //23
    "GetDateAndTime",               //24
    "SetDateAndTime",               //25
    "ChangeDOState",                //26
    "GetPumpControllerSettings",    //27
    "SetPumpControllerSwapTime",    //28
    "GetVOWCtrlSettings",           //29
    "SetVOWCtrlSettings",           //30
    "EnableTempAlarms",             //31
    "GetTempAlarmState",            //32
    "GetRoomData",                  //33
    "SetButtonCode",                //34
    "GetPersonnelList",             //35
    "CheckoutRoom",                 //36
    "SetRoomStatus",                //37
    "SetAirCond",                   //38
    "GetPlantState",                //39
    "SetPlantState",                //40
    "SetAlarmState",                //41
    "SetSetpoint",                  //42
    "SetFloorSettings",             //43
    "GetFloorSettings",             //44
    "GetVLVSettings",               //45
    "SetVLVSettings",               //46
    "SetZoneState",                 //47
    "ActivateController",           //48
    "GetSpontaneousData",           //49
    "EnableFarneto",                //50
    "TurnON",                       //51
    "GetClimaticCurve",             //52
    "SetClimaticCurve",             //53
    "ChangeDriverData",             //54
    "SetIPAddress",                 //55
    "ChangeINIFile",                //56
    "BlockCommand",                 //57
    "CommandError"                  //58
};

const char *HotelCommandStrings[] =
{
    "NoCommand",
    "GetButtonRomCode",
    "ProgramRoomModule",
    "GetRoomData",
    "GetRoomSettings",
    "SetRoomSettings",
    "EraseRoomCodes",
    "CommandError"
};

const char *timerValsString[] =
{
    "Digital",
    "Analog",
    "PID",
    "PIDH",
    "PIDL",
    "Alarm",
    "Hyst"
};

const char* daysStrings[] =
{
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat"
};


