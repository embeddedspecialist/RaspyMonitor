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
#include "nthmgc.h"


static const char *hotel_file_strings[] = {
    "RoomFree",
    "RemoteOn",
    "GuestName",
    "NofGuestKeys",
    "AirConditioned",
    "PID",
    "HotWater",
    "ColdWater",
    "Watt",
    "Calories",
    "Frigories",
    "CheckIn"
};

///////////////////////////////////////////////////
//              StandardConstructor
///////////////////////////////////////////////////
CNTHMGC::CNTHMGC(const char *configString, CTimer *timer) : CUPID2((char*)(configString))
{
    int input2Val = 0;

    if (configString != 0x0){
        m_IniLib.GetConfigParamInt(configString, "ROOM", &m_RoomNumber, -1);
        m_IniLib.GetConfigParamInt(configString, "INPUT2", &input2Val, -1);
        m_IniLib.GetConfigParamString(configString, "FILENAME", &m_IniFileName,"./presenze.ini");

        if (input2Val < 0){
            m_HasCounters = false;
            m_RoomCounter = 0x0;
        }
        else {
            m_HasCounters = true;
        }
    }


    m_IsRoomFree = false;
    m_IsRoomOn = false;
    m_PresenceType = 0;
    m_RoomTemp = 0.0;
    m_RoomSetPoint = 0.0;
    memset (m_PidParameters, 0x0, 3*sizeof(int));
    m_PIDDivider = 0;
    m_PIDIsSummer = false;
    m_CounterInitOk = false;
    m_RoomCounter = 0x0;

    memset (m_CheckInCounters, 0x0, 5*sizeof(float));

    m_CheckInDateSec = m_CheckOutDateSec = 0;
    m_TypeOfTimerVal = TIMERVAL_DIGITAL;
    m_ControllerType = DEV_MGC;
}

///////////////////////////////////////////////////
//              StandardDestructor
///////////////////////////////////////////////////
CNTHMGC::~CNTHMGC()
{

}

///////////////////////////////////////////////////
//              Getter/Setter
///////////////////////////////////////////////////
bool  CNTHMGC::SetRoomSetPoint(float newSP) {
    CString pidConfString;
    CString sectionName = "Room";
    char floatStr[32];
    CIniFileHandler iniFileHandler;

    if (!iniFileHandler.Load(m_IniFileName)) {
        if (m_DebugLevel){
            cout << "MGC address: "<<m_Address<<" Impossibile aprire il file ini"<<m_IniFileName<<endl;
        }
        return false;
    }

    iniFileHandler.Reload();
    
    sectionName+=m_RoomNumber;
    m_RoomSetPoint = newSP;
    if (ProgramPID()){
        pidConfString = iniFileHandler.GetString(hotel_file_strings[PID], sectionName);
        sprintf(floatStr,"%2.1f", m_RoomSetPoint);
        m_IniLib.SetConfigParamString(&pidConfString,"SP",floatStr);
        iniFileHandler.SetValue("PID",pidConfString,"",sectionName);
        iniFileHandler.Save();
        return true;
    }
    return false;
}

///////////////////////////////////////////////////
//              TurnOnRoom
//////////////////////////////////////////////////
bool CNTHMGC::TurnOnRoom(bool turnOn) {
    CString sectionName = "Room";
    uchar mapMem[16];
    CIniFileHandler iniFileHandler;

    if (!iniFileHandler.Load(m_IniFileName)) {
        if (m_DebugLevel){
            cout << "MGC address: "<<m_Address<<" Impossibile aprire il file ini"<<m_IniFileName<<endl;
        }
        return false;
    }

    sectionName+=m_RoomNumber;
    memset (mapMem, 0xFF, 16*sizeof(uchar));
    mapMem[0] = 0x02;
    mapMem[1] = 0x04;

    if (turnOn) {
        mapMem[10] = 1;
    }
    else {
        mapMem[10] = 0;
    }

    mapMem[15] = CalcCRC(mapMem);

    if (WriteToDevice(mapMem) && CheckCRC(mapMem)){
        iniFileHandler.Reload();
        m_IsRoomOn = turnOn;
        iniFileHandler.SetBool(hotel_file_strings[REMOTE_ON], turnOn, "", sectionName);
        iniFileHandler.Save();
        return true;
    }

    return false;
}

