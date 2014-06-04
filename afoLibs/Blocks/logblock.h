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

#ifndef STDLOGBLOCK_H
#define STDLOGBLOCK_H

#include "block.h"
#include "commonDefinitions.h"

using namespace std;

/**
Questa classe incapsula gli operatori logici tra i suoi due input. L'input 1 è considerato il primo argomento del confronto.

 * Il blocco ha 2 ingressi e 1 uscita
 * Stringa di configurazione
 * NAME:LOGICO, TYPE, INPUT1, INPUT2, OUTPUT1, ADDR, COMMENT
 * Dove:
 * INPUT1   -- Primo argomento operazione
 * INPUT2   -- Secondo argomento operazione
 * OUTPUT1  -- Risultato
 * ADDR     -- Indirizzo in memoria
 * COMMENT  -- commento
 * TYPE     -- Tipo di operazione: AND, OR, XOR, NOT, NAND, NOR, XNOR
 * 
	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CLOGBlock : public CBlock
{
public:
    CLOGBlock(const char* configString);

    ~CLOGBlock();

    e_LOGType GetLOGType() const
    {
      return m_LOGType;
    }
    
    bool Update();
    
    private:
        
        e_LOGType m_LOGType;
        

};


#endif
