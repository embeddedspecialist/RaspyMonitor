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

#ifndef _GATEBLOCK_H
#define	_GATEBLOCK_H

#include "block.h"

using namespace std;
/**
 * Stringa di configurazione:
 * NAME:GATE, TYPE, INPUTX, OUTPUTX, STARTV, ADDR, COMMENT
 * Dove:
 * INPUTX   -- Ingresso:
 *          FLIP FLOP JK: INPUT1 = J, INPUT2 = K
 * OUTPUTX  -- Uscita del gate: 1 = uscita, 2 = uscita negata
 * ADDR     -- Indirizzo in memoria
 * COMMENT  -- commento
 * TYPE     -- Tipo del GATE, puo' essere : 
 *              "JK" -- Flip Flop JK, 
 *              TODO -- Da aggiungere altri blocchi eventualmente
 * STARTV -- Valore iniziale dell'uscita 1
 
	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CGateBlock : public CBlock
{
public:
    CGateBlock(const char* configString);

    ~CGateBlock();
    
    bool Update();
    
private:
    
    int m_OutputState;
    e_GATEType m_GateType;
    int m_StartValue;
};


#endif	/* _GATEBLOCK_H */

