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
#include "pidlmd.h"
#include "conewirenet.h"
#include "conewireengine.h"
        
CPIDLMD::CPIDLMD(const char* configString, CVDevice *in1Device, CVDevice *in2Device, CVDevice *outDevice, CTimer *timer)
    : CVPID( configString )
{

    m_InputDevices[0] = in1Device;
    m_InputDevices[1] = in2Device;

    
    //Get default setpoint
    m_IniLib.GetConfigParamFloat(configString, "SETPOINT",  &m_SetPoint[0], 20.0);
    m_IniLib.GetConfigParamFloat(configString, "SETPOINTH",  &m_SetPoint[1], 20.0);
    m_IniLib.GetConfigParamFloat(configString, "SETPOINTL",  &m_SetPoint[2], 20.0);
    
    //Get default controller parameters
    m_IniLib.GetConfigParamFloat(configString, "KP1", &m_Parameters[0], 1.0);
    m_IniLib.GetConfigParamFloat(configString, "Tint1",  &m_Parameters[1], 0.0);
    m_IniLib.GetConfigParamFloat(configString, "Tder1",  &m_Parameters[2], 0.0);

    if  (m_Parameters[1]>0.0)
    {
        m_MaxIntegralError[0] = 255.0/m_Parameters[1];
    }
    else
    {
      m_Parameters[1] = 1.0;
        m_MaxIntegralError[0] = 255.0;
    }
    
    m_IniLib.GetConfigParamFloat(configString, "KP2",  &m_Parameters[3], 1.0);
    m_IniLib.GetConfigParamFloat(configString, "Tint2",  &m_Parameters[4], 0.0);
    m_IniLib.GetConfigParamFloat(configString, "Tder2",  &m_Parameters[5], 0.0);
    if (m_Parameters[4]>0.0)
    {
        m_MaxIntegralError[1] = 255.0/m_Parameters[4];
    }
    else
    {
      m_Parameters[4] = 1.0;
        m_MaxIntegralError[1] = 255.0;
    }    
    
    
    memset (m_IntegralState, 0, 3*sizeof(float));
    memset (m_DerivativeState, 0, 3*sizeof(float));
    
    m_IniLib.GetConfigParamBool( configString, "SUMMER", &m_IsSummer, false);
    
    m_IniLib.GetConfigParamInt( configString, "ADDR", &m_Address, -1);
    
    m_IniLib.GetConfigParamInt( configString, "TIMERID", &m_TimerID, -1);
    
    if ( (m_TimerID > 0) && (timer != 0x0) )
    {
        m_Timer = timer;
        m_UseTimer = true;
    }
    else
    {
        m_UseTimer = false;
    }
    
    //Get the normalizing factors:
    m_IniLib.GetConfigParamFloat(configString, "MFACTOR",  &m_normalizeM, 1.0);
    m_IniLib.GetConfigParamFloat( configString, "QFACTOR",  &m_normalizeQ, 0.0);
    
    m_IniLib.GetConfigParamBool( configString, "TIMERSTD", &m_UseCreativeTimer, false);
    
    m_LastPositionVolt = 0.0;
    m_controlVar = m_limitVar = 85.0;
    
    m_LmdVariable.value = ANALOG_ERRVAL;
    m_LmdVariable.isRemoted = false;
    
    m_TypeOfTimerVal = TIMERVAL_PID;
    m_ControllerType = DEV_PIDLMD;
    
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
CPIDLMD::CPIDLMD( const char* configString, CTimer * timer ): CVPID(configString)
{
    memset (m_IntegralState, 0, 3*sizeof(float));
    memset (m_DerivativeState, 0, 3*sizeof(float));
    
    //Get default setpoint
    m_IniLib.GetConfigParamFloat(configString, "SETPOINT",  &m_SetPoint[0], 20.0);
    m_IniLib.GetConfigParamFloat(configString, "SETPOINTH",  &m_SetPoint[1], 35.0);
    m_IniLib.GetConfigParamFloat(configString, "SETPOINTL",  &m_SetPoint[2], 15.0);
    
    //Get default controller parameters
    m_IniLib.GetConfigParamFloat(configString, "KP1", &m_Parameters[0], 8.0);
    m_IniLib.GetConfigParamFloat(configString, "Tint1",  &m_Parameters[1], 1.0);
    m_IniLib.GetConfigParamFloat(configString, "Tder1",  &m_Parameters[2], 0.0);

    if (m_Parameters[1]>0.0)
    {
        m_MaxIntegralError[0] = 255.0/m_Parameters[1];
    }
    else
    {
        m_Parameters[1]=1.0;
        m_MaxIntegralError[1]=255.0;
    }
    
    m_IniLib.GetConfigParamFloat(configString, "KP2",  &m_Parameters[3], 1.0);
    m_IniLib.GetConfigParamFloat(configString, "Tint2",  &m_Parameters[4], 0.1);
    m_IniLib.GetConfigParamFloat(configString, "Tder2",  &m_Parameters[5], 0.0);

    if (m_Parameters[4]>0.0)
    {
        m_MaxIntegralError[1] = 255.0/m_Parameters[4];
    }
    else
    {
        m_Parameters[4] = 1.0;
        m_MaxIntegralError[1] = 255.0;
    }
    
    m_IniLib.GetConfigParamBool( configString, "SUMMER", &m_IsSummer, false);
    
    m_IniLib.GetConfigParamInt( configString, "ADDR", &m_Address, -1);
    
    m_IniLib.GetConfigParamInt( configString, "TIMERID", &m_TimerID, -1);
    
    if ( (m_TimerID > 0) && (timer != 0x0) )
    {
        m_Timer = timer;
        m_UseTimer = true;
    }
    else
    {
        m_UseTimer = false;
    }
    
    //Get the normalizing factors:
    m_IniLib.GetConfigParamFloat(configString, "MFACTOR",  &m_normalizeM, 1.0);
    m_IniLib.GetConfigParamFloat( configString, "QFACTOR",  &m_normalizeQ, 0.0);
    
    m_IniLib.GetConfigParamBool( configString, "TIMERSTD", &m_UseCreativeTimer, false);
    
    m_LastPositionVolt = 0.0;
    
    m_TypeOfTimerVal = TIMERVAL_PID;
    m_ControllerType = DEV_PIDLMD;
    
    m_InputDevices[0] = 0x0;
    m_InputDevices[1] = 0x0;
    m_OutputDev = 0x0;

    m_LmdVariable.value = TEMP_ERRVAL;
    m_LmdVariable.isRemoted = false;
    
}

CPIDLMD::~CPIDLMD()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPIDLMD::SetInputOutputDevices( CVDevice * in1Device, CVDevice * in2Device, CVDevice * outDevice )
{
    if (!m_CtrlVariable.isRemoted)
    {
        if ((in1Device->GetFamNum() != DS18S20_FN) && (in1Device->GetFamNum() != DS18B20_FN) && (in1Device->GetFamNum() != DS2438_FN))
        {
            return false;
        }

        m_InputDevices[0] = in1Device;
    }

    if (!m_LmdVariable.isRemoted)
    {
        if ((in2Device->GetFamNum() != DS18S20_FN) && (in2Device->GetFamNum() != DS18B20_FN) && (in2Device->GetFamNum() != DS2438_FN))
        {
            return false;
        }

        m_InputDevices[1] = in2Device;
    }

    if (outDevice->GetFamNum() != DS2890_FN)
    {
        return false;
    }
    
    m_OutputDev = outDevice;

    return true;
}

///////////////////////////////////////////////////
//              Integrate
///////////////////////////////////////////////////
float CPIDLMD::Integrate( int pidIndex, float Ki, float value )
{
    float maxIntegralError = 0.0;
    
    m_IntegralState[pidIndex]+=value;
    
    switch (pidIndex)
    {
        case 0: maxIntegralError = this->m_MaxIntegralError[0];break;
        case 1:
        case 2: maxIntegralError = this->m_MaxIntegralError[1];break;
    }
    
    if (m_IntegralState[pidIndex] > maxIntegralError)
    {
        m_IntegralState[pidIndex] = maxIntegralError;
    }
    else if (m_IntegralState[pidIndex] < 0.0)
    {
        m_IntegralState[pidIndex] = 0.0;
    }
    
    //TBR
//     cout << "PID Addr:" << m_Address << " Valore Integrale: " << m_IntegralState[pidIndex] << " Max Integral Error: " << this->m_MaxIntegralError[pidIndex]<< endl;
    
    return Ki*m_IntegralState[pidIndex];
}

///////////////////////////////////////////////////
//              Derivate
///////////////////////////////////////////////////
float CPIDLMD::Derivate( int pidIndex, float Kd, float value )
{
    float retVal = 0.0;

    retVal = Kd*(value - m_DerivativeState[pidIndex]);
    
    m_DerivativeState[pidIndex] = value;
    
    return retVal;
    
}

///////////////////////////////////////////////////
//              Update
///////////////////////////////////////////////////
bool CPIDLMD::Update( bool updateData )
{
    float error1 = 0.0, error2 = 0.0, error3 = 0.0;
    float normalizedDeviceInput1 = 0.0, normalizedSetpointDirect = 0.0;
    float normalizedDeviceInput2 = 0.0, normalizedSetpointH = 0.0, normalizedSetpointL = 0.0;
    float pid1Output = 0.0, pid2Output = 0.0, pid3Output = 0.0;
    float newPosition = m_LastPosition;
    bool retVal = false;

    if (!m_IsActive)
    {
            return true;
    }
    else if (!m_IsOn)
    {
        //Forzo a 0 l'uscita
        if (m_LastPosition != 255)
        {
            retVal = ((CDS2890*)(m_OutputDev))->SetPosition(255);
            if (retVal)
            {
                m_LastPosition = 255;
                m_LastPositionVolt = 0.0;
            }
        }
        
        //TODO da inserire il controllo sugli errori
        return true;
    }
                        
    if (CheckHandles())
    {
        //Get inputs
        if (!m_CtrlVariable.isRemoted)
        {
            m_CtrlVariable.value = GetValFromInput( 0, updateData );
            m_CtrlVariable.updated = true;
        }
        else
        {
            if (!m_CtrlVariable.updated)
            {
                //La variabile di controllo non è aggiornata
                return true;
            }
        }

        if (!m_LmdVariable.isRemoted)
        {
            m_LmdVariable.value = GetValFromInput( 1, updateData );
            m_LmdVariable.updated = true;
        }
        else
        {
            if (!m_LmdVariable.updated)
            {
                //La variabile di controllo non è aggiornata
                return true;
            }
        }
        
        m_LmdVariable.value = GetValFromInput( 1, updateData );
        
        //Get temperature
        if ( ( m_CtrlVariable.value != TEMP_ERRVAL ) && (m_LmdVariable.value != TEMP_ERRVAL) )
        {

            //Check if the timer is active and going
            if (m_UseTimer && IsTimerEnabled())
            {
                GetValFromTimer();
            }

            //Normalize inputs
            NormalizeInputs(&normalizedDeviceInput1, &normalizedSetpointDirect,0);
            NormalizeInputs(&normalizedDeviceInput2, &normalizedSetpointH,1);
            NormalizeInputs(&normalizedDeviceInput2, &normalizedSetpointL,2);

            //Calculate errors
            error1 = GetError(normalizedDeviceInput1, normalizedSetpointDirect);
           
            //Controllo di minima
            error2 = normalizedSetpointL - normalizedDeviceInput2;
            //Controllo di massima
            error3 = normalizedDeviceInput2 - normalizedSetpointH;

            //Apply the PID parameters
            pid1Output = m_Parameters[0]*error1 + Integrate( 0, m_Parameters[1], error1 ) + Derivate( 0, m_Parameters[2], error1 );

            if (pid1Output < 0)
            {
                pid1Output = 0;
            }
            else if (pid1Output > 255)
            {
                pid1Output = 255;
            }

            //Calcolo il PID di minima
            pid2Output = m_Parameters[3]*error2 + Integrate( 1, m_Parameters[4], error2 ) + Derivate( 1, m_Parameters[5], error2 );

            if (pid2Output < 0)
            {
                pid2Output = 0;
            }
            else if (pid2Output > 255)
            {
                pid2Output = 255;
            }

            //Calcolo il PID di massima
            pid3Output = m_Parameters[3]*error3 + Integrate( 2, m_Parameters[4], error3 ) + Derivate( 2, m_Parameters[5], error3 );

            if (pid3Output < 0)
            {
                pid3Output = 0;
            }
            else if (pid3Output > 255)
            {
                pid3Output = 255;
            }

            //Calculate new position
            if (m_IsSummer)
            {
                //Report last position
                newPosition = (int)(255.0 - pid1Output + pid2Output - pid3Output);
            }
            else
            {
                newPosition = (int)(255.0 - pid1Output - pid2Output + pid3Output);
            }


            //Set the analog output: 255 means the output is low, 0 is at Vmax
            if (newPosition > 255)
            {
                newPosition = 255;
            }
            else if ( newPosition < 0 )
            {
                newPosition = 0;
            }

            if (newPosition != m_LastPosition)
            {
                retVal = ((CDS2890*)(this->m_OutputDev))->SetPosition((uchar)newPosition);

                if (retVal)
                {
                    m_LastPosition = (int)newPosition;
                }
            }
            else
            {
                retVal = true;
            }

            m_LastPositionVolt = (float)(( 255.0 - m_LastPosition )/255.0 * 10.0);
        }
        else
        {
            PushError( AFOERROR_PID_UNABLE_TO_GETINPUT, m_NetNumber, m_DeviceNumber);
        }
    } //IF CheckHandles
        
    
    if (retVal)
    {
        ClearError();
    }
    else
    {
        PushError( AFOERROR_PID_UNABLE_TO_UPDATE, m_NetNumber, m_DeviceNumber);
        AddError();
    }
    
    return retVal;
}

///////////////////////////////////////////////////
//              SetSetPoint
///////////////////////////////////////////////////
bool CPIDLMD::SetSetPoint( float *newSP, int nOfParam )
{
    bool retVal = false;
    int i = 0;
    
    if (nOfParam <= 3)
    {
        //Just a little check on the argument...
        if ( newSP != 0x0 )
        {
            for (i = 0; i < nOfParam; i++)
            {
                m_SetPoint[i] = newSP[i];
                m_PidVector[i].SetSetPoint(newSP[i]);
            }
    
            retVal = true;
        }
    }
    
    return retVal;
}

///////////////////////////////////////////////////
//              SetSetPointH
///////////////////////////////////////////////////
bool CPIDLMD::SetSetPointH( float *newSP )
{
    bool retVal = true;

    m_SetPoint[1] = *newSP;
    m_PidVector[LMMAX_PID].SetSetPoint(*newSP);

    return retVal;
}

///////////////////////////////////////////////////
//              SetSetPointL
///////////////////////////////////////////////////
bool CPIDLMD::SetSetPointL( float *newSP )
{
    bool retVal = true;

    m_SetPoint[2] = *newSP;
    m_PidVector[LMMIN_PID].SetSetPoint(*newSP);

    return retVal;
}

///////////////////////////////////////////////////
//              SetSummer
///////////////////////////////////////////////////
bool CPIDLMD::SetSummer( bool summerSet )
{
    m_IsSummer = summerSet;
    m_PidVector[DIRECT_PID].SetSummer( summerSet );
    return true;
}

///////////////////////////////////////////////////
//              GetInfo
///////////////////////////////////////////////////
bool CPIDLMD::GetInfo( float * parameters, bool * isSummer, CString &type )
{
    bool retVal = false;
    int i = 0;
    
    if ( (parameters != 0x0) && (isSummer != 0x0) )
    {
        for (i=0; i < 6; i++)
        {
            parameters[i] = m_Parameters[i]; 
        }

        parameters[6] = m_SetPoint[0]; //Set point
        parameters[7] = m_SetPoint[1]; //Set point
        parameters[8] = m_SetPoint[2]; //Set point
        parameters[9] = m_CtrlVariable.value;
        
        *isSummer = m_IsSummer;
        
        type = Device_strings[DEV_PIDLMD];
        
        retVal = true;
    }
    
    return retVal;
}

///////////////////////////////////////////////////
//              SetParameters
///////////////////////////////////////////////////
bool CPIDLMD::SetParameters( float * parameters, int nOfParam )
{
    bool retVal = false;
    int i = 0;
    
    if (parameters != 0x0)
    {
        for (i = 0; i<nOfParam; i++)
        {
            m_Parameters[i] = parameters[i];
        }
        
        if (m_Parameters[1]>0.0)
        {
            m_MaxIntegralError[0] = 255.0/m_Parameters[1];
        }
        else
        {
            m_Parameters[1] = 1.0;
            m_MaxIntegralError[0] = 255.0;
        }
        
        if (m_Parameters[4]>0.0)
        {
            m_MaxIntegralError[1] = 255.0/m_Parameters[4];
        }
        else
        {
            m_Parameters[4] = 1.0;
            m_MaxIntegralError[1] = 255.0;
        }
        
        m_PidVector[DIRECT_PID].SetPIDParams(m_Parameters);
        m_PidVector[LMMIN_PID].SetPIDParams(&m_Parameters[3]);
        m_PidVector[LMMAX_PID].SetPIDParams(&m_Parameters[3]);
        
        for (i = 0 ; i < 3; i++)
        {
            m_PidVector[i].InitPID();
        }
        
        retVal = true;
    }
    
    return retVal;
}

///////////////////////////////////////////////////
//              CheckHandles
///////////////////////////////////////////////////
bool CPIDLMD::CheckHandles( )
{
    if ( (!m_CtrlVariable.isRemoted) && (m_InputDevices[0] == 0x0))
    {
        PushError( AFOERROR_PID_IO_NOT_VALID, m_NetNumber, m_DeviceNumber);
        return false;
    }

    if ( (!m_LmdVariable.isRemoted) && (m_InputDevices[1] == 0x0))
    {
        PushError( AFOERROR_PID_IO_NOT_VALID, m_NetNumber, m_DeviceNumber);
        return false;
    }

    return true;
}

///////////////////////////////////////////////////
//              GetValFromTimer
///////////////////////////////////////////////////
void CPIDLMD::GetValFromTimer( )
{   int actualDigitalState = 0;

    if (m_UseCreativeTimer)
    {
        //Se usiamo il timer creativo controllo nella stessa fascia oraria se i digital sono passati da 0 a 1: in questo caso si considera il
        // sistema avviato e ripristino l'ultimo valore di setpoint altrimenti (se i digital sono a 0) lo prendo dal timer classico
        actualDigitalState = m_Timer->GetValue( m_TimerID, TIMERVAL_DIGITAL);
        
//         cout << "actualDigitalState: " << actualDigitalState << " m_LastDigital: " << m_LastDigital << endl;
        
        if ((m_LastDigital == 0) && (actualDigitalState == 1))
        {
            m_SetPoint[0] = m_LastSetpoint;
            m_LastDigital = actualDigitalState;
        }
        else if ((actualDigitalState == 0) && (m_LastDigital == 1))
        {
            m_LastSetpoint = m_SetPoint[0];
            m_LastDigital = actualDigitalState;
        }
        else if ((actualDigitalState == 0) && (m_LastDigital == 0))
        {
            m_SetPoint[0] = m_Timer->GetValue( m_TimerID, m_TypeOfTimerVal );
        } 
    }
    else
    {
        m_SetPoint[0] = m_Timer->GetValue( m_TimerID, m_TypeOfTimerVal );
    }
   
}

///////////////////////////////////////////////////
//              GetValFromInput
///////////////////////////////////////////////////
float CPIDLMD::GetValFromInput( int inputIndex, bool updateData = false )
{
    float retVal = TEMP_ERRVAL;
    CString deviceName;
        
    deviceName = m_InputDevices[inputIndex]->GetName();

//     retVal = ((CDS18X20*)(m_InputDevices[inputIndex]))->ReadTemperature(false);
    
    if ( 
         (deviceName == Device_strings[DEV_DS18S20]) || 
         (deviceName == Device_strings[DEV_DS18B20]) ||
         (deviceName == Device_strings[DEV_DS18X20])
       )
    {
        retVal = ((CDS18X20*)(m_InputDevices[inputIndex]))->ReadTemperature(updateData);
    }
    else if (deviceName == Device_strings[DEV_HUMIDITY])
    {
        retVal = ((CHumController*)(m_InputDevices[inputIndex]))->ReadTemperature(true);
    }

    return retVal;
}

///////////////////////////////////////////////////
//              VerifyIOPresence
///////////////////////////////////////////////////
bool CPIDLMD::VerifyIOPresence( )
{
    return (m_InputDevices[0]->VerifyPresence() && m_InputDevices[1]->VerifyPresence() && m_OutputDev->VerifyPresence());
}

///////////////////////////////////////////////////
//              InitPID
///////////////////////////////////////////////////
bool CPIDLMD::InitPID( )
{
    bool retVal = true;

    m_IntegralState[0] = m_IntegralState[1] = m_IntegralState [2] = 0;
    m_DerivativeState [0] = m_DerivativeState[1] = m_DerivativeState[2] = 0;
    
    if ((m_TimerID > 0) && m_UseCreativeTimer)
    {
        m_LastDigital = m_Timer->GetValue( m_TimerID, TIMERVAL_DIGITAL);
        m_LastSetpoint = m_SetPoint[0];
    }
        
    m_PidVector[DIRECT_PID].SetPIDParams(m_Parameters);
    m_PidVector[DIRECT_PID].SetSetPoint(m_SetPoint[0]);
    m_PidVector[DIRECT_PID].SetSummer(m_IsSummer);

    m_PidVector[LMMIN_PID].SetPIDParams(&m_Parameters[3]);
    m_PidVector[LMMIN_PID].SetSummer(false);
    m_PidVector[LMMIN_PID].SetSetPoint(m_SetPoint[2]);

    m_PidVector[LMMAX_PID].SetPIDParams(&m_Parameters[3]);
    m_PidVector[LMMAX_PID].SetSummer(true);
    m_PidVector[LMMAX_PID].SetSetPoint(m_SetPoint[1]);
        
    for (int i = 0 ; i < 3; i++)
    {
        m_PidVector[i].InitPID();
    }
    
    return retVal;

}
///////////////////////////////////////////////////////////////////////////////////
void CPIDLMD::NormalizeInputs(float * ctrlVar, float * setPoint, int index)
{
    if (m_IsTemp)
    {
        if (index == 0)
        {
            //Normalize the inputs
            *ctrlVar = m_normalizeM * m_CtrlVariable.value + m_normalizeQ;
        }
        else
        {
            *ctrlVar = m_normalizeM * m_LmdVariable.value + m_normalizeQ;
        }
        
        *setPoint = m_SetPoint[index];
    }
    else
    {
        if (index == 0)
        {
            *ctrlVar = m_CtrlVariable.value;
        }
        else
        {
            *ctrlVar = m_LmdVariable.value;
        }
        
        //In questo caso uso M come fine scala e Q copme inizio scala e scalo il setpoint tra 0 e 10V
        *setPoint = (m_SetPoint[index]*10.0)/(m_normalizeM - m_normalizeQ);
    }
}


//////////////////////////////////////////////////////////
bool CPIDLMD::ConnectDevices(void * netVoidPtr)
{
    int devIndex = 0;
    CVDevice *in1Device, *in2Device, *outDevice;
    COneWireNet* netPtr = reinterpret_cast<COneWireNet*> ( netVoidPtr );
    CString ioIndexStr;
    

    //Get Handles to the devices
    m_IniLib.GetConfigParamString(m_ConfigString.c_str(), "INPUT1", &ioIndexStr, "NA");
    if (ioIndexStr.ToLower() == "remote")
    {
        m_CtrlVariable.isRemoted = true;
        in1Device = 0x0;
    }
    else
    {
        m_IniLib.GetConfigParamInt ( m_ConfigString.c_str(), "INPUT1", &devIndex, -1 );
        in1Device = netPtr->GetDeviceHndlrByConfigNumber(m_NetNumber-1, devIndex);

        CHECK_PTR_BOOL(in1Device)
    }

    m_IniLib.GetConfigParamString(m_ConfigString.c_str(), "INPUT2", &ioIndexStr, "NA");
    if (ioIndexStr.ToLower() == "remote")
    {
        m_LmdVariable.isRemoted = true;
        in2Device = 0x0;
    }
    else
    {
        m_IniLib.GetConfigParamInt ( m_ConfigString.c_str(), "INPUT2", &devIndex, -1 );
        in2Device = netPtr->GetDeviceHndlrByConfigNumber(m_NetNumber-1, devIndex);

        CHECK_PTR_BOOL(in2Device)
    }
    
    m_IniLib.GetConfigParamInt ( m_ConfigString.c_str(), "OUTPUT", &devIndex, -1 );
    outDevice = netPtr->GetDeviceHndlrByConfigNumber(m_NetNumber-1, devIndex);

    CHECK_PTR_BOOL(outDevice)

    return SetInputOutputDevices(in1Device, in2Device, outDevice);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPIDLMD::Update2(bool updateData)
{
    
    t_DataVal controlDriverData = m_InputDevices[0]->GetDriverData();
    t_DataVal limitDriverData = m_InputDevices[1]->GetDriverData();
    int newOutput;
    
    COneWireEngine* engPtr = reinterpret_cast<COneWireEngine*>(m_EnginePtr);
    
    //Acquisisco gli ingressi
    if (controlDriverData.isValid && limitDriverData.isValid)
    {
        //Acquisisco gli ingressi
        m_controlVar = controlDriverData.floatData[0];
        m_limitVar = limitDriverData.floatData[0];
        
        //Integro
        m_LastPositionVolt = (m_PidVector[DIRECT_PID].UpdatePID(m_controlVar) - m_PidVector[LMMIN_PID].UpdatePID(m_limitVar) - m_PidVector[LMMAX_PID].UpdatePID(m_limitVar))/100.0;

        if (m_LastPositionVolt < 0.0)
        {
            m_LastPositionVolt = 0.0;
        }
        else if (m_LastPositionVolt > 10.0)
        {
            m_LastPositionVolt = 10.0;
        }
    
        //Scrivo sulla uscita
        newOutput = 255 - 255*((int)m_LastPositionVolt)/10;
        
        //Scrivo sull'uscita
        if (newOutput != m_LastPosition)
        {
            bool retVal = ((CDS2890*)(m_OutputDev))->SetPosition((unsigned char)newOutput);
            if (retVal)
            {
                m_LastPosition = newOutput;
            }
        }
        
        if (m_DebugLevel)
        {
            cout << "PIDLMD -- Address: "<<m_Address
                    <<" Setpoint: "<<m_SetPoint[0]<<" Input: "<<m_controlVar<<" InputLimit: "<<m_limitVar
                    <<" Out: "<<m_LastPosition<<" Out(v): "<<m_LastPositionVolt
                    <<" Summer: "<<m_IsSummer<<" Comment: "<<m_Comment<<endl;
        }
        
        if (engPtr->CheckInterfacePortsForConnection())
        {
            Cmd com("DEVICE");

            com.putValue("TYPE","PIDOutput");
            com.putValue("ADDRESS",m_Address);
            com.putValue("VAL",m_LastPositionVolt);
            com.putValue("SETPOINT",m_SetPoint[0]);
            com.putValue("SUMMER", m_IsSummer);
            com.putValue("INPUT",m_controlVar);
            com.putValue("LMDINPUT",m_limitVar);
            com.putValue("TEMP",m_controlVar); //Solo per compatibilita'
            
            engPtr->WriteOnInterfacePorts(com.toString().c_str(), com.toString().size());
        }
    }

    return true;
}


bool CPIDLMD::ExecCommand(CXMLUtil* xmlUtil)
{
    bool retVal = false;
    e_CommandTypes command = (e_CommandTypes)ParseCommand(xmlUtil);
    COneWireEngine *engPtr = reinterpret_cast<COneWireEngine*>(m_EnginePtr);
    CIniFileHandler iniFile;

    switch (command)
    {
        case COMM_GETPIDINFO:
        {
            //Il formato è il seguente:
            //NOFPOINTS= POINT1=text:tmnd

            Cmd com("DEVICE");
            com.putValue("TYPE", "PIDInfo");
            com.putValue("ADDRESS", m_Address);
            com.putValue("KAPPA",m_Parameters[0]);
            com.putValue("TINT",m_Parameters[1]);
            com.putValue("TDER",m_Parameters[2]);
            com.putValue("SETPOINT",m_SetPoint[0]);
            com.putValue("SUMMER",m_IsSummer);
            com.putValue("TEMP",m_controlVar);
            com.putValue("KAPPA2",m_Parameters[3]);
            com.putValue("TINT2",m_Parameters[4]);
            com.putValue("TDER2",m_Parameters[5]);
            com.putValue("SETPOINTH",m_SetPoint[1]);
            com.putValue("SETPOINTL",m_SetPoint[2]);


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
