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
#ifndef VMULTIDIDO_H
#define VMULTIDIDO_H

#include <vcontroller.h>
#include "vdevice.h"
#include "ds2405.h"
#include "ds2408.h"

#define NOF_INDEVICES 10
#define NOF_OUTDEVICES 10
        
/**
Generic class used in all situations where there is the need to correlate some digital inputs with some digital outputs

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CVMultiDIDO : public CVController
{
public:
    CVMultiDIDO(const char* configString = NULL, CTimer *timer=0x0);

    virtual ~CVMultiDIDO();

    virtual bool Update(bool updateData) = 0; //Sì
    bool VerifyIOPresence(); //Sì

    
    /**
     * Legge dal dispositivo di ingresso e di uscita il registro e lo memorizza internamente
     */
    void UpdateInputsAndOutputs(){GetStatusRegister(true,0x0,0,false); GetStatusRegister(false,0x0,0,false);};
    
    bool SetDevice(bool isOutput, CVDevice* theValue, int devindex = 0); //Sì
    //TODO proteggere dagli errori di indice queste funzioni
    void SetInputDevice(CVDevice* device, int devIndex = 0){ m_InDevices[devIndex] = device;};//Sì
    void SetOutputDevice(CVDevice* device, int devIndex = 0){ m_OutDevices[devIndex] = device;};//Sì
    
    CVDevice* GetDevice(bool isOutput, int devIndex = 0) const;//Sì

    bool GetStatusRegister(bool isOutput, uchar *destBuffer = 0x0, int devIndex = 0, bool updateData = true);
    bool SetStatusRegister(bool isOutput, uchar *buffer, int devIndex = 0);
    
    uchar* GetDeviceSN(bool isOutput, int devIndex = 0);//Sì
    void GetDeviceSN(bool isOutput, uchar *SN, int devIndex = 0);//Sì
    
    bool GetActivity(bool updateRegisterFirst, int devIndex = 0);//Sì
    
    int GetDeviceConfigIndex(bool isOutput, int devIndex = 0);//Forse
    
    int GetSwitchTime(){ return m_SwitchTime;};//da rivedere..
    void SetSwitchTime(int newST){m_SwitchTime = newST;};//da rivedere..
    
    bool IsOutputInverted(int devIndex = 0){ return m_IsInputInverted[devIndex];};

    bool InitInput(int inIndex = 0);

    
    virtual bool InitOutput(int outIndex = 0) = 0;//Sì
    
    virtual bool ChangeOutput(int outIndex = 0) = 0;//Sì
    
    virtual bool SetOutput(bool newState, int outIndex = 0) = 0;//Sì
   
    /**
     * FOR OUTPUT CHANNELS reads the current state of the channel by reading it from the device register
     * @param updateFirst if true the DIDO will read the register before calculating the channel state
     * @return state of the channel or -1 if an error occurred or the channel is an output
     */
    int GetOutputState(bool updateFirst, int devindex = 0);//Sì
    /**
     * FOR INPUT CHANNELS reads the current state of the channel by reading it from the device register
     * @return state of the input channel (0 low, 1 high) or -1 if an error occurred or the channel is an output
     */
    int GetInputState(bool updateFirst, int devIndex = 0);//Sì

    bool IOSameDevices() const;//Sì

    bool IsInputInverted(int devIndex = 0) const//Sì
    {
      return m_IsInputInverted[devIndex];
    }
    
    int GetRemoteAddress();//da rivedere..
    int GetRemoteNET();//da rivedere..
    
    bool IsRemoted(){ return m_IsRemoted;};//da rivedere..
    void SetRemoted (bool set) { m_IsRemoted = set; };//da rivedere..

    int GetNofInputs() const
    {
      return m_NofInputs;
    }//sì

    int GetNofOutputs() const
    {
      return m_NofOutputs;
    }//sì

    //Funzioni usate nel caso di uscita temporizzata per impostare e ripristinare lo stato
    bool PerformOutputChange(int newOutput);
    bool RestoreOutput(int originalOutput);
    
    //TIMEOUT//
    inline float GetTimeOut(bool sec = false) const
    { if(m_OutTimeOut[0]<=0 || !sec) { return (float)m_OutTimeOut[0]; } else { return ( (float)( ((float)m_OutTimeOut[0])/1000.0 ) ); } } 

    int GetRemotePort() const
    {
        return m_RemotePort;
    }
    

protected:
    
    CVDevice *m_InDevices[NOF_INDEVICES];    
    CVDevice *m_OutDevices[NOF_OUTDEVICES];
    
    int m_OutTimeOut[NOF_OUTDEVICES];

    int m_InChannels[NOF_INDEVICES];
    int m_OutChannels[NOF_OUTDEVICES];
    
    uchar m_InStatusRegisters[NOF_INDEVICES][3];
    uchar m_OutStatusRegisters[NOF_OUTDEVICES][3];
    
    int m_InDeviceStates[NOF_INDEVICES];
    int m_OutDeviceStates[NOF_OUTDEVICES];
    
    bool m_IsOutputInverted[NOF_OUTDEVICES];
    bool m_IsInputInverted[NOF_INDEVICES];
    
    int m_SwitchTime;
    bool m_StartOn;              //!Starting position
    
    bool m_SameDevices;
    
    bool m_IsRemoted;
    int m_RemoteAddr;
    int m_RemoteNet;
    int m_RemotePort;
    
    bool m_UseCreativeTimer;//da rivedere..
    int m_TimerDefaultValue;//da rivedere..
    bool m_InvertTimerValue;//da rivedere..
    
    int m_NofInputs;
    int m_NofOutputs;

    bool m_IsOutputTimed;
    int m_OutputTimeOut;
    unsigned int m_OutputChangeStartTime;
    bool m_OutputTimerActivated;
    //Flag che indica che è arrivata una richiesta di cambio uscita ma che non e' ancora stata servita
    bool m_PerformOutputChange;
    bool m_OutputRestored;
};

#endif
