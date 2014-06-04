/***************************************************************************
*   Copyright (C) 2007 by Alessandro Mirri                                *
*   alessandro.mirri@newtohm.it                                           *
*                                                                         *
*   This program is NOT free software; you can NOT redistribute it and/or *
*   modify it in any way without the authorization of the author          *
*                                                                         *
*   This program is distributed WITHOUT ANY WARRANTY;                     *
*   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
*                                                                         *
***************************************************************************/
#include "utactrl.h"

//* NAME:UTAController, ADDR, DIN1, DIN1CH,DOUT1,DOUT1CH, DOUT2, DOUT2CH,DOUT3,DOUT3CH,DOUT4,DOUT4CH, AN1OUT, AN2OUT, INVERT,SP,SPH,SPL,KP1,TI1,TD1,KP2,TI2,TD2,SHDELAY,TIMERID,STARTV,COMMENT
CUtaCtrl::CUtaCtrl(const char* configString, CTimer *timer) : CVController(configString)
{
    int input, channel;
    
    m_ShutterOpen = false;
    m_ShutterOpening = false;
    m_ShutterClosing = false;
    m_Tag = 0;
    m_TempControllers[0] = m_TempControllers[1] = 0x0;
    m_OutDigitals[0] = m_OutDigitals[1] = m_OutDigitals[2] = m_OutDigitals[3] = 0x0;
    m_Analogs[0] = m_Analogs[1] = 0x0;
    
    if (configString != 0x0)
    {
        //Delay between the shutter opening and the start of the fan, default 180 sec
        m_IniLib.GetConfigParamInt( configString, "SHDELAY", &m_ShutterDelay, 180);
        
        m_IniLib.GetConfigParamBool( configString, "STARTV", &m_StartOn, false);
        
        m_IniLib.GetConfigParamInt( configString, "INVERT", &m_InvertDigital, 0);
        
        //TAG DigitalIN
        m_IniLib.GetConfigParamInt( configString, "DIN1", &input, -1);
        m_IniLib.GetConfigParamInt( configString, "DIN1CH", &channel, -1);
        CreateDigital(true, 0, input, channel, configString);
        
        //SHUTTEROPEN DigitalOut
        m_IniLib.GetConfigParamInt( configString, "DOUT1", &input, -1);
        m_IniLib.GetConfigParamInt( configString, "DOUT1CH", &channel, -1);
        
        if ((input > -1) && (channel > -1))
        {
            CreateDigital(false, SHUTTEROPEN_IDX, input, channel, configString);
            m_HasShutterCommand = true;
        }
        else
        {
            m_OutDigitals[SHUTTERCLOSE_IDX] = 0x0;
            m_HasShutterCommand = false;
        }
        
        //SHUTTERCLOSE DigitalOut
        m_IniLib.GetConfigParamInt( configString, "DOUT2", &input, -1);
        m_IniLib.GetConfigParamInt( configString, "DOUT2CH", &channel, -1);
        if ((input > -1) && (channel > -1))
        {
            CreateDigital(false, SHUTTERCLOSE_IDX, input, channel, configString);
            m_HasOneShutterCommand = false;
        }
        else
        {
            m_OutDigitals[SHUTTERCLOSE_IDX] = 0x0;
            m_HasOneShutterCommand = true;
        }
        
        //FANIN DigitalOut
        m_IniLib.GetConfigParamInt( configString, "DOUT3", &input, -1);
        m_IniLib.GetConfigParamInt( configString, "DOUT3CH", &channel, -1);
        CreateDigital(false, FANIN_IDX, input, channel, configString);

        //FANOUT DigitalOut
        m_IniLib.GetConfigParamInt( configString, "DOUT4", &input, -1);
        m_IniLib.GetConfigParamInt( configString, "DOUT4CH", &channel, -1);
        if ((input > -1) && (channel > -1))
        {
            CreateDigital(false, FANOUT_IDX, input, channel, configString);
            m_HasOneFanCommand = false;
        }
        else
        {
            m_OutDigitals[FANOUT_IDX] = 0x0;
            m_HasOneFanCommand = true;
        }
        
        //air in (or room) temperature, no extra parameters given/needed
        m_TempControllers[TEMPAMB_IDX] = new CTempCtrl(0x0);
        
        //air out temperature, no extra parameters given/needed
        m_TempControllers[TEMPLMD_IDX] = new CTempCtrl(0x0);
        
        //PID
        CreatePID( configString );
        
        m_IniLib.GetConfigParamInt( configString, "TIMERID", &m_TimerID, -1);
        m_IniLib.GetConfigParamBool( configString, "SUMMER", &m_IsSummer, 0);
    
        if ( (m_TimerID > 0) && (timer != 0x0) )
        {
            m_Timer = timer;
            m_UseTimer = true;
        }
        else
        {
            m_UseTimer = false;
        }
    
        m_TypeOfTimerVal = TIMERVAL_DIGITAL;
        m_ControllerType = DEV_UTACTRL;
        
    }
}

