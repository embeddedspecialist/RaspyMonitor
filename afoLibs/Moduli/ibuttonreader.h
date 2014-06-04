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
 #ifndef STDIBUTTONREADER_H
#define STDIBUTTONREADER_H

#include "vcoordinator.h"
#include "vdevice.h"
#include "ownet.h"
#include "commonDefinitions.h"
#include "digitalio.h"
#include "cstring.h"

using namespace std;

/**
Legge, memorizza e invia gli iButton trovati sul bus
 * Riga di configurazione:
 * NAME:IButtRdr, GREENLIGHT:1234, REDLIGHT:4321
 * Dove:
 * GREENLIGHT e' un digitalinout che controlla la luce verde
 * REDLIGHT e' un digitalinout che controlla la luce rossa 

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
//TODO invio del numero di serie via rete
class CIButtonReader : public CVCoordinator
{
public:
    CIButtonReader( const char *configString, CTimer *timer);

    ~CIButtonReader();
    
    bool ConnectControllers();
    bool Update(bool updateData);
    bool Update2(bool updateData){return false;};
    bool SetVal(float val){return false;};
    //TODO
    CString GetSpontaneousData(int lParam = 0){return "";};
    
    vector<CString> m_SerialNumberVector;
    
    void SetLight(bool greenLight, bool turnOn);
    
    private:
        CDigitalIO *m_GreenLight, *m_RedLight;
        int m_GreenLightAddr, m_RedLightAddr;
        bool m_LightControlEnabled;
        
};


#endif
