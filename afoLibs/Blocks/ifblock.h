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
 
 #ifndef STDIFBLOCK_H
#define STDIFBLOCK_H

#include "block.h"
#include "commonDefinitions.h"

using namespace std;

/**
Questa classe incapsula l'operatore IF logico tra i suoi due input. L'input 1 è considerato il primo argomento del confronto.

 * Il blocco ha 2 ingressi e 1 uscita
	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CIFBlock : public CBlock
{
public:
    CIFBlock(const char* configString);

    ~CIFBlock();

    e_IFType GetIFType() const
    {
      return m_IFType;
    }
    
    bool Update();
    
    private:
        
        e_IFType m_IFType;
        

};


#endif
