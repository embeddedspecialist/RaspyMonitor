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
#include "timeddido.h"


CTimedDIDO::CTimedDIDO(CVDevice* device, const char* configString, CTimer *timer)
 : CVDIDO(configString, timer)
{
        //Get infos for this class
    if (configString != 0x0 )
    {
        //Get if input or output
        m_IniLib.GetConfigParamInt( configString, "TRANSTIME", &m_TransitionTime, 60);
       
        m_IniLib.GetConfigParamInt( configString, "TIMERID", &m_TimerID, -1);
    
        if ( (m_TimerID > 0) && (timer != 0x0) )
        {
            m_Timer = timer;
            m_UseTimer = true;
        }
        else
        {
            m_UseTimer = false;
        }
    }
    
    m_IsMoving = false;
    m_OutChannel = -1;
    m_OutDevice = 0x0;
    
}

CTimedDIDO::CTimedDIDO( const char* configString, CTimer * timer ) : CVDIDO(configString, timer)
{
    //Get infos for this class
    if (configString != 0x0 )
    {
        //Get if input or output
        m_IniLib.GetConfigParamInt( configString, "TRANSTIME", &m_TransitionTime, 60);
       
        m_IniLib.GetConfigParamInt( configString, "TIMERID", &m_TimerID, -1);
    
        if ( (m_TimerID > 0) && (timer != 0x0) )
        {
            m_Timer = timer;
            m_UseTimer = true;
        }
        else
        {
            m_UseTimer = false;
        }
    }
    
    m_IsMoving = false;
    m_OutChannel = -1;
    m_OutDevice = 0x0;
}

CTimedDIDO::~CTimedDIDO()
{
}