///////////////////////////////////////////////////
//             Standard Destructor
///////////////////////////////////////////////////
CUtaCtrl::~CUtaCtrl()
{
    int i = 0;
    
    try
    {
        delete (m_Tag);
        
        delete (m_TempControllers[0]);
        delete (m_TempControllers[1]);
        delete (m_Analogs[0]);
        delete (m_Analogs[1]);
            
        for (i = 0; i < 4; i++)
        {
            delete (m_OutDigitals[i]);
        }
    }
    catch (exception &e)
    {
        cout << "Errore nella distruzione dell'oggetto UTACtrl : "<< e.what()<<endl;
    }

}

///////////////////////////////////////////////////
//              ChangeOutput
///////////////////////////////////////////////////
bool CUtaCtrl::ChangeOutput(int outIndex)
{
    return false;
}

///////////////////////////////////////////////////
//              InitOutput
///////////////////////////////////////////////////
bool CUtaCtrl::InitOutput(int outIndex)
{
    //TODO da inventarsi qualcosa... forse serve forse no
    return false;
}

///////////////////////////////////////////////////
//              SetOutput
///////////////////////////////////////////////////
bool CUtaCtrl::SetOutput(bool newState, int outIndex)
{
   
    return false;
}

///////////////////////////////////////////////////
//              Update
///////////////////////////////////////////////////
bool CUtaCtrl::Update(bool updateData)
{
    bool retVal = false;
    bool newState = false;
    
    bool tagPresent;
    
    tagPresent = m_Tag->GetState();

    if (tagPresent)
    {
        SetAllAOUT( ANALOG_OPEN );
        if (!CloseShutters())
        {
            PushError(AFOERROR_UTACTRL_UNABLE_TO_CLOSE_SHUTTER);
        }
        else
        {
            retVal = true;
        }
    }
    else
    {
        //TAG off, check for timers and shutters then work as a normal PIDLMD
        if ( m_UseTimer && IsTimerEnabled() )
        {
            newState = GetValFromTimer();
            
            if (newState)
            {   
                if ((m_ShutterOpen) && (!m_ShutterOpening) && (!m_ShutterClosing))
                {
                    //PID
                    retVal = m_PIDLMD->Update( updateData );
                }
                else 
                {
                    retVal = OpenShutters();
                    
                    if (!retVal)
                    {
                        PushError(AFOERROR_UTACTRL_UNABLE_TO_OPEN_SHUTTER);
                        retVal = false;
                    }
                }
            }
            else
            {
                if (m_ShutterOpen || m_ShutterOpening || m_ShutterClosing)
                {
                    retVal = CloseShutters();
                    
                    if (!retVal)
                    {
                        PushError( AFOERROR_UTACTRL_UNABLE_TO_CLOSE_SHUTTER );
                        retVal = false;
                    }
                }
                else
                {
                    retVal = true;
                }
            }
        }
        else
        {
            if (m_StartOn)
            {
                if ((m_ShutterOpen) && (!m_ShutterOpening) && (!m_ShutterClosing))
                {
                    //PID
                    retVal = m_PIDLMD->Update( updateData );
                }
                else if (!OpenShutters())
                {
                    PushError( AFOERROR_UTACTRL_UNABLE_TO_OPEN_SHUTTER );
                    retVal = false;
                }
                else
                {
                    retVal = true;
                }
            }
            else
            {
                if (m_ShutterOpen)
                {
                    if (!CloseShutters())
                    {
                        PushError( AFOERROR_UTACTRL_UNABLE_TO_CLOSE_SHUTTER );
                        retVal = false;
                    }
                    else
                    {
                        retVal = true;
                    }
                }
                else
                {
                    retVal = true;
                }
            }
        }
    }
        
    
    if (retVal)
    {
        ClearError();
    }
    else
    {
        AddError();
    }
    
    return retVal;
}

