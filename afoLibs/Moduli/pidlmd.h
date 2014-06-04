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
#ifndef STDCPIDLMD_H
#define STDCPIDLMD_H

#include "vpid.h"
#include "LibIniFile.h"
#include "vdevice.h"
#include "ds18X20.h"
#include "ds2890.h"
#include "timer.h"
#include "humcontroller.h"

//using namespace std;

/**
This class is used to implement a generic PID with 2 standard closed loops: the first one is used to control the main variable while the second one is used to limit the output of the control plant thus preventing the plant itself to overreact on the controlled system i.e. in an Air Treatment Unit where one doesn't want to input air too cold or too hot in the controlled space.

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CPIDLMD : public CVPID
{
public:
    CPIDLMD(const char* configString, CVDevice *in1Device, CVDevice *in2Device, CVDevice *outDevice, CTimer *timer = 0x0);
    CPIDLMD(const char* configString, CTimer *timer = 0x0);

    ~CPIDLMD();

    bool SetInputOutputDevices(CVDevice* in1Device, CVDevice* in2Device, CVDevice* outDevice);
    bool Update( bool updatedata );
    bool Update2(bool updateData);
    bool SetVal(float val){return false;};
    
    bool VerifyIOPresence();
    
    CString GetSpontaneousData(int lParam = 0) { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};
   
    /**
     * Function used to fix a new set point for the PID
     * @param newSP the new set point
     * @return TRUE if the new set point is 5 < newSP < 30, false otherwise
     */
    bool SetSetPoint(float *newSP, int nOfParam=0);
    bool SetSetPointL( float *newSP );
    bool SetSetPointH( float *newSP );
    
    inline float GetSetPoint() { return m_SetPoint[0];};
    
    /**
     * Set the summer state
     * @param summerSet the summer state
     * @return true
     */
    bool SetSummer(bool summerSet);
    
    /**
     * Retrieves all the settings of the PID
     * @param parameters destination array of floats containing K, Ti, Td, setpoint
     * @param isSummer settings for the summer state
     * @return true if the parameters were correctly written in the destinations, false otherwise
     */
    bool GetInfo(float *parameters, bool *isSummer, CString &type);
    
    /**
     * Sets the new parameters for the PID
     * @param parameters array containing the parameters: K, Ti and Td
     * @return true if new parameters were correctly set
     */
    bool SetParameters(float *parameters, int nOfParam=0);
    
    bool InitPID();


    float GetLMDVariable() const
    {
        return m_LmdVariable.value;
    }

    void setIsLMDRemote ( bool theValue )
    {
        m_LmdVariable.isRemoted = theValue;
    }
    

    bool getIsLMDRemote() const
    {
        return m_LmdVariable.isRemoted;
    }
    
    
    bool ConnectDevices(void* netVoidPtr);

    void setLmdVariableValue ( const float& theValue )
    {
        m_LmdVariable.value = theValue;
        m_LmdVariable.updated = true;
    }
    void UpdateInput(t_PidVar* input);

    bool ExecCommand(CXMLUtil* xmlUtil);
    
    
    private:
        
        CVDevice *m_InputDevices[2];
        
        t_PidVar m_LmdVariable;
        
        bool CheckHandles();
        
        float Integrate(int pidIndex, float Ki, float value);
        float Derivate(int pidIndex, float Kd, float value);
        
        void GetValFromTimer();
        float GetValFromInput(int inputIndex, bool update);
 
        //!Array containing all the parameters in the following list: Proportional, Integrative and Derivative
        float m_Parameters[6];

        //0 = Setpoint diretto, 1 = setpoint di massima, 2 = setpoint di minima
        float m_SetPoint[3];
        
        float m_IntegralState[3];
        float m_MaxIntegralError[2];
        
        float m_DerivativeState[3];

        void NormalizeInputs(float *ctrlVar, float *setPoint, int index);

        float m_controlVar, m_limitVar;

        
};


#endif
