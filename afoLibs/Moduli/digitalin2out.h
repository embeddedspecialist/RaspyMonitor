/***************************************************************************
 *   Copyright (C) 2006 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it                                           *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 *************************************************************************/
 #ifndef DIGITALIN2OUT_H
#define DIGITALIN2OUT_H

#include "vmultidido.h"
#include "timer.h"

/**
	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CDigitalIN2OUT : public CVMultiDIDO
{
public:
    CDigitalIN2OUT(const char* configString, CTimer* timer);
    virtual ~CDigitalIN2OUT();
    
    inline bool IsLevelIN(void) const { return m_IsLevelIN; }
    inline bool IsLevelOUT(void) const { return m_IsLevelOUT; }   

protected:
    bool m_IsLevelIN; //true se ingresso di tipo 'Livello', false se di tipo 'Impulso'
    bool m_IsLevelOUT; //true se ingresso di tipo 'Livello', false se di tipo 'Impulso'
    //true,true: TAG
    //false,true: Button
    //true,false: Step
    //non ammessa la configurazione false,false.
};

#endif
