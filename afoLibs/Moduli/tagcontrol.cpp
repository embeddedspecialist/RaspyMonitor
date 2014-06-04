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
#include "tagcontrol.h"
#include "conewireengine.h"

CTAGControl::CTAGControl(const char* configString, CTimer* timer): CVMultiDIDO(configString, timer)
{
    m_ControllerType = DEV_TAGCTRL;
}


CTAGControl::~CTAGControl()
{
}

//////////////////////////////////////////////////////////////
//          InitOutput
/////////////////////////////////////////////////////////////
bool CTAGControl::InitOutput(int outIndex )
{
    if (m_IsRemoted)
    {
        return true;
    }

    if (m_IsOutputTimed)
    {
        //Se è temporizzato inizializzo l'uscita al valore dell'ingresso dopodichè gestirò solo le transizioni opposte
        m_StartOn = GetInputState(true);
    }
    else if (GetInputState( true ))
    {
        //Get the state of the TAG
        //09/2008 -- NON mi ricordo perche' c'e' questo if!!!
        m_StartOn = false;
    }
    else if (m_UseTimer && IsTimerEnabled() )
    {
        m_StartOn = GetValFromTimer();
    }
        
    return SetOutput( m_StartOn );
}
//////////////////////////////////////////////////////////////
//          Update
/////////////////////////////////////////////////////////////
bool CTAGControl::Update(bool updateData)
{
    int inputVal = 0;

    //Get the input
    inputVal = GetInputState( updateData );
    
    //If it is remoted we don't have to make anything
    if (m_IsRemoted)
    {
        return true;
    }
    else if (m_IsOutputTimed)
    {
        return UpdateTimedOut(inputVal);
    }

    if (m_UseTimer && IsTimerEnabled() )
    {
        return SetOutput( GetValFromTimer() );
    }
    else
    {
        //Get the state of the INPUT and copy it to the output
        return SetOutput(inputVal);
    }

}
//////////////////////////////////////////////////////////////
//          SetOutput
/////////////////////////////////////////////////////////////
bool CTAGControl::SetOutput( bool newState,int outIndex  )
{
    bool retVal = false;

    if (m_IsRemoted)
    {
        return false;
    }

    if (m_OutDeviceStates[0] == newState)
    {
        //TBR
//         cout << "Inutile aggiornare l'uscita del TAG: "<<m_Address<<endl;
        return true;
    }
    
    if (m_IsOutputInverted)
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
        if (m_IsOutputInverted)
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
////////////////////////////////////////////////////////////
bool CTAGControl::UpdateTimedOut(int inputVal)
{
    bool retVal;
    time_t actTime;
    int inputState;

    time(&actTime);

    //Prima controllo se ho avuto un errore nel precedente cambiamento di uscita
    if (m_PerformOutputChange)
    {
        if (!PerformOutputChange(!m_StartOn))
        {
            //TODO Generazione errore kosher
            cout << "Impossibile impostare l'uscita del TAG Controller "<< GetConfigFileDevIndex() << endl;
            return false;
        }
    }
    else if (!m_OutputRestored)
    {
        if (!RestoreOutput(m_StartOn))
        {
                 //TODO Generazione errore kosher
            cout << "Impossibile impostare l'uscita del TAG Controller "<< GetConfigFileDevIndex() << endl;
            return false;
        }
    }
        

    if (m_OutputTimerActivated)
    {
        //Check if enough time has passed
        if (actTime > m_OutputTimeOut + m_OutputChangeStartTime)
        {
            if (!RestoreOutput(m_StartOn))
            {
                 //TODO Generazione errore kosher
                cout << "Impossibile impostare l'uscita del TAG Controller "<< GetConfigFileDevIndex() << endl;   
            }
        }
        else
        {
            retVal = true;
        }
    }
    else
    {
        if (m_UseTimer && IsTimerEnabled() )
        {
            inputState = GetValFromTimer();
        }
        else
        {
            //Get the state of the INPUT
            inputState = inputVal;
        }
        
        if ((inputState>=0) && (inputState != m_StartOn))
        {
            //Devo eseguire una transizione
            if (SetOutput(inputState))
            {
                m_OutputTimerActivated = true;
                m_OutputChangeStartTime = actTime;
                retVal = true;
                m_PerformOutputChange = false;
            }
            else
            {
                //C'e' stato un errore, mi segno che al prossimo ciclo devo riprovare ad eseguire il cambio dell'uscita
                m_PerformOutputChange = true;
                //TODO Generazione errore kosher
                cout << "Impossibile impostare l'uscita del TAG Controller "<< GetConfigFileDevIndex() << endl;
            }
        }
    }

    return retVal;

}

///////////////////////////////////////////////////////////////
bool CTAGControl::Update2(bool updateData)
{
    int inputVal = 0, outputVal = 0;
    COneWireEngine* engPtr = reinterpret_cast<COneWireEngine*>(m_EnginePtr);

    //Aggiorno ingressi e uscite
    UpdateInputsAndOutputs();
    
    //Get the input
    inputVal  = GetInputState( updateData );
    outputVal = GetOutputState( updateData );
    
    if (m_DebugLevel==3)
    {
        cout << "Tipo: TAG Controller";
        cout << " Address: " << m_Address << " Comment : "<<m_Comment;
        cout << "  StatoIngresso:  " << inputVal<<endl;cout.flush();
    }
    
    //If it is remoted we don't have to make anything
    if (m_IsRemoted)
    {
        if (inputVal != outputVal)
        {
            Cmd com("DEVICE");
            com.putValue("COMMAND","SetDigitalOutput");
            com.putValue("STATE",inputVal);
            com.putValue("ADDRESS",m_RemoteAddr);

            if (m_DebugLevel)
            {
                cout << "TAGCTRL -- address: "<<m_Address<<" -- Messaggio in uscita:\n" << com.toString() << endl;
            }

            if (engPtr->CheckInterfacePortsForConnection())
            {
                engPtr->WriteOnOutputPorts(com.toString(),m_RemoteAddr, m_RemotePort);
            }
        }
        
        return true;
    }
    else if (m_IsOutputTimed)
    {
        return UpdateTimedOut(inputVal);
    }

    if (m_UseTimer && IsTimerEnabled() )
    {
        return SetOutput( GetValFromTimer() );
    }
    else
    {
        //Get the state of the INPUT and copy it to the output
        return SetOutput(inputVal);
    }
}

