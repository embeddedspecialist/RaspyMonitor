/***************************************************************************
 *   Copyright (C) 2006 by Alessandro Mirri   *
 *   amirrix@Trantor   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "ds2751.h"
#include "conewirenet.h"

#ifndef AFO_NO_VALVECTRL
CDS2751::CDS2751(int portNum, COWNET* master, const char* configString): CVDevice(configString, master)
{
    memset (m_SRAMMap, 0x0, SRAMSIZE*sizeof(char));
    
    m_Name = "DS2751";
    m_FamilyNumber = 0x51;
    m_PortNum = portNum;
    
    m_Temp = TEMP_ERRVAL;
    m_Voltage = ANALOG_ERRVAL;
    m_Current = ANALOG_ERRVAL;
    
}


CDS2751::~CDS2751()
{
}

///////////////////////////////////////////////////
//              ReadTemperature
///////////////////////////////////////////////////
float CDS2751::ReadTemperature( bool updateFirst )
{
    float temperature;
    
    ReadTemperature( updateFirst, &temperature );
    
    return temperature;
}

///////////////////////////////////////////////////
//              ReadTemperature
///////////////////////////////////////////////////
bool CDS2751::ReadTemperature( bool updateFirst, float * newTemp )
{
    bool retVal = true;
    //Temperature MSB, LSB
    uchar buffer[2];
    
    *newTemp = 0;
    
    if (updateFirst)
    {
        if (GetMemoryByte( buffer, TEMP_MSB ) && GetMemoryByte( &buffer[1], TEMP_LSB ))
        {
            *newTemp = ( ((buffer[0] << 8) | buffer[1]) >> 5 ) * 0.125;
            m_Temp = *newTemp;
        }
        else
        {
            //TODO mettere errorre
        }
    }
    else
    {
        *newTemp = m_Temp;
    }
    
    return retVal;
    
}

///////////////////////////////////////////////////
//              ReadVoltage
///////////////////////////////////////////////////
float CDS2751::ReadVoltage( bool updateFirst )
{   
    //Voltage MSB, LSB
    uchar buffer[2];
    
    if (updateFirst)
    {
        if (GetMemoryByte( buffer, VOLT_MSB ) && GetMemoryByte( &buffer[1], VOLT_LSB ))
        {
            m_Voltage = ( ((buffer[0] << 8) | buffer[1]) >> 5 ) * 0.00488;
        }
        else
        {
            //TODO mettere errorre
        }
    }

    
    return m_Voltage;
}

///////////////////////////////////////////////////
//              ReadCurrent
///////////////////////////////////////////////////
float CDS2751::ReadCurrent( bool updateFirst )
{
    //Voltage MSB, LSB
    uchar buffer[2];
    
    if (updateFirst)
    {
        if (GetMemoryByte( buffer, CURR_MSB ) && GetMemoryByte( &buffer[1], CURR_LSB ))
        {
            m_Current = ( ((buffer[0] << 8) | buffer[1]) >> 5 ) * 0.00625;
        }
        else
        {
            //TODO mettere errorre
        }
    }

    
    return m_Current;
}

///////////////////////////////////////////////////
//              SetSpecialReg
///////////////////////////////////////////////////
bool CDS2751::SetSpecialReg( uchar newReg )
{
    bool retVal = false;
    uchar sendBuffer[3];
    
    this->m_Master->owSerialNum( m_PortNum,m_SerialNum, false );
    
    if (this->m_Master->owAccess( m_PortNum ))
    {
        sendBuffer[0] = 0x6c;
        sendBuffer[1] = SPEC_REG;
        sendBuffer[2] = newReg;
        
        if (this->m_Master->owBlock(m_PortNum, false, sendBuffer, 3 ))
        {
            msDelay( 10 );
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
    
    return retVal;
        
}

///////////////////////////////////////////////////
//              GetSpecialReg
///////////////////////////////////////////////////
bool CDS2751::GetSpecialReg( uchar * destReg )
{
    bool retVal = false;
    uchar sendBuffer[3];
    
    this->m_Master->owSerialNum( m_PortNum, m_SerialNum, false);
    
    if (this->m_Master->owAccess( m_PortNum ))
    {
        sendBuffer[0] = 0x69;
        sendBuffer[1] = SPEC_REG;
        sendBuffer[2] = 0xFF;
        
        if (this->m_Master->owBlock( m_PortNum, false, sendBuffer, 3))
        {
            msDelay (10) ;
            
            *destReg = sendBuffer[2];
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
    
    
    return retVal;
}

///////////////////////////////////////////////////
//              GetSRAM
///////////////////////////////////////////////////
bool CDS2751::GetSRAM( uchar * destination )
{
    bool retVal = false;
    uchar buffer[18];

    memset (destination, 0x0, SRAMSIZE);
    
    this->m_Master->owSerialNum(m_PortNum,m_SerialNum, false);

    this->m_Master->owTouchReset( m_PortNum );
    
    if(this->m_Master->owAccess(m_PortNum))
    {
        //Set all the buffer register to 1's
        memset (buffer, 0xff, 18*sizeof(uchar));
            
        //Change the first byte to insert the ReadData command
        buffer[0] = 0x69;
        //Start from address 0
        buffer[1] = 0x80;
            
        if (this->m_Master->owBlock( m_PortNum, false, buffer, 18))
        {
            msDelay (10) ;
            memcpy(destination, buffer+2, SRAMSIZE);
            memcpy(m_SRAMMap, buffer+2, SRAMSIZE);
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
        PushError( AFOERROR_DS2751_UNABLE_TO_READ_MEM, m_NetNumber, m_DeviceNumber);
    }

    return retVal;
}

///////////////////////////////////////////////////
//              WriteSRAMByte
///////////////////////////////////////////////////
bool CDS2751::WriteSRAMByte( uchar address, uchar value )
{
    bool retVal = false;
    uchar sendBuffer[2];
    
    this->m_Master->owTouchReset( m_PortNum );
    
    this->m_Master->owSerialNum( m_PortNum, m_SerialNum, false);
    
    if (this->m_Master->owAccess( m_PortNum ))
    {
        sendBuffer[0] = 0x6C;
        sendBuffer[1] = address;
        sendBuffer[2] = value;
        
        if (this->m_Master->owBlock( m_PortNum, false, sendBuffer, 3))
        {
            msDelay (10);
            
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
    
    return retVal;
}

///////////////////////////////////////////////////
//              GetMemoryByte
///////////////////////////////////////////////////
bool CDS2751::GetMemoryByte( uchar * destination, uchar address )
{
    bool retVal = false;
    uchar buffer[3];
    
    this->m_Master->owSerialNum(m_PortNum,m_SerialNum, false);

    this->m_Master->owTouchReset( m_PortNum );
    
    if(this->m_Master->owAccess(m_PortNum))
    {
        //Change the first byte to insert the ReadData command
        buffer[0] = 0x69;
        //Start from address 0
        buffer[1] = address;
        
        buffer[2] = 0xFF;
            
        if (this->m_Master->owBlock( m_PortNum, false, buffer, 3))
        {
            msDelay (10) ;
            *destination = buffer[2];
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
        PushError( AFOERROR_DS2751_UNABLE_TO_READ_MEM, m_NetNumber, m_DeviceNumber);
    }

    return retVal;
}

bool CDS2751::WriteAllSRAM( uchar * values )
{
    bool retVal = false;
    uchar sendBuffer[32];
    int i = 0;
    
    this->m_Master->owTouchReset( m_PortNum );
    this->m_Master->owSerialNum( m_PortNum, m_SerialNum, false);
    if (this->m_Master->owAccess( m_PortNum ))
    {
        sendBuffer[0] = 0x6C;
        sendBuffer[1] = 0x80;
        for (i = 0; i < SRAMSIZE; i++)
        {
            sendBuffer[i+2] = values[i];
        }
        
        if (this->m_Master->owBlock( m_PortNum, false, sendBuffer, 18))
        {
            msDelay (20);
            
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
    
    return retVal;
}
//////////////////////////////// SMALL TARGET ///////////////////////
#else

CDS2751::CDS2751( int portNum, COWNET * master, const char* configString ): CVDevice(configString, master)
{
}

CDS2751::~ CDS2751( )
{
}

float CDS2751::ReadTemperature( bool updateFirst )
{
    return -1.0;
}

bool CDS2751::ReadTemperature( bool updateFirst, float * newTemp )
{
    return true;
}

float CDS2751::ReadVoltage( bool updateFirst )
{
    return -1.0;
}

float CDS2751::ReadCurrent( bool updateFirst )
{
    return -1.0;
}

bool CDS2751::GetSRAM( uchar * destination )
{
    return true;
}

bool CDS2751::GetMemoryByte( uchar * destination, uchar address )
{
    return true;
}

bool CDS2751::WriteSRAMByte( uchar address, uchar value )
{
    return true;
}

bool CDS2751::WriteAllSRAM( uchar * values )
{
    return true;
}

bool CDS2751::SetSpecialReg( uchar newReg )
{
    return true;
}

bool CDS2751::GetSpecialReg( uchar * destReg )
{
    return true;
}


#endif
