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

#ifndef _TIMERBLOCK_H
#define	_TIMERBLOCK_H

#include "block.h"
#include "timer.h"


using namespace std;
//TODO: Il timer è annuale!!! Non credo proprio vada bene....
//o comunque va data la possbilita' di averlo annuale o settimanale.
/**
 *Blocco di gestione dei timer. Il blocco legge dal file di configurazione
 *il suo ID e carica dal file timers.ini i parametri relativi al timer.
 *
 *String di configurazione:
 *NAME:Timer, ID, ADDR, INPUT1, OUTPUTX, COMMENT
 *Dove:
 *ID        -- Identificativo del timer a cui si riferisce nel file timers.ini
 *ADDR      -- indirizzo in memoria dell'oggetto
 *OUTPUTX   -- Uscite del timer:
 *          1 - Uscita digitale ON/OFF 
 *          2 - Uscita Analogica
 *COMMENT   -- Commento
 */

class CTimerBlock : public CBlock
{
public:
    CTimerBlock(const char* configString);
    ~CTimerBlock();
    
    bool Update();
    
    bool LoadTimer();
protected:
    
    //Which timer is represented by this object
    int m_TimerID;
    bool m_IsTimerEnabled;

    CTimer m_Timer;
    
    
};


#endif	/* _TIMERBLOCK_H */


