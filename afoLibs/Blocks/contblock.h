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

#ifndef STDCONTBLOCK_H
#define STDCONTBLOCK_H

#include "block.h"
#include "commonDefinitions.h"

using namespace std;

#define COUNTER_IN_COUNT 0
#define COUNTER_IN_RESET 1
#define COUNTER_IN_LOAD 2

/**
 * Blocco contatore: conta gli eventi 0->1 che avvengono sul suo ingresso di conteggio e
 * fornisce in uscita il valore di conteggio attuale e un uscita preprogrammabile che
 * scatta ad 1 al termine di un certo conteggio.
 *
 * Stringa di configurazione:
 * NAME:CONTATORE, AMOUNT, INPUTX, OUTPUTx,ADDR,COMMENT
 * Dove:
 * AMOUNT -- e' di quanto deve contare
 * INPUT1 -- e' l'ingresso di conteggio
 * INPUT2 -- e' il reset (fac)
 * INPUT3 -- consente di precaricare un valore (fac)
 *
 * OUTPUT1 -- fornisce il valore attuale di conteggio
 * OUTPUT2 -- va ad 1 al termine del conteggio
 *
 * COMANDI:
 * RESET -- resetta il conteggio
 * SetAmount -- Cambia il valore di conteggio e resetta il contatore, sottocampo VAL
 * GetAmount -- Legge il valore del contatore: risposta <BLOCK COMMAND="Amount" ADDRESS="" VAL="" />
	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CCONTBlock : public CBlock
{
public:
    CCONTBlock(const char* configString);

    ~CCONTBlock();
    
    bool Update();

    bool ExecCommand(CXMLUtil *xmlUtil, bool *commandRetCode);

    private:
        
	float m_CounterValue;
        float m_ValueToCount;
        float m_LastCountState;
	
};

#endif
