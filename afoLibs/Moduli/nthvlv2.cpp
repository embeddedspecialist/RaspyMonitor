/***************************************************************************
 *   Copyright (C) 2008 by Alessandro Mirri   *
 *   alesssandro.mirri@newtohm.it   *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it
 *   and/or modify  it in any way                                          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY; without even the    *
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR       *
 *   PURPOSE.                                                              *
 ***************************************************************************/
 #include "nthvlv2.h"
 
#include "conewirenet.h"
#include "conewireengine.h"

CNTHVLV2::CNTHVLV2(const char *configString,  CTimer *timer):CUPID2((char*)(configString))
{

    if (configString != 0x0)
    {
        //Leggo i parametri del PID
        m_IniLib.GetConfigParamFloat( configString, "SP1",&m_SetPoint1, 25.0);
        m_IniLib.GetConfigParamInt( configString, "KP1",(int*)( &m_Pid1Parameters[0]), 10);
        m_IniLib.GetConfigParamInt( configString, "Tint1", (int*)(&m_Pid1Parameters[1]), 1);
        m_IniLib.GetConfigParamInt( configString, "Tder1", (int*)(&m_Pid1Parameters[2]), 0);
        m_IniLib.GetConfigParamInt( configString, "DIV1", &m_PID1Divider, 0);

        m_IniLib.GetConfigParamFloat( configString, "SP2",&m_SetPoint2, 25.0);
        m_IniLib.GetConfigParamInt( configString, "KP2",(int*)( &m_Pid2Parameters[0]), 10);
        m_IniLib.GetConfigParamInt( configString, "Tint2", (int*)(&m_Pid2Parameters[1]), 1);
        m_IniLib.GetConfigParamInt( configString, "Tder2", (int*)(&m_Pid2Parameters[2]), 0);
        m_IniLib.GetConfigParamInt( configString, "DIV2", &m_PID2Divider, 0);

        m_IniLib.GetConfigParamBool( configString, "SUMMER1", &m_PID1_IsSummer, false);
        //Se non c'è SUMMER2 lo metto uguale all'1
        m_IniLib.GetConfigParamBool( configString, "SUMMER2", &m_PID2_IsSummer, m_PID1_IsSummer);

        m_IniLib.GetConfigParamInt( configString, "TIMERID", &m_TimerID, -1);

        m_IniLib.GetConfigParamInt(configString, "CH1TYPE", &m_CH1Type,TEMP_IN);
        m_IniLib.GetConfigParamInt(configString, "CH2TYPE", &m_CH2Type,TEMP_IN);

        m_IniLib.GetConfigParamFloat( configString, "SETPOINTL",&m_SetPointLow, 15.0);
        m_IniLib.GetConfigParamFloat( configString, "SETPOINTH",&m_SetPointHigh, 35.0);

        int tempInt;
        m_IniLib.GetConfigParamInt(configString, "EXTRIT", &tempInt, 0xFF);
        m_ExtRit = (unsigned char)tempInt;
        m_IniLib.GetConfigParamBool(configString, "EXTSUM", &m_ExtSummerSwitch, false);
        m_IniLib.GetConfigParamInt(configString, "EXTPWR", &tempInt, 0xFF);
        m_ExtPowerSwitch = (unsigned char)tempInt;
        m_IniLib.GetConfigParamInt(configString, "TIMESTOP", (int*)(&m_TimeStop), 1);

        if ( (m_TimerID > 0) && (timer != 0x0) )
        {
            m_Timer = timer;
            m_UseTimer = true;
        }
        else
        {
            m_UseTimer = false;
        }
    }

    m_Temp1 = m_Temp2 = m_AnalogIn1 = m_AnalogIn2 = m_AnalogOut1 = m_AnalogOut2 = ANALOG_ERRVAL;
    m_IsPID1On = m_IsPID2On = false;

    m_ModuleInitOK = false;

    m_TypeOfTimerVal = TIMERVAL_DIGITAL;
    m_ControllerType = DEV_AFOVLV;

    m_CodeRevision = 1;

}

