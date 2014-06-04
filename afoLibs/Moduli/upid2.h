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
#ifndef STDUPID2_H
#define STDUPID2_H

#include <vcontroller.h>
#include "ds2751.h"
#include "LibIniFile.h"
#include "IniFileHandler.h"
#include "xmlutil.h"

//Errore massimo di scarto tra il clock del cervelletto e quello dei moduli (sec.)
#define MAX_TIME_ERROR 60
//Intervallo di aggiornamento di data e ora
#define CLOCK_UPDATE_INTERVAL 3600

#define N_OF_DIV 9
extern const float listaDivisori[];

using namespace std;

/**
Class to manage the various UPID2 based devices

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CUPID2 : public CVController
{
    public:
        CUPID2(const char* configString, CTimer *timer = 0x0);

        ~CUPID2();
        
        bool VerifyIOPresence();

        virtual bool InitDevice( CIniFileHandler *iniFileHandler) = 0;

        bool SetVal(float val){return false;};

        //Aggiorna data e ora sul dispositivo target se la differenza tra
        //quella memorizzata e quella del cervelletto è superiore a MAX_TIME_ERROR
        bool UpdateDateTime(time_t newTime);

        bool UpdateCommonData();
        
        void SetInputDevice(CDS2751* theValue);
        CDS2751* GetDevice() const;

        bool m_InitOk;

         //Invia il comando al dispositivo e, se eseguito correttamente, mette in mapmem la risposta
        bool WriteToDevice(uchar *mapMem);

        bool CheckCRC(uchar *mapmem);
        unsigned char CalcCRC(uchar* mapmem);

        void ResetTimers(){
            m_LastUpdateTime = 0;
            m_LastClockUpdate = 0;
        }

     protected:
         CDS2751 *m_Device;

         //tempo ultimo aggiornamento
         unsigned int m_LastUpdateTime;
         //Tempo che intercorre tra due aggiornamenti
         unsigned int m_UpdateTime;
         //Tempo dell'ultimoi aggiornamento del clock
         time_t m_LastClockUpdate;

         void GenerateUpdateTime();

};


#endif
