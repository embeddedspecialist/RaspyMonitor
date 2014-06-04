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
#ifndef STDDIGITALIO_H
#define STDDIGITALIO_H

#include "vdevice.h"
#include "vdido.h"
#include "commonDefinitions.h"
#include "ds2405.h"
#include "ds2408.h"

//using namespace std;

/**
This class is a common wrapper for the ds2408, ds2405, ds24.. digital switch.
Its purpose is to give a common way to interact with the switches by specifying only the memory mapped address
Config Line
 * DeviceXX= NAME:DigitalINOUT,INPUT,IO,STARTV,CHANNEL,ADDR,TIMERID,INVERTOUT,STEP,COMMENT
 * 
	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CDigitalIO : public CVDIDO
{
public:
    CDigitalIO(CVDevice* device, const char* configString = NULL, CTimer *timer=0x0);
    CDigitalIO(const char* configString = NULL, CTimer *timer=0x0);

    ~CDigitalIO();
    
    bool GetState(bool updateFirst = true);
    bool GetLastState() {return m_LastState;};
    bool SetState(bool newState, bool updateFirst=true);

    
    /**
     * Controlla il valore di m_LastState e quello del registro interno al 2408
     * @return TRUE se i due stati coincidono, false altrimenti
     */
    bool IsDataAligned(bool updateData);
    
    bool InitDevice();
    
    bool IsInput(){ return m_IsInput;} 
    
    bool Update(bool updateData);
    bool Update2(bool updateData);
    bool SetVal(float val){return SetState((bool)val,false);};
    
    CString GetSpontaneousData(int lParam = 0) { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};

    bool ChangeOutput();

    void SetStateCheck(bool theValue)
    {
      m_StateCheck = theValue;
    }
    

    bool GetStateCheck() const
    {
      return m_StateCheck;
    }
    
    
    
    private:
        
        bool m_LastState;
        
        //bool m_IsInput;              //!Flag indicating if it is input or output
        
        bool m_StateCheck;

        bool m_IsStep; //Indica il controllo di un rele' passo passo
};

#endif
