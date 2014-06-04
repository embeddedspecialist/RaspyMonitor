/***************************************************************************
 *   Copyright (C) 2005 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it                                           *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 *************************************************************************/
#include "cownet.h"

static const char* slewVals[] =
{
    "15.0",
    "2.2",
    "1.65",
    "1.37",
    "1.1",
    "0.83",
    "0.7",
    "0.55"
};

static uchar parmsetDefines[] =
{
    0x00,
    0x02,
    0x04,
    0x06,
    0x08,
    0x0A,
    0x0C,
    0x0E
};

static const char* write1Vals[] =
{
    "8",
    "9",
    "10",
    "11",
    "12",
    "13",
    "14",
    "15"
};

static const char* sampleOffsetVals[] =
{
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10"
};

////////////////////////////////////////
//         Standard Constructor       //
////////////////////////////////////////
COWNET::COWNET()
{
  // new global for DS1994/DS2404/DS1427.  If TRUE, puts a delay in owTouchReset to compensate for alarming clocks.
  FAMILY_CODE_04_ALARM_TOUCHRESET_COMPLIANCE = false; // default owTouchReset to quickest response.
  m_Wireless = false;
  
  m_NetDelay = 0;

  
  m_NetNumber = 0;
  
  m_AfoErrorHandler = 0x0;

  dodebug = 0;

  m_SlewRate = PARMSET_Slew2p2Vus;
  m_Write1LowTime = PARMSET_Write11us;
  m_SampleOffset = PARMSET_SampOff10us;
  
  InitMaster();
}

COWNET::COWNET(CIniFileHandler* configFile, const char * section)
{
    CString param;
    
    if (configFile != 0x0)
    {
        param = configFile->GetString("SlewRate", section, "2.2");

        m_SlewRate = DecodeStringParameter(param, slewVals);

        param = configFile->GetString("Write1Low", section, "11");

        m_Write1LowTime = DecodeStringParameter(param, write1Vals);

        if (m_Write1LowTime == 0xFF)
        {
            m_Write1LowTime = PARMSET_Write11us;
        }

        param = configFile->GetString("DataSampleOff", section, "10");

        m_SampleOffset = DecodeStringParameter(param, sampleOffsetVals);

        if (m_SampleOffset == 0xFF)
        {
            m_SampleOffset = PARMSET_SampOff10us;
        }

    }
    else
    {
        m_SlewRate = PARMSET_Slew2p2Vus;
        m_Write1LowTime = PARMSET_Write11us;
        m_SampleOffset = PARMSET_SampOff10us;
    }

    // new global for DS1994/DS2404/DS1427.  If TRUE, puts a delay in owTouchReset to compensate for alarming clocks.
    FAMILY_CODE_04_ALARM_TOUCHRESET_COMPLIANCE = false;
    m_Wireless = false;
  
    m_NetDelay = 0;

  
    m_NetNumber = 0;
  
    m_AfoErrorHandler = 0x0;

    dodebug = 0;

    InitMaster();
        
}

////////////////////////////////////////
//         Standard Destructor       //
////////////////////////////////////////
COWNET::~COWNET()
{
}

