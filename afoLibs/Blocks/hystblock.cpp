/***************************************************************************
 *   Copyright (C) 2007 by root   *
 *   root@linux-xr01   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "hystblock.h"
#include "conewireengine.h"


CHYSTBlock::CHYSTBlock(const char* configString)
    : CBlock(configString)
{
    CString tempString;
    float setpoint=0.0, delta=0.0, Min=0.0, Max=0.0;
    //Get HYST type and parameters
    m_LibIniReader.GetConfigParamString( configString, "TYPE", &tempString, "HYST");
    
    for (int i = 0; i < HYST_TYPE_NUM_TOT; i++)
    {
        if (tempString == HYST_Type_Strings[i])
        {
            m_HYSTType = (e_HYSTType)i;
	    m_LibIniReader.GetConfigParamFloat( configString, "SETPOINT", &setpoint, 0.0);
            m_LibIniReader.GetConfigParamFloat( configString, "DELTA", &delta, 0.0);
	    m_LibIniReader.GetConfigParamFloat( configString, "MIN", &Min, 0.0);
            m_LibIniReader.GetConfigParamFloat( configString, "MAX", &Max, 0.0);
	    setSetpoint(&setpoint);
            setDelta(&delta);
	    setLastOutput(&Min);
		
	    setMin(&Min);
	    setMax(&Max);
	}
    }
}

CHYSTBlock::~CHYSTBlock()
{
}

///////////////////////////////////////////////////
//              Update
///////////////////////////////////////////////////
bool CHYSTBlock::Update( )
{
    float input1 = 0.0;
    bool retVal = false, isInputValid = false;
    
    //Check if inputs are valid
    //isInputValid = IsInputValid(0) && IsInputValid( 1 );
    isInputValid = IsInputValid(0);
    GetDataFromInput( 0, &input1);

    if ((IsInputConnected(1)) && (IsInputValid(1)) ) {
        GetDataFromInput(1,&m_Setpoint);
    }
    
    switch (m_HYSTType)
    {
        case    HYST_TYPE_HYST:
        {
		if ((input1<=(m_Setpoint+m_Delta)) && (input1>=(m_Setpoint-m_Delta)))
		{
			retVal = SetOutputVal( 0, m_LastOutput, isInputValid);
		}
		else if (input1>(m_Setpoint+m_Delta))
		{
			setLastOutput(&m_Max);
			retVal = SetOutputVal( 0, m_LastOutput, isInputValid);
		}
		else if (input1<(m_Setpoint-m_Delta))
		{
			setLastOutput(&m_Min);
			retVal = SetOutputVal( 0, m_LastOutput, isInputValid);
		}
	};break;
        default: retVal = false;
    }

    return retVal;
    
}

bool CHYSTBlock::setSetpoint(float* Threshold)
{
	m_Setpoint=(*Threshold);
        return true;
}

bool CHYSTBlock::setDelta(float* dT)
{
	m_Delta=(*dT);
        return true;
}

bool CHYSTBlock::setLastOutput(float* Start)
{
	m_LastOutput=(*Start);
        return true;
}

bool CHYSTBlock::setMin(float* Min)
{
	m_Min=(*Min);
        return true;
}

bool CHYSTBlock::setMax(float* Max)
{
	m_Max=(*Max);
        return true;
}

////////////////////////////////////////////////////////////////////////////////
bool CHYSTBlock::ExecCommand(CXMLUtil *xmlUtil, bool *commandRetCode){
    COneWireEngine *eng = reinterpret_cast<COneWireEngine*>(m_EngPtr);

    if (!IsCommandForMe(xmlUtil)){
        return false;
    }

     *commandRetCode = false;
    //Ora interpreto il messaggio
     CString command = xmlUtil->GetStringParam("BLOCKEXEC");
    if (command.ToUpper() == "GETPARAMETERS"){
        Cmd com("BLOCK");
        com.putValue("ADDRESS",m_BlockAddress);
        com.putValue("BLOCKEXEC","HystParameters");

        com.putValue("SETPOINT",m_Setpoint);
        com.putValue("DELTA",m_Delta);
        com.putValue("MIN",m_Min);
        com.putValue("MAX",m_Max);

        CString message = com.getXMLValue();
        eng->WriteOnInterfacePorts(message.c_str(), message.size());

        *commandRetCode = true;
        
    }
    else if (command.ToUpper() == "SETPARAMETERS"){
        *commandRetCode = ParseCommandArgument(xmlUtil,"SETPOINT",&m_Setpoint) || (*commandRetCode);
        *commandRetCode = ParseCommandArgument(xmlUtil,"DELTA",&m_Delta) || (*commandRetCode);
        *commandRetCode = ParseCommandArgument(xmlUtil,"MIN",&m_Min) || (*commandRetCode);
        *commandRetCode = ParseCommandArgument(xmlUtil,"MAX",&m_Max) || (*commandRetCode);

    }

    return true;

}

