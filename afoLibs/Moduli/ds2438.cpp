/***************************************************************************
 *   Copyright (C) 2005 by Alessandro Mirri                                *
 *   amirri@deis.unibo.it                                                  *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 ***************************************************************************/
#include "ds2438.h"
#include "conewirenet.h"

///////////////////////////////////////////////////
//              STANDARD CONSTRUCTOR
///////////////////////////////////////////////////
CDS2438::CDS2438(int portNum, COWNET *master, const char* configString)
 : CVDevice(configString, master)
{
  //Do some initialization
  m_Name = "DS2438";
  m_FamilyNumber = 0x26;
  m_PortNum = portNum;
  m_DeviceType = DEV_DS2438;
  
  m_IniLib.GetConfigParamBool( configString, "HASPOWER", &m_HasExtPower, true);
  m_IniLib.GetConfigParamFloat (configString, "COMP", &m_Compensation, 0.0);
  
  m_IniLib.GetConfigParamBool( configString, "CURRENT", &m_ReadCurrent, true);
  m_IniLib.GetConfigParamBool( configString, "TEMPERATURE", &m_ReadTemp, true);
  m_IniLib.GetConfigParamBool( configString, "VOLTAGE", &m_ReadVad, true);
  
  memset(m_RAMScratchPad, 0, sizeof(m_RAMScratchPad));
  
  memset(m_EEScracthPad, 0, sizeof(m_EEScracthPad));

  
  m_IsMeasureCurrentEnabled = false;
  m_IsMeasureVddEnabled = true;
  
}

///////////////////////////////////////////////////
//              STANDARD DESTRUCTOR
///////////////////////////////////////////////////
CDS2438::~CDS2438()
{
}

///////////////////////////////////////////////////
//              READTEMPERATURE
///////////////////////////////////////////////////
float CDS2438::ReadTemperature( bool updateFirst)
{
    float ret=-1.0;
    uchar send_block[50];
    int send_cnt=0;
    int i = 0;
    ushort lastcrc8;
    bool deviceOK = false;
    
    if (updateFirst)
    {
        UpdateTemp();
    }
    
    //Init the search engine
    m_Master->owSerialNum(m_PortNum,m_SerialNum,false);


    //Access the device
    if(m_Master->owAccess(m_PortNum))
    {
        // Recall the Status/Configuration page
        // Recall command
        send_block[send_cnt++] = 0xB8;
    
        // Page to Recall
        send_block[send_cnt++] = 0x00;
    
        if(m_Master->owBlock(m_PortNum,false,send_block,send_cnt))
        {
            send_cnt = 0;
    
            if(m_Master->owAccess(m_PortNum))
            {
                // Read the Status/Configuration byte
                // Read scratchpad command
                send_block[send_cnt++] = 0xBE;
            
                // Page for the Status/Configuration byte
                send_block[send_cnt++] = 0x00;
    
                //Put al bits to 1
                for(i=0;i<9;i++)
                {
                    send_block[send_cnt++] = 0xFF;
                }
    
                //Write the command
                if(m_Master->owBlock(m_PortNum,false,send_block,send_cnt))
                {
                    //Device has been read correctly
                    deviceOK = true;
                    
                    m_CRCUtil->setcrc8(m_PortNum,0);
    
                    //Check CRC
                    for(i=2;i<send_cnt;i++)
                    {
                        lastcrc8 = m_CRCUtil->docrc8(m_PortNum,send_block[i]);
                    }
                    
                    //CRC OK?
                    if(lastcrc8 == 0x00)
                    {
                        //Calculate temperature
                        ret = (((send_block[4] << 8) | send_block[3]) >> 3) * 0.03125;
                        m_DriverData.floatData[DS2438_TEMPERATURE_INDEX] =  ret;
                    }
                    else
                    {
                        //The device has been correctly read but something, in the communication, went wrong, just return the "false" value
                        SendPresenceError();
                        ret = TEMP_ERRVAL;
                    }
                }
                else
                {
                    //An error occurred in reading the device
                    PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
                    deviceOK = false;
                    ret = TEMP_ERRVAL;
                }  
            }
            else
            {
                //An error occurred
                PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
                deviceOK = false;
                ret = TEMP_ERRVAL;
            }
        }
        else
        {
            //An error occurred
            deviceOK = false;
            PushError(OWERROR_WRITE_BYTE_FAILED, m_NetNumber, m_DeviceNumber);
            ret = TEMP_ERRVAL;
        }
    }
    else
    {
        //AN error occurred
        PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
        deviceOK = false;
        ret = TEMP_ERRVAL;
    }

    //Send Reset
    m_Master->owTouchReset( m_PortNum );
    
    if (!deviceOK)
    {
        PushError( AFOERROR_DS2438_UNABLE_TO_READ_TEMP, m_NetNumber, m_DeviceNumber);
        if (!m_Master->DS2480Detect( m_PortNum )){
          COneWireNet *net = reinterpret_cast<COneWireNet*>(m_NetPtr);
          net->GetNetHandler(m_NetNumber-1)->recheckMaster = true;
        }
    }
        
  //Update the error condition
    if (ret == TEMP_ERRVAL)
    {
        m_DriverData.isValid = false;
    }
   
  
  return ret;
}

