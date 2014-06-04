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
 #include "c3pointctrl.h"

#include "conewirenet.h"
#include "conewireengine.h"


C3PointCtrl::C3PointCtrl(const char* configString, CTimer * timer )
    : CVController(configString)
{
    m_ControllerType = DEV_C3POINT;
    m_TypeOfTimerVal = TIMERVAL_DIGITAL;

    if (configString != 0x0)
    {
        m_IniLib.GetConfigParamFloat(configString,"SETPOINT",&m_Setpoint, 20.0);
        m_IniLib.GetConfigParamBool(configString, "SUMMER", &m_IsSummer, true);
        m_IniLib.GetConfigParamInt( configString, "TIMERID", &m_TimerID, -1);
        m_IniLib.GetConfigParamInt( configString, "MOVETIMEOUT", &m_MovementTimeOut, 300);
        m_IniLib.GetConfigParamFloat(configString,"NULLZONE",&m_NullZoneAmplitude, 1.0);
        m_IniLib.GetConfigParamBool(configString,"LMD",&m_IsLMD,false);

        if (m_IsLMD)
        {
            m_IniLib.GetConfigParamFloat(configString, "SPH", &m_SetpointH, 35.0);
            m_IniLib.GetConfigParamFloat(configString, "SPL", &m_SetpointL, 16.0);
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
    }

    m_IsFullClosed = m_IsFullOpen = false;
    m_TimeOfClosing = 0;
    m_TimeOfOpening = 0;
    m_ControllerStatus = STOPPED;
}


C3PointCtrl::~C3PointCtrl()
{
}
////////////////////////////////////////////////////////
bool C3PointCtrl::SetInputDevice(CTempCtrl * tempCtrl, bool lmdDevice)
{
    if (lmdDevice)
    {
        if (tempCtrl->GetControllerType() == DEV_TEMPCTRL)
        {
            m_TempLMDCtrl = tempCtrl;
            return true;
        }
    }
    else
    {
        if (tempCtrl->GetControllerType() == DEV_TEMPCTRL)
        {
            m_TempCtrl = tempCtrl;
            return true;
        }
    }

    return false;
}
////////////////////////////////////////////////////////
bool C3PointCtrl::SetOpenCloseDevices(CDigitalIO * openDevice, CDigitalIO * closeDevice)
{
    if ( (openDevice->GetControllerType() != DEV_DIDO) || (closeDevice->GetControllerType() != DEV_DIDO))
    {
        return false;
    }
    else
    {
        m_OpenCtrl = openDevice;
        m_CloseCtrl = closeDevice;
    }

    return true;
}
////////////////////////////////////////////////////////
//          Update
////////////////////////////////////////////////////////
bool C3PointCtrl::Update(bool updateData)
{
    bool retVal = false;
    if (!m_IsActive)
    {
        return true;
    }
    else if (!m_IsOn)
    {
        //Chiudo la valvola
        CloseValve();
        
        //TODO da inserire il controllo sugli errori
    }
    else
    {    
        if (m_IsLMD)
        {
            retVal = UpdateLMDControl(updateData);
        }
        else
        {
            retVal = UpdateSimpleControl(updateData);
        }
    }

    if (m_DebugLevel) {
        cout << "C3Point: Setpoint: "<<m_Setpoint<<" Status: " << (int)m_ControllerStatus<<endl;
    }

    return retVal;
}
////////////////////////////////////////////////////////
//                  VerifyIOPresence
////////////////////////////////////////////////////////
bool C3PointCtrl::VerifyIOPresence()
{
    //TODO
    return true;
}
////////////////////////////////////////////////////////
//                  ConnectControllers
////////////////////////////////////////////////////////
bool C3PointCtrl::ConnectControllers(int netIndex, const char* configString)
{
    int devIndex, devIndex2;
    COneWireNet* netPtr = reinterpret_cast<COneWireNet*> ( m_NetPtr );
    T_Net *netHandler;
    CString strParam;

    //Prendo l'handler della rete
    netHandler = netPtr->GetNetHandler(netIndex);

    if (netHandler == 0x0)
    {
        return false;
    }

    //Controllo se l'input è locale o da remoto
    m_IniLib.GetConfigParamString(configString, "INPUT", &strParam, "NA");
    strParam.ToLower();
    
    if ( strParam == "remote")
    {
        m_Temperature.isRemoted = true;
        m_TempCtrl = 0x0;
    }
    else
    {
        //Leggo il dispositivo di ingresso
        m_IniLib.GetConfigParamInt(configString, "INPUT", &devIndex, -1);
        devIndex = netPtr->GetCtrlIndexByConfigNumber ( netIndex, devIndex );

        if ((devIndex < 0) || (devIndex > netHandler->CtrlList.size()) )
        {
            return false;
        }

        if (!SetInputDevice((CTempCtrl*)(netHandler->CtrlList[devIndex]), false))
        {
            return false;
        }

    }

    if (m_IsLMD)
    {
         //Controllo se l'input è locale o da remoto
        m_IniLib.GetConfigParamString(configString, "INPUT2", &strParam, "NA");
        strParam.ToLower();
    
        if ( strParam == "remote")
        {
            m_LMDTemperature.isRemoted = true;
            m_TempLMDCtrl = 0x0;
        }
        else
        {
            //Leggo il dispositivo di ingresso
            m_IniLib.GetConfigParamInt(configString, "INPUT2", &devIndex, -1);

            devIndex = netPtr->GetCtrlIndexByConfigNumber ( netIndex, devIndex );

            if ((devIndex < 0) || (devIndex > netHandler->CtrlList.size()) )
            {
                return false;
            }

            if (!SetInputDevice((CTempCtrl*)(netHandler->CtrlList[devIndex]), true))
            {
                return false;
            }
        }
    }

    //Collego i digitali di IO
    m_IniLib.GetConfigParamInt(configString, "OPEN", &devIndex, -1);
    m_IniLib.GetConfigParamInt(configString, "CLOSE", &devIndex2, -1);
    devIndex = netPtr->GetCtrlIndexByConfigNumber ( netIndex, devIndex );
    devIndex2 = netPtr->GetCtrlIndexByConfigNumber ( netIndex, devIndex2 );

    if ( (devIndex<0) || (devIndex2 < 0) || (devIndex > netHandler->CtrlList.size()) || (devIndex2 > netHandler->CtrlList.size()) )
    {
        return false;
    }

    if ( !SetOpenCloseDevices((CDigitalIO*)(netHandler->CtrlList[devIndex]), (CDigitalIO*)(netHandler->CtrlList[devIndex2])) )
    {
        return false;
    }

    return true;

}
////////////////////////////////////////////////////////////
//      UpdateSimpleControl
////////////////////////////////////////////////////////////
bool C3PointCtrl::UpdateSimpleControl(bool updateData)
{
    float error;
    
    bool retVal = false;
    bool isOn = true;

    //GetTemperatures();

//    if (!m_IsActive)
//    {
//        return true;
//    }
//
//    if (m_UseTimer && IsTimerEnabled())
//    {
//        isOn = GetValFromTimer();
//    }
//
//    if (isOn)
//    {
        if (m_Temperature.value != TEMP_ERRVAL)
        {
            if (m_Temperature.updated)
            {
                m_Temperature.updated = false;
                error = GetError(m_Temperature.value, m_Setpoint, false);

                if (error > m_NullZoneAmplitude/2.0)
                {
                    retVal = OpenValve();
                }
                else if (error < (-m_NullZoneAmplitude/2.0))
                {
                    retVal = CloseValve();
                }
                else
                {
                    //Area nulla, fermo la valvola
                    retVal = StopValve();
                }
            }
        }
        else
        {
            //Per sicurezza chiudo la valvola
            retVal = CloseValve();
        }
//    }
//    else
//    {
//        //Chiudo la valvola
//        retVal = CloseValve();
//    }

    return retVal;
}
/////////////////////////////////////////////////////////
//  GetError
/////////////////////////////////////////////////////////
float C3PointCtrl::GetError(float temp, float setpoint, bool isLmd)
{
    if (isLmd)
    {
        return temp - setpoint;
    }
    else if (m_IsSummer)
    {
        return temp - setpoint;
    }
    else
    {
        return setpoint - temp;
    }
}

/////////////////////////////////////////////////////////
//  UpdateLMDControl
/////////////////////////////////////////////////////////
bool C3PointCtrl::UpdateLMDControl(bool updateData)
{
    float errorDirect, errorH, errorL;
    
    bool retVal = false;
    bool isOn = true;

    //GetTemperatures();

//    if (!m_IsActive)
//    {
//        return true;
//    }
//
//    if (m_UseTimer && IsTimerEnabled())
//    {
//        isOn = GetValFromTimer();
//    }
//
//    if (isOn)
//    {
        if ((m_Temperature.value != TEMP_ERRVAL) && (m_LMDTemperature.value != TEMP_ERRVAL))
        {
            if (m_Temperature.updated && m_LMDTemperature.updated)
            {
                m_Temperature.updated = false;
                m_LMDTemperature.updated = false;
                errorDirect = GetError(m_Temperature.value, m_Setpoint, false);
                errorH = GetError(m_LMDTemperature.value, m_SetpointH, true);
                //Cambio di segno per ottenere sempre un valore positivo
                errorL = (-1.0)*(GetError(m_LMDTemperature.value, m_SetpointL, true));

                //Calcolo cosa devo fare: se l'errore sui limiti di mandata si attiva devo dare precedenza a lui
                if ((errorH > 0.0) || (errorL > 0.0))
                {
                    //Ho superato il limite di mandata superiore: chiudo la valvola
                    retVal = CloseValve();
                }
                else
                {
                    if (errorDirect > m_NullZoneAmplitude/2.0)
                    {
                        retVal = OpenValve();
                    }
                    else if (errorDirect < (-m_NullZoneAmplitude/2.0))
                    {
                        retVal = CloseValve();
                    }
                    else
                    {
                        //Area nulla, fermo la valvola
                        retVal = StopValve();
                    }
                }
            }
        }
        else
        {
            //Per sicurezzafermo la valvola
            retVal = StopValve();
        }
//    }
//    else
//    {
//        //Chiudo la valvola
//        retVal = CloseValve();
//    }

    return retVal;
}

/////////////////////////////////////////////////////////
//  OpenValve
/////////////////////////////////////////////////////////
bool C3PointCtrl::OpenValve()
{
    bool retVal;
    unsigned int actTime;

    actTime = time(0x0);
    
    if (m_TimeOfOpening == 0)
    {
                    //Devo aprire la valvola
        if( m_CloseCtrl->SetState(false, false))
        {
            retVal = m_OpenCtrl->SetState(true, false);
        }

                    //Tutto OK
        if (retVal)
        {
            m_TimeOfOpening = time(0x0);
            m_TimeOfClosing = 0;
            m_IsFullClosed = false;
            m_ControllerStatus = OPENING;
        }
    }
    else
    {
        if (actTime > m_TimeOfOpening + m_MovementTimeOut)
        {
            //Fermo al valvola perche' e' da troppo tempo che ho il comando attivo
            retVal = m_OpenCtrl->SetState(false, false);
            m_IsFullOpen = true;
            m_IsFullClosed = false;
            m_ControllerStatus = STOPPED;
        }
        else
        {
            m_IsFullOpen = false;
            m_IsFullClosed = false;
            retVal = true;
        }
    }

    return retVal;
}

/////////////////////////////////////////////////////////
//  CloseValve
/////////////////////////////////////////////////////////
bool C3PointCtrl::CloseValve()
{
    bool retVal = false;
    unsigned int actTime;
    COneWireEngine* engPtr = reinterpret_cast<COneWireEngine*>(m_EnginePtr);
    
    actTime = time(0x0);

    if (m_TimeOfClosing == 0)
    {
            //Devo aprire la valvola
        if( m_OpenCtrl->SetState(false, false))
        {
            retVal = m_CloseCtrl->SetState(true, false);
        }

                    //Tutto OK
        if (retVal)
        {
            m_TimeOfClosing = time(0x0);
            m_TimeOfOpening = 0;
            m_IsFullOpen = false;
            m_ControllerStatus = CLOSING;
        }
    }
    else
    {
        if (actTime > m_TimeOfClosing + m_MovementTimeOut)
        {
            //Fermo al valvola perche' e' da troppo tempo che ho il comando attivo
            retVal = m_CloseCtrl->SetState(false, false);
            m_IsFullClosed = true;
            m_ControllerStatus = STOPPED;
        }
        else
        {
            m_IsFullOpen = false;
            m_IsFullClosed = false;
            retVal = true;
        }
    }

//    if (m_SendNETMessages && (engPtr->CheckInterfacePortsForConnection()))
//    {
//        Cmd com("DEVICE");
//        com.putValue("TYPE","C3POINTCTRL");
//        com.putValue("ADDRESS",m_Address);
//        com.putValue("SUMMER",m_IsSummer);
//        com.putValue("STATUS", m_ControllerStatus);
//        com.putValue("SETPOINT", m_Setpoint);
//        com.putValue("FULLOPEN", m_IsFullOpen);
//        com.putValue("FULLCLOSED", m_IsFullClosed);
//
//        engPtr->WriteOnInterfacePorts(com.toString().c_str(), com.toString().size());
//    }

    return retVal;
}

/////////////////////////////////////////////////////////
//  StopValve
/////////////////////////////////////////////////////////
bool C3PointCtrl::StopValve()
{
    bool retVal;
    retVal = m_CloseCtrl->SetState(false, false);
    retVal = m_OpenCtrl->SetState(false, false) && retVal;
    m_TimeOfOpening = 0;
    m_TimeOfClosing = 0;
    m_ControllerStatus = STOPPED;

    return retVal;
}

/////////////////////////////////////////////////////////
//  GetTemperatures
/////////////////////////////////////////////////////////
void C3PointCtrl::GetTemperatures()
{
    if (!m_Temperature.isRemoted)
    {
        m_Temperature.value = m_TempCtrl->GetLastTemp();
        m_Temperature.updated = true;
    }

    if (m_IsLMD && (!m_LMDTemperature.isRemoted))
    {
        m_LMDTemperature.value = m_TempLMDCtrl->GetLastTemp();
        m_LMDTemperature.updated = true;
    }
}

/////////////////////////////////////////////////////////
//  SetTemperature
/////////////////////////////////////////////////////////
bool C3PointCtrl::SetTemperature(float newTemp, bool isLmd)
{
    if (isLmd && m_IsLMD)
    {
        m_LMDTemperature.value = newTemp;
        m_LMDTemperature.updated = true; 
        return true;
    }
    else if ( !isLmd )
    {
        m_Temperature.value = newTemp;
        m_Temperature.updated = true;
        return true;
    }

    return true;
}

/////////////////////////////////////////////////////////
//  SetLMDSetpoint
/////////////////////////////////////////////////////////
void C3PointCtrl::SetLMDSetpoint(float newSPH, float newSPL)
{
    m_SetpointH = newSPH;
    m_SetpointL = newSPL;

    SaveConfigParam("SPH", CString("")+m_SetpointH);
    SaveConfigParam("SPL", CString("")+m_SetpointL);
}

/////////////////////////////////////////////////////////////
//  Update2
/////////////////////////////////////////////////////////////
bool C3PointCtrl::Update2(bool updateData)
{
    COneWireEngine* engPtr = reinterpret_cast<COneWireEngine*>(m_EnginePtr);

    bool retVal = false;
    if (!m_IsActive)
    {
        return true;
    }

    if (m_UseTimer && IsTimerEnabled())
    {
        m_IsOn = GetValFromTimer();
    }

    //Leggo le temperature
    GetTemperatures_2();

    if (m_IsOn)
    {
        if (m_IsLMD)
        {
            retVal = UpdateLMDControl(updateData);
        }
        else
        {
            retVal = UpdateSimpleControl(updateData);
        }
    }
    else
    {
        retVal = CloseValve();
    }

    if (m_DebugLevel)
    {
        cout<<"C3POINTCTRL -- "<<
        " address: "<<m_Address<<
        " summer: "<<m_IsSummer<<
        " status: "<<m_ControllerStatus<<
        " SETPOINT: "<<m_Setpoint<<
        " FULLOPEN: "<<m_IsFullOpen<<
        " FULLCLOSED: "<<m_IsFullClosed<<endl;
    }
    
    if (engPtr->CheckInterfacePortsForConnection())
    {
        Cmd com("DEVICE");
        com.putValue("TYPE","C3POINTCTRL");
        com.putValue("ADDRESS",m_Address);
        com.putValue("SUMMER",m_IsSummer);
        com.putValue("STATUS", m_ControllerStatus);
        com.putValue("SETPOINT", m_Setpoint);
        com.putValue("FULLOPEN", m_IsFullOpen);
        com.putValue("FULLCLOSED", m_IsFullClosed);
        com.putValue("INPUT", m_Temperature.value);
        com.putValue("INPUTLMD",m_LMDTemperature.value);
        com.putValue("NULLZONE", m_NullZoneAmplitude);
        com.putValue("MOVEMENT",m_MovementTimeOut);

        com.putValue("ISON", m_IsOn);
        com.putValue("ISACTIVE", m_IsActive);

        CString message = com.getXMLValue();
        engPtr->WriteOnInterfacePorts(message.c_str(), message.size());
    }
    //TODO da implementare il messaggio in uscita
    return retVal;
}

/////////////////////////////////////////////////////////////
//  GetTemperatures_2
/////////////////////////////////////////////////////////////
void C3PointCtrl::GetTemperatures_2()
{
    if (!m_Temperature.isRemoted)
    {
        //Leggo dal driver
        m_Temperature.value = m_TempCtrl->GetLastTemp();
        m_Temperature.updated = m_TempCtrl->IsDataValid();
    }

    if (m_IsLMD && (!m_LMDTemperature.isRemoted))
    {
        m_LMDTemperature.value = m_TempLMDCtrl->GetLastTemp();
        m_LMDTemperature.updated = m_TempLMDCtrl->IsDataValid();
    }
}
