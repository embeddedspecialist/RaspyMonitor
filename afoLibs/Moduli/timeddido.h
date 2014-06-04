/***************************************************************************
*   Copyright (C) 2007 by Alessandro Mirri                                *
*   alessandro.mirri@newtohm.it                                           *
*                                                                         *
*   This program is NOT free software; you can NOT redistribute it and/or *
*   modify it in any way without the authorization of the author          *
*                                                                         *
*   This program is distributed WITHOUT ANY WARRANTY;                     *
*   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
*                                                                         *
***************************************************************************/
#ifndef STDTIMEDDIDO_H
#define STDTIMEDDIDO_H

#include "vdido.h"
#include "digitalio.h"


/**
This controller is used to activate a digital output for a predefined amount of time.
It extends the common virtual DIDO class

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CTimedDIDO : public CVDIDO
{
public:
    CTimedDIDO(CVDevice* device, const char* configString = NULL, CTimer *timer=0x0);
    CTimedDIDO(const char* configString = NULL, CTimer *timer=0x0);

    ~CTimedDIDO();

//     void SetTransitionTime(const int& theValue)
//     {
//       m_TransitionTime = theValue;
//     }
//
//
//     int GetTransitionTime() const
//     {
//       return m_TransitionTime;
//     }
//
//     void SetOutDevice(CDigitalIO* theValue)
//     {
//       m_OutDevice = theValue;
//     }
//
//
//     CDigitalIO* GetOutDevice() const
//     {
//       return m_OutDevice;
//     }
//
//     void SetOutChannel(const int& theValue)
//     {
//       m_OutChannel = theValue;
//     }
//
//
//     int GetOutChannel() const
//     {
//       return m_OutChannel;
//     }




private:
    int m_TransitionTime;
    int m_ActivationTime;
    CDS2408 *m_OutDevice;
    int m_OutChannel;
    bool m_IsMoving;

};

#endif