///////////////////////////////////////////////////
//              ReadTemperature (2)
///////////////////////////////////////////////////
bool CDS2438::ReadTemperature( bool updateFirst, float * newTemp )
{
    float temp;
    
    if (newTemp == NULL)
    {
        return false;
    }
    
    temp = ReadTemperature( updateFirst );
    *newTemp = temp;
    
    if (temp != TEMP_ERRVAL)
    {
        
        return true;
    }
    else
    {
        return false;
    } 
}

///////////////////////////////////////////////////
//              ReadVoltage
///////////////////////////////////////////////////
float CDS2438::ReadVoltage( bool readVdd )
{
  uchar send_block[50];
  int send_cnt=0;
  int i;
  int busybyte = 0, timeout = 100000;; 
  char lastcrc8;
  int volts = 0;
  float ret = ANALOG_ERRVAL;
  int done = true;
  bool deviceOK = false;
  int count = 2;
  bool isCRCError = false;

  memset (send_block, 0x0, 50*sizeof(uchar));
  
  m_Master->owSerialNum(m_PortNum, m_SerialNum, false);
    
  do
  {
    if(SetVoltageMeasurement(readVdd))
    {
      //Access the device
      if(m_Master->owAccess(m_PortNum))
      {
        //Write conversion command
        if(m_Master->owWriteByte(m_PortNum,0xB4))
        {
            //Conversion needs approx 10ms
            msDelay(15);

            //FIXME Aggiungere un watchdog o qualcosa
            while(busybyte == 0)
            {
                busybyte = m_Master->owReadByte(m_PortNum);
                timeout--;
                msDelay(1);
            }
            
            //Conversion needs approx 10ms
//             msDelay( 10 );
            
            if(m_Master->owAccess(m_PortNum))
            {
                // Recall the Status/Configuration page
                // Recall command
                send_block[send_cnt++] = 0xB8;
        
                    // Page to Recall
                send_block[send_cnt++] = 0x00;

                //Send command
                if(m_Master->owBlock(m_PortNum,false,send_block,send_cnt))
                {
                    send_cnt = 0;
                    
                    if(m_Master->owAccess(m_PortNum))
                    {
                            // Read the Status/Configuration byte
                            // Read scratchpad command
                        send_block[send_cnt++] = 0xBE;
                
                            // Page for the Status/Configuration byte
                        send_block[send_cnt++] = 0x00;
                
                        for(i=0;i<9;i++)
                        {
                            send_block[send_cnt++] = 0xFF;
                        }
                
                        if(m_Master->owBlock(m_PortNum,false,send_block,send_cnt))
                        {
                            
                            //Device correctly accessed
                            deviceOK = true;
                            
                            m_Master->owTouchReset( m_PortNum );

                            //Check the CRC
                            m_CRCUtil->setcrc8(m_PortNum,0);
                    
                            for(i=2;i<send_cnt;i++)
//                             for(i=2;i<11;i++)
                            {
                                lastcrc8 = m_CRCUtil->docrc8(m_PortNum,send_block[i]);
                            }
                            
                    
                            if(lastcrc8 == 0x00)
                            {
                                if((!readVdd) && ((send_block[2] & 0x08) == 0x08))
                                {
                                    done = false;
                                    count--;
                                }
                                else
                                {
                                    isCRCError = false;
                                    done = true;
                                }
                                
                                volts = (send_block[6] << 8) | send_block[5];
                                ret = (float) volts/100.0;
                                
                                if (done)
                                {
                                    m_DriverData.floatData[DS2438_VOLTAGE_INDEX] = ret*2.0;
                                }
                            }
                            else
                            {
                                //The device has been correctly read but something, in the communication, went wrong, retry the reading sequence
                                VerifyPresence();
                                if (count > 0)
                                {
                                    done = false;
                                    count--;
                                }
                                else
                                {
                                    SendPresenceError();
                                    done = true;
                                }
                            }
                        }
                        else
                        {
                            //error
                            PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
                            deviceOK = false;
                            done = true;
                        }    

                    }
                    else
                    {
                        //Error
                        PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
                        deviceOK = false;
                        done = true;
                    }
                }
                else
                {
                    //Error
                    PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
                    deviceOK = false;
                    done = true;
                }
            }
            else
            {
                //Error
                PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
                deviceOK = false;
                done = true;
            }
        }
        else
        {
            //Error
            PushError(OWERROR_WRITE_BYTE_FAILED, m_NetNumber, m_DeviceNumber);
            deviceOK = false;
            done = true;
        }
      
      }//Access
      else
      {
          //Error
          PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
          deviceOK = false;
          done = true;
      }
    } //SetAtoD
    else
    {
        deviceOK = false;
        count--;
    }
  } while ((!done) && (count > 0));

//     if (isCRCError)
//     {
//         SendPresenceError();
//     }
    
  if (!deviceOK)
  {
      m_DriverData.isValid = false;
      if (readVdd)
      {
          PushError( AFOERROR_DS2438_UNABLE_TO_READ_VDD, m_NetNumber, m_DeviceNumber);
      }
      else
      {
          PushError( AFOERROR_DS2438_UNABLE_TO_READ_VOLTAGE, m_NetNumber, m_DeviceNumber);
      }

      if (!m_Master->DS2480Detect( m_PortNum )){
          COneWireNet *net = reinterpret_cast<COneWireNet*>(m_NetPtr);
          net->GetNetHandler(m_NetNumber-1)->recheckMaster = true;
      }
  }
  
  //Update error condition

  //17/72008 -- Aggiunto perchè se e' fuori scala l'ingresso leggo 10V
  if (ret > 5.5)
  {
      ret = ANALOG_ERRVAL;
  }

  return ret;
}

