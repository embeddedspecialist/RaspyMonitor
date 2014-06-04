/***************************************************************************
 *   Copyright (C) 2007 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it                                           *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 ***************************************************************************/
#ifndef STDNTHVLV_H
#define STDNTHVLV_H

#include "vcontroller.h"
#include "timer.h"
#include "ds2751.h"

//using namespace std;

//TODO aggiungere la gestione degli errori e il conteggio per la blacklist
/**
THis class is used to access the newtohm's valve control modules over the 1wire net
 *
---------------------------------------CMD 0x01-----------------------------------------------------
Ms-Tx
  sram[0]=0x01 il master chiede lo stato dei parametri e delle impostazioni

Ms-Rx
  Mapmem[0] = (~comando)-1
  Mapmem[1] = Temp sonda ambiente (reg_lw.del dsxx)
  Mapmem[2] = Temp sonda limite (reg_lw.del dsxx)
  Mapmem[3] = Bit 7-4 reg_hi sonda ambiente  Bit 3-0 reg_hi sonda limite
  Mapmem[4] = P1
  Mapmem[5] = I1
  Mapmem[6] = D1
  Mapmem[7] = P2
  Mapmem[8] = I2
  Mapmem[9] = D2
  Mapmem[10]= Setpoint interno
  Mapmem[11]= Setpoint esterno
  Mapmem[12]= Val dac_A
  Mapmem[13]= Val dac_B
  Mapmem[14]= Flag_state :(X)(X)(X)(PotInt/Ext)(Pid2)(Pid1)(E/I)(ViW)
  Mapmem[15]= 0xXX

---------------------------------------END-----------------------------------------------------------

---------------------------------------CMD 0x02-----------------------------------------------------
Ms-Tx
  Mapmem[0]=0x02 il master passa leconfigurazioni dei parametri per le impostazioni

Ms-Rx
  Mapmem[0] = (~comando)-1
  Mapmem[1] = Flag_START   //avvia il pid. deve essere mantenuto a 0x00
  Mapmem[2] = SetPoint_sondaLimite--H--
  Mapmem[3] = SetPoint_sondaLimite--L--
  Mapmem[4] = P1
  Mapmem[5] = I1
  Mapmem[6] = D1
  Mapmem[7] = P2
  Mapmem[8] = I2
  Mapmem[9] = D2
  Mapmem[10]= Setpoint
  Mapmem[11]= bit0 = rele' DO1, bit1 = rele' DO2 //TODO Sistemare lo spegnimento sul bit1
  Mapmem[12]= Val dac_A
  Mapmem[13]= Val dac_B
  Mapmem[14]= Flag_state :(X)(X)(X)(PotInt/Ext)(Pid2)(Pid1)(I/E)(X)
  Mapmem[15]= 0xXX

---------------------------------------END-----------------------------------------------------------

---------------------------------------CMD 0x03-----------------------------------------------------
Ms-Tx
  Mapmem[0]=0x03 il master chiede ll valore degi Internal Special Parameters

Ms-Rx
  Mapmem[0] = (~comando)-1
  Mapmem[1] = cal_max
  Mapmem[2] = cal_min
  Mapmem[3] = SetPointHigh limite mandata
  Mapmem[4] = SetPointLow  limite mandata
  Mapmem[5] = 0xFF
  Mapmem[6] = 0xFF
  Mapmem[7] = 0xFF
  Mapmem[8] = 0xFF
  Mapmem[9] = 0xFF
  Mapmem[10]= 0xFF
  Mapmem[11]= 0xFF
  Mapmem[12]= 0xFF
  Mapmem[13]= 0xFF
  Mapmem[14]= 0xFF
  Mapmem[15]= 0xFF

---------------------------------------END-----------------------------------------------------------

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CNTHVLV : public CVController
{
public:
    CNTHVLV(const char* configString, CTimer *timer = 0x0);

    ~CNTHVLV();

    bool Update(bool updateData);
    bool Update2(bool updateData){return false;};
    bool SetVal(float newVal){return false;};
    
    bool VerifyIOPresence();
    CString GetSpontaneousData(int lParam = 0) { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};

    void SetInputDevice(CDS2751* theValue);

    CDS2751* GetInputDevice() const;

    //TODO aggiungere gestione dei timer per il setpoint!!
    //TODO aggiungere funzione per spegnere e accendere

    /**
     * Writes on the SRAM of the device the provided 16 bytes
     * @param newSRAM a 16 bytes array to write on SRAM
     * @return true if writing successfull, false otherwise
     */
    bool SetSRAM(uchar *newSRAM);


    /**
     * Retrieves all the configuration from the device. It reads the status from the device then adds the
     * extra parameters from the state vector
     * @param destination a 17 bytes array, fixed length
     * @return true if data has been recovered
     */
    bool GetAllData(uchar *destination);

    /**
     * Retrieves fron the device the temperatures and the setpoint
     * @param tempInt Internal temperature measured by the device
     * @param tempExt Outside temperature measured
     * @param setpoint current setpoint used
     * @return true if data has been correctly read from the device
     */
    bool GetBasicdata(float *tempInt, float *tempExt, float *setpoint, bool *isSummer);

    /**
     * Reads the content of the device SRAM
     * @param destination a 16 bytes fixed length array
     * @return true if memory has been correctly read
     */
    bool GetSRAM (uchar *destination);
    /**
     * Function used to initialize the device: it writes all the configuration read from the config.ini file
     * and sets the FLAG_START to 0xFF once initialization is done
     * @return true if initialization performed
     */
    bool InitDevice();

    /**
     * Changes the internal setpoint of the device
     * @param newSetPoint new setpoint
     * @return true if setpoint has been written
     */
    bool SetSetPoint( float newSetPoint );

    /**
     * Sets all the configuration in the device by writing into it the values. Then it reads back the answer of the device
     * and stores it in the state vector
     * @param newData an 15 bytes fixed length array
     * @return true if data correctly set
     */
    bool SetAllData(uchar command, uchar *newData);

    /**
     * Writes the informations stored in the provided state vector into the device
     * @return true if operation successfull
     */
    bool WriteStateVector(uchar *newStateVector);

    /**
     * Retrieves the informations stored in the state vector
     * @param destination an 17 bytes fixed lentgh array
     */
    void GetStateVector(uchar *destination) {GetAllData(m_StateVector)?memcpy(destination, m_StateVector, 17):memset(destination,0x0,17);};
    
    bool ChangeDOOutput(bool turnOn );

    bool SetSummer(bool isSummer);

    bool getIsOutputOn() const
    {
        return m_IsOutputOn;
    }
    
    
