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
 #include "nthacc.h"
#include "conewireengine.h"
#include "Cmd.h"

#define CHECK_VIRTUAL_FALSE() if (m_IsVirtual) return false;

CNTH_ACC::CNTH_ACC(const char* configString, CTimer *timer)
    : CUPID2(configString, timer)
{
    CLibIniFile iniFileParser;

    if (configString != 0x0){
        iniFileParser.GetConfigParamInt(configString,"IDCAT",(int*)(&m_IDCategory),ALL);

        m_IniLib.GetConfigParamInt( configString, "TIMERID", &m_TimerID, -1);

        m_IniLib.GetConfigParamString(configString, "INIFILENAME", &m_IniFileName,"");

        m_IniLib.GetConfigParamString(configString, "SECTION", &m_DataSection,"");
        m_IniLib.GetConfigParamString(configString, "KEY", &m_DataKey,"");
        
        m_IniLib.GetConfigParamInt( configString, "REMOTEADDR", &m_RemoteAddr, -1);

        if ( (m_TimerID > 0) && (timer != 0x0) )
        {
            m_Timer = timer;
            m_UseTimer = true;
        }
        else
        {
            m_UseTimer = false;
        }
    }

    m_IsVirtual = false;
    m_ControllerType = DEV_ACC;
    m_TypeOfTimerVal = TIMERVAL_DIGITAL;
}


CNTH_ACC::~CNTH_ACC()
{
}
////////////////////////////////////////////////////
//            Update
////////////////////////////////////////////////////
bool CNTH_ACC::Update(bool updatefirst)
{
    return true;
}

////////////////////////////////////////////////////
//            AddKey
////////////////////////////////////////////////////
bool CNTH_ACC::AddKey(uchar * SNum, unsigned int expDateSec, int channel)
{
    CHECK_VIRTUAL_FALSE()
            
    uchar mapMem[16];

    //TBI la gestione del canale
    memset (mapMem, 0xff, 16*sizeof(uchar));

    mapMem[0] = 0x03;
    mapMem[1] = 0x02; //Aggiungi chiave
    memcpy (&(mapMem[2]), &expDateSec, 4);
    memcpy (&(mapMem[6]), SNum, 8);
    mapMem[14] = 2;//Aggiungo ad entrambi i canali TBM
    mapMem[15] = CalcCRC(mapMem);

    return WriteToDevice(mapMem);
}

//////////////////////////////////////////////////////
bool CNTH_ACC::AddKey(t_AccessData newKey)
{
    bool retVal = false;
    uchar hexSerNum[16];

    memset (hexSerNum, 0x0, 16);

    ConvertSN2Hex(newKey.keySN.c_str(), hexSerNum);

    if (!m_IsVirtual)
    {
        if (AddKey(hexSerNum, newKey.expireDateSec, newKey.channel))
        {
            m_RegisteredKeysArray.push_back(newKey);
            EraseKeysFromFile();
            WriteKeysToFile();
            retVal = true;
        }
    }
    else
    {
        m_RegisteredKeysArray.push_back(newKey);
        EraseKeysFromFile();
        WriteKeysToFile();
        retVal = true;
    }

    return retVal;
}