///////////////////////////////////////////////////
//              SetVoltageMeasurement
///////////////////////////////////////////////////
bool CDS2438::SetVoltageMeasurement( bool measureVdd )
{
  uchar send_block[50];
  uchar test;
  int send_cnt=0;
  int i;
  ushort lastcrc8;
  int busybyte;
  bool deviceOK = false;
  bool retVal = false;

  //Check if we have already set up this
  if (m_IsMeasureVddEnabled == measureVdd)
  {
      return true;
  }
  
  //Init search engine
  m_Master->owSerialNum(m_PortNum,m_SerialNum,false);

  if (!m_Master->owAccess( m_PortNum ))
  {
      PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
      return false;
  }
 
   // Recall the Status/Configuration page
   // Recall command
  send_block[send_cnt++] = 0xB8;

   // Page to Recall
  send_block[send_cnt++] = 0x00;

  if(m_Master->owBlock(m_PortNum,false,send_block,send_cnt))
  {
    send_cnt = 0;
    
    // Read the Status/Configuration byte
    // Read scratchpad command
    send_block[send_cnt++] = 0xBE;

    // Page for the Status/Configuration byte
    send_block[send_cnt++] = 0x00;

    for(i=0;i<9;i++)
    {
        send_block[send_cnt++] = 0xFF;
    }

    if(m_Master->owBlock(m_PortNum,false,send_block,send_cnt))
    {

        m_CRCUtil->setcrc8(m_PortNum,0);

        for(i=2;i<send_cnt;i++)
        {
            lastcrc8 = m_CRCUtil->docrc8(m_PortNum,send_block[i]);
        }


//         if(lastcrc8 == 0x00)
        if(true)
        {
            test = send_block[2] & 0x08;
            if(((test == 0x08) && measureVdd) || ((test == 0x00) && (!measureVdd)))
            {
                //The desired conversion is already set
                return true;
            }
        }
        else
        {
            //The device has been correctly read but something, in the communication, went wrong, just return the "false" value
            SendPresenceError();
            //TBM
            return false;
        }
    }//Block
    else
    {
        PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
//         return false;
    }

    
    //Setup the desired conversion
    if (m_Master->owAccess(m_PortNum))
    {
        send_cnt = 0;
        // Write the Status/Configuration byte
        // Write scratchpad command
        send_block[send_cnt++] = 0x4E;
    
        // Write page
        send_block[send_cnt++] = 0x00;
    
        if(measureVdd)
        {
            send_block[send_cnt++] = send_block[2] | 0x08;
        }
        else
        {
            send_block[send_cnt++] = send_block[2] & 0xF7;
        }
    
        for(i=0;i<7;i++)
        {
            send_block[send_cnt++] = send_block[i+4];
        }
    
        if(m_Master->owBlock(m_PortNum,false,send_block,send_cnt))
        {
            send_cnt = 0;
        
            if(m_Master->owAccess(m_PortNum))
            {
                    // Copy the Status/Configuration byte
                    // Copy scratchpad command
                send_block[send_cnt++] = 0x48;
        
                    // Copy page
                send_block[send_cnt++] = 0x00;
        
                if(m_Master->owBlock(m_PortNum,false,send_block,send_cnt))
                {
                    busybyte = m_Master->owReadByte(m_PortNum);

                    //FIXME: Aggiungere un watchdog o qualcosa
                    while(busybyte == 0)
                    {
                        busybyte = m_Master->owReadByte(m_PortNum);
                    }
        
                    retVal = true;
                }//Block
                else
                {
                    //Error
                    PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
                    deviceOK = false;
                    retVal = false;
                }
            }//Access
            else
            {
                //Error
                PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
                deviceOK = false;
                retVal = false;
            }
        }//Block
        else
        {
            //Error
            PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
            deviceOK = false;
            retVal = false;
        }
    
    }//Access
    else
    {
        //Error
        PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
        retVal = false;
    }
  }
  else
  {
      //Error
      PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
      retVal = false;
  }
  
  if (!retVal)
    {
        PushError( AFOERROR_DS2438_UNABLE_TO_SET_VOLTAGE_MEASUREMENT, m_NetNumber, m_DeviceNumber);
        if (!m_Master->DS2480Detect( m_PortNum )){
          COneWireNet *net = reinterpret_cast<COneWireNet*>(m_NetPtr);
          net->GetNetHandler(m_NetNumber-1)->recheckMaster = true;
        }
    }
    else
    {
        m_IsMeasureVddEnabled = measureVdd;
    }
 
  return retVal;
}

