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
#ifndef STDHUMSENSOR_H
#define STDHUMSENSOR_H

#include "vcontroller.h"
#include "ds2438.h"
#include "digitalio.h"
#include "vdevice.h"
#include "vhystcontroller.h"

//using namespace std;

/**
This class acts as a wrapper to the DS2438 sensor to measure the air humidity

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
    
    Rev. 1.1 - 22/06/2006
 * Spostata la lettura di temperatura nella funzione GetHumidity perche'sembra che non legga VDD se non legge prima la temperatura... aggiunto un FIXME
*/
class CHumController : public CVHystController
{
public:
    CHumController(CDS2438* inDevice, const char* configString = NULL, CTimer *timer=0x0);
    CHumController(const char* configString = NULL, CTimer *timer=0x0);

    ~ CHumController();
    
    float GetHumidity();
    bool GetHumidity(float* absHum, float *relHum = 0x0, float *temp = 0x0);
    float ReadTemperature(bool update);
    
    float GetLastAbsHumidity() {return m_LastAbsHumidity;};
    float GetLastRelHumidity() {return m_LastRelHumidity;};
    float GetLastTemp(){ return m_LastTemp;};

    
    bool Update(bool updateData);
    
    bool Update2(bool updateData){return false;};
    bool SetVal(float val){return false;};
    
    bool VerifyIOPresence();
    CString GetSpontaneousData(int lParam = 0) { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};
    
    bool SetInputDevice(CDS2438* inDevice) {m_InDevice = inDevice;return true;};

    private:
        CDS2438* m_InDevice;
        
        float m_LastRelHumidity, m_LastAbsHumidity;
        float m_LastTemp;
};


#endif