///////////////////////////////////////////////////
//              StandardDestructor
///////////////////////////////////////////////////
CNTHVLV2::~CNTHVLV2()
{

}

bool CNTHVLV2::SetTimeStop(unsigned int newTimeStop, bool saveFile)
{
    uchar mapmem[16];

    memset (mapmem, 0xFF, 16);

    //Lo metto da subito uguale al valore interno così se il comando fallisce ci
    //riprovera' nei prossimi cicli di update
    m_TimeStop = newTimeStop;
    
    mapmem[0] = 0X02;
    mapmem[1] = 0x80;
    mapmem[14] = m_TimeStop;
    mapmem[15] = CalcCRC(mapmem);

    if (WriteToDevice(mapmem))
    {
        if (saveFile)
        {
            SaveConfigParam("TIMESTOP", CString("")+m_TimeStop);
        }

        return true;
    }

    return false;
}
/** @brief Sets the Setpoint for the PID controller
  *
  * @param pidNumber number of the PID to which the setpoint will be applied (1 or 2) or 0 for both PID's
  * @param newSP the new setpoint to apply to the PIDs
  */
bool CNTHVLV2::SetPIDSetpoint(int pidNumber, float newSP, bool saveFile)
{
    uchar mapmem[16];

    memset (mapmem, 0xFF, 16);

    mapmem[0] = 0X02;

    if (pidNumber == 0)
    {
        mapmem[1] = 6;
    }
    else if (pidNumber == 1)
    {
        mapmem[1] = 2;
    }
    else if (pidNumber == 2)
    {
        mapmem[1] = 4;
    }
    else
    {
        return false;
    }

//    //Scalo il setpoint tra 0 e 30 (Se e' temperatura sono gradi, se e' analogico i volt da tenere
//    mapmem[6] = (uchar)( (255.0/30.0)*newSP);
//    mapmem[7] = (uchar)( (255.0/30.0)*newSP);

    /* 5/11/2009
     * Modifico la scala del SetPoint per fare Step da 0,5°C
    */

    mapmem[6] = (uchar)( newSP*2 );
    mapmem[7] = (uchar)( newSP*2 );


    mapmem[15] = CalcCRC(mapmem);
    
    if (WriteToDevice(mapmem))
    {
        switch (pidNumber)
        {
            case 0: {
                m_SetPoint1 = m_SetPoint2 = newSP;
                break;
            }
            case 1: {
                m_SetPoint1 = newSP;
                break;
            }
            case 2: {
                m_SetPoint2 = newSP;
                break;
            }
        }

        if (saveFile)
        {
            SaveConfigParam("SP1", CString("")+m_SetPoint1);
            SaveConfigParam("SP2", CString("")+m_SetPoint2);
        }
        return true;
    }
    else
    {
        //TODO generazione errori
        return false;
    }
}
bool CNTHVLV2::SetSetPointLimits()
{
    uchar mapmem[16];

    memset(mapmem, 0xFF, 16);

    mapmem[0] = 0x06;
    mapmem[1] = 0x03;
    mapmem[2] = (uchar)(m_SetPointLow * 2);
    mapmem[3] = (uchar)(m_SetPointHigh * 2);
    mapmem[15] = CalcCRC(mapmem);

    if (WriteToDevice(mapmem))
    {
        return true;
    }

    return false;
}

/** @brief (one liner)
  *
  * (documentation goes here)
  */
bool CNTHVLV2::GetPIDParam(int pidNumber,float *dest, int *pidDivider)
{
    return false;

}

/** @brief (one liner)
  *
  * (documentation goes here)
  */
