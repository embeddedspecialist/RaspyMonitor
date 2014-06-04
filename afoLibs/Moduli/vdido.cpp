
 
#include "vdido.h"

CVDIDO::CVDIDO( const char* configString, CTimer * timer ) : CVController(configString)
{
    if (configString != 0x0 )
    {
        //Get channel
        m_IniLib.GetConfigParamInt( configString, "CHANNEL", &m_Channel, -1);
        //Adjust for internal settings: channels are internally numbered from 0 to 8
        m_Channel = m_Channel - 1;
    }
    
    memset (m_StatusRegister, 0x0, 3*sizeof(char));
    m_Device = 0x0;
}

CVDIDO::~ CVDIDO( )
{
}

///////////////////////////////////////////////////
//              GetStatusRegister
///////////////////////////////////////////////////
bool CVDIDO::GetStatusRegister( uchar * destBuffer, bool updateData )
{
    uchar tempRegister[3] = {0x0, 0x0, 0x0};
    bool retVal = false;

    if (m_Device->GetFamNum() == DS2408_FN)
    {
        if (updateData)
        {
            if ( ((CDS2408*)(m_Device))->ReadRegister(0x88, tempRegister) )
            {
                if (destBuffer != 0x0)
                {
                    memcpy(destBuffer, tempRegister, 3*sizeof(uchar));
                }
                
                memcpy (m_StatusRegister, tempRegister, 3*sizeof(uchar));
                retVal = true;
            }
        }
        else
        {
            if (m_Device->GetDriverData().isValid)
            {
                //10/09/2009 -- Aggiorno lo stato interno solo se non ho errori, altrimenti lascio lo stato precedente.
                memcpy(m_StatusRegister, ((CDS2408*)(m_Device))->m_Register, 3);
                if (destBuffer != 0x0)
                {
                    memcpy(destBuffer, m_StatusRegister, 3*sizeof(uchar));
                }
            }
            retVal = true;
        }
    }
    else if (m_Device->GetFamNum() == DS2405_FN)
    {
        //TODO da implementare
    }
    
     
    return retVal;
}

///////////////////////////////////////////////////
//              VerifyIOPresence
///////////////////////////////////////////////////
bool CVDIDO::VerifyIOPresence( )
{
    return m_Device->VerifyPresence();
}

///////////////////////////////////////////////////
//              GetActivity
///////////////////////////////////////////////////
bool CVDIDO::GetActivity(bool updateRegisterFirst )
{
    if (updateRegisterFirst)
    {
        GetStatusRegister( );
    }
    
    if (m_Device->GetFamNum() == DS2408_FN)
    {
        return ((CDS2408*)(m_Device))->GetSensedActivity(m_Channel, m_StatusRegister);
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////
//              SetStatusregister
///////////////////////////////////////////////////
bool CVDIDO::SetStatusRegister( uchar * buffer )
{
    memcpy (m_StatusRegister, buffer, 3*sizeof(uchar));
    
    return true;
}
///////////////////////////////////////////////////
//              GetOutputState
///////////////////////////////////////////////////
int CVDIDO::GetChannelLatch( bool updateFirst )
{
    uchar  latch = (uchar) (0x01 << m_Channel);
    bool allOk = true;
    int retVal = -1;
    
    if (updateFirst)
    {
        allOk = GetStatusRegister( m_StatusRegister );
    }
    
    if (allOk)
    {
        if (m_IsOutputInverted)
        {
            retVal = !((m_StatusRegister[1] & latch) == latch);
        }
        else
        {
            retVal = ((m_StatusRegister[1] & latch) == latch);
        }
    }
    
    return retVal;
}
///////////////////////////////////////////////////
//              GetInputState
///////////////////////////////////////////////////
int CVDIDO::GetChannelLevel( bool updateFirst )
{
    uchar  level = (uchar) (0x01 << m_Channel);
    bool allOk = true;
    int retVal = -1;
    
    if (updateFirst)
    {
        allOk = GetStatusRegister( );
    }
    
    if (allOk)
    {
        if (m_IsOutputInverted)
        {
            retVal = ((m_StatusRegister[0] & level) == level);
        }
        else
        {
            retVal = !((m_StatusRegister[0] & level) == level);
        }
    }
    
    return retVal;
}


