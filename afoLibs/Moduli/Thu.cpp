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
#include "Thu.h"
#include "conewireengine.h"

///////////////////////////////////////////////////
//              STANDARD CONSTRUCTOR
///////////////////////////////////////////////////
CThu::CThu(const char* configString): CVController(configString)
{
    CLibIniFile m_IniLib;

    m_ControllerType = DEV_THU;

    if (configString != 0x0)
    {
        m_IniLib.GetConfigParamBool( configString, "ISRIT", &m_IsRit, false);
        m_IniLib.GetConfigParamFloat(configString,"SP",&m_SetPoint,20.0);
        m_IniLib.GetConfigParamBool( configString, "ISRELATIVE", &m_IsRelative, false);
        m_IniLib.GetConfigParamFloat(configString,"RIT",&m_Rit,3.0);
    }

    m_SetPointThu = m_HumThu = m_TempThu = -100.0;
}
///////////////////////////////////////////////////
//      STANDARD DESTRUCTOR
///////////////////////////////////////////////////
CThu::~CThu()
{
}
///////////////////////////////////////////////////
//              SetInputDevice
///////////////////////////////////////////////////
bool CThu::SetInputDevice( CVDevice * inDevice )
{
    bool retVal = false;
    m_InDevice = inDevice;

    if (m_InDevice->GetFamNum() == DS2438_FN)
    {
        retVal = true;
    }

    return retVal;
}
///////////////////////////////////////////////////
//              Update2
///////////////////////////////////////////////////
bool CThu::Update2(bool updateData)
{
   t_DataVal dataVal = m_InDevice->GetDriverData();
   COneWireEngine* engPtr = reinterpret_cast<COneWireEngine*>(m_EnginePtr);

   if (!dataVal.isValid)
   {
       if (m_DebugLevel)
        {
            cout << "THU -- Address: "<<m_Address<<" INPUT NOT VALID "<<endl;
        }
        m_Data.isValid = false;
        return false;
   }
   else
   {
       m_Data.isValid = true;
   }

   //Controllo sul ritaratore
   if (m_IsRit)
   {
       int currentRegister;

       //Read current
       currentRegister = (int)dataVal.floatData[DS2438_CURRENT_INDEX];
       //Sto analizzando un ritaratore
       AcquireSetPoint(currentRegister);

       m_Data.floatData[THU_RIT_IDX] = m_SetPointThu;
   }

   //Umidità
   m_HumThu = dataVal.floatData[DS2438_VOLTAGE_INDEX]*20.0;
   m_Data.floatData[THU_HUMIDITY_IDX] = m_HumThu;

   //Temperatura
   m_TempThu = dataVal.floatData[DS2438_TEMPERATURE_INDEX];
   m_Data.floatData[THU_TEMPERATURE_IDX] = m_TempThu;

   if (m_DebugLevel)
    {
       if (m_IsRit)
       {
        cout << "THU -- Address: "<<m_Address<<" SetPoint: "<<m_SetPointThu<<" Temp: "<<m_TempThu<<" Hum: "<<m_HumThu<<endl;
       }
       else 
       {
        cout << "THU -- Address: "<<m_Address<<" Temp: "<<m_TempThu<<" Hum: "<<m_HumThu<<endl;
       }
    }

    if (engPtr->CheckInterfacePortsForConnection())
    {
        Cmd com("DEVICE");
        com.putValue("TYPE","THU");
        com.putValue("ADDRESS",m_Address);
        com.putValue("TEMP", m_TempThu);
        com.putValue("HUM",m_HumThu);
        com.putValue("SP",m_SetPointThu);

        engPtr->WriteOnInterfacePorts(com.toString().c_str(), com.toString().size());
    }

    return true;
    
}
bool CThu::Update(bool updateData)
{
    return false;
}

void CThu::AcquireSetPoint(int currentRegister)
{
    //Leggo il valore in corrente
   double angle;

   if (currentRegister >= 0)
    {
        if (currentRegister > 950)
        {
            currentRegister = 950;
        }

        //Cerco di linearizzare l'andamento del potenziometro dividendo i dati in due fasce
        if (currentRegister < 180)
        {
            angle = -0.0108*currentRegister*currentRegister
                    + 4.6356*currentRegister
                    - 356.03;
        }
        else if (currentRegister > 205)
        {
            angle = -0.0002*currentRegister*currentRegister
                    + 0.4482*currentRegister
                    +66.74;
        }
        else
        {
            //Lascio una fascia di intermezzo per i 20 gradi
            angle = 135.0;
        }

        if (angle < 0.0)
        {
            angle = 0.0;
        }
        else if (angle > 270.0)
        {
            angle = 270.0;
        }

       if (m_IsRelative)
       {
            //Ritaratore Relativo
           //Ritaratore relativo
            float newSetPoint;

            newSetPoint = angle * 6.0/270.0 - 3.0;
            m_SetPointThu = m_SetPoint + newSetPoint;
       }
       else
       {
            //Ritaratore Assoluto
           m_SetPointThu = angle * 20.0 / 270.0 + 10.0;
       }
    }
}