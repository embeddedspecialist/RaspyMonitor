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
#include "stepdigitalout.h"
#include "conewireengine.h"

CStepDigitalOut::CStepDigitalOut(const char* configString, CTimer* timer): CVMultiDIDO(configString, timer)
{
    m_IsInitOK = false;
    m_ControllerType = DEV_STEPDIGITALOUT;
    m_IsStepDone = false;
    
    if (configString != 0x0)
    {
        m_IniLib.GetConfigParamBool(configString,"ACTIVITY",&m_WorkOnActivity, true);
    }
}


CStepDigitalOut::~CStepDigitalOut()
{
}


//////////////////////////////////////////////////////////////
//          Update
//////////////////////////////////////////////////////////////
bool CStepDigitalOut::Update( bool updateData )
{
    int newState = 0, actualState = 0;
    bool retVal = false;
    
    if (updateData)
    {
        //Read first the input registry
        m_InDeviceStates[0] = GetInputState( true );
    }
    else
    {
        m_InDeviceStates[0] = GetInputState( false );
    }
    
    if (( m_UseTimer && IsTimerEnabled()) && (m_InDeviceStates[0] != -1))
    {
        //It is an output AND is under timer AND the timer is enabled
        newState = GetValFromTimer();
        if (m_UseCreativeTimer && m_InvertTimerValue)
        {
            newState = !newState;
        }
            
        actualState = m_InDeviceStates[0];
                
        //Check if the states differ
        if ((m_UseCreativeTimer) && (newState == 1) && (!m_IsStepDone))
        {
            //Timer is "creative" and on: restore the output to the default value
            newState = m_TimerDefaultValue;
            if (actualState != newState)
            {
                retVal = SetOutput( newState, false );
                if (retVal)
                {
                    m_IsStepDone = true;
                }
            }
            else
            {
                retVal = true;
            }
        }
        else if ((m_UseCreativeTimer) && (newState == 0))
        {
            //Timer is "creative" and off: do not change the output
            m_IsStepDone = false;
            retVal = true;
        }
        else if (!m_UseCreativeTimer)
        {
            retVal = SetOutput( newState );
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
    
    return retVal;
}

//////////////////////////////////////////////////////////////
//          ChangeOutput
//////////////////////////////////////////////////////////////
bool CStepDigitalOut::ChangeOutput( int outIndex)
{
    bool retVal = false;
    
    m_InDeviceStates[0] = GetInputState( true );
    
    if (m_InDeviceStates[0] != -1)
    {
        retVal = SetOutput (!m_InDeviceStates[0], false);
    }
    
    return retVal;
}

//////////////////////////////////////////////////////////////
//          InitDevice
//////////////////////////////////////////////////////////////
bool CStepDigitalOut::InitOutput(int outIndex  )
{
    if ((m_UseTimer && IsTimerEnabled() ) && (!m_UseCreativeTimer))
    {
        m_StartOn = GetValFromTimer();
    }
        
    if (SetOutput( m_StartOn, true ))
    {
        m_IsInitOK = true;
        return true;
    }
    
    return false;
}

//////////////////////////////////////////////////////////////
//          SetOutput
//////////////////////////////////////////////////////////////
bool CStepDigitalOut::SetOutput( bool newState, bool update )
{
    bool retVal = false;
    int counter = 0;

//     cout << "m_IsOutputInverted[0]: " << m_IsInputInverted[0]<<endl;
    if (m_IsOutputInverted[0])
    {
        newState = !newState;
    }
    
    if (m_OutDevices[0]->GetFamNum() == DS2408_FN)
    {
        if ((update) || (!m_IsInitOK))
        {
            //Read first the input registry
            m_InDeviceStates[0] = GetInputState( true );

            if (m_InDeviceStates[0] == -1)
            {
                //TODO generazione errore e uscita
                return false;
            }
            else
            {
                m_IsInitOK = true;
            }
        }

//         cout << "Stato ingresso : " << m_InDeviceStates[0] << " Nuovo Stato: " << newState << endl;
        if (m_InDeviceStates[0] != newState)
        {
            retVal = ((CDS2408*)(m_OutDevices[0]))->SetLatchState( m_OutChannels[0], 0 );
            if (retVal)
            {
//                 cout << "Comando Dato" << endl;
                //Ho dato il comando
                msDelay( STEPTIME );
                retVal = false;
                while ((counter < 10) && (!retVal))
                {
                    //Provo fino a 10 volte
                    retVal = ((CDS2408*)(m_OutDevices[0]))->SetLatchState( m_OutChannels[0], 1 );
                    counter++;
                }
                    
//                 if (retVal)
//                     cout << "Comando Tolto" << endl;
            }
        }
        else
        {
            retVal = true;
        }
    }
    else if (m_OutDevices[0]->GetFamNum() == DS2405_FN)
    {
        //TODO: Da implementare
    }
    
    if (retVal)
    {
        //For step relais the output is always 0 (open)
        m_OutDeviceStates[0] = 0;
        
        ClearError();
    }
    else
    {
        AddError();
    }

    return retVal;
}
///////////////////////////////////////////////////////////////////////
bool CStepDigitalOut::Update2(bool updateData)
{
    int newState = 0, actualState = 0;
    bool retVal = false;
    COneWireEngine* engPtr = reinterpret_cast<COneWireEngine*>(m_EnginePtr);
    
    UpdateInputsAndOutputs();
    
    if (m_DebugLevel)
    {
        cout <<"STEPDIGITAL -- address:"<<m_Address<<" stato ingresso:"<<GetInputState(false)<<" stato uscita:"<<GetOutputState(false)<<endl;
    }
    
    if (m_WorkOnActivity)
    {
        if (GetActivity(updateData))
        {
            if ( m_DebugLevel == 3)
            {
                cout << "Tipo:StepDigitalOutput";
                cout << " Address: " << m_Address << " Comment : "<<m_Comment;
                cout << "  Attivita' rilevata  " << endl;cout.flush();
            }
    
            ChangeOutput();
        }
    }
    else
    {   
        if (updateData)
        {
            //Read first the input registry
            m_InDeviceStates[0] = GetInputState( true );
        }
        else
        {
            m_InDeviceStates[0] = GetInputState( false );
        }
        
        if (( m_UseTimer && IsTimerEnabled()) && (m_InDeviceStates[0] != -1))
        {
            //It is an output AND is under timer AND the timer is enabled
            newState = GetValFromTimer();
            if (m_UseCreativeTimer && m_InvertTimerValue)
            {
                newState = !newState;
            }
                
            actualState = m_InDeviceStates[0];
                    
            //Check if the states differ
            if ((m_UseCreativeTimer) && (newState == 1) && (!m_IsStepDone))
            {
                //Timer is "creative" and on: restore the output to the default value
                newState = m_TimerDefaultValue;
                if (actualState != newState)
                {
                    retVal = SetOutput( newState, false );
                    if (retVal)
                    {
                        m_IsStepDone = true;
                    }
                }
                else
                {
                    retVal = true;
                }
            }
            else if ((m_UseCreativeTimer) && (newState == 0))
            {
                //Timer is "creative" and off: do not change the output
                m_IsStepDone = false;
                retVal = true;
            }
            else if (!m_UseCreativeTimer)
            {
                retVal = SetOutput( newState );
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
    
    Cmd com("DEVICE");
    com.putValue("TYPE","DIDO");
    com.putValue("ADDRESS",m_Address);
    com.putValue("STATE",GetInputState(false));
    com.putValue("VAL",GetInputState(false));
    com.putValue("OUTSTATE",GetOutputState(false));
            
    if (engPtr->CheckInterfacePortsForConnection())
    {
        engPtr->WriteOnInterfacePorts(com.getXMLValue().c_str(), com.getXMLValue().size());
    }
    
    return retVal;   
}


