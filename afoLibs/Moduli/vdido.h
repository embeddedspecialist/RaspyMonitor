/***************************************************************************
 *   Copyright (C) 2006 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it                                           *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 *************************************************************************/
 

#ifndef STDVDIDO_H
#define STDVDIDO_H

#include "vcontroller.h"
#include "vdevice.h"
#include "ds2408.h"
#include "ds2405.h"

//using namespace std;

/**
This class groups all the informations needed to work with DigitalIO and RemoteDigitalIO

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CVDIDO : public CVController
{
public:
    CVDIDO(const char* configString = NULL, CTimer *timer=0x0);

    ~CVDIDO();
    
    bool SetInputDevice(CVDevice* inDevice){m_Device = inDevice; return true;};
    int GetChannel(){ return m_Channel;};
    
    bool GetStatusRegister(uchar *destBuffer = 0x0, bool updateData = true);
    bool SetStatusRegister(uchar *buffer);
    bool VerifyIOPresence();
    
    uchar* GetDeviceSN(){ return m_Device->GetSN();};
    void GetDeviceSN(uchar *SN){m_Device->GetSN(SN);};
    bool GetActivity(bool updateRegisterFirst);
    
    int GetDeviceConfigIndex(){ return m_Device->GetConfigFileDevIndex(); };
    
    int GetSwitchTime(){ return m_SwitchTime;};
    void SetSwitchTime(int newST){m_SwitchTime = newST;};
    
    bool IsOutputInverted(){ return m_IsOutputInverted;};
    
    virtual bool ChangeOutput() = 0;
   
    

    protected:
        int m_Channel;
        uchar m_StatusRegister[3];
        
        CVDevice *m_Device;         //!handler
        bool m_IsOutputInverted;
        int m_SwitchTime;
        bool m_StartOn;              //!Starting position
        bool m_IsInput; //If the channel is an input or an output

        /**
         * FOR OUTPUT CHANNELS reads the current state of the channel by reading it from the device register
         * @param updateFirst if true the DIDO will read the register before calculating the channel state
         * @return state of the channel or -1 if an error occurred or the channel is an output
         */
        int GetChannelLatch(bool updateFirst);
    
        /**
        * FOR INPUT CHANNELS reads the current state of the channel by reading it from the device register
        * @return state of the input channel (0 low, 1 high) or -1 if an error occurred or the channel is an output
        */
        int GetChannelLevel(bool updateFirst);

};


#endif
