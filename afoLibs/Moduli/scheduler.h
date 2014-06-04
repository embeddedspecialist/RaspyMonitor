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
 ***************************************************************************/
#ifndef STDSCHEDULER_H
#define STDSCHEDULER_H

#include "time.h"
#include "IniFileHandler.h"
#include "commonDefinitions.h"

#define LAST_DAY_IN_WEEK 6
#define LAST_HOUR_IN_DAY 23
#define LAST_MINUTE_IN_HOUR 59

#define SECONDS_IN_DAY 86400
#define SECONDS_IN_HOUR 3600
#define SECONDS_IN_MINUTE 60

//using namespace std;

/**
This class encapsulates basic scheduling functions to program activation and deactivation of events on a weekly basis

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CScheduler{
public:
    CScheduler();

    ~CScheduler();
    
    /**
     * This function loads in the internal variables the correct timings
     * @param onTime Time at which we have an ON condition
     * @param offTime Time at which we have an OFF condition
     * @return TRUE if operation successfull, FALSE if the two times are identical
     */
    bool SetTimes(struct tm onTime, struct tm offTime);
    
    /**
     * Function that checks the internal variables and calculate if the internal state should be On or OFF
     * It updates also the internal storage variable
     * @return TRUE if internal state is ON, FALSE viceversa
     */
    bool CheckStateOn ();
    
    void GetTimes(struct tm* times) { times[0] = m_Times[0]; times[1] = m_Times[1];};
    
    void GetTimes(CString &onTime, CString &offTime);
    
    bool ActivateScheduling();
    
    void DeactivateScheduling();
    
    bool GetActivationState();
    
    private:
        
        time_t m_TimeOn2Off;           //!Time in the week at which we have a transition from On state to OFF state
        time_t m_TimeOff2On;           //!Time in the week at which we have a transition from OFF state to ON state
        time_t m_TimeOfLastTransition; //!Time at which the last transition form ON to OFF, or viceversa, occurred
        
        struct tm m_Times[2];            //!Original times, 0 is for onTime, 1 for offTime
        
        bool m_IsSchedulerActive;         //!Is scheduling ON ?
     
        bool m_IsActualStateOn;             //!Actual internal scheduler state

};

#endif
