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
#ifndef STDCDS18X20_H
#define STDCDS18X20_H

#include <string.h>
#include <stdio.h>
#include <iostream>
#include "vdevice.h"
#include "LibIniFile.h"
#include "cownet.h"
#include "timeUtil.h"
#include "commonDefinitions.h"

#define DS18X20_TEMP_CONV_TIME 800

//using namespace std;

/**
This class is a container for the DS1820/DS18B20/DS18S20 thermometer device.
It can access/program the devices of this kind.

The configuration String has the following parameters:
 -ALARMMIN to set the minimum alarm level
 -ALARMMAX to set the maximum alarm level
 -HYST to set the Hysteresis
 * 
 *  * Rev. 0.7 -- 27/04/2006
 * Aggiunta la possibilita'  di disabilitare la ricerca su allarme hardware e gestirla in modo software: il device conosce gli allarmi di temperatura e li compara con l'ultima temperatura acquisita. Da finire di integrare
 * Aggiunto un ulteriore reset del master nel caso non si riesca a leggere la temperatura
 * Rimosso il parametro di configurazione HYST
 * 
 * Rev. 0.6c -- 03/03/2006
 * Rimosso da vdevice la lettura del parametro HASPOWER e messo qui
 * 
 * Rev. 0.6b -- 01/03/2006
 * Aggiunta la compensazione anche nella parte di definizione degli allarmi
 * 
Rev. 0.6a -- 29/1/2006
Aggiunto il define DS18X20_TEMP_CONV_TIME sul tempo di conversione del dispositivo alimentato
Rimosse le funzioni GetPortNum e SetPortNum perche'inserite nel vdevice.h
Corretto bug in ReadTemperature: non veniva controllato il falg updateFirst e quindi il sensore era aggiornato indipendentemente dalla chiamata
 * 
Rev. 0.6 -- 21/1/2006
Aggiunta una gestione piu' evoluta degli allarmi: aggiunti i parametri 
 * m_TimeOfLastAlarm -- Istante al quale il sensore e'entrato in allarme
 * m_TimeOutOnAlarm -- Tempo massimo di attesa prima di attivare il combinatore telefonico
Il meccanismo e'gestito dalla funzione CheckAlarmCondition()
 
Rev. 0.5b -- 15/1/2006
Cambiata la funzione GetLastTemp() : ora anche se non si sono acquisite temperature non cerca di acquisirne prima di ritornare in modo tale da evitare letture sbagliate da sensori non raggiungibili.
Aggiunto il calcolo esteso della temperatura per superare il limite dei 9 bit per i DS18S20

Rev. 0.5a -- 9/1/2006 Aggiunta la possibilit�di definire gli allarmi minimi e massimi nel file INI per ogni sensore. Se non ci sono vengono impostati a -100 di default
Rev. 0.5 -- 07/01/2006 Modified the functions to set or clear errors in the apropiate CError object. Removed all the references to the old functions. See vdevice.h for comparison
@author Alessandro Mirri
*/
class CDS18X20 : public CVDevice
{
public:
    CDS18X20(int portNum, COWNET *master, char type = 'B', const char* configString = NULL);

    ~CDS18X20();
    
    
    /**
     * Reads the temperature of the device and store it, if newTemp != NULL copies the new temp in the variable.
     * As a result also the local copy of the device memory is updated
     * @param updateFirst Flag that indicates if we have to perform a temperature conversion before reading the value
     * @param newTemp the variable to return
     * @return TRUE if operation successful
     */
    bool ReadTemperature(bool updateFirst, float *newTemp );
    
    float ReadTemperature(bool updateFirst);
    
    /**
     * Forces the temperature measurement of the device
     * @return TRUE if operation successful
     */
    bool UpdateTemp();
    
    bool SetAlarmLevel(int MaxAlarmLevel, int MinAlarmLevel);
    
    float GetCompensation() { return m_Compensation; };

    //Viene chiamata se è cambiato il numero di serie per verificarne la cosnistenza
    void SetFamilyAndDevice();

       
  private:
    float m_Compensation;
    uchar m_ScratchPad[9];      //!Local Copy of the device's scratchpad
    
};



#endif
