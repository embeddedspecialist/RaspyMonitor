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
#ifndef UTACTRL_H
#define UTACTRL_H

        
#include <time.h>
#include "pidlmd.h"
#include "digitalio.h"
#include "ds2890.h"
#include "temperaturecontroller.h"
#include "vcontroller.h"

/**
This class is used to control an Air Treatment Unit. It has one single input used for the antifreeze sensor and three outputs: one for input air fans, one for the extraction fans and one for opening the shutters and one for closing the shutters
Config Line
 * NAME:UTAController, ADDR,TEMP1,TEMP2, DIN1, DIN1CH,DOUT1,DOUT1CH, DOUT2, DOUT2CH,DOUT3,DOUT3CH,DOUT4,DOUT4CH, AN1OUT, AN2OUT, INVERT,SP,SPH,SPL,KP1,TI1,TD1,KP2,TI2,TD2,SHDELAY,TIMERID,STARTV,COMMENT
	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
#define ANALOG_CLOSE 255
#define ANALOG_OPEN 0

//Inputs
#define TAG_IDX 0
#define TEMPAMB_IDX 0
#define TEMPLMD_IDX 1
        
//Outputs
#define SHUTTEROPEN_IDX 0
#define SHUTTERCLOSE_IDX 1
#define FANIN_IDX 2         //Mandata
#define FANOUT_IDX 3        //Ripresa
        
#define HEATBATTERY 0
#define COLDBATTERY 1
        
        
class CUtaCtrl : public CVController
{
public:
    CUtaCtrl(const char* configString, CTimer *timer);

    ~CUtaCtrl();

    //MultiDIDO
    bool ChangeOutput(int outIndex = 0);
    bool InitOutput(int outIndex = 0);
    bool SetOutput(bool newState, int outIndex = 0);
    bool Update(bool updateData);
    bool Update2(bool updateData){return false;};
    bool SetVal(float val){return false;};
    
    CString GetSpontaneousData(int lParam = 0) { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};
    //PID params
    bool SetPIDParams(const char* configString);
    
    bool FinalizeSetup();
    
    bool Init();
    
    CDigitalIO* m_Tag;
    CTempCtrl* m_TempControllers[2];
    CDS2890* m_Analogs[2];
    CDigitalIO* m_OutDigitals[4];
   
    CPIDLMD *m_PIDLMD;    
    
    //TODO da implementare
    bool VerifyIOPresence(){return true;};
    
    bool SetSummer(bool isSummer);
    
    //PIDLMD INTERFACE
        /**
     * Function used to fix a new set point for the PID
     * @param newSP the new set point
     * @return TRUE if the new set point is 5 < newSP < 30, false otherwise
         */
    bool SetPIDSetPoint(float *newSP, int nOfParam){ return m_PIDLMD->SetSetPoint(newSP, nOfParam);};

    
    /**
     * Retrieves all the settings of the PID
     * @param parameters destination array of floats containing K, Ti, Td, setpoint
     * @param isSummer settings for the summer state
     * @return true if the parameters were correctly written in the destinations, false otherwise
     */
    bool GetPIDInfo(float *parameters, bool *isSummer, CString &type){return m_PIDLMD->GetInfo(parameters, isSummer, type);};
    
    /**
     * Sets the new parameters for the PID
     * @param parameters array containing the parameters: K, Ti and Td
     * @return true if new parameters were correctly set
     */
    bool SetPIDParameters(float *parameters, int nOfParam){ return m_PIDLMD->SetParameters(parameters, nOfParam);};
    
    void SetReferenceNumbers (int devNumber, int netNumber);

    bool HasShutterCommand() const
    {
      return m_HasShutterCommand;
    }

    bool HasOneShutterCommand() const
    {
      return m_HasOneShutterCommand;
    }

    bool HasOneFanCommand() const
    {
      return m_HasOneFanCommand;
    }
    
    
    

protected:
    
    int m_ShutterDelay;
    bool m_ShutterOpen;
    bool m_ShutterOpening;
    bool m_ShutterClosing;
    bool m_HasOneShutterCommand;
    bool m_HasShutterCommand;
    
    bool m_HasOneFanCommand;
    
    bool m_HasOneAnalogOut;
    
    time_t m_TimeOfShutterMoving;
        
    bool CreateDigital(bool isInput, int index, int input, int channel, const char* configString);
    bool CreatePID(const char* configString);
    
    bool m_IsSummer;
    bool m_StartOn; 
    
    int m_InvertDigital;
    
    bool SetDOUT(int index, bool val);
    bool SetAOUT(int index, int val);
    bool SetAllAOUT(int val);
    
    bool OpenShutters();
    bool CloseShutters();
    bool StopShutters();
    
    
};

#endif