SMALLINT COWNET::DS2480ChangeBaud(int portnum, uchar newbaud)
{
  uchar rt=false;
  uchar readbuffer[5],sendpacket[5],sendpacket2[5];
  uchar sendlen=0,sendlen2=0;

   // see if diffenent then current baud rate
  if (UBaud[portnum] == newbaud)
    return UBaud[portnum];
  else
  {
      // build the command packet
      // check if correct mode
    if (UMode[portnum] != MODSEL_COMMAND)
    {
      UMode[portnum] = MODSEL_COMMAND;
      sendpacket[sendlen++] = MODE_COMMAND;
    }
      // build the command
    sendpacket[sendlen++] = CMD_CONFIG | PARMSEL_BAUDRATE | newbaud;

      // flush the buffers
    FlushCOM(portnum);

      // send the packet
    if (!WriteCOM(portnum,sendlen,sendpacket))
    {
      PushError( OWERROR_WRITECOM_FAILED, m_NetNumber );
      rt = false;
    }
    else
    {
         // make sure buffer is flushed
      if (!m_Wireless)
      {
          //Standard value in owsdk
          msDelay(5); 
      }

         // change our baud rate
      SetBaudCOM(portnum,newbaud);
      UBaud[portnum] = newbaud;

         // wait for things to settle
      if (!m_Wireless)
      {
          //Standard value in owsdk
        msDelay(5);
      }

         // build a command packet to read back baud rate
      sendpacket2[sendlen2++] = CMD_CONFIG | PARMSEL_PARMREAD | (PARMSEL_BAUDRATE >> 3);

         // flush the buffers
      FlushCOM(portnum);

         // send the packet
      if (WriteCOM(portnum,sendlen2,sendpacket2))
      {
            // read back the 1 byte response
        if (ReadCOM(portnum,1,readbuffer) == 1)
        {
               // verify correct baud
          if (((readbuffer[0] & 0x0E) == (sendpacket[sendlen-1] & 0x0E)))
            rt = true;
          else
          {
            PushError( OWERROR_DS2480_WRONG_BAUD, m_NetNumber );
          }
        }
        else
        {
          PushError( OWERROR_READCOM_FAILED, m_NetNumber );
          
        }
      }
      else
      {
        PushError( OWERROR_WRITECOM_FAILED, m_NetNumber );
      }
    }
  }

   // if lost communication with DS2480 then reset
  if (rt != true)
    DS2480Detect(portnum);

  return UBaud[portnum];
  
}

////////////////////////////////////////
//         DS2480Detect               //
////////////////////////////////////////
SMALLINT COWNET::DS2480Detect(int portnum)
{
  uchar sendpacket[10],readbuffer[10];
  uchar sendlen=0;
  
  //TBR
  int nOfCharRead = 0;
  
  memset (sendpacket, 0x0, 10*sizeof(uchar));
  memset (readbuffer, 0x0, 10*sizeof(uchar));

   // reset modes
  UMode[portnum] = MODSEL_COMMAND;
  UBaud[portnum] = PARMSET_9600;
  
  
   if (!m_Wireless)
   {
       USpeed[portnum] = SPEEDSEL_FLEX;
   }
   else
   {
       USpeed[portnum] = SPEEDSEL_STD;
   }
              

   // set the baud rate to 9600
    SetBaudCOM(portnum,(uchar)PARMSET_9600);


   // send a break to reset the DS2480
  BreakCOM(portnum);

   // delay to let line settle
  if (!m_Wireless)
  {
      //Standard value in owsdk
    msDelay(2);
  }

   // flush the buffers
  FlushCOM(portnum);

   // send the timing byte
  sendpacket[0] = 0xC1;
  if (WriteCOM(portnum,1,sendpacket) != 1)
  {
    PushError( OWERROR_WRITECOM_FAILED, m_NetNumber );
    return false;
  }

   // delay to let line settle
  if (!m_Wireless)
  {
      //Standard value in owsdk
    msDelay(4);
  }

   // set the FLEX configuration parameters
   // default PDSRC = 1.37Vus -- PARMSET_Slew1p37Vus, PARMSET_Write11us
  
   //sendpacket[sendlen++] = CMD_CONFIG | PARMSEL_SLEW | PARMSET_Slew2p2Vus;
  sendpacket[sendlen++] = CMD_CONFIG | PARMSEL_SLEW | m_SlewRate;
    // default W1LT = 10us
   //sendpacket[sendlen++] = CMD_CONFIG | PARMSEL_WRITE1LOW | PARMSET_Write11us;
   sendpacket[sendlen++] = CMD_CONFIG | PARMSEL_WRITE1LOW | m_Write1LowTime;
    // default DSO/WORT = 8us
   sendpacket[sendlen++] = CMD_CONFIG | PARMSEL_SAMPLEOFFSET | m_SampleOffset;

   // construct the command to read the baud rate (to test command block)
  sendpacket[sendlen++] = CMD_CONFIG | PARMSEL_PARMREAD | (PARMSEL_BAUDRATE >> 3);

   // also do 1 bit operation (to test 1-Wire block)
  sendpacket[sendlen++] = CMD_COMM | FUNCTSEL_BIT | UBaud[portnum] | BITPOL_ONE;

   // flush the buffers
  FlushCOM(portnum);

  
   // send the packet
  if (WriteCOM(portnum,sendlen,sendpacket))
  {
    nOfCharRead = ReadCOM(portnum,5,readbuffer);
    
    
    // read back the response
    if (nOfCharRead == 5)
    {
         // look at the baud rate and bit operation
         // to see if the response makes sense
      if (((readbuffer[3] & 0xF1) == 0x00) &&
            ((readbuffer[3] & 0x0E) == UBaud[portnum]) &&
            ((readbuffer[4] & 0xF0) == 0x90) &&
            ((readbuffer[4] & 0x0C) == UBaud[portnum]))
        return true;
      else
      {
          PushError( OWERROR_DS2480_BAD_RESPONSE, m_NetNumber );
      }
    }
    else
    {
        PushError( OWERROR_READCOM_FAILED, m_NetNumber );
    }
  }
  else
  {
    PushError( OWERROR_WRITECOM_FAILED, m_NetNumber );
  }

//  //14/7/2008
//  //Aggiunto distacco della NET
//  owRelease(portnum);
//  if (!OpenCOM(portnum,(char*)(m_PortNames[portnum].c_str()) ))
//  {
//      //Esco: la porta non e' più disponibile... magari resettando...
//      //serve per casi come SICI in cui insipegabilmente il sistema non vede più il convertitore USB/seriale
//      //e va in palla
//      exit(1);
//  }

  
  return false;
}

