/***************************************************************************
 *   Copyright (C) 2008 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it                                           *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 *************************************************************************/
 #ifndef PLAINPID_H
#define PLAINPID_H

#include <string.h>
using namespace std;

/**
Implements the PID algorithm

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/

#define MAX_PID_OUTPUT 1000.0

class PID{
public:
    PID();
    PID(float *params);
    ~PID();

    float UpdatePID(float controlVar);
    void InitPID();
    void SetPIDParams(float* newParams);
    void SetSummer(bool isSummer){m_IsSummer = isSummer;};

    void GetPIDParams(float* dest){ memcpy(dest, m_Parameters, 3*sizeof(float));};

    void SetSetPoint ( float theValue )
    {
        m_SetPoint = theValue;
    }
    

    float GetSetPoint() const
    {
        return m_SetPoint;
    }

    float GetPIDOutput() const
    {
        return m_PIDOutput;
    }
    
    
    
    private:
        float Integrate(float error);
        float Derivate(float error);
        
        float m_SetPoint;
        float m_PIDOutput;
        float m_IntegralState;
        float m_DerivativeState;
        float m_Parameters[3];
        float m_MaxIntegralError;
        bool m_IsSummer;

};

#endif
