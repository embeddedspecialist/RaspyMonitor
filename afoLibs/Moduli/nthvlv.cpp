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
#include "nthvlv.h"

/////////////////////////////////////////////////////
//                   Standard costructor
/////////////////////////////////////////////////////
CNTHVLV::CNTHVLV(const char* configString, CTimer *timer): CVController(configString)
{
    int tempVal = 0;
    float setpoint = 0.0;
    bool isPIDLMD = true;
    
    memset (m_StateVector, 0x0, 17*sizeof(uchar));
    
    if (configString != 0x0)
    {
        m_InvertTimer = false;
        m_IniLib.GetConfigParamBool(configString, "INVERTTIMER", &m_InvertTimer, false);
        m_IniLib.GetConfigParamBool(configString, "OLDPROTO", &m_OldProtocol, false);
        
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
            tempVal = (int)(30.0);
        }
            
        //Transform the setpoint in a value between 0 and 255 where 0 means 10C and 255 means 30C
        m_StateVector[9] = (int)(255.0*(setpoint - 10.0)/20.0);
        
        //Setpoint esterno (sola lettura)
        m_StateVector[10] = 0xFF;
        
        //TODO da chiedere a Saps come impostare i prossimi valori
        m_StateVector[11] = 0x0; //Val_DACA
        m_StateVector[12] = 0x0; //Val_DACB

        //Check if it is a PIDLmd controller
        if (isPIDLMD)
        {
            m_StateVector[13] = m_StateVector[13] | 0x04;
        }
        
        //Check if it is summer
        m_IniLib.GetConfigParamInt( configString, "SUMMER", &tempVal, 0);
        if (tempVal)
        {
            m_StateVector[13] = m_StateVector[13] & 0xFD;
        }
        else
        {
            m_StateVector[13] = m_StateVector[13] | 0x02 ;
        }
        
        //Get Setpoint high and setpoint low
        if (isPIDLMD)
        {
            m_IniLib.GetConfigParamFloat(configString, "SETPOINTH", &setpoint, 35.0);
            m_StateVector[14] = (uchar)(setpoint);
            
            m_IniLib.GetConfigParamFloat(configString, "SETPOINTL", &setpoint, 15.0);
            m_StateVector[15] = (uchar)(setpoint);
        }
        else
        {
            m_StateVector[14] = m_StateVector[15] = 0xFF;
        }
        
        //Set FLAG_START
        m_StateVector[16] = 0x0;

        //Get Division factor
        m_IniLib.GetConfigParamInt( configString, "DIVFACTOR", &m_PIDDIvider, 0);

    }
    
    m_Device= 0x0;
    m_UpdateTime = 0;
    m_LastUpdateTime = 0;
    
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
CNTHVLV::~CNTHVLV()
{
}

/////////////////////////////////////////////////////
//                   Update
/////////////////////////////////////////////////////
bool CNTHVLV::Update(bool updateData)
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
        }
    }
        
    //If we have a timer active get the value
    if (m_UseTimer && IsTimerEnabled())
    {   
        turnOn = GetValFromTimer();

        if (m_InvertTimer)
        {
            turnOn = !turnOn;
        }
            
//         retVal = ChangeDOOutput( turnOn );
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
    
    
    return retVal;
}

/////////////////////////////////////////////////////
//                   VerifyIOPresence
/////////////////////////////////////////////////////
bool CNTHVLV::VerifyIOPresence()
{
    return m_Device->VerifyPresence();
}


/////////////////////////////////////////////////////
//                   GetInputDevice
/////////////////////////////////////////////////////
CDS2751* CNTHVLV::GetInputDevice() const
{
  return m_Device;
}

/////////////////////////////////////////////////////
//                   SetInputDevice
/////////////////////////////////////////////////////
void CNTHVLV::SetInputDevice(CDS2751* theValue)
{
  m_Device = theValue;
}