////////////////////////////////////////
//         owAcquire                  //
////////////////////////////////////////
SMALLINT COWNET::owAcquire(int portnum, char *port_zstr)
{
    int fd = -1;

    fd = OpenCOM(portnum,port_zstr);
    // attempt to open the comunications port
    if ( fd < 0)
    {
      //Se non riesco neanche ad aprire la porta esco...
      //Serve per situazioni tipo SICI in cui il convertitore USB/Seriale si stacca da solo
      //ogni tanto inspiegabilmente
      exit(1);
      PushError( OWERROR_OPENCOM_FAILED, m_NetNumber );
      return -1;
    }

    
    // detect DS2480
    if (!DS2480Detect(portnum))
    {
        owRelease(portnum);

        //Se non riesco ad acquisire il master cosa continuo a fare ?
        //28/12/2009
        exit(1);

        PushError( OWERROR_DS2480_NOT_DETECTED, m_NetNumber );
        return -1;
    }
    
    return fd;
}

////////////////////////////////////////
//         owAcquireEx(1)             //
////////////////////////////////////////
SMALLINT COWNET::owAcquireEx(char *port_zstr)
{
  int portnum = -1;

   // attempt to open the communications port
  if ((portnum = OpenCOMEx(port_zstr)) < 0)
  {
      //Se non riesco neanche ad aprire la porta esco...
      //Serve per situazioni tipo SICI in cui il convertitore USB/Seriale si stacca da solo
      //ogni tanto inspiegabilmente
      cout << "AAAAAAAAAAARRRRRRRGGGGGGGGGGHHHHHHHHHHHHHH!!!!!!!!!!!!!!!!!!\nImpossibile aprire la porta "<<port_zstr<<" Esco"<<endl;
      exit(1);

//      PushError( OWERROR_OPENCOM_FAILED, m_NetNumber );
//      return -1;
  }

  //TBR
//   cout << "COWNET: Port " << port_zstr << " Opened" << endl; cout.flush();

   // detect DS2480
  if (!DS2480Detect(portnum))
  {
    owRelease(portnum);

    //28/12/2009 - Se non riesco ad acquisire il master cosa continuo a fare ?
    exit(1);

    PushError( OWERROR_DS2480_NOT_DETECTED, m_NetNumber );
    return -1;
  }

  return portnum;
  
}

