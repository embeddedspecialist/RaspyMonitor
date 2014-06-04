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
#ifndef STDCANALOGIO_H
#define STDCANALOGIO_H

#include "vcontroller.h"
#include "ds2438.h"
#include "ds2890.h"

//Minima variazione che l'AO sente sul comando setVal
#define ANALOG_OUT_MINERROR 0.05
//using namespace std;

/**
This class is a wrapper for a generic Analog Input device based on the Dallas-Maxim ds2438
 NAME:AnalogINOUT,READCURRENT:0,MAXVOLTAGE:10,STARTVAL:0,TIMERID:0,RSENS:120,SCALE:10.0,OFFSET:0.0
 * Dove:
 * READCURRENT -- indica se e' una lettura in corrente
 * RSENS -- E' il valore della resistenza di trasformazione corrente/tensione per lettura ritaratore
 * STARTVAL -- Valore di avvio
 * SCALE e OFFSET -- sono per la messa in scala della lettura
 * MAXVOLTAGE -- e' il fondo scala
	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CAnalogIO : public CVController
{
    public:
        CAnalogIO(CVDevice *inputDevice, const char* configString = NULL, CTimer *timer=0x0);
        CAnalogIO(const char* configString = NULL, CTimer *timer=0x0);
    
        ~CAnalogIO();
        
        bool SetInputDevice(CVDevice* inDevice);
        /////////////////// INPUT Declarations //////////////////////////////////
        
        /**
         * Allows to change the input from current to voltage and viceversa
         * @param isCurrentInput if true the device will read the current, if false it will read the voltage
         * @return true if operation successfull
         */
        bool SetInputCurrent(bool isCurrentInput);
        bool GetReadCurrentState() { return m_ReadCurrent; };
        
        ///////////////////// Output declarations ///////////////////////////////
        /**
         * Set the position of the wiper as an absolute number between 0x0 and 0xff
         * @param newPos the new position of the wiper
         * @return true if operation successfull
        */
        bool SetPosition(uchar newPos);
    
   
        /**
         * Given the max voltage at which the ds2890 is connected it sets the wiper position to match the desired voltage
         * @param newV the new voltage output to set
         * @return true if operation successfull
        */
        bool SetVOutput(float newV);
    
        /**
         * Stores internally the maximum voltage value at which it is connected. Maximum voltage is 11 V.
         * @param newMaxV the new max voltage
         * @return true if operation successfull
        */
        bool SetMaxVoltage(int newMaxV) ;
    
        
        
        /////////////////// Generic ////////////////////////////////
        bool Update (bool updateData);
        bool Update2(bool updateData);
        bool SetVal(float val);
        
        bool VerifyIOPresence();
        CString GetSpontaneousData(int lParam = 0) { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};
        
        bool IsInput() {return m_IsInput;};
        
        /**
         * Returns the last good measure
         * @return last good measure 
        */
        float GetLastValue() { return m_LastVal;};

        float GetLastValueVolt() { return m_LastOutputVolt;};
        
        /**
         * Reads from the device the input value
         * @return the value read from the device or -1.0 if an error occurred
         */
        float GetValue(bool updateFirst);
        
        bool InitDevice();

        int ReadCurrentRegister();

    
    private:
        
        //If output it holds the last position of the wiper, if input the last value acquired
        float m_LastVal;
        float m_LastOutputVolt;

        CVDevice* m_InDevice;
        
        bool m_ReadCurrent;
        
        bool m_IsInput;

        //fattore di scala per avere la misura "vera"
        float m_ScaleFactor, m_OffsetValue;
        
        //!Voltage at which it is connected
        int m_MaxVoltage;
        
        int m_StartPositionVolt;
        
        //The value of the Rsens resistor to measure the current.
        int m_ResistorValue;
};


#endif