///////////////////////////////////////////////////
//              SetAirCondEnable
//////////////////////////////////////////////////
bool CNTHMGC::SetAirCondEnable(bool enable){

    CString sectionName = "Room";
    uchar mapMem[16];
    CIniFileHandler iniFileHandler;

    if (!iniFileHandler.Load(m_IniFileName)) {
        if (m_DebugLevel){
            cout << "MGC address: "<<m_Address<<" Impossibile aprire il file ini"<<m_IniFileName<<endl;
        }
        return false;
    }

    sectionName+=m_RoomNumber;
    memset (mapMem, 0xFF, 16*sizeof(uchar));
    mapMem[0] = 0x02;
    mapMem[1] = 0x10;

    if (enable) {
        mapMem[12] = 1;
    }
    else {
        mapMem[12] = 0;
    }

    mapMem[15] = CalcCRC(mapMem);

    if (WriteToDevice(mapMem) && CheckCRC(mapMem)){
        iniFileHandler.Reload();
        m_IsAirCondEnabled = enable;
        iniFileHandler.SetBool(hotel_file_strings[AIR_COND], enable, "", sectionName);
        iniFileHandler.Save();
        return true;
    }

    return false;
}

///////////////////////////////////////////////////
//              SetSummer
//////////////////////////////////////////////////
bool CNTHMGC::SetSummer(bool isSummer)
{
    CString pidConfString;
    CString sectionName = "Room";
    char boolStr[8];
    bool isSummerBackup = m_PIDIsSummer;
    CIniFileHandler iniFileHandler;

    if (!iniFileHandler.Load(m_IniFileName)) {
        if (m_DebugLevel){
            cout << "MGC address: "<<m_Address<<" Impossibile aprire il file ini"<<m_IniFileName<<endl;
        }
        return false;
    }

    sectionName+=m_RoomNumber;

    m_PIDIsSummer = isSummer;
    
    if (ProgramPID()){
        iniFileHandler.Reload();
        pidConfString = iniFileHandler.GetString(hotel_file_strings[PID], sectionName);
        sprintf(boolStr,"%d", isSummer);
        m_IniLib.SetConfigParamString(&pidConfString,"SUMMER",boolStr);
        iniFileHandler.SetValue("PID",pidConfString,"",sectionName);
        iniFileHandler.Save();
        return true;
    }
    else {
        //C'e' stato un errore: ripristino il vecchio valore
        m_PIDIsSummer = isSummerBackup;
    }
    return false;
}

///////////////////////////////////////////////////
//              SetPIDParam
//////////////////////////////////////////////////
bool CNTHMGC::SetPIDParam(float *param, int pidDivider) {
    CString pidConfString;
    CString sectionName = "Room";
    char intStr[32];
    CIniFileHandler iniFileHandler;

    if (!iniFileHandler.Load(m_IniFileName)) {
        if (m_DebugLevel){
            cout << "MGC address: "<<m_Address<<" Impossibile aprire il file ini"<<m_IniFileName<<endl;
        }
        return false;
    }

    sectionName+=m_RoomNumber;

    for (int i = 0; i < 3; i++) {
        m_PidParameters[i] = (int)(param[i]);
    }

    m_PIDDivider = pidDivider;

    if (ProgramPID()){

        iniFileHandler.Reload();
        
        pidConfString = iniFileHandler.GetString(hotel_file_strings[PID], sectionName);

        //Kp
        sprintf(intStr,"%d", m_PidParameters[0]);
        m_IniLib.SetConfigParamString(&pidConfString,"Kp",intStr);
        //Ki
        sprintf(intStr,"%d", m_PidParameters[1]);
        m_IniLib.SetConfigParamString(&pidConfString,"Ki",intStr);
        //Kd
        sprintf(intStr,"%d", m_PidParameters[2]);
        m_IniLib.SetConfigParamString(&pidConfString,"Kd",intStr);

        iniFileHandler.SetValue("PID",pidConfString,"",sectionName);
        iniFileHandler.Save();
        return true;
    }
    return false;
}

///////////////////////////////////////////////////
//              GetPIDParam
//////////////////////////////////////////////////
bool CNTHMGC::GetPIDParam(float *dest, int *pidDivider){
    if ( (dest != 0x0) && (pidDivider != 0x0))
    {
        memcpy (dest, m_PidParameters, 3*sizeof(float));
        *pidDivider = m_PIDDivider;
        return true;
    }
    else
    {
        return false;
    }
    
    
}

