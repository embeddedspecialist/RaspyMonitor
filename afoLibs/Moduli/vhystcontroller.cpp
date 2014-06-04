#include "vhystcontroller.h"


CVHystController::CVHystController(const char* configString)
 : CVController(configString)
{
    m_OutDevice = 0x0;
    m_Channel = -1;
    m_Hysteresis = 0.0;
    m_IsAutoControlEnabled = false;
    m_SetPoint = -1;
    
    //Get infos for this class
    if (configString != 0x0 )
    {
        //Get Hysteresis amount
        m_IniLib.GetConfigParamFloat( configString, "HYST", &m_Hysteresis, 3.0);
        
        //Get if auto control is enabled
        m_IniLib.GetConfigParamBool( configString, "AUTO", &m_IsAutoControlEnabled, 0);
        
        //Get the set point
        m_IniLib.GetConfigParamFloat( configString, "SETPOINT", &m_SetPoint, -1);
        
    }
    
    m_TypeOfTimerVal = TIMERVAL_HYST;
}


CVHystController::~CVHystController()
{
}

///////////////////////////////////////////////////
//       SetHysteresis
///////////////////////////////////////////////////
bool CVHystController::SetHysteresis( float newHysteresis )
{
    bool retVal = false;
    
    if ( (newHysteresis >= 0.0) && (newHysteresis <= 50.0) )
    {
        m_Hysteresis = newHysteresis;
        
        retVal = true;
    }
    
    return retVal;
}

///////////////////////////////////////////////////
//       SetAutoControl
///////////////////////////////////////////////////
bool CVHystController::SetAutoControl( bool enableAutoControl )
{
    if ((m_OutDevice != 0x0) && (m_Channel > -1) && (m_Channel < 8))
    {
        m_IsAutoControlEnabled = enableAutoControl;
    }
    else
    {
        m_IsAutoControlEnabled = false;
    }
    
    return true;
}

///////////////////////////////////////////////////
//       SetSetPoint
///////////////////////////////////////////////////
bool CVHystController::SetSetPoint( float newSetPoint )
{
    bool retVal = false;
    
    if ( (newSetPoint >= 0.0 ) && (newSetPoint <=100.0) ) 
    {
        m_SetPoint = newSetPoint;
        retVal = true;
    }
    
    return retVal;
}

///////////////////////////////////////////////////
//       SetOutputDevice
///////////////////////////////////////////////////
bool CVHystController::SetOutputDevice( CDS2408 *device, int channel )
{
    bool retVal = false;
    
    if ( (device != 0x0 ) && (device->GetFamNum() == DS2408_FN) && ((channel > -1 ) && (channel < 8)) )
    {
        m_OutDevice = device;
        m_Channel = channel;
        retVal = true;
    }
    
    return retVal;
}