bool CNTHVLV2::SetPIDParam(int pidNumber,float *param, int pidDivider, bool saveFile)
{
    if (pidNumber == 0)
    {
    	for (int i = 0; i < 3; i++)
    	{
    	    m_Pid1Parameters[i] = m_Pid2Parameters[i] = (int)(param[i]);
    	}

    	m_PID1Divider = m_PID2Divider = pidDivider;
    }
    else if (pidNumber == 1)
    {
        for (int i = 0; i < 3; i++)
    	{
    	    m_Pid1Parameters[i] = (int)(param[i]);
    	}

    	m_PID1Divider = pidDivider;
    }
    else if (pidNumber == 2)
    {
        for (int i = 0; i < 3; i++)
    	{
    	    m_Pid2Parameters[i] = (int)(param[i]);
    	}

    	m_PID2Divider = pidDivider;;
    }
    else
    {
        return false;
    }

    if (ProgramPID(pidNumber) && saveFile)
    {
        //Salvo i parametri
        SaveConfigParam("KP1",CString("")+m_Pid1Parameters[0]);
        SaveConfigParam("Tint1",CString("")+m_Pid1Parameters[1]);
        SaveConfigParam("Tder1",CString("")+m_Pid1Parameters[2]);
        SaveConfigParam("DIV1",CString("")+m_PID1Divider);

        SaveConfigParam("KP2",CString("")+m_Pid2Parameters[0]);
        SaveConfigParam("Tint2",CString("")+m_Pid2Parameters[1]);
        SaveConfigParam("Tder2",CString("")+m_Pid2Parameters[2]);
        SaveConfigParam("DIV2",CString("")+m_PID2Divider);

        return true;
    }
    else
    {
        return false;
    }

}

/** @brief (one liner)
  *
  * (documentation goes here)
  */
bool CNTHVLV2::SetSummer(bool isSummer, bool saveFile, int pidNumber)
{
    uchar mapmem[16];

    unsigned char valueTotWrite = 0;
    if (pidNumber == 0){
        valueTotWrite = (((unsigned char)isSummer)<<4)|(((unsigned char)isSummer));
    }
    else if (pidNumber == 1)
    {
        valueTotWrite = (((unsigned char)m_PID2_IsSummer)<<4)|(((unsigned char)isSummer));
    }
    else
    {
        valueTotWrite = (((unsigned char)isSummer)<<4)|(((unsigned char)m_PID1_IsSummer));
    }

    memset(mapmem, 0xFF, 16);

    mapmem[0] = 0x02;

    mapmem[1] = 32;

    mapmem[10] = valueTotWrite;

    mapmem[15] = CalcCRC(mapmem);
    
    if (WriteToDevice(mapmem))
    {
        CString parameter = "SUMMER";
        if (pidNumber == 0){
            m_PID1_IsSummer = isSummer;
            if (saveFile)
            {
                SaveConfigParam("SUMMER1", CString("")+(int)(isSummer));
            }

            m_PID2_IsSummer = isSummer;
            if (saveFile)
            {
                SaveConfigParam("SUMMER2", CString("")+(int)(isSummer));
            }
        }
        else if (pidNumber == 1)
        {
            m_PID1_IsSummer = isSummer;
            parameter+=1;
        }
        else
        {
            m_PID2_IsSummer = isSummer;
            parameter+=2;
        }

        if (saveFile)
        {
            SaveConfigParam(parameter.c_str(), CString("")+(int)(isSummer));
        }

        return true;
    }
    else
    {
        return false;
    }
}

/** @brief (one liner)
  *
  * (documentation goes here)
  */
bool CNTHVLV2::ProgramPID(int pidNumber)
{
    uchar mapMem[16];

    memset (mapMem, 0xFF, 16);

    mapMem[0] = 0x04;

    if (pidNumber == 0)
    {
        mapMem[1] = 3;
    }
    else if (pidNumber == 1)
    {
        mapMem[1] = 1;
    }
    else
    {
        mapMem[1] = 2;
    }

    //Copio tutti i parametri
    for (int i = 0; i < 3; i++)
    {
        mapMem[2+i] = m_Pid1Parameters[i];
    }

    mapMem[5] = m_PID1Divider;

    for (int i = 0; i < 3; i++)
    {
        mapMem[6+i] = m_Pid2Parameters[i];
    }

    mapMem[9] = m_PID2Divider;

    mapMem[15] = CalcCRC(mapMem);

    return WriteToDevice(mapMem);
}

/** @brief (one liner)
  *
  * (documentation goes here)
  */