private:
    CDS2751 *m_Device;

    uchar m_SRAMVector[15];

    /*!Contains the state of the device in the following form

      Mapmem[0] = Temp sonda ambiente (reg_lw.del dsxx)
      Mapmem[1] = Temp sonda limite (reg_lw.del dsxx)
      Mapmem[2] = Bit 7-4 segno sonda ambiente  Bit 3-0 segno sonda limite
      Mapmem[3] = P1
      Mapmem[4] = I1
      Mapmem[5] = D1
      Mapmem[6] = P2
      Mapmem[7] = I2
      Mapmem[8] = D2
      Mapmem[9]= Setpoint interno
      Mapmem[10]= Setpoint esterno
      Mapmem[11]= Val dac_A
      Mapmem[12]= Val dac_B
      Mapmem[13]= Flag_state :(X)(X)(X)(PotInt/Ext)(Pid2)(Pid1)(E/I)(ViW)
      Mapmem[14]= SetpointH
      Mapmem[15]= SetpointL
      Mapmem[16]=FLAG_START;
  */
    uchar m_StateVector[17];

    /**
     * Provided the commandChar it reads the SRAM of the device and computes if the last command was correctly executed.
     * It copies the SRAM in the state vector to keep it up to date
     * @param commandChar the last command issued to the device
     * @return true if the execution was correctly performed
     */
    bool CheckCommandExecution(uchar commandChar);
    
    
    bool m_IsOutputOn;
    
    bool m_FirstUpdate;
    
    bool m_ModuleInitOK;

    bool m_InvertTimer;

    unsigned int m_UpdateTime;
    //tempo ultimo aggiornamento
    unsigned int m_LastUpdateTime;

    void GenerateUpdateTime();

    bool WriteToDevice(uchar *mapMem);

    bool SendCommand(uchar command, uchar *newData);

    int m_PIDDIvider;
    
    bool m_OldProtocol;

};


#endif