////////////////////////////////////////////////////
//            InitDevice
////////////////////////////////////////////////////
bool CNTH_ACC::InitDevice()
{
    CString recordKey;
    int nOfKeys;
    CString recordString;
    CString serNum;
    CString name;
    uchar hexSerNum[8];
    unsigned int expDateSec;
    CLibIniFile libIni;
    t_AccessData newAccessData;

    if (!m_IniFileHandler.Load(m_IniFileName.c_str()))
    {
        return false;
    }
    
    nOfKeys = m_IniFileHandler.GetInt("NofRecords", m_DataSection, 0);

    //Cancello tutte le chiavi presenti
    if ( (!m_IsVirtual) && (!ClearDevice()) )
    {
            //TBI messaggi errore
        cout << "Errore nel modulo di controllo accessi: "<<m_NetNumber<<" -- "<<m_DeviceNumber << "Impossibile cancellare le chiavi"<<endl;
        return false;
    }

    for (int i = 0; i < nOfKeys; i++)
    {
        recordKey = m_DataKey;
        recordKey +=(i+1);

        recordString = m_IniFileHandler.GetString(recordKey,m_DataSection,"");
        if (recordString.length() == 0)
        {
            //TBI messaggi di errore

            cout << "Errore nel modulo di controllo accessi: "<<m_NetNumber<<" -- "<<m_DeviceNumber << "Manca la chiave numero: "<<i+1<<endl;
            return false;
        }

        //TBI -- Controllo della categoria
        libIni.GetConfigParamString(recordString.c_str(),"SN",&serNum,"");
        if (serNum.length() == 0)
        {
            //TBI messaggi errore
            cout << "Errore nel modulo di controllo accessi: "<<m_NetNumber<<" -- "<<m_DeviceNumber << "Manca il numero di serie nella riga numero: "<<i+1<<endl;
            return false;
        }

        libIni.GetConfigParamInt(recordString.c_str(),"EXP",(int*)(&expDateSec),0);
        if (expDateSec == 0)
        {
            cout << "Attenzione!! Nel modulo di controllo accessi: "<<m_NetNumber<<" -- "<<m_DeviceNumber << "Manca la scadenza della chiave numero: "<<i+1<<endl;
        }

        libIni.GetConfigParamString(recordString.c_str(),"NAME",&name,"NA");
        if (name == "NA")
        {
            cout << "Attenzione!! Nel modulo di controllo accessi: "<<m_NetNumber<<" -- "<<m_DeviceNumber << "Manca il nome associato alla chiave numero: "<<i+1<<endl;
        }

        newAccessData.name = name;
        newAccessData.keySN = serNum;
        newAccessData.expireDateSec = expDateSec;
                
        //Converto il numero di serie
        ConvertSN2Hex(serNum.c_str(), hexSerNum);

        if ( (!m_IsVirtual) && (!AddKey(hexSerNum,expDateSec,2)) )
        {
            //TBI messaggi errore
            cout << "Errore nel modulo di controllo accessi: "<<m_NetNumber<<" -- "<<m_DeviceNumber << "Impossibile impostare chiave"<<i+1<<endl;
            return false;
        }
        else
        {
            m_RegisteredKeysArray.push_back(newAccessData);
        }
    }

    if ( (!m_IsVirtual) && (!UpdateCommonData()) )
    {
        //TBI messaggi errore
        cout << "Errore nel modulo di controllo accessi: "<<m_NetNumber<<" -- "<<m_DeviceNumber << "Impossibile aggiornare orario"<<endl;
        return false;
    }

    m_InitOk = true;

    return true;
}

////////////////////////////////////////////////////
//            ClearDevice
////////////////////////////////////////////////////
bool CNTH_ACC::ClearDevice()
{
    uchar mapMem[16];
    
    if (!m_IsVirtual)
    {
        memset(mapMem, 0xff, 16*sizeof(uchar));
    
        mapMem[0] = 0x03;
        mapMem[1] = 0x08 | 0x10;
        mapMem[15] = CalcCRC(mapMem);
    
        return WriteToDevice(mapMem);
    }
    else
    {
        m_RegisteredKeysArray.clear();
        return true;
    }
}

////////////////////////////////////////////////////
//            RemoveKey
////////////////////////////////////////////////////
bool CNTH_ACC::RemoveKey(uchar *SNum, int channel){
    
    CHECK_VIRTUAL_FALSE()
            
    uchar mapMem[16];

    memset(mapMem, 0xFF, 16*sizeof(uchar));

    mapMem[0] = 0x03;
    mapMem[1] = 0x04; //Cancellazione chiave

    memcpy (&mapMem[6], SNum, 8);
    mapMem[14] = (uchar)channel;
    mapMem[15] = CalcCRC(mapMem);

    return WriteToDevice(mapMem);

}

