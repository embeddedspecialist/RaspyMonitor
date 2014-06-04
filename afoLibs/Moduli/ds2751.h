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
#ifndef DS2751_H
#define DS2751_H

#include "vdevice.h"
#include "cownet.h"

#define VOLT_MSB 0x0C
#define VOLT_LSB 0x0D
#define TEMP_MSB 0x18
#define TEMP_LSB 0x19
#define SPEC_REG 0x08
#define CURR_MSB 0x0E
#define CURR_LSB 0x0F
#define SRAMSIZE 16

//using namespace std;
/**
This class is a driver for the DS2751 device

	@author Alessandro Mirri <amirrix@Trantor>
*/
class CDS2751 : public CVDevice
{
public:
    CDS2751(int portNum, COWNET* master,  const char* configString);

    ~CDS2751();

        /**
     * Reads the last temperature measured from the internal device
     * @return the current measured temperature, or TEMP_ERRVAL if an error occurred
         */
    float ReadTemperature(bool updateFirst);

    bool ReadTemperature(bool updateFirst, float *newTemp );

    /**
     * Reads the voltage sensed
     * @return the value if operation succesful, -1 otherwise
     */
    float ReadVoltage(bool updateFirst);


    float ReadCurrent(bool updateFirst);

    bool GetSRAM(uchar* destination);
    bool GetMemoryByte (uchar *destination, uchar address);
    bool WriteSRAMByte(uchar address, uchar value);
    bool WriteAllSRAM(uchar *values);

    bool SetSpecialReg(uchar newReg);
    bool GetSpecialReg(uchar *destReg);

    private:

        uchar m_SRAMMap[SRAMSIZE];
        float m_Temp;
        float m_Voltage;
        float m_Current;


};

#endif
