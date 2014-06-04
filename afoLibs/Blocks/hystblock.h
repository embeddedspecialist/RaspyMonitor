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

#ifndef STDHYSTBLOCK_H
#define STDHYSTBLOCK_H

#include "block.h"
#include "commonDefinitions.h"

using namespace std;

/**
 Implementa un controllo con isteresi
 *
 * Stringa:
 * NAME:ISTERESI, SETPOINT, BAND, MIN,MAX, ADDR, INPUT1, INPUT2, OUTPUT1, COMMENT
 * Dove:
 * SETPOINT -- e' il punto centrale dell'isteresi
 * DELTA -- SemiAmpiezza della regolazione di isteresi
 * MIN,MAX -- Valori in uscita da applicare nelle due fasce
 * INPUT1 -- ingresso del controllo
 * INPUT2 -- Ingresso setpoint (fac)
 * OUTPUT1 -- uscita

 * Comandi:
 * SetParameters -- Imposta i parametri a seconda dei sottocampi SETPOINT,DELTA,MIN e MAX
 * GetParameters -- ritorna i parametri <BLOCK COMMAND="HystParameters" ADDRESS="" SETPOINT="" DELTA="" MIN="" MAX=""/>
 *
 * Il blocco ha 2 ingressi e 1 uscita
	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CHYSTBlock : public CBlock
{
public:
    CHYSTBlock(const char* configString);

    ~CHYSTBlock();

    e_HYSTType GetHYSTType() const
    {
      return m_HYSTType;
    }
    
    bool Update();

    bool setSetpoint(float* setpoint);

    bool setDelta(float* dT);

    bool setLastOutput(float* output);

    bool setMin(float* Min);

    bool setMax(float* Max);

    bool ExecCommand(CXMLUtil *xmlUtil, bool *commandRetCode);
    
    private:
        
        e_HYSTType m_HYSTType;
	float m_Setpoint;
	float m_Delta;
	float m_Max;
	float m_Min;
	float m_LastOutput;
};

#endif
