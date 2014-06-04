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
#ifndef STEPDIGITALOUT_H
#define STEPDIGITALOUT_H

#include "vmultidido.h"
#include "cstring.h"
#include "timer.h"

//Time used to change the output
#define STEPTIME 100 
/**
Implementation of a digital output class capable of interfacing with step relais. This class has a connection with a single digitalIN channel for status verification and single digitalOUT channel for command. Everytime there is a tunrON or turnOFF command the digitalOUT is moved from open to close and then back to open to force the relais to move just one step.

config file line:
DeviceXX=NAME:StepDigitalOut, ADDR, STARTV, INPUT:,CHANNEL,OUTPUT,OUTCHANNEL, TIMERID, INVERTOUT,INVERTIN,COMMENT

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CStepDigitalOut : public CVMultiDIDO
{
public:
    CStepDigitalOut(const char* configString, CTimer* timer);

    ~CStepDigitalOut();
    
    bool InitOutput(int outIndex = 0);

    bool Update(bool updateData);
    bool Update2(bool updateData);
    bool SetVal(float val){return false;};
    
    CString GetSpontaneousData(int lParam = 0) { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};

    bool ChangeOutput(int outIndex = 0);

    bool SetOutput(bool newState, bool update);
    
    bool SetOutput(bool newState,int outIndex = 0){ return SetOutput(newState, true);};
    
    
    private:
        
        int m_IsInitOK;
        
        bool m_IsStepDone;
        
        bool m_WorkOnActivity;

};

#endif
