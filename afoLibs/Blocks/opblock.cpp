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

#include "opblock.h"


COPBlock::COPBlock(const char* configString)
    : CBlock(configString)
{
    CString tempString;
    
    //Get OP type and parameters
    m_LibIniReader.GetConfigParamString( configString, "TYPE", &tempString, "ADD");
    
    for (int i = 0; i < OP_TYPE_NUM_TOT; i++)
    {
        if (tempString == OP_Type_Strings[i])
        {
            m_OPType = (e_OPType)i;
        }
    }
}


COPBlock::~COPBlock()
{
}

///////////////////////////////////////////////////
//              Update
///////////////////////////////////////////////////
bool COPBlock::Update( )
{
    float input1 = 0.0, input2 = 0.0;
    bool retVal = false, isInputValid = false;
    
    //Check if inputs are valid
    isInputValid = IsInputValid(0) && IsInputValid( 1 ); 
    GetDataFromInput( 0, &input1);
    GetDataFromInput( 1, &input2);
    
    switch (m_OPType)
    {
        case    OP_TYPE_ADD:
        {
            retVal = SetOutputVal( 0, input1 + input2, isInputValid);
        };break;
        case    OP_TYPE_SUB:
        {
            retVal = SetOutputVal( 0, input1 - input2 , isInputValid);
        };break;
        case    OP_TYPE_MUL:
        {
            retVal = SetOutputVal( 0, input1 * input2, isInputValid);
        };break;
        case    OP_TYPE_DIV:
        {
            if (input2!=0.0)
		retVal = SetOutputVal( 0, input1 / input2, isInputValid);
	    else
		retVal = SetOutputVal( 0, -100.0, false);
        };break;
	case    OP_TYPE_REM:
        {
            if (input2!=0.0)
		retVal = SetOutputVal( 0, (int)input1 % (int)input2, isInputValid);
	    else
		retVal = SetOutputVal( 0, -100.0, false);
        };break;
        default: retVal = false;
    }
    
    
    return retVal;
    
}
