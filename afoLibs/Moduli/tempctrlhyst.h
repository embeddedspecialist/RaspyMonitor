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
 ************************************************************************/
#ifndef STDTEMPCTRLHYST_H
#define STDTEMPCTRLHYST_H

#include "vhystcontroller.h"
#include "vdevice.h"
#include "ds18X20.h"
#include "ds2438.h"

//using namespace std;

/**
This class is used to perform a simple temperature control with hysteresis

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CTempCtrlHyst : public CVHystController
{
public:
    CTempCtrlHyst(CVDevice* inDevice, const char* configString = NULL, CTimer *timer=0x0);
    CTempCtrlHyst(const char* configString = NULL, CTimer *timer=0x0);

    ~CTempCtrlHyst();
    
    float ReadTemperature(bool update);
    bool ReadTemperature(bool updateFirst, float *newTemp );

    
    float GetLastTemp(){ return m_LastTemp;};
    
    bool Update(bool updateData);
    bool Update2(bool updateData){return false;};
    bool SetVal(float val){return false;};
    
    bool VerifyIOPresence();
    CString GetSpontaneousData(int lParam = 0) { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};
    
    bool UpdateTemp();
    
    bool SetInputDevice(CVDevice* inDevice){m_InDevice = inDevice;return true;};

    private:
        //Input device, could be either DS18X20 either DS2438
        CVDevice* m_InDevice;
        float m_LastTemp;
        bool m_IsSummer;
};


#endif
