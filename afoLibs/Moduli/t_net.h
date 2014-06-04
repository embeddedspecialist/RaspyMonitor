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
#ifndef STDT_NET_H
#define STDT_NET_H

#include <vector>

#include "cownet.h"
#include "afoerror.h"
#include "vdevice.h"
#include "vcontroller.h"

//using namespace std;

/**
This class is a replacement for the old typedef T_Net. It holds all the specific values of the NET

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class T_Net{
public:
    T_Net(){ CtrlList.reserve(MAX_NUM_DEV_PER_NET/2); deviceList.reserve(MAX_NUM_DEV_PER_NET/2); blackList.reserve(MAX_NUM_DEV_PER_NET/2);};

    ~T_Net(){};

    COWNET *master;                                                                //!The master of the subnet
    vector<CVController*> CtrlList;
    CString netPortName;                                                            //!name of th eport of the NET
    //TBR
    CAfoErrorHandler errorHandler;                                                           //!Error handling object
    SMALLINT portHandler;                                                     //!The Port to which the master is linked
    bool isWl;                                                                          //!Is the net Wireless ?
    int wlAddr[2];                                                                     //!Wireless address
    CString ipAddress;
    int ipPort;
    bool isOverIp;
    int netDelay;
    bool isAcquired;                                                                    //!Acquired flag
    bool correctlySet;                                                               //!Flag indicating correct setup of the net
    bool areAlarmsEnabled;                                                      //!Flag indicating if the alarms are enabled
    bool areTempAlarmsSW;                                                    //!Flag indicating if the sensor manage by themselves the alarm conditions
    int defAlMin;                                                                      //!Default Minimum Alarm level for Temp sensors
    int defAlMax;                                                                     //!Default Maximum Alarm level for Temp sensors
    vector <CVDevice*> deviceList;                  //!All devices in the net
    int timerID;
    CTimer *netTimer;                                                               //!The one and only timer object
    bool isTimerOn;
    bool hasTempDevices;
    bool hasDIDOs;
    bool hasPIDs;
    bool hasHums;
    bool hasAIAOs;
    bool hasButtonDIDOs;
    bool hasAdvCtrls;
    bool hasHotelRooms;
    bool hasAccessControl;
    bool hasIButtonReader;
    bool hasCoordinators;
    bool isVirtual;

    bool tempConversionLaunched;
    
    unsigned int temperatureClock;
    unsigned int didoClock;

    unsigned long timeOfLastTempConversion;
    unsigned long int m_LastDOPollTime;

    //Flag per i salvataggi
    bool saveDigitalState;

    int nOfErrors;
    vector<CVController*> blackList;

    bool recheckMaster;

    inline void ResetTimers() { tempConversionLaunched = false; timeOfLastTempConversion = 0; m_LastDOPollTime = 0;};
};



#endif
