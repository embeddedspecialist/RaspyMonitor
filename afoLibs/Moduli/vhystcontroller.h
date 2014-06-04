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
#ifndef STDVHYSTCONTROLLER_H
#define STDVHYSTCONTROLLER_H

#include "vcontroller.h"
#include "ds2408.h"

//using namespace std;

/**
This class is the common superclass for all the controllers with hysteresis.

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CVHystController : public CVController
{
public:
    CVHystController(const char* configString);

        

    ~CVHystController();
    
    bool SetOutputDevice(CDS2408* device, int channel);
    
//     bool Update(bool updateData);
//     bool VerifyIOPresence();
    
    bool SetAutoControl (bool enableAutoControl) ;
    bool GetAutoControl () {return m_IsAutoControlEnabled;};
    
    bool SetSetPoint(float newSetPoint);
    float GetSetPoint(){return m_SetPoint;};
    
    bool SetHysteresis(float newHysteresis);
    float GetHysteresis() { return m_Hysteresis;};
    

    protected:
        
        CDS2408* m_OutDevice;
        
        int m_Channel;
        float m_Hysteresis;
        bool m_IsAutoControlEnabled;
        float m_SetPoint;
        
};


#endif
