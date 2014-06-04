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
#ifndef STDCDS2890_H
#define STDCDS2890_H

#include "vdevice.h"
#include <string.h>
#include <stdio.h>
#include <iostream>
#include "LibIniFile.h"
#include "cownet.h"
#include "timeUtil.h"

//using namespace std;

/**
This class encapsulates the functions needed to use the DS2890 Digital Potentiometer device

 * The configuration String has the following parameters:
 -MAXVOLTAGE to set the voltage at which the vdd pin is connected
 * UPDATESTATE: se 1 indica che lo stato deve essere letto come driver ad ogni ciclo, se 0 questo non viene fatto
 *              può servire per non appessantire troppo il ciclo di programma in alcuni casi in cui il suo stato è
 *              già riportato attraverso altri controlli (es. PID);

 *
 * Rev. 0.1 -- 31/01/2006
 * Prima implementaazione
 *
 * NAME:DS2890,SN:xxxxx,UPDATESTATE:0
@author Alessandro Mirri
*/
class CDS2890 : public CVDevice
{
public:
    CDS2890(int portNum, COWNET *master, const char* configString);

    ~CDS2890();

    
    /**
     * Aggiorna lo stato come driver
     * @return true se tutto ok
     */
    bool UpdateState();
    
    /**
     * Allows to read the position of the wiper
     * @return position of the wiper between 0x0 and 0xff
     */
    int ReadPosition();
    bool ReadPosition(int *pos);

    /**
     * Allows to get the last known position of the device without accessing it
     * @return current position of the wiper
     */
    int GetCurrentPosition() { return m_CurrentPosition;};

    /**
     * Set the position of the wiper as an absolute number between 0x0 and 0xff
     * @param newPos the new position of the wiper
     * @return true if operation successfull
     */
    bool SetPosition(uchar newPos);

    /**
     * Increment the position by one step
     * @return true if successfull
     */
    bool Increment1Step();

    /**
     * Increment position by a given number of steps
     * @param nSteps the number of steps to increment
     * @return true if operation successfull
     */
    bool IncrementBySteps(int nSteps);

    /**
     * Decrement wiper position by one step
     * @return true if operation successfull
     */
    bool Decrement1Step();

    /**
     * Decrement the wiper position by a given number of steps
     * @param nSteps the number of steps to decrement
     * @return true if operation successfull
     */
    bool DecrementBySteps(int nSteps);

    /**
     * Read the control register
     * @return the control register
     */
    uchar ReadControlRegister();
    bool ReadControlRegister(uchar *contReg);

    /**
     * Write the control register
     * @param newVal the new control register value
     * @return true if operation successfull
     */
    bool WriteControlRegister(uchar newVal);


    private:
        //!The current wiper position
        uchar m_CurrentPosition;

        //!Control register
        uchar m_ControlRegister;

        //!Feature register
        uchar m_FeatureRegister;
        
        bool m_UpdateState;

};


#endif
