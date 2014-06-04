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
#include "ds2405.h"

///////////////////////////////////////////////////
//              STANDARDCONSTRUCTOR
///////////////////////////////////////////////////
CDS2405::CDS2405(int portNum, COWNET *master, const char* configString)
 : CVDevice(configString, master)
{
 
  //Do some initialization
  m_FamilyNumber = 0x05;
  m_Name = "DS2405";
  m_PortNum = portNum;
  
}

///////////////////////////////////////////////////
//              STANDARD DESTRUCTOR
///////////////////////////////////////////////////
CDS2405::~CDS2405()
{
}

///////////////////////////////////////////////////
//              READSTATEFROMDEVICE
///////////////////////////////////////////////////
bool CDS2405::ReadStateFromDevice( )
{
    bool deviceOk = false, retVal = false;
    
  //Store the serial number for the library
  m_Master->owSerialNum(m_PortNum,m_SerialNum, false);

  //First check for the presence of the device
  if(m_Master->owVerify(m_PortNum,false))
  {
      //The device is present
      deviceOk = true;

     //check its state
    if(m_Master->owTouchByte(m_PortNum,0xFF) == 0xFF)
    {
      m_StateOn = true;
    }
    else
    {
      m_StateOn = false;
    }

    if(m_Master->owVerify(m_PortNum,true))
    {
      retVal = true;
    }
    else
    {
      retVal = false;
    }
  }
  else
  {
      //Device NOT detected, signal an alarm
      deviceOk = false;
  }
    

  //Update the CError object
   

  return retVal;
}

///////////////////////////////////////////////////
//              SETSWITCH
///////////////////////////////////////////////////
bool CDS2405::SetSwitch( bool setOn )
{
  int compare;  //Value used to compare the actual state with the one requested
  bool retVal = false;

  //Init the search engine
  m_Master->owSerialNum(m_PortNum,m_SerialNum, false);

  //Check if the device is present
  if (m_Master->owVerify(m_PortNum, false))
  {
    
    //Get the actual state of the switch
    compare = m_Master->owVerify(m_PortNum,true);
    
    if((compare && setOn) || (!compare && !setOn))
    {
        //Nothing to be done, exit
        retVal = true;
    }
    else
    {
        //Change state to the device
        if(m_Master->owAccess(m_PortNum))
        {
            //Update and read back the new state
            compare = m_Master->owVerify(m_PortNum,true);

            //FIXME: controllare che la logica di questo funzionamento sia giusta
            if((compare && setOn) || (!compare && !setOn))
            {
                //Requested setting and actual setting match, update the internal variable
                m_StateOn = setOn;
                retVal = true;
            }
            else
            {
                //a problem occurred: requested settings and actual settings are different
                retVal = false;
            }
        }
        else
        {
            //Device not reacheable
            PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
            retVal = false;
        }
    }
  }
  else
  {
      retVal = false;
  }

  //Update the CError object
   

  return retVal;
}