///////////////////////////////////////////////////
//              Update
///////////////////////////////////////////////////
bool CNTHMGC::Update( bool updatefirst )
{
    time_t actDateSec;
    unsigned int devTimeSec;
    uchar mapMem[16];

    memset (mapMem, 0xFF, 16*sizeof(uchar));
    time(&actDateSec);

    if (m_InitOk) {

        if (actDateSec < m_LastUpdateTime + m_UpdateTime)
        {
            return true;
        }
            

        mapMem[0] = 0x01;
        mapMem[15] = CalcCRC(mapMem);

        if (WriteToDevice(mapMem) && CheckCRC(mapMem)){
            //Decodifico i diversi campi

            //Data e ora
            memcpy (&devTimeSec, &mapMem[1], 4);

            if (labs(actDateSec - devTimeSec) > MAX_TIME_ERROR){
                UpdateDateTime(actDateSec);
            }

            //FLAG di STATO
            if (mapMem[5] & 0x01){
                m_PresenceType = 1;
            }
            else if (mapMem[5] & 0x02){
                m_PresenceType = 2;
            }
            else {
                m_PresenceType = 0;
            }

            if (mapMem[5] & 0x04){
                m_IsRoomOn = true;
            }
            else {
                m_IsRoomOn = false;
            }

            //TBI l'errore di lettura della sonda
            if (mapMem[5] & 0x08)
            {
                cout << "Il modulo ha un errore sulla sonda di temp" << endl;
            }

            //Divisore parametri PID
            m_PIDDivider = mapMem[9] & 0x0F;
            m_PidParameters[0] = mapMem[6];
            m_PidParameters[1] = mapMem[7];
            m_PidParameters[2] = mapMem[8];
            m_PIDIsSummer = mapMem[9] & 0x10;

            //Temperatura misurata: 1 byte scalato nel range -5 +50
            m_RoomTemp = ((float)mapMem[10])/255.0*55.0-5.0;

            //Setpoint: 1 byte scalato nel range 5 - 30
            m_RoomSetPoint = ((float)mapMem[11])/255.0*25.0+5.0;

            //Qui ci sono le chiavi ospite e clienti: se c'è discordanza tra il sw ed il modulo
            //potrei decidere di fare qualcosa tipo ricaricare dal modulo le chiavi
            //TBI
            //Tutto bene: aggiorno il tempo di aggiornamento (;-)
            m_LastUpdateTime = actDateSec;
            //Ricalcolo il tempo di aggiornamento
            GenerateUpdateTime();
            return true;

        }
        else
        {
            //TBI
            cout << "Impossibile comunicare con il modulo di controllo camera"<<endl;
        }

        return false;
    }
    else {
        return InitDevice();
    }

}


///////////////////////////////////////////////////
//              ProgramRomCode
///////////////////////////////////////////////////
bool CNTHMGC::ProgramRomCode( uchar * romCode, bool isGuest, unsigned int expDateSec )
{
    unsigned char mapMem[16];
    t_MGC_KeyData tempKey;

    memset (mapMem, 0xFF, 16*sizeof(uchar));

    mapMem[0] = 0x03;
    mapMem[1] = 0x02; //Aggiungi chiave

    if (isGuest){
        mapMem[1] = mapMem[1] | 0x01;
    }

    memcpy (&(mapMem[2]), &expDateSec, 4);
    memcpy (&(mapMem[6]), romCode, 8);

    mapMem[15] = CalcCRC(mapMem);

    if (WriteToDevice(mapMem) && CheckCRC(mapMem)){
        tempKey.expireDateSec = expDateSec;
        memcpy (&(tempKey.keySN), romCode, 8);

        if (isGuest){
            m_GuestArray.push_back(tempKey);
        }
        else {
            m_PersArray.push_back(tempKey);
        }
        return true;
    }
    else
    {
      cout << "Impossibile programmare la chiave"<<endl;
    }
    
    return false;
}

///////////////////////////////////////////////////
//              EraseRomCode
///////////////////////////////////////////////////
bool CNTHMGC::EraseRomCode(uchar* romCode, bool eraseGuest, int* keyIndex )
{
    unsigned char mapMem[16];

    memset (mapMem, 0xFF, 16*sizeof(uchar));
    mapMem[0] = 0x03;
    mapMem[1] = 0x04;
    if (eraseGuest){
        mapMem[1] = mapMem[1] | 0x01;
    }

    memcpy (&(mapMem[6]), romCode, 8);

    mapMem[15] = CalcCRC(mapMem);

    if (WriteToDevice(mapMem) && CheckCRC(mapMem)){
        //Trovo la chiave e la tolgo dall'array
        vector<t_MGC_KeyData>::iterator keyIt;
        int keyIdx = FindKey(romCode, eraseGuest, &keyIt);

        if (keyIdx >= 0){
            if (keyIndex != 0x0)
            {
                *keyIndex = keyIdx;
            }
            
            if (eraseGuest){
                EraseOnFileGuestKeys();
                m_GuestArray.erase(keyIt);
                WriteOnFileGuestKeys();
            }
            else {
                m_PersArray.erase(keyIt);
            }

            return true;
        }

    }

    return false;
}

