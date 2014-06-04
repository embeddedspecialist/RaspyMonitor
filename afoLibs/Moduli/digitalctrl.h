/***************************************************************************
 *   Copyright (C) 2006 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it                                           *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 *************************************************************************/
 #ifndef STDDIGITALCTRL_H
#define STDDIGITALCTRL_H

#include <vmultidido.h>

using namespace std;

/**
The new and only one digital controller class: it can be used to manage every type of digital input/output device.
configuration string:
 * NAME:DiDo, ADDR:,INPUT:,INCH:,OUTPUT:, OUTCH:, INVERTIN:, INVERTOUT:, TIMERID:, //REMOTEADDRESS:, REMOTENET:,TIMEOUT://,STATECHECK:,//ACTIVITY://, //INVERTTIMER:,TIMERSTD:,TIMERDEFVAL://,STARTV:,STEP:,
 * where:
 * INPUT, CHANNEL are the inputs of the controller, if no ouput is specified the controller acts just like an input device
 * OUTPUT, OUTCHANNEL are the outputs, if no input is specifed it acts like an "old" DigitalINOUT: the outputs are changed via external command or via the timer
 * REMOTEADDRESS, REMOTENET - must be used when the output has to be sent to an external device or NET
 * ACTIVITY - if 0 Indicates if the output is controlled by the state of the input (default), if 1 that an event on the input controls the output
 * TIMEOUT - Indicates that a change in the input activates a change in the output only for an amount of time (milliseconds)
 * STATECHECK - if 1 (default) indicates that the device must check its internal state against the real state of the output
 * 

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class DigitalCtrl : public CVMultiDIDO
{
public:
    DigitalCtrl(const char* configString, CTimer* timer);

    ~DigitalCtrl();

    bool ConnectController(const char* configString, void* netHandler);

    bool InitOutput(int outIndex = 0){return false;};
    
    bool ChangeOutput(int outIndex = 0){return false;};
    
    bool SetOutput(bool newState, int outIndex = 0){return false;};

    private:
        bool m_IsInitOK;

        bool m_IsTimedDO;
        int m_TimeOutSec;
        bool m_HasInput;
        bool m_HasOutput;

        bool m_GetInputFromActivity; //Indica se l'input è collegato allo stato del DI o all'attività
        
};

#endif
