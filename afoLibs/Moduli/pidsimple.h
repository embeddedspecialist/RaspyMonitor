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
#ifndef STDPIDSIMPLE_H
#define STDPIDSIMPLE_H

#include "vpid.h"
#include "LibIniFile.h"
#include "vdevice.h"
#include "ds18X20.h"
#include "ds2890.h"
#include "ds2438.h"
#include "timer.h"
#include "vcontroller.h"

#define MAX_TEMP_DIFF 10

//using namespace std;

/**
This class is used to implement a simple PID controller having just 1 temperature input and one analog output. Parameters of the controller are read from the .ini file
ConfigString:
 * NAME:PIDSimple, ADDR, INPUT, OUTPUT, SETPOINT, KP, Tint, Tder, SUMMER, TIMERID, ISTEMP, COMMENT
	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
    
    Rev. 1.3 - 17/03/2006
 * Aggiunte nuovi algoritmi per integrazione e derivazione
 * Corretti alcuni bug minori
 * 
    Rev. 1.2 - 11-03-2206
 * Aggiunte funzioni di intefaccia per gestire i parametri e poterli leggere dall'esterno
 * Aggiunto il nome del dispositivo come parametro
 * Aggiunto il parametro m_IsSummer che consente di invertire il funzionamento tra estate/inverno
*/
class CPIDSimple : public CVPID
{
public:
    CPIDSimple(const char* configString, CVDevice *inDevice, CVDevice *outDevice, CTimer *timer = 0x0);
    CPIDSimple(const char* configString, CTimer *timer = 0x0);

    ~CPIDSimple();
    
    bool SetInputOutputDevices(CVDevice* inDevice, CVDevice* outDevice);

    /**
     * Called from the engine to update the pid status and perform control on the output device
     * @param updateData if true the controller sends an update command before reading the input device
     * @return true if everything went well
     */
    bool Update( bool updateData );
    
    bool Update2(bool updateData);
    bool SetVal(float val){return false;};

    
    /**
     * Called from a coordinator. It has the same algorithm except that it does not take in account the timers and
     * does not write on the output device. The PID state is saved onto the m_PIDOutput variable
     * @param controlVar the control variable
     * @return true
     */
    bool Update (float controlVar);
    
    bool VerifyIOPresence();

    CString GetSpontaneousData(int lParam = 0) { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};
    
    bool ConnectDevices(void* netVoidPtr);
            
    /**
     * Function used to fix a new set point for the PID
     * @param *newSP the new set point
     * @return TRUE if the new set point is 5 < newSP < 30, false otherwise
     */
    bool SetSetPoint(float *newSP, int nOfParam = 0);
    
    inline float GetSetPoint() { return m_SetPoint;};
    
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
    bool SetParameters(float *parameters, int nOfParam = 0);
    
    bool InitPID();

    //!Flag that indicates wether the internal PID functions allow to have a negative value
    //!it is used only when the PID is used just as an integration routine
    bool m_AllowsNegativeCalc;

    bool ExecCommand(CXMLUtil* xmlUtil);
    
    private:
        
        CVDevice *m_InputDev;
        
        
        bool CheckHandles();
        
        float Integrate(float Ki, float value);
        float Derivate(float Kd, float value);
        
        float GetValFromInput();
        
        //!Array containing all the parameters in the following list: Proportional, Integrative and Derivative
        float m_Parameters[3];
        
        float m_SetPoint;
        
        float m_IntegralState;
        float m_MaxIntegralError;
        
        float m_DerivativeState;
        float m_controlVar;

        void NormalizeInputs(float *ctrlVar, float *setPoint);

};



#endif
