/***************************************************************************
 *   Copyright (C) 2008 by Alessandro Mirri   *
 *   alesssandro.mirri@newtohm.it   *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it
 *   and/or modify  it in any way                                          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY; without even the    *
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR       *
 *   PURPOSE.                                                              *
 ***************************************************************************/
#ifndef STDNTHMGC_H
#define STDNTHMGC_H

#include <stdlib.h>
#include <upid2.h>
#include "nthCnt.h"

#define MAX_NUM_KEYS 32

typedef struct {
    uchar keySN[8];
    int expireDateSec;
}t_MGC_KeyData;



typedef enum HConfigVars
{
    ROOM_FREE,
    REMOTE_ON,
    GUEST_NAME,
    NOF_G_KEYS,
    AIR_COND,
    PID,
    HOT_WATER,
    COLD_WATER,
    WATT,
    CALORIES,
    FRIGORIES,
    CHECKIN_DATE,
} e_HConfigVars;

/**
 * Classe di gestione dei moduli camera NTH-AFO-MGC: il modulo gestisce sia i servizi di camera che un eventuale
 * contabilizzatore associato (tipo Bedazzo). Se è presente il campo Input2 questo indica la presenza del contaenergia
 * Riga di configurazione:
 * NAME:NTH-MGC, ROOM, INPUT1, INPUT2, COMMENT
 * Dove:
 * ROOM = numero della camera, serve per associarla nel file rooms.ini ai dati di stanza
 * INPUT1 = driver del modulo servizi di camera
 * INPUT2 = controller del modulo contabilizzatore (opzionale)
 * FILENAME = nome del file ini (default presenze.ini)
 * COMMENT = commento libero
 * Se e' presente il modulo contabilizzatore questo viene gestito all'esterno e la classe si limita a ricrearne un altro sullo stesso
 * device al fine di raccogliere tutti i dati della stanza
 * @author Alessandro Mirri <alessandro.mirri@newtohm.it>
 */
class CNTHMGC : public CUPID2 {
public:
    CNTHMGC(const char *configString,  CTimer *timer = 0x0);

    ~CNTHMGC();

    bool Update(bool updatefirst);
    bool Update2(bool updateData){return false;};
    
    CString GetSpontaneousData(int lParam = 0) { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};

    bool CheckInKey(CString name, uchar* SNum,unsigned int checkInDate,unsigned int expDateSec);
    bool CheckOutKey(uchar *SNum);
    bool CheckOutRoom();


    //Copia in un array di 4*5 i valori di tutti i contatori
    bool GetCounterReadings(float *destArray);

    int GetRoomNumber();

    //Legge attraverso la classe passata i suoi parametri da file
    //e riprogramma il modulo nth-afo-mgc associato secondo i dati
    //Si copia anche in locale il puntatore al file per gestire le variazioni
    bool InitDevice(CIniFileHandler *iniFileHandler = 0x0);

    bool SetRoomSetPoint(float newSP);
    bool TurnOnRoom(bool turnOn);
    bool SetAirCondEnable(bool enable);
    bool SetSummer(bool isSummer);
    bool SetPIDParam(float *param, int pidDivider);
    bool GetPIDParam(float *dest, int *pidDivider);

    bool m_HasCounters;
    int m_RoomNumber;
    bool m_IsRoomFree;
    CString m_GuestName;
    bool m_IsRoomOn;
    bool m_IsAirCondEnabled;

    float m_RoomTemp;
    float m_RoomSetPoint;

    //Tipo di presenza in camera: 0=nessuno, 1=ospite, 2=personale
    int m_PresenceType;
    //Valori di lettura all'atto del check in: acqua calda, fredda, watt, frigorie, calorie
    float m_CheckInCounters[5];
    CNTH_CNT *m_RoomCounter;

    bool m_PIDIsSummer;

    vector<t_MGC_KeyData> m_GuestArray;
    vector<t_MGC_KeyData> m_PersArray;

    bool ProgramRomCode(uchar *romCode, bool isGuest, unsigned int expDateSec=0);


    /**
     * Erases the given key code from the board and from the internal key vector
     * @param romCode code of the key to remove
     * @param isGuest flag that indicates if the key is guest or personnel
     * @param  *keyIndex optional to have the index of the element deleted in the vector of the keys
     * @return true if operation successfull
     */
    bool EraseRomCode(uchar *romCode, bool isGuest, int* keyIndex=0x0);

    unsigned int m_CheckInDateSec;
    unsigned int m_CheckOutDateSec;

    
    CString m_IniFileName;

private:

    bool m_CounterInitOk;

    bool ReadCounters();
    //Parametri del PID
    int m_PidParameters[3];
    int m_PIDDivider;


    //Cerca una chiave nell'array delle chiavi, torna -1 se la chiave non è stata
    //trovata o il suo indice.
    int FindKey(uchar *SN, bool isGuest, vector<t_MGC_KeyData>::iterator *retIt = 0);

    bool ProgramPID();

    bool WriteOnFileGuestKeys();
    bool EraseOnFileGuestKeys();
};


#endif
