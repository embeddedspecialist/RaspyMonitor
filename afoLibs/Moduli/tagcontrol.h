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
#ifndef TAGCONTROL_H
#define TAGCONTROL_H

#include <vmultidido.h>
#include "timer.h"
#include "cstring.h"

/**
This class uses a TAG sensor to close or open a digital output. It must be connected to the shutters and to the fan (by means of two distinct objects) of an air treatment unit.
It also has a timer and in a race condition between the timer and the TAG sensor the latter wins.
 * if there is a timeout value the controller starts setting output == input then if input changes the output follows just for the
 * timeout period then returns in the start condition until the input has performed a full cycle. If a changeOutput commands arrives the output is changed only for the timeot period

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CTAGControl : public CVMultiDIDO
{
public:
    CTAGControl(const char* configString, CTimer* timer);

    ~CTAGControl();

    CString GetSpontaneousData(int lParam = 0) { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};
    bool ChangeOutput(int outIndex = 0) { return false;};
    bool InitOutput(int outIndex = 0);
    bool Update(bool updateData);
    bool Update2(bool updateData);
    bool SetVal(float val){return false;};
    bool SetOutput(bool newState,int outIndex = 0);
    
    void SetOutputState(bool newState) {m_OutDeviceStates[0] = newState;};

    private:
        /**
         * Function invoked if the TAG controller has a timeout on the output
         * @param  updateData if True forces the update of the internal register
         * @return true if operation successfull
         */
        bool UpdateTimedOut(int inputValue);

};

#endif