/////////////////////////////////////////////////////
//                   SetSRAM
/////////////////////////////////////////////////////
bool CNTHVLV::SetSRAM(uchar *newSRAM)
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
bool CNTHVLV::GetAllData(uchar *destination)
{
    bool retVal = false;
    uchar mapmem[32];

    memset (mapmem, 0xFF, 32*sizeof(uchar));
    
    if (m_Device != 0x0)
    {
       //Set command
        mapmem[0] = 0x1;
        
        //Update the SRAM
        if (WriteToDevice( mapmem ))
        {

            memcpy (m_SRAMVector, mapmem, 16*sizeof(uchar));
            
            //Everything ok, copy the first 15 bytes coming from the device
            memcpy (destination, m_SRAMVector+1, 14*sizeof(uchar));

            //Append the extra values from the state vector
            destination[14] = m_StateVector[14]; //Setpoint H
            destination[15] = m_StateVector[15]; //Setpoint L
            destination[16] = m_StateVector[16]; //FLAG_START
            retVal = true;        
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
bool CNTHVLV::GetBasicdata(float *tempInt, float *tempExt, float *setpoint, bool *isSummer)
{
    bool retVal = false;
    time_t actTimeSec;

    time(&actTimeSec);

    //Check if enough time has passed OR the temperature is NOT valid
//     if ((actTimeSec > m_LastUpdateTime + m_UpdateTime) ||
//          (m_StateVector[0] == 0xFF))
//     {
//         //Aggiorno i dati
//         if (GetAllData(m_StateVector))
//         {
//             m_LastUpdateTime = actTimeSec;
//             GenerateUpdateTime();
//             retVal = true;
//         }
//     }
//     else
//     {
//         retVal = true;
//     }

    if (GetAllData(m_StateVector))
    {
        m_LastUpdateTime = actTimeSec;
        GenerateUpdateTime();
        retVal = true;
    }
    
    if (retVal)
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
        
    }
    
    return retVal;
}


/////////////////////////////////////////////////////
//                   GetSRAM
/////////////////////////////////////////////////////
bool CNTHVLV::GetSRAM (uchar *destination)
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
bool CNTHVLV::SetSetPoint( float newSetPoint )
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
bool CNTHVLV::CheckCommandExecution( uchar commandChar )
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
    
//     if (!retVal)
//     {
//         cout << "Contenuto della memoria ds2751: "<<endl;
//         for (int i = 0; i < 16; i++){
//             printf ("%X ",m_SRAMVector[i]);
//         }
//         printf ("\n");
//     }
    
    return retVal;
}

/////////////////////////////////////////////////////
//                   WriteStateVector
/////////////////////////////////////////////////////
bool CNTHVLV::WriteStateVector(uchar *newStateVector )
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
    
    //CRC -- not implemented
    mapmem[14] = 0xFF;
    
    return SetAllData( 0x02, mapmem );
}

/////////////////////////////////////////////////////
//                   WriteStateVector
/////////////////////////////////////////////////////
bool CNTHVLV::ChangeDOOutput(bool turnOn )
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
    //Originale
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
    
    return SetAllData( 0x02,mapmem );
}

/////////////////////////////////////////////////////
//                   InitDevice
/////////////////////////////////////////////////////
bool CNTHVLV::InitDevice()
{
    bool retVal = false;
    uchar mapmem[32];
    uchar potentiometersState;
    
    //First extract the settings from the device:
    if (GetAllData( mapmem ))
    {
        //Register the state of the potentiometers
        potentiometersState = mapmem[13] & 0x10;
        
        //Update the state vector
        m_StateVector[13] = m_StateVector[13] | potentiometersState;

        //Se è la scheda di fiege non ho i parametri avanzati
        if (m_OldProtocol)
        {
            m_StateVector[16] = 0x0;
            
            if (WriteStateVector(m_StateVector)) {
                retVal = true;
            }
        }
        else {
            //Write advanced parameters
            memset (mapmem, 0xFF, 32*sizeof(uchar));
            mapmem[0] = m_PIDDIvider;
            
            //Write the configuration in the device
            if ( (SendCommand( 0x04, mapmem )) && (WriteStateVector(m_StateVector)) )
            {
                //Init ok, move the Flag_START value to an high value
                //13-11-2008 -- rimosso perche' guardando il codice di SAPS sembra che se mando FLAG_START != 0x0
                //la scheda smetta di funzionare
    //             m_StateVector[16] = 0xFF;
                m_StateVector[16] = 0x0;
                
                retVal = true;
            }
        }
    }
    
    //Update the init flag
    m_ModuleInitOK = retVal;

    if (retVal)
    {
        //Adjust for timers
        Update( true );
    }
    
            
    return retVal;
}

/////////////////////////////////////////////////////
//                   SetAllData
/////////////////////////////////////////////////////
bool CNTHVLV::SetAllData(uchar command, uchar *newData)
{
    bool retVal = false;
    uchar mapmem[32];

    memset (mapmem, 0x0, 32*sizeof(uchar));
    
    if (m_Device != 0x0)
    {
       //Set command
        mapmem[0] = command;
         
        //Copy all status vector
        memcpy (mapmem+1, newData, 15*sizeof(uchar));
        
        //Update the SRAM
        if (WriteToDevice( mapmem ))
        {
            //Everything ok, save new state
            memcpy (m_StateVector, newData, 15*sizeof(uchar));
            retVal = true;
        }
        else
        {
            PushError( AFOERROR_CTRLVLV_UNABLE_TO_WRITE_SRAM, m_NetNumber, m_DeviceNumber );
        }
        
    }
      
    return retVal;
}

bool CNTHVLV::SendCommand(uchar command, uchar *newData)
{
    bool retVal = false;
    uchar mapmem[32];

    memset (mapmem, 0x0, 32*sizeof(uchar));
    
    if (m_Device != 0x0)
    {
       //Set command
        mapmem[0] = command;
         
        //Copy all status vector
        memcpy (mapmem+1, newData, 15*sizeof(uchar));
        
        //Update the SRAM
        if (!WriteToDevice( mapmem ))
        {

            PushError( AFOERROR_CTRLVLV_UNABLE_TO_WRITE_SRAM, m_NetNumber, m_DeviceNumber );
        }
        else
        {
            retVal = true;
        }
    }
      
    return retVal;
}


