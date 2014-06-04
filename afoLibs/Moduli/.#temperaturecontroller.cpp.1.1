/***************************************************************************
 *   Copyright (C) 2006 by Alessandro Mirri   *
 *   alessandro.mirri@newtohm.it   *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 *************************************************************************/
 
#include "temperaturecontroller.h"
#include "conewireengine.h"

CTempCtrl::CTempCtrl( const char* configString, CVDevice * inDevice, CTimer * timer )
    : CVController(configString)
{
    
    if (configString != NULL)
    {
        m_IniLib.GetConfigParamInt(configString,"ALARMMIN",&m_AlarmMin, -100);
        m_IniLib.GetConfigParamInt(configString,"ALARMMAX",&m_AlarmMax, -100);
        m_IniLib.GetConfigParamInt( configString, "TOA", (int*)(&m_TimeOutOnAlarm), 60);
        m_IniLib.GetConfigParamBool(configString, "SWALARMS", &m_SoftwareAlarms, 1);
        m_IniLib.GetConfigParamInt( configString, "TIMERID", &m_TimerID, -1);
       
        CreateIO(configString, false);
        CreateIO(configString, true);
    
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
 
    m_AlarmEnabled = true;
    m_PhoneAlarmActive = false;
    
    
    m_LastTemp = TEMP_ERRVAL;
    m_IsInAlarm = false;
    m_TimeOfLastAlarm = 0;
    m_TypeOfTimerVal = TIMERVAL_ALARM;  
    
    //Set input device
    m_Device = inDevice;
    m_DeviceFamilyNumber = m_Device->GetFamNum();

   //Set controller type
    m_ControllerType = DEV_TEMPCTRL;
    m_DeviceType = DEV_TEMPCTRL;

}

CTempCtrl::CTempCtrl( const char* configString, CTimer * timer ) : CVController(configString)
{
    if (configString != NULL)
    {
        m_IniLib.GetConfigParamInt(configString,"ALARMMIN",&m_AlarmMin, -100);
        m_IniLib.GetConfigParamInt(configString,"ALARMMAX",&m_AlarmMax, -100);
        m_IniLib.GetConfigParamInt( configString, "TOA", (int*)(&m_TimeOutOnAlarm), 60);
        m_IniLib.GetConfigParamBool(configString, "SWALARMS", &m_SoftwareAlarms, 1);
        m_IniLib.GetConfigParamInt( configString, "TIMERID", &m_TimerID, -1);
        m_IniLib.GetConfigParamBool( configString, "INVPIO", &m_IsPIOInverted, true);
   
        
        CreateIO(configString, false);
        CreateIO(configString, true);
        
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
  
    m_AlarmEnabled = true;
    m_PhoneAlarmActive = false;
    
    m_LastTemp = TEMP_ERRVAL;
    m_IsInAlarm = false;
    m_TimeOfLastAlarm = 0;
    m_TypeOfTimerVal = TIMERVAL_ALARM;  
    
    //Set input device
    m_Device = 0x0;
    m_DeviceFamilyNumber = 0x0;

   //Set controller type
    m_ControllerType = DEV_TEMPCTRL;
}

///////////////////////////////////////////////////
//              STANDARD DESTRUCTOR
///////////////////////////////////////////////////
CTempCtrl::~CTempCtrl()
{
    vector<CDigitalIO*>::iterator inIt;
    
    if (m_DefreezeDevice != 0x0)
    {
        delete m_DefreezeDevice;
    }   
    
    if (m_InVector.size() > 0)
    {
        for (inIt = m_InVector.begin(); inIt < m_InVector.end(); inIt++)
        {
            delete (*inIt);
            m_InVector.erase (inIt);
        }
    }
    
    if (m_OutVector.size() > 0)
    {
        for (inIt = m_OutVector.begin(); inIt < m_OutVector.end(); inIt++)
        {
            delete (*inIt);
            m_OutVector.erase (inIt);
        }
    }
        
}


///////////////////////////////////////////////////
//              READTEMPERATURE
///////////////////////////////////////////////////
bool CTempCtrl::ReadTemperature(bool updateFirst, float *newTemp)
{
    bool retVal = false;
    
    //Get The temperature
    if ( (m_DeviceFamilyNumber == DS18S20_FN) || (m_DeviceFamilyNumber == DS18B20_FN) )
    {
        retVal = ((CDS18X20*)(m_Device))->ReadTemperature(updateFirst, newTemp);
    }
    else if (m_DeviceFamilyNumber == DS2438_FN)
    {
        retVal = ((CDS2438*)(m_Device))->ReadTemperature(updateFirst, newTemp);
    }
  
    if (retVal)
    {
        m_LastTemp = *newTemp;
        ClearError();
    }
    else
    {
        AddError();
    }
    
   // return the result flag retVal
   return retVal;
}

///////////////////////////////////////////////////
//              ReadTemperature (2)
///////////////////////////////////////////////////
float CTempCtrl::ReadTemperature( bool updateFirst )
{
    float temp = TEMP_ERRVAL;
    
    ReadTemperature( updateFirst, &temp);
    
    return temp;
}

///////////////////////////////////////////////////
//              UPDATETEMPERATURE
///////////////////////////////////////////////////
bool CTempCtrl::UpdateTemp( )
{
    bool retVal = false;
    
    //Get The temperature
    if ( (m_DeviceFamilyNumber == DS18B20_FN) || (m_DeviceFamilyNumber == DS18S20_FN) )
    {
        retVal = ((CDS18X20*)(m_Device))->UpdateTemp();
    }
    else if (m_DeviceFamilyNumber == DS2438_FN)
    {
        retVal = ((CDS2438*)(m_Device))->UpdateTemp();
    }
  
     
    return retVal;
}

///////////////////////////////////////////////////
//              SETALARMLEVEL
///////////////////////////////////////////////////
bool CTempCtrl::SetAlarmLevel( int maxAlarmLevel, int minAlarmLevel )
{
    bool retVal;
    int tempMaxAl, tempMinAl; //Temporary values for the alarms
    float compensation;
  
      
    if  ( (m_DeviceFamilyNumber == DS18B20_FN) || (m_DeviceFamilyNumber == DS18S20_FN) )
    {
        compensation = ((CDS18X20*)(m_Device))->GetCompensation();
    }
    else
    {
        compensation = ((CDS2438*)(m_Device))->GetCompensation();
    }
    
    //Update the alarms and add the compensation to render the device transparent
    if (compensation > 0)
    {
        tempMaxAl = maxAlarmLevel + (int)compensation;
        tempMinAl = minAlarmLevel - (int)compensation;
    }
    else
    {
        tempMaxAl = maxAlarmLevel - (int)compensation;
        tempMinAl = minAlarmLevel + (int)compensation;
    }
      
    //Check if the alarms have to be managed by software or if the device is a DS2438 that does not have the software alarms
    if ((m_SoftwareAlarms) || (m_DeviceFamilyNumber == DS2438_FN))
    {
        retVal = true;
    }
    else
    {
        retVal = ((CDS18X20*)(m_Device))->SetAlarmLevel(maxAlarmLevel, minAlarmLevel);
    }

    //Update the alarms levels
    if (retVal)
    {
        m_AlarmMax = tempMaxAl;
        m_AlarmMin = tempMinAl;
    }
    
     

    return retVal;
}

///////////////////////////////////////////////////
//              GETLASTTEMP
///////////////////////////////////////////////////
float CTempCtrl::GetLastTemp( )
{
    //If we don't have any Temperature just return -100
    return m_LastTemp;
}

///////////////////////////////////////////////////
//              AUTOSETALARMLEVEL
///////////////////////////////////////////////////
bool CTempCtrl::AutoSetAlarmLevel( )
{
    //Check if the alarms have been correctly loaded
    if ((m_AlarmMax != -100) && (m_AlarmMin != -100))
    {
        return SetAlarmLevel(m_AlarmMax, m_AlarmMin);
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////
//              SetAlarmState
///////////////////////////////////////////////////
void CTempCtrl::SetAlarmState( )
{
    bool alarmCondition = false;
    
    //Check if we manage the alarms by ourselves or by external calling
    if (m_SoftwareAlarms)
    {
        //Check if we have a valid temperature and if  the temperature is outside the alarms interval
        if ( (m_LastTemp != TEMP_ERRVAL)  &&  ((m_LastTemp < m_AlarmMin) || (m_LastTemp > m_AlarmMax)) )
        {
            alarmCondition = true;
        }
    }
    else
    {
        //The alarms are managed by upper NET
        alarmCondition = true;
    }
    
    if (alarmCondition)
    {
        //Check if alarm already signaled
        if (!m_IsInAlarm)
        {
            //Alarm not already signaled
            
            //Get Time
            m_TimeOfLastAlarm = time(NULL);
            
            //Raise flag
            m_IsInAlarm = true;
        }
    }

}

///////////////////////////////////////////////////
//              CheckAlarmTime
///////////////////////////////////////////////////
bool CTempCtrl::CheckAlarmTime( )
{
    time_t actTime;
    bool retVal = false;
    
    if (m_IsInAlarm)
    {
        //Sensor in alarm condition
        actTime = time(NULL);
        
        if (actTime > m_TimeOfLastAlarm + m_TimeOutOnAlarm)
        {
            //Time has expired
            retVal = true;
        }
    }
    
    return retVal;
}

///////////////////////////////////////////////////
//              UpdateAlarmStatus
///////////////////////////////////////////////////
bool CTempCtrl::UpdateAlarmStatus(t_DataVal data )
{
    
    //Check if the timer is active
    if ( m_UseTimer && IsTimerEnabled())
    {
        if ( !(bool)(GetValFromTimer()))
        {
            //Timer is enabled AND the timer says alarms off
            m_IsInAlarm = false;
        }   
    }
    else
    {
        //Check if the last temperature measured is a good one
        if ( data.isValid )
        {
            //Check if we are inside the alarms levels or outside
            if ( (data.floatData[0] < m_AlarmMin) || (data.floatData[0] > m_AlarmMax) )
            {
                //Check we we already discovered an alarm
                if (!m_IsInAlarm)
                {
                    m_IsInAlarm = true;
                    m_TimeOfLastAlarm = time(NULL);
                }
            }
            else
            {
                m_IsInAlarm = false;
            }
        }
    }
    
    return m_IsInAlarm;
}

///////////////////////////////////////////////////
//              SetSoftwareAlarms
///////////////////////////////////////////////////
void CTempCtrl::SetSoftwareAlarms( bool enableSWAlarms )
{
    if (enableSWAlarms)
    {
        m_SoftwareAlarms = true;
    }
    else
    {
        m_SoftwareAlarms = false;
    }
}

///////////////////////////////////////////////////
//              Update
///////////////////////////////////////////////////
bool CTempCtrl::Update( bool updateData )
{
    time_t actTime;
    bool defreezeActive = false;

    actTime = time(NULL);
    
    //Update the internal temp
    if (updateData)
    {
        ReadTemperature( true );
    }
    
    //Update the internal status
    //UpdateAlarmStatus();
   
                
    if (m_HasDefreeze)
    {
        defreezeActive = m_DefreezeDevice->GetState();
    }
    else
    {
        defreezeActive = false;
    }
    //If it is in alarm AND the alarm is enabled AND is not defreezing
    if (m_IsInAlarm && m_AlarmEnabled && (!defreezeActive)) 
    {
        if (CheckAlarmTime())
        {
            //Abilita allarmi HW
            //TODO da riempire
            if (!m_PhoneAlarmActive)
            {
                //Muovi piedino 6 del connettore JP8
                ActivatePhoneCaller(true);
            }
        }       
    }
    
    return true;
}

///////////////////////////////////////////////////
//              GetAlarmLevel
///////////////////////////////////////////////////
bool CTempCtrl::GetAlarmLevel( int * maxAlarmLevel, int * minAlarmLevel )
{
    if ((maxAlarmLevel != 0x0) && (minAlarmLevel != 0x0))
    {
        *maxAlarmLevel = m_AlarmMax;
        *minAlarmLevel = m_AlarmMin;
        return true;
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////
//              VerifyIOPresence
///////////////////////////////////////////////////
bool CTempCtrl::VerifyIOPresence( )
{
    //TODO da cambiare nel caso ci siano altri dispositivi
    return m_Device->VerifyPresence();
}

///////////////////////////////////////////////////
//              SetInputDevice
///////////////////////////////////////////////////
bool CTempCtrl::SetInputDevice( CVDevice * inDevice )
{
    m_Device = inDevice;
    m_DeviceFamilyNumber = m_Device->GetFamNum();
    
    return true;
}

///////////////////////////////////////////////////
//              CreateIO
///////////////////////////////////////////////////
bool CTempCtrl::CreateIO(const char * configString, bool isOutput)
{
    int nOfDigitals = 0, digitalIndex, input, channel;
    CString idxString, chString;
    bool retVal = false;
    char configBuffer[255];
    int invertDevice, invertValue;
    
    memset (configBuffer, 0x0, 255*sizeof(char));
    
    if (!isOutput)
    {
        //First search for the defreeze device
        m_IniLib.GetConfigParamInt(configString, "INPUTDF", &input, -1);
        
        if (input > 0)
        {
            m_IniLib.GetConfigParamInt(configString, "DFCH", &channel, -1);
            if (channel > 0)
            {
                m_HasDefreeze = true;
                
                m_IniLib.GetConfigParamInt(configString, "INVDF", &invertDevice, 0);
                sprintf (configBuffer,"INPUT:%d,CHANNEL:%d,INVERTOUT:%d,IO:1,TIMERID:-1",input,channel,invertDevice);
                m_DefreezeDevice = new CDigitalIO(configBuffer);
            }
            else
            {
                m_HasDefreeze = false;
                m_DefreezeDevice = 0x00;
            }
        }
        else
        {
            m_HasDefreeze = false;
            m_DefreezeDevice = 0x00;
        }
    }
    
    if (isOutput)
    {
        m_IniLib.GetConfigParamInt( configString, "NOUT", &nOfDigitals, 0);
        m_IniLib.GetConfigParamInt( configString, "INVERTOUT", &invertValue, 0);
        m_NofOutputs = nOfDigitals;
    }
    else
    {
        m_IniLib.GetConfigParamInt( configString, "NIN", &nOfDigitals, 0);
        m_IniLib.GetConfigParamInt( configString, "INVERTIN", &invertValue, 0);
        m_NofInputs = nOfDigitals;
    }
    
    if (nOfDigitals == 0)
    {
        return retVal;
    }
    
    
    
    for (digitalIndex = 1; digitalIndex < nOfDigitals+1; digitalIndex++)
    {
        if (isOutput)
        {
            idxString="OUT";
        }
        else
        {
            idxString="IN";
        }
        
        idxString+=digitalIndex;
        
        if (isOutput)
        {
            chString="OUT";
            chString+="CH";
            chString+=digitalIndex;
        }
        else
        {   
            chString="IN";
            chString+="CH";
            chString+=digitalIndex;
        }
        
        m_IniLib.GetConfigParamInt( configString, idxString.c_str(), &input, -1);
        m_IniLib.GetConfigParamInt( configString, chString.c_str(), &channel, -1);
        
        if ((input < 0) || (channel < 0))
        {
            retVal = false;
            break;
        }
        else
        {
            invertDevice = (invertValue & (0x01<<(digitalIndex-1)))>>(digitalIndex-1);
            memset (configBuffer, 0x0, 255*sizeof(char));
            
            if (isOutput)
            {
                sprintf (configBuffer,"INPUT:%d,CHANNEL:%d,INVERTOUT:%d,IO:0,TIMERID:-1",input,channel,invertDevice);
                m_OutVector.push_back(new CDigitalIO(configBuffer, 0x0));
            }
            else
            {
                sprintf (configBuffer,"INPUT:%d,CHANNEL:%d,INVERTOUT:%d,IO:1,TIMERID:-1",input,channel,invertDevice);
                m_InVector.push_back(new CDigitalIO(configBuffer, 0x0));
            }
            
            
            retVal = true;
        }
    }
    
    return retVal;
}

///////////////////////////////////////////////////
//              ActivatePhoneCaller
///////////////////////////////////////////////////
void CTempCtrl::ActivatePhoneCaller(bool activate)
{
    int fdg; //Handlers to all ports
    uchar exitChar = 'X';
    int portNum = 0;
    EtraxGPIO gpio;
    
    m_PhoneAlarmActive = activate;

    //12-2009 I cervelletti FOX si piantano spesso, non vorrei ci fosse qualche casino
    //su sta cosa: la disattivo tanto non serve ad una beata mazza.
    return;
    
#ifndef CRIS
    //TBR
    if (activate)
    {
        cout << "Allarme telefonico ATTIVO !!!"<<endl;
    }
#else
    if ((fdg = open("/dev/gpiog", O_RDWR))>=0) 
    {
        //This pins must be accessed in group
        gpio.init_output(fdg, LINE_8 | LINE_9 | LINE_10 | LINE_11 | LINE_12 | LINE_13 | LINE_14 | LINE_15 );
        gpio.init_output(fdg, LED_1);
    
        //Use line 13 for reprogramming the device and line 14 to send a reset
        gpio.set_output(fdg, LINE_13 | LED_1);
        gpio.set_output( fdg, LINE_14 );
    
        //Use pin 6 of the JP8 connector
        if (m_IsPIOInverted)
        {
            activate = !activate;
        }
        
        if (activate)
        {
            gpio.set_output(fdg, LINE_13 | LED_1);
        }
        else
        {
            gpio.clear_output(fdg,  LINE_13 | LED_1);
        }
    
        //Close port
        close(fdg);
    }
    else
    {
        //TODO aggiungere errore codificato!!
        printf("\n\n********** Open error on /dev/gpiog **********\n\n");
    }
#endif
    
    //Update all the outputs
    for (int i = 0; i < m_NofOutputs; i++)
    {
        m_OutVector[i]->SetState(activate);
    }
}

///////////////////////////////////////////////////
//              InitTempController
///////////////////////////////////////////////////
void CTempCtrl::InitTempController()
{
    //Init the pin 6 of JP8 connector
    ActivatePhoneCaller(false);
    
    m_PhoneAlarmActive = false;

    m_IsInAlarm = false;
    
    //Update all the outputs
    for (int i = 0; i < m_NofOutputs; i++)
    {
        m_OutVector[i]->SetState(false);
    }
}

///////////////////////////////////////////////////
//              EnablePhoneAlarm
///////////////////////////////////////////////////
void CTempCtrl::EnablePhoneAlarm(bool newState)
{
    ActivatePhoneCaller(false);
    m_IsInAlarm = false;
    m_AlarmEnabled = newState;
}


///////////////////////////////////////////////////
//              GetSpontaneousData
///////////////////////////////////////////////////
CString CTempCtrl::GetSpontaneousData(int lParam)
{
    CString retVal;
    
    Cmd com("DEVICE");
    
    com.putValue("ADDRESS", GetMemoryAddress());
    com.putValue("TYPE","TempController");
    com.putValue("TEMP",m_LastTemp);
    com.putValue("ALL",m_IsInAlarm);
    
    retVal = com.getXMLValue();
    
    return retVal;
}
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
bool CTempCtrl::Update2(bool updateData)
{
    time_t actTime;
    bool defreezeActive = false;
    COneWireEngine *engPtr = reinterpret_cast<COneWireEngine*>(m_EnginePtr);
    actTime = time(NULL);
    
    //Invio il messaggio se il dato è valido:
    t_DataVal data = m_Device->GetDriverData();
    if (data.isValid)
    {
        m_LastTemp = data.floatData[0];
        m_Data.isValid = true;
        m_Data.floatData[0] = m_LastTemp;
        
        //Update the internal status
        UpdateAlarmStatus(data);
        
        Cmd com("DEVICE");
        com.putValue("TYPE","TempController");
        com.putValue("TEMP",data.floatData[0]);
        com.putValue("ADDRESS",m_Address);
        com.putValue("ALL",m_IsInAlarm);
        
        engPtr->UpdateServerPorts(true,false);
        engPtr->WriteOnInterfacePorts(com.toString().c_str(), (int)com.toString().size());
    }
    else
    {
         m_Data.isValid = false;
    }
    
    return true;
}