bool CNTHVLV2::Update(bool updatefirst)
{
    bool retVal = false;
    bool turnOn = false;
    uchar mapmem[16];
    float ctrlVar;

    memset (mapmem, 0xFF, 16);

    //Aggiornamento orario
    UpdateCommonData();

    if (!m_ModuleInitOK)
    {
        if (!InitDevice())
        {
            //Aggiungere errore sull'inizializzazione
            return false;
        }
    }

    //If we have a timer active get the value
    if (m_UseTimer && IsTimerEnabled())
    {
        //TODO al momento il controller è soggetto ad un solo timer quindi le accensioni dei due canali
        //sono trattate in maniera coerente
        turnOn = GetValFromTimer();

        if (turnOn != m_IsPID1On)
        {
            TurnOnModule(0, turnOn);
        }
    }

    //Leggo i valori dalla scheda
    mapmem[0] = 0x01;
    
    if (m_CH1Type == REMOTE_IN)
    {
        //Devo leggere la variabile di controllo
        GetControlVar(1,&ctrlVar);
        memcpy(&mapmem[1], &ctrlVar, 4);
    }
    
    if (m_CH2Type == REMOTE_IN)
    {
        //Devo leggere la variabile di controllo
        GetControlVar(2,&ctrlVar);
        memcpy(&mapmem[5], &ctrlVar, 4);
    }

    mapmem[15] = CalcCRC(mapmem);
    
    if (WriteToDevice(mapmem) && CheckCRC(mapmem))
    {
        float setpoint1, setpoint2;
        //Decodifico i vari campi
        //Faccio anche un controllo sui campi impostati per la coerenza di funzionamento perchè la scheda
        //se si riavvia non ha tutti i parametri impostati
        m_Temp1 = (float)(mapmem[6]/2.0);

        // 5/11/2009 Modificato per avere Step da 0,5°C
        //setpoint1 = (float)(mapmem[7]*30.0)/255.0;
        setpoint1 = (float)(mapmem[7]/2.0);
        
        m_AnalogOut1 = (mapmem[13]*10.0)/255.0;
        m_Temp2 = (float)(mapmem[8]/2.0);

        // 5/11/2009 Modificato per avere Step da 0,5°C
        //setpoint2 = (float)(mapmem[9]*30.0)/255.0;
        setpoint2 = (float)(mapmem[9]/2.0);
        
        m_AnalogOut2 = (mapmem[14]*10.0)/255.0;
        m_AnalogIn1 = (mapmem[11]*10.0)/255.0;
        m_AnalogIn2 = (mapmem[12]*10.0)/255.0;

        //Confronto lo stato della scheda con quello memorizzato e riallineo se necessario
        if (m_ExtPowerSwitch & 0x0F)
        {
            //C'e' l'interruttore esterno-> mi limito a memorizzare lo stato
            m_IsPID1On = !(mapmem[10] & 0x01);
        }
        else
        {
            if (m_IsPID1On != (!(mapmem[10] & 0x01)))
            {
                cout << "RIPRISTINATO STATO ACCENSIONE"<<endl;
                TurnOnModule(0, m_IsPID1On);
            }
        }

        //Controllo estate/inverno
        if (m_ExtSummerSwitch)
        {
            //C'è l'interruttore esterno
            m_PID1_IsSummer = mapmem[1];
            m_PID2_IsSummer = mapmem[1];
        }
        else
        {
            if (m_PID1_IsSummer != (bool)(mapmem[1]&0x0F))
            {
                cout << "RIPRISTINATO stato estate/inverno PID1"<<endl;
                SetSummer(m_PID1_IsSummer,false,1);
            }

            if (m_PID2_IsSummer != (bool)((mapmem[1]&0xF0)>>4))
            {
                cout << "RIPRISTINATO stato estate/inverno PID2"<<endl;
                SetSummer(m_PID2_IsSummer,false,2);
            }
        }

        if ( (m_ExtRit & 0x0F) != 0x0F)
        {
            //C'è il ritaratore, mi limito a leggere dentro il setpoint
            m_SetPoint1 = setpoint1;
        }
        else
        {
            //Sulla FOX non c'e' la libreria math.h, faccio il controllo "manuale"
            if ( ((m_SetPoint1 - setpoint1) > 1.0) || ((setpoint1 - m_SetPoint1) > 1.0))
            {
                //Riallineo i setpoint
                cout<<"Riallineato setpoint1"<<endl;
                SetPIDSetpoint(1,m_SetPoint1,false);
            }
            else
            {
                m_SetPoint1 = setpoint1;
            }
        }

        
        //ATTENZIONE IL PROGRAMMA IN HARLEY SULLE Upid2 NON AGGIORNA IL SETPOINT2 PERCHÈ NON LO USA E QUINDI QUESTO CONTROLLO
        //FALLISCE SEMPRE!!!
        if ( (m_ExtRit & 0xF0) != 0xF0)
        {
            //C'e' il ritaratore assoluto
            m_SetPoint2 = setpoint2;
        }
        else
        {
            if ( ((m_SetPoint2 - setpoint2) > 1.0) || ((setpoint2 - m_SetPoint2) > 1.0))
            {
                cout<<"Setpoint2 = " << setpoint2 <<" Riallineato setpoint2"<<endl;
                SetPIDSetpoint(2,m_SetPoint2,false);
            }
            else
            {
                m_SetPoint2 = setpoint2;
            }
        }

        //Controllo l'eventuale temp di isteresi delle batterie se c'e' il cambio automatico:
        if (mapmem[2] != m_TimeStop)
        {
            cout<<"TimeStop Interno: " << m_TimeStop <<" TimeStop Ricevuto: "<<mapmem[2]<<" Riallineato timestop"<<endl;
            SetTimeStop(m_TimeStop, false);
        }
        
        retVal = true;

        if (m_DebugLevel)
        {
            cout << "Afo-Vlvl indirizzo: " << m_Address << " Impostazioni temperature: Temp1:" << m_Temp1
                    << ", TEMP2:" << m_Temp2 << ", SP1:" << m_SetPoint1<< ", SP2:" << m_SetPoint2 << ", SUMMER1:"<<m_PID1_IsSummer
                    << ", SUMMER2:"<<m_PID2_IsSummer<<" Timestop:"<<m_TimeStop<<endl;
        }

    }

    return retVal;
}

