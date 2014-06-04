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
#include "vmultidido.h"

CVMultiDIDO::CVMultiDIDO(const char* configString, CTimer *timer): CVController(configString)
{
    int i = 0;
    if (configString != 0x0)
    {
        //RETROCOMPATIBILITA canali IN/OUT//
        int Ch = -1;
        
        //Get output channel
        m_IniLib.GetConfigParamInt( configString, "OUTCHANNEL", &Ch/*&m_OutChannels[0]*/, -1);
        //Adjust for internal settings: channels are internally numbered from 0 to 7
        if( Ch != -1 ) { m_OutChannels[0] = Ch - 1; }
        else //Se non c'è OUTCHANNEL, allora OUTCH, altrimenti resta il -1
            {
                m_IniLib.GetConfigParamInt( configString, "OUTCH", &Ch/*&m_OutChannels[0]*/, -1);
                m_OutChannels[0] = Ch - 1;
            }
                          
        
        //Get input channel
        m_IniLib.GetConfigParamInt( configString, "CHANNEL", &Ch/*&m_InChannels[0]*/, -1);
        //Adjust for internal settings: channels are internally numbered from 0 to 7
        if( Ch != -1 ) { m_InChannels[0] = Ch - 1; }
        else //Se non c'è INCHANNEL, allora INCH, altrimenti resta il -1
            {
                m_IniLib.GetConfigParamInt( configString, "INCH", &Ch/*&m_OutChannels[0]*/, -1);
                m_InChannels[0] = Ch - 1;
            }
        //-------------------------------//
            
        //Get TImeOut
        m_IniLib.GetConfigParamInt( configString, "TIMEOUT", &m_OutTimeOut[0], -1);
                    
        
        //Get Start Value
        m_IniLib.GetConfigParamBool( configString, "STARTV", &m_StartOn, false);
        
        //Invert output
        m_IniLib.GetConfigParamBool( configString, "INVERTOUT", &m_IsOutputInverted[0], false);
        
        //Invert output
        m_IniLib.GetConfigParamBool( configString, "INVERTIN", &m_IsInputInverted[0], false);
        
        //Timer Info
        m_IniLib.GetConfigParamInt( configString, "TIMERID", &m_TimerID, -1);
        m_IniLib.GetConfigParamBool( configString, "TIMERSTD", &m_UseCreativeTimer, false);
        m_IniLib.GetConfigParamInt( configString, "TIMERDEFVAL", &m_TimerDefaultValue, 0);
        m_IniLib.GetConfigParamBool( configString, "INVERTTIMER", &m_InvertTimerValue, false);
        
        //Is the Controller remoted
        m_IniLib.GetConfigParamInt (configString, "REMOTEADDRESS", &m_RemoteAddr, -1);
        m_IniLib.GetConfigParamInt (configString, "REMOTENET", &m_RemoteNet, -1);
        m_IniLib.GetConfigParamInt(configString,"REMOTEPORT",&m_RemotePort, -1);

        //Scalo la porta da 0
        m_RemotePort = m_RemotePort - 1;
        
        if ((m_RemoteAddr == -1) && (m_RemoteNet == -1))
        {
            m_IsRemoted = false;
        }
        else
        {
            m_IsRemoted  = true;
        }
               
        //Is a timed Controller: in other words if the input changes the output changes for a given amount of time
        m_IniLib.GetConfigParamInt( configString, "TIMEOUT", &m_OutputTimeOut, -1);
        m_OutputChangeStartTime = 0;
        m_OutputTimerActivated = false;
        m_PerformOutputChange = false;

        if (m_OutputTimeOut > 0)
        {
            m_IsOutputTimed = true;
        }
        else
        {
            m_IsOutputTimed = false;
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
    
    memset (m_InStatusRegisters, 0x0, 3*NOF_INDEVICES*sizeof(uchar));
    memset (m_OutStatusRegisters, 0x0, 3*NOF_OUTDEVICES*sizeof(uchar));
    memset (m_InDevices, 0x0, NOF_INDEVICES*sizeof(CVDevice*));
    memset (m_OutDevices, 0x0, NOF_OUTDEVICES*sizeof(CVDevice*));
    
    m_NofInputs = m_NofOutputs = 0;
    
    for (i = 0; i < NOF_INDEVICES; i++)
    {
        m_InDeviceStates[i] = -1;
    }
    
    for (i = 0; i < NOF_OUTDEVICES; i++)
    {
        m_OutDeviceStates[i] = -1;
    }
    
    m_TypeOfTimerVal = TIMERVAL_DIGITAL;
    m_SwitchTime = 0;
}


CVMultiDIDO::~CVMultiDIDO()
{
}

///////////////////////////////////////////////////
//              VerifyIOPresence
///////////////////////////////////////////////////
bool CVMultiDIDO::VerifyIOPresence( )
{
    bool inputOk = true, outputOk = true;
    
    for (int i = 0; i < NOF_INDEVICES; i++)
    {
        inputOk = m_InDevices[i]->VerifyPresence() && inputOk;
    }
    
    //Check if the devices are the same, in this case just return
    if (m_SameDevices)
    {
        return inputOk;
    }
    else
    {
        for (int i = 0; i < NOF_OUTDEVICES; i++)
        {
            outputOk = m_OutDevices[i]->VerifyPresence() && outputOk;
        }
        
        return (inputOk && outputOk);
    }
}

///////////////////////////////////////////////////
//              SetDevice
///////////////////////////////////////////////////
bool CVMultiDIDO::SetDevice( bool isOutput, CVDevice * theValue, int devIndex )
{
    
    if (theValue == 0x0)
    {
        return false;
    }
    else if (isOutput)
    {
        m_OutDevices[devIndex] = theValue;
    }
    else 
    {
        m_InDevices[devIndex] = theValue;
    }
    
    return true;
}

///////////////////////////////////////////////////
//              GetDevice
///////////////////////////////////////////////////
CVDevice * CVMultiDIDO::GetDevice( bool isOutput, int devIndex ) const
{
    if (isOutput)
    {
        return m_OutDevices[devIndex];
    }
    else
    {
        return m_InDevices[devIndex];
    }
}

///////////////////////////////////////////////////
//              GetStatusRegister
///////////////////////////////////////////////////
bool CVMultiDIDO::GetStatusRegister( bool isOutput, uchar * destBuffer, int devIndex, bool updateData )
{
    uchar tempRegister[3] = {0x0, 0x0, 0x0};
    bool retVal = false;
    
    //13/2/2009 -- aggiunto il controllo anche sulla validità del puntatore di uscita
    //perche' i buttoncontroller se sono jolly non inizializzano questo valore
    if ( (isOutput) && (!m_IsRemoted) && (m_OutDevices[devIndex] != 0x0) )
    {
        if (m_OutDevices[devIndex]->GetFamNum() == DS2408_FN)
        {
            if (updateData)
            {
                if ( ((CDS2408*)(m_OutDevices[devIndex]))->ReadRegister(0x88, tempRegister) )
                {
                    memcpy (&m_OutStatusRegisters[devIndex][0], tempRegister, 3*sizeof(uchar));
                    retVal = true;
                }
            }
            else
            {
                memcpy(&m_OutStatusRegisters[devIndex][0],((CDS2408*)(m_OutDevices[devIndex]))->m_Register,3);
                retVal = true;
            }

            if (destBuffer != 0x0)
            {
                memcpy(destBuffer, &m_OutStatusRegisters[devIndex][0], 3*sizeof(uchar));
            }
        }
        else if (m_OutDevices[devIndex]->GetFamNum() == DS2405_FN)
        {
            //TODO da implementare
        }
    }
    else
    {
        if (m_InDevices[devIndex]->GetFamNum() == DS2408_FN)
        {
            if (updateData)
            {
                if ( ((CDS2408*)(m_InDevices[devIndex]))->ReadRegister(0x88, tempRegister) )
                {
                    memcpy (&m_InStatusRegisters[devIndex][0], tempRegister, 3*sizeof(uchar));
                    retVal = true;
                }
            }
            else
            {
                memcpy (&m_InStatusRegisters[devIndex][0],((CDS2408*)(m_InDevices[devIndex]))->m_Register, 3);
                retVal = true;
            }

            if (destBuffer != 0x0)
            {
                memcpy(destBuffer, &m_InStatusRegisters[devIndex][0], 3*sizeof(uchar));
            }
        }
        else if (m_OutDevices[devIndex]->GetFamNum() == DS2405_FN)
        {
            //TODO da implementare
        }
    }
    
    return retVal;
}

///////////////////////////////////////////////////
//              SetStatusRegister
///////////////////////////////////////////////////
bool CVMultiDIDO::SetStatusRegister( bool isOutput, uchar * buffer,int devIndex )
{
    if (isOutput)
    {
        memcpy (&m_OutStatusRegisters[devIndex][0], buffer, 3*sizeof(uchar));
    }
    else
    {
        memcpy (&m_InStatusRegisters[devIndex][0], buffer, 3*sizeof(uchar));
    }
    
    return true;
}

///////////////////////////////////////////////////
//              GetActivity
///////////////////////////////////////////////////
bool CVMultiDIDO::GetActivity( bool updateRegisterFirst, int devIndex )
{
    
    if (updateRegisterFirst)
    {
        GetStatusRegister( false, 0x0, devIndex );
    }
    
    if (m_InDevices[devIndex]->GetFamNum() == DS2408_FN)
    {
        return ((CDS2408*)(m_InDevices[devIndex]))->GetSensedActivity(m_InChannels[0], &m_InStatusRegisters[0][0]);
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////
//              IOSameDevices
///////////////////////////////////////////////////
bool CVMultiDIDO::IOSameDevices() const
{
  return m_SameDevices;
}

///////////////////////////////////////////////////
//              GetOutputState
///////////////////////////////////////////////////
int CVMultiDIDO::GetOutputState( bool updateFirst, int devIndex )
{
    bool allOk = true;
    uchar  latch = (uchar) (0x01 << m_OutChannels[devIndex]);
    int retVal = -1;
    
    if (m_IsRemoted)
    {
        return m_OutDeviceStates[devIndex];
    }
    else if (m_OutDevices[devIndex] == 0x0)
    {
        return retVal;
    }
    
    if (updateFirst)
    {
        allOk = GetStatusRegister( true, 0x0, devIndex );
    }
    
    if (allOk)
    {
        if (m_IsOutputInverted[devIndex])
        {  
            retVal = !((m_OutStatusRegisters[devIndex][1] & latch) == latch);
        }
        else
        {
            retVal = ((m_OutStatusRegisters[devIndex][1] & latch) == latch);
        }
    }
    
    //Update the internal status
    m_OutDeviceStates[devIndex] = retVal;
    
    return retVal;
}

///////////////////////////////////////////////////
//              GetInputState
///////////////////////////////////////////////////
int CVMultiDIDO::GetInputState( bool updateFirst, int devIndex )
{
    bool allOk = true;
    uchar  level = (uchar) (0x01 << m_InChannels[0]);
    int retVal = -1;
    
    if (updateFirst)
    {
        allOk = GetStatusRegister( false, 0x0, devIndex );
    }
    
    if (allOk)
    {
        if (m_IsInputInverted[devIndex])
        {
            retVal = !((m_InStatusRegisters[devIndex][0] & level) == level);
        }
        else
        {
            retVal = ((m_InStatusRegisters[devIndex][0] & level) == level);
        }
    }
    
    //Update the internal status
    m_InDeviceStates[devIndex] = retVal;
    
    return retVal;
}

///////////////////////////////////////////////////
//              GetRemoteAddress
///////////////////////////////////////////////////
int CVMultiDIDO::GetRemoteAddress( )
{
    if (m_IsRemoted)
    {
        return m_RemoteAddr;
    }
    else
    {
        return -1;
    }
}

///////////////////////////////////////////////////
//              GetDeviceSN
///////////////////////////////////////////////////
uchar * CVMultiDIDO::GetDeviceSN( bool isOutput, int devIndex )
{
    if (isOutput)
    {
        return m_OutDevices[devIndex]->GetSN();
    }
    else
    {
        return m_InDevices[devIndex]->GetSN();
    }
}

///////////////////////////////////////////////////
//              GetDeviceSN
///////////////////////////////////////////////////
void CVMultiDIDO::GetDeviceSN( bool isOutput, uchar * SN, int devIndex )
{
    if (isOutput)
    {
        m_OutDevices[devIndex]->GetSN(SN);
    }
    else
    {
        m_InDevices[devIndex]->GetSN(SN);
    }
}

///////////////////////////////////////////////////
//              GetDeviceConfigIndex
///////////////////////////////////////////////////
int CVMultiDIDO::GetDeviceConfigIndex( bool isOutput, int devIndex )
{
    if (isOutput )
    {
        return m_OutDevices[devIndex]->GetConfigFileDevIndex();
    }
    else
    { 
        return m_InDevices[devIndex]->GetConfigFileDevIndex();
    }
}

///////////////////////////////////////////////////
//              GetRemoteNET
///////////////////////////////////////////////////
int CVMultiDIDO::GetRemoteNET( )
{
    return m_RemoteNet;
}

///////////////////////////////////////////////////
//              PerformOutputChange
///////////////////////////////////////////////////
bool CVMultiDIDO::PerformOutputChange(int newOutput)
{
    bool retVal = false;
    time_t actTime;

    time(&actTime);

    //Devo eseguire una transizione
    if (SetOutput(newOutput))
    {
        m_OutputTimerActivated = true;
        m_OutputChangeStartTime = actTime;
        m_PerformOutputChange = false;

        retVal = true;
    }
    else
    {
        //C'e' stato un errore, mi segno che al prossimo ciclo devo riprovare ad eseguire il cambio dell'uscita
        m_PerformOutputChange = true;
    }

    return retVal;
}

///////////////////////////////////////////////////
//              RestoreOutput
///////////////////////////////////////////////////
bool CVMultiDIDO::RestoreOutput(int originalOutput)
{
    bool retVal = false;

    //Restore output
    if (SetOutput(originalOutput))
    {
        m_OutputTimerActivated = false;
        m_OutputChangeStartTime = 0;
        m_OutputRestored = true;

        retVal = true;
    }
    else
    {
        m_OutputRestored = false;
    }

    return retVal;
}

///////////////////////////////////////////////////
//              InitInput
///////////////////////////////////////////////////
bool CVMultiDIDO::InitInput(int inIndex)
{
    return ((CDS2408*)(m_InDevices[inIndex]))->SetLatchState( m_InChannels[inIndex], 1 );
}

