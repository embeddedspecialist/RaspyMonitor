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
#ifndef STDREMOTEDIDO_H
#define STDREMOTEDIDO_H

#include "vdevice.h"
#include "vmultidido.h"
#include "commonDefinitions.h"
#include "ds2405.h"
#include "ds2408.h"

//using namespace std;

/**
This class is used to activate digital outputs on remote machines by acquiring locally the command
Config Line:
 * DeviceXX= NAME:RemoteDIDO,INPUT,CHANNEL,ADDR,REMOTEADDRESS,INVERTIN,COMMENT
 * 
	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CRemoteDIDO : public CVMultiDIDO
{
public:
    CRemoteDIDO(const char* configString = NULL, CTimer *timer=0x0);

    ~CRemoteDIDO();
    

    bool Update(bool updateData);
    bool Update2(bool updateData);
    bool SetVal(float val){return false;};
    
    CString GetSpontaneousData(int lParam = 0) { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};
    
    int GetRemoteAddress(){return m_RemoteAddress;};
    
    bool ChangeOutput(int channel = 0) {return true;};
    
    bool InitOutput(int channel = 0){ return true;};
    
    bool SetOutput(bool newState, int channel = 0){ return false;};

    bool m_RemoteChangeState;
    int m_JollyValue;
    
    private:
        

        
        int m_RemoteAddress;
                
        
};


#endif
