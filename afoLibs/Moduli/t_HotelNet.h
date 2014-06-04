/***************************************************************************
 *   Copyright (C) 2006 by Alessandro Mirri   *
 *   alessandro.mirri@newtohm.it   *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 *************************************************************************/
#ifndef STDT_NET_H
#define STDT_NET_H

#include <vector>

#include "cownet.h"
#include "afoerror.h"
#include "vcontroller.h"

using namespace std;

/**
This class is a replacement for the old typedef T_Net. It holds all the specific values of the NET

        @author Alessandro Mirri <alessandro.mirri@newtohm.it>
 */
class T_HNet{
    public:
        T_HNet(){ CtrlList.reserve(MAX_NUM_DEV_PER_NET/2); };

        ~T_HNet(){};

        COWNET master;                                                            //!The master of the subnet
        vector<CVController*> CtrlList;
        SMALLINT portHandler;                                                     //!The Port to which the master is linked
        bool isAcquired;                                                          //!Acquired flag
        bool correctlySet;                                                        //!Flag indicating correct setup of the net
        int configNetIndex;                                                       //!Index of the net in the configuration file
};



#endif