///////////////////////////////////////////////////
//              FindKey
///////////////////////////////////////////////////
int CNTHMGC::FindKey(uchar *SN, bool isGuest, vector<t_MGC_KeyData>::iterator *retIt){
    int retVal = 0;
    vector<t_MGC_KeyData>::iterator it;

    if (isGuest) {
        for (it = m_GuestArray.begin(); it < m_GuestArray.end(); it++) {
            if (!memcmp(it->keySN,SN, 8)){
                //elemento trovato
                if (retIt != 0x0){
                    *retIt = it;
                }
                break;
            }

            retVal++;
        }

        if (retVal == (int)m_GuestArray.size()){
            //Elelemento NON trovato
            retVal = -1;
        }
    }
    else {
        for (it = m_PersArray.begin(); it < m_PersArray.end(); it++) {
            if (!memcmp(it->keySN,SN, 8)){
                //elemento trovato
                if (retIt != 0x0){
                    *retIt = it;
                }
                break;
            }
            retVal++;
        }

        if (retVal == (int)m_PersArray.size()){
            //Elelemento NON trovato
            retVal = -1;
        }
    }

    return retVal;

}
///////////////////////////////////////////////////
//              InitDevice
///////////////////////////////////////////////////
bool CNTHMGC::InitDevice(CIniFileHandler* inifileHandler){

    CString section, keyStr, keyKey, keySN, keyExp;
    t_MGC_KeyData tempkey;
    int nOfKeys;
    uchar mapMem[16];
    CIniFileHandler iniFileHandler;

    if (!iniFileHandler.Load(m_IniFileName)) {
        if (m_DebugLevel){
            cout << "MGC address: "<<m_Address<<" Impossibile aprire il file ini"<<m_IniFileName<<endl;
        }
        return false;
    }
 
    iniFileHandler.Clear();
    if (!iniFileHandler.Load(m_IniFileName))
    {
        cout << "Impossibile aprire il file "<< m_IniFileName << endl;
        return false;
    }
    
    //TBI tutta la gestione degli errori
    section = "Room";
    section+=m_RoomNumber;

    //Leggo dal file le impostazioni
    m_IsRoomFree = iniFileHandler.GetBool(hotel_file_strings[ROOM_FREE], section, true);

    //Riprogrammo la scheda
    memset (mapMem, 0xFF, 16*sizeof(uchar));
    //Comando di cancellazione di tutte le chiavi ospite
    mapMem[0] = 0x03;
    mapMem[1] = 0x08;

    mapMem[15] = CalcCRC(mapMem);

    if ( !(WriteToDevice(mapMem)&& CheckCRC(mapMem)) ) {
        if (m_DebugLevel){
            cout << "MGC Address:"<<m_Address<<"Impossibile cancellare le chiavi"<<endl;
        }
        return false;
    }

    //Chiavi ospite
    if (!m_IsRoomFree){
        m_GuestName = iniFileHandler.GetString(hotel_file_strings[GUEST_NAME], section, "");
        nOfKeys = iniFileHandler.GetInt(hotel_file_strings[NOF_G_KEYS], section, 0);

        for (int i = 0; i < nOfKeys; i++){
            keyKey = "GuestKey";
            keyKey+=(i+1);
            keyStr = iniFileHandler.GetString(keyKey, section);
            if (keyStr.size() == 0){
                if (m_DebugLevel){
                    cout << "MGC Address:"<<m_Address<<"Impossibile trovare la chiave ospite numero: "<<(i+1)<<endl;
                }
                return false;
            }

            m_IniLib.GetConfigParamString(keyStr.c_str(), "SN", &keySN, "");
            if (keySN.size()==0){
                if (m_DebugLevel){
                    cout << "MGC Address:"<<m_Address<<"Numero di serie della chiave ospite numero: "<<(i+1)<<endl;
                }
                return false;
            }

            ConvertSN2Hex(keySN.c_str(),tempkey.keySN);

            m_IniLib.GetConfigParamInt(keyStr.c_str(), "EXP", &(tempkey.expireDateSec), 0);

            //Quando parto prendo la prima scadenza di chiave e la segno come data di checkout della stanza
            //non è bellissimo ma ...
            if (m_CheckOutDateSec == 0){
                m_CheckOutDateSec = tempkey.expireDateSec;
            }

            //Carico il checkin
            m_CheckInDateSec = iniFileHandler.GetInt(hotel_file_strings[CHECKIN_DATE], section);


            if (!ProgramRomCode(tempkey.keySN, true, tempkey.expireDateSec)){
                if (m_DebugLevel){
                    cout << "MGC Address:"<<m_Address<<"Impossibile programmare la chiave ospite numero: "<<(i+1)<<endl;
                }
                return false;
            }

        }

        //Devo caricare i contaenergia dal file ini
        m_CheckInCounters[0] = iniFileHandler.GetFloat(hotel_file_strings[HOT_WATER],section,0.0);
        m_CheckInCounters[1] = iniFileHandler.GetFloat(hotel_file_strings[COLD_WATER],section,0.0);
        m_CheckInCounters[2] = iniFileHandler.GetFloat(hotel_file_strings[WATT],section,0.0);
        m_CheckInCounters[3] = iniFileHandler.GetFloat(hotel_file_strings[FRIGORIES],section,0.0);
        m_CheckInCounters[4] = iniFileHandler.GetFloat(hotel_file_strings[CALORIES],section,0.0);
    }

    //Se la camera è occupata ed ho il contabilizzatore
    //carico i valori che avevo registrato al checkin
    if ((!m_IsRoomFree) && (m_HasCounters)){
        m_RoomCounter->InitDevice();
        for (int i = 0; i < 4; i++){
            m_CheckInCounters[i] = iniFileHandler.GetInt(hotel_file_strings[i+HOT_WATER], section);
        }
    }

    //Controllo se era stata attivata da remoto solo se la camera è vuota
    if (m_IsRoomFree){
        m_IsRoomOn = iniFileHandler.GetBool(hotel_file_strings[REMOTE_ON], section, false);

        if (m_IsRoomOn){
            //Accendo la camera
            memset(mapMem, 0xFF, 16*sizeof(uchar));
            mapMem[0] = 0x02;
            mapMem[1] = 0x04;
            mapMem[10] = m_IsRoomOn;
            mapMem[15] = CalcCRC(mapMem);
            if (!(WriteToDevice(mapMem)&&CheckCRC(mapMem))){
                return false;
            }
        }
    }
    
   //Aggiorno i clock delle schede
    if (!UpdateCommonData()){
        return false;
    }

    //Imposto i restanti funzionamenti
    //Il falg lo carico sempre per aggiornare correttamente l'interfaccia
    m_IsAirCondEnabled = iniFileHandler.GetBool(hotel_file_strings[AIR_COND], section);

    if (!m_IsRoomFree){
        memset(mapMem, 0xFF, 16*sizeof(uchar));
        mapMem[0] = 0x02;
        mapMem[1] = 0x04; //Maschera del byte di AC
        if (m_IsAirCondEnabled){
            mapMem[12] = 0x01;
        }
        else {
            mapMem[12] = 0x0;
        }

        mapMem[15] = CalcCRC(mapMem);
        
        if (!(WriteToDevice(mapMem) && CheckCRC(mapMem)) ) {
            return false;
        }
    }

    //Prendo i parametri del PID e il setpoint e li programmo
    keyStr = iniFileHandler.GetString(hotel_file_strings[PID], section);
    m_IniLib.GetConfigParamInt(keyStr.c_str(),"Kp", &m_PidParameters[0], 10);
    m_IniLib.GetConfigParamInt(keyStr.c_str(),"Ki", &m_PidParameters[1], 1);
    m_IniLib.GetConfigParamInt(keyStr.c_str(),"Kd", &m_PidParameters[2], 0);
    m_IniLib.GetConfigParamInt(keyStr.c_str(),"DIV", &m_PIDDivider, 1);
    m_IniLib.GetConfigParamBool(keyStr.c_str(),"SUMMER", &m_PIDIsSummer, false);
    m_IniLib.GetConfigParamFloat(keyStr.c_str(),"SP", &m_RoomSetPoint, 20.0);

    if (!ProgramPID()){
        return false;
    }

    //Carico le chiavi del personale:; siccome la sezione nella quale si trovano è condivisa con il modulo accessi devo fare
    //una "zozzeria" finchè non ho tempo di fare meglio: se la chiave ha scadenza nulla è davvero del personale altrimenti è un ospite
    //e quindi non la devo programmare
    //TODO da rivedere insieme alla gestione dell'hotel
    section = "Personnel";
    nOfKeys = iniFileHandler.GetInt(ACCESS_CNTRL_NOF_RECORDS, section, 0);
    for (int i = 0; i < nOfKeys; i++){
        int expDateSec = 0;
        keyKey = "PersKey";
        keyKey+=(i+1);
        keyStr = iniFileHandler.GetString(keyKey, section);
        if (keyStr.size() == 0){
            if (m_DebugLevel){
                cout << "MGC Address:"<<m_Address<<" Impossibile trovare la chiave personale numero "<<(i+1)<<" Esco dall'Init"<<endl;
            }
            return false;
        }

        m_IniLib.GetConfigParamInt(keyStr.c_str(), "EXP", &(expDateSec), 0);
        if (expDateSec > 0)
        {
            //La chiave ha una scadenza: non è una chiave personale e quindi passepartout
            continue;
        }
        
        m_IniLib.GetConfigParamString(keyStr.c_str(), "SN", &keySN, "");
        if (keySN.size()==0){
            if (m_DebugLevel){
                cout << "MGC Address:"<<m_Address<<" La chiave personale numero "<<(i+1)<<" ha un numero di serie NON valido. Esco dall'Init"<<endl;
            }
            return false;
        }

        ConvertSN2Hex(keySN.c_str(),tempkey.keySN);

        tempkey.expireDateSec = 0;

        m_PersArray.push_back(tempkey);
    }


    //Comando di cancellazione di tutte le chiavi personale
    memset(mapMem, 0xFF, 16*sizeof(uchar));
    mapMem[0] = 0x03;
    mapMem[1] = 0x10;

    mapMem[15] = CalcCRC(mapMem);
    
    if (!(WriteToDevice(mapMem)&& CheckCRC(mapMem)) ) {
        if (m_DebugLevel){
            cout << "MGC Address:"<<m_Address<<"Impossibile cancellare le chiavi personale"<<endl;
        }
        return false;
    }

    //Riprogrammo tutte le chiavi personale
    for (unsigned int i = 0; i < m_PersArray.size(); i++){
        memset(mapMem, 0xFF, 16*sizeof(uchar));

        mapMem[0] = 0x03;
        mapMem[1] = 0x02; //Comando aggiungi chiave personale
        memcpy(&(mapMem[6]), m_PersArray[i].keySN,8);

        mapMem[15] = CalcCRC(mapMem);

        if (!(WriteToDevice(mapMem) && CheckCRC(mapMem))) {
            if (m_DebugLevel){
                cout << "MGC Address:"<<m_Address<<"Impossibile programmare la chiave personale"<<endl;
            }
            return false;
        }

        msDelay(150);
    }

    m_InitOk = true;
    return true;
}

