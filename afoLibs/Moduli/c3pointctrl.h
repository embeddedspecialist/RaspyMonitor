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
 #ifndef STDC3POINTCTRL_H
#define STDC3POINTCTRL_H

#include <vcontroller.h>
#include "temperaturecontroller.h"
#include "digitalio.h"

using namespace std;

typedef enum C3PointStatus
{
    STOPPED,
    OPENING,
    CLOSING
} e_C3PointStatus;

/**
Classe che implementa un controllo di temperatura su valvola a 3 punti. Di fatto la suddetta valvola e' un dispositivo integratore quindi mi basta controllare l'errore tra setpoint e temperatura da mantenere e, in base al segno e alla stagione, vedere se devo aprire o chiudere la valvola. Esistea anche un parametro di "insensibilita'" pari ad 1 grado di default.
E' possibile farlo funzionare anche come LMD, in questo caso al raggiungimento della soglia di temperatura minima o massima l'algoritmo lmd interviene
e agisce in maniera prioritaria nel senso opposto a quanto chiede l'algoritmo diretto.
 * File ini
 * NAME:3PointCtrl, ADDR:, INPUT:, OPEN:,CLOSE:,SUMMER:,SETPOINT:,NULLZONE:,MOVETIMEOUT:,LMD:,SPH,SPL,COMMENT:,TIMERID:
 * Dove:
 * INPUT        -- E' tempCtrl (o remote se viene impostato da qualche altra classe
 * OPEN e CLOSE -- sono digitalio
 * SETPOINT     -- e' il setpint di temperatura
 * SUMMER       -- Indicazione state/inverno
 * MOVETIMEOUT  -- Tempo massimo per andare da tutto chiuso a tutto aperto (default 5 minuti)
 * NULLZONE     -- ampiezza zone "morta" per il calcolo dell'integrazione
 * LMD          -- Se 1 richiede un'ulteriore sonda di temperatura e due altri setpoint per il limite di mandata
 * SPH, SPL     -- Setpoint alto e basso di temperatura
 * INPUT2       -- Sonda di temperatura per limite di mandata (o remote se viene impostato da qualche altra classe)

    @author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class C3PointCtrl : public CVController
{
public:
    C3PointCtrl(const char* configString, CTimer * timer );

    ~C3PointCtrl();

    float m_Setpoint;
    float m_SetpointH, m_SetpointL;
    t_PidVar m_Temperature;
    t_PidVar m_LMDTemperature;
    float m_NullZoneAmplitude;

    //Flag che indicano che la valvola è, rispettivamente, tutta aperta e tutta chiusa
    bool m_IsFullOpen, m_IsFullClosed;

    //Stato del controllo
    e_C3PointStatus m_ControllerStatus;

    int m_TimeOfOpening, m_TimeOfClosing; //Flag che indica l'istante in cui inizio a muovere la valvola
    int m_MovementTimeOut;
    bool m_IsSummer;

    bool m_IsLMD;

    CTempCtrl* m_TempCtrl;
    CTempCtrl* m_TempLMDCtrl;
    CDigitalIO* m_OpenCtrl;
    CDigitalIO* m_CloseCtrl;
    void* m_NetPtr;

    bool SetInputDevice (CTempCtrl* tempCtrl, bool lmdDevice);

    bool SetOpenCloseDevices(CDigitalIO* openDevice, CDigitalIO* closeDevice);

    bool Update(bool updateData);
    bool Update2(bool updateData);
    bool SetVal(float val){return false;};
    
    CString GetSpontaneousData(int lParam = 0) { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};

    void SetSummer(bool isSummer){m_IsSummer = isSummer;};
    void SetSetpoint(float newSetpoint){m_Setpoint = newSetpoint; SaveConfigParam("SETPOINT",CString("")+m_Setpoint);};
    void SetLMDSetpoint(float newSPH, float newSPL);

    bool ConnectControllers(int netIndex, const char* configString);

    bool SetTemperature(float newTemp, bool isLmd);

    bool VerifyIOPresence();

    bool StopValve();

    private:
        /**
         * Funzione usata in caso di controllo semplice
         * @param updateData
         * @return
         */
        bool UpdateSimpleControl(bool updateData);
        /**
         * Funzione usata in caso di controllo LMD
         * @param updateData
         * @return
         */
        bool UpdateLMDControl (bool updateData);

        float GetError(float temp, float setpoint, bool isLmd);
        void GetTemperatures();
        void GetTemperatures_2();

        bool OpenValve();
        bool CloseValve();
};

#endif