///////////////////////////////////////////////////
//              READTIMER
///////////////////////////////////////////////////
int CDS2438::ReadTimer( )
{
  uchar send_block[50];
  int send_cnt=0;
  int retVal = -1;
  ushort lastcrc8;
  int i;
  bool deviceOK = false;
    
  //Issue the recall memory command to copy the timer value in the scratchpad

  //set the serial number to search for
  m_Master->owSerialNum(m_PortNum,m_SerialNum,false);
  
   // Recall the Status/Configuration page
   // Recall memory command
  send_block[send_cnt++] = 0xB8;

   // Page to Recall
  send_block[send_cnt++] = 0x01;

  if(m_Master->owBlock(m_PortNum,false,send_block,send_cnt))
  {
    send_cnt = 0;
    
    if(m_Master->owAccess(m_PortNum))
    {

        // Read scratchpad page where the ETM is stored
        send_block[send_cnt++] = 0xBE;
        
            // Page for the Status/Configuration byte
        send_block[send_cnt++] = 0x01;
        
        for(i=0;i<9;i++)
        {
            send_block[send_cnt++] = 0xFF;
        }
        
        if(m_Master->owBlock(m_PortNum,false,send_block,send_cnt))
        {
            m_CRCUtil->setcrc8(m_PortNum,0);
        
            for(i=2;i<send_cnt;i++)
            {
                lastcrc8 = m_CRCUtil->docrc8(m_PortNum,send_block[i]);
            }
        
            if(lastcrc8 == 0x00)
            {
                //Device correctly accessed
                deviceOK = true;
                
                //Terminate the buffer containing the number
                send_block[9] = '\0';
        
                //get The timer value
                sscanf((const char*)send_block, "%d", &retVal);
            }
            else
            {
                SendPresenceError();
            }
        }//Block
        else
        {
            //Error
            PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
            deviceOK = false;
        }
    }
    else
    {
        //Error
        PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
        deviceOK = false;
    }
  }
  else
  {
      //Errore
      PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
      deviceOK = false;
  }
  
  if (!retVal)
  {
      PushError( AFOERROR_DS2438_UNABLE_TO_READ_TIMER, m_NetNumber, m_DeviceNumber);
  }

  //Update error condition
   
  
  return retVal;

}

