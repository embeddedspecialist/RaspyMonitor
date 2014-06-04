

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
#include "digitalio.h"
#include "conewireengine.h"


///////////////////////////////////////////////////
//              STANDARD CONSTRUCTOR
///////////////////////////////////////////////////
CDigitalIO::CDigitalIO(CVDevice* device, const char* configString, CTimer *timer) 
    : CVDIDO(configString, timer)
{

    //Get infos for this class
    if (configString != 0x0 )
    {
        //Get if input or output
        m_IniLib.GetConfigParamBool( configString, "IO", &m_IsInput, false);
        //Get Start Value
        m_IniLib.GetConfigParamBool( configString, "STARTV", &m_StartOn, false);
        //Invert output
        m_IniLib.GetConfigParamBool( configString, "INVERTOUT", &m_IsOutputInverted, false);

        m_IniLib.GetConfigParamBool( configString, "STEP", &m_IsStep, false);
        
        m_IniLib.GetConfigParamInt( configString, "TIMERID", &m_TimerID, -1);
    
        if ( (m_TimerID > 0) && (timer != 0x0) )
        {
            m_Timer = timer;
            m_UseTimer = true;
        }
        else
        {
            m_UseTimer = false;
        }
        
        //State Check, if ON (default) the system will check the current state versus the recorded one, if OFF the system will
        //ignore if the current state differs from the recorded one, it is useful for managing remote controlled push buttons
        m_IniLib.GetConfigParamBool( configString, "STATECHECK", &m_StateCheck, true);
    }

    m_ControllerType = DEV_DIDO;
    m_DeviceType = DEV_DIDO;
    m_TypeOfTimerVal = TIMERVAL_DIGITAL;
    m_SwitchTime = 0;
    m_LastState = 0;

    m_IsInput = false;
            
}

CDigitalIO::CDigitalIO( const char* configString, CTimer * timer ) : CVDIDO(configString, timer)
{
        //Get infos for this class
    if (configString != 0x0 )
    {
        //Get if input or output
        m_IniLib.GetConfigParamBool( configString, "IO", &m_IsInput, false);
        //Get Start Value
        m_IniLib.GetConfigParamBool( configString, "STARTV", &m_StartOn, false);
        //Invert output
        m_IniLib.GetConfigParamBool( configString, "INVERTOUT", &m_IsOutputInverted, false);
        
        m_IniLib.GetConfigParamInt( configString, "TIMERID", &m_TimerID, -1);

        m_IniLib.GetConfigParamBool( configString, "STEP", &m_IsStep, false);
    
        if ( (m_TimerID > 0) && (timer != 0x0) )
        {
            m_Timer = timer;
            m_UseTimer = true;
        }
        else
        {
            m_UseTimer = false;
        }
        
        //State Check, if ON (default) the system will check the current state versus the recorded one, if OFF the system will
        //ignore if the current state differs from the recorded one, it is useful for managing remote controlled push buttons
        m_IniLib.GetConfigParamBool( configString, "STATECHECK", &m_StateCheck, true);
    }
        
    m_ControllerType = DEV_DIDO;
    m_TypeOfTimerVal = TIMERVAL_DIGITAL;
    m_SwitchTime = 0;
    m_LastState = 0;
}
///////////////////////////////////////////////////
//              STANDARD DESTRUCTOR
///////////////////////////////////////////////////
CDigitalIO::~CDigitalIO()
{
}


///////////////////////////////////////////////////
//              GetState
///////////////////////////////////////////////////
bool CDigitalIO::GetState(bool updateFirst )
{
    bool retVal = false;
    
    //Se non aggiorno tanto vale ritornare m_LastState
    if (!updateFirst)
    {
        return m_LastState;
    }
    
    if (m_Device->GetFamNum() == DS2408_FN)
    {
        if ( m_IsInput )
        {
            //If it is an input return the level because the latch is always on
//             retVal = ((CDS2408*)(m_Device))->GetChannelLevel( m_Channel, updateFirst );
            retVal = GetChannelLevel(updateFirst);
        }
        else
        {
            //if it is an output get the latch state (On or Off)
//             retVal = ((CDS2408*)(m_Device))->GetChannelState( m_Channel, updateFirst );
            retVal = GetChannelLatch(updateFirst);
        }

        m_LastState = retVal;
    }
    else if (m_Device->GetFamNum() == DS2405_FN)
    {
        //TODO: Da implementare
    }

        
    return retVal;
}

///////////////////////////////////////////////////
//              SetState
///////////////////////////////////////////////////
bool CDigitalIO::SetState( bool newState, bool updateFirst )
{
    bool retVal = false;
    
    //check if input
    if (!m_IsInput)
    {
        GetState(updateFirst);
        
        if (m_LastState != newState)
        {
            if (m_IsOutputInverted)
            {
                newState = !newState;
            }
            
            if (m_Device->GetFamNum() == DS2408_FN)
            {
                retVal = ((CDS2408*)(m_Device))->SetLatchState( m_Channel, newState );
    
                //Wait a bit to not overload powerline
                msDelay(m_SwitchTime);
            }
            else if (m_Device->GetFamNum() == DS2405_FN)
            {
                //TODO: Da implementare
            }
        
            if (retVal)
            {
                if (m_IsOutputInverted)
                {
                    m_LastState = !newState;
                }
                else
                {
                    m_LastState = newState;
                }
            }
        }
        else
        {
            retVal = true;
        }

        if (retVal)
        {
            ClearError();
        }
        else
        {
            AddError();
        }
    }
    
    return retVal;
}