/////////////////////////////////////////////////////
//                  ProgramPID
/////////////////////////////////////////////////////
bool CNTHMGC::ProgramPID(){
    uchar mapMem[16];

    memset(mapMem, 0xFF, 16*sizeof(uchar));

    mapMem[0] = 0x02;
    mapMem[1] = 0x2A; //Imposta summer, pid e setpoint
    mapMem[11] = (uchar)(255.0/25.0*(m_RoomSetPoint - 5.0));
    mapMem[13] = (uchar)m_PIDIsSummer;
    for (int i = 0; i < 3; i++){
        mapMem[6+i] = m_PidParameters[i];
    }
    mapMem[9] = (uchar)m_PIDDivider;

    mapMem[15] = CalcCRC(mapMem);

    return WriteToDevice(mapMem) && CheckCRC(mapMem);
}

/////////////////////////////////////////////////////
//                  CheckInKey
/////////////////////////////////////////////////////
bool CNTHMGC::CheckInKey(CString name, uchar* SNum, unsigned int checkinDate, unsigned int expdateSec) {
    bool retVal = false;
    CString sectionName = "Room";
    char snString[32];
    CString keyEntry = "GuestKey", keyConfStr;
    vector<t_MGC_KeyData>::iterator guestIt;
    CIniFileHandler iniFileHandler;

    if (!iniFileHandler.Load(m_IniFileName)) {
        if (m_DebugLevel){
            cout << "MGC address: "<<m_Address<<" Impossibile aprire il file ini"<<m_IniFileName<<endl;
        }
        return false;
    }

    sectionName+=m_RoomNumber;

    iniFileHandler.Reload();
    
    //Controllo se la chiave esiste già: in questo caso è una modifica della
    //data di checkout
    if ( FindKey(SNum, true, &guestIt) >= 0){
        //Aggiorno la chiave
        memcpy (guestIt->keySN, SNum, 8*sizeof(uchar));
        guestIt->expireDateSec = expdateSec;
        retVal = true;
        //Tolgo dal modulo la chiave e la reinserisco
        retVal = EraseRomCode(SNum, true) && retVal;
        retVal = ProgramRomCode(SNum,true,expdateSec) && retVal;
        //Cancello tutte le chiavi dal file e le riscrivo
        retVal = EraseOnFileGuestKeys() && retVal;
        retVal = WriteOnFileGuestKeys() && retVal;

    }
    else if (ProgramRomCode(SNum, true, expdateSec)){
        if (m_IsRoomFree){
            float counters[20];

            memset (counters, 0, 20*sizeof(float));
            
            m_IsRoomFree = false;
            m_GuestName = name;
            iniFileHandler.SetValue(hotel_file_strings[GUEST_NAME],m_GuestName,"",sectionName );

            //Sistemo lo stato della camera
            keyEntry = hotel_file_strings[ROOM_FREE];
            iniFileHandler.SetBool(keyEntry, m_IsRoomFree,"",sectionName );

            //Aggiornamento valori nel file ini dei contatori!!
            if (m_HasCounters)
            {
                GetCounterReadings(counters);
            }
            
            m_CheckInCounters[0] = counters[5]; //Calda
            m_CheckInCounters[1] = counters[10];//Fredda
            m_CheckInCounters[2] = counters[15];//Watt
            m_CheckInCounters[3] = counters[4]; //Frigorie
            m_CheckInCounters[4] = counters[3]; //Calorie

            iniFileHandler.SetFloat(hotel_file_strings[HOT_WATER], m_CheckInCounters[0],"",sectionName);
            iniFileHandler.SetFloat(hotel_file_strings[COLD_WATER], m_CheckInCounters[1],"",sectionName);
            iniFileHandler.SetFloat(hotel_file_strings[WATT], m_CheckInCounters[2],"",sectionName);
            iniFileHandler.SetFloat(hotel_file_strings[FRIGORIES], m_CheckInCounters[3],"",sectionName);
            iniFileHandler.SetFloat(hotel_file_strings[CALORIES], m_CheckInCounters[4],"",sectionName);

            //Aggiorno il checkin/checkout dell'intera stanza
            keyEntry = hotel_file_strings[CHECKIN_DATE];
            keyConfStr = "";
            keyConfStr += checkinDate;
            iniFileHandler.SetValue(keyEntry,keyConfStr,"",sectionName );

            m_CheckInDateSec = checkinDate;
            m_CheckOutDateSec = expdateSec;

            iniFileHandler.Save();
            //Rimetto a posto la chiave ospite
            keyEntry = "GuestKey";
        }

        //Preparo la stringa di configurazione
        ConvertSN2Str(snString, SNum);
        keyConfStr = "SN:";
        keyConfStr+= snString;
        keyConfStr+= ", EXP:";
        keyConfStr+=expdateSec;

        keyEntry+=m_GuestArray.size();
        iniFileHandler.SetValue(keyEntry,keyConfStr,"",sectionName);

        //Aggiorno il numero di chiavi
        keyEntry = hotel_file_strings[NOF_G_KEYS];
        keyConfStr = "";
        keyConfStr+=m_GuestArray.size();
        iniFileHandler.SetValue(keyEntry,keyConfStr,"",sectionName);


        iniFileHandler.Save();
        retVal = true;
    }

    return retVal;
}