/** @brief (one liner)
  *
  * (documentation goes here)
  */
bool CNTHVLV2::InitDevice(CIniFileHandler *iniFileHandler)
{
    
    //Programmazione dei PID
    m_ModuleInitOK = ProgramBoard();
    m_ModuleInitOK = m_ModuleInitOK && ProgramPID(0);
    m_ModuleInitOK = m_ModuleInitOK && SetPIDSetpoint(1,m_SetPoint1, false);
    m_ModuleInitOK = m_ModuleInitOK && SetPIDSetpoint(2,m_SetPoint2, false);
    m_ModuleInitOK = m_ModuleInitOK && SetSetPointLimits();
    m_ModuleInitOK = m_ModuleInitOK && SetSummer(m_PID1_IsSummer, false,1);
    m_ModuleInitOK = m_ModuleInitOK && SetSummer(m_PID2_IsSummer, false,2);
    m_ModuleInitOK = m_ModuleInitOK && TurnOnModule(0, false);
    m_ModuleInitOK = m_ModuleInitOK && SetTimeStop(m_TimeStop,false);
    m_ModuleInitOK = m_ModuleInitOK && UpdateCommonData();

    return m_ModuleInitOK;

}

/** @brief (one liner)
  *
  * (documentation goes here)
  */
bool CNTHVLV2::TurnOnModule(int pidNumber, bool turnOn)
{
    uchar mapmem[16];

    memset(mapmem, 0xFF, 16);

    mapmem[0] = 0x02;

    if (pidNumber == 0)
    {
        mapmem[1] = 24;
    }
    else if (pidNumber == 1)
    {
        mapmem[1] = 8;
    }
    else if (pidNumber == 2)
    {
        mapmem[1] = 16;
    }
    else
    {
        return false;
    }

    mapmem[8] = (int)turnOn;
    mapmem[9] = (int)turnOn;

    mapmem[15] = CalcCRC(mapmem);
    
    if (WriteToDevice(mapmem))
    {
        switch (pidNumber)
        {
            case 0:{
                m_IsPID1On = m_IsPID2On = turnOn;
                break;
            }
            case 1:{
                m_IsPID1On = turnOn;
                break;
            }
            case 2:{
                m_IsPID2On = turnOn;
            }
        };
        return true;
    }
    else{
        return false;
    }

}

