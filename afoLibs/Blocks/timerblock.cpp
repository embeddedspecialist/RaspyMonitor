/***************************************************************************
 *   Copyright (C) 2007 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 ***************************************************************************/
#include "timerblock.h"

CTimerBlock::CTimerBlock(const char* configString)
        : CBlock(configString)
{
     m_LibIniReader.GetConfigParamInt(configString, "ID",&m_TimerID, -1);

     m_IsTimerEnabled = false;
     
     if (m_TimerID < 0){
         m_IsTimerEnabled = false;
     }
     else if (m_Timer.LoadTimers()){
         m_IsTimerEnabled = true;
     }
}

CTimerBlock::~CTimerBlock()
{
    
}

//////////////////////////////////////////
// Update
/////////////////////////////////////////
bool CTimerBlock::Update( )
{
    if (m_IsTimerEnabled){
        float timerOut;

        timerOut = m_Timer.GetValue(m_TimerID, TIMERVAL_DIGITAL);
        SetOutputVal(0,timerOut,true);

        timerOut = m_Timer.GetValue(m_TimerID,TIMERVAL_ANALOG);
        SetOutputVal(1,timerOut,true);

    }
    else {
        SetOutputVal(0,-100.0,false);
        SetOutputVal(1,-100.0,false);
    }
    
    return true;
}

