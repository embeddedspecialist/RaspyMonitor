

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
 ************************************************************************/
#include "tempctrlhyst.h"

#ifndef AFO_NO_TEMPERATURES
///////////////////////////////////////////////////
//              STANDARD CONSTRUCTOR (1)
///////////////////////////////////////////////////
CTempCtrlHyst::CTempCtrlHyst(CVDevice* inDevice, const char* configString, CTimer *timer): CVHystController(configString)
{
    m_LastTemp = TEMP_ERRVAL;
    
    m_InDevice = inDevice;
    m_ControllerType = DEV_TEMPCTRLHYST;
    
    if (m_SetPoint < 0.0)
    {
        m_SetPoint = 20.0;
    }

}

///////////////////////////////////////////////////
//              STANDARD CONSTRUCTOR (2)
///////////////////////////////////////////////////
CTempCtrlHyst::CTempCtrlHyst( const char* configString, CTimer * timer ): CVHystController(configString)
{
    m_LastTemp = TEMP_ERRVAL;
    
    m_InDevice = 0x0;
    m_ControllerType = DEV_TEMPCTRLHYST;
    
    if (m_SetPoint < 0.0)
    {
        m_SetPoint = 20.0;
    }
    
    //Get summer settings
    if (configString != 0x0 )
    {
        //Get Hysteresis amount
        m_IniLib.GetConfigParamBool( configString, "SUMMER", &m_IsSummer, false);        
    }
    else
    {
        m_IsSummer = false;
    }
}

CTempCtrlHyst::~CTempCtrlHyst()
{
}

///////////////////////////////////////////////////
//              ReadTemperature (1)
///////////////////////////////////////////////////
float CTempCtrlHyst::ReadTemperature( bool update )
{
    float temp = TEMP_ERRVAL;
    
    ReadTemperature( update, &temp);
    
    return temp;
}

///////////////////////////////////////////////////
//              ReadTemperature (2)
///////////////////////////////////////////////////
bool CTempCtrlHyst::ReadTemperature( bool updateFirst, float * newTemp )
{
    bool retVal = false;
    
    //Get The temperature
    if ( (m_InDevice->GetFamNum() == DS18S20_FN) || (m_InDevice->GetFamNum() == DS18B20_FN) )
    {
        retVal = ((CDS18X20*)(m_InDevice))->ReadTemperature(updateFirst, newTemp);
    }
    else if (m_InDevice->GetFamNum() == DS2438_FN)
    {
        retVal = ((CDS2438*)(m_InDevice))->ReadTemperature(updateFirst, newTemp);
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
//              Update
///////////////////////////////////////////////////
bool CTempCtrlHyst::Update( bool updateData )
{
    bool retVal = false;
    float tempError = 0.0;
    
    if (m_UseTimer && IsTimerEnabled())
    {
        m_IsAutoControlEnabled = GetValFromTimer();
    }
    
    if ( (m_OutDevice == 0x0) || (!m_IsAutoControlEnabled) )
    {
        return retVal;
    }
    
    if (updateData)
    {
        ReadTemperature( true );
    }
    
    if (m_IsAutoControlEnabled)
    {
        //TODO da provare
        if (m_IsSummer)
        {
            tempError = m_SetPoint - m_LastTemp;
        }
        else
        {
            tempError = m_LastTemp - m_SetPoint;
        }
        
        //We are in full automatic.... control the output
        if (tempError > m_Hysteresis)
        {
            //Turn off
            retVal = m_OutDevice->SetLatchState( m_Channel, false);
        }
        else if (m_LastTemp < m_SetPoint - m_Hysteresis)
        {
            //Turn On
            retVal = m_OutDevice->SetLatchState( m_Channel, true);
        }
    }
    else
    {
        //Default action
        retVal = true;
    }
    
    return retVal;
}

///////////////////////////////////////////////////
//              VerifyIOPresence
///////////////////////////////////////////////////
bool CTempCtrlHyst::VerifyIOPresence( )
{
    bool retVal = false;
    
    retVal = m_InDevice->VerifyPresence();
    
    if (m_OutDevice != 0x0)
    {
        retVal = retVal && m_OutDevice->VerifyPresence();
    }
    
    return retVal;
}

///////////////////////////////////////////////////
//              UpdateTemp
///////////////////////////////////////////////////
bool CTempCtrlHyst::UpdateTemp( )
{
    bool retVal = false;
    
    //Get The temperature
    if ( (m_InDevice->GetFamNum() == DS18B20_FN) || (m_InDevice->GetFamNum() == DS18S20_FN) )
    {
        retVal = ((CDS18X20*)(m_InDevice))->UpdateTemp();
    }
    else if (m_InDevice->GetFamNum() == DS2438_FN)
    {
        retVal = ((CDS2438*)(m_InDevice))->UpdateTemp();
    }
  
     
    return retVal;
}


///////////////////////// SMALL TARGET ////////////////////////////////
#else
CTempCtrlHyst::CTempCtrlHyst( const char* configString, CTimer * timer ): CVHystController(configString)
{
}

CTempCtrlHyst::~ CTempCtrlHyst( )
{
}

float CTempCtrlHyst::ReadTemperature( bool update )
{
    return -1.0;
}

bool CTempCtrlHyst::ReadTemperature( bool updateFirst, float * newTemp )
{
    return true;
}

bool CTempCtrlHyst::Update( bool updateData )
{
    return true;
}

bool CTempCtrlHyst::VerifyIOPresence( )
{
    return true;
}

bool CTempCtrlHyst::UpdateTemp( )
{
    return true;
}


#endif
