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
#include "timer.h"

CTimer::CTimer()
{
    m_TimersFile = new CIniFileHandler();

    m_TotalNumberOfTimers = 0;

    memset (m_TimerList, 0, MAX_NUMBER_OF_TIMERS*sizeof(t_TimerMx));

}


CTimer::~CTimer()
{
    if (m_TimersFile != 0x0)
    {
        delete m_TimersFile;
    }
}

///////////////////////////////////////////////////
//              LoadTimers
///////////////////////////////////////////////////
bool CTimer::LoadTimers( )
{
    int timerIndex = 0;
    int dayIndex = 0;
    int levelIndex = 0;
    CString levelString;
    bool retVal = false;
    int timerValIndex = 0;

    //Start by reading the ini file
    if (m_TimersFile->Load( "./timers.ini"))
    {
        //Get the number of timers
        m_TotalNumberOfTimers = m_TimersFile->GetInt( "NumberOfTimers", "COMMON",0);
        retVal = true;

        //Start by reading all the timers
        for (timerIndex = 0; timerIndex < m_TotalNumberOfTimers; timerIndex++)
        {
            char timerSection[]="Timer0";

            sprintf (timerSection+5, "%1d", timerIndex+1);

            //Start Loading the levels
            for (levelIndex = 1; levelIndex < 4; levelIndex++)
            {
                char levelKey[]="Level0";
                sprintf (levelKey+5, "%1d", levelIndex);

                levelString = m_TimersFile->GetString( levelKey, timerSection );

                //Get all the levels
                for (timerValIndex = 0; timerValIndex < TIMERVAL_NUMTOT; timerValIndex++)
                {
                    m_IniLib.GetConfigParamInt( levelString.c_str(),
                                                timerValsString[timerValIndex],
                                                &m_TimerList[timerIndex].levelsMatrix[levelIndex-1][timerValIndex],
                                                0);
                }
            }

            //Go on by filling the matrix with the timevalues
            for (dayIndex = 0; dayIndex < 7; dayIndex++)
            {
                CString dayString;

                //Get The string relative to the day
                dayString = m_TimersFile->GetString( daysStrings[dayIndex], timerSection);

                //Parse the string to fill the matrix
                retVal = ParseTimerDay(timerIndex, dayIndex, dayString );
            }

            //Finish up by enabling the timer
            m_TimerList[timerIndex].isTimerEnabled = true;

        }//For timerIndex

    }//IF m_Timers

    return retVal;
}

///////////////////////////////////////////////////
//              ParseTimerDay
///////////////////////////////////////////////////
bool CTimer::ParseTimerDay( int timerIndex, int day,  CString dayString)
{
    int totalNumberOfTZ = 0;
    string::size_type colonIdx = 0;
    int index = 0;
    bool retVal = false;
    CString tzString, levelString;

    //Firstly search for the separators
    for (totalNumberOfTZ = 0; totalNumberOfTZ < MAX_NUMBER_OF_TZ; totalNumberOfTZ++)
    {
        colonIdx = dayString.find (":", colonIdx+1);
        if (colonIdx == string::npos)
        {
            //No more semicolon chars found, exit
            break;
        }
    }

    //Check if we have at least one timezone, otherwise exit with an error
    if (totalNumberOfTZ == 0)
    {
        return retVal;
    }

    //Save the number of timezones
    m_TimerList[timerIndex].timerDay[day].nOfValues = totalNumberOfTZ;

    //Start getting all the infos
    colonIdx = 0;
    for (index = 0; index < totalNumberOfTZ; index++)
    {
        char buffer[255];

        memset (buffer, 0, 255);
        colonIdx = dayString.find (":", colonIdx+1);
        tzString = dayString.substr(colonIdx-4,4);
        levelString = dayString.substr(colonIdx+1,4);

        strcpy(buffer, tzString.c_str());
        //Extract the values
        sscanf(buffer, "%2d", &m_TimerList[timerIndex].timerDay[day].timeZones[index].tz_Hour);
        sscanf(buffer+2, "%2d", &m_TimerList[timerIndex].timerDay[day].timeZones[index].tz_Min);

        memset(buffer, 0, 255);
        strcpy(buffer,levelString.c_str());
        sscanf(buffer+3, "%2d", &m_TimerList[timerIndex].timerDay[day].timeZones[index].tz_Level);

    }

    retVal = true;

    return retVal;
}



