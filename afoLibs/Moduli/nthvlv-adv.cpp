/***************************************************************************
 *   Copyright (C) 2007 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it                                                *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 ***************************************************************************/
#include "nthvlv-adv.h"

/////////////////////////////////////////////////////
//                   Standard costructor
/////////////////////////////////////////////////////
CNTHVLV_ADV::CNTHVLV_ADV(const char* configString, CTimer *timer): CVController(configString)
{
    int tempVal = 0;
    float setpoint = 0.0;
    bool isPIDLMD = true;

    memset (m_StateVector, 0x0, 17*sizeof(uchar));

    if (configString != 0x0)
    {
        //Set the flag start
        m_StateVector[0] = 0xFF;
        m_StateVector[1] = 0xFF;
        m_StateVector[2] = 0xFF;

        //Get PID parameters
        m_IniLib.GetConfigParamInt( configString, "KP1",(int*)( &m_StateVector[3]), 10);
        m_IniLib.GetConfigParamInt( configString, "Tint1", (int*)(&m_StateVector[4]), 0);
        m_IniLib.GetConfigParamInt( configString, "Tder1", (int*)(&m_StateVector[5]), 1);


        m_IniLib.GetConfigParamInt( configString, "KP2", &tempVal, -1);
        if (tempVal < 0 )
        {
            isPIDLMD = false;
        }
        else
        {
            m_StateVector[6] = (uchar)tempVal;
        }

        m_IniLib.GetConfigParamInt( configString, "Tint2", &tempVal, -1);
        if (tempVal < 0 )
        {
            isPIDLMD = false;
        }
        else
        {
            m_StateVector[7] = (uchar)tempVal;
        }

        m_IniLib.GetConfigParamInt( configString, "Tder2", &tempVal, -1);
        if (tempVal < 0 )
        {
            isPIDLMD = false;
        }
        else
        {
            m_StateVector[8] = (uchar)tempVal;
        }

        //Get Setpoint
        m_IniLib.GetConfigParamFloat( configString, "SETPOINT", &setpoint, 19.0);

        if (setpoint < 10.0)
        {
            setpoint = 10.0;
        }
        else if (setpoint > 30.0)
        {
            setpoint = 30.0;
        }

        //Transform the setpoint in a value between 0 and 255 where 0 means 10C and 255 means 30C
        m_StateVector[9] = (int)(255.0*(setpoint - 10.0)/20.0);

        //Setpoint esterno (sola lettura)
        m_StateVector[10] = 0xFF;

        //TODO da chiedere a Saps come impostare i prossimi valori
        m_StateVector[11] = 0x0; //Val_DACA
        m_StateVector[12] = 0x0; //Val_DACB

        //Check if it is a PIDLmd controller
//         if (isPIDLMD)
//         {
//             m_StateVector[13] = m_StateVector[13] | 0x04;
//         }

        //TODO Azzero PID1 e PID2 per usare il funzionamento speciale 1, da mettere sotto parametro
        //m_StateVector[13] = m_StateVector[13] & 0xF3;

        //Check if it is summer
        m_IniLib.GetConfigParamBool( configString, "SUMMER", &m_IsSummerForModule, false);
//        if (m_IsSummer) m_IsSummerForModule = false;
//        else m_IsSummerForModule = true;
//         if (tempVal)
//         {
//             m_StateVector[13] = m_StateVector[13] & 0xFD;
//         }
//         else
//         {
//             m_StateVector[13] = m_StateVector[13] | 0x02 ;
//         }


        m_IniLib.GetConfigParamInt(configString, "SETPOINTH", &tempVal, 35);
        m_StateVector[14] = (uchar)(tempVal);

        m_IniLib.GetConfigParamInt(configString, "SETPOINTL", &tempVal, 15);
        m_StateVector[15] = (uchar)(tempVal);



        //Get hysteresis
        m_IniLib.GetConfigParamInt( configString, "HYST", &m_Hysteresis, 3);

        //Get Division factor
        m_IniLib.GetConfigParamInt( configString, "DIVFACTOR", &m_DivFactor, 0);


        //Set FLAG_START
        m_StateVector[16] = 0x0;


    }

    m_Device= 0x0;

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

    m_IsOutputOn = false;
    m_FirstUpdate = false;
    m_ModuleInitOK = false;

    m_TypeOfTimerVal = TIMERVAL_DIGITAL;
    m_ControllerType = DEV_VLVCTRL;
}

/////////////////////////////////////////////////////
//                   Standard destructor
/////////////////////////////////////////////////////
CNTHVLV_ADV::~CNTHVLV_ADV()
{
}