/////////////////////////////////////////////////////
//                  CheckOutKey
/////////////////////////////////////////////////////
bool CNTHMGC::CheckOutKey(uchar *SNum) {
    bool retVal = false;
    CString sectionName = "Room";
    int keyIdx;
    CString keyEntry = "GuestKey", keyConfStr;
    CIniFileHandler iniFileHandler;

    if (!iniFileHandler.Load(m_IniFileName)) {
        if (m_DebugLevel){
            cout << "MGC address: "<<m_Address<<" Impossibile aprire il file ini"<<m_IniFileName<<endl;
        }
        return false;
    }

    sectionName+=m_RoomNumber;

    iniFileHandler.Reload();
    
    //La devo cancellare anche dal file ini
    if (EraseRomCode(SNum, true, &keyIdx)){

        //TBM
        keyEntry+=keyIdx;
        iniFileHandler.DeleteKey(keyEntry, sectionName);
        //Aggiorno il numero di chiavi
        iniFileHandler.SetInt(hotel_file_strings[NOF_G_KEYS],m_GuestArray.size(),"",sectionName);
        iniFileHandler.Save();
        retVal = true;
    }

    return retVal;
}

/////////////////////////////////////////////////////
//                  CheckOutRoom
//Pulisce completamente la stanza dalle chiavi ospite
/////////////////////////////////////////////////////
bool CNTHMGC::CheckOutRoom() {

    CString sectionName = "Room";
    bool retVal = false;
    uchar mapMem[16];
    CIniFileHandler iniFileHandler;

    if (!iniFileHandler.Load(m_IniFileName)) {
        if (m_DebugLevel){
            cout << "MGC address: "<<m_Address<<" Impossibile aprire il file ini"<<m_IniFileName<<endl;
        }
        return false;
    }

    sectionName+=m_RoomNumber;
    memset (mapMem, 0xFF, 16*sizeof(uchar));

    mapMem[0] = 0x03;
    mapMem[1] = 0x08;

    mapMem[15] = CalcCRC(mapMem);
    
    if (WriteToDevice(mapMem)&&CheckCRC(mapMem)) {
        
        iniFileHandler.Reload();
        
        //Riazzero lo stato della camera
        iniFileHandler.SetBool(hotel_file_strings[ROOM_FREE],true,"",sectionName);
        iniFileHandler.SetInt(hotel_file_strings[NOF_G_KEYS],0,"",sectionName);
        iniFileHandler.SetBool(hotel_file_strings[AIR_COND], true,"",sectionName);
        iniFileHandler.SetValue(hotel_file_strings[GUEST_NAME],"","",sectionName);
        iniFileHandler.SetInt(hotel_file_strings[CHECKIN_DATE],0,"",sectionName);
        iniFileHandler.Save();
        EraseOnFileGuestKeys();
        m_GuestArray.clear();
        if (!m_IsAirCondEnabled){
            SetAirCondEnable(true);
        }
        m_CheckInDateSec = 0;
        m_CheckOutDateSec = 0;
        m_GuestName = "";
        memset (m_CheckInCounters, 0x0, 5*sizeof(int));
        m_IsRoomFree = true;
        retVal = true;
    }

    return retVal;
}

