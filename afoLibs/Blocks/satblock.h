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

#include "block.h"
#include "commonDefinitions.h"

#ifndef STDSATBLOCK_H
#define STDSATBLOCK_H

using namespace std;

/**
 L'uscita non supera i valori MIN e MAX al variare dell'ingresso
 *
 *
 * COMANDI:
 * GetParameters - <BLOCK COMMAND="SatParameters" ADDRESS="" MIN="" MAX="" />
 * SetParameters -

 * Il blocco ha 2 ingressi e 1 uscita
	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CSATBlock : public CBlock
{
public:
    CSATBlock(const char* configString);

    ~CSATBlock();

    e_SATType GetSATType() const
    {
      return m_SATType;
    }
    
    bool Update();

    bool setMinSat(float* Min);

    bool setMaxSat(float* Max);

    bool ExecCommand(CXMLUtil *xmlUtil, bool *commandRetCode);
    
    private:
        
        e_SATType m_SATType;
	float m_Max;
	float m_Min;
        

};

#endif