///////////////////////////////////////////////////
//              InitDevice
///////////////////////////////////////////////////
bool CDigitalIO::InitDevice( )
{
    int startState = 0;

    if (m_IsInput)
    {
        return true;
    }
        

    //Se controlla un passo passo stacca solo il contatto
    if (m_IsStep)
    {
        return ((CDS2408*)(m_Device))->SetLatchState( m_Channel, 1 );
    }
    
    if (m_UseTimer && IsTimerEnabled() )
    {
        startState = GetValFromTimer();
    }
    else
    {
        startState = m_StartOn;
    }
        
    return SetState(startState);
}

///////////////////////////////////////////////////
//              Update
///////////////////////////////////////////////////
bool CDigitalIO::Update( bool updateData )
{
    int newState;
    bool retVal = false;

    //Aggiorno gli stati interni se sono un input
    if (m_IsInput)
    {
        m_LastState = GetChannelLevel(updateData);
    }
    
    if (!m_IsActive)
    {
        //Se non è attivo lo spengo
        //SetState(0, updateData);
            
        return true;
    }

    //TODO da inserire statecheck
    //Check if we are a digital output
    if ( (!m_IsInput) && (m_UseTimer) && IsTimerEnabled())
    {
        //It is an output AND is under timer AND the timer is enabled
        newState = GetValFromTimer();
                
        if (m_LastState != newState)
        {
            retVal = SetState( newState );
        }
        else
        {
            retVal = true;
        }
    }
    else
    {
        //By default return true if the timer is not active
        retVal = true;
    }
    
    return retVal;
}

///////////////////////////////////////////////////
//              InvertOutput
///////////////////////////////////////////////////
bool CDigitalIO::ChangeOutput( )
{
    bool retVal = false;
    int counter = 0 ;
    
    if (!m_IsInput)
    {
        if (m_IsStep)
        {
            retVal = ((CDS2408*)(m_Device))->SetLatchState( m_Channel, 0 );
            if (retVal)
            {
//                 cout << "Comando Dato" << endl;
                //Ho dato il comando
                msDelay( 100 );
                retVal = false;
                while ((counter < 10) && (!retVal))
                {
                    //Provo fino a 10 volte
                    retVal = ((CDS2408*)(m_Device))->SetLatchState( m_Channel, 1 );
                    counter++;
                }
                    
//                 if (retVal)
//                     cout << "Comando Tolto" << endl;
            }
        }
        else
        {
            if (((CDS2408*)(m_Device))->ChangeOutput(m_Channel))
            {
                this->m_LastState = !this->m_LastState;
                retVal = true;
            }
        }
    }
    
    return retVal;
}
////////////////////////////////////////////////////////////////////
bool CDigitalIO::IsDataAligned(bool updateData)
{
    if (m_IsInput)
    {
        //Non ha senso fare il controllo sugli ingressi
        return true;
    }

    return (m_LastState == GetChannelLatch(updateData));
}
////////////////////////////////////////////////////////////////////
bool CDigitalIO::Update2(bool updateData)
{
    int newState;
    bool retVal = false;
    COneWireEngine *engPtr = reinterpret_cast<COneWireEngine*>(m_EnginePtr);

    GetStatusRegister(0x0, false);
    
    //Aggiorno gli stati interni se sono un input
    if (m_IsInput)
    {
        m_LastState = GetChannelLevel(updateData);
        retVal = true;
    }
    else
    {
        if (!m_IsActive)
        {
            //Se non è attivo lo spengo
            //SetState(0, updateData);

            return true;
        }

        //Qui controllo lo stato attuale e quello che dovrebbe essere...
        if (m_StateCheck && (!IsDataAligned(false)) )
        {
            cout << "Indirizzo DIDO : "<< m_Address << " Stato Interno: "<<m_LastState<<" Stato reale : "<< !m_LastState << " Reimposto lo stato"<< endl;
            SetState(m_LastState, true);
        }

        //Check if we are a digital output
        if ( (!m_IsInput) && (m_UseTimer) && IsTimerEnabled())
        {
            //It is an output AND is under timer AND the timer is enabled
            newState = GetValFromTimer();

            if (m_LastState != newState)
            {
                retVal = SetState( newState );
                if (retVal)
                {
                    m_LastState = newState;
                }
            }
            else
            {
                retVal = true;
            }
        }
        else
        {
            //By default return true if the timer is not active
            retVal = true;
        }
    }
    
    if (retVal)
    {
        if (m_DebugLevel)
        {
            cout << "DIDO Indirizzo "<<m_Address<<" Stato: "<<m_LastState<<" Commento:"<<m_Comment<<endl;
        }

         m_Data.floatData[0] = m_LastState;
         m_Data.isValid = true;
        //Per questioni di prestazioni compongo il messaggio solo e unicamente se ci sono delle porte connesse
        
        Cmd com("DEVICE");
        com.putValue("TYPE","DIDO");
        com.putValue("ADDRESS",m_Address);
        com.putValue("STATE",m_LastState);
        
        if (engPtr->CheckInterfacePortsForConnection())
        {
            engPtr->WriteOnInterfacePorts(com.toString().c_str(), (int)com.toString().size());
        }
    }
    else
    {
        m_Data.isValid = false;
    }
    
    return retVal;   
}



