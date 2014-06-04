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

#ifndef STDMUXBLOCK_H
#define STDMUXBLOCK_H

#include "block.h"
#include "commonDefinitions.h"

using namespace std;

/**
 * Questa classe implementa il funzionamento di un multiplexer/demultiplexer (TODO)
 * Ha due tipi di ingressi: quelli di comando e quelli di multiplexing
 *
 * Riga di configurazione:
 * NAME:MUX, INPUT1:,INPUT2:, COMMAND:1,COMMAND:2,OUTPUT1:
 * Dove:
 * - INPUT ingressi soliti
 * - COMMAND ingressi di comando del MUX per la selezione del canale
 * - OUTPUT il canale di uscita
 * 	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CMUXBlock : public CBlock
{
public:
    CMUXBlock(const char* configString);

    ~CMUXBlock();
    
    bool Update();
    
    private:

        

};

#endif
