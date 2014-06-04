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

#include "satblock.h"
#include "conewireengine.h"


CSATBlock::CSATBlock(const char* configString)
    : CBlock( configString)
{
    CString tempString;
    float Min=0.0, Max=0.0;
    //Get SAT type and parameters
    m_LibIniReader.GetConfigParamString( configString, "TYPE", &tempString, "SAT");
    
    for (int i = 0; i < SAT_TYPE_NUM_TOT; i++)
    {
        if (tempString == SAT_Type_Strings[i])
        {
            m_SATType = (e_SATType)i;
	    m_LibIniReader.GetConfigParamFloat( configString, "MIN", &Min, 0.0);
            m_LibIniReader.GetConfigParamFloat( configString, "MAX", &Max, 0.0);
            setMinSat(&Min);
            setMaxSat(&Max);
        }
    }
}


CSATBlock::~CSATBlock()
{
}

///////////////////////////////////////////////////
//              Update
///////////////////////////////////////////////////
bool CSATBlock::Update( )
{
    float input1 = 0.0;
    bool retVal = false, isInputValid = false;
    
    //Check if inputs are valid
    //isInputValid = IsInputValid(0) && IsInputValid( 1 );
    isInputValid = IsInputValid(0);
    GetDataFromInput( 0, &input1);
    //GetDataFromInput( 1, &input2);
    
    switch (m_SATType)
    {
        case    SAT_TYPE_SAT:
        {
            	if ((input1 <= m_Max) && (input1 >=m_Min))
		{
			retVal = SetOutputVal( 0, input1, isInputValid);
		}
		else if (input1 > m_Max)
		{
			retVal = SetOutputVal( 0, m_Max, isInputValid);
		}
		else if (input1 < m_Min)
		{
			retVal = SetOutputVal( 0, m_Min, isInputValid);
		}
	};break;
	case    SAT_TYPE_HI:
        {
            	if (input1 >= m_Max)
		{
			retVal = SetOutputVal( 0, m_Max, isInputValid);
		}
		else
		{
			retVal = SetOutputVal( 0, input1, isInputValid);
		}
	};break;
	case    SAT_TYPE_LOW:
        {
            	if (input1 <= m_Min)
		{
			retVal = SetOutputVal( 0, m_Min, isInputValid);
		}
		else
		{
			retVal = SetOutputVal( 0, input1, isInputValid);
		}
	};break;
        default: retVal = false;
    }
    return retVal;
    
}

bool CSATBlock::setMaxSat(float* Max)
{
	m_Max=(*Max);
        return true;
}

bool CSATBlock::setMinSat(float* Min)
{
	m_Min=(*Min);
        return true;
}


bool CSATBlock::ExecCommand(CXMLUtil* xmlUtil, bool* commandRetCode){
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
        com.putValue("BLOCKEXEC","SatParameters");

        com.putValue("MIN",m_Min);
        com.putValue("MAX",m_Max);

        CString message = com.getXMLValue();
        eng->WriteOnInterfacePorts(message.c_str(), message.size());
        *commandRetCode = true;

    }
    else if (command.ToUpper() == "SETPARAMETERS"){

        *commandRetCode = ParseCommandArgument(xmlUtil,"MIN",&m_Min) || (*commandRetCode);
        *commandRetCode = ParseCommandArgument(xmlUtil,"MAX",&m_Max) || (*commandRetCode);
    }

    return true;
}