//////////////////////////////////////////////////////////
bool CNTH_ACC::RemoveKey(t_AccessData keyToRemove)
{
    bool retVal = false;
    uchar hexSerNum[16];
    vector<t_AccessData>::iterator keyIt;
    

    memset (hexSerNum, 0x0, 16);
    
    ConvertSN2Hex(keyToRemove.keySN.c_str(), hexSerNum);

    if (!m_IsVirtual)
    {
        if (RemoveKey(hexSerNum, keyToRemove.channel))
        {
            for (keyIt = m_RegisteredKeysArray.begin(); keyIt < m_RegisteredKeysArray.end(); keyIt++)
            {
                if (keyIt->keySN == keyToRemove.keySN)
                {
                    m_RegisteredKeysArray.erase(keyIt);
                    break;
                }
            }
    
            EraseKeysFromFile();
            WriteKeysToFile();
            retVal = true;
        }
    }
    else
    {
        for (keyIt = m_RegisteredKeysArray.begin(); keyIt < m_RegisteredKeysArray.end(); keyIt++)
        {
            if (keyIt->keySN == keyToRemove.keySN)
            {
                m_RegisteredKeysArray.erase(keyIt);
                break;
            }
        }
    
        EraseKeysFromFile();
        WriteKeysToFile();
        retVal = true;
    }

   
    
    return retVal;
}

////////////////////////////////////////////////////
//            EraseKeysFromFile
////////////////////////////////////////////////////
bool CNTH_ACC::EraseKeysFromFile()
{
    unsigned int recordIdx;
    bool retVal = true;

    m_IniFileHandler.Reload();
    
    for ( recordIdx = 0; recordIdx < m_RegisteredKeysArray.size(); recordIdx++ )
    {
        CString recordKey = m_DataKey;
        recordKey += recordIdx+1;
        retVal = m_IniFileHandler.DeleteKey ( recordKey, m_DataSection ) && retVal;
    }

    m_IniFileHandler.SetInt ( "NofRecords", 0, "", m_DataSection );
    retVal = m_IniFileHandler.Save() && retVal;

    return retVal;
}
////////////////////////////////////////////////////
//            WriteKeysToFile
////////////////////////////////////////////////////
bool CNTH_ACC::WriteKeysToFile()
{
    unsigned int recordIdx;
    CString recordData;
    bool retVal = true;

    //Ricarico il file 
    m_IniFileHandler.Reload();
    
    for ( recordIdx = 0; recordIdx < m_RegisteredKeysArray.size(); recordIdx++ )
    {
        CString recordKey = m_DataKey;

        recordKey+=recordIdx+1;

        recordData="NAME:";
        recordData+=m_RegisteredKeysArray.at ( recordIdx ).name;

        recordData+=",SN:";
        recordData+=m_RegisteredKeysArray.at ( recordIdx ).keySN;

        recordData+=",EXP:";
        recordData+=m_RegisteredKeysArray.at ( recordIdx ).expireDateSec;

        //TBI
//         recordData+=", CHANNEL:";
//         recordData+=m_RegisteredKeysArray.at (recordIdx ).channel;

        retVal = m_IniFileHandler.SetValue ( recordKey,recordData,"",m_DataSection ) && retVal;
    }

    m_IniFileHandler.SetInt ( "NofRecords",m_RegisteredKeysArray.size(),"",m_DataSection );
    retVal = m_IniFileHandler.Save() && retVal;

    return retVal;
    
}

bool CNTH_ACC::SendInfoToRemoteAcc(bool addKey, t_AccessData keyData)
{
    COneWireEngine *engine = reinterpret_cast<COneWireEngine*> ( m_EnginePtr );
    char checkOutDate[32];
    
    memset (checkOutDate, 0x0, 32);
    
     //Informo eventuali schede remote
    if (m_RemoteAddr >= 0) {
        //Qui c'è un problema....
        if (engine == 0x0) {
            return false;
        }
        
        Cmd com("COMMAND");
        com.putValue("TYPE", "SetButtonCode");
        com.putValue("ADDRESS", CString("")+m_RemoteAddr);
        if (addKey) {
            com.putValue("REMOVE","0");
        }
        else {
            com.putValue("REMOVE", "1");
        }
        com.putValue("NAME",keyData.name);
        com.putValue("SERNUM",keyData.keySN); 
        //TODO da sistemare la gestione del canale
        com.putValue("CHANNEL",CString("")+2);
        ConvertSecs2DateStr(keyData.expireDateSec, checkOutDate, 32);
        com.putValue("CHECKOUT",CString("")+checkOutDate);
        com.putValue("RELAYMSG","1");//Identifica che il messaggio e' stato inviato da un acc e' non e' arrivato da un'interfaccia
        
        engine->WriteOnOutputPorts(com.getXMLValue(), m_RemoteAddr);
    }
    
    return true;
}