/////////////////////////////////////////////////////
//                   Update
/////////////////////////////////////////////////////
bool CNTHVLV_ADV::Update(bool updateData)
{
    bool turnOn = false;
    bool retVal = false;
    uchar mapmem[32];

    memset (mapmem, 0x0, 32);

    //First check if the module has been correctly programmed
    if (!m_ModuleInitOK)
    {
        if (!InitDevice())
        {
            //Aggiungere errore sull'inizializzazione
            return false;
        }
    }

    //If we have a timer active get the value
    if (m_UseTimer && IsTimerEnabled())
    {
        turnOn = GetValFromTimer();

        if (!m_FirstUpdate)
        {
            retVal = ChangeDOOutput( turnOn );

            if (retVal)
            {
                m_IsOutputOn = turnOn;
                m_FirstUpdate = true;
            }
        }
        else if (turnOn != m_IsOutputOn)
        {
           retVal = ChangeDOOutput( turnOn );

            if (retVal)
            {
                m_IsOutputOn = turnOn;
            }
        }
    }
    else
    {
        retVal = true;
    }

//     memset (mapmem, 0xFF, 32);
//     memset (temp, 0xFF, 32);
//     GetAllData( 0x01, temp);
//     GetAllData( 0x03, mapmem);


    return retVal;
}

/////////////////////////////////////////////////////
//                   VerifyIOPresence
/////////////////////////////////////////////////////
bool CNTHVLV_ADV::VerifyIOPresence()
{
    return m_Device->VerifyPresence();
}


/////////////////////////////////////////////////////
//                   GetInputDevice
/////////////////////////////////////////////////////
CDS2751* CNTHVLV_ADV::GetInputDevice() const
{
  return m_Device;
}

/////////////////////////////////////////////////////
//                   SetInputDevice
/////////////////////////////////////////////////////
void CNTHVLV_ADV::SetInputDevice(CDS2751* theValue)
{
  m_Device = theValue;
}



/////////////////////////////////////////////////////
//                   SetSRAM
/////////////////////////////////////////////////////
bool CNTHVLV_ADV::SetSRAM(uchar *newSRAM)
{
    bool retVal = false;

    memcpy (m_SRAMVector, newSRAM, 16*sizeof(uchar));

    if (m_Device->WriteAllSRAM( m_SRAMVector ))
    {
        retVal = true;
    }
    else
    {
        //TODO inserire errore
    }

    return retVal;
}


/////////////////////////////////////////////////////
//                   GetAllData
/////////////////////////////////////////////////////
bool CNTHVLV_ADV::GetAllData(uchar address, uchar *destination)
{
    bool retVal = false;
    uchar mapmem[32];

    memset (mapmem, 0xFF, 16*sizeof(uchar));

    if (m_Device != 0x0)
    {
       //Set command
        mapmem[0] = address;

        //Update the SRAM
        if (m_Device->WriteAllSRAM( mapmem ))
        {
            //Move special PIN ti signal to the device
            if (m_Device->SetSpecialReg( 0x80 ))
            {
                msDelay( 1500 );

                if (CheckCommandExecution( mapmem[0]))
                {
                    switch (address)
                    {
                        case 0x01:
                        {
                            //Everything ok, copy the first 15 bytes coming from the device
                            memcpy (destination, m_SRAMVector+1, 14*sizeof(uchar));

                            //Append the extra values from the state vector
                            destination[14] = m_StateVector[14]; //Setpoint H
                            destination[15] = m_StateVector[15]; //Setpoint L
                            destination[16] = m_StateVector[16]; //FLAG_START
                            retVal = true;
                        };break;
                        case 0x03:
                        {
                            //Everything ok, copy the first 15 bytes coming from the device
                            memcpy (destination, m_SRAMVector+1, 15*sizeof(uchar));

                            retVal = true;
                        };break;
                        default: break;
                    }

                }
             }
            else
            {
                PushError( AFOERROR_CTRLVLV_UNABLE_TO_SET_SPECREG, m_NetNumber, m_DeviceNumber );
            }
        }
        else
        {
            PushError( AFOERROR_CTRLVLV_UNABLE_TO_WRITE_SRAM, m_NetNumber, m_DeviceNumber );
        }
    }

    return retVal;

}