///////////////////////////////////////////////////
//              CreateDigital
///////////////////////////////////////////////////
bool CUtaCtrl::CreateDigital( bool isInput, int index, int input, int channel, const char* configString )
{
    char configBuffer[255];
    int invertout;
    
    memset (configBuffer, 0x0, 255*sizeof(char));
    if (isInput)
    {
        invertout = m_InvertDigital & 0x01;
    }
    else
    {
        invertout = (m_InvertDigital & (0x01<<(index+1)))>>(index+1);
    }

    if (isInput)
    {
        sprintf (configBuffer,"INPUT:%d,CHANNEL:%d,INVERTOUT:%d,IO:1,TIMERID:-1",input,channel,invertout);
        m_Tag = new CDigitalIO(configBuffer, 0x0);
    }
    else
    {
        sprintf (configBuffer,"INPUT:%d,CHANNEL:%d,INVERTOUT:%d,IO:0,TIMERID:-1",input,channel,invertout);
        m_OutDigitals[index] = new CDigitalIO(configBuffer,0x0);
    }
    
    return true;
}

///////////////////////////////////////////////////
//              CreatePID
///////////////////////////////////////////////////
bool CUtaCtrl::CreatePID( const char* configString )
{
    CString configuration;
    float parameter;
    int summer;
    
    configuration = "SETPOINT:";
    m_IniLib.GetConfigParamFloat( configString, "SP", &parameter, 20.0);
    configuration+=parameter;
    
    configuration+=", SETPOINTH:";
    m_IniLib.GetConfigParamFloat( configString, "SPH", &parameter, 35.0);
    configuration+=parameter;
    
    configuration+=", SETPOINTL:";
    m_IniLib.GetConfigParamFloat( configString, "SPL", &parameter, 15.0);
    configuration+=parameter;
    
    configuration+=", KP1:";
    m_IniLib.GetConfigParamFloat( configString, "KP1", &parameter, 15.0);
    configuration+=parameter;
    
    configuration+=", Tint1:";
    m_IniLib.GetConfigParamFloat( configString, "TI1", &parameter, 15.0);
    configuration+=parameter;
    
    configuration+=", Tder1:";
    m_IniLib.GetConfigParamFloat( configString, "TD1", &parameter, 15.0);
    configuration+=parameter;
    
    configuration+=", KP2:";
    m_IniLib.GetConfigParamFloat( configString, "KP2", &parameter, 15.0);
    configuration+=parameter;
    
    configuration+=", Tint2:";
    m_IniLib.GetConfigParamFloat( configString, "TI2", &parameter, 15.0);
    configuration+=parameter;
    
    configuration+=", Tder2:";
    m_IniLib.GetConfigParamFloat( configString, "TD2", &parameter, 15.0);
    configuration+=parameter;
    
    configuration+=", SUMMER:";
    m_IniLib.GetConfigParamInt( configString, "SUMMER", &summer, 0);
    configuration+=summer;
    
    m_PIDLMD = new CPIDLMD((char*)configuration.c_str());
    
    m_PIDLMD->SetReferenceNumbers( m_DeviceNumber, m_NetNumber);
    
    if (m_PIDLMD)
    {
        return true;
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////
//              FinalizeSetup
///////////////////////////////////////////////////
bool CUtaCtrl::FinalizeSetup( )
{
    bool retVal = false;
    
    if (m_Analogs[COLDBATTERY] == 0x0)
    {
        m_HasOneAnalogOut = true;
    }
    else
    {
        m_HasOneAnalogOut = false;
    }
    
    if ((m_HasOneAnalogOut) || (!m_IsSummer))
    {
        //Setup the PID
        m_PIDLMD->SetInputOutputDevices( m_TempControllers[TEMPAMB_IDX]->GetInputDevice(), m_TempControllers[TEMPLMD_IDX]->GetInputDevice(), m_Analogs[HEATBATTERY]);
    }
    else
    {
        m_PIDLMD->SetInputOutputDevices( m_TempControllers[TEMPAMB_IDX]->GetInputDevice(), m_TempControllers[TEMPLMD_IDX]->GetInputDevice(), m_Analogs[COLDBATTERY]);
    }
    
    //Write device informations inside the controllers
    m_Tag->SetReferenceNumbers( m_DeviceNumber, m_NetNumber);
    
    if (m_HasShutterCommand)
    {
        m_OutDigitals[SHUTTEROPEN_IDX]->SetReferenceNumbers( m_DeviceNumber, m_NetNumber);
        if (!m_HasOneShutterCommand)
        {
            m_OutDigitals[SHUTTERCLOSE_IDX]->SetReferenceNumbers( m_DeviceNumber, m_NetNumber);
        }
    }
    
    m_OutDigitals[FANIN_IDX]->SetReferenceNumbers( m_DeviceNumber, m_NetNumber);
    if (!m_HasOneFanCommand)
    {
        m_OutDigitals[FANOUT_IDX]->SetReferenceNumbers( m_DeviceNumber, m_NetNumber);
    }
    
    m_Analogs[HEATBATTERY]->SetReferenceNumbers( m_DeviceNumber, m_NetNumber);

    if (!m_HasOneAnalogOut)
    {
        m_Analogs[COLDBATTERY]->SetReferenceNumbers( m_DeviceNumber, m_NetNumber);
    }
    
    retVal = true;
    
    return retVal;
}

///////////////////////////////////////////////////
//              Init
///////////////////////////////////////////////////
bool CUtaCtrl::Init( )
{
    bool retVal = false;
    if (m_UseTimer && IsTimerEnabled() ) 
    {
        m_StartOn = GetValFromTimer();
    }
    
    //Close batteries valves
    SetAllAOUT(ANALOG_CLOSE);
    
    //Turn off FANS
    SetDOUT( FANIN_IDX,false );
    SetDOUT( FANOUT_IDX,false );
    
    //Close Shutters
    CloseShutters();
    
    retVal = true;
    
    return retVal;
}

///////////////////////////////////////////////////
//              SetDOUT
///////////////////////////////////////////////////
bool CUtaCtrl::SetDOUT( int index, bool val )
{
    bool retVal = false;
    e_AFOErrors error = OWERROR_NO_ERROR_SET;
    
    switch (index)
    {
        case SHUTTEROPEN_IDX: 
        {
            if (val)
            {
                error = AFOERROR_UTACTRL_UNABLE_TO_START_OPEN_SHUTTER_MOTOR;
            }
            else
            {
                error = AFOERROR_UTACTRL_UNABLE_TO_STOP_OPEN_SHUTTER_MOTOR;
            }
            
            retVal = m_OutDigitals[SHUTTEROPEN_IDX]->SetState( val);break;
        }
        
        case SHUTTERCLOSE_IDX: 
        {
            if (val)
            {
                error = AFOERROR_UTACTRL_UNABLE_TO_START_CLOSE_SHUTTER_MOTOR;
            }
            else
            {
                error = AFOERROR_UTACTRL_UNABLE_TO_STOP_CLOSE_SHUTTER_MOTOR;
            }
            
            if (m_HasOneShutterCommand)
            {
                retVal = true;
            }
            else
            {
                retVal = m_OutDigitals[SHUTTERCLOSE_IDX]->SetState(val);
            }
        };break;
        
        case FANIN_IDX: 
        {
            if (val)
            {
                error = AFOERROR_UTACTRL_UNABLE_TO_START_IN_FANS;
            }
            else
            {
                error = AFOERROR_UTACTRL_UNABLE_TO_STOP_IN_FANS;
            }

            retVal = m_OutDigitals[FANIN_IDX]->SetState( val);break;
        }
        
        case FANOUT_IDX:
        {
            if (val)
            {
                error = AFOERROR_UTACTRL_UNABLE_TO_START_OUT_FANS;
            }
            else
            {
                error = AFOERROR_UTACTRL_UNABLE_TO_STOP_OUT_FANS;
            }
                
            if (m_HasOneFanCommand)
            {
                retVal = true;
            }
            else
            {
                retVal = m_OutDigitals[FANOUT_IDX]->SetState( val);
            }
        };break;
        
        default: retVal = false;
    }
    
    if (!retVal)
    {
        PushError( error );
    }
        
    return retVal;
}

///////////////////////////////////////////////////
//              SetAOUT
///////////////////////////////////////////////////
bool CUtaCtrl::SetAOUT( int index, int val )
{
    bool retVal = false;
    e_AFOErrors error = OWERROR_NO_ERROR_SET;
    
    if (index == HEATBATTERY)
    {
        error = AFOERROR_UTACTRL_UNABLE_TO_SET_HEATBATTERY;
        retVal = m_Analogs[HEATBATTERY]->SetPosition( (uchar)(val));
    }
    else if (index == COLDBATTERY)
    {
        error = AFOERROR_UTACTRL_UNABLE_TO_SET_COLDBATTERY;
        if (m_HasOneAnalogOut)
        {
            retVal = true;
        }
        else
        {
            retVal = m_Analogs[COLDBATTERY]->SetPosition( (uchar)(val));
        }
    }
    
    if (!retVal)
    {
        PushError( error );
    }
         
    return retVal;
}

///////////////////////////////////////////////////
//              SetAllAOUT
///////////////////////////////////////////////////
bool CUtaCtrl::SetAllAOUT( int val )
{
    bool retVal = true;
    
    retVal = SetAOUT( HEATBATTERY, val) && SetAOUT( COLDBATTERY, val);
    
    return retVal;
}

///////////////////////////////////////////////////
//              OpenShutters
///////////////////////////////////////////////////
bool CUtaCtrl::OpenShutters( )
{
    bool retVal = false;
    time_t actTime = 0;
    
    //Get Time
    time(&actTime);
    
    if (!m_HasShutterCommand)
    {
        m_ShutterOpen = true;
        //Start Fans
        SetDOUT( FANIN_IDX, true);
        SetDOUT( FANOUT_IDX, true);
        return true;
    }
    
    if (((!m_ShutterOpen) && (!m_ShutterOpening)) || (m_ShutterClosing))
    {
        if (m_HasOneShutterCommand)
        {
            if (SetDOUT( SHUTTEROPEN_IDX, true ))
            {
                retVal = true;
            }
            else
            {
                //TODO messaggio errore
            }
        }
        else
        {
            if (SetDOUT( SHUTTERCLOSE_IDX, false))
            {
                if (SetDOUT( SHUTTEROPEN_IDX, true))
                {
                    retVal = true;
                }
                else
                {
                    //TODO messaggio errore
                }
            }
            else
            {
                //TODO messaggio errore
            }
            
        }
        
        if (retVal)
        {
            time (&m_TimeOfShutterMoving);
            m_ShutterOpening = true;
            m_ShutterClosing = false;
        }

    }
    else if ( (actTime > m_TimeOfShutterMoving + m_ShutterDelay) && (m_ShutterOpening))
    {
        StopShutters();
        m_ShutterOpen = true;
        m_ShutterOpening = false;
                    
        //Start Fans
        SetDOUT( FANIN_IDX, true);
        SetDOUT( FANOUT_IDX, true);
        
        retVal = true;
    }
    else
    {
        retVal = true;
    }
    
    return retVal;
}

///////////////////////////////////////////////////
//              CloseShutters
///////////////////////////////////////////////////
bool CUtaCtrl::CloseShutters( )
{
    bool retVal = false;
    time_t actTime = 0;
    
    //Get Time
    time(&actTime);
    if (!m_HasShutterCommand)
    {
        m_ShutterOpen = false;
        //Stop fans
        SetDOUT( FANIN_IDX, false);
        SetDOUT( FANOUT_IDX, false);
        return true;
    }
       
    if ( (m_ShutterOpen && (!m_ShutterClosing)) || (m_ShutterOpening) )
    {
        //Stop fans
        SetDOUT( FANIN_IDX, false);
        SetDOUT( FANOUT_IDX, false);
        
        if (m_HasOneShutterCommand)
        {
            if (SetDOUT( SHUTTEROPEN_IDX, false ))
            {
                retVal = true;
            }
            else
            {
                //TODO messaggio errore
            }
        }
        else
        {
            //TODO da mettere i messaggi di errore
            //First turn off the opening
            if (SetDOUT( SHUTTEROPEN_IDX, false))
            {
                if ( SetDOUT( SHUTTERCLOSE_IDX, true ))
                {
                    retVal = true;
                }
                else
                {
                    //TODO messaggio errore
                }
            }
            else
            {
                //TODO messaggio errore
            }
                    
        }
        
        if (retVal)
        {
            m_ShutterClosing = true;
            m_ShutterOpening = false;
            time (&m_TimeOfShutterMoving);
        }
    }
    else if (m_ShutterClosing && (actTime > m_TimeOfShutterMoving + m_ShutterDelay))
    {
        StopShutters();
        m_ShutterClosing = false;
        m_ShutterOpen = false;
        retVal = true;
    }
    else
    {
        retVal = true;
    }
    
    return retVal;
        
}

///////////////////////////////////////////////////
//              StopShutters
///////////////////////////////////////////////////
bool CUtaCtrl::StopShutters( )
{
    bool retVal = false;
    
    retVal = SetDOUT( SHUTTEROPEN_IDX, false);
    
    if (!m_HasOneShutterCommand)
    {
        retVal = SetDOUT( SHUTTERCLOSE_IDX, false) && retVal;
    }
    
    return retVal;
}

///////////////////////////////////////////////////
//              SetSummer
///////////////////////////////////////////////////
bool CUtaCtrl::SetSummer( bool isSummer )
{
    
    m_IsSummer = isSummer;
    m_PIDLMD->SetSummer( isSummer );
    
    if (!m_HasOneAnalogOut)
    {
        if (m_IsSummer)
        {
            m_PIDLMD->SetInputOutputDevices( m_TempControllers[TEMPAMB_IDX]->GetInputDevice(), m_TempControllers[TEMPLMD_IDX]->GetInputDevice(), m_Analogs[COLDBATTERY]);
            SetAOUT( HEATBATTERY, ANALOG_CLOSE);
        }
        else
        {
            m_PIDLMD->SetInputOutputDevices( m_TempControllers[TEMPAMB_IDX]->GetInputDevice(), m_TempControllers[TEMPLMD_IDX]->GetInputDevice(), m_Analogs[HEATBATTERY]);
            SetAOUT( COLDBATTERY, ANALOG_CLOSE);
        }
    }
    

    return true;
}

///////////////////////////////////////////////////
//              SetReferenceNumbers
///////////////////////////////////////////////////
void CUtaCtrl::SetReferenceNumbers( int devNumber, int netNumber )
{
    this->m_DeviceNumber = devNumber;
    this->m_NetNumber = netNumber;
    
    m_PIDLMD->SetReferenceNumbers( devNumber, netNumber);
}


