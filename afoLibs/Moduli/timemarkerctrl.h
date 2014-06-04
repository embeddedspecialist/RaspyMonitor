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
 #ifndef STDTIMEMARKERCTRL_H
#define STDTIMEMARKERCTRL_H

#include <upid2.h>
#include <iostream>
#include <fstream>
#include "timeUtil.h"
#include "IniFileHandler.h"
#include "fileUtil.h"

#define COPY_INTERVAL 3600

#define KEY_NAME "Key"

#define MARK_IN 1
#define MARK_OUT 2

typedef struct {
    CString name;
    CString serialNum;
}t_TM_KeyData;

typedef struct KeyMark {
    KeyMark(){ markTime = 0; typeOfMark = 0;};
    t_TM_KeyData keyData;
    unsigned int markTime;         //Orario di registrazione in secondi
    unsigned int typeOfMark;       //0 invalid, 1 ingresso, 2 uscita
}t_KeyMark;

using namespace std;

/**
Classe che gestisce una uPID2 programmata per l'utilizzo come un marcatempo.
 * La classe si appoggia sul file esterno timeMark.ini per la propria configurazione e scrive nel file fornito da riga di comando. Usa il file markerIDPresence.ini per segnarsi chi è dentro e chi è fuori e ricostuire la situazione in caso di failures; entrambi i file sono salvati ogni ora in caso di passaggi sulla flash interna ma risiedono in RAM
 * Il file viene creato in RAM (cartella var/empty) e salvato ogni ora sul disco interno se ci sono delle variazioni dall'ultimo salvataggio
 * NAME:TimeMarker,ADDR:,INPUT:,FILE:,ID:,CHANNEL:
 * Dove:
 * FILE -   e' il nome del file di salvataggio
 * ID   -   e' il riferimento all'interno del file di configurazione che serve solo per il caricamento delle chiavi
 *          in caso di crash/riavvio
 * CHANNEL: sono i canali da usare: 1, 2 o 3 per entrambi (default:1)
 *
 * TODO la gestione dei canali: al momento us solo il canale 1
 * TODO gestione dimensione file per avere una finestra di almeno 30gg
 * 
	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CTimeMarkerCtrl : public CUPID2
{
public:
    CTimeMarkerCtrl(const char* configString, CTimer *timer);

    ~CTimeMarkerCtrl();

    bool Update(bool updateData);
    bool Update2(bool updateData){return false;};
    
    CString GetSpontaneousData(int lParam = 0) { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};

    bool InitDevice(CIniFileHandler* inifile=0x0);

    bool SaveKeyMarkData(t_KeyMark keyMark);
    void CopyKeyMarkFileToDisk();

    bool AddKey(CString name, CString serialNum);
    bool RemoveKey(CString serialNum);
    bool RemoveAllKeys();

    bool WriteOnFileValidKeys();
    bool EraseOnFileValidKeys();

    int FindRegisteredKey(CString serNumStr, vector<t_TM_KeyData>::iterator *destIt = 0x0);
    int GetTypeOfMark(t_TM_KeyData keyData);

    bool SaveKeyFile();

    CString m_FileName;
    int m_ID;

    //30/10/2008 -- Eliminato perchè inutile
//     //Contiene tutti i passaggi della giornata, a mezzanotte viene resettato
//     vector<t_KeyMark> m_DayMarks; 
    //E' l'elenco delle chiavi valide
    vector<t_TM_KeyData> m_ValidKeys;
    //E' l'elenco delle chiavi attualmente registrate come presenti: mi serve per registrare correttamente i passaggi allo scadere della mezzanotte
    vector<t_TM_KeyData> m_Presences;

    CIniFileHandler *m_TimeMarkIniHndlr;
    CIniFileHandler *m_PresenceIniHndlr;
    
    unsigned int m_LastCopyFileTimeSec;
    bool m_ScheduleFileCopy;
    CString m_PresenceFileName;
    //Percorso del file temporaneo
    CString m_TempFilePath;
    //Percorso del file fisso (./di default)
    CString m_LocalFilePath;

    int m_MaxDimFile;

    private:
        bool SaveAllKeysToIniFile(CIniFileHandler *iniFile, vector<t_TM_KeyData> *keyList);
        bool EraseAllKeysFromIniFile(CIniFileHandler *iniFile, vector<t_TM_KeyData> *keyList);
    
};

#endif
