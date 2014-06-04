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
#include "ds2408.h"
#include "conewirenet.h"

CDS2408::CDS2408(int portNum, COWNET *master, const char* configString) 
    : CVDevice(configString, master)
{
    char configBuffer[32];
    
    memset (configBuffer, 0, 32);
    
    //Save the port number
    m_PortNum = portNum;
    
    //Save info on device
    m_FamilyNumber = 0x29;
    m_Name = "DS2408";
    m_DeviceType = DEV_DS2408;

    memset(m_Register, 0, 3*sizeof(uchar));
    
    if (configString != 0x0)
    {
        m_IniLib.GetConfigParamBool( configString, "ACTIVITY", &m_ActivateConditionalSearch, false);
        m_IniLib.GetConfigParamInt( configString, "POLARMASK", &m_PolarityMask, 0);
        m_IniLib.GetConfigParamBool( configString, "ANDLOGIC", &m_UseAndLogic, false);
        m_IniLib.GetConfigParamInt( configString, "ACTIMASK", &m_ActivityMask, 0);
        m_IniLib.GetConfigParamInt( configString, "TYPE", (int*)&m_ModuleType, 0);
    }
    
         
}


CDS2408::~CDS2408()
{
}





//--------------------------------------------------------------------------
// SUBROUTINE - SetSwitch05
//
// This routine turns the device on or off.
//
// 'm_PortNum'  - number 0 to MAX_PORTNUM-1.  This number was provided to
//              OpenCOM to indicate the port number.
// 'SwNum'    - The serial number of the swith that is to be turned on or off
// 'channel'  - The channel the switch is suppose to be set on.
// 'seton'    - 'true' then it is turned on 
//              'false' turns it off 
//
// Returns: true(1):    If set is successful
//          false(0):   If set is not successful
//
bool CDS2408::SetSwitch( uchar *state)
{
    uchar buff[5];
    int i;
    bool retVal = false;

    m_Master->owSerialNum(m_PortNum,m_SerialNum, false);

    if(m_Master->owAccess(m_PortNum))
    {
        buff[0] = 0x5A;
        buff[1] = state[1];
        buff[2] = (uchar) ~state[1];
        for(i=0;i<2;i++)
            buff[i+3] = 0xFF;

        if(!m_Master->owBlock(m_PortNum,false,&buff[0],5))
        {
            PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
        }

        if((buff[3] == 0xAA))
        {
            m_Master->owTouchReset( m_PortNum );
            retVal = true;
        }
        else
        {
            SendPresenceError();
        }
    }
    else
    {
        PushError( OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
    }
    
    if (!retVal)
    {
        PushError( AFOERROR_DS2408_UNABLE_TO_SETSWITCH, m_NetNumber, m_DeviceNumber);
    }

    return retVal;
}

//--------------------------------------------------------------------------
// SUBROUTINE - ReadSwitch05
//
// This routine reads the state of the DS2408.
//
// 'm_PortNum'    - number 0 to MAX_PORTNUM-1.  This number was provided to
//                OpenCOM to indicate the port number.
// 'SNum'       - The Serial number of the DS2405 that info is requested on
// 'state'      - This is the state information of the DS2408.
//
// Returns: true(1):    If device is found and active
//          false(0):   If device is not found and not active or not there
//
bool CDS2408::ReadSwitch( uchar *state)
{
    uchar buff[32];
    int send_cnt = 0, i = 0;
    ushort lastcrc16;
    bool retVal;

    memset (buff, 0x0, 32*sizeof(uchar));
    
    m_Master->owSerialNum(m_PortNum,m_SerialNum, false);
    

    if(m_Master->owAccess(m_PortNum))
    {
        buff[send_cnt++] = 0xF0;

        // address 1
        buff[send_cnt++] = (uchar) (0x88 & 0xFF);
        // address 2
        buff[send_cnt++] = (uchar) (((0x88 & 0xFFFF) >> 8) & 0xFF);

        for(i=0;i<((0x8f -0x88)+1);i++)
        {
            buff[send_cnt++] = 0xFF;
        }
        
        buff[send_cnt++] = 0xFF;
        buff[send_cnt++] = 0xFF;
        
        if(m_Master->owBlock(m_PortNum,false,&buff[0],send_cnt))
        {
        
            // initialize the CRC16
            m_CRCUtil->setcrc16(m_PortNum,0);
            
            // perform the CRC16 on all packet
            for (i = 0; i < send_cnt; i++)
            {
                lastcrc16 = m_CRCUtil->docrc16(m_PortNum,buff[i]);
            }
    
            // verify CRC16 is correct
            if (lastcrc16 == 0xb001)
            {
                for(i=0;i<3;i++)
                {
                    state[i] = buff[i+3];
                    m_Register[i] = buff[i+3];
                }
                
                retVal = true;
            }
            else
            {
                SendPresenceError();
            }
        }
        else
        {
            PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
        }
    }
    else
    {
        PushError( OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
    }

    if (!retVal)
    {
        PushError( AFOERROR_DS2408_UNABLE_TO_READSWITCH, m_NetNumber, m_DeviceNumber);
        if (!m_Master->DS2480Detect( m_PortNum )){
          COneWireNet *net = reinterpret_cast<COneWireNet*>(m_NetPtr);
          net->GetNetHandler(m_NetNumber-1)->recheckMaster = true;
        }
    }
    return retVal;
}

// SUBROUTINE - setResetMode
//
// Turns the Reset mode on/off.
//
// 'm_PortNum'   - number 0 to MAX_PORTNUM-1.  This number was provided to
//               OpenCOM to indicate the port number.
// 'SNum'      - The Serial number of the DS2405 that info is requested on
// 'seton'     - 'true' set reset mode, 'false' turn off reset mode
//
// Returns: true(1):    If operation worked
//          false(0):   If an error occured
//
bool CDS2408::SetResetMode( bool seton)
{
    uchar reg[3];

    if(ReadRegister(COND_SEARCH_CHANNEL_SEL,reg))
    {
        if(seton && ((reg[2] & 0x04) == 0x04))
        {
            reg[2] = (uchar) (reg[2] & 0xFB);
        }
        else if((!seton) && ((reg[2] & 0x04) == 0x00))
        {
            reg[2] = (reg[2] | 0x04);
        }

        if(!SetRegister(COND_SEARCH_CHANNEL_SEL,reg))
            return false;
    }
    else
        return false;

    return true;
}

// SUBROUTINE - getVCC
//
// Retrieves the state of the VCC pin.  If the pin is powered 'true' is
// returned else 'false' is returned if the pin is grounded.
//
// 'reg'   - register value that was read.
//
// Returns: true(1):    If VCC is powered
//          false(0):   If VCC is grounded
//
bool CDS2408::GetVCC(uchar *reg)
{
    if((reg[2] & 0x80) == 0x80)
        return true;

    return false;
}

// SUBROUTINE - clearPowerOnReset
//
// Checks if the Power On Reset if on and if so clears it.
//
// 'm_PortNum'   - number 0 to MAX_PORTNUM-1.  This number was provided to
//               OpenCOM to indicate the port number.
// 'SNum'      - The Serial number of the DS2405 that info is requested on
//
// Returns: true(1):    If cleared
//          false(0):   If an error occured
//
bool CDS2408::ClearPowerOnReset()
{
    uchar reg[3];

    if(!ReadRegister(COND_SEARCH_CHANNEL_SEL,reg))
        return false;

    if((reg[2] & 0x08) == 0x08)
    {
        reg[2] = (reg[2] & 0xF7);
    }

    if(!SetRegister(COND_SEARCH_CHANNEL_SEL,reg))
        return false;

    return true;
}

// SUBROUTINE - orConditionalSearch
//
// Checks if the 'or' Condition Search is set and if not sets it.
//
// 'm_PortNum'   - number 0 to MAX_PORTNUM-1.  This number was provided to
//               OpenCOM to indicate the port number.
// 'SNum'      - The Serial number of the DS2405 that info is requested on
//
// Returns: true(1):    If set
//          false(0):   If an error occured
//
bool CDS2408::OrConditionalSearch()
{
    uchar reg[3];

    if(!ReadRegister(COND_SEARCH_CHANNEL_SEL,reg))
        return false;

    if((reg[2] & 0x02) == 0x02)
    {
        reg[2] = (reg[2] & 0xFD);

        if(!SetRegister(COND_SEARCH_CHANNEL_SEL,reg))
            return false;
    }

    return true;
}

// SUBROUTINE - andConditionalSearch
//
// Checks if the 'and' Conditional Search is set and if not sets it.
//
// 'm_PortNum'   - number 0 to MAX_PORTNUM-1.  This number was provided to
//               OpenCOM to indicate the port number.
// 'SNum'      - The Serial number of the DS2405 that info is requested on
//
// Returns: true(1):    If set
//          false(0):   If an error occured
//
bool CDS2408::AndConditionalSearch()
{
    uchar reg[3];

    if(!ReadRegister(COND_SEARCH_CHANNEL_SEL,reg))
        return false;

    if((reg[2] & 0x02) != 0x02)
    {
        reg[2] = (reg[2] | 0x02);

        if(!SetRegister(COND_SEARCH_CHANNEL_SEL,reg))
            return false;
    }

    return true;
}

// SUBROUTINE - pioConditionalSearch
//
// Checks if the 'PIO' Conditional Search is set for input and if not sets it.
//
// 'm_PortNum'   - number 0 to MAX_PORTNUM-1.  This number was provided to
//               OpenCOM to indicate the port number.
// 'SNum'      - The Serial number of the DS2405 that info is requested on
//
// Returns: true(1):    If set
//          false(0):   If an error occured
//
bool CDS2408::SetPioConditionalSearch()
{
    uchar reg[3];

    if(!ReadRegister(COND_SEARCH_CHANNEL_SEL,reg))
        return false;

    if((reg[2] & 0x01) == 0x01)
    {
        reg[2] = (reg[2] & 0xFE);

        if(!SetRegister(COND_SEARCH_CHANNEL_SEL,reg))
            return false;
    }

    return true;
}

// SUBROUTINE - activityConditionalSearch
//
// Checks if the activity latches are set for Conditional Search and if not sets it.
//
// 'm_PortNum'   - number 0 to MAX_PORTNUM-1.  This number was provided to
//               OpenCOM to indicate the port number.
// 'SNum'      - The Serial number of the DS2405 that info is requested on
//
// Returns: true(1):    If set
//          false(0):   If an error occured
//
bool CDS2408::SetActivityConditionalSearch()
{
    uchar reg[3];
    
    memset (reg, 0x0, 3*sizeof(uchar));

    if(!ReadRegister(COND_SEARCH_CHANNEL_SEL, reg))
        return false;
       
    if((reg[2] & 0x01) != 0x01)
    {
        reg[2] = (reg[2] | 0x01);

        if(!SetRegister(COND_SEARCH_CHANNEL_SEL, reg))
            return false;
    }

    return true;
}

// SUBROUTINE - setChannelMask
//
// Sets the channel passed to the proper state depending on the set parameter for
//    responding to the Conditional Search.
//
// 'm_PortNum'   - number 0 to MAX_PORTNUM-1.  This number was provided to
//               OpenCOM to indicate the port number.
// 'SNum'      - The Serial number of the DS2405 that info is requested on
// 'channel'   - channel to set
// 'seton'     - 'true' turn channel on, 'false' turn channel off
//
// Returns: true(1):    If set
//          false(0):   If an error occured
//
bool CDS2408::SetChannelMask( int channel, bool seton)
{
    uchar reg[3];
    uchar mask;

    mask = (uchar) (0x01 << channel);

    if(!ReadRegister(COND_SEARCH_CHANNEL_SEL, reg))
        return false;

    if(seton)
        reg[0] = (uchar) (reg[0] | mask);
    else
        reg[0] = (uchar) (reg[0] & ~mask);

    if(!SetRegister(COND_SEARCH_CHANNEL_SEL, reg))
        return false;

    return true;
}

// SUBROUTINE - setChannelPolarity
//
// Sets the channel passed to the proper state depending on the set parameter for
//    responding to the Conditional Search.
//
// 'm_PortNum'   - number 0 to MAX_PORTNUM-1.  This number was provided to
//               OpenCOM to indicate the port number.
// 'SNum'      - The Serial number of the DS2405 that info is requested on
// 'channel'   - channel to set
// 'seton'     - 'true' turn channel on for polarity condition search, 
//               'false' turn channel off
//
// Returns: true(1):    If set
//          false(0):   If an error occured
//
bool CDS2408::SetChannelPolarity( int channel, bool seton)
{
    uchar reg[3];
    uchar polarity;

    polarity = (uchar) (0x01 << channel);;

    if(!ReadRegister(COND_SEARCH_CHANNEL_SEL,reg))
        return false;

    if(seton)
        reg[1] = (uchar) (reg[1] | polarity);
    else
        reg[1] = (uchar) (reg[1] & ~polarity);

    if(!SetRegister(COND_SEARCH_CHANNEL_SEL,reg))
        return false;

    return true;
}

// SUBROUTINE - getChannelMask
//
// Retrieves the information if the channel is masked for the Conditional Search.
//
// 'm_PortNum'   - number 0 to MAX_PORTNUM-1.  This number was provided to
//               OpenCOM to indicate the port number.
// 'SNum'      - The Serial number of the DS2405 that info is requested on
// 'channel'   - Channel to look for mask
//
// Returns: true(1):    If channel is masked
//          false(0):   otherwise
//
bool CDS2408::GetChannelMask( int channel)
{
    uchar reg[3];
    uchar mask;

    if(!ReadRegister(COND_SEARCH_CHANNEL_SEL, reg))
        return false;

    mask = (uchar) (0x01 << channel);

    return ((reg[0] & mask) == mask);
}

// SUBROUTINE - getChannelPolarity
//
// Retrieves the polarity of the channel for the Conditional Search.
//
// 'm_PortNum'   - number 0 to MAX_PORTNUM-1.  This number was provided to
//               OpenCOM to indicate the port number.
// 'SNum'      - The Serial number of the DS2405 that info is requested on
// 'channel'   - Channel to look for polarity
//
// Returns: true(1):    If channel polarity is set
//          false(0):   otherwise
//
bool CDS2408::GetChannelPolarity( int channel, uchar *reg)
{
    uchar polarity;

    polarity = (uchar) (0x01 << channel);;

    return ((reg[1] & polarity) == polarity);
}



//--------------------------------------------------------------------------
// SUBROUTINE - SetRegister
//
// This routine set the register values
//
// 'm_PortNum'  - number 0 to MAX_PORTNUM-1.  This number was provided to
//              OpenCOM to indicate the port number.
// 'SNum'     - The serial number of the swith that is to be turned on or off
// 'reg'      - register values to be set.
//
// Returns: true(1):    If set is successful
//          false(0):   If set is not successful
//
bool CDS2408::SetRegister(uchar regAddr, uchar *reg)
{
    uchar buff[10];
    int i;
    bool retVal = false;

    m_Master->owSerialNum(m_PortNum,m_SerialNum, false);

    if(m_Master->owAccess(m_PortNum))
    {
        buff[0] = 0xCC;
        buff[1] = (uchar) ((regAddr) & 0xFF);
        buff[2] = (uchar) ((((regAddr) & 0xFFFF) >> 8) & 0xFF);

        for(i=0;i<3;i++)
        {
            buff[i+3] = reg[i];
        }

        if(m_Master->owBlock(m_PortNum,false,&buff[0],6))
        {
            m_Master->owTouchReset( m_PortNum );
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

    if (!retVal)
    {
        PushError( AFOERROR_DS2408_UNABLE_TO_SET_REGISTER, m_NetNumber, m_DeviceNumber);
        if (!m_Master->DS2480Detect( m_PortNum )){
          COneWireNet *net = reinterpret_cast<COneWireNet*>(m_NetPtr);
          net->GetNetHandler(m_NetNumber-1)->recheckMaster = true;
        }
    }
    
    return retVal;
}

//--------------------------------------------------------------------------
// SUBROUTINE - ReadRegister
//
// This routine reads the register values.
//
// 'm_PortNum' - number 0 to MAX_PORTNUM-1.  This number was provided to
//             OpenCOM to indicate the port number.
// 'SNum'    - The Serial number of the DS2405 that info is requested on
// 'reg'     - The register values that were read.
//
// Returns: true(1):    If read was successful.
//          false(0):   Otherwise
//
bool CDS2408::ReadRegister( uchar regAddr, uchar *reg)
{
    uchar buff[32];
    int send_cnt = 0, i = 0;
    ushort lastcrc16;
    bool retVal = false;

    memset (buff, 0x0, 32*sizeof(uchar));
    
    m_Master->owSerialNum(m_PortNum,m_SerialNum, false);
    

    if(m_Master->owAccess(m_PortNum))
    {
        buff[send_cnt++] = 0xF0;

        // address 1
        buff[send_cnt++] = (uchar) (regAddr & 0xFF);
        // address 2
        buff[send_cnt++] = (uchar) (((regAddr & 0xFFFF) >> 8) & 0xFF);

        for(i=0;i<((0x8F - regAddr)+1);i++)
        {
            buff[send_cnt++] = 0xFF;
        }
        
        buff[send_cnt++] = 0xFF;
        buff[send_cnt++] = 0xFF;
        
        if(m_Master->owBlock(m_PortNum,false,&buff[0],send_cnt))
        {
        
            // initialize the CRC16
            m_CRCUtil->setcrc16(m_PortNum,0);
            
            // perform the CRC16 on all packet
            for (i = 0; i < send_cnt; i++)
            {
                lastcrc16 = m_CRCUtil->docrc16(m_PortNum,buff[i]);
            }
    
            // verify CRC16 is correct
            if (lastcrc16 == 0xb001)
            {
                for(i=0;i<3;i++)
                {
                    reg[i] = buff[i+3];
                    if (regAddr == 0x88)
                    {
                        m_Register[i] = buff[i+3];
                    }
                }
                
                retVal = true;
            }
            else
            {
                SendPresenceError();
            }
        }
        else
        {
            PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
        }
    }
    else
    {
        PushError( OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
    }

    if (!retVal)
    {
        PushError( AFOERROR_DS2408_UNABLE_TO_READ_REGISTER, m_NetNumber, m_DeviceNumber);
        if (!m_Master->DS2480Detect( m_PortNum )){
          COneWireNet *net = reinterpret_cast<COneWireNet*>(m_NetPtr);
          net->GetNetHandler(m_NetNumber-1)->recheckMaster = true;
        }
    }
    return retVal;
}



bool CDS2408::ReadRegister( uchar * reg )
{
    return ReadRegister( PIO_LOGIC_STATE, reg);
}

//--------------------------------------------------------------------------
// SUBROUTINE - getLatchState
//
// Checks the latch state of the indicated channel.
//
// 'channel' - the channel to check the latch state
// 'state'   - array of state variables that has already been read.
//
// Returns: true(1):    If latch is set
//          false(0):   If latch is not set
//
bool CDS2408::GetLatchState(int channel, uchar *state)
{
    uchar latch = (uchar) (0x01 << channel);
    return ((state [1] & latch) == latch);
}

//--------------------------------------------------------------------------
// SUBROUTINE - getLevel
//
// Checks the sensed level on the indicated channel.
//
// 'channel' - the channel to check the latch state
// 'state'   - array of state variables that has already been read.
//
// Returns: true(1):    If level is high
//          false(0):   If level is low
//
bool CDS2408::GetLevel(int channel, uchar *state)
{
    uchar  level = (uchar) (0x01 << channel);
    return ((state[0] & level) == level);
}

//--------------------------------------------------------------------------
// SUBROUTINE - getSensedActivity
//
// Checks if the indicated channel has experienced activity.
//
// 'channel' - the channel to check the latch state
// 'state'   - array of state variables that has already been read.
//
// Returns: true(1):    If activity was detected
//          false(0):   If no activity was detected
//
bool CDS2408::GetSensedActivity (int channel, uchar *state)
{
    uchar activity = (uchar) (0x01 << channel);
    return ((state[2] & activity) == activity);
}

//--------------------------------------------------------------------------
// SUBROUTINE - setLatchState
//
// Sets the channel to the state provided.
//
// 'm_PortNum' - number 0 to MAX_PORTNUM-1.  This number was provided to
//             OpenCOM to indicate the port number.
// 'SNum'    - The Serial number of the DS2405 that info is requested on
// 'channel' - the channel to check the latch state
// 'set'     - to set the channel on(1) or off(0)
//
// Returns: true(1):    If latch was set
//          false(0):   If an error occured
//
bool CDS2408::SetLatchState( int channel, uchar set)
{
    uchar state[3];
    uchar latch = (uchar) (0x01 << channel);

    if(ReadRegister( PIO_LOGIC_STATE, state))
    {
        if(set)
            state[1] = (uchar) (state[1] | latch);
        else
            state[1] = (uchar) (state[1] & ~latch);

        if(SetSwitch(state))
            return true;
    }

    return false;
}

/////////////////////////////////////////////////////////////
//              ClearActivityLatch
/////////////////////////////////////////////////////////////
bool CDS2408::ClearActivityLatch( )
{
    uchar buff[2];
    bool retVal = false;
    
    m_Master->owSerialNum(m_PortNum,m_SerialNum, false);

    if(m_Master->owAccess(m_PortNum))
    {
        buff[0] = 0xC3;
        buff[1] = 0xFF;

        if(m_Master->owBlock(m_PortNum,false,&buff[0],2))
        {
            if((buff[1] != 0xAA))
            {
                SendPresenceError();
            }
            else
            {
                //Reset
                m_Master->owTouchReset( m_PortNum );
                retVal = true;
            }
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
    
    if (!retVal)
    {
        PushError( AFOERROR_DS2408_UNABLE_TO_CLEAR_LATCHES, m_NetNumber, m_DeviceNumber);
        if (!m_Master->DS2480Detect( m_PortNum )){
          COneWireNet *net = reinterpret_cast<COneWireNet*>(m_NetPtr);
          net->GetNetHandler(m_NetNumber-1)->recheckMaster = true;
        }
    }
    
    return retVal;
}

/////////////////////////////////////////////////////////////
//              GetChannelState
/////////////////////////////////////////////////////////////
bool CDS2408::GetChannelState( int channel, bool updateFirst )
{
    bool state = false;
    uchar stateRegister[3]; //Internal State register
    
    //First check the channel
    if ((channel >= 0) && (channel < 8))
    {
        if (updateFirst)
        {
            //Get the state of the channels
            ReadSwitch( stateRegister );
        }
        else
        {
            for (int i = 0; i < 3; i++)
            {
                stateRegister[i] = m_Register[i];
            }
        }
        
        //Read the state
        state = GetLatchState( channel, stateRegister );
    }
    
    //Return the value negated
    return !state;
        
}

/////////////////////////////////////////////////////////////
//              GetChannelLevel
/////////////////////////////////////////////////////////////
bool CDS2408::GetChannelLevel( int channel, bool updateFirst )
{
    bool state = false;
    uchar stateRegister[3]; //Internal State register
    
    //First check the channel
    if ((channel >= 0) && (channel < 8))
    {
        if (updateFirst)
        {
            //Get the state of the channels
            ReadSwitch( stateRegister );
        }
        else
        {
            for ( int i = 0; i < 3; i++)
            {
                stateRegister[i] = m_Register[i];
            }
        }
        
        //Read the state
        state = GetLevel( channel, stateRegister );
    }
    
    return !state;
}
///////////////////////////////////////////////////////////////////
bool CDS2408::InitDevice( )
{
//     bool retVal = true;
//     uchar reg[3];
//     
//     if (!ReadRegister( COND_SEARCH_CHANNEL_SEL, reg))
//     {
//         return false;
//     }
//     
//     if (m_ActivateConditionalSearch)
//     {
//         reg[2] = (reg[2] | 0x01);
//     }
//     else
//     {
//         reg[2] = (reg[2] & 0xFE);
//     }
//     
//     //Setup AND or OR logic on activity search
//     if (m_UseAndLogic)
//     {
//         reg[2] = reg[2] | 0x02;
//     }
//     else
//     {
//         reg[2] = reg[2] & 0xFD;
//     }
//     
//     //Clear power on
//     reg[2] = (reg[2] & 0xF7);
//     
//     //Set Polarity Mask
//     reg[1] = (uchar)m_PolarityMask;
//     
//     //Set Channel mask
//      reg[0] = (uchar)m_ActivityMask;
//     
//     //Write all the configuration
//     if(!SetRegister(COND_SEARCH_CHANNEL_SEL,reg))
//     {
//         retVal = false;
//     }
//     
//     retVal = retVal && ClearActivityLatch();
//     
//     return retVal;
    
    return true;
}
/////////////////////////////////////////////////////////////////////////////
bool CDS2408::ChangeOutput(int channel )
{
    uchar reg[3] = {0x0, 0x0, 0x0};
    uchar mask;

    if (ReadRegister( PIO_LOGIC_STATE, reg ))
    {
        mask = reg[0] & (0x01 << channel);
        
        if (mask)
        {
            return SetLatchState( channel, 0);
        }
        else
        {
            return SetLatchState( channel, 1);
        }
    }
    
    return false;
    
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
///////////////////  NUOVE FUNZIONI DEL DRIVER ///////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
bool CDS2408::UpdateData()
{
    if (ReadPIORegisters())
    {
        //Controllo se c'è attività e la resetto
        m_DriverData.regData[3] = m_DriverData.regData[2];
        if ((m_DriverData.regData[2] != 0) && (m_ActivateConditionalSearch) && (m_ModuleType!=2))
        {
            ClearActivityLatch();
        }
        else if (!m_ActivateConditionalSearch)
        {
            //Se non mi interessa l'attività la azzero nel registro
            m_DriverData.regData[2] = 0;
            m_DriverData.regData[3] = 0;
        }
            
        if (m_DebugLevel)
        {
            cout << "DS2408 -- Device"<<m_DeviceNumber<<
                    " Registri:"<<(int)m_DriverData.regData[0]<<"-"<<(int)m_DriverData.regData[1]<<"-"<<(int)m_DriverData.regData[2]<<endl;
        }
        return true;
    }
    else
    {
        m_DriverData.isValid = false;
    }

    return false;
}
////////////////////////////////////////////////////////////////////
bool CDS2408::ReadPIORegisters( uchar *state )
{
    uchar buff[32];
    int send_cnt = 0, i = 0;
    ushort lastcrc16;
    bool retVal = false;

    memset (buff, 0x0, 32*sizeof(uchar));
    
    m_Master->owSerialNum(m_PortNum,m_SerialNum, false);
    

    if(m_Master->owAccess(m_PortNum))
    {
        buff[send_cnt++] = 0xF0;

        // address 1
        buff[send_cnt++] = (uchar) (0x88 & 0xFF);
        // address 2
        buff[send_cnt++] = (uchar) (((0x88 & 0xFFFF) >> 8) & 0xFF);

        for(i=0;i<((0x8F-0x88)+1);i++)
        {
            buff[send_cnt++] = 0xFF;
        }

        buff[send_cnt++] = 0xFF;
        buff[send_cnt++] = 0xFF;
        
        if(m_Master->owBlock(m_PortNum,false,&buff[0],send_cnt))
        {
        
            // initialize the CRC16
            m_CRCUtil->setcrc16(m_PortNum,0);

            // perform the CRC16 on all packet
            for (i = 0; i < send_cnt; i++)
            {
                lastcrc16 = m_CRCUtil->docrc16(m_PortNum,buff[i]);
            }
            
            // verify CRC16 is correct
            if (lastcrc16 == 0xb001)
            {
                for(i=0;i<3;i++)
                {
                    if (state != 0x0)
                    {
                        state[i] = buff[i+3];
                    }
                    m_DriverData.regData[i] = buff[i+3];
                    m_DriverData.isValid = true;
                    //TBR in fase di transizione questo rimane poi si toglierà
                    m_Register[i] = buff[i+3];
                }
                
                retVal = true;
            }
            else
            {
                SendPresenceError();
            }
        }
        else
        {
            PushError(OWERROR_BLOCK_FAILED, m_NetNumber, m_DeviceNumber);
        }
    }
    else
    {
        PushError( OWERROR_ACCESS_FAILED, m_NetNumber, m_DeviceNumber);
    }

    if (!retVal)
    {
        PushError( AFOERROR_DS2408_UNABLE_TO_READSWITCH, m_NetNumber, m_DeviceNumber);
        if (!m_Master->DS2480Detect( m_PortNum )){
          COneWireNet *net = reinterpret_cast<COneWireNet*>(m_NetPtr);
          net->GetNetHandler(m_NetNumber-1)->recheckMaster = true;
        }
    }

    return retVal;
}