///////////////////////////////////////////////////
//              GetTZIndex
///////////////////////////////////////////////////
bool CTimer::GetTZIndex( int timerIndex, int *dayIndex, int *tzIndex)
{
    time_t actTime = 0;
    struct tm *timeOfWeek = NULL;   //Time of the week
    int nOfTZ = 0;
    bool retVal = false;
    int index = 0;

    //First get actual date
    actTime = time(NULL);
    timeOfWeek = localtime(&actTime);

    //Use the data obtained as indexes
    *dayIndex = timeOfWeek->tm_wday;

    //Get the total number of timezones for the current day
    nOfTZ = m_TimerList[timerIndex].timerDay[*dayIndex].nOfValues;

    //Now get the correct value by comparing current time with next TZ
    for (index = 0; index < nOfTZ; index++)
    {
        if (    (timeOfWeek->tm_hour < m_TimerList[timerIndex].timerDay[*dayIndex].timeZones[index].tz_Hour) ||
                 (
                 (timeOfWeek->tm_hour == m_TimerList[timerIndex].timerDay[*dayIndex].timeZones[index].tz_Hour) &&
                 (timeOfWeek->tm_min < m_TimerList[timerIndex].timerDay[*dayIndex].timeZones[index].tz_Min)
                 )
           )
        {
            break;
        }
    }

    if (index != nOfTZ)
    {
        *tzIndex = index-1;
        retVal = true;
    }

    return retVal;
}



///////////////////////////////////////////////////
//              UpdateTimerSettings
///////////////////////////////////////////////////
bool CTimer::UpdateTimerSettings( int timerIndex, CString wDay, CString newSettings )
{
    CString section;
    char buffer[32];

    section = "Timer";
    sprintf (buffer, "%d", timerIndex);
    section += buffer;

    //Decrease timerIndex to match the internal array
    timerIndex -= 1;

    if ( (timerIndex < 0) || (timerIndex >= m_TotalNumberOfTimers ) )
    {
        return false;
    }

    m_TimersFile->SetValue( wDay, newSettings, "", section);
    m_TimersFile->Save();

    m_TimersFile->Clear();

    LoadTimers();

    return true;
}

///////////////////////////////////////////////////
//              UpdateTimerLevels
///////////////////////////////////////////////////
bool CTimer::UpdateTimerLevels( int timerIndex, int level, CString newSettings )
{
    char sectionStr[16];
    char levelStr[8];

    memset ( sectionStr, 0x0, 16 );
    memset (levelStr, 0x0, 8);
    sprintf (sectionStr, "Timer%1d", timerIndex);

    //Decrease timerIndex to match the internal array
    timerIndex -= 1;

    if ( (timerIndex < 0) || (timerIndex >= m_TotalNumberOfTimers ) )
    {
        return false;
    }

    sprintf (levelStr, "Level%1d", level);

    if ( m_TimersFile->SetValue( levelStr, newSettings, "", sectionStr) )
    {
        //save the file
        m_TimersFile->Save();
        m_TimersFile->Clear();
        //Reload the timers
        LoadTimers();
        return true;
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////
//              GetValue
///////////////////////////////////////////////////
int CTimer::GetValue( int timerIndex, e_TimerValues typeOfValue )
{
    int tzIndex = -1;
    int dayIndex = 0;
    int levelIndex = 0;
    int retVal = -1;

    //Decrease timerIndex to match the internal array
    timerIndex -= 1;

    if ( GetTZIndex( timerIndex, &dayIndex, &tzIndex) )
    {
        levelIndex = m_TimerList[timerIndex].timerDay[dayIndex].timeZones[tzIndex].tz_Level;

        retVal = m_TimerList[timerIndex].levelsMatrix[levelIndex-1][typeOfValue];
    }

    return retVal;
}

///////////////////////////////////////////////////
//              SetValue
///////////////////////////////////////////////////
bool CTimer::SetValue( int timerIndex, e_TimerValues typeOfValue, int level, int newValue )
{
    return true;
}

///////////////////////////////////////////////////
//              EnableTimer
///////////////////////////////////////////////////
bool CTimer::EnableTimer( int timerIndex, bool timerEnable )
{
    bool retVal = false;

    //Decrease timerIndex to match the internal array
    timerIndex -= 1;

    if (timerIndex < m_TotalNumberOfTimers)
    {
        m_TimerList[timerIndex].isTimerEnabled = timerEnable;
        retVal = true;
    }

    return retVal;
}

///////////////////////////////////////////////////
//              IsTimerEnabled
///////////////////////////////////////////////////
bool CTimer::IsTimerEnabled( int timerIndex )
{
    bool retVal = false;

    //Decrease timerIndex to match the internal array
    timerIndex -= 1;

    if (timerIndex < m_TotalNumberOfTimers)
    {
        retVal = m_TimerList[timerIndex].isTimerEnabled;
    }

    return retVal;
}




