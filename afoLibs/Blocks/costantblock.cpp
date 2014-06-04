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
 
#include "costantblock.h"
#include "conewireengine.h"


CCostantBlock::CCostantBlock( const char* configString)
    : CBlock( configString)
{
    m_LibIniReader.GetConfigParamFloat( configString, "VALUE", &m_CostantValue, -100.0);

    SetOutputVal(0,m_CostantValue,true);
    
}


CCostantBlock::~CCostantBlock()
{
}

bool CCostantBlock::Update( )
{
    //Il blocco non esegue alcuna operazione: ricopia il valore costante sull'uscita per aggiornare il tempo
    SetOutputVal(0, m_CostantValue, true);
    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool CCostantBlock::ExecCommand(CXMLUtil *xmlUtil, bool *commandRetCode){
    COneWireEngine *eng = reinterpret_cast<COneWireEngine*>(m_EngPtr);

    if (!IsCommandForMe(xmlUtil)){
        return false;
    }

     *commandRetCode = false;
    //Ora interpreto il messaggio
     CString command = xmlUtil->GetStringParam("BLOCKEXEC");
    if (command.ToUpper() == "GETCOSTANT"){
        Cmd com("BLOCK");
        com.putValue("ADDRESS",m_BlockAddress);
        com.putValue("BLOCKEXEC","Costant");
        com.putValue("VAL",m_CostantValue);

        CString message = com.getXMLValue();
        eng->WriteOnInterfacePorts(message.c_str(), message.size());

        *commandRetCode = true;

    }
    else if (command.ToUpper() == "SETCOSTANT"){
            ParseCommandArgument(xmlUtil,"VAL",&m_CostantValue);
            SetOutputVal(0,m_CostantValue,true);
            *commandRetCode = true;
    }

    return true;

}

