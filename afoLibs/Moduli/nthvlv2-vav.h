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
#ifndef NTHVLV2_VAV_H_INCLUDED
#define NTHVLV2_VAV_H_INCLUDED

#include "upid2.h"

#define TEMP_IN 0
#define ANALOG_IN 1
#define REMOTE_IN 2

using namespace std;

/**
Class used to interface with the NTH-VLV modules based on the new hardware (ARM7)
NAME:AFO-VLV, INPUT, SUMMER, SP1,KP1, Tint1, Tder1, DIV1,[SP2,KP2, Tint2, Tder2, DIV2], CH1TYPE, CH2TYPE, TIMERID, ADDR, COMMENT
 * dove CH1TYPE indica il tipo di ingresso che la uPID deve gestire: 0 = temperatura, 1 - Analogico, 2 - Fornito dall'esterno

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CNTHVLV2_VAV : public CUPID2
{
    public:
    CNTHVLV2_VAV(const char *configString,  CTimer *timer = 0x0);

    ~CNTHVLV2_VAV();

    bool Update(bool updatefirst);
    bool Update2(bool updateData){return false;};
    
    CString GetSpontaneousData(int lParam = 0);// { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};

    //Definisco 2 variabili per estate inverno
    bool m_PID1_IsSummer;
    bool m_PID2_IsSummer;

    //Parametri dei PID
    int m_Pid1Parameters[3];
    int m_PID1Divider;

    int m_Pid2Parameters[3];
    int m_PID2Divider;

    bool ProgramPID(int pidNumber);
    bool SetSummer(bool isSummer, bool saveFile = true, int pidNumber = 0);
    bool SetPIDParam(int pidNumber,float *param, int pidDivider, bool saveFile = true);
    bool GetPIDParam(int pidNumber,float *dest, int *pidDivider);
    bool SetPIDSetpoint(int pidNumber, float newSP, bool saveFile = true);
    bool TurnOnModule(int pidNumber, bool turnOn);
    bool ProgramBoard();
    bool SetSetPointLimits();

    bool ConnectControllers(void* netPtr, const char *configString);

    //TODO da completare con il parsing fi tutti i comandi accettati
    bool ExecCommand(CXMLUtil* xmlUtil);

    bool m_PIDIsSummer;

    float m_Temp1;
    float m_SetPoint1;
    bool m_IsPID1On;
    float m_AnalogIn1;
    float m_AnalogOut1;

    float m_Temp2;
    float m_SetPoint2;
    bool m_IsPID2On;
    float m_AnalogIn2;
    float m_AnalogOut2;

    float m_SetPointLow;
    float m_SetPointHigh;

    int m_CH1Type, m_CH2Type;
    
    //Controllori usati per la remotizzazione delle informazioni
    CVController *m_InController1, *m_InController2;
    //Puntatore alla classe COnewireNet
    void* m_NetPtr;

    //Legge attraverso la classe passata i suoi parametri da file
    //e riprogramma il modulo nth-afo-vlv associato secondo i dati
    bool InitDevice(CIniFileHandler *iniFileHandler=0x0);
    
    bool GetControlVar(int channel, float *dest);

    bool m_ModuleInitOK;
    
    //Indica lo stato dei ritaratori esterni (byte basso rit1): 0 = rit relativo, 1 = rit assoluto, F = no ritaratore
    unsigned char m_ExtRit;
    
    //Indica la presenza di interruttore E/I (se 1)
    bool m_ExtSummerSwitch;
    
    //Indica la presenza di interruttori esterni per acc/spegn (byte basso accensione 1)
    unsigned char m_ExtPowerSwitch;
};





#endif // NTHVLV2_H_INCLUDED
