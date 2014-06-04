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
 #ifndef STDCFULLUTACTRL_H
#define STDCFULLUTACTRL_H

#include <vcoordinator.h>
#include <timer.h>
#include "pidsimple.h"



using namespace std;

typedef enum FullUTAConfigs
{
    //PIDs
    UTA_HEATBAT,
    UTA_COLDBAT,
    UTA_HUMBAT,
    UTA_VOCPID,
    UTA_CO2PID,
    UTA_LMMAX,
    UTA_LMMIN,
    UTA_POSTPID,
    UTA_FREECOOLINGPID,
    UTA_DEHUMPID,       //PID "fittizio" che serve solo per la deumidificazione

    //Sensors
    UTA_TEMPMND,
    UTA_TEMPRIP,
    UTA_TEMPEXT,
    UTA_HUMIDITY,
    UTA_VOC,
    UTA_CO2,
    UTA_SUMMERSWITCH,
    UTA_ONOFF,
    UTA_TEMPREG,

    //Actuators
    UTA_POST1,
    UTA_POST2,
    UTA_FANMND,
    UTA_FANRIP,
    UTA_MAINSHUTT,
    UTA_RECSHUTT,
    UTA_HUMIDIFIER,
    UTA_HEATPUMP,
    UTA_COLDPUMP,

    //SetPoints
    UTA_SUMMERTEMPSP,
    UTA_WINTERTEMPSP,
    UTA_SUMMERHUMSP,
    UTA_WINTERHUMSP,
    UTA_POSTHYST,
    UTA_SHUTTDELAY,

    //Main Logic
    UTA_USEFREECOOLING,
    UTA_USEAIRQUALITY,

    //State
    UTA_SUMMERSTATE,

    UTA_TOTPARAMETERS

}e_FullUTAConfigs;

/**
The Full UTA Controller is a coordinator that allows to manage all the controllers used in an Air Treatment Unit.
 * The confgiuration string is as follows:
 * NAME:FullUTACtrl,ADDR:YY,CONFIGID:,TIMERID, COMMENT:
 * Where:
 * CONFIGID: number of the configuration in the UTAConfig.ini file.

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CFullUTACtrl : public CVCoordinator
{
public:
    CFullUTACtrl(const char *configString,CTimer *timer);

    ~CFullUTACtrl();

    bool Update(bool updateData);
    bool Update2(bool updateData){return false;};
    bool SetVal(float val){return false;};
    
    CString GetSpontaneousData(int lParam = 0) { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};

    bool InitData();

    bool CheckSummer();
    bool SetSummer(bool isSummer);

    bool ConnectControllers();

    bool AcquireTempSetPoint(float *setPoint);

    /**
     * Il controllo umidita' viene fatto nel seguente modo: se e' estate ed e' troppo
    *umido aggiungo il valore del PID umidita' alla valvola principale per fare
    *freddissimo e condensare sulla batteria freddo, contemporaneamente il pid del
    *post aprira' il post per mantenere costante la mandata. Nasce il problema
    *se il post non ce la fa a mantenere la mandata...
     * @return true se va tutto bene
     */
    bool HumidityControl();
    bool AirQualityControl();

    //Funzione che implementa il controllo delle serrande
    //Presuppone che al minimo ci sia la serranda principale, se questa è assente esce
    //altrimenti controlla il funzionamento in funzione della logica
    //Le serrande DEVONO ESSERE dello stesso tipo (analogiche o ON/OFF)
    bool ShuttersControl(float tempRip);

    //I/O functions (failsafe rispetto alla presenza del dispositivo o meno)
    bool WriteOutput(int device, int value);
    bool GetInput(int device, float *value);
    bool GetInput(int device, bool *value);
    bool GetInput(int device, int * value);

    //PID functions
    bool UpdatePID(int device, float value);
    bool SetPIDSetpoint(int device, float value);

	bool SetIsUTAOn ( bool theValue );
	//{
	//	m_IsUTAOn = theValue;
	//}
	

	bool GetIsUTAOn()
	{
		return m_IsUTAOn;
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

    int m_ConfigID;

    float m_MainPIDOutputVolt;
    int m_MainPIDOutput;
    int m_HumidityPIDOutput;
    float m_TempMnd, m_TempRip, m_TempSetPoint;
    float m_Humidity;

    //se 0 sonda 0-10V con range 5-30, se 1 nostro ritaratore
    int m_TempRegType;

	bool m_IsUTAOn;

    //Configurazioni
    bool m_UseFreeCooling;
    int m_FreeCoolingPercentage; //Percentuale minima di apertura serrande esterne per il ricambio di aria
    bool m_UseAirQuality;


};

#endif