/////////////////////////////////////////////////////
//                   GetBasicdata
/////////////////////////////////////////////////////
bool CNTHVLV_ADV::GetBasicdata(float *tempInt, float *tempExt, float *setpoint, bool *isSummer)
{
    bool retVal = false;

    //Get data from device
    if (GetAllData( 0x01, m_StateVector ))
    {
        *tempInt = m_StateVector[0];
        *tempExt = m_StateVector[1];

        //Check sign of temps
        if (m_StateVector[2] & 0x0F)
        {
            *tempInt = 0.0 - *tempInt;
        }

        if (m_StateVector[2] & 0xF0)
        {
            *tempExt = 0.0 - *tempExt;
        }

        *setpoint = m_StateVector[9] * 20.0/255.0 + 10.0; //Re-transform it from 0-255 to 10C - 30C

        if (m_StateVector[13] & 0x02)
        {
            *isSummer = false;
        }
        else
        {
            *isSummer = true;
        }
        
        printf("Out1:%2x, Out2:%2x FLSTART:%2x, ByteImpostazioni:%2x", m_StateVector[11],m_StateVector[12],m_StateVector[16],m_StateVector[13]);

        retVal = true;
    }
    else
    {
        *tempInt = TEMP_ERRVAL;
        *tempExt = TEMP_ERRVAL;
        *setpoint = TEMP_ERRVAL;
    }

    return retVal;
}


/////////////////////////////////////////////////////
//                   GetSRAM
/////////////////////////////////////////////////////
bool CNTHVLV_ADV::GetSRAM (uchar *destination)
{
    bool retVal = false;

    if (m_Device->GetSRAM( destination ))
    {
        retVal = true;
    }

    return retVal;
}

/////////////////////////////////////////////////////
//                   SetSetPoint
/////////////////////////////////////////////////////
bool CNTHVLV_ADV::SetSetPoint( float newSetPoint )
{
    bool retVal = false;
    uchar oldVal;

    //Save previous value
    oldVal = m_StateVector[9];

    //Update the state vector
    //Transform the setpoint in a value between 0 and 255 where 0 means 10C and 255 means 30C
    if (newSetPoint < 10.0)
    {
        newSetPoint = 10.0;
    }
    else if (newSetPoint > 30.0)
    {
        newSetPoint = 30.0;
    }

    m_StateVector[9] = (int)(255.0*(newSetPoint - 10.0)/20.0);

    //Write it to the device
    if (WriteStateVector(m_StateVector))
    {
        //Everything OK!!
        retVal = true;
    }
    else
    {
        //write failed, restore previous setpoint value to keep state vector equal to the device
        m_StateVector[9] = oldVal;
    }

    return retVal;
}




/////////////////////////////////////////////////////
//                   CheckCommandExecution
/////////////////////////////////////////////////////
bool CNTHVLV_ADV::CheckCommandExecution( uchar commandChar )
{
    bool retVal = false;
    uchar specialPIN;
    uchar commandCheck;

    if (m_Device->GetSpecialReg( &specialPIN ))
    {
        if (m_Device->GetSRAM( m_SRAMVector ))
        {
            commandCheck = (~commandChar) - 1;
            if ((specialPIN == 0xFF) && (commandCheck == m_SRAMVector[0]))
            {
                retVal = true;
            }
            else
            {
                PushError( AFOERROR_CTRLVLV_UNABLE_TO_EXEC_COMMAND, m_NetNumber, m_DeviceNumber );
            }
        }
        else
        {
            PushError( AFOERROR_CTRLVLV_UNABLE_TO_READ_SRAM, m_NetNumber, m_DeviceNumber );
        }
    }
    else
    {
        PushError( AFOERROR_CTRLVLV_UNABLE_TO_GET_SPECREG, m_NetNumber, m_DeviceNumber );
    }

    return retVal;
}

/////////////////////////////////////////////////////
//                   WriteStateVector
/////////////////////////////////////////////////////
bool CNTHVLV_ADV::WriteStateVector(uchar *newStateVector )
{
    uchar mapmem[15];
    int i = 0;

    //Start building the vector that will be written to the device
    //FLAG_START
    mapmem[0] = newStateVector[16];

    //Setpoint H and L
    mapmem[1] = newStateVector[14];
    mapmem[2] = newStateVector[15];

    //P1,I1,D1,P2,I2,D2
    for (i = 0; i < 6; i++)
    {
        mapmem[i+3] = newStateVector[i+3];
    }

    //Setpoint
    mapmem[9] = newStateVector[9];

    //Always 0xFF
    mapmem[10] = 0xFF;

    //DAC A and B
    mapmem[11] = newStateVector[10];
    mapmem[12] = newStateVector[11];

    //Flag_state
    mapmem[13] = newStateVector[13];

    //Hysteresis
    mapmem[14] = m_Hysteresis;

    return SetAllData( 0x02, mapmem );
}

