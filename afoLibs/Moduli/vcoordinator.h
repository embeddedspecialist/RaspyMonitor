/***************************************************************************
 *   Copyright (C) 2008 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it                                           *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 *************************************************************************/
 #ifndef STDVCOORDINATOR_H
#define STDVCOORDINATOR_H

#include "vcontroller.h"
#include "temperaturecontroller.h"
#include "digitalio.h"
#include "analogIO.h"

using namespace std;

/**
This class represents a coordinator for the the controllers. In other words it interfaces with more coordinators in order to perform special advanced tasks that require more information than a single controller.

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/

//Defines degli errori restituiti da RetrieveCtrl//
#define RC_OK 0
#define RC_INVALID_VPTR 1
#define RC_INVALID_ADDR 2
#define RC_INVALID_NETINDEX 3
#define RC_INVALID_CTRLINDEX 4


class CVCoordinator : public CVController
{
public:
    CVCoordinator(const char *configString) : CVController( configString ) {m_InitOk = false;m_IsCoordinator = true;};

    virtual ~CVCoordinator(){};

    bool Update(bool updateData){return false;};

    vector<CVController*> m_ControllerList;

    virtual bool ConnectControllers() = 0;

    bool VerifyIOPresence(){return true;};
    
    //Funzione per ricavare un puntatore ad un controller dal suo ADDR//
    //anche statica, può servire a prescindere dalla classe!
    // errorr = 0x0 se non lo si vuole, comunque in caso d'errore ritorna NULL;
    static CVController* RetrieveCtrl( void* pNet, int Addr, unsigned short* errorCode = 0x0 );
        
    protected:
        bool m_InitOk;
        
        CVController* RetrieveCtrl( int Addr, unsigned short* errorCode = 0x0 ); //non serve che sia virtual tanto è privata!
                                                                             //nella classe derivata ne faccio l'override per la messaggistica
                                                                             //e richiamo la funzione della classe base;
};

#endif
