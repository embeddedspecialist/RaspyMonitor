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
 #include "timemarkerctrl.h"

CTimeMarkerCtrl::CTimeMarkerCtrl(const char* configString, CTimer *timer):CUPID2(configString,timer)
{
    if (configString != 0x0)
    {
        m_IniLib.GetConfigParamString(configString, "FILE",&m_FileName,"");
        m_IniLib.GetConfigParamInt(configString,"MAXDIMFILE",&m_MaxDimFile, 300000);
        m_IniLib.GetConfigParamInt(configString,"ID",&m_ID,-1);
    }

    m_LastCopyFileTimeSec = 0;
    m_ScheduleFileCopy = false;
    m_PresenceIniHndlr = 0x0;
    m_TimeMarkIniHndlr = 0x0;
    m_ControllerType=DEV_TIMEMARKER;

#ifdef CRIS
    m_TempFilePath="/var/";
#else
    m_TempFilePath="/home/amirrix/";
#endif

    m_LocalFilePath="./";
   
}


CTimeMarkerCtrl::~CTimeMarkerCtrl()
{
    if (m_PresenceIniHndlr != 0x0)
    {
        delete m_PresenceIniHndlr;
        m_PresenceIniHndlr = 0x0;
    }

    if (m_TimeMarkIniHndlr != 0x0)
    {
        delete m_TimeMarkIniHndlr;
        m_TimeMarkIniHndlr = 0x0;
    }
}

