/***************************************************************************
 *   Copyright (C) 2008 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it                                           *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 *************************************************************************/
#ifndef ACCESSLOG_H
#define ACCESSLOG_H

#include <iostream>
#include "commonDefinitions.h"
#include "LibIniFile.h"
#include "IniFileHandler.h"
#include "vcoordinator.h"

using namespace std;

/**
Class to manage the the access control devices such as MGC, ACC
 * configurazione
 * NAME:AccessCoord
 * FILENAME -- file ini in cui sono memorizzate le chiavi
 * KEY -- chiave nel file ini
 * SECTION -- sezione in cui memorizzare le chiavi

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CAccessLog : public CVCoordinator
{
    public:
//         CAccessLog(const char* fileName, const char* accessKey, const char* section);
//         CAccessLog(CString fileName, CString accessKey, CString section);
        CAccessLog(const char* configString, CTimer *timer);

        ~CAccessLog();

        bool Init();

        bool AddKeyData(CString name, CString snumStr, int addr, unsigned int expDateSec);
        bool RemoveKeyData(CString snumStr);

        bool ConnectControllers(){return true;};

        bool SetVal(float val){return false;};
        
        //TBM -- per ora è pubblico
        vector<t_AccessData> m_RegisteredPersonnelArray;

        //TBI funzioni da aggiungere (forse):
        //Rimozione di tutte le chiavi associate ad un nome

        CString m_SectionId, m_KeyId;

        CIniFileHandler m_IniFileHandler;

     protected:

         bool WriteOnFileAllRecords();
         bool EraseOnFileAllRecords();

         //Numero di chiavi da cercare
         int m_NofRecords;
         CString m_FileName;

         //TRova se l'accoppiata chiave-nome è già presente e ne ritorna l'indice
         //o l'iteratore. Torna -1 se la chiave non esiste
         int FindKey(CString snumStr, vector<t_AccessData>::iterator *It = 0x0);
};
#endif
