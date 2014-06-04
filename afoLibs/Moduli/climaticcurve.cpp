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
#include "climaticcurve.h"
#include "conewireengine.h"
#include "conewirenet.h"

CClimaticCurve::CClimaticCurve(const char *configString,CTimer *timer)
    : CVCoordinator(configString)
{
    if (configString != 0x0)
    {
        m_IniLib.GetConfigParamInt( configString, "CONFIGID", &m_ConfigID, -1);
        m_IniLib.GetConfigParamInt( configString, "TIMERID", &m_TimerID, -1);
    }

    if ( (m_TimerID > 0) && (timer != 0x0) )
    {
        m_Timer = timer;
        m_UseTimer = true;
    }
    else
    {
        m_UseTimer = false;
    }

    m_NewSetpoint = 20.0;
    m_TempExt = 0x0;

    m_TypeOfTimerVal = TIMERVAL_DIGITAL;
    m_ControllerType = DEV_CLIMATICOORD;

    m_TempExt = 0x0;
    m_DigitalIn = 0x0;
    m_SetpointOff = m_SetpointOn = 20.0;

    m_Type = "compext";

    //Risponde da solo ai comandi
    m_CodeRevision = 1;
}


CClimaticCurve::~CClimaticCurve()
{
}
////////////////////////////////////////////////
//             ConnectControllers
///////////////////////////////////////////////
bool CClimaticCurve::ConnectControllers()
{
    CIniFileHandler iniFileReader;
    COneWireNet* netPtr = reinterpret_cast<COneWireNet*> ( m_NetPtr );
    CString configIDString;
    int netIndex, ctrlIndex,ctrlAddress, nOfPids;
    
    int nOfPoints;

    //Esco subito perchè non ho il puntatore alla NET
    if ( netPtr == 0x0 )
    {
        cout << "Attenzione il coordinatore di indirizzo: " << m_Address << " NON è connesso al conewireNET"<<endl;
        return false;
    }

    //Leggo dal file di configurazione (changeover.ini) tutte le chiavi e le zone e le istanzio, programmo i moduli di controllo accessi
    if ( !iniFileReader.Load ( "./climateCurve.ini" ) )
    {
        cout << "Attenzione: impossibile aprire il file climateCurve.ini"<<endl;
        return false;
    }

    if (m_ConfigID <= 0)
    {
        cout << "Attenzione il coordinatore di indirizzo: " << m_Address << " NON ha il campo corretto in CONFIGID"<<endl;
        return false;
    }
    else
    {
        configIDString="CLIMATE";
        configIDString+=m_ConfigID;

        if (!iniFileReader.ExistSection(configIDString))
        {
            cout << "Attenzione coordinatore di indirizzo: " << m_Address << " in climateCurve.ini NON esiste la sezione"<<configIDString<<endl;
            return false;
        }
    }

    //Devo discernere che tipo di controllo devo eseguire
    m_Type = iniFileReader.GetString("Type",configIDString,"compExt");

    cout << "ClimateCurve indirizzo "<< m_Address<<". Il tipo è:" << m_Type<<endl;

    m_Type.ToLower();

    if (m_Type == "compext")
    {
        //Connetto la temperatura esterna
        //TODO controllo che sia davvero un controllore di temperatura
        ctrlAddress = iniFileReader.GetInt("TempExt",configIDString,-1);
        netIndex = netPtr->GetNetByMemoryAddress(ctrlAddress);
        ctrlIndex = netPtr->GetDeviceIndexByMemoryAddress(netIndex, ctrlAddress);

        if ( (netIndex<0) || (ctrlIndex < 0))
        {
            cout << "Attenzione: coordinatore di indirizzo: " << m_Address<< " campo TempExt NON valido in climateCurve.ini\n IMPOSSIBILE CONTINUARE"<<endl;
            msDelay(1000);
            return false;
        }

        m_TempExt = ((CTempCtrl*)(netPtr->GetNetHandler ( netIndex )->CtrlList[ctrlIndex]));

        //Carico i punti della curva climatica
        nOfPoints = iniFileReader.GetInt("nOfPoints", configIDString,0);

        for (int i = 0; i < nOfPoints; i++)
        {
            CString pointString = "Point";
            CString pointConfig;
            t_ClimaticPoint newPoint;

            pointString+=(i+1);

            pointConfig = iniFileReader.GetString(pointString, configIDString, "");

            if (pointConfig.size() == 0)
            {
                cout << "Attenzione: coordinatore di indirizzo: " << m_Address<< " campo "<<pointString<<" NON valido in climateCurve.ini"<<endl;
                msDelay(1000);
                continue;
            }

            //TODO da mettere protezione nel caso in cui i sottocampi non siano validi
            m_IniLib.GetConfigParamFloat(pointConfig.c_str(),"TEXT",&(newPoint.tExt),0.0);
            m_IniLib.GetConfigParamFloat(pointConfig.c_str(),"TMND",&(newPoint.tMnd),0.0);

            m_ClimaticPoints.push_back(newPoint);
        }
    }
    else
    {
        //Carico i setpoint
        m_SetpointOff = iniFileReader.GetFloat("SPOff",configIDString,20.0);
        m_SetpointOn = iniFileReader.GetFloat("SPOn",configIDString,20.0);

        if ((m_Type == "digital") || (m_Type == "mixedmode"))
        {
            //carico il digitale
            ctrlAddress = iniFileReader.GetInt("DigitalIN",configIDString,-1);
            netIndex = netPtr->GetNetByMemoryAddress(ctrlAddress);
            ctrlIndex = netPtr->GetDeviceIndexByMemoryAddress(netIndex, ctrlAddress);

            if ( (netIndex<0) || (ctrlIndex < 0))
            {
                cout << "Attenzione: coordinatore di indirizzo: " << m_Address<< " campo DigitalIN NON valido in climateCurve.ini\n IMPOSSIBILE CONTINUARE"<<endl;
                msDelay(1000);
                return false;
            }

            m_DigitalIn = ((CDigitalIO*)(netPtr->GetNetHandler ( netIndex )->CtrlList[ctrlIndex]));

        }
    }
    
    nOfPids = iniFileReader.GetInt("nOfPid",configIDString,0);
    
    //Carico i controllori
    for (int i = 0; i < nOfPids; i++)
    {
        CString pidString ="PID";

        pidString+=(i+1);
        
        ctrlAddress = iniFileReader.GetInt(pidString,configIDString,-1);
        netIndex = netPtr->GetNetByMemoryAddress(ctrlAddress);
        ctrlIndex = netPtr->GetDeviceIndexByMemoryAddress(netIndex, ctrlAddress);

        if ( (netIndex<0) || (ctrlIndex < 0))
        {
            cout << "Attenzione: coordinatore di indirizzo: " << m_Address<< " campo "<<pidString<<" NON valido in climateCurve.ini"<<endl;
            msDelay(1000);
            continue;
        }

        m_ControllerList.push_back(netPtr->GetNetHandler ( netIndex )->CtrlList[ctrlIndex]);
    }

    
        
    return true;
}
////////////////////////////////////////////////////////////////////////////////
bool CClimaticCurve::ExecCommand(CXMLUtil* xmlUtil)
{
    bool retVal = false;
    e_CommandTypes command = (e_CommandTypes)ParseCommand(xmlUtil);
    COneWireEngine *engPtr = reinterpret_cast<COneWireEngine*>(m_EnginePtr);
    CIniFileHandler iniFile;

    switch (command)
    {
        case COMM_SETCLIMATICURVE:
        {
            int nOfPoints = xmlUtil->GetIntParam("NOFPOINTS");

            m_ClimaticPoints.clear();

            for (int i = 0; i < nOfPoints; i++)
            {
                t_ClimaticPoint newPoint;
                float tExt, tMnd;
                CString pointStr = "POINT";
                CString valString;

                pointStr+=(i+1);
                valString = xmlUtil->GetStringParam(pointStr);

                if (sscanf(valString.c_str(), "%f:%f", &tExt, &tMnd) == 2)
                {
                    newPoint.tExt = tExt;
                    newPoint.tMnd = tMnd;
                    m_ClimaticPoints.push_back(newPoint);
                }
            }

            SaveClimaticCurve();
            retVal = true;

        };break;
        case COMM_GETCLIMATICCURVE:
        {
            //Il formato è il seguente:
            //NOFPOINTS= POINT1=text:tmnd

            Cmd com("DEVICE");
            com.putValue("TYPE", "ClimaticCurve");
            com.putValue("ADDRESS", m_Address);
            com.putValue("NOFPOINTS",(unsigned int)m_ClimaticPoints.size());
            for (unsigned int i = 0; i < m_ClimaticPoints.size(); i++)
            {
                CString pointStr = "POINT";
                CString valString = "";
                pointStr += (i+1);
                valString+=m_ClimaticPoints.at(i).tExt;
                valString+=":";
                valString+=m_ClimaticPoints.at(i).tMnd;
                com.putValue(pointStr,valString);
            }
            

            if (engPtr->CheckInterfacePortsForConnection())
            {
                engPtr->WriteOnInterfacePorts(com.toString().c_str(), com.toString().size());
            }

            retVal = true;

        };break;
        case COMM_ACTIVATECONTROLLER:
        {
            if (xmlUtil->GetBoolParam("ACTIVATE"))
            {
                m_IsActive = true;
            }
            else
            {
                m_IsActive = false;
            }
        };break;
        default:break;
    }

    return retVal;
}
/////////////////////////////////////////////////////////////////////////
void CClimaticCurve::SaveClimaticCurve()
{
    CIniFileHandler iniFileReader;
    CString configIDString="CLIMATE";
    configIDString+=m_ConfigID;

    if (!iniFileReader.Load("./climateCurve.ini"))
    {
        return;
    }

    iniFileReader.SetInt("nOfPoints",m_ClimaticPoints.size(),"",configIDString);

    for (unsigned int i = 0; i < m_ClimaticPoints.size(); i++)
    {
        CString pointStr = "Point";
        pointStr+=(i+1);

        CString valString = "TEXT:";
        valString+=m_ClimaticPoints.at(i).tExt;
        valString+=",TMND:";
        valString+=m_ClimaticPoints.at(i).tMnd;

        iniFileReader.SetValue(pointStr,valString,"",configIDString);
    }

    iniFileReader.Save();
}
//////////////////////////////////////////////////////////////////////////
bool CClimaticCurve::Update(bool updateData)
{
     return Update2(updateData);
}
//////////////////////////////////////////////////////////////////////////
bool CClimaticCurve::Update2(bool updateData)
{
    if (!m_IsActive) 
    {
        return true;
    }

    if (m_Type == "compext")
    {
        UpdateCompExtMode();
    }
    else if (m_Type == "timer")
    {
        UpdateTimerMode();
    }
    else if (m_Type == "digital")
    {
        UpdateDigitalMode();
    }
    else if (m_Type == "mixedmode")
    {
        UpdateMixedMode();
    }

    return true;
}
//////////////////////////////////////////////////////////////////////////
bool CClimaticCurve::UpdateSetpoint(float newSetpoint)
{
    for (unsigned int i = 0; i < m_ControllerList.size(); i++)
    {
        if (m_ControllerList[i] == 0x0)
        {
            continue;
        }
        
        if (m_ControllerList[i]->GetControllerType() == DEV_PIDSIMPLE)
        {
            if (((CPIDSimple*)(m_ControllerList[i]))->GetSetPoint() != newSetpoint)
            {
                ((CPIDSimple*)(m_ControllerList[i]))->SetSetPoint(&newSetpoint);
            }
        }
        else if (m_ControllerList[i]->GetControllerType() == DEV_C3POINT)
        {
            if (((C3PointCtrl*)(m_ControllerList[i]))->m_Setpoint != newSetpoint)
            {
                ((C3PointCtrl*)(m_ControllerList[i]))->m_Setpoint = newSetpoint;
            }
        }
        else if (m_ControllerList[i]->GetControllerType() == DEV_AIAO)
        {
            if (newSetpoint > 10.0)
            {
                newSetpoint = 10.0;
            }
            else if (newSetpoint < 0.0)
            {
                newSetpoint = 0.0;
            }

            ((CAnalogIO*)(m_ControllerList[i]))->SetVOutput(newSetpoint);
        }



        //TODO da riempire con gli altri dispositivi
    }

    return true;
}
///////////////////////////////////////////////////////////////////////////////////
bool CClimaticCurve::UpdateCompExtMode()
{
    float tExt;
    unsigned int tempPointIdx;
    int nOfPoints = m_ClimaticPoints.size();
    COneWireEngine* engPtr = reinterpret_cast<COneWireEngine*>(m_EnginePtr);

    if (nOfPoints == 0)
    {
        return true;
    }
    
    if (m_TempExt == 0x0)
    {
        return false;
    }

    //Controllo la tempext in che fascia si trova e interpolo per avere il setpoint
    tExt = m_TempExt->GetLastTemp();
    
    if (tExt < -50.0)
    {
        //la temperatura non e' ancora aggiornata, esco
        return true;
    }

    if (tExt < m_ClimaticPoints[0].tExt)
    {
        m_NewSetpoint = m_ClimaticPoints[0].tMnd;
    }
    else if (tExt > m_ClimaticPoints[nOfPoints-1].tExt)
    {
        m_NewSetpoint = m_ClimaticPoints[nOfPoints-1].tMnd;
    }
    else
    {
        //Qui devo ciclare per vedere tra che punti della curva sono
        for (tempPointIdx = 0; tempPointIdx<m_ClimaticPoints.size()-1;tempPointIdx++)
        {
            if ((tExt >= m_ClimaticPoints[tempPointIdx].tExt) &&
                (tExt <= m_ClimaticPoints[tempPointIdx+1].tExt)
               )
            {
                float x1,y1,x2,y2;

                x1 = m_ClimaticPoints[tempPointIdx].tExt;
                y1 = m_ClimaticPoints[tempPointIdx].tMnd;

                x2 = m_ClimaticPoints[tempPointIdx+1].tExt;
                y2 = m_ClimaticPoints[tempPointIdx+1].tMnd;

                //Punto trovato, interpolo per trovare il setpoint
                m_NewSetpoint = ((y2-y1)/(x2-x1))*tExt + y1 - ((y2-y1)/(x2-x1))*x1;
            }
        }
    }

    UpdateSetpoint(m_NewSetpoint);

    if (m_DebugLevel)
    {
        cout << "ClimaticCoord -- Indirizzo: "<<m_Address<<" Text:"<<tExt<<" Setpoint:"<<m_NewSetpoint<<endl;
    }

    //Qui lancio i messaggi
    if (engPtr->CheckInterfacePortsForConnection())
    {
        Cmd com("DEVICE");
        com.putValue("TYPE","ClimaticCurve");
        com.putValue("ADDRESS",m_Address);
        com.putValue("TEXT",m_TempExt->GetLastTemp());
        com.putValue("SETPOINT",m_NewSetpoint);
        com.putValue("ISACTIVE",m_IsActive);
        com.putValue("ISON",m_IsOn);


        engPtr->WriteOnInterfacePorts(com.toString().c_str(), com.toString().size());
    }

    return true;
}
///////////////////////////////////////////////////////////////////////////////////
bool CClimaticCurve::UpdateTimerMode()
{
    int newState = 1;
    COneWireEngine* engPtr = reinterpret_cast<COneWireEngine*>(m_EnginePtr);
    float setpointToApply = 0.0;

    if ((m_UseTimer) && IsTimerEnabled())
    {
        //It is an output AND is under timer AND the timer is enabled
        newState = GetValFromTimer();
    }

    if (newState)
    {
        setpointToApply = m_SetpointOn;
    }
    else
    {
        setpointToApply = m_SetpointOff;
    }

    UpdateSetpoint(setpointToApply);
    m_NewSetpoint = setpointToApply;

    if (m_DebugLevel)
    {
        cout << "ClimaticCoord -- Indirizzo: "<<m_Address<<" Text:NA Setpoint:"<<m_NewSetpoint<<endl;
    }

    //Qui lancio i messaggi
    if (engPtr->CheckInterfacePortsForConnection())
    {
        Cmd com("DEVICE");
        com.putValue("TYPE","ClimaticCurve");
        com.putValue("ADDRESS",m_Address);
        com.putValue("TEXT","NA");
        com.putValue("SETPOINT",m_NewSetpoint);
        com.putValue("ISACTIVE",m_IsActive);
        com.putValue("ISON",m_IsOn);


        engPtr->WriteOnInterfacePorts(com.toString().c_str(), com.toString().size());
    }

    return true;
}
///////////////////////////////////////////////////////////////////////////////////
bool CClimaticCurve::UpdateDigitalMode()
{
    t_DataVal newState;
    COneWireEngine* engPtr = reinterpret_cast<COneWireEngine*>(m_EnginePtr);
    float setpointToApply = 0.0;

    newState = m_DigitalIn->GetDataStructure();

    if (!newState.isValid)
    {
        return true;
    }

    if (newState.floatData[0] == 1.0)
    {
        setpointToApply = m_SetpointOn;
    }
    else
    {
        setpointToApply = m_SetpointOff;
    }

    UpdateSetpoint(setpointToApply);
    m_NewSetpoint = setpointToApply;

    if (m_DebugLevel)
    {
        cout << "ClimaticCoord -- Indirizzo: "<<m_Address<<" Text:NA Setpoint:"<<m_NewSetpoint<<endl;
    }

    //Qui lancio i messaggi
    if (engPtr->CheckInterfacePortsForConnection())
    {
        Cmd com("DEVICE");
        com.putValue("TYPE","ClimaticCurve");
        com.putValue("ADDRESS",m_Address);
        com.putValue("TEXT","NA");
        com.putValue("SETPOINT",m_NewSetpoint);
        com.putValue("ISACTIVE",m_IsActive);
        com.putValue("ISON",m_IsOn);


        engPtr->WriteOnInterfacePorts(com.toString().c_str(), com.toString().size());
    }

    return true;
}
///////////////////////////////////////////////////////////////////////////////////
bool CClimaticCurve::UpdateMixedMode()
{
    int timerState = 1;
    t_DataVal digitalData;
    COneWireEngine* engPtr = reinterpret_cast<COneWireEngine*>(m_EnginePtr);
    float setpointToApply = 0.0;

    if ((m_UseTimer) && IsTimerEnabled())
    {
        //It is an output AND is under timer AND the timer is enabled
        timerState = GetValFromTimer();
    }

    digitalData = m_DigitalIn->GetDataStructure();

    if (!digitalData.isValid)
    {
        return true;
    }

    if (((bool)timerState) && ((bool)digitalData.floatData[0]))
    {
        setpointToApply = m_SetpointOn;
    }
    else
    {
        setpointToApply = m_SetpointOff;
    }

    UpdateSetpoint(setpointToApply);
    m_NewSetpoint = setpointToApply;


    if (m_DebugLevel)
    {
        cout << "ClimaticCoord -- Indirizzo: "<<m_Address<<" Text:NA Setpoint:"<<m_NewSetpoint<<endl;
    }
    
    //Qui lancio i messaggi
    if (engPtr->CheckInterfacePortsForConnection())
    {
        Cmd com("DEVICE");
        com.putValue("TYPE","ClimaticCurve");
        com.putValue("ADDRESS",m_Address);
        com.putValue("TEXT","NA");
        com.putValue("SETPOINT",m_NewSetpoint);
        com.putValue("ISACTIVE",m_IsActive);
        com.putValue("ISON",m_IsOn);


        engPtr->WriteOnInterfacePorts(com.toString().c_str(), com.toString().size());
    }

    return true;
}
