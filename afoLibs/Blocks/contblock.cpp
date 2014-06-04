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

#include "contblock.h"
#include "conewireengine.h"


CCONTBlock::CCONTBlock(const char* configString)
    : CBlock(configString)
{
    
    //Get CONT type and parameters
    m_LibIniReader.GetConfigParamFloat( configString, "AMOUNT", &m_ValueToCount, 1.0);

    if (m_ValueToCount <= 0.0){
        m_ValueToCount = 1.0;
    }
    m_LastCountState = -1.0;
    m_CounterValue = 0.0;
    
}

CCONTBlock::~CCONTBlock()
{
}

///////////////////////////////////////////////////
//              Update
///////////////////////////////////////////////////
bool CCONTBlock::Update( )
{
    float countInput = 0.0;
    float reset = 0.0;
    
    bool retVal = false, isInputValid = false;
    
    //Check if inputs are valid
    isInputValid = IsInputValid(COUNTER_IN_LOAD) &&
                   IsInputValid(COUNTER_IN_RESET) &&
                   IsInputValid(COUNTER_IN_COUNT);

    if (!isInputValid){
        SetOutputVal(0,-100.0,false);
        SetOutputVal(1,-100.0,false);
        return false;
    }
    
    GetDataFromInput( COUNTER_IN_COUNT, &countInput);
    GetDataFromInput( COUNTER_IN_RESET, &reset);

    //Controllo se devo resettare... che faccio mi perdo un conteggio ?
    if (reset){
        m_CounterValue = 0.0;
        SetOutputVal(1,0.0,true);
    }
    else {
        if ( (m_LastCountState == 0.0) &&
             (countInput == 1.0) ) {
            m_CounterValue++;

            if (!( ((int)m_CounterValue)%((int)m_ValueToCount))){
                SetOutputVal(1,1.0,true);
            }
        }

         m_LastCountState = countInput;
    }

    SetOutputVal(0,m_CounterValue,true);

    return retVal;
    
}
////////////////////////////////////////////////////////////////////////////////
bool CCONTBlock::ExecCommand(CXMLUtil *xmlUtil, bool *commandRetCode){
    COneWireEngine *eng = reinterpret_cast<COneWireEngine*>(m_EngPtr);

    if (!IsCommandForMe(xmlUtil)){
        return false;
    }

     *commandRetCode = false;
    //Ora interpreto il messaggio
    CString command = xmlUtil->GetStringParam("BLOCKEXEC");
    if (command.ToUpper() == "RESET"){
        m_CounterValue = 0.0;
        SetOutputVal(0,m_CounterValue,true);
        SetOutputVal(1,0.0,true);
        *commandRetCode = true;
    }
    else if (command.ToUpper() == "GETAMOUNT"){
        Cmd com("BLOCK");
        com.putValue("ADDRESS",m_BlockAddress);
        com.putValue("BLOCKEXEC","Amount");
        com.putValue("VAL",m_ValueToCount);

        CString message = com.getXMLValue();
        eng->WriteOnInterfacePorts(message.c_str(), message.size());

        *commandRetCode = true;

    }
    else if (command.ToUpper() == "SETAMOUNT"){
        if (xmlUtil->GetFloatParam("VAL") > 0.0){
            ParseCommandArgument(xmlUtil,"VAL",&m_ValueToCount);
            m_CounterValue = 0.0;
            SetOutputVal(0,m_CounterValue,true);
            SetOutputVal(1,0.0,true);
            *commandRetCode = true;
        }
    }

    return true;

}