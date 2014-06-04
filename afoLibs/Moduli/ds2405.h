/***************************************************************************
 *   Copyright (C) 2005 by Alessandro Mirri                                *
 *   amirri@deis.unibo.it                                                  *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 ***************************************************************************/
#ifndef DS2405_H
#define DS2405_H

#include <string.h>

#include "vdevice.h"
#include "LibIniFile.h"
#include "cownet.h"
/**
This class is a common manage class for the DS2405 addressable switch

Rev. 0.5 - 07/01/2006 -- Added the handling of errors through the use of the CError object in all functions
@author Alessandro Mirri
*/
class CDS2405 : public CVDevice
{
public:
    CDS2405(int portNum, COWNET *master, const char* configString);

    ~CDS2405();
    
    //FIXME aggiungere una gestione piu' intelligente: ogni switch deve sapere se e'un ingresso o una uscita e memorizzarselo
    
    /**
     * Gets the internally stored state of the switch, does not necessarily reflect the real state
     * @return the stored state of the switch (ON
     */
    bool GetState(){return m_StateOn;};
    
    /**
     * Reads the real state of the switch, updates the internal variable
     * @return the real state of the device
     */
    bool ReadStateFromDevice();
    
    /**
     * Sets the switch on or off, updates the internal variable
     * @param setOn 
     * @return TRUE if operation successful
     */
    bool SetSwitch(bool setOn);
    
    
  private:
    bool m_StateOn;  //State of the device: TRUE if switch On

};

#endif
