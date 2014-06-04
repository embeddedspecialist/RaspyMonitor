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
#include "accessLog.h"


// CAccessLog::CAccessLog ( const char* fileName, const char* accessKey, const char* section )
// {
//     m_FileName = fileName;
//     m_KeyId = accessKey;
//     m_SectionId = section;
//
// }
//
// CAccessLog::CAccessLog ( CString fileName, CString accessKey, CString section )
// {
//     m_FileName = fileName;
//     m_KeyId = accessKey;
//     m_SectionId = section;
// }

CAccessLog::CAccessLog(const char* configString, CTimer *timer) : CVCoordinator(configString)
{
    if (configString != 0x0)
    {
        m_IniLib.GetConfigParamString(configString,"FILENAME",&m_FileName,"./presenze.ini");
        m_IniLib.GetConfigParamString(configString,"SECTION",&m_FileName,"Personnel");
        m_IniLib.GetConfigParamString(configString,"KEY",&m_FileName,"PersKey");
    }

    m_ControllerType = DEV_ACCESSCOORD;

}

CAccessLog::~CAccessLog()
{
}

///////////////////////////////////////////////////
//              Init
///////////////////////////////////////////////////
bool CAccessLog::Init()
{
    t_AccessData tempAccessData;
    CLibIniFile iniParser;

    if ( !m_IniFileHandler.Load ( m_FileName ) )
    {
        //TBM
        cout << "ERRORE!! - Impossibile aprire il file:"<<m_FileName<<" di inizializzazione della sezione controllo accessi"<<endl;
        return false;
    }

    //carico le chiavi
    m_NofRecords = m_IniFileHandler.GetInt ( ACCESS_CNTRL_NOF_RECORDS, m_SectionId, 0 );

    for ( int keyIndex = 0; keyIndex < m_NofRecords; keyIndex++ )
    {
        CString recordKey = m_KeyId;
        CString recordData;
        recordKey+=keyIndex+1;

        recordData = m_IniFileHandler.GetString ( recordKey, m_SectionId, "" );

        if ( recordData.length() == 0 )
        {
            //TBM
            cout << "ERRORE!! Impossibile trovare il record numero "<< keyIndex+1<<" Nel file "<<m_FileName<<endl;
            return false;
        }

        iniParser.GetConfigParamString ( recordData.c_str(),"NAME",&tempAccessData.name,"NA" );
        iniParser.GetConfigParamString ( recordData.c_str(),"SN",&tempAccessData.keySN,"" );
        if ( tempAccessData.keySN.length() == 0 )
        {
            //TBM
            cout << "ERRORE!! Numero di serie NON trovato per record :"<<keyIndex+1<<" Nel file "<<m_FileName<<endl;
            return false;
        }

        iniParser.GetConfigParamInt ( recordData.c_str(),"EXP", ( int* ) ( &tempAccessData.expireDateSec ), 0 );
        iniParser.GetConfigParamInt ( recordData.c_str(),"ADDR", &(tempAccessData.roomNumber ), -1 );

        m_RegisteredPersonnelArray.push_back ( tempAccessData );
    }

    return true;
}

/////////////////////////////////////////////////////
//                  WriteOnFileAllRecords
/////////////////////////////////////////////////////
bool CAccessLog::WriteOnFileAllRecords()
{
    unsigned int recordIdx;
    bool retVal = true;

    //Ricarico il file 
    m_IniFileHandler.Reload();
    
    for ( recordIdx = 0; recordIdx < m_RegisteredPersonnelArray.size(); recordIdx++ )
    {
        CString recordKey = m_KeyId;
        CString recordData;

        recordKey+=recordIdx+1;

        recordData="NAME:";
        recordData+=m_RegisteredPersonnelArray.at ( recordIdx ).name;

        recordData+=",ADDR:";
        recordData+=m_RegisteredPersonnelArray.at ( recordIdx ).roomNumber;

        recordData+=",SN:";
        recordData+=m_RegisteredPersonnelArray.at ( recordIdx ).keySN;

        recordData+=",EXP:";
        recordData+=m_RegisteredPersonnelArray.at ( recordIdx ).expireDateSec;

        retVal = m_IniFileHandler.SetValue ( recordKey,recordData,"",m_SectionId ) && retVal;
    }

    m_IniFileHandler.SetInt ( ACCESS_CNTRL_NOF_RECORDS,m_RegisteredPersonnelArray.size(),"",m_SectionId );
    retVal = m_IniFileHandler.Save() && retVal;

    return retVal;

}

