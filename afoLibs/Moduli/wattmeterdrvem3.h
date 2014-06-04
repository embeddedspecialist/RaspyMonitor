/***************************************************************************
*   Copyright (C) 2007 by Alessandro Mirri                                *
*   alessandro.mirri@newtohm.it                                           *
*                                                                         *
*   This program is NOT free software; you can NOT redistribute it and/or *
*   modify it in any way without the authorization of the author          *
*                                                                         *
*   This program is distributed WITHOUT ANY WARRANTY;                     *
*   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
*                                                                         *
***************************************************************************/
#ifndef STDWATTMETERDRVEM3_H
#define STDWATTMETERDRVEM3_H

#include <vafoobject.h>
#include "cownet.h"
#include "commonDefinitions.h"

#define READ_PARAMS 't'
#define READ_MEMORY 'Q'
#define STX 0x02
#define ETX 0x03
#define DLE 0x10
#define NACK 0x15
#define ACK 0x06
        
        
using namespace std;

/**
Driver class for the Elcotronic EM3 wattmeter device

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CWattMeterDrvEM3 : public CVAFOObject
{
public:
    CWattMeterDrvEM3(COWNET *master, char *configString);

    ~CWattMeterDrvEM3();
    
    /**
     * Function used to change the address of the device on the RS485 NET
     * @param newIDX the new address
     * @return true if operation successfull
     */
    bool SetIDX (int newIDX);
    
    /**
     * Gets the index of the device on the RS485 NET
     * @return the Index of the device or -1 if no index was assigned
     */
    unsigned char GetIDX() {return m_IDX;};
    
    /**
     * Function used to setup the recording of the device. On 30/01/2007 there is only one data: KWh
     * @return true if setup successfull
     */
    bool Setup();
    

    /**
     * Gets all the parameters inside the device.
     * @param destination a 38 element array where the parameters are stored
     * @return true if operation successfull
     */
    bool ReadParameters(float* destination);
    
                
private:
    
    //!Index of the device in the rs485 NET (from 0 to 127, -1 means unspecified)
    unsigned char m_IDX;
    
    /**
     * Parses the incoming message to extract all the parameters
     * @param message the incoming message
     * @param destination the destination array (38 elements)
     * @return true if operation successfull
     */
    bool ParseMessage(char *message, float *destination);
    
    /**
     * Starting from the 2 byte input array in the following form:
     * byte[0]
     * bit 7-4 = first (MSB) nibble
     * bit 3-0 = second nibble
     * byte[1]
     * bit 7-4 = third (LSB) nibble
     * bit 3 = Mega scale (1 == means Mega)
     * bit 2 = Kilo scale (1 == means Kilo)
     * bit 1-0 = position of decimal point 0 = no decimal point, 1 decimal point before LSB nibble, 2 decimal point before middle nibble
     * @param inBuffer input buffer containing the values to be converted in float
     * @return the float representation of the number contained in input buffer or ANALOG_ERRVAL if an error occurred
     */
    float ExtractVal(unsigned char* inBuffer);
    
    /**
     * Starting from the 2 byte input array in the following form:
     * byte[0]
     * bit 7-4 = first (MSB) nibble
     * bit 3-0 = second nibble
     * byte[1]
     * bit 7-4 = third (LSB) nibble
     * bit 3 = Indicates if it is a capacitance measure or an inductive one (1 means capacitance)
     * bit 2 = Unknown
     * bit 1-0 = position of decimal point 0 = no decimal point, 1 decimal point before LSB nibble, 2 decimal point before middle nibble
     * @param inBuffer input buffer containing the values to be converted in float
     * @return the float representation of the number contained in input buffer or ANALOG_ERRVAL if an error occurred
     */
    float ExtractValCos(unsigned char* inBuffer);
    
    /**
     * Starting from the 3 byte input array in the following form:
     * byte[0]
     * bit 7-4 = first (MSB) nibble
     * bit 3-0 = second nibble
     * byte[1]
     * bit 7-4 = third nibble
     * bit 3-0 = fourth nibble
     * byte[1]
     * bit 7-4 = fifth nibble
     * bit 3-0 = decimal nibble
     * @param inBuffer the three byte buffercontaining the values to be converted in float
     * @return the float representation of the number contained in the provided array or ANALOG_ERRVAL if an error occurred
     */
    float ExtractVallong(unsigned char* inBuffer);

    COWNET *m_Master;

};


#endif
