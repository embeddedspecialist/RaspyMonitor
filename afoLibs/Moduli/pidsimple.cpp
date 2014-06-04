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
#include "pidsimple.h"
#include "conewireengine.h"

CPIDSimple::CPIDSimple(const char* configString, CVDevice *inDevice, CVDevice *outDevice, CTimer *timer)
    : CVPID(configString)
{
    char tempBuff[8];
    
    memset (tempBuff, 0x0, 8);
    
    m_InputDev = inDevice;

    m_AllowsNegativeCalc = false;
    
    //Get default setpoint
    m_IniLib.GetConfigParamFloat(configString, "SETPOINT",  &m_SetPoint, 20.0);
    
    //Get default controller parameters
    m_IniLib.GetConfigParamFloat(configString, "KP",  &m_Parameters[0], 10.0);
    m_IniLib.GetConfigParamFloat(configString, "Tint",  &m_Parameters[1], 1.0);
    m_IniLib.GetConfigParamFloat(configString, "Tder",  &m_Parameters[2], 0.0);
    
    m_IntegralState = 0;
    m_DerivativeState = 0;
    if (m_Parameters[1]>0.0)
    {
        m_MaxIntegralError = 255.0/m_Parameters[1];
    }
    else
    {
      m_Parameters[1] = 1.0;
      m_MaxIntegralError = 255.0;
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

    m_controlVar = 85.0;
    //Get the normalizing factors:
    m_IniLib.GetConfigParamFloat(configString, "MFACTOR",  &m_normalizeM, 1.0);
    m_IniLib.GetConfigParamFloat( configString, "QFACTOR",  &m_normalizeQ, 0.0);
    
    m_TypeOfTimerVal = TIMERVAL_PID;
    m_ControllerType = DEV_PIDSIMPLE;
}

CPIDSimple::CPIDSimple( const char* configString, CTimer * timer ): CVPID(configString)
{
   
    m_InputDev = 0x0;
    m_OutputDev = 0x0;

    m_AllowsNegativeCalc = false;
    
    //Get default setpoint
    m_IniLib.GetConfigParamFloat(configString, "SETPOINT",  &m_SetPoint, 20.0);
    
    //Get default controller parameters
    m_IniLib.GetConfigParamFloat(configString, "KP",  &m_Parameters[0], 10.0);
    m_IniLib.GetConfigParamFloat(configString, "Tint",  &m_Parameters[1], 1.0);
    m_IniLib.GetConfigParamFloat(configString, "Tder",  &m_Parameters[2], 0.0);
    
    m_IntegralState = 0;
    m_DerivativeState = 0;
    if ((m_Parameters[1] < 1.0) && (m_Parameters[1]>0.0))
    {
        m_MaxIntegralError = 255.0/m_Parameters[1];
    }
    else
    {
        m_MaxIntegralError = 255.0;
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
    
    m_TypeOfTimerVal = TIMERVAL_PID;
    m_ControllerType = DEV_PIDSIMPLE;
}

CPIDSimple::~CPIDSimple()
{
}
////////////////////////////////////////////////////////////////////////////////////////////
bool CPIDSimple::SetInputOutputDevices( CVDevice * inDevice, CVDevice * outDevice )
{
    if (!m_CtrlVariable.isRemoted)
    {
        if ((inDevice->GetFamNum() != DS18S20_FN) && (inDevice->GetFamNum() != DS18B20_FN) && (inDevice->GetFamNum() != DS2438_FN))
        {
            return false;
        }

        m_InputDev = inDevice;
    }

    if (outDevice->GetFamNum() != DS2890_FN)
    {
        return false;
    }
    
    m_OutputDev = outDevice;

    return true;
}

///////////////////////////////////////////////////
//              CheckHandles
///////////////////////////////////////////////////
bool CPIDSimple::CheckHandles( )
{
    if ( (!m_CtrlVariable.isRemoted) && (m_InputDev == 0x0))
    {
        PushError( AFOERROR_PID_IO_NOT_VALID, m_NetNumber, m_DeviceNumber);
        return false;
    }
    
    if (m_OutputDev == 0x0)
    {
        PushError( AFOERROR_PID_IO_NOT_VALID, m_NetNumber, m_DeviceNumber);
        return false;
    }

    return true;
}

///////////////////////////////////////////////////
//              Update
///////////////////////////////////////////////////
bool CPIDSimple::Update( bool updateData )
{
    float error = 0.0, normalizedSetpoint = 0.0, normalizedDeviceInput = 0.0, pidOutput = 0.0;
    bool retVal = false;
    int newPosition = m_LastPosition;

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
        if (!m_CtrlVariable.isRemoted)
        {
            m_CtrlVariable.value = GetValFromInput();
            m_CtrlVariable.updated = true;
        }
        else
        {
            if (!m_CtrlVariable.updated)
            {
                //Se non e' aggiornata non e' proprio un errore, semplicemente non posso andare avanti
                return true;
            }
        }
        //Get temperature
        if ( m_CtrlVariable.value != TEMP_ERRVAL )
        {
            //"Consumo" lo stato di aggiornata della variabile
            m_CtrlVariable.updated = false;
            
            //If we have a timer active get the value
            if (m_UseTimer && IsTimerEnabled())
            {
                m_SetPoint = GetValFromTimer();
            }

            NormalizeInputs(&normalizedDeviceInput, &normalizedSetpoint);

            error = GetError(normalizedDeviceInput, normalizedSetpoint);


            //Apply the PID parameters
            pidOutput = m_Parameters[0]*error + Integrate( m_Parameters[1], error ) + Derivate( m_Parameters[2], error );

            newPosition = 255 - (int)pidOutput;

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
                retVal = ((CDS2890*)(m_OutputDev))->SetPosition((unsigned char)newPosition);
                if (retVal)
                {
                    m_LastPosition = newPosition;
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
    }

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
//              Integrate
///////////////////////////////////////////////////
float CPIDSimple::Integrate( float Ki, float value )
{
    m_IntegralState += value;

    if (m_AllowsNegativeCalc)
    {
        if (m_IntegralState > m_MaxIntegralError)
        {
            m_IntegralState = m_MaxIntegralError;
        }
        else if (m_IntegralState < (0 - m_MaxIntegralError))
        {
            m_IntegralState = 0 - m_MaxIntegralError;
        }
    }
    else
    {
        if (m_IntegralState > m_MaxIntegralError)
        {
            m_IntegralState = m_MaxIntegralError;
        }
        else if ( m_IntegralState < 0.0)
        {
            m_IntegralState = 0.0;
        }
    }
    
    return m_IntegralState*Ki;
}

///////////////////////////////////////////////////
//              Derivate
///////////////////////////////////////////////////
float CPIDSimple::Derivate( float Kd, float value )
{
    float retVal = 0.0;

    retVal = Kd*(value - m_DerivativeState);
    
    m_DerivativeState = value;
    
    return retVal;
}

///////////////////////////////////////////////////
//              SetSetPoint
///////////////////////////////////////////////////
bool CPIDSimple::SetSetPoint( float *newSP, int nOfParam )
{   
    m_SetPoint = *newSP;
    m_PidVector[DIRECT_PID].SetSetPoint(*newSP);
    
    return true;
        
}

///////////////////////////////////////////////////
//              SetSummer
///////////////////////////////////////////////////
bool CPIDSimple::SetSummer( bool summerSet )
{
    m_IsSummer = summerSet;
    m_PidVector[DIRECT_PID].SetSummer(m_IsSummer);
    
    return true;
}

///////////////////////////////////////////////////
//              GetPIDInfo
///////////////////////////////////////////////////
bool CPIDSimple::GetInfo( float * parameters, bool * isSummer, CString &type )
{
    bool retVal = false;
    
    if ( (parameters != 0x0) && (isSummer != 0x0) )
    {
        parameters[0] = m_Parameters[0]; //Kappa
        parameters[1] = m_Parameters[1]; //Tint
        parameters[2] = m_Parameters[2]; //Tder
        parameters[3] = m_SetPoint; //Set point
        parameters[4] = m_CtrlVariable.value;
        
        *isSummer = m_IsSummer;
        
        type = Device_strings[DEV_PIDSIMPLE];
        retVal = true;
    }
    
    return retVal;
}

///////////////////////////////////////////////////
//              SetParameters
///////////////////////////////////////////////////
bool CPIDSimple::SetParameters( float * parameters, int nOfParam )
{
    bool retVal = false;
    
    if (parameters != 0x0)
    {
        m_Parameters[0] = parameters[0]; //Kappa
        m_Parameters[1] = parameters[1]; //Tint
        m_Parameters[2] = parameters[2]; //Tder
        
        if (m_Parameters[1]>0.0)
        {
            m_MaxIntegralError = 255.0/m_Parameters[1];
        }
        else
        {
          m_Parameters[1] = 1.0;
          m_MaxIntegralError = 255.0/m_Parameters[1];
        }
        
        ////////////////////////////////////////////////////
        //NUOVE FUNZIONI
        ////////////////////////////////////////////////////
        m_PidVector[DIRECT_PID].SetPIDParams(m_Parameters);
        m_PidVector[DIRECT_PID].InitPID();
                
        retVal = true;
    }
    
    return retVal;
        
}

///////////////////////////////////////////////////
//              GetValFromInput
///////////////////////////////////////////////////
float CPIDSimple::GetValFromInput( )
{
    float retVal = TEMP_ERRVAL;
    CString deviceName;
    
    deviceName = m_InputDev->GetName();
    
    if (m_IsTemp)
    {
        if ( 
            (deviceName == Device_strings[DEV_DS18S20]) || 
            (deviceName == Device_strings[DEV_DS18B20]) ||
            (deviceName == Device_strings[DEV_DS18X20])
        )
        {
        retVal = ((CDS18X20*)(m_InputDev))->ReadTemperature(false);
        }
        else if (deviceName == Device_strings[DEV_DS2438])
        {
            retVal = ((CDS2438*)(m_InputDev))->ReadTemperature(true);
        }
    }
    else
    {
        if (deviceName == Device_strings[DEV_DS2438])
        {
            retVal = ((CDS2438*)(m_InputDev))->ReadVoltage(false);
            
            if (retVal == ANALOG_ERRVAL)
            {
                retVal = TEMP_ERRVAL;
            }
        }
    }
    


    return retVal;
}

///////////////////////////////////////////////////
//              VerifyIOPresence
///////////////////////////////////////////////////
bool CPIDSimple::VerifyIOPresence( )
{
    return (m_InputDev->VerifyPresence() && m_OutputDev->VerifyPresence());
}

///////////////////////////////////////////////////
//              InitPID
///////////////////////////////////////////////////
bool CPIDSimple::InitPID( )
{
    m_IntegralState = 0;
    m_DerivativeState = 0;
    
    if (!m_IsTemp)
    {
        ((CDS2438*)(m_InputDev))->SetVoltageMeasurement(false);
    }
    
    ////////////////////////////////////////////////////
    //NUOVE FUNZIONI
    ////////////////////////////////////////////////////
    m_PidVector[DIRECT_PID].SetPIDParams(m_Parameters);
    m_PidVector[DIRECT_PID].SetSummer(m_IsSummer);
    m_PidVector[DIRECT_PID].SetSetPoint(m_SetPoint);
    m_PidVector[DIRECT_PID].InitPID();
    
    return true;
}

///////////////////////////////////////////////////
//              Update(2)
///////////////////////////////////////////////////
bool CPIDSimple::Update(float controlVar)
{
    float error = 0.0, normalizedDeviceInput = 0.0, pidOutput=255.0, normalizedSetpoint = 5.0;
    bool retVal = true;
        
    m_CtrlVariable.value = controlVar;
    m_CtrlVariable.updated = true;

    NormalizeInputs(&normalizedDeviceInput, &normalizedSetpoint);
    
    //Calculate the error
    error = GetError(normalizedDeviceInput, normalizedSetpoint);

    //Apply the PID parameters
    pidOutput = m_Parameters[0]*error + Integrate( m_Parameters[1], error ) + Derivate( m_Parameters[2], error );

    //Se il PID conta in negativo devo tenere conto del valore "vero" e non del fatto che lo riporto in scala da 255(min) a 0 (max)
    //Quindi se può fare calcoli negativi va da 255 a 1 e da -255 a -1
    
    if ((pidOutput<0) && (m_AllowsNegativeCalc))
    {
        m_PIDOutput = -255 -(int)pidOutput;

        if (m_PIDOutput >0)
        {
            m_PIDOutput = -1;
        }

        m_LastPosition = -m_PIDOutput;
    }
    else
    {
        m_PIDOutput = 255 - (int)pidOutput;
        //Set the analog output: 255 means the output is low, 0 is at Vmax
        if (m_PIDOutput > 255)
        {
            m_PIDOutput = 255;
        }
        else if ( m_PIDOutput < 0 )
        {
            m_PIDOutput = 0;
        }
        else if ((m_PIDOutput == 0) && (m_AllowsNegativeCalc))
        {
            m_PIDOutput = 1;
        }

        m_LastPosition = m_PIDOutput;
    }    

    m_LastPositionVolt = (float)(( 255.0 - m_LastPosition )/255.0 * 10.0);
    
    return retVal;
}
///////////////////////////////////////////////////////////////
void CPIDSimple::NormalizeInputs(float * ctrlVar, float * setPoint)
{
    if (m_IsTemp)
    {
        //Normalize the inputs
        *ctrlVar = m_normalizeM * m_CtrlVariable.value + m_normalizeQ;
        *setPoint = m_SetPoint;
    }
    else
    {
        *ctrlVar = m_CtrlVariable.value;
        //In questo caso uso M come fine scala e Q copme inizio scala e scalo il setpoint tra 0 e 10V
        *setPoint = (m_SetPoint*10.0)/(m_normalizeM - m_normalizeQ);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////
bool CPIDSimple::ConnectDevices(void * netVoidPtr)
{
    int inputIndex = -1;
    int outputIndex = -1;
    COneWireNet* netPtr = reinterpret_cast<COneWireNet*> ( netVoidPtr );
    CString ioIndexStr;
    CVDevice* inDevice, *outDevice;

    //Get Handles to the devices
    m_IniLib.GetConfigParamString(m_ConfigString.c_str(), "INPUT", &ioIndexStr, "NA");
    
    //Se l'ingresso è remotato mi fermo qui
    if (ioIndexStr.ToLower() == "remote")
    {
        m_CtrlVariable.isRemoted = true;
        inDevice = 0x0;
    }
    else
    {
        m_IniLib.GetConfigParamInt ( m_ConfigString.c_str(), "INPUT", &inputIndex, -1 );
        inDevice = netPtr->GetDeviceHndlrByConfigNumber(m_NetNumber-1, inputIndex);

        if (inDevice == 0x0)
        {
            return false;
        }
    }
    
    m_IniLib.GetConfigParamInt ( m_ConfigString.c_str(), "OUTPUT", &outputIndex, -1 );
    outDevice = netPtr->GetDeviceHndlrByConfigNumber(m_NetNumber-1, outputIndex);

    if (outDevice == 0x0)
    {
        return false;
    }

    return SetInputOutputDevices(inDevice, outDevice);
}

bool CPIDSimple::ExecCommand(CXMLUtil* xmlUtil)
{
    bool retVal = false;
    e_CommandTypes command = (e_CommandTypes)ParseCommand(xmlUtil);
    COneWireEngine *engPtr = reinterpret_cast<COneWireEngine*>(m_EnginePtr);
    CIniFileHandler iniFile;

    switch (command)
    {
        case COMM_GETPIDINFO:
        {
            Cmd com("DEVICE");
            com.putValue("TYPE", "PIDInfo");
            com.putValue("ADDRESS", m_Address);
            com.putValue("KAPPA",m_Parameters[0]);
            com.putValue("TINT",m_Parameters[1]);
            com.putValue("TDER",m_Parameters[2]);
            com.putValue("SETPOINT",m_SetPoint);
            com.putValue("SUMMER",m_IsSummer);
            com.putValue("TEMP",m_controlVar);

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
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
///////////////////  NUOVE FUNZIONI CONTROLLER ///////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

bool CPIDSimple::Update2(bool updateData)
{
    float newOutput = 0.0;
    t_DataVal driverData = m_InputDev->GetDriverData();
    COneWireEngine* engPtr = reinterpret_cast<COneWireEngine*>(m_EnginePtr);

    if (!m_IsActive)
    {
        return true;
    }

    //Acquisisco gli ingressi
    if (driverData.isValid)
    {
        //TODO attenzione che se voglio integrare un dato preso in corrente non riesco a farlo
        m_controlVar = driverData.floatData[0];
    
        //TODO da sistemare la messa in scala

        if (m_IsOn)
        {
            //Integro
            m_LastPositionVolt = m_PidVector[DIRECT_PID].UpdatePID(m_controlVar)/100.0;
        }
        else
        {
            m_LastPositionVolt = 0.0;
        }

        //Scrivo sulla uscita
        newOutput = 255 - 255*m_LastPositionVolt/10;

        
        if (newOutput != m_LastPosition)
        {
            bool retVal = ((CDS2890*)(m_OutputDev))->SetPosition((unsigned char)newOutput);
            if (retVal)
            {
                m_LastPosition = (unsigned char)newOutput;
            }
        }
        
        if (m_DebugLevel)
        {
            cout << "PIDSIMPLE -- Address: "<<m_Address
                    <<" Setpoint: "<<m_SetPoint<<" Input: "<<m_controlVar
                    <<" Out: "<<m_LastPosition<<" Out(v): "<<m_LastPositionVolt
                    <<" Summer: "<<m_IsSummer<<" Comment: "<<m_Comment<<endl;
        }
        
        if (m_SendNETMessages && (engPtr->CheckInterfacePortsForConnection()))
        {
            Cmd com("DEVICE");

            com.putValue("TYPE","PIDOutput");
            com.putValue("ADDRESS",m_Address);
            com.putValue("VAL",m_LastPositionVolt);
            com.putValue("SETPOINT",m_SetPoint);
            com.putValue("SUMMER", m_IsSummer);
            com.putValue("INPUT", m_controlVar);
            com.putValue("TEMP", m_controlVar); //Compatibilita'

            com.putValue("ISON", m_IsOn);
            com.putValue("ISACTIVE",m_IsActive);
            
            engPtr->WriteOnInterfacePorts(com.toString().c_str(), com.toString().size());
        }
    }

    return true;
}