/////////////////////////////////////////////////////
//                  EraseOnFileAllRecords
/////////////////////////////////////////////////////
bool CAccessLog::EraseOnFileAllRecords()
{
    CString section = "Room";
    unsigned int recordIdx;
    bool retVal = true;

    m_IniFileHandler.Reload();
    
    for ( recordIdx = 0; recordIdx < m_RegisteredPersonnelArray.size(); recordIdx++ )
    {
        CString recordKey = m_KeyId;
        recordKey+=recordIdx+1;
        retVal = m_IniFileHandler.DeleteKey ( recordKey, m_SectionId ) && retVal;
    }

    m_IniFileHandler.SetInt ( ACCESS_CNTRL_NOF_RECORDS,0,"",m_SectionId );
    retVal = m_IniFileHandler.Save() && retVal;

    return retVal;
}

/////////////////////////////////////////////////////
//                  FindKey
/////////////////////////////////////////////////////
int CAccessLog::FindKey ( CString snumStr, vector<t_AccessData>::iterator *It )
{
    int retVal = -1;
    int keyIdx = -1;
    vector<t_AccessData>::iterator keyIt;

    for ( keyIt = m_RegisteredPersonnelArray.begin(); keyIt < m_RegisteredPersonnelArray.end(); keyIt++ )
    {
        keyIdx++;
        
        if ( keyIt->keySN == snumStr )
        {
            if (It != 0x00)
            {
                //Chiave trovata
                *It = keyIt;
            }

            retVal = keyIdx;
            break;

        }
    }

    //Controllo se ho trovato una chiave o meno
    if (keyIdx == (int)m_RegisteredPersonnelArray.size())
    {
        //Chiave NON trovata
        return -1;
    }
    else
    {
        return retVal;
    }
}

/////////////////////////////////////////////////////
//                  AddKeyData
/////////////////////////////////////////////////////
bool CAccessLog::AddKeyData ( CString name, CString snumStr, int addr, unsigned int expDateSec )
{
    bool retVal = false;
    CString keyEntry = m_KeyId;
    t_AccessData tempData;

    //TBI la gestione della scadenza
    //Controllo che la chiave non esista già
    if ( FindKey ( snumStr) < 0 )
    {
        EraseOnFileAllRecords();
        tempData.name = name;
        tempData.keySN = snumStr;
        tempData.expireDateSec = expDateSec;
        tempData.roomNumber = addr;
        m_RegisteredPersonnelArray.push_back ( tempData );
        //Aggiorno il file di configurazione
        WriteOnFileAllRecords();
        retVal = true;
    }

    return retVal;
}

/////////////////////////////////////////////////////
//                  RemoveKeyData
/////////////////////////////////////////////////////
bool CAccessLog::RemoveKeyData ( CString snumStr )
{
    bool retVal = false;
    CString keyEntry = m_KeyId;
    vector<t_AccessData>::iterator keyIt;

    if ( FindKey ( snumStr,&keyIt ) >=0 )
    {
        //Cancello tutte le chiavi
        EraseOnFileAllRecords();
        //Tolgo dal vettore la chiave
        m_RegisteredPersonnelArray.erase ( keyIt );
        WriteOnFileAllRecords();
        retVal = true;
    }

    return retVal;
}
