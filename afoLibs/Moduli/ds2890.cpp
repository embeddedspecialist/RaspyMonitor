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
#include "ds2890.h"
#include "conewirenet.h"

///////////////////////////////////////////////////
//              STANDARD CONSTTRUCTOR            //
///////////////////////////////////////////////////
CDS2890::CDS2890(int portNum, COWNET *master, const char* configString)
    : CVDevice(configString, master)
{
    m_ControlRegister = 0x0;
    m_CurrentPosition = 0x0;
    m_FeatureRegister = 0x00;
    m_PortNum = portNum;
    m_FamilyNumber = 0x2C;
    m_DeviceType = DEV_DS2890;
    
    if (configString != 0x0)
    {
        m_IniLib.GetConfigParamBool(configString,"UPDATESTATE",&m_UpdateState, false);
    }

}

///////////////////////////////////////////////////
//              STANDARD DESTRUCTOR
///////////////////////////////////////////////////
CDS2890::~CDS2890()
{
}

///////////////////////////////////////////////////
//              WriteControlRegister
///////////////////////////////////////////////////
bool CDS2890::WriteControlRegister( uchar newVal )
{
    bool retVal = false;
    uchar outBuff[3] = {0xff, 0xff, 0xff};
    
    m_Master->owSerialNum(m_PortNum, m_SerialNum, false);
    
    //Send a reset pulse
    if (!m_Master->owTouchReset(m_PortNum))
    {
        PushError( OWERROR_RESET_FAILED, m_NetNumber, m_DeviceNumber);
        return retVal;
    }
    
    if (m_Master->owAccess(m_PortNum))
    {
        //Write Register command
        outBuff[0] = 0x55;
        //New Value
        outBuff[1] = newVal;
        outBuff[2] = 0xff;
        
        //Write out the command and read back the response
        if (m_Master->owBlock(m_PortNum, false, outBuff, 3))
        {
        
            //Check the response
            if (outBuff[2] == newVal)
            {
                //Transaction successfull, send the Release code
                outBuff[0] = 0x96;
                outBuff[1] = 0xff;
                outBuff[2] = 0xff;
                
                if (m_Master->owBlock(m_PortNum, false, outBuff, 3))
                {
            
                    //If release successfull we have 0's in the buffer
                    if (!outBuff[2])
                    {
                        retVal = true;
                    }
                    else
                    {
                        PushError( AFOERROR_DS2890_UNABLE_TO_SEND_RELCODE, m_NetNumber, m_DeviceNumber);
                    }
                }
                else
                {
                    PushError( OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
                } 
            }
            else
            {
                SendPresenceError();
            }
        }
        else
        {
            PushError( OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
        }
        
    }
    else
    {
        //An error occurred in RESET
        PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
    }
    
    //terminate the sequence with a reset pulse
    if (!m_Master->owTouchReset(m_PortNum))
    {
        PushError( OWERROR_RESET_FAILED, m_NetNumber, m_DeviceNumber);
        retVal = false;
    }
    
    if (!retVal)
    {
        PushError( AFOERROR_DS2890_UNABLE_TO_WRITE_CTRLREG, m_NetNumber, m_DeviceNumber);
    }
    
    //Update the situation of the sensor
     
    
    return retVal;
}

///////////////////////////////////////////////////
//              ReadControlRegister
///////////////////////////////////////////////////
uchar CDS2890::ReadControlRegister( )
{
    bool deviceOK = false;
    uchar outBuf[4] = {0xff, 0xff, 0xff, 0xff};
    
    //Reset the control register
    m_ControlRegister = 0x00;
    
    //Send a reset
       //Send a reset pulse
    if (!m_Master->owTouchReset(m_PortNum))
    {
        PushError( OWERROR_RESET_FAILED, m_NetNumber, m_DeviceNumber);
        return m_ControlRegister;
    }
    
    m_Master->owSerialNum(m_PortNum, m_SerialNum, false);
    
    if (m_Master->owAccess(m_PortNum))
    {
        //prepare the buffer
        outBuf[0] = 0xaa; //Command
        outBuf[1] = 0xff; //Feature register
        outBuf[2] = 0xff; //Control register
        outBuf[3] = 0xff; //0's if command successfull
        
        //Send the command
        if (m_Master->owBlock(m_PortNum, false, outBuf, 4))
        {
            //If the command was really successfull we should read 0
            if (!outBuf[3])
            {
                m_FeatureRegister = outBuf[1];
                m_ControlRegister = outBuf[2];
                deviceOK = true;
            }
            else
            {
                SendPresenceError();
            }
        }
        else
        {
            PushError( OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
        } 
    }
    else
    {
        //An error occurred in RESET
        PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
    }
    
    //terminate the sequence with a reset pulse
    if (!m_Master->owTouchReset(m_PortNum))
    {
        PushError( OWERROR_RESET_FAILED, m_NetNumber, m_DeviceNumber);
        m_ControlRegister = 0x0;
        deviceOK = false;
    }
    
    if (!deviceOK)
    {
        PushError( AFOERROR_DS2890_UNABLE_TO_READ_CTRLREG, m_NetNumber, m_DeviceNumber);
        if (!m_Master->DS2480Detect( m_PortNum )){
          COneWireNet *net = reinterpret_cast<COneWireNet*>(m_NetPtr);
          net->GetNetHandler(m_NetNumber-1)->recheckMaster = true;
        }
    }
    
    //Update the situation of the sensor
     
    
    return m_ControlRegister;
    
}

///////////////////////////////////////////////////
//              ReadControlRegister(2)
///////////////////////////////////////////////////
bool CDS2890::ReadControlRegister( uchar * contReg )
{
    uchar newRegister = 0x0;
    
    newRegister = ReadControlRegister();
    
    if (newRegister != 0x0)
    {
        *contReg = m_ControlRegister;
        return true;
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////
//              ReadPosition
///////////////////////////////////////////////////
int CDS2890::ReadPosition( )
{
    bool deviceOK = false;
    uchar outBuf[4] = {0xff, 0xff, 0xff, 0xff};
    int position = -1;
    
    //Send a reset
    if (!m_Master->owTouchReset(m_PortNum))
    {
        PushError( OWERROR_RESET_FAILED, m_NetNumber, m_DeviceNumber);
        return position;
    }
    
    //Set the serial number of the device
    m_Master->owSerialNum(m_PortNum, m_SerialNum, false);
    
    if (m_Master->owAccess(m_PortNum))
    {
        //prepare the buffer with the read position command
        outBuf[0] = 0xf0; //Read command
        outBuf[1] = 0xff; //Control Register
        outBuf[2] = 0xff; //Position
        outBuf[3] = 0xff; //0's if successfull
        
        //Send the command
        if (m_Master->owBlock(m_PortNum, false, outBuf, 4))
        {
            //If the command was really successfull we should read 0
            if (!outBuf[3])
            {
                position = (int)outBuf[2];
                deviceOK = true;
            }
            else
            {
                SendPresenceError();
            }
        }
        else
        {
            PushError( OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
        }  
    }
    else
    {
        //An error occurred in RESET
        PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
    }
    
    //terminate the sequence with a reset pulse
    if (!m_Master->owTouchReset(m_PortNum))
    {
        PushError( OWERROR_RESET_FAILED, m_NetNumber, m_DeviceNumber);
        position = -1;
        deviceOK = false;
    }
    
    if (!deviceOK)
    {
        PushError( AFOERROR_DS2890_UNABLE_TO_READPOS, m_NetNumber, m_DeviceNumber);
        if (!m_Master->DS2480Detect( m_PortNum )){
          COneWireNet *net = reinterpret_cast<COneWireNet*>(m_NetPtr);
          net->GetNetHandler(m_NetNumber-1)->recheckMaster = true;
      }
    }
    
    //Update the situation of the sensor
     
    
    return position;
}

///////////////////////////////////////////////////
//              ReadPosition(2)
///////////////////////////////////////////////////
bool CDS2890::ReadPosition( int * pos )
{
    bool retVal = false;
    
    *pos = ReadPosition();
    
    if (*pos != -1)
    {
        retVal = true;
    }
    
    return retVal;
}

///////////////////////////////////////////////////
//              SetPosition
///////////////////////////////////////////////////
bool CDS2890::SetPosition( uchar newPos )
{
    bool retVal = false;
    uchar outBuff[3] = {0xff, 0xff, 0xff};
    
    m_Master->owSerialNum(m_PortNum, m_SerialNum, false);
    
    VerifyPresence();
    
    if (m_Master->owAccess(m_PortNum))
    {
        //SetPosition command
        outBuff[0] = 0x0f;
        //New Value
        outBuff[1] = newPos;
        //New position control
        outBuff[2] = 0xff;
        
        //Write out the command and read back the response
        m_Master->owBlock(m_PortNum, false, outBuff, 3);
        
        //Wait a bit
        msDelay (10);
        
        //Check the response
        if (outBuff[2] == newPos)
        {
            //Transaction successfull, send the Release code
            outBuff[0] = 0x96;
            outBuff[1] = 0xff;
            outBuff[2] = 0xff;
            
            if (m_Master->owBlock(m_PortNum, false, outBuff, 3))
            {
            
                //If release successfull we have 0's in the buffer
                if (!outBuff[2])
                {
                    m_CurrentPosition = newPos;
                    m_DriverData.regData[0] = newPos;
                    m_DriverData.isValid = true;
                    retVal = true;
                }
                else
                {
                    SendPresenceError();
                }
            }
            else
            {
                PushError( OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
            }
        }
        else
        {
            SendPresenceError();
        }
        
    }
    else
    {
        //An error occurred in RESET
        PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
    }
    
    //msDelay( 50 );
    
    //terminate the sequence with a reset pulse
    if (!m_Master->owTouchReset(m_PortNum))
    {
        PushError( OWERROR_RESET_FAILED, m_NetNumber, m_DeviceNumber);
        retVal = false;
    }
    
    if (!retVal)
    {
        PushError( AFOERROR_DS2890_UNABLE_TO_SETPOS, m_NetNumber, m_DeviceNumber);
        if (!m_Master->DS2480Detect( m_PortNum )){
          COneWireNet *net = reinterpret_cast<COneWireNet*>(m_NetPtr);
          net->GetNetHandler(m_NetNumber-1)->recheckMaster = true;
        }
    }
    
    //Update the situation of the sensor
     
    
    return retVal;
}

///////////////////////////////////////////////////
//              Increment1Step
///////////////////////////////////////////////////
bool CDS2890::Increment1Step( )
{
    bool retVal = false;
    uchar outBuff[2] = {0xff, 0xff};
    
    m_Master->owSerialNum(m_PortNum, m_SerialNum, false);
    
    //Send a reset pulse
    m_Master->owTouchReset(m_PortNum);
    
    if (m_Master->owAccess(m_PortNum))
    {
        //Check if we are at the top
        if (m_CurrentPosition != 0xff)
        {
            //Increment command
            outBuff[0] = 0xc3;
            outBuff[1] = 0xff;
            
            //Write out the command and read back the response
            if (m_Master->owBlock(m_PortNum, false, outBuff, 2))
            {
                m_CurrentPosition = outBuff[1];
                retVal = true;
            }
            else
            {
                PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
            }
        }
        else
        {
            retVal = true;
        }
    
    }
    else
    {
        //An error occurred in RESET
        PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
    }
    
    //terminate the sequence with a reset pulse
    if (!m_Master->owTouchReset(m_PortNum))
    {
        PushError( OWERROR_RESET_FAILED, m_NetNumber, m_DeviceNumber);
        retVal = false;
    }
    
    if (!retVal)
    {
        PushError( AFOERROR_DS2890_UNABLE_TO_INCREMENT_POS, m_NetNumber, m_DeviceNumber);
    }
    
    //Update the situation of the sensor
     
    
    return retVal;
}

///////////////////////////////////////////////////
//              IncrementbySteps
///////////////////////////////////////////////////
bool CDS2890::IncrementBySteps( int nSteps )
{
    bool retVal = false;
    uchar outBuff[2] = { 0xff, 0xff };
    int actStep = 0;
    
    m_Master->owSerialNum(m_PortNum, m_SerialNum, false);
    
    //Send a reset pulse
    m_Master->owTouchReset(m_PortNum);
    
    if (m_Master->owAccess(m_PortNum))
    {
        for (actStep = 0; actStep < nSteps; actStep++)
        {
            //Increment command
            outBuff[0] = 0xc3;
            outBuff[1] = 0xff;
            
            //Write out the command and read back the response
            if (m_Master->owBlock(m_PortNum, false, outBuff, 2))
            {
                m_CurrentPosition = outBuff[1];
            
                //Check if we are at top
                if (outBuff[1] == 0xff)
                {
                    break;
                }
            }
            else
            {
                PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
                break;
            }

        }
        
        if (actStep == nSteps)
        {
            retVal = true;
        }
    
    }
    else
    {
        //An error occurred in RESET
        PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
    }
    
    //terminate the sequence with a reset pulse
    if (!m_Master->owTouchReset(m_PortNum))
    {
      PushError(OWERROR_RESET_FAILED, m_NetNumber, m_DeviceNumber);
      retVal = false;
    }
    
    if (!retVal)
    {
        PushError( AFOERROR_DS2890_UNABLE_TO_INCREMENT_POS, m_NetNumber, m_DeviceNumber);
    }
    
    //Update the situation of the sensor
     
    
    return retVal;
}


///////////////////////////////////////////////////
//              Decrement1Step
///////////////////////////////////////////////////
bool CDS2890::Decrement1Step( )
{
    bool retVal = false;
    uchar outBuff[2] = { 0xff, 0xff };
    
    m_Master->owSerialNum(m_PortNum, m_SerialNum, false);
    
    //Send a reset pulse
    if (!m_Master->owTouchReset(m_PortNum))
    {
        PushError(OWERROR_RESET_FAILED, m_NetNumber, m_DeviceNumber);
    }

    
    if (m_Master->owAccess(m_PortNum))
    {
        //Check if we are at the bottom
        if (m_CurrentPosition != 0x00)
        {
            //Decrement command
            outBuff[0] = 0x99;
            outBuff[1] = 0xff;
            
            //Write out the command and read back the response
            if (m_Master->owBlock(m_PortNum, false, outBuff, 2))
            {
                m_CurrentPosition = outBuff[1];
                retVal = true;
            }
            else
            {
                PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
            }
        }
        else
        {
            retVal = true;
        }
    
    }
    else
    {
        //An error occurred in RESET
        PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
    }
    
    //terminate the sequence with a reset pulse
    if (!m_Master->owTouchReset(m_PortNum))
    {
        PushError(OWERROR_RESET_FAILED, m_NetNumber, m_DeviceNumber);
        retVal = false;
    }

    if (!retVal)
    {
        PushError( AFOERROR_DS2890_UNABLE_TO_DECREMENT_POS, m_NetNumber, m_DeviceNumber);
    }
    //Update the situation of the sensor
     
    
    return retVal;   
}

///////////////////////////////////////////////////
//              DecrementBySteps
///////////////////////////////////////////////////
bool CDS2890::DecrementBySteps( int nSteps )
{
    bool retVal = false;
    uchar outBuff[2] = { 0xff, 0xff };
    int actStep = 0;
    
    m_Master->owSerialNum(m_PortNum, m_SerialNum, false);
    
    //Send a reset pulse
    if (!m_Master->owTouchReset(m_PortNum))
    {
        PushError(OWERROR_RESET_FAILED, m_NetNumber, m_DeviceNumber);
    }
    
    if (m_Master->owAccess(m_PortNum))
    {
        for (actStep = 0; actStep < nSteps; actStep++)
        {
            //Decrement command
            outBuff[0] = 0x99;
            outBuff[1] = 0xff;
            
            //Write out the command and read back the response
            if (m_Master->owBlock(m_PortNum, false, outBuff, 2))
            {
                m_CurrentPosition = outBuff[1];
            
                //Check if we are at bottom
                if (outBuff[1] == 0x00)
                {
                    break;
                }
            }
            else
            {
                PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
                break;
            }

        }
        
        if (actStep == nSteps)
        {
            //EveryStep has been performed
            retVal = true;
        }
    
    }
    else
    {
        //An error occurred in RESET
        PushError(OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
    }
    
    //terminate the sequence with a reset pulse
    if (!m_Master->owTouchReset(m_PortNum))
    {
        PushError(OWERROR_RESET_FAILED, m_NetNumber, m_DeviceNumber);
    }
    
    if (!retVal)
    {
        PushError( AFOERROR_DS2890_UNABLE_TO_DECREMENT_POS, m_NetNumber, m_DeviceNumber);
    }
    
    //Update the situation of the sensor
     
    
    return retVal;
}
//////////////////////////////////////////////////////////////////////////////////////////////
bool CDS2890::UpdateState()
{
    if (m_UpdateState)
    {
        m_DriverData.floatData[0] = ReadPosition();
        if ((int)(m_DriverData.floatData[0]) != -1)
        {
            m_DriverData.isValid = true;
        }
        else
        {
            m_DriverData.isValid = false;
        }
        
        if (m_DebugLevel)
        {
            cout <<"DS2890 -- Device"<<m_DeviceNumber<<" Posizione Wiper: "<< (int)(m_DriverData.floatData[0])<<endl;
        }
    }
    
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////////