///////////////////////////////////////////////////
//              UpdateTemp
///////////////////////////////////////////////////
bool CDS2438::UpdateTemp( )
{    
    bool retVal = false;
    
    // set the device serial number
    m_Master->owSerialNum(m_PortNum,m_SerialNum,false);
        
    // access the device
    if (m_Master->owAccess(m_PortNum))
    {
        
        //Check if the device is in parasite power
        if (m_HasExtPower)
        {
                //No parasite power, the conversion can start immediately
            if (m_Master->owWriteByte(m_PortNum, 0x44))
            {
                //Wait for the conversion to be done
                msDelay(DS2438_TEMP_CONV_TIME);
                retVal = true;
            }
            else
            {
                //Could not write the command
                retVal = false;
                PushError(OWERROR_WRITE_BYTE_FAILED, m_NetNumber, m_DeviceNumber);
            }
        }
        else
        {
                // Parasite power: send the convert command and start power delivery
            if (!m_Master->owWriteBytePower(m_PortNum,0x44))
            {
                retVal = false;
            }
            else
            {
                //TBR
                cout << "************WriteBytePOWER****************" << endl; cout.flush();
                // sleep for 1 second
                msDelay(1000);
            
                // turn off the 1-Wire Net strong pull-up
                if (m_Master->owLevel(m_PortNum,MODE_NORMAL) != MODE_NORMAL)
                {
                    retVal = true;
                }
            }
        }
        
        m_Master->owTouchReset( m_PortNum );
    }
    else
    {
        PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
        retVal = false;
    }

    if (!retVal)
    {
        PushError( AFOERROR_DS2438_UNABLE_TO_UPDATE_TEMP, m_NetNumber, m_DeviceNumber);
    }
    //Update the CError object
     
    
    return retVal;
}

