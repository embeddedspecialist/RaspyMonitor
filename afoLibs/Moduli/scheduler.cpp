/***************************************************************************
 *   Copyright (C) 2005 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it                                           *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 *************************************************************************/
#include "scheduler.h"

///////////////////////////////////////////////////
//              STANDARD CONSTRUCTOR
///////////////////////////////////////////////////
CScheduler::CScheduler()
{
    m_TimeOfLastTransition = 0;
    m_TimeOff2On = 0;
    m_TimeOn2Off = 0;
    m_IsSchedulerActive = false;
    m_IsActualStateOn = false;
}

///////////////////////////////////////////////////
//              STANDARD DESTRUCTOR
///////////////////////////////////////////////////
CScheduler::~CScheduler()
{
}



///////////////////////////////////////////////////
//              SetTimes
///////////////////////////////////////////////////
bool CScheduler::SetTimes( struct tm onTime, struct tm offTime )
{
    int countedSeconds = 0;
    int absONTime = 0, absOFFTime = 0;
    bool retVal = false;
    
    //transform in absolute time
    m_TimeOff2On = onTime.tm_wday * SECONDS_IN_DAY + onTime.tm_hour * SECONDS_IN_HOUR + onTime.tm_min * SECONDS_IN_MINUTE;
    m_TimeOn2Off = offTime.tm_wday * SECONDS_IN_DAY + offTime.tm_hour * SECONDS_IN_HOUR + offTime.tm_min * SECONDS_IN_MINUTE;
    
    m_Times[0] = onTime;
    m_Times[1] = offTime;

    return true;
        
}

///////////////////////////////////////////////////
//              CheckStateOn
///////////////////////////////////////////////////
bool CScheduler::CheckStateOn( )
{
    int absActTime = 0;         //Absolute actual time
    time_t actTime = 0;
    struct tm *timeOfWeek = NULL;   //Time of the week
    
    //Check if scheduler is active
    if (!m_IsSchedulerActive)
    {
        return false;
    }
    
    //get actual time
    actTime = time(NULL);
    timeOfWeek = localtime(&actTime);
    
    //transform it in absolute week time
    absActTime = timeOfWeek->tm_wday*SECONDS_IN_DAY + timeOfWeek->tm_hour * SECONDS_IN_HOUR + timeOfWeek->tm_min*SECONDS_IN_MINUTE;
    
    //Check if, in this moment, the internal state should be ON or OFF
    if (m_TimeOn2Off > m_TimeOff2On)
    {
        if ( (absActTime < m_TimeOn2Off) && (absActTime > m_TimeOff2On) )
        {
            //State should be ON
            m_IsActualStateOn = true;
        }
        else
        {
            //State Should be OFF
            m_IsActualStateOn = false;
        }
    }
    else if (m_TimeOn2Off < m_TimeOff2On)
    {
        if ( (absActTime > m_TimeOn2Off) && (absActTime < m_TimeOff2On))
        {
            //State should be OFF
            m_IsActualStateOn = false;
        }
        else
        {
            //State should be ON
            m_IsActualStateOn = true;
        }
    }
    else
    {
        //Time On-to-Off and Off-to-On are the same, probably they were NOT initialized, return an error
        m_IsActualStateOn = false;
    }
    
    return m_IsActualStateOn;
        
}

///////////////////////////////////////////////////
//              ActivateScheduling
/////////////////////////////////////////////////
bool CScheduler::ActivateScheduling( )
{
    bool retVal = false;
    
    //Activate scheduler
    m_IsSchedulerActive = true;
    
    //check if internal times have been correctly initialized
    if (m_TimeOff2On != m_TimeOn2Off)
    {
        //Update the inernal variable
        CheckStateOn();
        retVal = true;
    }
    
    return retVal;

}

///////////////////////////////////////////////////
//              DeactivateScheduling
/////////////////////////////////////////////////
void CScheduler::DeactivateScheduling( )
{
    m_IsSchedulerActive = false;
}

///////////////////////////////////////////////////
//              GetActivationState
/////////////////////////////////////////////////
bool CScheduler::GetActivationState( )
{
    return m_IsSchedulerActive;
}

///////////////////////////////////////////////////
//              GetTimes
/////////////////////////////////////////////////
void CScheduler::GetTimes( CString & onTime, CString & offTime )
{
    char tempBuffer[8];
    
    //Erase strings
    onTime = "";
    offTime = "";
    memset(tempBuffer, 0, 8);
    
    //Get day
    sprintf (tempBuffer, "%d", m_Times[0].tm_wday);
    onTime = tempBuffer;
    
    memset(tempBuffer, 0, 8);
    if (m_Times[0].tm_hour < 10)
    {
        sprintf (tempBuffer, "0%d", m_Times[0].tm_hour);
    }
    else
    {
        sprintf (tempBuffer, "%d", m_Times[0].tm_hour);
    }
    
    onTime = onTime + tempBuffer;
    
    memset(tempBuffer, 0, 8);
    if (m_Times[0].tm_min < 10)
    {
        sprintf (tempBuffer, "0%d", m_Times[0].tm_min);
    }
    else
    {
        sprintf (tempBuffer, "%d", m_Times[0].tm_min);
    }
    onTime = onTime + tempBuffer;
    
    //OFF
    memset(tempBuffer, 0, 8);
    sprintf (tempBuffer, "%1d", m_Times[1].tm_wday);
    offTime = tempBuffer;
    
    memset(tempBuffer, 0, 8);
    if (m_Times[1].tm_hour < 10)
    {
        sprintf (tempBuffer, "0%d", m_Times[1].tm_hour);
    }
    else
    {
        sprintf (tempBuffer, "%d", m_Times[1].tm_hour);
    }
    offTime = offTime + tempBuffer;
    
    memset(tempBuffer, 0, 8);
    if (m_Times[1].tm_min < 10)
    {
        sprintf (tempBuffer, "0%d", m_Times[1].tm_min);
    }
    else
    {
        sprintf (tempBuffer, "%d", m_Times[1].tm_min);
    }
    offTime = offTime + tempBuffer;
    
}
