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
 #include "pid.h"

static const float defaultParams[] = {50.0, 10.0, 0.0};

PID::PID()
{
    memcpy (m_Parameters, defaultParams, 3*sizeof(float));
    m_SetPoint = 0.0;

}

PID::PID(float * params)
{
    memcpy (m_Parameters, params, 3*sizeof(float));
    m_SetPoint = 0.0;
}

PID::~PID()
{
}
/////////////////////////////////////////////////////////////////////w
void PID::InitPID()
{
    m_PIDOutput = 0.0;
    m_IntegralState = 0.0;
    m_DerivativeState = 0.0;
    
    m_MaxIntegralError = MAX_PID_OUTPUT/m_Parameters[1];
}
///////////////////////////////////////////////////////////////////
float PID::UpdatePID(float controlVar)
{
    float error = 0.0;
    if (m_IsSummer)
    {
        error = controlVar - m_SetPoint;
    }
    else
    {
        error = m_SetPoint - controlVar;
    }
    
    m_PIDOutput = m_Parameters[0]*error + Integrate(error) + Derivate(error);
  
    if (m_PIDOutput > MAX_PID_OUTPUT){
        m_PIDOutput = MAX_PID_OUTPUT;
    }
    else if (m_PIDOutput < 0.0) {
        m_PIDOutput = 0.0;
    }
     
    return m_PIDOutput;
}
///////////////////////////////////////////////////////////////////////
float PID::Integrate(float error)
{
    float Ki = m_Parameters[1]; 
  
    m_IntegralState+=error;
  
    if (m_IntegralState > m_MaxIntegralError){
        m_IntegralState = m_MaxIntegralError;
    }
    else if (m_IntegralState < 0.0){
        m_IntegralState = 0.0;
    }
  
    return Ki*m_IntegralState;
}
///////////////////////////////////////////////////////////////////////
float PID::Derivate(float error)
{
    float Kd = m_Parameters[2];
  
    m_DerivativeState = error - m_DerivativeState;
    
    return Kd*(m_DerivativeState);
}
///////////////////////////////////////////////////////////////////////
void PID::SetPIDParams(float * newParams)
{
    memcpy (m_Parameters, newParams, 3*sizeof(float));
}