///////////////////////////////////////////////////
//              ReadCurrent
///////////////////////////////////////////////////
int CDS2438::ReadCurrentRegister( )
{
    uchar send_block[50];
    int send_cnt=0;
    int i;
    int busybyte; 
    ushort lastcrc8;
    int ret= 0;
    int done = true;
    bool deviceOK = false;

    m_Master->owSerialNum(m_PortNum, m_SerialNum, false);

    do
    {
        if(EnableCurrentMeasuring( true ))
        {

            //Access the device
            if(m_Master->owAccess(m_PortNum))
            {
                //Write conversion command
                if(m_Master->owWriteByte(m_PortNum,0xB4))
                {

                    msDelay(15);
                    //Read Back the answer
                    busybyte = m_Master->owReadByte(m_PortNum);

                    //FIXME Aggiungere un watchdog o qualcosa
                    while(busybyte == 0)
                    {
                        busybyte = m_Master->owReadByte(m_PortNum);
                    }

                    if(m_Master->owAccess(m_PortNum))
                    {
                        // Recall the Status/Configuration page
                        // Recall command
                        send_block[send_cnt++] = 0xB8;
        
                        // Page to Recall
                        send_block[send_cnt++] = 0x00;

                        //Send command
                        if(m_Master->owBlock(m_PortNum,false,send_block,send_cnt))
                        {
                            send_cnt = 0;
        
                            if(m_Master->owAccess(m_PortNum))
                            {
                                // Read the Status/Configuration byte
                                // Read scratchpad command
                                send_block[send_cnt++] = 0xBE;
                
                                // Page for the Status/Configuration byte
                                send_block[send_cnt++] = 0x00;
                
                                for(i=0;i<9;i++)
                                {
                                    send_block[send_cnt++] = 0xFF;
                                }
                
                                if(m_Master->owBlock(m_PortNum,false,send_block,send_cnt))
                                {

                                    //Check the CRC
                                    m_CRCUtil->setcrc8(m_PortNum,0);
                    
                                    for(i=2;i<send_cnt;i++)
                                    {
                                        lastcrc8 = m_CRCUtil->docrc8(m_PortNum,send_block[i]);
                                    }
                    
                                    if(lastcrc8 == 0x00)
                                    {
                                         
                                        ret = (send_block[8] << 8) | send_block[7];
                                        deviceOK = true;
                                        m_DriverData.floatData[DS2438_CURRENT_INDEX] = ret;
                                    }
                                    else
                                    {
                                        //The device has been correctly read but something, in the communication, went wrong, just return the "false" value
                                        SendPresenceError();
                                        ret = -1;
                                        done = true;
                                    }
                                }
                                else
                                {
                            //error
                                    PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
                                    deviceOK = false;
                                    ret = -1;
                                    done = true;
                                }    

                            }
                            else
                            {
                        //Error
                                PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
                                deviceOK = false;
                                ret = -1;
                                done = true;
                            }
                        }
                        else
                        {
                    //Error
                            PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
                            deviceOK = false;
                            ret = -1;
                            done = true;
                        }
                    }
                    else
                    {
                //Error
                        PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
                        deviceOK = false;
                        ret = -1;
                        done = true;
                    }
                }
                else
                {
            //Error
                    PushError(OWERROR_WRITE_BYTE_FAILED, m_NetNumber, m_DeviceNumber);
                    deviceOK = false;
                    ret = -1;
                    done = true;
                }
      
            }//Access
            else
            {
          //Error
                PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
                deviceOK = false;
                ret = -1;
                done = true;
            }
        } //Enable Current
        else
        {
            deviceOK = false;
            ret = -1;
            done = true;
        }
    } while(!done);

    if (!deviceOK)
    {
        PushError( AFOERROR_DS2438_UNABLE_TO_READ_CURRENT, m_NetNumber, m_DeviceNumber);
        m_DriverData.isValid = false;
        if (!m_Master->DS2480Detect( m_PortNum )){
          COneWireNet *net = reinterpret_cast<COneWireNet*>(m_NetPtr);
          net->GetNetHandler(m_NetNumber-1)->recheckMaster = true;
        }
    }
    
  //Update error condition
    
     
  
    return ret;
}