////////////////////////////////////////
//         owRelease                   //
////////////////////////////////////////
int COWNET::owRelease(int portnum)
{
    if (fd[portnum])
    {
        return CloseCOM(portnum);
    }
    else
    {
        //La porta e' gia' chiusa
        return 1;
    }
}

////////////////////////////////////////
//         FindDevices                //
////////////////////////////////////////
SMALLINT COWNET::FindDevices(int portnum, uchar FamilySN[][8], SMALLINT family_code, int MAXDEVICES)
{
  int NumDevices=0;
  SMALLINT retVal;

   // find the devices
   // set the search to first find that family code
  owFamilySearchSetup(portnum,family_code);

  do
  {
      retVal = owNext(portnum,true, false);
      
      // perform the search
      if ( (!retVal) || (retVal <0) )
      break;

      owSerialNum(portnum,FamilySN[NumDevices], true);

    if ((FamilySN[NumDevices][0] & 0x7F) == (family_code & 0x7F))
    {
      NumDevices++;
    }
  }
  while (NumDevices < (MAXDEVICES));

  return NumDevices;
}

////////////////////////////////////////
//         InitMaster                //
////////////////////////////////////////
void COWNET::InitMaster( )
{
    memset (fd, 0x0, MAX_PORTNUM*sizeof(int));
    fd_init = 0;
   
    // global DS2480B state
    memset (ULevel, 0x0, MAX_PORTNUM*sizeof(SMALLINT));
    memset (UBaud, 0x0, MAX_PORTNUM*sizeof(SMALLINT));
    memset (UMode, 0x0, MAX_PORTNUM*sizeof(SMALLINT));
    memset (USpeed, 0x0, MAX_PORTNUM*sizeof(SMALLINT));
    memset (UVersion, 0x0, MAX_PORTNUM*sizeof(SMALLINT));
    
    memset(LastDiscrepancy, 0x0, MAX_PORTNUM*sizeof(int));;
    memset (LastFamilyDiscrepancy, 0x0, MAX_PORTNUM*sizeof(int));
    memset (LastDevice,0x0, MAX_PORTNUM*sizeof(uchar));
    memset (SerialNum, 0x0, MAX_PORTNUM*8*sizeof(uchar));
    
    memset (ProgramAvailable,0x0, MAX_PORTNUM*sizeof(SMALLINT));

    //!Flag indicating wether the net is wireless
    m_Wireless = false;

    //!Delay to be used when the NET is wireless
    m_NetDelay = 0;
    
}

int COWNET::GetNofPortsAcquired( )
{
    int retVal = 0;
    int i = 0;
    
    for (i = 0; i < MAX_PORTNUM; i++)
    {
        if (fd[i] != 0)
        {
            retVal++;
        }
    }
    
    return retVal;
    
}

uchar COWNET::DecodeStringParameter(CString param, const char** paramArray)
{
    int i = 0;
    uchar retVal = 0xFF;

    for (i = 0; i < 8; i++)
    {
        if (!strcasecmp(paramArray[i],param.c_str()))
        {
            retVal = parmsetDefines[i];
        }
    }

    return retVal;
            
}

// uchar COWNET::DecodeIntParameter(int param, int * paramArray)
// {
//     int i = 0;
//     uchar retVal = 0xFF;
// 
//     for (i = 0; i < 8; i++)
//     {
//         if (paramArray[i] == param)
//         {
//             retVal = parmsetDefines[i];
//         }
//     }
// 
//     return retVal;
// }






