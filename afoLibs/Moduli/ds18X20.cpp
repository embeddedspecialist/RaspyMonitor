/***************************************************************************
 *   Copyright (C) 2005 by Alessandro Mirri                    *
 *   amirri@deis.unibo.it                                      *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author      *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;             *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 ***************************************************************************/
#include "ds18X20.h"
#include "conewirenet.h"

///////////////////////////////////////////////////
//              STANDARD CONSTRUCTOR
///////////////////////////////////////////////////
CDS18X20::CDS18X20(int portNum, COWNET *master, char type, const char* configString)
 : CVDevice(configString, master)
{
  //Do some initialization
  SetFamilyAndDevice();

  m_PortNum = portNum;

  //Read from ini file the device unique parameters
  if (configString != NULL)
  {
    m_IniLib.GetConfigParamBool(configString, "HASPOWER", &m_HasExtPower, 1);
    m_IniLib.GetConfigParamFloat( configString, "COMP", &m_Compensation, 0.0);
  }

  m_DriverData.type = DATA_FLOAT;
  m_LastMeasure = TEMP_ERRVAL;
  memset(m_ScratchPad, 0, 9);
  m_UseTimer = false;

}

///////////////////////////////////////////////////
//              STANDARD DESTRUCTOR
///////////////////////////////////////////////////
CDS18X20::~CDS18X20()
{
}

///////////////////////////////////////////////////
//              UpdateDeviceType
///////////////////////////////////////////////////
void CDS18X20::SetFamilyAndDevice(){
  if (m_SerialNum[0] == 0x10)
  {
    m_FamilyNumber = 0x10;
    m_Name = "DS18S20";
    m_DeviceType = DEV_DS18S20;
  }
  else if (m_SerialNum[0] == 0x28)
  {
      m_FamilyNumber = 0x28;
      m_Name = "DS18B20";
      m_DeviceType = DEV_DS18B20;
  }
}

///////////////////////////////////////////////////
//              READTEMPERATURE
///////////////////////////////////////////////////
bool CDS18X20::ReadTemperature(bool updateFirst, float *newTemp)
{
  bool retVal = false;
  uchar lastcrc8;
  int send_cnt, i, loop=0;
  float tmp;
  int mask1, mask2;
  float mask3, mask4;
  float extTemp;
  bool isAlreadyUpdated = false;

  if (newTemp == NULL)
  {
      return retVal;
  }
  else
  {
      *newTemp = TEMP_ERRVAL;
  }

   // set the device serial number
  m_Master->owSerialNum(m_PortNum,m_SerialNum,false);

  //If an error occurs try to get the temperature a second time.
  for (loop = 0; loop < 2; loop ++)
  {

      if ( updateFirst && (!isAlreadyUpdated))
      {
        //Force new temperature measurement
        if (!UpdateTemp())
        {
            //An error occurred, skip the rest and retry
            continue;
        }
        else
        {
            isAlreadyUpdated = true;
        }
      }

    // access the device
    if (m_Master->owAccess(m_PortNum))
    {
        // create a block to send that reads the temperature
        // read scratchpad command
      send_cnt = 0;
      m_ScratchPad[send_cnt++] = 0xBE;
          // now add the read bytes for data bytes and crc8
      for (i = 0; i < 9; i++)
        m_ScratchPad[send_cnt++] = 0xFF;

          // now send the block
      if (m_Master->owBlock(m_PortNum,false,m_ScratchPad,send_cnt))
      {
          //Reset
          m_Master->owTouchReset( m_PortNum );

              // initialize the CRC8
        m_CRCUtil->setcrc8(m_PortNum,0);
              // perform the CRC8 on the last 8 bytes of packet
        for (i = send_cnt - 9; i < send_cnt; i++)
          lastcrc8 = m_CRCUtil->docrc8(m_PortNum,m_ScratchPad[i]);

              // verify CRC8 is correct
        if (lastcrc8 == 0x00)
        {

          if (m_Name == "DS18B20")
          {

                //Codice di Saputo
                // usando opportune mascherature estrae la temperatura misurata
                if ((m_ScratchPad[2]&0xF8)==0xF8)
                {
                  mask1=((((m_ScratchPad[2]<<8)|m_ScratchPad[1])>>4)|0xFFFFF000)+0x1;
                  mask4=( (float)((~(m_ScratchPad[1]&0x0F)+1)&0xF) )*0.0625;
                  tmp=((float)(mask1))-mask4;
                }
                else
                {
                  mask1=((m_ScratchPad[1]&0xF0)>>4);
                  mask2=((m_ScratchPad[2]&0x0F)<<4);
                  mask3=mask1|mask2;
                  mask4= ((float)(m_ScratchPad[1]&0x0F))*0.0625;
                  tmp=((float)(mask3))+mask4;
                }

                m_LastMeasure = tmp + m_Compensation;
                
          }
          else if (m_Name == "DS18S20")
          {
              //Truncate the 0.5 bit
              m_ScratchPad[1] = m_ScratchPad[1] &0xFE;

            //Check if it is a negative number
            if ((m_ScratchPad[2]&0xFF)==0xFF)
            {
              //Convert number from 2's complement: subtract 1 to the number and complement the result

                uchar tVal = ~(m_ScratchPad[1] - 0x01);
                tmp = (float)(0.0 - tVal);
            }
            else
            {
                tmp = (float)m_ScratchPad[1];
            }

            //The scale is 0.5C so convert the number
            tmp = tmp*0.5;

            //Calculate extended temp range
            extTemp = tmp - 0.25 + (16.0 - m_ScratchPad[7])/16.0 + m_Compensation;

            m_LastMeasure = extTemp;

          }

          *newTemp = m_LastMeasure;
          m_DriverData.floatData[0] = m_LastMeasure;
          m_DriverData.isValid = true;
          
          if (m_DebugLevel)
          {
              cout << "DS18X20 -- NET"<<m_NetNumber<<" Device"<<m_DeviceNumber<<" Temperatura:"<<m_DriverData.floatData[0]<<endl;
          }

          // success, exit from the loop
          retVal = true;

          break;
        }
        else
        {
            SendPresenceError();
            retVal = false;
        }
      }
      else
      {
          //Could not write on port
          PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
          retVal = false;
      }
    }
    else
    {
        //Could not access the master
        PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
        retVal = false;
    }
      

  } //FOR

  if (!retVal)
  {
        PushError( AFOERROR_DS18X20_UNABLE_TO_READ_TEMP, m_NetNumber, m_DeviceNumber);
        m_DriverData.isValid = false;
        if (!m_Master->DS2480Detect( m_PortNum )){
          COneWireNet *net = reinterpret_cast<COneWireNet*>(m_NetPtr);
          net->GetNetHandler(m_NetNumber-1)->recheckMaster = true;
      }
  }
    //Update the CError object


  // return the result flag retVal
  return retVal;
}