/*!
    \fn CNTHVLV::SetSummer(bool isSummer)
 */
bool CNTHVLV::SetSummer(bool isSummer)
{   
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

///////////////////////////////////////////////////
//              GenerateUpdateTime
///////////////////////////////////////////////////
void CNTHVLV::GenerateUpdateTime()
{
    long int randomNumber;
    double randomNumberDbl;
    time_t seed;

    //Genero il tempo di aggiornamento cone distribuzione uniforme tra 20 e 300 sec.
    time(&seed);
    srandom(seed);
    randomNumber = random();
    randomNumberDbl = ((double)randomNumber)/((double)RAND_MAX);
    m_UpdateTime = (unsigned int)(20.0 + randomNumberDbl*(300.0-20.0));
}

///////////////////////////////////////////////////
//              GenerateUpdateTime
///////////////////////////////////////////////////
bool CNTHVLV::WriteToDevice(uchar *mapMem){

    uchar data = 0x0, commandCheck;
    bool retVal;
    int retries = 1;
    bool commOk = false, sameMessage = false, deviceNotPresent = false;
    //Metto 10 secondi di timeout
    unsigned int timeout = 10;
    unsigned int actTime, startTime;

    commandCheck = (~mapMem[0]) - 1;

    //Sopprimo gli errori perche' il modulo e' asincrono
    m_AfoErrorHandler->SetErrorSuppression(true);

    commOk = false;

    //TBR
//     cout << "Scrivo messaggio con intestazione: " << (int)mapMem[0]<<endl;

    while ((retries < 5) && (!commOk))
    {
        if (m_Device->WriteAllSRAM(mapMem))
        {
            if (m_Device->SetSpecialReg(0x80))
            {
                //Aspetto un po' per essere sicuro di beccare la risposta su messaggi "pesanti"
                //msDelay(1000);
                //Entro in un loop di 10 secondi, cerco di fare il prima possibile
                msDelay(100);

                actTime = time(0x0);
                startTime = actTime;

                while (actTime < startTime + timeout)
                {
                    actTime = time(0x0);
                    if (m_Device->GetSpecialReg(&data))
                    {
                        if (data == 0xFF)
                        {
                            //Rileggo tutta la memoria se è tutta 0xFF il dispositivo potrebbe essere ancora nelle mani della uPID2 o non funzionare
                            if (m_Device->GetSRAM(mapMem))
                            {
                                deviceNotPresent = true;
                                for (int i = 0; i < 16; i++)
                                {
                                    if (mapMem[i] != 0xFF)
                                    {
                                        deviceNotPresent = false;
                                        commOk = true;
                                        break;
                                    }
                                }

                                if ((commandCheck == mapMem[0]) && (!deviceNotPresent))
                                {
                                    commOk = true;
                                    break;
                                }
                                else
                                {
//                                     cout<<"Errore di CRC sul comando della uPID2 di indirizzo "<<m_Address<<endl;
                                    msDelay(100);
                                }
                            }
                            else
                            {
                                //TODO messaggio errore
                                //TBR
                                cout << "Impossibile leggere la RAM dalla uPID1"<<endl;
                                msDelay(100);
                            }
                        }
                        else
                        {
                            msDelay(100);
                        }
                    }
                    else
                    {
                        //TODO messaggio di errore
                        //TBR
                        cout << "Impossibile leggere lo stato del PIO dalla uPID1"<<endl;
                        msDelay(100);
                    }
                }//WHILE timeout
            }
            else
            {
                //TODO messaggio di errore
                cout << "Impossibile impostare lo stato del PIO della uPID1"<<endl;
                msDelay(100);
            }
        }
        else
        {
            //TODO messaggio di errore
            cout << "Impossibile scrivere sulla SRAM della uPID1"<<endl;
            msDelay(100);
        }

        retries++;
    }

        //Riabilito gli errori
    m_AfoErrorHandler->SetErrorSuppression(false);


    if (commandCheck == mapMem[0]){
        //Everything ok
        retVal = true;
        cout << "Comunicazione OK"<<endl;
    }
    else {
        if (sameMessage)
        {
            PushError( AFOERROR_CTRLUPID2_DEVICE_NOT_RESPONDING, m_NetNumber, m_DeviceNumber );
        }

        if (deviceNotPresent)
        {
            PushError( AFOERROR_CTRLUPID2_DEVICE_NOT_PRESENT, m_NetNumber, m_DeviceNumber );
        }
            //La risposta non è corretta
            //TBR
//         cout << "Contenuto della memoria ds2751: "<<endl;
//         for (int i = 0; i < 16; i++){
//             printf ("%X ",mapMem[i]);
//         }
//         printf ("\n");
        retVal = false;
    }

    return retVal;
}