/////////////////////////////////////////////////////
//                   WriteStateVector
/////////////////////////////////////////////////////
bool CNTHVLV_ADV::ChangeDOOutput(bool turnOn )
{
    uchar mapmem[15];
    int i = 0;

    //Start building the vector that will be written to the device
    //FLAG_START
    mapmem[0] = m_StateVector[16];

    //Setpoint H and L
    mapmem[1] = m_StateVector[14];
    mapmem[2] = m_StateVector[15];

    //P1,I1,D1,P2,I2,D2
    for (i = 0; i < 6; i++)
    {
        mapmem[i+3] = m_StateVector[i+3];
    }

    //Setpoint
    mapmem[9] = m_StateVector[9];

    //Always 0xFF
    if (turnOn)
    {
        mapmem[10] = 0xFF;
    }
    else
    {
        mapmem[10] = 0x0;
    }

    //DAC A and B
    mapmem[11] = m_StateVector[10];
    mapmem[12] = m_StateVector[11];

    //Flag_state
    mapmem[13] = m_StateVector[13];

    //CRC -- not implemented
    mapmem[14] = 0xFF;

    return SetAllData( 0x02, mapmem );
}

/////////////////////////////////////////////////////
//                   InitDevice
/////////////////////////////////////////////////////
bool CNTHVLV_ADV::InitDevice()
{
    bool retVal = false;
    uchar mapmem[32];
    uchar potentiometersState;

    //First extract the settings from the device:
    if (GetAllData( 0x01, mapmem ))
    {
        //Register the state of the potentiometers
        //potentiometersState = mapmem[13] & 0x10;

        //Update the state vector

        //TODO Non va bene perchè se devo resettare il bit non funziona
        //m_StateVector[13] = m_StateVector[13] | potentiometersState;
        m_StateVector[13] = mapmem[13];
        
        //if (m_IsSummer)

        //18/12/2009 Modificato per fare si che il sistema sia coerente
        if (m_IsSummerForModule)
        {
            m_StateVector[13] = m_StateVector[13] & 0xFD;
        }
        else
        {
            m_StateVector[13] = m_StateVector[13] | 0x02;
        }



        //Write advanced parameters
        memset (mapmem, 0xFF, 32*sizeof(uchar));
        mapmem[0] = m_DivFactor;

        //Write the configuration in the device
        if ((WriteStateVector(m_StateVector)) && (SetAllData( 0x04, mapmem )))
        {
            //Init ok
            m_StateVector[16] = 0x0;

            //Module correctly initialized
            m_ModuleInitOK = true;

            //Adjust for timers
            Update( true );

            retVal = true;
        }
    }

    //Update the init flag
    m_ModuleInitOK = retVal;


    return retVal;
}

/////////////////////////////////////////////////////
//                   SetAllData
/////////////////////////////////////////////////////
bool CNTHVLV_ADV::SetAllData(uchar address, uchar *newData)
{
    bool retVal = false;
    uchar mapmem[32];

    memset (mapmem, 0x0, 32*sizeof(uchar));

    if (m_Device != 0x0)
    {
       //Set command
        mapmem[0] = address;

        //Copy all status vector
        memcpy (mapmem+1, newData, 15*sizeof(uchar));

        //Update the SRAM
        if (m_Device->WriteAllSRAM( mapmem ))
        {
            //Move special PIN ti signal to the device
            if (m_Device->SetSpecialReg( 0x80 ))
            {

                msDelay( 1500 );

                if (CheckCommandExecution( mapmem[0]))
                {
                    switch (address)
                    {
                        case 0x02:
                        {
                            //Everything ok, save new state
//                             memcpy (m_StateVector, newData, 15*sizeof(uchar));
                            retVal = true;
                        };break;
                        case 0x04:
                        {
                            retVal = true;
                        };break;
                        default: break;
                    }
                }
            }
            else
            {
                PushError( AFOERROR_CTRLVLV_UNABLE_TO_SET_SPECREG, m_NetNumber, m_DeviceNumber );
            }
        }
        else
        {
            PushError( AFOERROR_CTRLVLV_UNABLE_TO_WRITE_SRAM, m_NetNumber, m_DeviceNumber );
        }
    }

    return retVal;
}


/*!
    \fn CNTHVLV_ADV::SetSummer(bool isSummer)
 */
bool CNTHVLV_ADV::SetSummer(bool isSummer)
{

//    if (isSummer) m_IsSummerForModule = false;
//        else m_IsSummerForModule = true;

    if (isSummer)
    {
        m_StateVector[13] = m_StateVector[13] & 0xFD;
    }
    else
    {
        m_StateVector[13] = m_StateVector[13] | 0x02 ;
    }

    return WriteStateVector( m_StateVector );
}
