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
#ifndef BUTTONCONTROLLER_H
#define BUTTONCONTROLLER_H

#include "vmultidido.h"
#include "vdevice.h"
#include "ds2405.h"
#include "ds2408.h"

/**
This class represents a generic button: a device with a digital input and a digital output commanded by the input
config file line:
 * DeviceXX= NAME:ButtonController,INPUT,CHANNEL,OUTPUT,[OUTCHANNEL,STARTV,ADDR,TIMERID,INVERTOUT,INVERTIN,] [DEFAULTVAL,] COMMENT:
	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CButtonController : public CVMultiDIDO
{
public:
    CButtonController(const char* configString= 0x0, CTimer* timer = 0x0);

    ~CButtonController();

    bool Update(bool updateData);
    bool Update2(bool updateData);
    bool SetVal(float val){return false;};
    
    CString GetSpontaneousData(int lParam = 0) { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};

    bool InitOutput(int outIndex = 0);

    bool ChangeOutput(int outIndex = 0);

    bool SetOutput(bool newState,int outIndex = 0);
    
    //If it is a jolly device it sends a "turn on" or "turn off" command
    bool m_IsJolly;
    int m_JollyValue;
    bool m_RemoteChangeState;

};

#endif
