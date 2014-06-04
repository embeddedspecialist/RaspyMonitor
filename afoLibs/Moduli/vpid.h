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

#ifndef STDPID_H
#define STDPID_H

#include <time.h>
#include "vcontroller.h"
#include "vdevice.h"
#include "timer.h"
#include "LibIniFile.h"
#include "ds2890.h"
#include "commonDefinitions.h"
#include "pid.h"

#define SHUTTER_OPENING_TIME 120

#define CHECK_PTR_BOOL(ptr) if (ptr == 0x0) return false;

#define DIRECT_PID 0
#define LMMIN_PID 2
#define LMMAX_PID 1

/**
Generic interface for the PIDSimple and PIDLMD objects

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CVPID : public CVController
{
public:
    CVPID(const char* configString) : CVController(configString)
    {


        m_normalizeM = 1.0;
        m_normalizeQ = 0.0;

        if (configString != 0x0)
        {
            m_IniLib.GetConfigParamBool( configString, "ISTEMP", &m_IsTemp, true);
        }

        m_PIDOutput = 0;
        //Questo forza l'aggiornamento al prima volta se alla partenza l'uscita e' diversa da 0
        m_LastPosition = -1;
        m_CtrlVariable.value = ANALOG_ERRVAL;
    };


    virtual ~CVPID(){};

    virtual bool InitPID() = 0;

    uchar GetLastOutput() {return m_LastPosition;};
    float GetLastOutputInVolt() {return m_LastPositionVolt;};

    /**
     * Function used to fix a new set point for the PID
     * @param newSP the new set point
     * @return TRUE if the new set point is 5 < newSP < 30, false otherwise
     */
    virtual bool SetSetPoint(float *newSP, int nOfParam = 0) = 0;

    virtual float GetSetPoint() = 0;

    /**
     * Set the summer state
     * @param summerSet the summer state
     * @return true
     */
    virtual bool SetSummer(bool summerSet) = 0;

    inline bool GetSummer() {return m_IsSummer;};

    /**
     * Retrieves all the settings of the PID
     * @param parameters destination array of floats containing K, Ti, Td, setpoint
     * @param isSummer settings for the summer state
     * @return true if the parameters were correctly written in the destinations, false otherwise
     */
    virtual bool GetInfo(float *parameters, bool *isSummer, CString &type) = 0;

    /**
     * Sets the new parameters for the PID
     * @param parameters array containing the parameters: K, Ti and Td
     * @return true if new parameters were correctly set
     */
    virtual bool SetParameters(float *parameters, int nOfParam = 0) = 0;

    bool SetOutputDevice(CVDevice* outDevice) {m_OutputDev = outDevice;return true;};

    float GetControlVariable() const
    {
        return m_CtrlVariable.value;
    }

    CVDevice* GetOutputDevice(){return m_OutputDev;};

    //Output of the PID
    int m_PIDOutput;

    /**
     * Writes the given value directly to the AO controlled by this pid
     * @param outVar value to be written (0-255 where 0 means 10Volts)
     * @return true if operation successfull
     */
    bool WritePIDOutput(int outVar);

    void setIsCtrlVarRemote ( bool theValue )
    {
        m_CtrlVariable.isRemoted = theValue;
    }
    

    bool getIsCtrlVarRemote() const
    {
        return m_CtrlVariable.isRemoted;
    }

    void setCtrlVariableValue ( float theValue )
    {
        m_CtrlVariable.value = theValue;
        m_CtrlVariable.updated = true;
    }

    float GetError(float ctrlVar, float setpoint);



    protected:

        CVDevice *m_OutputDev;

        //!Boolean flag indicating wether we have to invert the error given to the PID to compensate for conditioning
        bool m_IsSummer;

        //Value written in the output device
        int m_LastPosition;
        float m_LastPositionVolt;

        float m_normalizeM, m_normalizeQ;

        //!Flag that indicates wether the input is a temperature sensor or an analogic generic input
        bool m_IsTemp;

        bool m_UseCreativeTimer;
        int m_LastDigital;
        float m_LastSetpoint;

        //Control variable used by the pid
        t_PidVar m_CtrlVariable;

//         void* m_NetPtr;
        PID m_PidVector[3];
        

        

};

#endif
