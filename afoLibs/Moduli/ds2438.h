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
#ifndef DS2438_H
#define DS2438_H

#include "vdevice.h"
#include "cownet.h"
#include "timeUtil.h"

#define DS2438_TEMP_CONV_TIME 15

#define DS2438_TEMPERATURE_INDEX 0
#define DS2438_VOLTAGE_INDEX 1
#define DS2438_CURRENT_INDEX 2

//FIXME:Aggiungere gestione completa della memoria

/**
This class is a wrapper class for the DS2438 device.

 * Rev. 0.6 - 22/3/2006
 * Aggiunte le funzioni per impostare il campionamento della corrente da parte del dispositivo e la successiva lettura
 * Corretti alcuni bug nella ReadAtoD
 * Cambiati i nomi di alcune funzioni per riflettere meglio quello che fanno
 *
 * Rev 0.5a -28/04/2006 -- Aggiunta la funzione GetMemoryPage
 *
Rev 0.5 - 07/01/2006 -- Riscritte completamente le funzioni ReadTimer(), ReadAtoD() e SetAtoD() al fine di utilizzare efficacemente la gestione
                        degli errori attraverso l'oggetto CError e di rimuovere un probabile bug nella logica di funzionamento.
 *
 * 
 * NAME:DS2438,SN:xxxxx, HASPOWER:1,COMP:0.0,CURRENT:1,TEMPERATURE:1,VOLTAGE:1
 * Dove:
 * HASPOWER -- indica se il sensore e' alimentato
 * COMP -- fattore di compensazione della temperatura
 * CURRENT -- Lettura corrente si/no
 * TEMPERATURE -- lettura temperatura si/no
 * VOLTAGE -- Lettura tensione si/no


@author Alessandro Mirri
*/
class CDS2438 : public CVDevice
  {
  public:
      CDS2438( int portNum, COWNET *master, const char* configString);

    ~CDS2438();

    /**
     * Funzione principale di aggiornamento per i sensori ds2438: da questa funzione vengono ricavate tutte le misure
     * @return TRUE se tutte le operazioni sono a buon fine
     */
    bool ReadAllData();
    
    /**
     * Reads the last temperature measured from the internal device
     * @return the current measured temperature, or -200.0 if an error occurred
     */
    float ReadTemperature(bool updateFirst);

    bool ReadTemperature(bool updateFirst, float *newTemp );

    /**
     * Reads the VDD or VAD
     * @param readVdd flag indicating weather to read the Vdd or the Vad
     * @return the value if operation succesful, -1 otherwise
     */
    float ReadVoltage( bool readVdd );

    /**
     * Reads the internal ETM (Elapsed Time Meter) timer
     * @return The time elapsed in seconds from the starting event, -1 if an error occurred
     */
    int ReadTimer();

    /**
    * Forces the temperature measurement of the device
    * @return TRUE if operation successful
    */
    bool UpdateTemp();

    int ReadCurrentRegister();


    /**
     * Gets the given memory page from the device
     * @param memoryPage the memory page to read
     * @param destination the destination array
     * @return FALSE if an error occurred
     */
    bool GetMemoryPage(uchar memoryPage, uchar* destination);

    bool UpdateMeasures();

    void SetCompensation(float theValue)
    {
      m_Compensation = theValue;
    }


    float GetCompensation() const
    {
      return m_Compensation;
    }

    //TODO:aggiungere tutte le funzioni per configurare il dispositivo

      /**
     * Sets the device to measure Vad or Vdd
     * @param measureVdd flag indicating weather to set the Vdd or the Vad
     * @return TRUE if operation successful
       */
    bool SetVoltageMeasurement( bool measureVdd );


  private:

    bool EnableCurrentMeasuring(bool enable);

    bool m_IsMeasureVddEnabled, m_IsMeasureCurrentEnabled;

    char m_RAMScratchPad[ 2 ][ 8 ];  //Copy of the internal RAM memory
    char m_EEScracthPad[ 5 ][ 8 ];   //Copy of the internal EEPROM memory
    float m_Compensation;
    bool m_ReadTemp, m_ReadVad, m_ReadCurrent;

  };

#endif