/**
 * Programs the board to exclude the external summer switch and on/of switch
 * @return true if successfull
 */
bool CNTHVLV2::ProgramBoard()
{

    uchar mapmem[16];

    memset(mapmem, 0xFF, 16);

    mapmem[0] = 0x04;
    mapmem[1] = 0x3C;
    mapmem[10] = (unsigned char)(m_ExtSummerSwitch); //Interruttore esterno estate/inverno
    mapmem[11] = m_ExtPowerSwitch; //Comandi di accensione sono da sistema
    mapmem[12] = m_ExtRit; //Ritaratori esterni
    
    mapmem[13] = 0x0;
    //Imposto se gli ingressi li deve prendere dalla temperatura, dagli analogici o li fornisco io da remoto
    if (m_CH1Type == TEMP_IN)
    {
        mapmem[13] = 0; //Ingresso in temperatura
    }
    else if (m_CH1Type == ANALOG_IN)
    {
        mapmem[13] = 1; //Ingresso analogico
    }
    else 
    {
        mapmem[13] = 2; //Ingresso fornito da remoto
    }
    
    if (m_CH2Type == TEMP_IN)
    {
        mapmem[13] = mapmem[13] & (0x0F);
    }
    else if (m_CH2Type == ANALOG_IN)
    {
        mapmem[13] |= 0x10;
    }
    else
    {
        mapmem[13] |=0x20;
    }

    mapmem[15] = CalcCRC(mapmem);

    if (WriteToDevice(mapmem))
    {
        return true;
    }

    return false;

}
///////////////////////////////////////////////////////////////////////////////////////////
bool CNTHVLV2::ConnectControllers(void * netPtr, const char * configString)
{
    int netIndex = m_NetNumber-1;
    int devIndex = -1;
    
    if (netPtr == 0x0)
    {
        return false;
    }
    
    m_NetPtr = netPtr;
    COneWireNet *netHandler = reinterpret_cast<COneWireNet*>( netPtr );
    
    if ((configString == 0x0) || (netHandler == 0x0))
    {
        return false;
    }
    
    if ((m_CH1Type != REMOTE_IN) && (m_CH2Type != REMOTE_IN))
    {
        //Gli ingressi non sono remotati esco
        return true;
    }
    
    if (m_CH1Type == REMOTE_IN)
    {
        m_IniLib.GetConfigParamInt(configString, "CH1INPUT", &devIndex, -1);
        devIndex = netHandler->GetCtrlIndexByConfigNumber(netIndex, devIndex);
        if (devIndex == -1)
        {
            cout << "Si è verificato un errore nel collegamento del canale 1 del modulo VLV2, indice: "<<m_DeviceNumber<< " della NET: "<< m_NetNumber<<endl;
            return false;
        }
        
        m_InController1 = netHandler->GetNetHandler(netIndex)->CtrlList[devIndex];
    }
    
    if (m_CH2Type == REMOTE_IN)
    {
        m_IniLib.GetConfigParamInt(configString, "CH2INPUT", &devIndex, -1);
        devIndex = netHandler->GetCtrlIndexByConfigNumber(netIndex, devIndex);
        if (devIndex == -1)
        {
            cout << "Si è verificato un errore nel collegamento del canale 2 del modulo VLV2, indice: "<<m_DeviceNumber<< " della NET: "<< m_NetNumber<<endl;
            return false;
        }
        
        m_InController2 = netHandler->GetNetHandler(netIndex)->CtrlList[devIndex];
    }
    
    return true;
}
/////////////////////////////////////////////////////////////////////////
bool CNTHVLV2::GetControlVar(int channel, float * dest)
{
    CVController *controller;
    
    if (channel == 1)
    {
        controller = m_InController1;
    }
    else if (channel == 2)
    {
        controller = m_InController2;
    }
    else
    {
        return false;
    }
    
    if (controller->GetControllerType() == DEV_AIAO)
    {
        //Becco il valore in volt
        *dest = ((CAnalogIO*)controller)->GetLastValueVolt();
        return true;
    }
    else if (controller->GetControllerType() == DEV_TEMPCTRL)
    {
        //Becco la temperatura
        *dest = ((CTempCtrl*)controller)->GetLastTemp();
        return true;
    }
    else
    {
        return false;
    }

    return false;
}
/////////////////////////////////////////////////////////////////////////
CString CNTHVLV2::GetSpontaneousData(int lParam)
{
    Cmd com("DEVICE");

    com.putValue("TYPE","VLV2State");
    com.putValue("ADDRESS",GetMemoryAddress());
    com.putValue("SETPOINT",m_SetPoint1);
    com.putValue("TEMP1",m_Temp1);
    com.putValue("TEMP2",m_Temp2);
    com.putValue("SETPOINT2", m_SetPoint2);
    com.putValue("SUMMER1",m_PID1_IsSummer);
    com.putValue("SUMMER2",m_PID2_IsSummer);
    com.putValue("ANALOGOUT1", m_AnalogOut1);
    com.putValue("ANALOGOUT2", m_AnalogOut2);
    com.putValue("ANALOGIN1", m_AnalogIn1);
    com.putValue("ANALOGIN2", m_AnalogIn2);
    com.putValue("DIGOUT",m_IsPID1On);
     
    return com.toString();
}
/////////////////////////////////////////////////////////////////////////
bool CNTHVLV2::ExecCommand(CXMLUtil* xmlUtil)
{
    bool retVal = false;

    e_CommandTypes command = (e_CommandTypes)ParseCommand(xmlUtil);
    COneWireEngine *engPtr = reinterpret_cast<COneWireEngine*>(m_EnginePtr);
    CIniFileHandler iniFile;

    //Faccio un primo screening su indirizzo per la generalità della funzione:
    //Se non c'è indirizzo è un comando "broadcast" quindi vedo se lo posso eseguire
    //altrimenti filtro sull'indirizzo
    if (xmlUtil->ExistsParam("ADDRESS"))
    {
        int address = xmlUtil->GetIntParam("ADDRESS");
        if (address != m_Address)
        {
            return false;
        }
    }
    else if (xmlUtil->ExistsParam("NET"))
    {
        int net = xmlUtil->GetIntParam("NET");
        if (net != m_NetNumber)
        {
            return false;
        }
    }

    switch (command)
    {
        case COMM_SETSETPOINT:
        {
            retVal = true;
            float setpoint = 0.0;

            if (xmlUtil->ExistsParam("SETPOINT"))
            {
                setpoint = xmlUtil->GetFloatParam("SETPOINT");
                retVal = SetPIDSetpoint(1,setpoint);
            }

            //Se c'è questo parametro differenzio i setpoint, altrimenti no
            if (xmlUtil->ExistsParam("SETPOINT2"))
            {
                setpoint = xmlUtil->GetFloatParam("SETPOINT2");
                retVal = SetPIDSetpoint ( 2, setpoint ) && retVal;
            }

            retVal = true;
        };break;
        case COMM_SETSUMMER:{
            retVal = true;
            bool setSummer = false;

            if (xmlUtil->ExistsParam("SUMMER"))
            {
                setSummer = xmlUtil->GetBoolParam("SUMMER");
                retVal = SetSummer(setSummer,true,0);
            }

            if (xmlUtil->ExistsParam("SUMMER1")){
                setSummer = xmlUtil->GetBoolParam("SUMMER1");
                retVal = SetSummer(setSummer,true,1);
            }

            //Se c'è questo parametro differenzio i setpoint, altrimenti no
            if (xmlUtil->ExistsParam("SUMMER2"))
            {
                setSummer = xmlUtil->GetBoolParam("SUMMER2");
                retVal = SetSummer(setSummer,true,2)&&retVal;
            }

            retVal = true;
        };break;
        default:break;
    }

    return retVal;
}
