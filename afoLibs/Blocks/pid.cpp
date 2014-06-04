/***************************************************************************
 *   Copyright (C) 2007 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 ***************************************************************************/
 
 #include "pid.h"

CPID::CPID(const char* configString): CBlock(configString)
{
    CString tempString;
    
    //Get PID type and parameters
    m_LibIniReader.GetConfigParamString( configString, "TYPE", &tempString, "PF");
    
    for (int i = 0; i < PID_TYPE_NUM_TOT; i++)
    {
        if (tempString == PID_Type_Strings[i])
        {
            m_PIDType = (e_PIDType)i;
        }
    }
    
    //Read required parameters
    m_Parameters.push_back(m_LibIniReader.GetConfigParamFloat(configString, "KP", 0x0, 8.0));
    m_Parameters.push_back(m_LibIniReader.GetConfigParamFloat(configString, "KI", 0x0, 1.0));
    m_Parameters.push_back(m_LibIniReader.GetConfigParamFloat(configString, "KD", 0x0, 0.0));
    m_SetPoints.push_back(m_LibIniReader.GetConfigParamFloat(configString, "SP", 0x0, 20.0));
    m_IntegralState.push_back(0.0);
    m_DerivativeState.push_back(0.0);

    m_MaxIntegralError.push_back(255.0/m_Parameters.at(1));
    
    if (m_PIDType == PID_TYPE_LMD)
    {
        //read the parameters for the second PID
        m_Parameters.push_back(m_LibIniReader.GetConfigParamFloat(configString, "KP2", 0x0, 1.0));
        m_Parameters.push_back(m_LibIniReader.GetConfigParamFloat(configString, "KI2", 0x0, 0.1));
        m_Parameters.push_back(m_LibIniReader.GetConfigParamFloat(configString, "KD2", 0x0, 0.0));
        m_SetPoints.push_back(m_LibIniReader.GetConfigParamFloat(configString, "SPH", 0x0, 20.0));
        m_SetPoints.push_back(m_LibIniReader.GetConfigParamFloat(configString, "SPL", 0x0, 10.0));
        m_IntegralState.push_back(0.0);
        m_DerivativeState.push_back(0.0);
        m_MaxIntegralError.push_back(255.0/m_Parameters.at(4));
    }
    
    m_LibIniReader.GetConfigParamBool( configString, "SUMMER", &m_IsSummer, 0);
    
    m_LibIniReader.GetConfigParamFloat(configString, "MFACTOR",  &m_normalizeM, 1.0);
    m_LibIniReader.GetConfigParamFloat( configString, "QFACTOR",  &m_normalizeQ, 0.0);

}


CPID::~CPID()
{
}

///////////////////////////////////////////////////
//              Update
///////////////////////////////////////////////////
bool CPID::Update( )
{
    bool retVal = false;
    float input1Val, normInput1Val, error1, input2Val, normInput2Val, error2;
    float pid1_Output, pid2_Output;
    
    try
    {
        //Get the inputs
        if (IsInputValid(PID1_INDEX))
        {
            if (!GetDataFromInput(PID1_INDEX,  &input1Val))
            {
                throw "Impossibile leggere ingresso";
            }
        }
        else
        {
            //TODO da migliorare
            m_OutputVector.at(PID1_INDEX).isValid = false;
            throw "Ingresso non valido";
        }
            
        //Normalize the inputs
        normInput1Val = m_normalizeM * input1Val + m_normalizeQ;
       
        //TODO se è con compensazione esterna va cambiato il SP
        //Calculate error
        if (m_IsSummer)
        {
            error1 = normInput1Val - m_SetPoints.at(0);
        }
        else
        {
            error1 = m_SetPoints.at(0) - normInput1Val;
        }

        //Apply the PID parameters
        pid1_Output = m_Parameters.at(0)*error1 + m_Parameters.at(1)*Integrate( PID1_INDEX, error1 ) + m_Parameters.at(2)*Derivate( PID1_INDEX, error1 );
            
        //Set the analog output: 255 means the output is low, 0 is at Vmax
        if (pid1_Output > 255)
        {
            pid1_Output = 255;
        }
        else if ( pid1_Output < 0 )
        {
            pid1_Output = 0;
        }

        if (m_PIDType == PID_TYPE_PF)
        {

            retVal = SetOutputVal(PID1_INDEX, 255 - pid1_Output);
            
            //TBR
            cout << "Valore PID impostato : " << 255 - pid1_Output << endl;
           
        }
        else if (m_PIDType == PID_TYPE_HYST)
        {
            if (pid1_Output > m_SetPoints.at(PID1_INDEX) + m_Hysteresis)
            {
                //Accendi valvola
            }
            else if (pid1_Output < m_SetPoints.at(PID1_INDEX) - m_Hysteresis)
            {
                //Spegni valvola
            }
        }
        else if (m_PIDType == PID_TYPE_LMD)
        {
            //Get the second input
            if (!GetDataFromInput(PID2_INDEX,  &input2Val))
            {
                throw "Impossibile leggere ingresso";
            }
            
            //Normalize the inputs
            normInput2Val = m_normalizeM * input2Val + m_normalizeQ;
       
            //TODO Per ora implemento solo il controllo di minima sia in estate che in inverno
            //Calculate the error
            if (m_IsSummer)
            {
                error2 = m_SetPoints.at(2) - normInput2Val;
            }
            else
            {
                error2 = normInput2Val - m_SetPoints.at(1);
            }

            //Apply the PID parameters
            pid2_Output = m_Parameters.at(3)*error2 + m_Parameters.at(4)*Integrate( PID2_INDEX, error2 ) + m_Parameters.at(5)*Derivate( PID2_INDEX, error2 );
            
           
            //Set the analog output: 255 means the output is low, 0 is at Vmax
            if (pid2_Output > 255)
            {
                pid2_Output = 255;
            }
            else if ( pid2_Output < 0 )
            {
                pid2_Output = 0;
            }
            
            retVal = SetOutputVal( PID1_INDEX, 255.0 - pid1_Output + pid2_Output);
            
        }
    }
    catch (...)
    {
        retVal = false;
    }
    
    return retVal;
}

///////////////////////////////////////////////////
//              Integrate
///////////////////////////////////////////////////
float CPID::Integrate( int pidIndex, float value )
{
    if ((m_IntegralState[pidIndex] < m_MaxIntegralError[pidIndex]) && (m_IntegralState[pidIndex] >= 0))
    {
        //Until a better algorithm keep on summing the effects
        m_IntegralState[pidIndex] += value;
    }
    else if (m_IntegralState[pidIndex] < 0)
    {
        m_IntegralState[pidIndex] = 0;
    }
    
    return m_IntegralState[pidIndex];
}

///////////////////////////////////////////////////
//              Derivate
///////////////////////////////////////////////////
float CPID::Derivate( int pidIndex, float value )
{
    float retVal = 0.0;

    retVal = value - m_DerivativeState[pidIndex];
    
    m_DerivativeState[pidIndex] = value;
    
    return retVal;
    
}
