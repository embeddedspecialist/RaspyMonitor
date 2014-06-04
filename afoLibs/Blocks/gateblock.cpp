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

#include "gateblock.h"

CGateBlock::CGateBlock(const char* configString)
: CBlock(configString)
{
    CString tempString;
    
    //Get PID type and parameters
    m_LibIniReader.GetConfigParamString( configString, "TYPE", &tempString, "JK");
    m_LibIniReader.GetConfigParamInt(configString,"STARTV",&m_StartValue,0);
    
    m_OutputState = m_StartValue;
    
    for (int i = 0; i < GATE_TYPE_NUM_TOT; i++)
    {
        if (tempString == GATE_Type_Strings[i])
        {
            m_GateType = (e_GATEType)i;
        }
    }
}

CGateBlock::~CGateBlock()
{
    
}

bool CGateBlock::Update()
{
    bool retVal = false;
    bool isInputValid = false, isInput1Valid = false, isInput2valid = false;
    float input1 = 0, input2 = 0;
    
    //Check if inputs are valid
    isInput1Valid = IsInputValid(0);
    isInput2valid = IsInputValid( 1 );
    isInputValid = isInput1Valid && isInput2valid;
    GetDataFromInput( 0, &input1);
    GetDataFromInput( 1, &input2);
    
    if (!isInputValid)
    {
        return false;
    }
    
    switch (m_GateType)
    {
        case GATE_TYPE_JK:
        {
            //Implemento la tabella della verita' del flip flop JK
            if ((input1==0.0) && (input2==0.0))
            {
                //J = 0, K = 0: Hold, non aggiorno lo stato interno
                SetOutputVal(0, (float)m_OutputState, 1);
                SetOutputVal(1, (float)(!m_OutputState), 1);
            }
            else if ((input1==1.0) && (input2==0.0))
            {
                //J = 1, K = 0: Set
                SetOutputVal(0, 1.0, 1);
                SetOutputVal(1, 0.0, 1);
                m_OutputState = 1;
            }
            else if ((input1==0.0) && (input2==1.0))
            {
                //J = 0, K = 1: Reset
                SetOutputVal(0, 0.0, 1);
                SetOutputVal(1, 1.0, 1);
                m_OutputState = 0;
            }
            else if ((input1==1.0) && (input2==1.0))
            {
                //J = 1, K = 1: Toggle
                SetOutputVal(0, (float)(!m_OutputState), 1);
                SetOutputVal(1, (float)m_OutputState, 1);
                m_OutputState = (!m_OutputState);
            }
            
        };break;
    }
    
    return retVal;
}
