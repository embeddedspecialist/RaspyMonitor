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
 #ifndef STDNTHACC_H
#define STDNTHACC_H

#include <upid2.h>

using namespace std;

/**
Class that encapsulates the functions for access control

Configuration:
NAME:NTHACC, INPUT, SECTION, INIFILENAME, KEY, TIMERID, COMMENT
 * 
 * Se INPUT == VIRTUAL il controllo si limita a gestire il file ini senza interfacciarsi con un modulo in campo
 * Se aggiungo il campo REMOTE il modulo invia i comandi di deregsitrazione ad un'altra scheda remota
 * Mi serve in quelle situazioni tipo bedazzo dove uno dei cervelletti NON ha il modulo di controllo accessi ma voglio mantenere la coerenza
 * dei file dei passpartout

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CNTH_ACC : public CUPID2
{
public:
    CNTH_ACC(const char* configString, CTimer *timer = 0x0);

    ~CNTH_ACC();

    bool Update(bool updatefirst);
    bool Update2(bool updateData){return false;};
    
    CString GetSpontaneousData(int lParam = 0) { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};

    bool InitDevice(CIniFileHandler *iniFileHandler){ return true;};

    bool AddKey(uchar* SNum, unsigned int expDateSec, int channel);
    bool AddKey(t_AccessData newKey);
    
    bool RemoveKey(uchar *SNum, int channel);
    bool RemoveKey(t_AccessData keyToRemove);
    
    bool ClearDevice();

    bool InitDevice();

    void setIDCategory ( const e_IDcategories& theValue )
    {
        m_IDCategory = theValue;
    }


    e_IDcategories getIDCategory() const
    {
        return m_IDCategory;
    }

    vector< t_AccessData > getRegisteredKeysArray() const
    {
        return m_RegisteredKeysArray;
    }

    CString m_IniFileName;
    
    bool m_IsVirtual;

    vector<t_AccessData> m_RegisteredKeysArray;
    
    bool SendInfoToRemoteAcc(bool addKey, t_AccessData keyData);
    
private:
    
    int m_RemoteAddr;
    
    //Chiave e sezione del file ini in cui cercare i dati
    CString m_DataSection, m_DataKey;

    //Indica quali categorie di chiavi possono accedere a questo dispositivo
    //Maggio 2008 -- Nella prima implementazione NON viene gestito, sarà gestito a livello di engine
    e_IDcategories m_IDCategory;

    

    CIniFileHandler m_IniFileHandler;

    bool EraseKeysFromFile();
    bool WriteKeysToFile();


};

#endif