///////////////////////////////////////////////////
//              EnableCurrentMeasuring
///////////////////////////////////////////////////
bool CDS2438::EnableCurrentMeasuring( bool enable )
{
    uchar send_block[50];
    uchar test;
    int send_cnt=0;
    int i;
    ushort lastcrc8;
    int busybyte;
    bool deviceOK = false;
    bool retVal = false;

    if (m_IsMeasureCurrentEnabled == enable)
    {
        return true;
    }
    
  //Init search engine
    m_Master->owSerialNum(m_PortNum,m_SerialNum,false);

    if (!m_Master->owAccess( m_PortNum ))
    {
        PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
        return false;
    }
  
   // Recall the Status/Configuration page
   // Recall command
    send_block[send_cnt++] = 0xB8;

   // Page to Recall
    send_block[send_cnt++] = 0x00;

    //Check if current measurements is already enabled
    if(m_Master->owBlock(m_PortNum,false,send_block,send_cnt))
    {
        send_cnt = 0;
    
        // Read the Status/Configuration byte
        // Read scratchpad command
        send_block[send_cnt++] = 0xBE;

        // Page for the Status/Configuration byte
        send_block[send_cnt++] = 0x00;

        for(i=0;i<9;i++)
        {
            send_block[send_cnt++] = 0xFF;
        }

        if(m_Master->owBlock(m_PortNum,false,send_block,send_cnt))
        {

            m_CRCUtil->setcrc8(m_PortNum,0);

            for(i=2;i<send_cnt;i++)
            {
                lastcrc8 = m_CRCUtil->docrc8(m_PortNum,send_block[i]);
            }
            
            //TODO sembra che fallisca spesso questa lettura...
            lastcrc8 = 0x00;

            if(lastcrc8 == 0x00)
            {
                test = send_block[2] & 0x01;
                if(((test == 0x01) && enable) || ((test == 0x00) && !(enable)))
                {
                    retVal = true;
                }
                else
                {
                    retVal = false;
                }

                deviceOK = true;
            }
            else
            {
            //The device has been correctly read but something, in the communication, went wrong, just return the "false" value
                SendPresenceError();
                retVal = false;
            }
        }//Block
        else
        {
            //Error
            PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
        }


        if (!retVal)
        {
            //Write in the register the current configuration
            if ((deviceOK) && (m_Master->owAccess(m_PortNum)))
            {
                send_cnt = 0;
                // Write the Status/Configuration byte
                // Write scratchpad command
                send_block[send_cnt++] = 0x4E;
        
                // Write page
                send_block[send_cnt++] = 0x00;
        
                if(enable)
                {
                    send_block[send_cnt++] = send_block[2] | 0x01;
                }
                else
                {
                    send_block[send_cnt++] = send_block[2] & 0xF7;
                }
        
                for(i=0;i<7;i++)
                {
                    send_block[send_cnt++] = send_block[i+4];
                }
        
                if(m_Master->owBlock(m_PortNum,false,send_block,send_cnt))
                {
                    send_cnt = 0;
            
                    if(m_Master->owAccess(m_PortNum))
                    {
                        // Copy the Status/Configuration byte
                        // Copy scratchpad command
                        send_block[send_cnt++] = 0x48;
            
                        // Copy page
                        send_block[send_cnt++] = 0x00;
            
                        if(m_Master->owBlock(m_PortNum,false,send_block,send_cnt))
                        {
                            busybyte = m_Master->owReadByte(m_PortNum);
    
                            //FIXME: Aggiungere un watchdog o qualcosa
                            while(busybyte == 0)
                            {
                                busybyte = m_Master->owReadByte(m_PortNum);
                            }
            
                            retVal = true;
                        }//Block
                        else
                        {
                        //Error
                            PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
                            deviceOK = false;
                            retVal = false;
                        }
                    }//Access
                    else
                    {
                    //Error
                        PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
                        deviceOK = false;
                        retVal = false;
                    }
                }//Block
                else
                {
                //Error
                    PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
                    deviceOK = false;
                    retVal = false;
                }
        
            }//Access
            else
            {
                //Error
                PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
                deviceOK = false;
                retVal = false;
            }
        }
        else //retval
        {
            deviceOK = true;
        }
    }
    else
    {
        //Error
        PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
        deviceOK = false;
        retVal = false;
    }

    if (!retVal)
    {
        PushError( AFOERROR_DS2438_UNABLE_TO_SET_CURRENT_MEASUREMENT, m_NetNumber, m_DeviceNumber);
    }
    else {
        m_IsMeasureCurrentEnabled = enable;
    }
    

    //Update error condition
     
  
    return retVal;
}