/////////////////////////////////////////////////////
//                  GetCounterReadings
/////////////////////////////////////////////////////
bool CNTHMGC::GetCounterReadings(float * destArray)
{
    bool retVal = true;

    if (m_HasCounters){
        //Tolgo l'aggiornamento perchè dovrebbe essere l'engine ad aggiornare tutto
//        m_RoomCounter->Update(true);
        for (int i = 0; i < 4; i++){
            m_RoomCounter->GetData(i, &(destArray[i*5]));
        }
    }
    else {
        memset (destArray, 0, 20*sizeof(float));
    }

    return retVal;
}

/////////////////////////////////////////////////////
//                  WriteOnFileGuestKeys
/////////////////////////////////////////////////////
bool CNTHMGC::WriteOnFileGuestKeys(){
    CString section = "Room";
    unsigned int number;
    bool retVal = true;
    CIniFileHandler iniFileHandler;

    if (!iniFileHandler.Load(m_IniFileName)) {
        if (m_DebugLevel){
            cout << "MGC address: "<<m_Address<<" Impossibile aprire il file ini"<<m_IniFileName<<endl;
        }
        return false;
    }

    iniFileHandler.Reload();
    
    section+=m_RoomNumber;

    for (number = 0; number < m_GuestArray.size(); number++){
        CString guestKey = "GuestKey";
        CString guestConfigStr;
        char snumStr[32];

        guestConfigStr="SN:";
        ConvertSN2Str(snumStr,m_GuestArray.at(number).keySN);
        guestConfigStr+=snumStr;
        guestConfigStr+=",EXP:";
        guestConfigStr+=m_GuestArray.at(number).expireDateSec;
        guestKey+=number+1;
        retVal = iniFileHandler.SetValue(guestKey,guestConfigStr,"",section) && retVal;
    }
    iniFileHandler.SetInt(hotel_file_strings[NOF_G_KEYS],m_GuestArray.size(),"",section);
    retVal = iniFileHandler.Save() && retVal;

    return retVal;

}

/////////////////////////////////////////////////////
//                  EraseOnFileGuestKeys
/////////////////////////////////////////////////////
bool CNTHMGC::EraseOnFileGuestKeys(){
    CString section = "Room";
    unsigned int number;
    bool retVal = true;
    CIniFileHandler iniFileHandler;

    if (!iniFileHandler.Load(m_IniFileName)) {
        if (m_DebugLevel){
            cout << "MGC address: "<<m_Address<<" Impossibile aprire il file ini"<<m_IniFileName<<endl;
        }
        return false;
    }

    section+=m_RoomNumber;
    
    for (number = 0; number < m_GuestArray.size(); number++){
        CString guestKey = "GuestKey";
        guestKey+=number+1;
        retVal = iniFileHandler.DeleteKey(guestKey, section) && retVal;
    }

    iniFileHandler.SetInt(hotel_file_strings[NOF_G_KEYS],0,"",section);
    retVal = iniFileHandler.Save() && retVal;

    return retVal;
}
