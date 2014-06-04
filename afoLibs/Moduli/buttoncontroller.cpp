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
#include "buttoncontroller.h"
#include "conewireengine.h"
#include "conewirenet.h"

CButtonController::CButtonController(const char* configString, CTimer* timer): CVMultiDIDO(configString, timer)
{   
    char buffer[255];

    memset (buffer, 0x0, 255*sizeof(char));
    
    if (m_IsRemoted)
    {
        m_ControllerType = DEV_REMOTEDIDO;
        if (configString != 0x0)
        {
            //Get If the controller has to issue the changedostate command or the setdigitaloutput
            m_IniLib.GetConfigParamBool( configString, "SENDCHANGESTATE", &m_RemoteChangeState, 1);
            
            m_IniLib.GetConfigParamInt( configString, "DEFAULTVAL", &m_JollyValue, 0);
        }
        
        m_IsJolly = false;
    }
    else
    {
        m_ControllerType = DEV_BUTTONCTRL;
    
        if (configString != 0x0)
        {
            //Get output channel
            m_IniLib.GetConfigParamString( configString, "OUTPUT", buffer, 255, "NA");
            
            if (!strcasecmp(buffer,"ALL"))
            {
                m_IsJolly = true;
            }
            else
            {
                m_IsJolly = false;
            }
            
            m_IniLib.GetConfigParamInt( configString, "DEFAULTVAL", &m_JollyValue, 0);
        }
    }
            
}

CButtonController::~CButtonController()
{
}


//////////////////////////////////////////////////////////////
//          Update
//////////////////////////////////////////////////////////////
bool CButtonController::Update( bool updateData )
{
    int newState = 0, actualState = 0;
    bool retVal = false;
    
    if ((m_IsRemoted) || (m_IsJolly))
    {
        return true;
    }
    
    if ( m_UseTimer && IsTimerEnabled())
    {

        //It is an output AND is under timer AND the timer is enabled
        newState = GetValFromTimer();
        if (m_UseCreativeTimer && m_InvertTimerValue)
        {
            newState = !newState;
        }
        actualState = m_OutDeviceStates[0];
        
        //Check if the states differ
        if ((m_UseCreativeTimer) && (newState == 1))
        {
            //Timer is "creative" and on: restore the output to the default value
            newState = m_TimerDefaultValue;
            
            if (actualState != newState)
            {
                retVal = SetOutput( newState );
            }
            else
            {
                retVal = true;
            }
        }
        else if ((m_UseCreativeTimer) && (newState == 0))
        {
            //Timer is "creative" and off: do not change the output
            retVal = true;
        }
        else if ((!m_UseCreativeTimer) && (newState != actualState))
        {
            retVal = SetOutput( newState );
        }
        else
        {
            return true;
        }
    }
    else
    {
         retVal = true;
    }
    
    return retVal;
}

//////////////////////////////////////////////////////////////
//          InitDevice
//////////////////////////////////////////////////////////////
bool CButtonController::InitOutput(int outIndex)
{
    if ((m_IsRemoted) || (m_IsJolly))
    {
        return true;
    }
    else
    {
        if (m_UseTimer && IsTimerEnabled() )
        {
            m_StartOn = GetValFromTimer();
        }
            
        return SetOutput( m_StartOn );
    }
}

//////////////////////////////////////////////////////////////
//          ChangeOutput
/////////////////////////////////////////////////////////////
bool CButtonController::ChangeOutput( int outIndex)
{
    if ((m_IsRemoted) || (m_IsJolly))
    {
        return false;
    }
    else
    {
        return ((CDS2408*)(m_OutDevices[0]))->ChangeOutput(m_OutChannels[0]);
    }
}
 
//////////////////////////////////////////////////////////////
//          SetOutput
/////////////////////////////////////////////////////////////
bool CButtonController::SetOutput( bool newState,int outIndex  )
{
    bool retVal = false;

    if ((m_IsRemoted) || (m_IsJolly))
    {
        return false;
    }
    
    if (m_IsOutputInverted[0])
    {
        newState = !newState;
    }
    
    if (m_OutDevices[0]->GetFamNum() == DS2408_FN)
    {
        retVal = ((CDS2408*)(m_OutDevices[0]))->SetLatchState( m_OutChannels[0], newState );

        //Wait a bit to not overload powerline
        msDelay(m_SwitchTime);
    }
    else if (m_OutDevices[0]->GetFamNum() == DS2405_FN)
    {
        //TODO: Da implementare
    }
    
    if (retVal)
    {
        if (m_IsOutputInverted[0])
        {
            m_OutDeviceStates[0] = !newState;
        }
        else
        {
            m_OutDeviceStates[0] = newState;
        }
        
        ClearError();
    }
    else
    {
        AddError();
    }

    return retVal;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CButtonController::Update2(bool updateData)
{
    int newState = 0, actualState = 0;
    bool retVal = false;
    COneWireEngine* engPtr = reinterpret_cast<COneWireEngine*>(m_EnginePtr);
    COneWireNet* netPtr = reinterpret_cast<COneWireNet*>(m_NetPtr);
    
    //Aggiorno ingressi e uscite
    UpdateInputsAndOutputs();
    
    if (GetActivity(false))
    {
        if ( m_DebugLevel == 3 )
        {
            cout << "UpdateDIDO -------- Attivita' rilevata sul dispositivo di indirizzo "<< m_Address <<endl;
        }
        
        if (m_IsRemoted)
        {
            Cmd com("DEVICE");
                    
            if (m_RemoteChangeState)
            {
                com.putValue("COMMAND","ChangeDOState");
            }
            else
            {
                com.putValue("COMMAND","SetDigitalOutput");
                com.putValue("STATE",m_JollyValue);
            }
    
            if (m_RemoteAddr > -1)
            {
                com.putValue("ADDRESS",m_RemoteAddr);
            }
            else if (m_RemoteNet > -1)
            {
                com.putValue("NET",m_RemoteNet);
            }
    
            if (m_DebugLevel)
            {
                cout << "CBUTTONCTRL -- Address:"<<m_Address<<" --- Messaggio in uscita:\n" << com.toString() << endl;
            }
    
            if (engPtr->CheckInterfacePortsForConnection())
            {
                engPtr->WriteOnOutputPorts(com.toString(),m_RemoteAddr, m_RemotePort);
            }
            
            retVal = true;
        }
        else if (m_IsJolly)
        {
            //Jolly device, apply changes to all devices belonging to the same NET
            netPtr->SetAllDigitalOutputs ( m_NetNumber-1, m_JollyValue );
            
            retVal = true;
        }
        else
        {
            retVal = ChangeOutput();
        }
    }
    else if ( m_UseTimer && IsTimerEnabled())
    {

        //It is an output AND is under timer AND the timer is enabled
        newState = GetValFromTimer();
        if (m_UseCreativeTimer && m_InvertTimerValue)
        {
            newState = !newState;
        }
        actualState = m_OutDeviceStates[0];
        
        //Check if the states differ
        if ((m_UseCreativeTimer) && (newState == 1))
        {
            //Timer is "creative" and on: restore the output to the default value
            newState = m_TimerDefaultValue;
            
            if (actualState != newState)
            {
                retVal = SetOutput( newState );
            }
            else
            {
                retVal = true;
            }
        }
        else if ((m_UseCreativeTimer) && (newState == 0))
        {
            //Timer is "creative" and off: do not change the output
            retVal = true;
        }
        else if ((!m_UseCreativeTimer) && (newState != actualState))
        {
            retVal = SetOutput( newState );
        }
        else
        {
            return true;
        }
    }
    else
    {
        retVal = true;
    }
    
    return retVal;
}