///////////////////////////////////////////////////
//              GetMemoryPage
///////////////////////////////////////////////////
bool CDS2438::GetMemoryPage( uchar memoryPage, uchar * destination )
{
    uchar send_block[50];
    int send_cnt=0;
    int i = 0, loop = 0;
    ushort lastcrc8;
    bool deviceOK = false;
   
    //Init the search engine
    m_Master->owSerialNum(m_PortNum,m_SerialNum,false);

    //Try to read the requested page 2 times
    for (loop = 0; loop < 2; loop++)
    {
        //Access the device
        if(m_Master->owAccess(m_PortNum))
        {
            // Recall the Status/Configuration page
            // Recall command
            send_block[send_cnt++] = 0xB8;
        
            // Page to Recall
            send_block[send_cnt++] = memoryPage;
        
            if(m_Master->owBlock(m_PortNum,false,send_block,send_cnt))
            {
                send_cnt = 0;
        
                if(m_Master->owAccess(m_PortNum))
                {
                    // Read the Status/Configuration byte
                    // Read scratchpad command
                    send_block[send_cnt++] = 0xBE;
                
                    // Page for the Status/Configuration byte
                    send_block[send_cnt++] = memoryPage;
        
                    //Put al bits to 1
                    for(i=0;i<9;i++)
                    {
                        send_block[send_cnt++] = 0xFF;
                    }
        
                    //Write the command
                    if(m_Master->owBlock(m_PortNum,false,send_block,send_cnt))
                    {
                        //Device has been read correctly
                        deviceOK = true;
                        
                        m_CRCUtil->setcrc8(m_PortNum,0);
        
                        //Check CRC
                        for(i=2;i<send_cnt;i++)
                        {
                            lastcrc8 = m_CRCUtil->docrc8(m_PortNum,send_block[i]);
                        }
                        
                        //CRC OK?
                        if(lastcrc8 == 0x00)
                        {
                            int i = 0;
                            //Copy into destination the requested page
                            for (i = 0; i < 8; i++)
                            {
                                destination[i] = send_block[2+i];
                            }
                            break;
                        }
                        else
                        {
                            //The device has been correctly read but something, in the communication, went wrong, just return the "false" value
                            SendPresenceError();
                            memset (destination, 0x0, 8);
                        }
                    }
                    else
                    {
                        //An error occurred in reading the device
                        PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);

                        deviceOK = false;
                    }  
                }
                else
                {
                    //An error occurred
                    PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
                    deviceOK = false;
                }
            }
            else
            {
                //An error occurred
                deviceOK = false;
                PushError(OWERROR_WRITE_BYTE_FAILED, m_NetNumber, m_DeviceNumber);
            }
        }
        else
        {
            //AN error occurred
            PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
            deviceOK = false;
        }
    } //Loop
        
    if (!deviceOK)
    {
        PushError( OWERROR_READ_MEMORY_PAGE_FAILED, m_NetNumber, m_DeviceNumber);
    }
  //Update the error condition
     
  
    return deviceOK;

}

///////////////////////////////////////////////////
//              UpdateMeasures
///////////////////////////////////////////////////
bool CDS2438::UpdateMeasures( )
{
    bool retVal = false;
    uchar send_block[50];
    int send_cnt=0;
    int busybyte=0;
    
    //Write conversion command
    if(m_Master->owWriteByte(m_PortNum,0xB4))
    {
        msDelay(15);
        
        //Read Back the answer
        busybyte = m_Master->owReadByte(m_PortNum);

        //FIXME Aggiungere un watchdog o qualcosa
        while(busybyte == 0)
        {
            busybyte = m_Master->owReadByte(m_PortNum);
        }

        if(m_Master->owAccess(m_PortNum))
        {
            // Recall the Status/Configuration page
            // Recall command
            send_block[send_cnt++] = 0xB8;
        
            // Page to Recall
            send_block[send_cnt++] = 0x00;

            //Send command
            if(m_Master->owBlock(m_PortNum,false,send_block,send_cnt))
            {
                retVal = true;
            }
            else
            {
                PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
            }
        }
        else
        {
            PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
        }
    }
    else
    {
        PushError(OWERROR_WRITE_BYTE_FAILED, m_NetNumber, m_DeviceNumber);
    }
    
    return retVal;
}
//////////////////////////////////////////////////////////////////////////////
bool CDS2438::ReadAllData()
{
    bool retVal = true;
    
    //Leggo temperatura
    if (m_ReadTemp)
    {
        retVal = ReadTemperature(true) && retVal;
    }
    else
    {
        m_DriverData.floatData[DS2438_TEMPERATURE_INDEX] = TEMP_ERRVAL;
    }
    
    //Leggo VAD
    if (m_ReadVad)
    {
        retVal = ReadVoltage(false) && retVal;
    }
    else
    {
        m_DriverData.floatData[DS2438_VOLTAGE_INDEX] = ANALOG_ERRVAL;
    }
    
    //Leggo corrente
    if (m_ReadCurrent)
    {
        retVal = ReadCurrentRegister() && retVal;
    }
    else
    {
        m_DriverData.floatData[DS2438_CURRENT_INDEX] = ANALOG_ERRVAL;
    }
    
    m_DriverData.isValid = retVal;
    
    if (m_DebugLevel)
    {
        cout << "DS2438 -- Device"<<m_DeviceNumber<<" -- Temp: "
                <<m_DriverData.floatData[DS2438_TEMPERATURE_INDEX]
                <<" Vad: "<< m_DriverData.floatData[DS2438_VOLTAGE_INDEX]
                << " Iad: "<< m_DriverData.floatData[DS2438_CURRENT_INDEX]<<endl;
    }
               
    return retVal;
    
}
