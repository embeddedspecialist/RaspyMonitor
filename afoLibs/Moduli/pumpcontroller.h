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
#ifndef STDCPUMPCONTROLLER_H
#define STDCPUMPCONTROLLER_H

#include <time.h>
#include "vcontroller.h"
#include "digitalio.h"
#include "cownet.h"

//using namespace std;

#define PUMPOFF 0
#define PUMPON 1
#define PUMPALARM 2

typedef struct pumpStruct
{
    CDigitalIO *stateDevice;
    CDigitalIO *commandDevice;
    
    //0 - Off, 1 - On, 2 - Alarm
    int status;
} t_PumpStruct;
    
    //TODO da controllare la gestione errori e la blacklist
    //TODO da ripristinare la conversione da giorni a secondi per swap interval
/**
This class uses two digital inputs and two digital outputs to act as a pump control unit
 * The controller has to :
 * - Invert the pumps every given interval
 * - Check the correct functioning of the pumps by reading the state and, in case of error, send a message (???)
 * 
 * The devices are used as follows:
 * input1 is the state of pump number 1, ouput1 is the command line for pump number 1
 * input2 is the state of pump number 2, ouput2 is the command line for pump number 2

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CPumpController : public CVController
{
public:
    CPumpController(const char* configString, CTimer *timer = 0x0);

    ~CPumpController();
    
    bool SetIOs(CDigitalIO *input1, CDigitalIO *input2, CDigitalIO *output1, CDigitalIO *output2);
    bool Update(bool updateData);
    bool Update2(bool updateData){return false;};
    bool SetVal(float val){return false;};
    
    bool VerifyIOPresence();
    
    void GetSettings (int *pump1State, int *pump2State, int *swapTime);
    
    void SetSwapTime (time_t newInterval);
    
    CString GetSpontaneousData(int lParam = 0) { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};
    
    private:
        
        bool EnablePump (int pump, bool enable);
       
        t_PumpStruct m_Pump1, m_Pump2;
        
        time_t m_SwapInterval;
        time_t m_LastSwapTime;
        int m_LastPumpTried;

};


#endif
