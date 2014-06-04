/* 
 * File:   ClockManager.cpp
 * Author: amirrix
 * 
 * Created on 28 gennaio 2010, 9.30
 */

#include <sys/time.h>
#include <vector>

#include "ClockManager.h"

ClockManager::ClockManager() {

    gettimeofday(&m_LastClockVal,0x0);
}

ClockManager::~ClockManager() {
}

/**
     * Aggiunge un clock e ne ritorna l'indice
    */
unsigned int ClockManager::GetClock(){

    struct timeval actTime;

    gettimeofday(&actTime, 0x0);

    m_ClocksVector.push_back(actTime);

    return m_ClocksVector.size()-1;
}

int ClockManager::GetTimeElapsed(int clockIndex, bool reset){

    int retVal = -1;
    struct timeval actTime;

    gettimeofday(&actTime, 0x0);

    //Qui controllo se c'e' qualcosa di strano in data e ora
    if ( (actTime.tv_sec < m_LastClockVal.tv_sec) ||
         (actTime.tv_sec - m_LastClockVal.tv_sec > MAX_CLOCK_SKEW) )
    {
        ResetAllClocks();
    }
    else {
        gettimeofday(&m_LastClockVal, 0x0);
    }

    try {
        retVal = actTime.tv_sec - m_ClocksVector.at(clockIndex).tv_sec;

        if (reset){
            gettimeofday(&(m_ClocksVector.at(clockIndex)), 0x0);
        }
    }
    catch (...){
        
    }

    return retVal;
}

bool ClockManager::ResetClock(int clockIndex){
    try {
        gettimeofday(&(m_ClocksVector.at(clockIndex)), 0x0);
        return true;
    }
    catch (...){
        return false;
    }
}

bool ClockManager::ResetAllClocks(){

    try {
        for (int clockIndex =0 ; clockIndex < m_ClocksVector.size(); clockIndex++){
            gettimeofday(&(m_ClocksVector.at(clockIndex)), 0x0);
        }

        return true;
    }
    catch (...){
        return false;
    }

}

