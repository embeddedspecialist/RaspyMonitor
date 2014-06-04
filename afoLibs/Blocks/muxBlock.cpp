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

#include "muxBlock.h"


CMUXBlock::CMUXBlock(const char* configString)
    : CBlock( configString)
{

}


CMUXBlock::~CMUXBlock()
{
}

///////////////////////////////////////////////////
//              Update
///////////////////////////////////////////////////
bool CMUXBlock::Update( )
{

    bool dataValid = true;
    float outputVal = 0.0;
    float commandVal = 0.0;
    float inputIndex = 0;

    //Innanzitutto controllo se tutti gli ingressi sono validi....
    for (int i = 0; i < m_InputVector.size(); i++)
    {
        dataValid = dataValid && IsInputValid(i);
    }

    if (!dataValid){
        SetOutputVal(0,-100.0,false);
        return false;
    }

    GetDataFromInput(0,&inputIndex);

    //Aumento di 1 perche' l'ingresso 0 e' quello di selezione
    inputIndex++;
    
    if ((inputIndex <= 16.0) && (inputIndex>0)){
        float outVal;
        GetDataFromInput((int)inputIndex,&outVal);
        SetOutputVal(0,outVal,true);
    }
    else
    {
        //Lo marco come NON valido
        SetOutputVal(0, -100.0, 0);
    }

    return true;
    
}

