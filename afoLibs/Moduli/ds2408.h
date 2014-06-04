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
#ifndef STDDS2408_H
#define STDDS2408_H

#include "vdevice.h"
#include "LibIniFile.h"
#include "cownet.h"

//using namespace std;
#define PIO_LOGIC_STATE 0x88
#define PIO_OUTPUT_LATCH_STATE 0x89
#define PIO_ACTIVITY_LATCH_STATE 0x8A
#define COND_SEARCH_CHANNEL_SEL 0x8B
#define COND_SEARCH_CHANNEL_POL 0x8C
#define CONTROL_REG 0x8D

/**
 * Riga configurazione:
 * NAME:DS2408,SN:,ACTIVITY:,TYPE,COMMENT
 * ACTIVITY: indica se deve registrare l'attività o meno
 * TYPE: -- serve al motore principale: se input o dido viene "pollato" ad ogni ciclo, se DO viene pollato ogni 5 secondi.
 *  0 - DI
 *  1 - DO
 *  2 - DIDO
	@author Alessandro Mirri <alessandro.mirri@Newtohm.it>
*/
class CDS2408 : public CVDevice 
{
public:
    CDS2408(int portNum, COWNET *master, const char* configString = NULL);

    ~CDS2408();
    
    /*
    * Questa funzione legge i tre registri dei PIO del dispositivo
    */
    bool ReadPIORegisters( uchar *state=0x0);
    
    bool UpdateData();
    
    //TODO commentare tutte le funzioni
    bool ChangeOutput(int channel);
    
    bool ClearActivityLatch();
    
    bool SetSwitch( uchar *state);

    bool ReadSwitch( uchar *state);

    bool SetResetMode(bool seton);

    bool GetVCC(uchar *reg);

    bool ClearPowerOnReset();

    bool OrConditionalSearch();

    bool AndConditionalSearch();

    bool SetPioConditionalSearch();

    bool SetActivityConditionalSearch();

    bool SetChannelMask( int channel, bool seton);

    bool SetChannelPolarity( int channel, bool seton);

    bool GetChannelMask( int channel);

    bool GetChannelPolarity( int channel, uchar *reg);

    bool SetRegister( uchar regAddr, uchar *reg);

    bool ReadRegister( uchar regAddr, uchar *reg);
    
    bool ReadRegister( uchar *reg);

    bool GetLatchState(int channel, uchar *state);

    bool GetLevel(int channel, uchar *state);

    bool GetSensedActivity (int channel, uchar *state);

    bool SetLatchState( int channel, uchar set);
    
    bool GetChannelState(int channel, bool updateFirst=true);
    
    bool GetChannelLevel(int channel, bool updateFirst=true);
    
    bool InitDevice();
    
    bool m_ActivateConditionalSearch;
    int m_PolarityMask;
    bool m_UseAndLogic;
    int m_ActivityMask;

    uchar m_Register[3];
    
    unsigned int m_ModuleType;
    
        
};



#endif
