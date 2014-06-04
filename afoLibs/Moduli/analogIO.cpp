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
#include "analogIO.h"
#include "conewireengine.h"

///////////////////////////////////////////////////
//              STANDARD CONSTRUCTOR
///////////////////////////////////////////////////
CAnalogIO::CAnalogIO(CVDevice* inputDevice, const char* configString, CTimer *timer)
    : CVController(configString)
{
    CLibIniFile m_IniLib;
    
    m_InDevice = inputDevice;
    
    if (m_InDevice->GetFamNum() == DS2438_FN)
    {
        m_IsInput = true;
        if (configString != 0x0)
        {
            m_IniLib.GetConfigParamBool( configString, "READCURRENT", &m_ReadCurrent, false);
        }
        
        m_LastVal = ANALOG_ERRVAL;
    }
    else if (m_InDevice->GetFamNum() == DS2890_FN)
    {
        m_IsInput = false;
        m_LastVal = -1;
        
        //Get settings
        if (configString != NULL)
        {
            m_IniLib.GetConfigParamInt(configString,"MAXVOLTAGE",&m_MaxVoltage, 10);
            m_IniLib.GetConfigParamInt( configString, "STARTVAL", (int*)(&m_StartPositionVolt), 0);
            m_IniLib.GetConfigParamInt( configString, "TIMERID", &m_TimerID, -1);
    
//             //Convert the start position in the 0 - 255 range
//             m_StartPositionVolt = (int)(-25.5*m_StartPositionVolt + 255.0);
        
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
    }

    m_LastOutputVolt = ANALOG_ERRVAL;
    m_ControllerType = DEV_AIAO;
    m_TypeOfTimerVal = TIMERVAL_ANALOG;
}
//////////////////////////////////////
CAnalogIO::CAnalogIO( const char* configString, CTimer * timer )  : CVController(configString)
{

    m_LastVal = -1;
    m_IsInput = false;
    m_LastOutputVolt = ANALOG_ERRVAL;
    
    m_ControllerType = DEV_AIAO;
    m_TypeOfTimerVal = TIMERVAL_ANALOG;
    
    if (configString != 0x0)
    {
        m_IniLib.GetConfigParamBool( configString, "READCURRENT", &m_ReadCurrent, false);
        m_IniLib.GetConfigParamInt(configString,"MAXVOLTAGE",&m_MaxVoltage, 10);
        m_IniLib.GetConfigParamInt( configString, "STARTVAL", (int*)(&m_StartPositionVolt), 0);
        m_IniLib.GetConfigParamInt( configString, "TIMERID", &m_TimerID, -1);
        m_IniLib.GetConfigParamInt( configString,"RSENS",&m_ResistorValue,120);
        m_IniLib.GetConfigParamFloat(configString,"SCALE",&m_ScaleFactor,1.0);
        m_IniLib.GetConfigParamFloat(configString,"OFFSET",&m_OffsetValue,0.0);
        
    
        //Convert the start position in the 0 - 255 range
//         m_StartPositionVolt = (int)(-25.5*m_StartPositionVolt + 255.0);
         
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
}

///////////////////////////////////////////////////
//      STANDARD DESTRUCTOR
///////////////////////////////////////////////////
CAnalogIO::~CAnalogIO()
{
}

///////////////////////////////////////////////////
//      SetInputCurrent
///////////////////////////////////////////////////
bool CAnalogIO::SetInputCurrent( bool isCurrentInput )
{
    if (m_IsInput)
    {
        //If change setting, invalidate the last measure
        if (m_ReadCurrent != isCurrentInput)
        {
            m_ReadCurrent = isCurrentInput;
            m_LastVal = MINVAL;
        }
        
        return true;
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////
//      GetValue
///////////////////////////////////////////////////
float CAnalogIO::GetValue(bool updateFirst )
{
    float retVal = ANALOG_ERRVAL;

    if (!updateFirst)
    {
        return m_LastVal;
    }
    
    if (m_IsInput)
    {
        if (m_ReadCurrent)
        {
            int currentRegister;
            
            //Read current
            currentRegister = ((CDS2438*)(m_InDevice))->ReadCurrentRegister();
            //Con i ritaratori serie CLIMA vado fuori scala
            if (currentRegister > 1023)
            {
                currentRegister = 1023;
            }
            
            retVal = (float)(currentRegister/(4096.0*m_ResistorValue));
            
        }
        else
        {
            //The maximum range of the NTH-AFO-AI is 0-5V so we have to scale the value
            retVal = ((CDS2438*)(m_InDevice))->ReadVoltage( false )*2.0;
        }
    }
    else
    {
        retVal = (float)( ((CDS2890*)(m_InDevice))->ReadPosition() );
        
        //Transform the position in volts
        retVal = (255.0 - retVal)/25.5;
    }
    
    if (retVal >= 0)
    {
        //Scalo il valore
        retVal = retVal*m_ScaleFactor+m_OffsetValue;
        m_LastVal = retVal;
        ClearError();
    }
    else
    {
        AddError();
    }
    
    return retVal;
}

bool CAnalogIO::Update( bool updateData )
{
    //TODO da implementare
    return true;
}

///////////////////////////////////////////////////
//              SetPosition
///////////////////////////////////////////////////
bool CAnalogIO::SetPosition( uchar newPos )
{
    bool retVal;
    int lastPosition = (int)(-25.5*m_LastVal + 255.0);
            
    if (m_IsInput)
    {
        retVal = false;
    }
    else
    {
        if ((newPos != lastPosition) || (m_LastVal < 0))
        {
            if ( ((CDS2890*)(m_InDevice))->SetPosition(newPos) )
            {
                m_LastVal = (255.0 - newPos)/25.5;;
                retVal = true;
            }
        }
        else
        {
            retVal = true;
        }

    }
    
     
    
    return retVal;
}
///////////////////////////////////////////////////
//              SetVal
///////////////////////////////////////////////////
bool CAnalogIO::SetVal(float val)
{
    //Metto un errore minimo oltre il quale comunque non aggiorno

    if ( ((m_LastOutputVolt - val) > ANALOG_OUT_MINERROR) ||
         ((val - m_LastOutputVolt) > ANALOG_OUT_MINERROR)
       )
    {
        return SetVOutput(val);
    }
    else {
        return true;
    }
}
///////////////////////////////////////////////////
//              SetVOutput
///////////////////////////////////////////////////
bool CAnalogIO::SetVOutput( float newV )
{
    int newSetValue;
    bool retVal;
    
    //Check argument
    if ( m_IsInput || (newV < 0.0) || (newV > 12.0) )
    {
        retVal = false;
    }
    else
    {
        newSetValue = (int)(-25.5*newV + 255.0);
        
        if ( ((CDS2890*)(m_InDevice))->SetPosition (newSetValue) )
        {
            m_LastVal = newSetValue;
            m_LastOutputVolt = newV;
            retVal = true;
        }
        else {
            retVal = false;
        }

    }

    return retVal;
}

///////////////////////////////////////////////////
//              SetMaxVoltage
///////////////////////////////////////////////////
bool CAnalogIO::SetMaxVoltage( int newMaxV )
{
    if (m_IsInput || (newMaxV < 0) || (newMaxV > 12))
    {
        return false;
    }
    else
    {
        m_MaxVoltage = newMaxV;
        return true;
    }
}

///////////////////////////////////////////////////
//              InitDevice
///////////////////////////////////////////////////
bool CAnalogIO::InitDevice( )
{
    bool retVal = false;
    
    if (m_IsInput)
    {
        //Setup the voltage measurements as VAD and not VDD which is fixed
        return ((CDS2438*)(m_InDevice))->SetVoltageMeasurement(false);
    }
    else
    {
        //Cambiato il 31/12/2008
        //Set Pump Charge OFF
        ((CDS2890*)(m_InDevice))->WriteControlRegister( 0x0C );
        
        //Set the position to the startup value
        if (SetVOutput( m_StartPositionVolt ))
        {
            retVal = true;
        }

        return retVal;
    }
}

///////////////////////////////////////////////////
//              VerifyIOPresence
///////////////////////////////////////////////////
bool CAnalogIO::VerifyIOPresence( )
{
    return m_InDevice->VerifyPresence();
}

///////////////////////////////////////////////////
//              SetInputDevice
///////////////////////////////////////////////////
bool CAnalogIO::SetInputDevice( CVDevice * inDevice )
{
    bool retVal = false;
    m_InDevice = inDevice;
    
    if (m_InDevice->GetFamNum() == DS2438_FN)
    {
        m_IsInput = true;
        m_LastVal = ANALOG_ERRVAL;
        retVal = true;
    }
    else if (m_InDevice->GetFamNum() == DS2890_FN)
    {
        m_IsInput = false;
        m_LastVal = -1;
        retVal = true;
    }
    
    return retVal;
}

///////////////////////////////////////////////////
//              ReadCurrentRegister
///////////////////////////////////////////////////
int CAnalogIO::ReadCurrentRegister()
{
    int retVal = -1;

    if ( m_IsInput && m_ReadCurrent)
    {       
        //Read current
        retVal = ((CDS2438*)(m_InDevice))->ReadCurrentRegister();
    }

    return retVal;
}
///////////////////////////////////////////////////
//              Update2
///////////////////////////////////////////////////
bool CAnalogIO::Update2(bool updateData)
{
    t_DataVal dataVal = m_InDevice->GetDriverData();
    COneWireEngine* engPtr = reinterpret_cast<COneWireEngine*>(m_EnginePtr);
    
    if (dataVal.isValid)
    {
        if (m_IsInput)
        {
            if (m_ReadCurrent)
            {
                int currentRegister;
            
                //Read current
                currentRegister = (int)dataVal.floatData[DS2438_CURRENT_INDEX];
                
                //Con i ritaratori serie CLIMA vado fuori scala
                if (currentRegister > 1023)
                {
                    currentRegister = 1023;
                }
            
                m_LastVal = (float)(currentRegister/(4096.0*m_ResistorValue));

            }
            else
            {
                m_LastVal = dataVal.floatData[DS2438_VOLTAGE_INDEX];
                
            }
            
            //Metto in scala
            m_LastVal = m_LastVal*m_ScaleFactor+m_OffsetValue;
            m_LastOutputVolt = ANALOG_ERRVAL;
        }
        else
        {
            //Lo rileggo comunque fuori.. vorrei farlo ongi 10-30 secondi solo per controllo
            m_LastOutputVolt = (255 - dataVal.regData[0])/25.5;
        }

        m_Data.floatData[0] = m_LastVal;
        m_Data.isValid = true;
        
        if (m_DebugLevel)
        {
            cout << "ANALOGIO -- Address: "<<m_Address<<" Input: "<<m_IsInput<<" Value: "<<m_LastVal<<endl;
        }

        if (engPtr->CheckInterfacePortsForConnection())
        {
            Cmd com("DEVICE");
            com.putValue("TYPE","AIAO");
            com.putValue("ADDRESS",m_Address);
            com.putValue("VAL",m_LastVal);
            com.putValue("OUTVOLT", m_LastOutputVolt);
            com.putValue("ISCURRENT",m_ReadCurrent);
            
            engPtr->WriteOnInterfacePorts(com.toString().c_str(), com.toString().size());
        }

        return true;
    }
    else {
        if (m_DebugLevel)
        {
            cout << "ANALOGIO -- Address: "<<m_Address<<" INPUT NOT VALID "<<endl;
        }
        m_Data.isValid = false;
    }
    
    return false;
}
