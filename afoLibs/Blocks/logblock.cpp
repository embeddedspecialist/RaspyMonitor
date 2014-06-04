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

#include "logblock.h"


CLOGBlock::CLOGBlock(const char* configString)
    : CBlock(configString)
{
    CString tempString;
    
    //Get IF type and parameters
    m_LibIniReader.GetConfigParamString( configString, "TYPE", &tempString, "AND");
    
    for (int i = 0; i < LOG_TYPE_NUM_TOT; i++)
    {
        if (tempString == LOG_Type_Strings[i])
        {
            m_LOGType = (e_LOGType)i;
        }
    }
}


CLOGBlock::~CLOGBlock()
{
}

///////////////////////////////////////////////////
//              Update
///////////////////////////////////////////////////
bool CLOGBlock::Update( )
{
    float input1 = 0.0, input2 = 0.0;
    bool retVal = false, isInputValid = false;
    
    //Check if inputs are valid
    if (m_LOGType != LOG_TYPE_NOT){
        isInputValid = IsInputValid(0) && IsInputValid( 1 );
    }
    else {
        isInputValid = IsInputValid(0);
    }

    if (!isInputValid){
        SetOutputVal( 0, -100.0, false);
        return true;
    }
    //isInputValid=true;
    if (m_LOGType!=LOG_TYPE_NOT)
    {
	GetDataFromInput( 0, &input1);
    	GetDataFromInput( 1, &input2);
    }
    else
    {
	GetDataFromInput( 0, &input1);
    }
    
    switch (m_LOGType)
    {
        case     LOG_TYPE_AND:
        {
            retVal = SetOutputVal( 0, (int)input1 && (int)input2, isInputValid);
        };break;
	case     LOG_TYPE_NAND:
        {
            retVal = SetOutputVal( 0, !(input1 && input2), isInputValid);
        };break;
        case    LOG_TYPE_OR:
        {
            retVal = SetOutputVal( 0, input1 || input2 , isInputValid);
        };break;
	case    LOG_TYPE_NOR:
        {
            retVal = SetOutputVal( 0, !(input1 || input2) , isInputValid);
        };break;
        case    LOG_TYPE_XOR:
        {
            retVal = SetOutputVal( 0, (input1 || input2) && !(input1 && input2), isInputValid);
        };break;
	case    LOG_TYPE_XNOR:
        {
            retVal = SetOutputVal( 0, !((input1 || input2) && !(input1 && input2)), isInputValid);
        };break;
        case    LOG_TYPE_NOT:
        {
            retVal = SetOutputVal( 0, !(int)input1, isInputValid);
        };break;
        default: retVal = false;
    }
    
    
    return retVal;
    
}