///////////////////////////////////////////////////////
//Update
///////////////////////////////////////////////////////
bool CTimeMarkerCtrl::Update(bool updateData)
{
    bool retVal = false;
    uchar mapmem[16];
    int channel = 0; // canale 1
    unsigned int markTimeSec;
    uchar SNum[8];
    char SNumString[255];
    int keyIndex = -1;
    uchar nullKey[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

    //TODO aggiungere gestione flag m_InitOK

    //Aggiornamento orario
    UpdateCommonData();
    
    t_KeyMark newKeyMark;

    memset (mapmem, 0xFF, 16);
    memset (SNumString, 0x0, 255);

    //Chiedo una chiave alla scheda
    mapmem[0] = 0x04;
    mapmem[1] = (uchar)channel;
    
    mapmem[15] = CalcCRC(mapmem);
    if (WriteToDevice(mapmem))
    {
        if (memcmp(&(mapmem[5]),nullKey,8))
        {
            //Ho ricevuto un numero valido
            memcpy(SNum, &(mapmem[5]),8);
            ConvertSN2Str(SNumString,SNum);

            //Lo cerco nell'array delle chiavi registrate
            keyIndex = FindRegisteredKey(SNumString);

            if (keyIndex >= 0)
            {
                //Chiave riconosciuta
                newKeyMark.keyData = m_ValidKeys[keyIndex];

                newKeyMark.typeOfMark = GetTypeOfMark(m_ValidKeys[keyIndex]);
                memcpy (&markTimeSec, &mapmem[1],4);
                newKeyMark.markTime = markTimeSec;

//                 m_DayMarks.push_back(newKeyMark);
                m_ScheduleFileCopy = true;

                //TODO gestione errori
                //Salvo le chiavi nel file presenze:
                EraseAllKeysFromIniFile(m_PresenceIniHndlr, &m_Presences);
                SaveAllKeysToIniFile(m_PresenceIniHndlr, &m_Presences);

                //Salvo i passaggi
                SaveKeyMarkData(newKeyMark);
                
            }
            else
            {
                //La scheda ha registrato una chiave che il sistema NON conosce: c'è qualcosa di grave nella cosa
                //perche' vuol dire che sistema e scheda sono disallineati
                //TODO devo fare in modo che il fatto sia registrato senza perdere la chiave
            }
        }
        else {
            //Ho ricevuto solo 0xFF: non sono state registrate chiavi dall'ultima volta
            retVal = true;
        }
    }
    else
    {
        //TODO errore
    }

    CopyKeyMarkFileToDisk();



    return retVal;
}

///////////////////////////////////////////////////////////////
//                  FindRegisteredKey
//////////////////////////////////////////////////////////////
int CTimeMarkerCtrl::FindRegisteredKey(CString serNumStr, vector<t_TM_KeyData>::iterator *destIt)
{
    int keyIndex = 0;
    vector<t_TM_KeyData>::iterator keyIt;
    bool keyFound = false;

    for (keyIt = m_ValidKeys.begin(); keyIt < m_ValidKeys.end(); keyIt++)
    {
        
        if (serNumStr == keyIt->serialNum)
        {
            //Chiave trovata
            keyFound = true;
            if (destIt != 0x0)
            {
                *destIt = keyIt;
            }
            break;
        }

        keyIndex++;
    }

    if (keyFound)
    {
        return keyIndex;
    }
    else
    {
        return -1;
    }
    
}

///////////////////////////////////////////////////////////////
//                  GetTypeOfMark
//////////////////////////////////////////////////////////////
int CTimeMarkerCtrl::GetTypeOfMark(t_TM_KeyData keyData)
{
    int typeOfMark = MARK_IN;
    vector<t_TM_KeyData>::iterator presencesIt;
    bool keyFound = false;

    //Scansiono tutto l'array delle presenze per controllare se la chiave era gia' all'interno e registrare correttamente il passaggio
    for (presencesIt = m_Presences.begin(); presencesIt < m_Presences.end(); presencesIt++)
    {
        if (keyData.serialNum == (*presencesIt).serialNum)
        {
            keyFound = true;
            break;
        }
    }

    if (keyFound)
    {
        //Chiave trovata --> segno il passaggio come uscita
        typeOfMark = MARK_OUT;
        //Rimuovo la chiave dalle presenze
        m_Presences.erase(presencesIt);
    }
    else
    {
        //e' un ingresso --> lo registro come presente all'interno
        m_Presences.push_back(keyData);
    }



    return typeOfMark;
}

//////////////////////////////////////////////////////////////
//                  CopyKeyMarkFileToDisk
//////////////////////////////////////////////////////////////
void CTimeMarkerCtrl::CopyKeyMarkFileToDisk()
{
    CString command,commandPres;
    time_t actTime;

    actTime = time(NULL);
    
    if ((m_ScheduleFileCopy) && (actTime > m_LastCopyFileTimeSec + COPY_INTERVAL))
    {
        command = "rm -f ";
        command += m_LocalFilePath;
        commandPres = command;
        
        command +=m_FileName;
        commandPres+=m_PresenceFileName;

        //TODO forse da inserire un controllo sull'errore
        system (command.c_str());
        system(commandPres.c_str());

        command = "cp ";
        command += m_TempFilePath;
        commandPres = command;
        
        command +=m_FileName;
        commandPres+=m_PresenceFileName;

        command += " ";
        command += m_LocalFilePath;

        commandPres += " ";
        commandPres += m_LocalFilePath;


        if ( (system(command.c_str()) > -1) && (system(commandPres.c_str()) > -1) )
        {
            m_LastCopyFileTimeSec = (unsigned int)(actTime);
            m_ScheduleFileCopy = false;
        }
    }
}
//////////////////////////////////////////////////////////////
//                  InitDevice
//////////////////////////////////////////////////////////////
bool CTimeMarkerCtrl::InitDevice(CIniFileHandler* iniFile)
{
    int nOfKeys=0;
    CString sectionName="MARKER";
    CString keyName;
    CString keyString;
    ifstream inp;
    ofstream out;
    CString fileNameAndPath;
    CString command;
    uchar mapmem[16];
    time_t actTime;

    time(&actTime);

    if (m_ID <= 0)
    {
        cout << "Errore manca l'ID nel marcatempo "<<m_NetNumber<<":"<<m_DeviceNumber<<endl;
        return false;
    }

    UpdateDateTime(actTime);

    //Cancello tutte le chiavi
    memset (mapmem, 0xFF, 16);

    mapmem[0] = 0x03;
    mapmem[1] = 0x18;

    mapmem[15] = CalcCRC(mapmem);
    if (!WriteToDevice(mapmem))
    {
        cout << "Impossibile cancellare le chiavi sul marcatempo di indirizzo "<<m_Address<<endl;
        return false;
    }

    ///////////////// CARICO LE CHIAVI ////////////////////////////////////
    m_TimeMarkIniHndlr = new CIniFileHandler();

    if (!m_TimeMarkIniHndlr->Load("./timeMarker.ini"))
    {
        cout << "Impossibile aprire il file timeMark.ini"<<endl;
    }

    sectionName+=m_ID;

    nOfKeys = m_TimeMarkIniHndlr->GetInt("nOfKeys",sectionName,0);

    for (int i = 1; i<=nOfKeys;i++)
    {
        t_TM_KeyData newKey;
        
        keyName=KEY_NAME;
        keyName+=i;

        keyString = m_TimeMarkIniHndlr->GetString(keyName,sectionName,"");

        if (keyString.size()==0)
        {
            cout<<"Attenzione!!! Manca chiave numero "<<i<<endl;
            continue;
        }

        m_IniLib.GetConfigParamString(keyString.c_str(),"NAME",&(newKey.name),"NA");
        m_IniLib.GetConfigParamString(keyString.c_str(),"SN",&(newKey.serialNum),"NA");

        if (newKey.serialNum.size()!=16)
        {
            cout<<"Attenzione numero serie chiave NON valido per la chiave numero "<<i<<endl;
            continue;
        }

        AddKey(newKey.name, newKey.serialNum);
    }
    

    //////////////////// COPIO il file dei passaggi in RAM  ////////////////////////////////////
    fileNameAndPath=m_LocalFilePath+m_FileName;

    //Controllo se esiste il file fisso
    inp.open(fileNameAndPath.c_str(),ifstream::in);
    inp.close();
    if (inp.fail())
    {
        //non esiste, lo creo e lo copio in posizione
        inp.clear(ios::failbit);
        out.open(fileNameAndPath.c_str(), ofstream::out | ofstream::app);
        out.close();
    }

    command="cp ";
    command+=m_LocalFilePath;
    command+=m_FileName;
    command+=" ";
    command+=m_TempFilePath;

    system(command.c_str());

    //////////////////// CONTROLLO SE C'E' UN FILE PRESENZE ////////////////////////////////////
    m_PresenceFileName="marker";
    m_PresenceFileName+=m_ID;
    m_PresenceFileName+="_Presence.ini";

    fileNameAndPath=m_LocalFilePath;
    fileNameAndPath+=m_PresenceFileName;

    //Controllo se il file esiste
    inp.open(fileNameAndPath.c_str(), ifstream::in);
    inp.close();
    if(inp.fail())
    {
        cout<<"Creo il file dei passaggi in RAM..."<<endl;
        //Il file non esiste, ne creo uno in RAM
        inp.clear(ios::failbit);
        
        out.open(fileNameAndPath.c_str(), ofstream::out | ofstream::app);

        if (out.fail())
        {
            out.clear(ios::failbit);
            cout << "Errore nell'apertura del file :"<<fileNameAndPath<<endl;
            return false;
        }

        out << "["<<sectionName<<"]"<<endl;
        out.close();
    }
    else
    {
        //Il file esiste --> Lo caopio in ram e carico i dati
        fileNameAndPath=m_TempFilePath;
        fileNameAndPath+=m_PresenceFileName;

        command="cp ";
        command+=m_LocalFilePath;
        command+=m_PresenceFileName;
        command+=" ";
        command+=fileNameAndPath;

        system(command.c_str());
        
        m_PresenceIniHndlr = new CIniFileHandler();

        if (!m_PresenceIniHndlr->Load(fileNameAndPath))
        {
            return false;
        }

        nOfKeys=m_PresenceIniHndlr->GetInt("nOfKeys",sectionName,0);

        for (int i = 1; i < nOfKeys; i++)
        {
            t_TM_KeyData newPresence;
            keyName=KEY_NAME;
            keyName+=i;

            keyString = m_PresenceIniHndlr->GetString(keyName,sectionName,"");

            if (keyString.size()==0)
            {
                //TODO
                cout<<"Attenzione!!! Nelle presenze manca chiave numero "<<i<<endl;
                continue;
            }

            m_IniLib.GetConfigParamString(keyString.c_str(),"NAME",&(newPresence.name),"NA");
            m_IniLib.GetConfigParamString(keyString.c_str(),"SN",&(newPresence.serialNum),"NA");

            if (newPresence.serialNum.size()!=16)
            {
                //TODO
                cout<<"Attenzione!!! Nelle presenze il numero serie chiave NON valido per la chiave numero "<<i<<endl;
                continue;
            }

            m_Presences.push_back(newPresence);
        }
    }

    return true;
    
}

//////////////////////////////////////////////////////////////
//                  SaveKeyMarkData
//////////////////////////////////////////////////////////////
bool CTimeMarkerCtrl::SaveKeyMarkData(t_KeyMark keyMark)
{
    bool retVal = false;
    ifstream inp;
    ofstream out;
    CString fileNameAndPath;
    CString outString;
    struct tm markDate;
    char date[32];
    char hour[32];
    

    fileNameAndPath=m_TempFilePath;
    fileNameAndPath+=m_FileName;

    //Inserisco un carattere che mi segna l'inizio log per la funzione TrimFile
    outString = "<,";
    outString+=keyMark.keyData.name;
    outString+=",";

    if (keyMark.typeOfMark == 1)
    {
        outString+="Entrata";
    }
    else if (keyMark.typeOfMark == 2)
    {
        outString +="Uscita";
    }
    else
    {
        outString +="Invalid";
    }

    outString +=",";
    
    //Devo formattare data e ora....
    ConvertSecs2DateTM(keyMark.markTime, &markDate);

    memset (hour, 0x0, 32);
    memset (date, 0x0,32);
    //Creo la data
    strftime(date,31,"%d/%m/%Y",&markDate);
    //creo l'ora
    strftime(hour,31,"%H:%M",&markDate);

    outString+=hour;
    outString+=",";
    outString+=date;
    outString+="\r\n";

    if (!TrimFile(fileNameAndPath.c_str(), outString.size(), m_MaxDimFile))
    {
        return false;
    }

    out.open(fileNameAndPath.c_str(), ios::out | ios::app);

    if (out.fail())
    {
        out.clear(ios::failbit);
        cout << "SAVEKEYMARKDATA:Errore nell'apertura del file :"<<fileNameAndPath<<endl;
        return false;
    }

    out<<outString;

    out.close();


    return retVal;
}

//////////////////////////////////////////////////////////////
//                  AddKey
//////////////////////////////////////////////////////////////
bool CTimeMarkerCtrl::AddKey(CString name, CString serialNum)
{
    bool retVal = false;
    uchar mapmem[16];
    t_TM_KeyData newKey;

    memset(mapmem, 0xFF, 16);

    //Cerco se la chiave esiste
    if (FindRegisteredKey(serialNum) < 0)
    {
        mapmem[0] = 0x03; //Operazione su chiave

        //TODO aggiungere la gestione dei canali
        mapmem[1] = 0x02; //Aggiunta chiave

        ConvertSN2Hex(serialNum.c_str(), &(mapmem[6]));

        mapmem[14]=0x00; //Canale 1

        mapmem[15] = CalcCRC(mapmem);
        if (WriteToDevice(mapmem))
        {
            //Se programmata la aggiungo al vettore

            newKey.name = name;
            newKey.serialNum = serialNum;

            m_ValidKeys.push_back(newKey);

            //Salvo le chiavi nel file
            WriteOnFileValidKeys();
            
            retVal = true;
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////
//                  WriteOnFileValidKeys
//////////////////////////////////////////////////////////////
bool CTimeMarkerCtrl::WriteOnFileValidKeys()
{
    if (EraseAllKeysFromIniFile(m_TimeMarkIniHndlr, &m_ValidKeys))
    {
        return SaveAllKeysToIniFile(m_TimeMarkIniHndlr, &m_ValidKeys);
    }
    else
    {
        return false;
    }
}

//////////////////////////////////////////////////////////////
//                  EraseOnFileValidKeys
//////////////////////////////////////////////////////////////
bool CTimeMarkerCtrl::EraseOnFileValidKeys()
{
    return EraseAllKeysFromIniFile(m_TimeMarkIniHndlr, &m_ValidKeys);
}

//////////////////////////////////////////////////////////////
//                  SaveKeyToIniFile
//////////////////////////////////////////////////////////////
bool CTimeMarkerCtrl::SaveAllKeysToIniFile(CIniFileHandler * iniFile, vector< t_TM_KeyData > *keyList)
{
    bool retVal = true;
    int nOfKeys = 1;
    CString keyId;
    CString sectionId = "MARKER";
    vector<t_TM_KeyData>::iterator keyIterator;

    //Ricarico il file casomai qualcun'altro lo avesse modificato
    iniFile->Reload();

    sectionId+=m_ID;

    //Cancello tutte le chiavi
//     EraseOnFileValidKeysaass();

    //Riscrivo nell'ini tutte le chiavi
    for (keyIterator = keyList->begin(); keyIterator < keyList->end(); keyIterator++)
    {
        CString keyString;
        
        keyId = KEY_NAME;
        keyId += nOfKeys;

        keyString="SN:";
        keyString+=keyIterator->serialNum;

        keyString+=", NAME:";
        keyString+=keyIterator->name;

        retVal = iniFile->SetValue(keyId,keyString,"",sectionId) && retVal;

        nOfKeys++;
    }

    //Scrivo il numero totale di chiavi
    retVal = iniFile->SetInt("nOfKeys", (nOfKeys-1),"",sectionId) && retVal;
    
    return (iniFile->Save() && retVal);
    
}

//////////////////////////////////////////////////////////////
//                  EraseKeyFromIniFile
//////////////////////////////////////////////////////////////
bool CTimeMarkerCtrl::EraseAllKeysFromIniFile(CIniFileHandler * iniFile, vector< t_TM_KeyData > *keyList)
{
    bool retVal = true;
    unsigned int number;
    CString sectionId = "MARKER";

    sectionId+=m_ID;
    
    iniFile->Reload();
    
    for (number = 0; number < keyList->size(); number++){
        CString keyId = KEY_NAME;
        keyId+=number+1;
        iniFile->DeleteKey(keyId, sectionId) && retVal;
    }

    retVal = iniFile->SetInt("nOfKeys",0,"",sectionId) && retVal;
    retVal = iniFile->Save();

    return retVal;
}

//////////////////////////////////////////////////////////////
//                  RemoveAllKeys
//////////////////////////////////////////////////////////////
bool CTimeMarkerCtrl::RemoveAllKeys()
{
    bool retVal = false;
    uchar mapmem[16];

    memset (mapmem,0xff,16);

    mapmem[0] = 0x03;
    mapmem[1] = 0x18;

    mapmem[15] = CalcCRC(mapmem);
    if (WriteToDevice(mapmem))
    {
        
        EraseAllKeysFromIniFile(m_TimeMarkIniHndlr,&m_ValidKeys);
        m_ValidKeys.clear();
        retVal = true;
    }

    return retVal;
    
}

///////////////////////////////////////////////////////////////////
//                    RemoveKey
///////////////////////////////////////////////////////////////////
bool CTimeMarkerCtrl::RemoveKey(CString serialNum)
{
    unsigned char mapMem[16];
    unsigned char romCode[9];
    int keyIndex = -1;
    vector <t_TM_KeyData>::iterator keyIt;

    memset (mapMem, 0xff, 16*sizeof(uchar));
    mapMem[0] = 0x03;
    mapMem[1] = 0x04;

    ConvertSN2Hex(serialNum.c_str(), romCode);

    memcpy (&(mapMem[6]), romCode, 8);

    mapMem[15] = CalcCRC(mapMem);
    if (WriteToDevice(mapMem)){
        keyIndex = FindRegisteredKey(serialNum, &keyIt);

        if (keyIndex >= 0)
        {
            m_ValidKeys.erase(keyIt);
            //Salvo le chiavi nel file
            WriteOnFileValidKeys();

            return true;
        }

    }

    return false;
}