///////////////////////////////////////////////////
//              UPDATETEMPERATURE
///////////////////////////////////////////////////
bool CDS18X20::UpdateTemp( )
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
                msDelay(DS18X20_TEMP_CONV_TIME);
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
    }
    else
    {
        PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
        retVal = false;
    }

    if (!retVal)
    {
        PushError( AFOERROR_DS18X20_UNABLE_TO_UPDATE_TEMP, m_NetNumber, m_DeviceNumber);
        if (!m_Master->DS2480Detect( m_PortNum )){
          COneWireNet *net = reinterpret_cast<COneWireNet*>(m_NetPtr);
          net->GetNetHandler(m_NetNumber-1)->recheckMaster = true;
        }
    }
    //Update the CError object


    return retVal;
}

///////////////////////////////////////////////////
//              SETALARMLEVEL
///////////////////////////////////////////////////
bool CDS18X20::SetAlarmLevel( int maxAlarmLevel, int minAlarmLevel )
{
  unsigned char send_blk[4];
  int writeByte = 0;
  bool retVal;

    memset (send_blk, 0, 4);

    // set the device serial number
    m_Master->owSerialNum(m_PortNum,m_SerialNum,false);

    //Commented out to improve performance 3/5/2006
    //msDelay(100);

    // access the device
    if (m_Master->owAccess(m_PortNum))
    {
        //TBR
        //try to wait a bit
        //msDelay(300);

        //Write command
        send_blk[0] = 0x4E;

        //First byte
        send_blk[1] = maxAlarmLevel;

        //Second byte
        send_blk[2] = minAlarmLevel;

        //FIXME:aggiungere la gestione del registro di configurazione
        send_blk[3] = 0x7F; //Write the configuration for the highest resolution

        //The number of bytes written depends on the type of the device
        if (m_Name ==  "DS18B20")
        {
            writeByte = 4;
        }
        else
        {
            writeByte = 3;
        }

        if (!m_Master->owBlock(m_PortNum, false, send_blk, writeByte))
        {
            PushError( OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
            retVal = false;
        }
        else
        {

            if (m_Master->owAccess(m_PortNum))
            {

                //Write the value to the eeprom
                memset (send_blk, 0, 4);

                send_blk[0] = 0x48;

                if (!m_Master->owBlock(m_PortNum, false, send_blk, 1))
                {
                    //TBR
                    PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
                    retVal = false;
                }
                else
                {
                    //Everything OK
                    msDelay(10);
                    retVal = true;
                }
            }
            else
            {
                PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
                retVal = false;
            }
        }
    }
    else
    {
        PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
        retVal = false;
    }


    if (!retVal)
    {
        PushError( AFOERROR_DS18X20_UNABLE_TO_SET_ALARMS, m_NetNumber, m_DeviceNumber);
        if (!m_Master->DS2480Detect( m_PortNum )){
          COneWireNet *net = reinterpret_cast<COneWireNet*>(m_NetPtr);
          net->GetNetHandler(m_NetNumber-1)->recheckMaster = true;
      }
    }
     //Update the CError object


  return retVal;
}
///////////////////////////////////////////////////////////////////////////////////
float CDS18X20::ReadTemperature( bool updateFirst )
{
    float temp = TEMP_ERRVAL;

    ReadTemperature( updateFirst, &temp);

    return temp;

}

