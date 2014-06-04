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
 
#include "remotedido.h"
#include "conewireengine.h"

CRemoteDIDO::CRemoteDIDO(const char* configString, CTimer *timer)
    : CVMultiDIDO(configString, timer)
{
    m_ControllerType = DEV_REMOTEDIDO;
    
    //Get If the controller has to issue the changedostate command or the setdigitaloutput
    m_IniLib.GetConfigParamBool( configString, "SENDCHANGESTATE", &m_RemoteChangeState, true);
            
    m_IniLib.GetConfigParamInt( configString, "DEFAULTVAL", &m_JollyValue, 0);
}


CRemoteDIDO::~CRemoteDIDO()
{
}

bool CRemoteDIDO::Update( bool updateData )
{
    return true;
}

bool CRemoteDIDO::Update2(bool updateData)
{
    COneWireEngine* engPtr = reinterpret_cast<COneWireEngine*>(m_EnginePtr);
    //Recupero il registro
    UpdateInputsAndOutputs();
    
    //Controllo l'attività
    if (GetActivity(false,0))
    {
        Cmd com("DEVICE");
        
        if (m_RemoteChangeState)
        {
            com.putValue("COMMAND","ChangeDOState");
        }
        else
        {
            com.putValue("COMMAND","SetDigitalOutput");
        }
        
        com.putValue("STATE",m_JollyValue);
        
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
            cout << "REMOTEDIDO Address:" << m_RemoteAddr<<" -- Rilevata attività -- Messaggio in uscita:\n" << com.toString() << endl;
        }

        if (engPtr->CheckInterfacePortsForConnection())
        {
            engPtr->WriteOnOutputPorts(com.toString(),m_RemoteAddr, m_RemotePort);
        }
    }

    return true;
}




