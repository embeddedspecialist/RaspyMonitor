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
#ifndef TIMER_H
#define TIMER_H

#include "IniFileHandler.h"
#include "LibIniFile.h"
#include "commonDefinitions.h"
#include "time.h"



//using namespace std;
/**
This class implements the timers for the AFO systems. A timer is a special object that allows to retrieve specific settings for the various devices based on the week day and the current hour. 
The timer class can manage 4 kinds of values: 
-Digital for digital outputs settings
-Analog for Analog outputs settings
-PID for controllers
-Alarms for the various alarms
For each of the previous values it is possible to define three levels
The timer class reads its informations from a special ini file called "Timers.ini"

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CTimer{
public:
    CTimer();

    ~CTimer();
    
    //GetMethods
    /**
     * Function used to retrieve the value of the level for the current time
     * @param timerIndex index of the timer for which we would like to get the value of the level. Index must reflect the one in the timers.ini file
     * @param typeOfValue type of value we would like to retrieve (PID, Alarm, Digital...)
     * @return the value associated at the current time or INT_MIN if the timer is disabled
     */
    int GetValue( int timerIndex, e_TimerValues typeOfValue);
    
    //Set Method   
    bool SetValue (int timerIndex, e_TimerValues typeOfValue, int level, int newValue);
    
     bool LoadTimers();
     
     bool UpdateTimerSettings(int timerIndex, CString wDay, CString newSettings);
     
     bool UpdateTimerLevels(int timerIndex, int level, CString newSettings);
     
     int GetNofTimers() { return m_TotalNumberOfTimers;};
     
     bool EnableTimer(int timerIndex, bool timerEnable);
     
     /**
      * This function is used to retrieve the state of the timer, in other words if it is enabled or disabled
      * @param timerIndex index of the timer for which we would like to know the state. The index must reflect the number in the timers.ini file
      * @return true if timer enabled, false if timer disabled or an error occurred
      */
     bool IsTimerEnabled(int timerIndex);
private:

    bool ParseTimerDay(int timerIndex, int day, CString dayString);
    
    bool GetTZIndex(int timerIndex, int *dayIndex, int *tzIndex);

    int m_TotalNumberOfTimers;
    t_TimerMx m_TimerList[MAX_NUMBER_OF_TIMERS];
    CIniFileHandler *m_TimersFile;
    CLibIniFile m_IniLib;

};

#endif
