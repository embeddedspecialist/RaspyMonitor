/***************************************************************************
 *   Copyright (C) 2008 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it                                           *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 *************************************************************************/
 #ifndef STDCFULLUTACTRL2_H
#define STDCFULLUTACTRL2_H

#include <vcoordinator.h>
#include <timer.h>
#include "pid.h"


#define UTA2_INIFILE "./UTACoord2.ini"
using namespace std;


typedef enum FullUTAConfigs2
{
    //PID Params
    FULLUTA_PID_DIRECT,
    FULLUTA_PID_LMMAX,
    FULLUTA_PID_LMMIN,
    FULLUTA_PID_HUMD,
    FULLUTA_PID_FREECOOLING,
    FULLUTA_PID_AIRQUALITY,
    FULLUTA_PID_POST,
    
    //Sensors
    FULLUTA_TEMPMND,
    FULLUTA_TEMPRIP,
    FULLUTA_TEMPEXT,
    FULLUTA_HUMIDITY,
    FULLUTA_VOC,
    FULLUTA_CO2,
    FULLUTA_SUMMERSWITCH,
    FULLUTA_ONOFF,
    FULLUTA_TEMPREG,

    //Actuators
    FULLUTA_HEATBAT,    //AnalogIO Batteria calda analogica o monobatteria
    FULLUTA_COLDBAT,    //AnalogIO Batteria fredda
    FULLUTA_HUMBAT,     //AnalogIO umidificatore
    FULLUTA_POST1,      //DigitalIO Accensione post o AnalogIO per POST
    FULLUTA_POST2,      //DigitalIO seconda accensione POST
    FULLUTA_FANMND,     //DigitalIO ventilatori mandata
    FULLUTA_FANRIP,     //DigitalIO Ventilatori ripresa
    FULLUTA_MAINSHUTT,  //AnalogIO Serrande mandata e ripresa
    FULLUTA_RECSHUTT,   //AnalogIO Serranda di ricircolo
    FULLUTA_HUMIDIFIER, //DigitalIO Comando umidificatore
    FULLUTA_HEATPUMP,   //DigitalIO comando pompa caldo
    FULLUTA_COLDPUMP,   //DigitalIO comando pompa freddo

    //SetPoints
    FULLUTA_SUMMERTEMPSP,
    FULLUTA_WINTERTEMPSP,
    FULLUTA_SUMMERHUMSP,
    FULLUTA_WINTERHUMSP,
    FULLUTA_POSTHYST,
    FULLUTA_SHUTTDELAY,
    FULLUTA_TEMPLIMMAX,
    FULLUTA_TEMPLIMMIN,

    //Main Logic
    FULLUTA_USEFREECOOLING,
    FULLUTA_USEAIRQUALITY,

    //State
    FULLUTA_SUMMERSTATE,

    FULLUTA_TOTPARAMETERS

}e_FullUTAConfigs2;

/**
The Full UTA Controller is a coordinator that allows to manage all the controllers used in an Air Treatment Unit.
 * The confgiuration string is as follows:
 * NAME:FullUTACtrl,ADDR:YY,CONFIGID:,TIMERID, COMMENT:
 * Where:
 * CONFIGID: number of the configuration in the UTAConfig.ini file.

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CFullUTACtrl2 : public CVCoordinator
{
public:
    CFullUTACtrl2(const char *configString,CTimer *timer);

    ~CFullUTACtrl2();

    bool Update(bool updateData);
    bool Update2(bool updateData){return false;};
    bool SetVal(float val){return false;};
    
    CString GetSpontaneousData(int lParam = 0) { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};

    bool InitData();

    bool CheckSummer();
    bool SetSummer(bool isSummer);

    bool ConnectControllers();

    bool AcquireTempSetPoint(t_DataVal *setPoint);

    
    bool AirQualityControl(){return false;};

    //Funzione che implementa il controllo delle serrande
    //Presuppone che al minimo ci sia la serranda principale, se questa è assente esce
    //altrimenti controlla il funzionamento in funzione della logica
    //Le serrande DEVONO ESSERE dello stesso tipo (analogiche o ON/OFF)
    bool ShuttersControl();

    //I/O functions (failsafe rispetto alla presenza del dispositivo o meno)
    bool WriteOutput(int device, int value);
    //I/O functions (failsafe rispetto alla presenza del dispositivo o meno)
    bool WriteOutput(int device, float value);
    bool GetInput(int device, t_DataVal *value);
    bool GetInput(int device, bool *value);

    //PID functions
    bool UpdatePID(int device, float value);
    bool SetPIDSetpoint(int device, float value);

    bool ExecCommand(CXMLUtil* xmlUtil);
	

    //TBR
    float m_MainPIDOutput;
    float m_MainPIDOutputVolt;
    
	bool GetIsUTAOn()
	{
		return m_IsOn;
	}

    bool SetSetPoint ( float valueSP );

    bool m_AreShuttersOpen;
    time_t m_TimeOfShutterOpening;
    int m_ShutterDelay;
    
    bool m_IsSummer;
    
    float m_PostHyst;
    
    float m_WinterSetPoint;
    float m_SummerSetPoint;
    float m_WinterHumSP;
    float m_SummerHumSP;
    float m_HumHyst;
    
    int m_ConfigID;

    t_DataVal m_TempMnd, m_TempRip, m_TempSetPoint, m_TempExt;
    t_DataVal m_Humidity;

    //se 0 sonda 0-10V con range 5-30, se 1 nostro ritaratore
    int m_TempRegType;


    //Configurazioni
    bool m_UseFreeCooling;
    int m_FreeCoolingPercentage; //Percentuale minima di apertura serrande esterne per il ricambio di aria
    bool m_UseAirQuality;
    
    //Questi sono tutti i PID usati dall'UTA
    PID m_PIDVector[FULLUTA_PID_POST+1];
    
    void ManageHumdifier(bool turnOn);
    void ManagePOST(bool turnOn);
    void ManageMainPID();
    
    bool m_POST1_On;
    bool m_POST2_On;


};

#endif
