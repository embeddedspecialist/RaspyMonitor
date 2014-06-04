

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
#include "humcontroller.h"
#include "conewireengine.h"

///////////////////////////////////////////////////
//              STANDARD CONSTRUCTOR (1)
///////////////////////////////////////////////////
 CHumController:: CHumController(CDS2438* inDevice, const char* configString, CTimer *timer )
    : CVHystController(configString)
{
    //Create the correct sub_device
    m_InDevice = inDevice;
    m_ControllerType = DEV_HUMIDITY;
    
    m_LastRelHumidity = -1.0;
    m_LastAbsHumidity = -1.0;
    m_LastTemp = TEMP_ERRVAL;
    
    if (m_SetPoint < 0)
    {
        m_SetPoint = 50.0;
    }
}

///////////////////////////////////////////////////
//              STANDARD CONSTRUCTOR (2)
///////////////////////////////////////////////////
CHumController::CHumController( const char* configString, CTimer * timer )   : CVHystController(configString)
{
    m_InDevice = 0x0;
    m_ControllerType = DEV_HUMIDITY;
    
    m_LastRelHumidity = -1.0;
    m_LastAbsHumidity = -1.0;
    m_LastTemp = TEMP_ERRVAL;
    
    if (m_SetPoint < 0)
    {
        m_SetPoint = 50.0;
    }
}

///////////////////////////////////////////////////
//      STANDARD DESTRUCTOR
///////////////////////////////////////////////////
 CHumController::~ CHumController()
{

}

///////////////////////////////////////////////////
//       GetHumidity
///////////////////////////////////////////////////
float  CHumController::GetHumidity( )
{
    float Vdd = 0.0, Vad = 0.0;
    float retVal = ANALOG_ERRVAL;
                  
    //Read power supply
    //FIXME spostata la lettura di temperatura qui perche'sembra che non legga VDD se non legge prima la temperatura
    m_LastTemp = m_InDevice->ReadTemperature( true );
    
    
    //TBR
    //float current = 0.0;
//     current = m_InDevice->ReadCurrent();
    
    Vdd = m_InDevice->ReadVoltage( true );
    
     
    if ( (Vdd > 0.0) && (m_LastTemp != TEMP_ERRVAL) )
    {
        if(Vdd > 5.8)
        {
            Vdd = 5.8;
        }
        else if(Vdd < 4.0)
        {
            Vdd = 4.0;
        }

        
        Vad = m_InDevice->ReadVoltage( false );
        
        
        if (Vad > 0)
        {
        
    //         m_LastTemp = m_InDevice->ReadTemperature( true );
    
            m_LastRelHumidity = ((Vad/Vdd) - 0.16)/0.0062;
                    
            m_LastAbsHumidity = (((Vad/Vdd) - 0.16)/0.0062)/(1.0546 - 0.00216 * m_LastTemp);
            
            if(m_LastAbsHumidity > 100.0)
            {
                m_LastAbsHumidity = 100.0;
            }
            else if(m_LastAbsHumidity < 0.0)
            {
                m_LastAbsHumidity = 0.0;
            }
        }
        
        retVal = m_LastAbsHumidity;
        //TBR
//         cout << "Corrente: "<<current<<" -- Vdd: "<<Vdd<<" --Vad: " << Vad<<endl;
        ClearError();

    }
    else
    {
        //Error
        AddError();
    }
    
    return retVal;
}

///////////////////////////////////////////////////
//       GetHumidity
///////////////////////////////////////////////////
bool CHumController::GetHumidity( float * absHum, float * relHum, float *temp )
{
    bool retVal = false;
    
    if (absHum == 0x0)
    {
        return retVal;
    }
    
    if (GetHumidity() > ANALOG_ERRVAL)
    {
        *absHum = m_LastAbsHumidity;
        
        if (relHum != 0x0)
        {
            *relHum = m_LastRelHumidity;
        }
        
        if (temp != 0x0)
        {
            *temp = m_LastTemp;
        }
        
        retVal = true;
    }
    
    return retVal;
}



///////////////////////////////////////////////////
//       Update
///////////////////////////////////////////////////
bool CHumController::Update( bool updateData )
{
    bool retVal = false;
    
    if (m_TimerID > 0)
    {
        m_IsAutoControlEnabled = GetValFromTimer();
    }
    
    
    if ( (m_OutDevice == 0x0) || (!m_IsAutoControlEnabled) )
    {
        return retVal;
    }
    
    if (updateData)
    {
        GetHumidity();
    }
    
    if (m_IsAutoControlEnabled)
    {
        //We are in full automatic.... control the output
        if (m_LastAbsHumidity > m_SetPoint + m_Hysteresis)
        {
            //Turn off
            retVal = m_OutDevice->SetLatchState( m_Channel, false);
        }
        else if (m_LastAbsHumidity < m_SetPoint - m_Hysteresis)
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
//       ReadTemperature
///////////////////////////////////////////////////
float CHumController::ReadTemperature( bool update )
{
    return m_InDevice->ReadTemperature( update );
}

///////////////////////////////////////////////////
//       VerifyIOPresence
///////////////////////////////////////////////////
bool CHumController::VerifyIOPresence( )
{
    bool retVal = false;
    
    retVal = m_InDevice->VerifyPresence();
    
    if (m_OutDevice != 0x0)
    {
        retVal = retVal && m_OutDevice->VerifyPresence();
    }
    
    return retVal;
}

