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

#include "block.h"

#ifndef STDCOSTANTBLOCK_H
#define STDCOSTANTBLOCK_H



using namespace std;

/**
Classe che implementa una costante come tipo di blocco.

 *
 * COMANDI:
 * -GetCostant -- Legge il valore della costante: risposta <BLOCK COMMAND="Costant" ADDRESS="" VAL="" />
 * -SetCostant -- Sottocampo VAL
 * il blocco non ha ingressi ed ha una uscita
	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CCostantBlock : public CBlock
{
public:
    CCostantBlock(const char* configString);

    ~CCostantBlock();
    
    bool Update();

    bool ExecCommand(CXMLUtil *xmlUtil, bool *commandRetCode);
    
    private :
        
        float m_CostantValue;

};

#endif
