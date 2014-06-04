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
#include "vpid.h"

///////////////////////////////////////////////////
//              WritePIDOutput
///////////////////////////////////////////////////
bool CVPID::WritePIDOutput(int outVar)
{
    bool retVal = false;

    retVal = ((CDS2890*)(m_OutputDev))->SetPosition(outVar);
    
    if (retVal)
    {
        m_LastPosition = outVar;
        ClearError();
    }
    else
    {
        PushError( AFOERROR_PID_UNABLE_TO_UPDATE, m_NetNumber, m_DeviceNumber);
        AddError();
    }
    
    return retVal;
}
////////////////////////////////////////////////////////
float CVPID::GetError(float ctrlVar, float setpoint)
{
    {
        if (m_IsSummer)
        {
            return ctrlVar - setpoint;
        }
        else
        {
            return setpoint - ctrlVar;
        }
    }
}
