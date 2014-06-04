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
 #include "floorcoord2.h"

#include "conewireengine.h"
#include "conewirenet.h"

//***MACROS & DEFINES***//
#define stricmp strcasecmp

#define INVALID_FLOAT -9999.9
#define DATA_NOT_UPDATED_FLOAT -999999.9
#define INVALID_UINT (unsigned int)(-1)
#define TEMP_UPPERBOUND 9999.9

#define CTRL_DWCAST(ptr,toType) dynamic_cast<toType*>(ptr)

#define DEFAULT_SEASON true
#define DEFAULT_ONOFF true
#define DEFAULT_HYS 0.0
#define DEFAULT_ISMAINT false


#define CHECK_INI_VOID() if(!m_isIniLoaded) return;
#define CHECK_INI_BOOL() if(!m_isIniLoaded) return false;
#define CHECK_LOADED_BOOL() if(!m_isVectorLoaded) return false;
#define CHECK_CONNECTED_BOOL() if(!m_isConnected) return false;
//**********************//


//definizione membri statici//
const char* const CFloorCoord2::ConfigFile = "./floorcoord2.ini";

 //Attenzione: corrispondenza 1:1 con indice dell'array str_DHTSeason.
 //case-insensitive, uso la stricmp.
const char* CFloorCoord2::str_DHTSeason[ DHT_SEASON_OPTIONS ] = { "never", "always", "summer", "winter" };

e_DHTSeason CFloorCoord2::SStr2Enum(const char* season)
{
    if( season == 0x0 || strlen( season ) == 0 ) { return DHT_INVALID; }

    for( unsigned short index = 0; index < DHT_SEASON_OPTIONS; index++ )
    {
        if( stricmp( season, str_DHTSeason[ index ] ) == 0 ) { return e_DHTSeason( index ); } //compare case-insensitive.
    }
    //se sono qui, non l'ho trovata..
    return DHT_INVALID;
}

const char* CFloorCoord2::SEnum2Str(e_DHTSeason season)
{
    if( season == DHT_INVALID ) { return 0x0; }
    //NOTA: Posso ritornare il puntatore alla stringa in quanto statica, non è locale alla funzione.
    unsigned short index = static_cast<unsigned short>(season);
    if( index >= DHT_SEASON_OPTIONS ) { return 0x0; }

    const char* retValue = str_DHTSeason[ index ];
    return retValue;
}

//--------------------------//

//////////////////////////////////////////////////
//                                              //
//  CFloorCoord2 costructor                      //
//                                              //
//////////////////////////////////////////////////
CFloorCoord2::CFloorCoord2(const char *configString, CTimer *timer)
    : CVCoordinator(configString)
{
    m_ControllerType = CoordType;
    m_TypeOfTimerVal = TIMERVAL_DIGITAL;

    //inizializzazione//
    m_EnginePtr = 0x0; m_NetPtr = 0x0;

    m_isSummer = DEFAULT_SEASON; m_pSummerSwitch = 0x0;

    m_ZonesCount = 0;
    m_isVectorLoaded = false;
    m_ZoneVector.clear();

    m_isConnected = false;
    //---------------//
    if (configString != 0x0)
    {
        m_IniLib.GetConfigParamInt( configString, "TIMERID", &m_TimerID, -1);

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

    //carico ConfigFile//
    m_isIniLoaded = m_hIni.Load( ConfigFile );
    if( !m_isIniLoaded )
    { cout << "Attenzione: impossibile aprire il file" << ConfigFile << "!" << endl; }
    //----------------//

    //TODO: stringa di configurazione assente, timer non utilizzato.
}

//////////////////////////////////////////////////
//                                              //
//  ~CFloorCoord2 destructor                     //
//                                              //
//////////////////////////////////////////////////
CFloorCoord2::~CFloorCoord2()
{
}

//////////////////////////////////////////////////
//                                              //
//  RetrieveCtrlEx                    //
//                                              //
//////////////////////////////////////////////////
CVController* CFloorCoord2::RetrieveCtrlEx( int devAddr, const char* devName, const char* Zone, bool showMsg )
{
    //chiamata classe base//
    unsigned short errorCode = RC_INVALID_VPTR;
    CVController *retPtr = RetrieveCtrl( devAddr, &errorCode );

    //se non è NULL, posso già uscire
    if( IS_CTRL(retPtr) ) { return retPtr; }

    //altrimenti, messaggistica di errore//
    if( showMsg )
    {
        switch ( errorCode )
        {
            case RC_INVALID_VPTR:
                cout<< "Attenzione: il coordinatore di indirizzo: " << m_Address << " NON è connesso al conewireNET! " << endl;
                //comunque non verrà usato
                //questo controllo è meglio farlo all'inizio della ConnectControllers.
                break;
            case RC_INVALID_ADDR:
                cout << "ATTENZIONE: manca ADDR nel " << devName << ", sezione " << Zone << " nel file " << CFloorCoord2::ConfigFile << "!" << endl;
                sleep ( 2 );
                break;
            case RC_INVALID_NETINDEX:
            case RC_INVALID_CTRLINDEX:
                cout << "ATTENZIONE: l'indirizzo " << devAddr << ", sezione " << Zone << ", file " << CFloorCoord2::ConfigFile << ", non esiste nel sistema!" << endl;
                sleep ( 2 );
                break;
            default:
                break;
        }
    }

    return retPtr;
}

//***CARICAMENTO DELL'INI***//
//////////////////////////////////////////////////
//                                              //
//  LoadCommons                                 //
//                                              //
//////////////////////////////////////////////////
void CFloorCoord2::LoadCommons(void)
{
     //carico entrambi, non esco

    //Alessandro Mirri -- Cambiato perche' la classe ini salva i bool come True o False e se li rileggo come int non funzionano
    m_isSummer = m_hIni.GetBool( "Summer", "COMMON", DEFAULT_SEASON );

    //summer switch
    int addr = -1;
    CVController *ptrCtrl = 0x0;

    addr = m_hIni.GetInt( "SummerSwitch", "COMMON", -1 );
    ptrCtrl = RetrieveCtrlEx( addr, "SwitchSummer", "COMMON", false ); //messaggistica nella funzione
    m_pSummerSwitch = CTRL_DWCAST( ptrCtrl, CDigitalIO ); //NULL di default, NULL se downcast fallito

}

//////////////////////////////////////////////////
//                                              //
//  LoadZones                                   //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::LoadZones(void)
{
    m_ZoneVector.clear(); m_ZonesCount = 0; m_isVectorLoaded = false;//svuoto il vettore

    //leggo il numero di zone
    const unsigned int nOfZones = (unsigned int)m_hIni.GetInt ( "nOfZones", "COMMON" , 0 );
    if( nOfZones == 0 )
    {
        { cout << "Attenzione: numero di zone mancante in " << ConfigFile << "!" << endl; }
        return false;
        //il vettore è vuoto ed il numero 0;
    }

    //ho instanziato m_hIni e conosco il numero di zone: ciclo per caricare le zone e riempire il vettore
    for( unsigned int zIndex = 0; zIndex < nOfZones; zIndex++ )
    {
        t_FloorZone lFZ; //Zona locale usate per le push_back (copiata nel vettore); inizializzata nel costruttore.
        
        if( LoadSingleZone( zIndex+1, lFZ ) ) //ritorna true se sutte le load NECESSARIE hanno ritornato true
            { m_ZoneVector.push_back( lFZ ); }

        cout<<"LoadSingleZone numero "<< zIndex<<" OK"<<endl;cout.flush();
    }


    //setto variabili membro
    m_ZonesCount = (unsigned int)m_ZoneVector.size();
    if( m_ZonesCount > 0 ) { m_isVectorLoaded = true; }

    //Messaggio errore se sono state trovate meno zone di quelle riportate in [COMMON]
    if( m_ZonesCount < nOfZones )
    { cout << "Attenzione: caricate correttamente " << m_ZonesCount << " su " << nOfZones << " in " << ConfigFile << "!" << endl; }

    
    return m_isVectorLoaded;
}

void CFloorCoord2::LoadWaterTempProbe(const char* sectionZone, t_FloorZone & Dest)
{
    int addr = -1;
    CVController *ptrCtrl = 0x0;

    addr = m_hIni.GetInt( "WaterTempProbe", sectionZone, -1 );
    ptrCtrl = RetrieveCtrlEx( addr, "WaterTempProbe", sectionZone, false );
    Dest.waterTempProbe = CTRL_DWCAST( ptrCtrl, CTempCtrl ); //NULL se non trovato, NULL se downcast fallito

    //Se non c'e' la sonda di temperatura annullo anche i controlli tanto non li potrei usare
    if (Dest.waterTempProbe == 0x0)
    {
        Dest.waterCtrlType = WATER_NONE;
    }
    else
    {
        Dest.waterSummerSetpoint = m_hIni.GetFloat("WaterSummerSetpoint", sectionZone, 15.0);
        Dest.waterWinterSetpoint = m_hIni.GetFloat("WaterWinterSetpoint", sectionZone, 40.0);
    }
}
//////////////////////////////////////////////////
//                                              //
//  LoadSingleZone                              //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::LoadSingleZone(unsigned int zoneIndex, t_FloorZone& Dest)
{

    Dest.init();

    //stringa contenente il settore relativo alla zone
    CString sectionName = "ZONE";
    sectionName+=zoneIndex;
    if( !m_hIni.ExistSection( sectionName ) ) { return false; }

    //Setto il numero della zona
    Dest.zNumber = zoneIndex;

    //Valvole e sonde temperatura necessari: se non trovati esco, e non carico la zona.
    if( !LoadValves( sectionName.c_str(), Dest.Valves ) ) { return false; }

    if( !LoadTemps( sectionName.c_str(), Dest.Temp ) ) { return false; }


    //funzioni load per cui sono previsti valori di default.
    LoadOnOff( sectionName.c_str(), Dest.OnOff ); //DEFAULT: controllore NULL
    LoadMaintenance( sectionName.c_str(), Dest.Maintenance ); //DEFAULT: controllore NULL.
    LoadHums( sectionName.c_str(), Dest.Hum ); //DEFAULT: vettore vuoto
    LoadDeHums( sectionName.c_str(), Dest.DeHum ); //DEFAULT: vettore vuoto
    LoadSetPoint( sectionName.c_str(), Dest.SetPoint ); //DEFAULT: controllore NULL, hys 0.0
    LoadDHTemp( sectionName.c_str(), Dest.DHTSeason ); //INVALID se non trovata.
    LoadPump( sectionName.c_str(), Dest.pPump ); //DEFAULT: controllore NULL.
    LoadWaterCtrl(sectionName.c_str(), Dest);

    LoadWaterTempProbe(sectionName.c_str(), Dest);



    LoadAero( sectionName.c_str(), Dest);


    //ordino per VAL crescente i deumidificatori
    if( !SortDeHum( Dest.DeHum ) )
    { cout << "ATTENZIONE: Aerotermi della zona " << zoneIndex << ": non c'è corrispondenza tra l'ordinamento per VAL e DELTAT!" << endl; return false; }


    //Carico zona linkata
    Dest.linkedZone = m_hIni.GetInt("LinkedZone", sectionName, -1);
    
    //Carico il timer
    Dest.timerId = m_hIni.GetInt("TimerId", sectionName, -1);

    //Aggiorno eventuali PID acqua
    UpdateZMainWaterValveSettings(Dest);
    return true;
}

//////////////////////////////////////////////////
//                                              //
//  LoadAero                                    //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::LoadAero(CString sectionName, t_FloorZone& Dest)
{
    unsigned int nOfAero = 0;
    t_Aerotherm lAero;

    nOfAero = m_hIni.GetInt("nOfAero", sectionName);

    for (unsigned int aeroIndex = 0; aeroIndex < nOfAero; aeroIndex++)
    {
        CString aeroKey = CString("Aero")+(aeroIndex+1);
        CString aeroConfiguration = m_hIni.GetString(aeroKey,sectionName,"");
        lAero.init();

        if (aeroConfiguration.size() < 1)
        {
            cout<<"ATTENZIONE!! "<<sectionName<<" manca aerotermo "<<aeroKey<<endl;cout.flush();
            sleep(2);
            continue;
        }

        lAero.number = aeroIndex+1;

        SetAeroData(&lAero, aeroConfiguration.c_str(), "HUM");
        SetAeroData(&lAero, aeroConfiguration.c_str(), "DELTAT");
        SetAeroData(&lAero, aeroConfiguration.c_str(), "ADDR");

        //Ordino i due vettori in modo crescente
        sort(lAero.humiditySPVector.begin(), lAero.humiditySPVector.end());
        sort(lAero.temperatureDeltaVector.begin(), lAero.temperatureDeltaVector.end());

        //Carico l'uso in estate/inverno
        CString season;
        m_IniLib.GetConfigParamString(aeroConfiguration.c_str(), "DHT", &season, "summer");
        lAero.seasonUsage = SStr2Enum(season.c_str());
        Dest.Aerotherms.push_back(lAero);
    }

    return true;
}


//////////////////////////////////////////////////
//                                              //
//  LoadAeroData                          //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::SetAeroData(t_Aerotherm* aero, const char* configString, const char* param)
{
    for (int i = 0; i < 3; i++)
    {
        float lFloat;
        CString dataString;
        CString subKey;
        int address;
        CVController *pCtrl;

        //CArico prima i delta ed i setpoint e per ultimo gli indirizzi dei DO: se mancano i primi sicuramente non posso usare i secondi
        subKey = CString(param)+(i+1);

        m_IniLib.GetConfigParamFloat(configString, subKey.c_str(), &lFloat, -1.0);
        m_IniLib.GetConfigParamString(configString, subKey.c_str(),&dataString,"NA");
        if ((lFloat < 0.0) || (!dataString.compare("NA")))
        {
            cout << "ATTENZIONE!! Aerotermo "<<aero->number<<", parametro "<<subKey<<" NON presente!!"<<endl;cout.flush();
            sleep(2);

                //Inserisco un parametro "finto"
            lFloat = 100.0;
        }

        if (!strcasecmp(param,"ADDR"))
        {
            address = (int)lFloat;

            pCtrl = RetrieveCtrl( address);
            if (pCtrl == 0x0)
            {
                cout << "ATTENZIONE!! Aerotermo "<<aero->number<<", parametro "<<subKey<<" Errato!!"<<endl;cout.flush();
                sleep(2);
                //Esco dal ciclo perche' non puo' esserci la velocita' 3 ma non la 2
                break;
            }
            else
            {
                aero->speedVector.push_back(CTRL_DWCAST( pCtrl, CDigitalIO ));
            }
        }
        else if (!strcasecmp(param,"HUM"))
        {
            aero->humiditySPVector.push_back(lFloat);
        }
        else if (!strcasecmp(param,"DELTAT"))
        {
            aero->temperatureDeltaVector.push_back(lFloat);
        }
    }

    return true;

}

//////////////////////////////////////////////////
//                                              //
//  LoadSingleSetPoint                          //
//                                              //
//////////////////////////////////////////////////
void CFloorCoord2::LoadSetPoint(const char* sectionZone, t_FloorSetPoint & Dest)
{
    Dest.init();

    //SetPoint: valore di default, letto da file.
    Dest.Value = m_hIni.GetFloat("TempSetPoint", sectionZone, 20.0);
    Dest.fixedValue = Dest.Value;

    //Setpoint fissi estate/inverno
    Dest.summerSetpoint = m_hIni.GetFloat("SummerSetpoint", sectionZone, 26.0);
    Dest.winterSetpoint = m_hIni.GetFloat("WinterSetpoint", sectionZone, 20.0);

    //controller
    CVController *ptrCtrl = 0x0;
    int addr = -1;

    addr = m_hIni.GetInt( "TempReg", sectionZone, -1 );
    ptrCtrl = RetrieveCtrlEx( addr, "TempSetPoint", sectionZone, false );
    Dest.Ctrl = CTRL_DWCAST( ptrCtrl, CAnalogIO ); //NULL se non trovato, NULL se downcast fallito

    //Leggo il tipo di ritaratore: famiglia e tipo
    if (Dest.Ctrl != 0x0)
    {
        CString config;
        CString family, type;

        config = m_hIni.GetString("TempRegType",sectionZone,"NA");

        //Se è uguale a NA lascio i default
        if (strcasecmp(config.c_str(),"NA"))
        {
            m_IniLib.GetConfigParamString(config.c_str(),"FAMILY",&family,"NTH");
            m_IniLib.GetConfigParamString(config.c_str(), "TYPE",&type,"ABS");

            if (!strcasecmp(family.c_str(),"NTH"))
            {
                Dest.tempRegFamily = 1;
            }
            else
            {
                Dest.tempRegFamily = 0;
            }

            if (!strcasecmp(type.c_str(),"ABS"))
            {
                Dest.tempRegType = 0;
            }
            else
            {
                Dest.tempRegType = 1;
            }
        }
    }

    //Isteresi
    float lFloat = INVALID_FLOAT;
    lFloat = m_hIni.GetFloat( "TempHyst", sectionZone, INVALID_FLOAT );
    if( lFloat == INVALID_FLOAT || lFloat < 0.0 )
    { Dest.Hysteresis = DEFAULT_HYS; }//0.0
    else { Dest.Hysteresis = lFloat; }

}

//////////////////////////////////////////////////
//                                              //
//  LoadMaintenance                              //
//                                              //
//////////////////////////////////////////////////
void CFloorCoord2::LoadMaintenance(const char* sectionZone, t_FloorMaintenance & Dest)
{
    Dest.init();

    //bool: da settare con la relativa Set
    Dest.isMaintenance = DEFAULT_ISMAINT;

    //controller
    int addr = m_hIni.GetInt( "Maintenance", sectionZone, -1 ); //tutti i controlli gà nella funzione RetrieveCtrlEx;
    CVController*  ptrCtrl = RetrieveCtrlEx( addr, "Maintenance", sectionZone, false );
    Dest.Ctrl = CTRL_DWCAST( ptrCtrl, CDigitalIO ); //NULL se non trovato, NULL se downcast fallito

    //valore
    float lFloat = INVALID_FLOAT;
    lFloat = m_hIni.GetFloat( "SummerTempMaintenance", sectionZone, INVALID_FLOAT );
    if( lFloat == INVALID_FLOAT )
    { Dest.SummerTempValue = UNSET_TEMPMAINT; }
    else { Dest.SummerTempValue = lFloat; }

    lFloat = INVALID_FLOAT;
    lFloat = m_hIni.GetFloat( "WinterTempMaintenance", sectionZone, INVALID_FLOAT );
    if( lFloat == INVALID_FLOAT )
    { Dest.WinterTempValue = UNSET_TEMPMAINT; }
    else { Dest.WinterTempValue = lFloat; }

}

void CFloorCoord2::LoadDHTemp(const char* sectionZone, e_DHTSeason & DHTSeason)
{
    DHTSeason = DHT_INVALID;

    const CString line = m_hIni.GetString( "DeHumToTemp", sectionZone );
    if( line == "" ) { return; } //nessun messaggio, può non esserci.

    DHTSeason = SStr2Enum( line.c_str() ); //invalid se non trovata.

    if( DHTSeason == DHT_INVALID )
    { cout << "ATTENZIONE: chiave DeHumToTemp nella sezione " << sectionZone << " non valida! " << endl; }
}

//////////////////////////////////////////////////
//                                              //
//  LoadSingleDeHum                             //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::LoadSingleDeHum(const char* sectionZone, unsigned int dhIndex, t_FloorDeHum & Dest)
{
    Dest.init();

    CString dH = "deHum";
    dH+=dhIndex;

    Dest.Num = dhIndex;

    const CString line = m_hIni.GetString( dH, sectionZone );
    if( line == "" )
    { cout << "Attenzione: deumidificatore n. " << dhIndex << " non trovato in " << ConfigFile << " nella sezione " << sectionZone << "!" << endl; return false; }

    float val = INVALID_FLOAT; int addr = -1;

    //soglia umidità
    m_IniLib.GetConfigParamFloat( line.c_str(), "VAL", &val, INVALID_FLOAT );
    if( val == INVALID_FLOAT )
    { cout << "Attenzione: soglia di umidità n. " << dhIndex << " non trovata in " << ConfigFile << " nella sezione " << sectionZone << "!" << endl; return false; }
    else if( val < 0.0 || val > 100.0 ) //controllo che sia una percentuale
    { cout << "Attenzione: soglia di umidità n. " << dhIndex << " non valida in " << ConfigFile << " nella sezione " << sectionZone << "!" << endl; return false; }
    else
    { Dest.Level = val; }

    //Delta di temperatura.
    m_IniLib.GetConfigParamFloat( line.c_str(), "DELTAT", &val, INVALID_FLOAT );
    if( val == INVALID_FLOAT )
    { cout << "Attenzione: delta di temperatura dell'aerotermo n. " << dhIndex << " non trovato in " << ConfigFile << " nella sezione " << sectionZone << "!" << endl; return false; }
    else if( val <= 0.0 ) //controllo che sia positivo
    { cout << "Attenzione: delta di temperatura dell'aerotermo n. " << dhIndex << " non valido in " << ConfigFile << " nella sezione " << sectionZone << "!" << endl; return false; }
    else
    { Dest.DeltaT = val; }

    //controllore
    m_IniLib.GetConfigParamInt( line.c_str(), "ADDR", &addr, -1 ); //tutti i controlli gà nella funzione RetrieveCtrlEx;
    CVController*  ptrCtrl = RetrieveCtrlEx( addr, dH.c_str(), sectionZone );
    if ( !IS_CTRL(ptrCtrl) ) { return false; }
    else
    {
        Dest.Ctrl = CTRL_DWCAST( ptrCtrl, CDigitalIO );
        if( !IS_CTRL(Dest.Ctrl) ) { return false; } //downcast fallito
    }

    return true;
}

//////////////////////////////////////////////////
//                                              //
//  LoadPid                                     //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::LoadPid(const char* sectionZone, CPIDSimple* ptrPid)
{
//    float params[3] = {10.0, 8.0, 0.0};
//    CString config;
//
//    if (!m_hIni.GetBool("PID",sectionZone, false))
//    {
//        return false;
//    }
//
//    config = m_hIni.GetString("PidParams", sectionZone,"KP:10.0,Tint:1.0,Tder:0.0");
//    m_IniLib.GetConfigParamFloat(config.c_str(),"KP", &params[0],10.0);
//    m_IniLib.GetConfigParamFloat(config.c_str(),"Tint", &params[1],2.0);
//    m_IniLib.GetConfigParamFloat(config.c_str(),"Tder", &params[2],0.0);
//    ptrPid->SetParameters(params);
//
//    //Manca il controllore analogico per l'uscita
//    int addr = m_hIni.GetInt( "PidOutput", sectionZone, -1 );
//    CVController* ptrCtrl = RetrieveCtrlEx( addr, "PidOutput", sectionZone );
//
//    if (ptrCtrl->GetDeviceType() != DEV_AIAO)
//    {
//        cout << "ATTENZIONE!! "<<sectionZone<<" campo PidOutput NON è un controllore analogico"<<endl;
//        cout << "L'esecuzione prosegue SENZA controllo acqua"<<endl;
//        sleep(2);
//        return false;
//    }
//    else
//    {
//        //Per uniformità gli ingressi/uscite dei coordinatori sono controllori anche se in
//        //questo caso basterebbe un device
//        ptrPid->SetOutputDevice( ((CAnalogIO*)ptrCtrl)->GetInputDevice() );
//    }

    return true;

}
/////////////////////////////////////////////////////////////////////////////////////
void CFloorCoord2::LoadPump(const char* sectionZone, CDigitalIO* & ptrDido)
{
    ptrDido = 0x0;

    int addr = m_hIni.GetInt( "PUMP", sectionZone, -1 ); //tutti i controlli gà nella funzione RetrieveCtrlEx;
    CVController*  ptrCtrl = RetrieveCtrlEx( addr, "PUMP", sectionZone, false );
    ptrDido = CTRL_DWCAST( ptrCtrl, CDigitalIO ); //NULL se non trovato, NULL se downcast fallito
}

//////////////////////////////////////////////////
//                                              //
//  LoadWaterCtrl                               //
//                                              //
//////////////////////////////////////////////////
void CFloorCoord2::LoadWaterCtrl(const char* sectionZone, t_FloorZone& Zone)
{
    int addr = m_hIni.GetInt( "PID", sectionZone, -1 ); //tutti i controlli gà nella funzione RetrieveCtrlEx;
    CVController* ptrCtrl = RetrieveCtrlEx( addr, "PID", sectionZone, false );


    if (ptrCtrl == 0x0)
    {
        cout<<"ATTENZIONE!! La zona numero "<<Zone.zNumber<<" ha un controllore PID NON VALIDO!!"<<endl;
        cout<<"L'ESECUZIONE PROSEGUIRA' SENZA CONTROLLO VALVOLA"<<endl;
        sleep(2);
        Zone.waterCtrlType = WATER_NONE;
    }
    else if (ptrCtrl->GetControllerType() == DEV_PIDSIMPLE)
    {
        Zone.pPid = CTRL_DWCAST( ptrCtrl, CPIDSimple ); //NULL se non trovato, NULL se downcast fallito
        if (Zone.pPid != 0x0)
        {
            Zone.waterCtrlType = WATER_PID;
        }
        else
        {
            Zone.waterCtrlType = WATER_NONE;
            cout << "Attenzione!! Zona: "<<sectionZone<<" Errore nell'acquisizione del controllore PID"<<endl;
        }
    }
    else if (ptrCtrl->GetControllerType() == DEV_C3POINT)
    {
        Zone.digitalValveCtrl = CTRL_DWCAST( ptrCtrl, C3PointCtrl ); //NULL se non trovato, NULL se downcast fallito
        if (Zone.digitalValveCtrl != 0x0)
        {
            Zone.waterCtrlType = WATER_3POINT;
        }
        else
        {
            Zone.waterCtrlType = WATER_NONE;
            cout << "Attenzione!! Zona: "<<sectionZone<<" Errore nell'acquisizione del controllore PID"<<endl;
        }
    }
    else
    {
        Zone.waterCtrlType = WATER_NONE;
    }
}

//////////////////////////////////////////////////
//                                              //
//  LoadOnOff                                   //
//                                              //
//////////////////////////////////////////////////
void CFloorCoord2::LoadOnOff(const char* sectionZone, t_FloorOnOff & Dest)
{
    Dest.init();

    //isOn
    Dest.isOn = DEFAULT_ONOFF;

    //Ctrl
    int addr = m_hIni.GetInt( "OnOff", sectionZone, -1 ); //tutti i controlli gà nella funzione RetrieveCtrlEx;
    CVController*  ptrCtrl = RetrieveCtrlEx( addr, "OnOff", sectionZone, false );
    Dest.Ctrl = CTRL_DWCAST( ptrCtrl, CDigitalIO ); //NULL se non trovato, NULL se downcast fallito

}

//////////////////////////////////////////////////
//                                              //
//  LoadValves                                  //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::LoadValves(const char* sectionZone, vector <CDigitalIO*> & vDest)
{
    vDest.clear();

    const unsigned int nOfValves = (unsigned int)m_hIni.GetInt( "nOfValves", sectionZone, 0 );
    if( nOfValves == 0 )
    { cout << "Attenzione: valvole mancanti in " << ConfigFile << " nella sezione " << sectionZone << "!" << endl; return false; }

    int addr = -1;
    CString valve;

    CVController* ptrCtrl = 0x0;
    CDigitalIO* lp = 0x0;

    //ciclo caricamento
    unsigned int index;
    for( index = 0; index < nOfValves; index++ )
        {
            valve = "valve"; valve+=(index+1);

            addr = m_hIni.GetInt( valve, sectionZone, -1 );
            ptrCtrl = RetrieveCtrlEx( addr, valve.c_str(), sectionZone );

            if( IS_CTRL(ptrCtrl) )
                {
                    lp = CTRL_DWCAST( ptrCtrl, CDigitalIO );
                    if( IS_CTRL(lp) ) { vDest.push_back( lp ); }
                } //conto il numero di controllori effettivamente trovati
            //se null, non faccio niente e continuo.
        }

    //controllo su quanti ne sono stati caricati:
    const unsigned int size = (unsigned int)vDest.size();
    if( size == 0 )
    { cout << "Attenzione: nessuna valvola caricata in " << ConfigFile << " nella sezione " << sectionZone << "!" << endl; return false; }

    if( size < nOfValves )
    { cout << "Attenzione: caricate " << size << " valvole su " << nOfValves << "!" << endl; return true; } //RITORNO TRUE!!!
                                                                                                            //semplicemente, escluse quelle sbagliate

    return true;
}

//////////////////////////////////////////////////
//                                              //
//  LoadHums                                    //
//                                              //
//////////////////////////////////////////////////
void CFloorCoord2::LoadHums(const char* sectionZone, vector <CAnalogIO*> & vDest)
{
    vDest.clear();

    const unsigned int nOfHums = (unsigned int)m_hIni.GetInt( "nOfHum", sectionZone, 0 );
    if( nOfHums == 0 ) { return; }

    int addr = -1;
    CString hum;

    CVController* ptrCtrl = 0x0;
    CAnalogIO* lp = 0x0;

    //ciclo caricamento
    unsigned int index;
    for( index = 0; index < nOfHums; index++ )
    {
        hum = "hum"; hum+=(index+1);

        addr = m_hIni.GetInt( hum, sectionZone, -1 );
        ptrCtrl = RetrieveCtrlEx( addr, hum.c_str(), sectionZone, false );

        if( IS_CTRL(ptrCtrl) )
            {
                lp = CTRL_DWCAST( ptrCtrl, CAnalogIO );
                if( IS_CTRL(lp) ) { vDest.push_back( lp ); }
            } //conto il numero di controllori effettivamente trovati
            //se null, non faccio niente e continuo.
    }

    //controllo su quanti ne sono stati caricati:
    unsigned int size = (unsigned int)vDest.size();
    if( size < nOfHums )
    { cout << "Attenzione: caricate " << size << " sonde umidità su " << nOfHums << " in " << ConfigFile << "!" << endl; }

}

//////////////////////////////////////////////////
//                                              //
//  LoadTemps                                   //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::LoadTemps(const char* sectionZone, vector <CTempCtrl*> & vDest)
{
    vDest.clear();

    const unsigned int nOfTemps = (unsigned int)m_hIni.GetInt( "nOfTemp", sectionZone, 0 );
    if( nOfTemps == 0 )
    { cout << "Attenzione: sonde temperatura mancanti in " << ConfigFile << " nella sezione " << sectionZone << "!" << endl; return false; }

    int addr = -1;
    CString temp;

    CVController* ptrCtrl = 0x0;
    CTempCtrl* lp = 0x0;

    //ciclo caricamento
    unsigned int index;
    for( index = 0; index < nOfTemps; index++ )
    {
        temp = "Temp"; temp+=(index+1);

        addr = m_hIni.GetInt( temp, sectionZone, -1 );
        ptrCtrl = RetrieveCtrlEx( addr, temp.c_str(), sectionZone );

        if( IS_CTRL(ptrCtrl) )
            {
                lp = CTRL_DWCAST( ptrCtrl, CTempCtrl );
                if( IS_CTRL(lp) ) { vDest.push_back( lp ); }
            } //conto il numero di controllori effettivamente trovati
            //se null, non faccio niente e continuo.
    }

    //controllo su quanti ne sono stati caricati:
    const unsigned int size = (unsigned int)vDest.size();
    if( size == 0 )
    { cout << "Attenzione: nessuna sonda temperatura caricata in " << ConfigFile << " nella sezione " << sectionZone << "!" << endl; return false; }

    if( size < nOfTemps )
    { cout << "Attenzione: caricate " << size << " sonde temperatura su " << nOfTemps << " in " << ConfigFile << "!" << endl; return true; } //RITORNO TRUE!!!
                                                                                                            //semplicemente, escluse quelle sbagliate
    return true;
}

//////////////////////////////////////////////////
//                                              //
//  LoadDeHums                                  //
//                                              //
//////////////////////////////////////////////////
void CFloorCoord2::LoadDeHums(const char* sectionZone, vector <t_FloorDeHum> & vDest)
{
    vDest.clear();

    const unsigned int nOfDeHums = (unsigned int)m_hIni.GetInt( "nOfDeHum", sectionZone, 0 );
    if( nOfDeHums == 0 ) { return; }

    CString deHum;

    t_FloorDeHum lStr;

    //ciclo caricamento
    unsigned int index;
    for( index = 1; index <= nOfDeHums; index++ )
    {
        //struttura già inizializzata in LoadSingleDeHum
        if( LoadSingleDeHum( sectionZone, index, lStr ) )
            { vDest.push_back( lStr ); }
            //se false, non faccio nulla e continuo;
    }

    //controllo su quanti ne sono stati caricati:
    unsigned int size = (unsigned int)vDest.size();
    if( (unsigned int)vDest.size() < nOfDeHums )
    { cout << "Attenzione: caricati " << size << " aerotermi su " << nOfDeHums << " in " << ConfigFile << "!" << endl; }
}

//////////////////////////////////////////////////
//                                              //
//  Load3PointValve                             //
//                                              //
//////////////////////////////////////////////////
//bool CFloorCoord2::Load3PointValve(const char * sectionZone, t_3Point* Dest)
//{
//    if (!m_hIni.GetBool("3Point", sectionZone, false))
//    {
//        return false;
//    }
//
//    Dest->movementTimeOut = m_hIni.GetInt("MovementTimeOut", sectionZone, 300);
//    Dest->nullZone = m_hIni.GetFloat("NullZone", sectionZone, 1.0);
//
//    CString config = m_hIni.GetString("WaterLMDLimits", sectionZone,"MAX:55.0,MIN:15.0");
//    m_IniLib.GetConfigParamFloat(config.c_str(), "MAX", &(Dest->maxTemp), 45.0);
//    m_IniLib.GetConfigParamFloat(config.c_str(), "MIN", &(Dest->minTemp), 15.0);
//
//    int addr = m_hIni.GetInt( "OpenDevice", sectionZone, -1 );
//    CVController* ptrCtrl = RetrieveCtrlEx( addr, "OpenDevice", sectionZone );
//
//    if (ptrCtrl->GetDeviceType() != DEV_DIDO)
//    {
//        cout << "ATTENZIONE!! "<<sectionZone<<" campo OpenDevice NON è un digitalInOUT"<<endl;
//        cout << "L'esecuzione prosegue SENZA controllo acqua"<<endl;
//        sleep(2);
//        return false;
//    }
//    else
//    {
//        Dest->openDevice = (CDigitalIO*)ptrCtrl;
//    }
//
//    addr = m_hIni.GetInt( "CloseDevice", sectionZone, -1 );
//    ptrCtrl = RetrieveCtrlEx( addr, "CloseDevice", sectionZone );
//
//    if (ptrCtrl->GetDeviceType() != DEV_DIDO)
//    {
//        cout << "ATTENZIONE!! "<<sectionZone<<" campo CloseDevice NON è un digitalInOUT"<<endl;
//        cout << "L'esecuzione prosegue SENZA controllo acqua"<<endl;
//        sleep(2);
//        return false;
//    }
//    else
//    {
//        Dest->closeDevice = (CDigitalIO*)ptrCtrl;
//    }
//
//    return true;
//    /*
//    int addr = m_hIni.GetInt( "3Point", sectionZone, -1 ); //tutti i controlli gà nella funzione RetrieveCtrlEx;
//    CVController*  ptrCtrl = RetrieveCtrlEx( addr, "3POINT", sectionZone, false );
//    Dest.digitalValveCtrl = CTRL_DWCAST( ptrCtrl, C3PointCtrl ); //NULL se non trovato, NULL se downcast fallito
//     */
//}

//****************************//


//*** ConnectControllers ***//
//////////////////////////////////////////////////
//                                              //
//  ConnectControllers                          //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::ConnectControllers(void)
{

    CHECK_INI_BOOL()


    //controllo puntatore alla NET//
    if ( m_NetPtr == 0x0 )
    { cout << "Attenzione il coordinatore di indirizzo: " << m_Address << " NON è connesso al conewireNET!" <<endl;  return false; }

    //*** Carico i valori dalle chiavi ***//
    //carico le chiavi contenute in [COMMON]
    LoadCommons();

    //Carico le singole Zone//
    //tutti i controlli e i messaggi già inclusi nella LoadZones()
    m_isConnected = LoadZones();
//     cout<<"LoadZones OK"<<endl;cout.flush();
    //---------------------//
    //TODO da inizializzare ???

    return m_isConnected;
}
//*************************//

//////////////////////////////////////////////////
//                                              //
//  Destroy                                     //
//                                              //
//////////////////////////////////////////////////
void CFloorCoord2::Destroy(void)
{
    m_ZoneVector.clear();

    m_ZonesCount = 0;
    m_isVectorLoaded = false;

    m_isConnected = false;

    m_isSummer = DEFAULT_SEASON;
    m_pSummerSwitch = 0x0;
}

//***Ricerca nei vettori***//
//////////////////////////////////////////////////
//                                              //
//  GetZone                                     //
//                                              //
//////////////////////////////////////////////////
t_FloorZone* CFloorCoord2::GetZone(unsigned int zoneNum, unsigned int* vectorPos)
{
    if( !m_isVectorLoaded ) return 0x0;

    const unsigned int size = (unsigned int)m_ZoneVector.size();
    if( size == 0 ) return 0x0;

    t_FloorZone *ptrZone = 0x0, *local = 0x0;
    unsigned int pos = 0;

    for( pos = 0; pos < size; pos++ )
    {
        local = &(m_ZoneVector[pos]); //attenzione: la .at ritorna un reference, il suo indirizzo non è temporaneo!!! Posso farlo!
        if( local->zNumber == zoneNum )
        {
            ptrZone = local;
            if( vectorPos != NULL ) { *vectorPos = pos; }
            break;
        }
    }

    return ptrZone; //NULL se non trovato (è stato inizializzato, se non trovato non sono mai entrato nell'if e non è mai stato settato)
}

//////////////////////////////////////////////////
//                                              //
//  GetDeHum                                    //
//                                              //
//////////////////////////////////////////////////
t_FloorDeHum* CFloorCoord2::GetDeHum(unsigned int zoneNum, unsigned int num, unsigned int* vectorPos)
{
    t_FloorZone* pZone = GetZone( zoneNum );
    if( pZone == 0x0 ) { return 0x0; }

    const unsigned int size = (unsigned int)pZone->DeHum.size();
    if( size == 0 ) return 0x0;

    t_FloorDeHum *ptrDH = 0x0, *local = 0x0;
    unsigned int pos = 0;

    for( pos = 0; pos < size; pos++ )
    {
        local = &(pZone->DeHum[pos]); //attenzione: la .at ritorna un reference, il suo indirizzo non è temporaneo!!! Posso farlo!
        if( local->Num == num )
        {
            ptrDH = local;
            if( vectorPos != NULL ) { *vectorPos = pos; }
            break;
        }
    }

    return ptrDH;
}
//*********************//

//***SET DELLE VARUABILI DA FUNZIONE***//
//////////////////////////////////////////////////
//                                              //
//  SetSummer                                   //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::SetSummer(bool isSummer, bool backup)
{
    //variabile
    m_isSummer = isSummer;

    //Setto il setpoint di tutte le zone
    for (unsigned int i = 0; i < m_ZoneVector.size(); i++)
    {
        if (m_isSummer)
        {
            SetSetPoint(m_ZoneVector[i].zNumber, m_ZoneVector[i].SetPoint.summerSetpoint);
        }
        else
        {
            SetSetPoint(m_ZoneVector[i].zNumber, m_ZoneVector[i].SetPoint.winterSetpoint);
        }

        UpdateZMainWaterValveSettings(m_ZoneVector[i]);
    }

    //file
    if(backup)
    {
        CHECK_INI_BOOL()

        if( !( m_hIni.ExistSection( "COMMON" ) && m_hIni.SetBool( "Summer", isSummer, "", "COMMON" ) && m_hIni.Save() ) )
        { cout << "Attenzione: impossibile aggiornare Summer nella sezione COMMON in " << ConfigFile << "!" << endl; return false; }
    }

    return true;
}

//////////////////////////////////////////////////
//                                              //
//  SetOnOff                                    //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::SetOnOff(unsigned int zone, bool on)
{
    //solo variabile//
    t_FloorZone* pZone = GetZone( zone );

    if( pZone == 0x0 )
    { cout << "Attenzione: zona " << zone << " non trovata nel vettore. Impossibile settare OnOff." << endl; return false; }
    else
    { pZone->OnOff.isOn = on; return true; }

}


//////////////////////////////////////////////////
//                                              //
//  SetIsMaintenance                            //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::SetIsMaintenance(unsigned int zone, bool on)
{
    //solo variabile//
    t_FloorZone* pZone = GetZone( zone );

    if( pZone == 0x0 )
    { cout << "Attenzione: zona " << zone << " non trovata nel vettore. Impossibile settare Maintenance." << endl; return false; }

    //ATTENZIONE: controllo che sia impostato il valore, altrimenti non setto a true.
    t_FloorMaintenance& Maintenance = pZone->Maintenance;
    if( !on ) //posso settare indipendentemente dal valore della temperatura.
    { Maintenance.isMaintenance = false; return true; }

    //altrimenti, controllo il valore della temperatura.
    if( !Maintenance.IsValue(m_isSummer) )
    { cout << "Attenzione: zona " << zone << ", valore della temperatura di maintenimento non impostata. Impossibile attivare il maintenimento." << endl; return false; }

    Maintenance.isMaintenance = true;

    return true;
}

//////////////////////////////////////////////////
//                                              //
//  SetTempMaintenance                          //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::SetTempMaintenance(unsigned int zone, float SummerTemp, float WinterTemp, bool backup)
{
    bool setVar = false, setFile = false;

    //solo variabile//
    t_FloorZone* pZone = GetZone( zone );

    if( pZone == 0x0 )
    { cout << "Attenzione: zona " << zone << " non trovata nel vettore. Impossibile settare TempMaintenance." << endl; setVar = false; }
    else
    { pZone->Maintenance.SummerTempValue = SummerTemp; pZone->Maintenance.WinterTempValue = WinterTemp; setVar = true; }

    //file//
    if( backup )
    {
        CString sZone = "ZONE"; sZone+=zone;

        setFile = false;
        if( m_isIniLoaded && m_hIni.ExistSection( sZone ) )
        {
            if( m_hIni.SetFloat( "SummerTempMaintenance", SummerTemp, "", sZone ) &&
                m_hIni.SetFloat( "WinterTempMaintenance", WinterTemp, "", sZone ) &&
                m_hIni.Save()
              )
            {
                setFile = true;
            }
        }

        if( !setFile )
        { cout << "Attenzione: impossibile settare TempMaintenance nella zona " << zone << " in " << ConfigFile << "!" << endl; }

    }
    else { setFile = true; } //mi serve che sia true per l'and della return

    return ( setVar && setFile );
}

//////////////////////////////////////////////////
//                                              //
//  SetSetPoint                                 //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::SetSetPoint(unsigned int zone, float value)
{
    //solo variabile//
    t_FloorZone* pZone = GetZone( zone );

    if( pZone == 0x0 )
    { cout << "Attenzione: zona " << zone << " non trovata nel vettore. Impossibile settare SetPoint." << endl; return false; }
    else
    {
        pZone->SetPoint.Value = value;
        pZone->SetPoint.fixedValue = value;

        pZone->CurrentSetPoint = value;

//        UpdateZMainWaterValveSettings(*pZone);

        CHECK_INI_BOOL()

        CString zoneString = "ZONE";
        zoneString+=pZone->zNumber;
        m_hIni.Reload();
        m_hIni.SetFloat("TempSetPoint",value,"",zoneString);
        m_hIni.Save();
        return true;
    }

}

bool CFloorCoord2::SetAdvancedSetpoints(unsigned int zone, float *setpoints)
{
    //solo variabile//
    t_FloorZone* pZone = GetZone( zone );

    if( pZone == 0x0 )
    { cout << "Attenzione: zona " << zone << " non trovata nel vettore. Impossibile settare SetPoint." << endl; return false; }
    else
    {
        //L'ordine e' il seguente:
        //summerSetpoint, winterSetpoint, waterSummerSetpoint, waterWinterSetpoint
        pZone->SetPoint.summerSetpoint = setpoints[0];
        pZone->SetPoint.winterSetpoint = setpoints[1];
        pZone->waterSummerSetpoint = setpoints[2];
        pZone->waterWinterSetpoint = setpoints[3];

        CHECK_INI_BOOL()

        CString zoneString = "ZONE";
        zoneString+=pZone->zNumber;
        m_hIni.Reload();

        m_hIni.SetFloat("SummerSetpoint", setpoints[0],"",zoneString);
        m_hIni.SetFloat("WinterSetpoint", setpoints[1],"",zoneString);
        m_hIni.SetFloat("WaterSummerSetpoint", setpoints[2],"",zoneString);
        m_hIni.SetFloat("WaterWinterSetpoint", setpoints[3],"",zoneString);

        m_hIni.Save();

        //Aggiorno eventuali PID
        UpdateZMainWaterValveSettings(*pZone);
        return true;
    }
}

//////////////////////////////////////////////////
//                                              //
//  SetDeHumVal                                 //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::SetDeHumVal(unsigned int zone, unsigned int dhNum, float value, bool backup)
{
    //prima di tutto, controllo che sia un valore sensato//
    if( value < 0.0 || value > 100.0 ) return false;

    bool setVar = false, setFile = false;

    //variabile//
    t_FloorDeHum* ptrDH = GetDeHum( zone, dhNum );
    if( ptrDH == 0x0 )
    { cout << "Attenzione: aerotermo " << dhNum << " della zona " << zone << " non trovato nel vettore. Impossibile settare la soglia. " << endl; setVar = false; }
    else
    { ptrDH->Level = value; setVar = true; }

    //file//
    if( backup )
    {
        CString sZone = "ZONE"; sZone+=zone;
        CString Key = "deHum"; Key+=dhNum;

        setFile = false;
        if( m_isIniLoaded && m_hIni.ExistSection( sZone ) )
        {
            //leggo la stringa relativa al deumidificatore scento.
            string line = m_hIni.GetString( Key, sZone, "" );
            if( line != "" )
            {
                //creo una stringa da sostituire al campo VAL
                char newVal[8]; memset( newVal, 0, 8*sizeof(char) );
                sprintf( newVal, "%.1f", value );

                if( m_IniLib.SetConfigParamString( &line, "VAL", newVal ) && m_hIni.SetValue( Key, line, "", sZone ) && m_hIni.Save() )
                { setFile = true; }
            }
        }

        if( !setFile )
        { cout << "Attenzione: impossibile aggiornare la soglia dell'aerotermo " << dhNum << " della zona " << zone << " in " << ConfigFile << "!" << endl; }

    }
    else { setFile = true; } //mi serve che sia true per l'and della return

    return ( setVar && setFile );

}

//////////////////////////////////////////////////
//                                              //
//  SetDeHumDeltaT                              //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::SetDeHumDeltaT(unsigned int zone, unsigned int dhNum, float value, bool backup )
{
    bool setVar = false, setFile = false;

    //variabile//
    t_FloorDeHum* ptrDH = GetDeHum( zone, dhNum );
    if( ptrDH == 0x0 )
    { cout << "Attenzione: aerotermo " << dhNum << " della zona " << zone << " non trovato nel vettore. Impossibile settare il delta di temperatura. " << endl; setVar = false; }
    else
    { ptrDH->DeltaT = value; setVar = true; }

    //file//
    if( backup )
    {
        CString sZone = "ZONE"; sZone+=zone;
        CString Key = "deHum"; Key+=dhNum;

        setFile = false;
        if( m_isIniLoaded && m_hIni.ExistSection( sZone ) )
        {
            //leggo la stringa relativa al deumidificatore scento.
            string line = m_hIni.GetString( Key, sZone, "" );
            if( line != "" )
            {
                //creo una stringa da sostituire al campo DELTAT
                char newVal[8]; memset( newVal, 0, 8*sizeof(char) );
                sprintf( newVal, "%.1f", value );

                if( m_IniLib.SetConfigParamString( &line, "DELTAT", newVal ) && m_hIni.SetValue( Key, line, "", sZone ) && m_hIni.Save() )
                { setFile = true; }
            }
        }

        if( !setFile )
        { cout << "Attenzione: impossibile aggiornare il delta di temperatura dell'aerotermo " << dhNum << " della zona " << zone << " in " << ConfigFile << "!" << endl; }

    }
    else { setFile = true; } //mi serve che sia true per l'and della return

    return ( setVar && setFile );
}

//*************************//

//***Ordinamento deumidificatori***//
//////////////////////////////////////////////////
//                                              //
//  VALPredicate                                //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::VALPredicate( const t_FloorDeHum & dh1, const t_FloorDeHum & dh2 )
{
    return ( dh1.Level < dh2.Level );
}

//////////////////////////////////////////////////
//                                              //
//  SortDeHum                                   //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::SortDeHum( vector<t_FloorDeHum> & Vector )
{
    const unsigned int size = (unsigned int)Vector.size();
    if( size <= 1 ) { return true; }//inutile ordinare
                                    //ATTENZIONE: esco con true, in pratica ho già ordinato..

    sort( Vector.begin(), Vector.end(), VALPredicate );

    //ora verifico però che anche l'ordine dei DeltaT sia rispettato.
    bool verified = true; //parto con true, diventa false se una disuguaglianza non è rispettata, quindi break.
    float LastDelta = -1.0;//primo ciclo, sempre maggiore.
    for( unsigned int index = 0; index < size; index++ )
    {
        if( !( Vector[ index ].DeltaT >= LastDelta ) ) { verified = false; break; }
        LastDelta = Vector[ index ].DeltaT;
    }

    return verified;
}
//*****************************//


//****************** UPDATE **********************//
//////////////////////////////////////////////////
//                                              //
//  Update                                      //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::Update(bool updateData)
{
    if (m_DebugLevel)
    {
        cout << "Aggiornamento FloorCoord indirizzo: "<<m_Address<<endl;
    }


    COneWireEngine *engPtr = reinterpret_cast<COneWireEngine*>(m_EnginePtr);

    CHECK_CONNECTED_BOOL()
    CHECK_LOADED_BOOL()
    CHECK_INI_BOOL()

    //ciclo sulle zone//
    const unsigned int size = (unsigned int)m_ZoneVector.size();
    if( size == 0 ) { return false; }

    //aggiorno m_isSummer//
    m_isSummer = RetrieveIsSummer();
    //-------------------//

    unsigned int index = 0, updated = 0; //updated: var di controllo per contare i bool
    t_FloorZone *pZone = 0x0;
    bool globalNightKeeping = false; //Flag del mantenimento globale

    //Controllo se il timer mi manda in mantenimento o meno:
    if (m_UseTimer && IsTimerEnabled())
    {
        //Vado in mentenimento quando il timer dice off!!!
        globalNightKeeping = !((bool)GetValFromTimer());
        
    }

    for( index = 0; index < size; index++ )
    {
        bool localNightKeeping = globalNightKeeping;

        //Controllo se ci sono dei comandi da eseguire
        engPtr->CheckForCommands2();
        
        pZone = &(m_ZoneVector[index]);
        
        //Controllo se il timer è personalizzato per questa zona
        if (pZone->timerId > 0)
        {
            if (IsTimerEnabled(pZone->timerId))
            {
                localNightKeeping = GetValFromTimer(pZone->timerId);
            }
        }
            
        SetIsMaintenance(pZone->zNumber, localNightKeeping);
        
        if( UpdateZone( *pZone ) ) 
        {
            if (m_DebugLevel)
            {
                cout << "Zona Numero "<<pZone->zNumber<<" Temp:"<<pZone->CurrentTemp<<" Hum:"<<pZone->CurrentHum<<
                        " Setpoint:"<<pZone->CurrentSetPoint<<" Summer:"<<m_isSummer<<" Stato:"<<pZone->OnOff.isOn<<" Attiva:"<<m_IsActive<<endl;
            }

            if (engPtr->CheckInterfacePortsForConnection())
            {
                Cmd message("DEVICE");
                message.putValue("TYPE", "FloorCoord");
                message.putValue("ADDRESS", m_Address);
                message.putValue("ZONE", pZone->zNumber);
                message.putValue("TEMP", pZone->CurrentTemp);
                message.putValue("SETPOINT", pZone->CurrentSetPoint);
                message.putValue("HUM",pZone->CurrentHum);
                message.putValue("SUMMER", m_isSummer);
                message.putValue("STATE", pZone->IsTempOn);
                message.putValue("ISON", pZone->OnOff.isOn);
                message.putValue("ISACTIVE",m_IsActive);

                engPtr->WriteOnInterfacePorts(message.getXMLValue().c_str(), (int)message.getXMLValue().size());
            }
            updated++; 
        }
        else
        { 
            cout << "Attenzione: Update FALLITA per la zona " << pZone->zNumber << "!" << endl; 
        }
    }

    //Qui devo controllare di spegnere le pompe che sono condivise tra piu' zone.
    //E di regolare le valvole condivise
    if (m_IsActive)
    {
        UpdatePumpState();
        UpdateWaterCtrlState();
    }

    if( updated < (size-1) ) { return false; }
    return true;
}

//////////////////////////////////////////////////
//                                              //
//  UpdateZone                                  //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::UpdateZone( t_FloorZone & Zone )
{
    if (!m_IsActive)
    {
        return true;
    }
    
    if( !RetrieveOnOff( Zone ) )
    {
        bool isOff = TurnZoneOff( Zone );//ho settato anche le IsTempOn e IsHumOn singolarmente qui dentro
        return isOff;
    }

    //Aggiorno le grandezze
    if( IsHum( Zone ) )
    {
        Zone.CurrentHum = RetrieveMeanHum( Zone );
    }
    else
    {
        Zone.CurrentHum = ANALOG_ERRVAL;
    }

    bool UpdatedTemp = false;
    if ( RetrieveMeanTemp(Zone) != DATA_NOT_UPDATED_FLOAT)
    {
        //*** TEMPERATURA ***//
        //aggiornamento temperature//
        UpdatedTemp = UpdateZTemp( Zone );
    }

    //verifico se le valvole sono accese o spente.//
    //faccio comunque un ciclo di GetState, la UpdateZTemp potrebbe essere fallita o potrei non aver fatto nulla (se sono dentro l'isteresi) e quindi non sapere se sono accese o spente.
    bool ValvesON = CheckValvesOn( Zone.Valves );

    Zone.IsTempOn = ( ValvesON);
    //*******************//

    //*** UMIDITA' ***//
    bool UpdatedHum = false;

//     //ATTENZIONE!//
//     // 1) Se la parte di temperatura è a posto, Spengo gli aerotermi
    if( Zone.CurrentTemp < Zone.CurrentSetPoint - 1.0 ) { UpdatedHum = TurnAeroOff( Zone ); }
//     // 2) Se invece la parte di temperatura è accesa, allora PRIMA accendo gli aerotermi in funzione della temperatura.
    else if (Zone.IsTempOn)
    {
        //aerotermi per temperatura.
        bool NothingDone = true; //questa variabile mi dice se, per controllare la temperatura, ho acceso qualche aerotermo no;
                                 //è true se non è stato acceso nulla e quindi passo all'umidità.
        bool UpdatedDHT = UpdateZDHT( Zone, NothingDone );

        if( !UpdatedDHT ) { UpdatedHum = false; } //fallita l'accensione degli aerotermi, a prescindere che si tratti di temperatura o umidità, esco.
        else //Andata a buon fine, ora però DEVO VERIFICARE DI AVER EFFETTIVAMENTE ACCESO QUALCOSA PER LA TEMPERATURA
            //Altrimenti, passo all'umidità.
        {
            if( NothingDone ) { UpdatedHum = UpdateZHum( Zone ); } //non ho acceso nulla per la temperatura, ripasso all'umidità.
            else { UpdatedHum = UpdatedDHT; } //quindi, true.
        }
    }
    else
    {
        UpdatedHum = true;
    }

    //verifico se almeno un aerotermo è acceso
    bool DeHumON = CheckDeHumOn( Zone.DeHum );
    Zone.IsHumOn = DeHumON;

    //return ( UpdatedTemp && UpdatedPump && UpdatedHum );
    return ( UpdatedTemp && UpdatedHum );
}

bool CFloorCoord2::TurnPump( t_FloorZone & Zone, bool newState )
{
    CDigitalIO* pPump = Zone.pPump;
    if( !IS_CTRL( pPump ) ) { return true; } //non c'è, quindi ok, aggiornata.
    else
    {
        if( pPump->SetState( newState, false ) ) { return true; }
        else
        { cout << "Attenzione: impossibile accendere/spegnere la pompa nella zona " << Zone.zNumber << "!" << endl; return false; } //questo invece è un fallimento.
    }
}

//////////////////////////////////////////////////
//                                              //
//  TurnValvesOff                               //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::TurnValvesOff( t_FloorZone & Zone )
{
    //valvole//
    vector<CDigitalIO*>& VV = Zone.Valves;
    const unsigned int sizeV = VV.size();

    if( sizeV == 0 ) { return false; } //false: devono esserci, se non ci sono è un errore.

    unsigned int i = 0, done = 0;

    CDigitalIO* lpCtrl = 0x0;
    done = 0;
    for( i = 0; i < sizeV; i++ )
    {
        lpCtrl = VV.at(i);
        if( IS_CTRL( lpCtrl ) )
        { if( lpCtrl->SetState( false, false ) ) { done++; } }
    }

    if( done == 0 )
    { cout << "Attenzione: impossibile spegnere le valvole nella zona " << Zone.zNumber << "!" << endl; return false; }
    else if( done < sizeV )
    { cout << "Attenzione: spente " << done << " valvole su " << sizeV << " nella zona " << Zone.zNumber << "!" << endl; return true; }

    return true;

}

//////////////////////////////////////////////////
//                                              //
//  TurnDeHumsOff                               //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::TurnAeroOff( t_FloorZone & Zone )
{
    //aerotermi//
    vector<t_Aerotherm>& DHV = Zone.Aerotherms;
    const unsigned int sizeDH = DHV.size();

    if( sizeDH == 0 ) { return true; } //ATTENZIONE: true perchè possono anche non esserci, quindi non è un fallimento.

    unsigned int i = 0,j=0, done = 0;


    done = 0;
    for( i = 0; i < sizeDH; i++ )
    {
        vector<CDigitalIO*>& speedVector = DHV.at(i).speedVector;
        for (j = 0; j<speedVector.size(); j++)
        {
            CDigitalIO* lpCtrl = 0x0;

            lpCtrl = speedVector.at(j);
            if( IS_CTRL( lpCtrl ) )
            {
                if( lpCtrl->SetState( false, false ) )
                {
                    done++;
                }
            }
        }

        //Per consistenza imposto anche lo stato
        DHV.at(i).humSpeedState = DHV.at(i).tempSpeedState = 0;
    }

    if( done == 0 )
    { cout << "Attenzione: impossibile spegnere gli aerotermi nella zona " << Zone.zNumber << "!" << endl;return false; }
    else if( done < sizeDH )
    { cout << "Attenzione: spenti " << done << " aerotermi su " << sizeDH << " nella zona " << Zone.zNumber << "!" << endl; return true; }

    return true;

}

//////////////////////////////////////////////////
//                                              //
//  TurnZoneOff                                 //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::TurnZoneOff( t_FloorZone & Zone )
{
    //qui setto anche IsTempOn e IsHumOn della zona.

    bool toValves = false, toDeHums = false;

    //valvole//
    toValves = TurnValvesOff( Zone );//messaggistica integrata nella funzione.

    if( toValves) { Zone.IsTempOn = false; }

    //aerotermi//
    toDeHums = TurnAeroOff( Zone );//messaggistica integrata nella funzione.

    //SETTO IsOnTemp//
    if( toDeHums ) { Zone.IsHumOn = false; }

    Zone.OnOff.isOn = false;
    
    //16/09/2009 -- Quando la zona è spenta chiudo anche le valvole di zona per evitare che chiller o altre 
    //pompe spinganop comunque acqua nella zona
    
    

    return ( toValves && toDeHums );

}


//////////////////////////////////////////////////
//                                              //
//  UpdateZTemp                                 //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::UpdateZTemp( t_FloorZone & Zone )
{
    if( Zone.CurrentTemp == TEMP_ERRVAL ) { return false; } //è un fallimento.

    float SetPoint = INVALID_FLOAT, TempMaint = UNSET_TEMPMAINT;
    //distinguo maintenimento o no.
    const bool isMaintenance = RetrieveIsMaintenance( Zone, &TempMaint );
    if( isMaintenance )
    {
        if( TempMaint == UNSET_TEMPMAINT ) //errore: mentenimento ma il valore di temperatura non è settato, esco.
        { cout << "Attenzione: maintenimento impostato ma temperatura non settata nella zona " << Zone.zNumber << "!" << endl; return false; }
        else { SetPoint = TempMaint; } //il mio setpoint diventa la temperatura di maintenimento.
    }
    else
    { SetPoint = RetrieveSetPoint( Zone ); }

    //Setto CurrentSetPoint della zona//
    Zone.CurrentSetPoint = SetPoint;
    //--------------------------------//

    const float Hys = Zone.SetPoint.Hysteresis;

    //controllo sulle temperature//
    short TurnValves = CheckHysteresis( m_isSummer, Zone.CurrentTemp, SetPoint, Hys );

    //Controllo di non essere oltre la condensa
    float waterTemp;
    if (IS_CTRL(Zone.waterTempProbe) )
    {
         waterTemp = ReadController(Zone.waterTempProbe);
    }
    else
    {
        //La setto ad un valore ininfluente per il controllo
        waterTemp = 100.0;
    }

    //Controllo il punto di rugiada, solo d'estate
    //TODO cosa fare se non leggo la temperatura o l'umidita' ?
    //Per ora skippo il controllo
    float dewTemp;
    if ( m_isSummer && (Zone.CurrentTemp != TEMP_ERRVAL) && (Zone.CurrentHum != ANALOG_ERRVAL))
    {
        CalcDewPoint(Zone.CurrentTemp, Zone.CurrentHum, &dewTemp);
        //Aggiungo 1 grado per tenermi lontano dal punto di rugiada effettivo
        if (waterTemp < dewTemp+1.0)
        {
            //La temperatura dell'acqua e' troppo bassa: spengo le valvole
            TurnValves = HYS_DO_OFF;
        }
    }

    bool newState = false;
    if( TurnValves == HYS_DO_ON )
    {
        newState = true;
    }
    else if( TurnValves == HYS_DO_OFF )
    {        
        newState = false;
    }
    else if ( TurnValves == HYS_DO_NONE)
    {
        return true;
    }

    vector<CDigitalIO*> & VVector = Zone.Valves;
    const unsigned int size = VVector.size();
    if( size == 0 ) { return false; }

    //Mi assicuro che i PID della valvola di zona siano attivi
//    if (Zone.digitalValveCtrl != 0x0) {
//        Zone.digitalValveCtrl->TurnOn(true);
//    }
//    else if (Zone.pPid != 0x0)
//    {
//        Zone.digitalValveCtrl->TurnOn(true);
//    }
    
    //Ciclo sulle valvole//
    unsigned int index = 0, done = 0;
    CDigitalIO* lPtr = 0x0;

    for( index = 0; index < size; index++ )
    {
        lPtr = VVector.at(index);
        if( IS_CTRL( lPtr ) &&  lPtr->SetState( newState, false ) ) { done++; }
    }

    if( done == 0 )
    { cout << "ATTENZIONE: impossibile accendere/spegnere le testine nella zona n. " << Zone.zNumber << "!" << endl; return false; }

    if( done < size )
    { cout << "ATTENZIONE: accese/spente solo " << done << " testine su " << size << " nella zona n. " << Zone.zNumber << "!" << endl; return true; }

    return true;
}

//////////////////////////////////////////////////
//                                              //
//  IsHum                                       //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::IsHum( const t_FloorZone & Zone )
{
    if( ( (unsigned int)Zone.Hum.size() ) == 0 ) { return false; }
    //if( ( (unsigned int)Zone.Aerotherms.size() ) == 0 )  { return false; }
    return true;
}

//////////////////////////////////////////////////
//                                              //
//  CheckValvesOn                               //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::CheckValvesOn( const vector <CDigitalIO*> & Valves )
{
    const unsigned int size = (unsigned int)Valves.size();
    if( size == 0 ) { return false; } //comunque non devo accendere la pompa e la IsTempOn sarà false.

    bool found = false;

    CDigitalIO* lPtr = 0x0;
    for( unsigned int i = 0; i < size; i++ )
    {
        lPtr = Valves[i];
        if( IS_CTRL( lPtr ) )
        {
            if( lPtr->GetState(false) ) //TODO: attenzione, verificare se è corretto passare false dopo che è stata chiamata la set!!!
            { found = true; break; }
        }
    }

    return found;

}

//////////////////////////////////////////////////
//                                              //
//  CheckPumpOn                                 //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::CheckPumpOn( CDigitalIO* pPump )
{
    if( !IS_CTRL( pPump ) ) { return false; }

    return ( pPump->GetState( false ) );
}

//////////////////////////////////////////////////
//                                              //
//  CheckDeHumOn                                //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::CheckDeHumOn( const vector <t_FloorDeHum> & DeHum )
{
    const unsigned int size = (unsigned int)DeHum.size();
    if( size == 0 ) { return false; }

    bool found = false;

    CDigitalIO* lPtr = 0x0;
    for( unsigned int i = 0; i < size; i++ )
    {
        lPtr = DeHum[i].Ctrl;
        if( IS_CTRL( lPtr ) )
        {
            if( lPtr->GetState(false) ) //TODO: attenzione, verificare se è corretto passare false dopo che è stata chiamata la set!!!
            { found = true; break; }
        }
    }

    return found;
}


//////////////////////////////////////////////////
//                                              //
//  UpdateZHum                                  //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::UpdateZHum( t_FloorZone & Zone )
{
    //ATTENZIONE QUESTA FUNZIONE DEVE ESSERE SEMPRE CHIAMATA SOLO SE GLI AEROTERMI NON SONO
    //STATI TOCCATI DALLA TEMPERATURA

    //ATTENZIONE: possono non esserci sia le sonde che i deumidificatori!
    //Ma se non ci sono non è un errore, esco con true;
    if( !IsHum( Zone ) ) { return true; }

    //Se non e' estate spengo gli aerotermi perche' potrei avere delle inconsistenze di stato quando passo da estate a inverno
    if( !m_isSummer )
    {
        return TurnAeroOff(Zone);
    }

    //Alessandro Mirri
    //Spostata la lettura dell'umidita' nella UpdateZone
    if( Zone.CurrentHum == ANALOG_ERRVAL ) { return false; } //è un fallimento.

    //bool updated = TurnDeHumByHum( Zone, Zone.CurrentHum );
    bool updated = TurnAeroByHum(Zone, Zone.CurrentHum);
    return updated;
}

//////////////////////////////////////////////////
//                                              //
//  UpdateZDHT                                  //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::UpdateZDHT( t_FloorZone & Zone, bool& NothingDone )
{
    //esco con true se non c'è nulla da fare, perchè in pratica ho aggiornato, non è un fallimento.

    NothingDone = true; //ovvero, se esco prima della TurnDeHumByTemp, certamente non ho agito sui deumidificatori.
    //prima controllo se usarli o no. Se non devo usarli, l'update ritorna true, non è un fallimento.
    if( !IsDHTActive( Zone ) ) { return true; }//esclusi never e invalid (se la stringa manca nell'INI, in ogni caso non faccio nulla e proseguo)


    bool updated = TurnAeroByTemp(Zone, m_isSummer, &NothingDone );
    return updated;
}

//////////////////////////////////////////////////
//                                              //
//  UpdateZPidCtrlVariable                       //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::UpdateZWaterPid(t_FloorZone & Zone)
{
    bool retVal = false;
//    float waterTemp = INVALID_FLOAT;
//
//    if (!IS_CTRL(Zone.waterTempProbe))
//    {
//        return true; //Non ho la sonda dell'acqua
//    }
//
//    if ((waterTemp == INVALID_FLOAT) || (waterTemp == DATA_NOT_UPDATED_FLOAT))
//    {
//        return false; //Errore in lettura di temperatura
//    }
//
//    //Controllo che il PID sia valido
//    //Tolta qualsiasi logica che fa variare la valvola in funzione del setpoint ambiente:
//    //non ha senso a causa dell'inerzia del pavimento. Al massimo ci può essere una curva climatica esterna
//    //che varia il setpoint
//    if (Zone.waterCtrlType == WATER_PID)
//    {
//        retVal = Zone.pPid.Update2(false);
//    }
//    else if (Zone.waterCtrlType == WATER_3POINT)
//    {
//        retVal = Zone.digitalValveCtrl.Update2(false);
//    }

    return retVal;
}

//////////////////////////////////////////////////
//                                              //
//  UpdateZMainWaterValveSettings                       //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::UpdateZMainWaterValveSettings(t_FloorZone & Zone)
{
    bool retVal = false;
    float newSetpoint = 0.0;

    if (m_isSummer)
    {
        newSetpoint = Zone.waterSummerSetpoint;
    }
    else
    {
        newSetpoint = Zone.waterWinterSetpoint;
    }

    if (Zone.waterCtrlType == WATER_PID)
    {
        //Il PID controlla l'acqua ma se è un pidlmd usa la sonda acqua come limite e quella ambiente come regolazione
        Zone.pPid->SetSummer(m_isSummer);
        Zone.pPid->SetSetPoint(&newSetpoint, 1);

        retVal = true;
    }
    else if (Zone.waterCtrlType == WATER_3POINT)
    {
        Zone.digitalValveCtrl->SetSummer(m_isSummer);
        Zone.digitalValveCtrl->SetSetpoint(newSetpoint);
        retVal = true;

    }
    else
    {
        retVal = true;
    }

    return retVal;
}
//*******************************************************//

//////////////////////////////////////////////////
//                                              //
//  ReadController                              //
//                                              //
//////////////////////////////////////////////////
float CFloorCoord2::ReadController( CVController* pCtrl )
{
    if( !IS_CTRL(pCtrl) ) return INVALID_FLOAT;

    e_DeviceType Type = pCtrl->GetControllerType();
    if( Type == DEV_NONE ) return INVALID_FLOAT;

    float retValue = INVALID_FLOAT;
    //distingui i singoli casi//

    if( Type == DEV_DIDO )//CDigitalIO
    {
        CDigitalIO* lp = 0x0;
        lp = CTRL_DWCAST( pCtrl, CDigitalIO );
        if( !IS_CTRL( lp ) ) { return INVALID_FLOAT; }

        bool ret = lp->GetState(false);
        retValue = static_cast<float>(ret);
    }
    else if( Type == DEV_AIAO ) //CAnalogIO
    {
        CAnalogIO* lp = 0x0;
        lp = CTRL_DWCAST( pCtrl, CAnalogIO );
        if( !IS_CTRL( lp ) ) { return INVALID_FLOAT; }
        
        retValue = lp->GetValue(false);
    }
    else if( Type == DEV_TEMPCTRL )
    {
        CTempCtrl* lp = 0x0;
        lp = CTRL_DWCAST( pCtrl, CTempCtrl );
        if( !IS_CTRL( lp ) ) { return INVALID_FLOAT; }

        if (lp->IsDataValid())
        {
            retValue = lp->GetLastTemp();
        }
        else
        {
            return DATA_NOT_UPDATED_FLOAT;
        }
    }

    else { return INVALID_FLOAT; }

    return retValue;
}

//////////////////////////////////////////////////
//                                              //
//  RetrieveIsSummer                            //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::RetrieveIsSummer(void)
{
    //prima il controllore//
    if( IS_CTRL( m_pSummerSwitch ) )
    {
        float fRet = ReadController( m_pSummerSwitch );
        if( fRet != INVALID_FLOAT )
        {
            bool retValue = static_cast<bool>(fRet);
            return retValue; //se ho letto da controllore, esco.
        }
    }

    //se sono qui, non ho letto da controllore
    return m_isSummer;
}

//////////////////////////////////////////////////
//                                              //
//  Update                                      //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::RetrieveOnOff( t_FloorZone & Zone )
{
    t_FloorOnOff& OnOff = Zone.OnOff; //reference alla struttura OnOff della zona.

    //prima il controllore//
    if( OnOff.IsCtrl() )
    {
        float fRet = ReadController( OnOff.Ctrl );
        if( fRet != INVALID_FLOAT )
        {
            bool retValue = static_cast<bool>(fRet);
            return retValue; //se ho letto da controllore, esco.
        }
    }

    //se sono qui, non ho letto da controllore
    return OnOff.isOn = m_IsOn;
}


//////////////////////////////////////////////////
//                                              //
//  RetrieveIsMaintenance                       //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::RetrieveIsMaintenance( t_FloorZone & Zone, float* Temp )
{
    t_FloorMaintenance& Maintenance = Zone.Maintenance;

    //Valore//
    if( Temp != 0x0 )
    {
        if (m_isSummer)
        {
            *Temp = Maintenance.SummerTempValue;
        } //può anche essere UNSET_TEMPMAINT
        else
        {
            *Temp = Maintenance.WinterTempValue;
        }
    }

    //prima il controllore//
    if( Maintenance.IsCtrl() )
    {
        float fRet = ReadController( Maintenance.Ctrl );
        if( fRet != INVALID_FLOAT )
        {
            bool retValue = static_cast<bool>(fRet);
            return retValue; //se ho letto da controllore, esco.
        }
    }

    //se sono qui, non ho letto da controllore
    return Maintenance.isMaintenance;
}

//////////////////////////////////////////////////
//                                              //
//  RetrieveMeanTemp                            //
//                                              //
//////////////////////////////////////////////////
float CFloorCoord2::RetrieveMeanTemp( t_FloorZone & Zone )
{
    float singleTemp = TEMP_ERRVAL, MeanTemp = TEMP_ERRVAL;
    unsigned int nOfTempNotUpdated = 0;

    vector<CTempCtrl*>& TVector = Zone.Temp; //reference al vettore delle sonde di temperatura.
    CTempCtrl* lp = 0x0;

    const unsigned int size = (unsigned int)TVector.size();
    if( size == 0 ) return TEMP_ERRVAL;

    unsigned int index = 0, Found = 0; //conto gli elementi trovati, uso questo valore per fare la media!!!
    float Numerator = 0.0; //numeratore della media: sommo sempre e poi divido per Found;
    for( index = 0; index < size; index++ )
    {
        lp = TVector.at(index);
        if( IS_CTRL( lp ) )
        {
            singleTemp = ReadController( lp );
            if( singleTemp != INVALID_FLOAT && singleTemp != TEMP_ERRVAL && singleTemp != DATA_NOT_UPDATED_FLOAT )//temperatura letta correttamente: sommo al numeratore e incremento il denominatore.
            {
                Numerator = Numerator + singleTemp;
                Found++;
            }
            else if (singleTemp == DATA_NOT_UPDATED_FLOAT)
            {
                nOfTempNotUpdated++;
            }
        }
        //altrimenti, non sonno al numeratore e non incremento il denominatore.
    }

    //Qui potrei dover uscire perchè le temperature non sono aggiornate
    if (nOfTempNotUpdated == size)
    {
        return DATA_NOT_UPDATED_FLOAT;
    }
    
    if ( Found == 0 )
    { 
        cout << "Attenzione: Zona n. " << Zone.zNumber << ": impossibile leggere la temperatura! " << endl;
        Zone.CurrentTemp = TEMP_ERRVAL;
        return TEMP_ERRVAL;
    }

    //controllo: ho letto solo alcune temperature.
    if( Found != size )
    { cout << "Attenzione: Zona n. " << Zone.zNumber << ": lette " << Found << " temperature su " << size << "!" << endl; } //non esco, faccio cmq la media ovviamente!

    Zone.CurrentTemp = (float)( Numerator / (float)Found );

    return MeanTemp;

}
///////////////////////////////////////////////////////
//  AcquireZTempSetPoint
///////////////////////////////////////////////////////
bool CFloorCoord2::AcquireZTempSetPoint(t_FloorZone &Zone)
{
    double angle;
    int currentRegister;

    if (Zone.SetPoint.tempRegFamily)
    {
        //Sonda NTH
        currentRegister = Zone.SetPoint.Ctrl->ReadCurrentRegister();

        if (currentRegister >= 0)
        {
            if (currentRegister > 950)
            {
                currentRegister = 950;
            }

            //Cerco di linearizzare l'andamento del potenziometro dividendo i dati in due fasce
            if (currentRegister < 180)
            {
                angle = -0.0108*currentRegister*currentRegister
                        + 4.6356*currentRegister
                        - 356.03;
            }
            else if (currentRegister > 205)
            {
                angle = -0.0002*currentRegister*currentRegister
                        + 0.4482*currentRegister
                        +66.74;
            }
            else
            {
                //Lascio una fascia di intermezzo per i 20 gradi
                angle = 135.0;
            }

            if (angle < 0.0)
            {
                angle = 0.0;
            }
            else if (angle > 270.0)
            {
                angle = 270.0;
            }

            if (Zone.SetPoint.tempRegType == 0)
            {
                //Ritaratore assoluto
                Zone.SetPoint.Value = angle * 20.0 / 270.0 + 10.0;
            }
            else
            {
                //Ritaratore relativo
                float newSetPoint;

                newSetPoint = angle * 6.0/270.0 - 3.0;
                Zone.SetPoint.Value = Zone.SetPoint.fixedValue + newSetPoint;
            }

            Zone.CurrentSetPoint = Zone.SetPoint.Value;

            if (m_DebugLevel)
            {
                cout << "Zona: " << Zone.zNumber<<endl;
                cout << "Setpoint : "<<Zone.SetPoint.Value<<endl;
                cout << "Angle : "<<angle<<endl;
                cout << "Registro : "<<currentRegister<<endl;
            }

            return true;
        }
    }
    else
    {
        float tempVolt;

        tempVolt = Zone.SetPoint.Ctrl->GetLastValue();

        Zone.SetPoint.Value = (tempVolt*25.0)/10.0+5.0;
        Zone.CurrentSetPoint = Zone.SetPoint.Value;

        if (m_DebugLevel)
        {
            cout << "Setpoint Impostato: "<<Zone.SetPoint.Value<<endl;
        }

        return true;
    }

    return false;
}


//////////////////////////////////////////////////
//                                              //
//  RetrieveSetPoint                            //
//                                              //
//////////////////////////////////////////////////
float CFloorCoord2::RetrieveSetPoint( t_FloorZone & Zone )
{
    t_FloorSetPoint& SetPoint = Zone.SetPoint; //reference alla struttura SetPoint della zona.

    //prima il controllore//
    if( SetPoint.IsCtrl() )
    {
        float fRet = ReadController( SetPoint.Ctrl );
        if( fRet != INVALID_FLOAT )
        {
            //Devo trattare quello che ho letto per ricavare la temperatura di setpoint
            AcquireZTempSetPoint(Zone);

            //Aggiorno il setpoint
//            UpdateZMainWaterValveSettings(Zone);
        }
    }



    //se sono qui, non ho letto da controllore
    return SetPoint.Value;
}

//////////////////////////////////////////////////
//                                              //
//  RetrieveMeanHum                             //
//                                              //
//////////////////////////////////////////////////
float CFloorCoord2::RetrieveMeanHum( t_FloorZone & Zone )
{
    float singleHum = ANALOG_ERRVAL, MeanHum = ANALOG_ERRVAL;

    vector<CAnalogIO*>& HVector = Zone.Hum; //reference al vettore delle sonde di umidità.
    CAnalogIO* lp = 0x0;

    const unsigned int size = (unsigned int)HVector.size();
    if( size == 0 ) return ANALOG_ERRVAL;

    unsigned int index = 0, Found = 0; //conto gli elementi trovati, uso questo valore per fare la media!!!
    float Numerator = 0.0; //numeratore della media: sommo sempre e poi divido per Found;
    for( index = 0; index < size; index++ )
    {
        lp = HVector.at(index);
        if( IS_CTRL( lp ) )
        {
            singleHum = ReadController( lp );
            if( singleHum != INVALID_FLOAT && singleHum != ANALOG_ERRVAL )//umidità letta correttamente: sommo al numeratore e incremento il denominatore.
            {
                Numerator = Numerator + singleHum;
                Found++;
            }
        }
        //altrimenti, non sono al numeratore e non incremento il denominatore.
    }

    //controllo: non ho letto nulla.
    if( Found == 0 )//nessun default: errore.
    { cout << "Attenzione: Zona n. " << Zone.zNumber << ": impossibile leggere l'umidità! " << endl; return ANALOG_ERRVAL; }

    //controllo: ho letto solo alcune temperature.
    if( Found != size )
    { cout << "Attenzione: Zona n. " << Zone.zNumber << ": lette " << Found << " umidità su " << size << "!" << endl; } //non esco, faccio cmq la media ovviamente!

    MeanHum = (float)( Numerator / (float)Found );

    return MeanHum;

}

//////////////////////////////////////////////////
//                                              //
//  TurnAeroByHum                              //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::TurnAeroByHum( t_FloorZone & Zone, float Level )
{
    if( Level < 0.0 ) { Level = 0.0; }
    else if( Level > 100.0 ) { Level = 100.0; }

    vector<t_Aerotherm>& aeroVector = Zone.Aerotherms;
    const unsigned int size = aeroVector.size();
    if( size == 0 ) return true;

    vector<t_Aerotherm>::iterator aeroIt;

    try {
        for (aeroIt = aeroVector.begin(); aeroIt < aeroVector.end(); aeroIt++)
        {
            vector<CDigitalIO*>& speedVector = aeroIt->speedVector;
            int speedVectorSize = aeroIt->speedVector.size();

            //Mi calcolo il nuovo stato
            switch (aeroIt->humSpeedState)
            {
                case 0:
                {
                    if ((speedVectorSize > 0) && (Level > (aeroIt->humiditySPVector.at(0)+Zone.HumHyst)) )
                    {
                        if ((speedVector.at(0)->SetState(1, false)) )
                        {
                            aeroIt->humSpeedState = 1;
                        }
                    }
                };break;
                case 1:
                {
                    if (Level < (aeroIt->humiditySPVector.at(0)-Zone.HumHyst))
                    {
                        if (speedVector.at(0)->SetState(0, false))
                        {
                            aeroIt->humSpeedState = 0;
                        }
                    }
                    else if ( (speedVectorSize > 1) && (Level > (aeroIt->humiditySPVector.at(1)+Zone.HumHyst)) )
                    {
                        if ((speedVector.at(0)->SetState(0, false)))
                        {
                            if ((speedVector.at(1)->SetState(1, false)) )
                            {
                                aeroIt->humSpeedState = 2;
                            }
                        }
                    }
                };break;
                case 2:
                {
                    if (Level < (aeroIt->humiditySPVector.at(1)-Zone.HumHyst))
                    {
                        if (speedVector.at(1)->SetState(0, false))
                        {
                            if (speedVector.at(0)->SetState(1, false))
                            {
                                aeroIt->humSpeedState = 1;
                            }
                        }
                    }
                    else if ( (speedVectorSize > 2) && (Level > (aeroIt->humiditySPVector.at(2)+Zone.HumHyst)) )
                    {
                        if ((speedVector.at(1)->SetState(0, false)))
                        {
                            if ((speedVector.at(2)->SetState(1, false)) )
                            {
                                aeroIt->humSpeedState = 3;
                            }
                        }
                    }
                };break;
                case 3:
                {
                    if (Level < (aeroIt->humiditySPVector.at(2)-Zone.HumHyst))
                    {
                        if (speedVector.at(2)->SetState(0, false))
                        {
                            if (speedVector.at(1)->SetState(1, false))
                            {
                                aeroIt->humSpeedState = 2;
                            }
                        }
                    }
                };break;
            }//Switch
        }//For
    }
    catch (...)
    {
        cout << "FloorCoord - address:"<<m_Address<<" Si è verificato un errore nella TurnAeroByHum"<<endl;
    }

    //Tolti gli avvisi perche' con la logica dell'isteresi puo' essere che non aggiorno niente.

    return true;
}

//////////////////////////////////////////////////
//                                              //
//  TurnAeroByTemp                              //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::TurnAeroByTemp( t_FloorZone & Zone, bool isSummer, bool *Nothing )
{

    if( Zone.CurrentTemp == TEMP_ERRVAL || Zone.CurrentSetPoint == TEMP_ERRVAL ) { return false; }
    if( Nothing != 0x0 ) { *Nothing = true; } //se esco prima, comunque non ho fatto nulla.

    vector<t_Aerotherm>& aeroVector = Zone.Aerotherms;
    const unsigned int size = aeroVector.size();
    bool earlyExit = false;
    if( size == 0 ) return false;

    vector<t_Aerotherm>::iterator aeroIt;

    float CurrentDelta = TEMP_ERRVAL;

    if( isSummer )//ESTATE: CurrentTemp>SetPoint
    {
        if( Zone.CurrentTemp < Zone.CurrentSetPoint )
        {
            earlyExit = true;
        }
        else
        {
            CurrentDelta = Zone.CurrentTemp - Zone.CurrentSetPoint; //positivo
        }
    }
    else //INVERNO: CurrentTemp<SetPoint
    {
        if( Zone.CurrentTemp > Zone.CurrentSetPoint )
        {
            earlyExit = true;
        }
        else
        {
            CurrentDelta = Zone.CurrentSetPoint - Zone.CurrentTemp; //positivo
        }
    }

    //Check di consistenza con l'umidita': se per la temperatura gli aerotermi dovrebbero essere spenti
    //mi assicuro almeno una volta che siano tutti fermi, da qui in poi prende il controllo l'umidità
    //Il motivo è che se la dinamica dell'ambiente è molto veloce (o ci sono degli errori) potrei fare una lettura in cui il
    //CurrentDelta mi dice di accendere e la successiva in cui CurrentDelta è negativo e quindi uscirei senza toccare niente
    if (earlyExit)
    {
        for (aeroIt = aeroVector.begin(); aeroIt < aeroVector.end(); aeroIt++)
        {
//             vector<CDigitalIO*>& speedVector = aeroIt->speedVector;

            if (aeroIt->speedVector.size() == 0)
            {
                continue;
            }

            if ( (m_isSummer && (aeroIt->seasonUsage == DHT_WINTER)) ||
                  ( (!m_isSummer) && (aeroIt->seasonUsage == DHT_SUMMER))||
                  (aeroIt->seasonUsage == DHT_NEVER)
               )
            {
                continue;
            }

            //C'è almeno un aerotermo della zona che e' acceso per la temperatura
            //Li spengo tutti resettando lo stato
            if (aeroIt->tempSpeedState)
            {
                //Per questo giro ho toccato gli aerotermi, al prossimo se nulla cambia
                //uscirò con *Nothing = true
                *Nothing = false;
                TurnAeroOff(Zone);
                break;
            }
        }

        return true;
    }

    //A questo punto controllo se devo accendere o meno
    try {
        for (aeroIt = aeroVector.begin(); aeroIt < aeroVector.end(); aeroIt++)
        {
            vector<CDigitalIO*>& speedVector = aeroIt->speedVector;
            int speedVectorSize = aeroIt->speedVector.size();

            if ( (m_isSummer && (aeroIt->seasonUsage == DHT_WINTER)) ||
                 ( (!m_isSummer) && (aeroIt->seasonUsage == DHT_SUMMER))||
                  (aeroIt->seasonUsage == DHT_NEVER)
               )
            {
                continue;
            }

            //Mi calcolo il nuovo stato
            switch (aeroIt->tempSpeedState)
            {
                case 0:
                {
                    if ((speedVectorSize > 0) && (CurrentDelta > (aeroIt->temperatureDeltaVector.at(0)+Zone.SetPoint.Hysteresis)) )
                    {
                        *Nothing = false;
                        if ((speedVector.at(0)->SetState(1, false)) )
                        {
                            aeroIt->tempSpeedState = 1;
                        }
                    }
                };break;
                case 1:
                {
                    *Nothing = false;
                    if (CurrentDelta < (aeroIt->temperatureDeltaVector.at(0)-Zone.SetPoint.Hysteresis))
                    {
                        if (speedVector.at(0)->SetState(0, false))
                        {
                            aeroIt->tempSpeedState = 0;
                        }
                    }
                    else if ( (speedVectorSize > 1) && (CurrentDelta > (aeroIt->temperatureDeltaVector.at(1)+Zone.SetPoint.Hysteresis)) )
                    {
                        if ((speedVector.at(0)->SetState(0, false)))
                        {
                            if ((speedVector.at(1)->SetState(1, false)) )
                            {
                                aeroIt->tempSpeedState = 2;
                            }
                        }
                    }
                };break;
                case 2:
                {
                    *Nothing = false;
                    if (CurrentDelta < (aeroIt->temperatureDeltaVector.at(1)-Zone.SetPoint.Hysteresis))
                    {
                        if (speedVector.at(1)->SetState(0, false))
                        {
                            if (speedVector.at(0)->SetState(1, false))
                            {
                                aeroIt->tempSpeedState = 1;
                            }
                        }
                    }
                    else if ( (speedVectorSize > 2) && (CurrentDelta > (aeroIt->temperatureDeltaVector.at(2)+Zone.SetPoint.Hysteresis)) )
                    {
                        if ((speedVector.at(1)->SetState(0, false)))
                        {
                            if ((speedVector.at(2)->SetState(1, false)) )
                            {
                                aeroIt->tempSpeedState = 3;
                            }
                        }
                    }
                };break;
                case 3:
                {
                    *Nothing = false;
                    if (CurrentDelta < (aeroIt->temperatureDeltaVector.at(2)-Zone.SetPoint.Hysteresis))
                    {
                        if (speedVector.at(2)->SetState(0, false))
                        {
                            if (speedVector.at(1)->SetState(1, false))
                            {
                                aeroIt->tempSpeedState = 2;
                            }
                        }
                    }
                };break;
            }//Switch
        }//For
    }
    catch (...)
    {
        cout << "FloorCoord - address:"<< m_Address <<" Si è verificato un errore nella TurnAeroByHum"<<endl;
    }

    //Tolti gli avvisi perche' con la logica dell'isteresi puo' essere che non aggiorno niente.

    return true;
}

//////////////////////////////////////////////////
//                                              //
//  TurnDeHumByHum                              //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::TurnDeHumByHum( t_FloorZone & Zone, float Level )
{
    if( Level < 0.0 ) { Level = 0.0; }
    else if( Level > 100.0 ) { Level = 100.0; }

    vector<t_FloorDeHum>& DHVector = Zone.DeHum;
    const unsigned int size = DHVector.size();
    if( size == 0 ) return false;

    t_FloorDeHum* lpStr = 0x0;

    float lower = 0.0, upper = 0.0;

    unsigned int index = 0, done = 0;
    bool newState = false;

    for( index = 0; index < size; index++ )
    {
        bool updateOutput = false;

        lpStr = &(DHVector.at(index));

        if( lpStr != 0x0 )
        {
            lower = lpStr->Level;
            upper = FindNextHumThreshold( DHVector, index ); //ritorna 100.1 se non trovato: maintengo il < stretto nell'if!

            if (Level >= upper+Zone.HumHyst)
            {
                //Posso spegnere questo deumidifcatore perchè entra quello successivo
                newState = false;
                updateOutput = true;
            }
            else if (( Level >= lower+Zone.HumHyst ) && (Level < upper-Zone.HumHyst))
            {
                //NB: se più di uno, al ciclo successivo lower avrà la stessa soglia e sarà aggiunto comunque!
                newState = true;
                updateOutput = true;
            }
            else if (Level <= lower-Zone.HumHyst)
            {
                //Lo spengo perche', se c'e', entra il precedente
                newState = false;
                updateOutput = true;
            }

            if (updateOutput)
            {
                if( lpStr->IsCtrl() && lpStr->Ctrl->SetState( newState, false ) ) { done++; }
            }
        }
    }

    //Tolti gli avvisi perche' con la logica dell'isteresi puo' essere che non aggiorno niente.

    return true;
}

//////////////////////////////////////////////////
//                                              //
//  TurnDeHumByTemp                             //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::TurnDeHumByTemp( t_FloorZone & Zone, bool isSummer, bool *Nothing )
{
    if( Zone.CurrentTemp == TEMP_ERRVAL || Zone.CurrentSetPoint == TEMP_ERRVAL ) { return false; }
    if( Nothing != 0x0 ) { *Nothing = true; } //se esco prima, comunque non ho fatto nulla.

    //controllo ESTATE/INVERNO: non controllo never, always ecc perchè controllato a monte, se sono qui è già giusto.
    float CurrentDelta = TEMP_ERRVAL;

    //nel caclolare CurrentDelta, ne prendo comunque il valore assoluto per confrontarlo con delta che è positivo.
    if( isSummer )//ESTATE: CurrentTemp>SetPoint
    {
        if( Zone.CurrentTemp < Zone.CurrentSetPoint ) { return true;}//ok, non devo fare nulla. Nothing è già true.
        CurrentDelta = Zone.CurrentTemp - Zone.CurrentSetPoint; //positivo
    }
    else //INVERNO: CurrentTemp<SetPoint
    {
        if( Zone.CurrentTemp > Zone.CurrentSetPoint ) { return true; }//ok, non devo fare nulla.Nothing è già true.
        CurrentDelta = Zone.CurrentSetPoint - Zone.CurrentTemp; //positivo
    }

    vector<t_FloorDeHum>& DHVector = Zone.DeHum;
    const unsigned int size = DHVector.size();
    if( size == 0 ) return false;

    t_FloorDeHum* lpStr = 0x0;

    float lower = 0.0, upper = 0.0;

    unsigned int index = 0, done = 0;
    bool newState = false;
    for( index = 0; index < size; index++ )
    {
        lpStr = &(DHVector.at(index));

        if( lpStr != 0x0 )
        {
            lower = lpStr->DeltaT;
            upper = FindNextTempDelta( DHVector, index ); //ritorna 9999.9 se non trovato: maintengo il < stretto nell'if!

            if( CurrentDelta >= lower && CurrentDelta < upper )
            { newState = true; }
                //NB: se più di uno, al ciclo successivo lower avrà la stessa soglia e sarà aggiunto comunque!
            else
            { newState = false; }

            if( lpStr->IsCtrl() && lpStr->Ctrl->SetState( newState, false ) )
            {
                done++;
                if( Nothing != 0x0 )
                { if( newState ) { *Nothing = false; } }//se almeno uno ne ho acceso, allora Nothing è false, ho fatto qualcosa e non entro nella parte di umidità.
            }
        }
    }

    if( done == 0 )
    { cout << "ATTENZIONE: impossibile accendere/spegnere le gli aerotermi nella zona n. " << Zone.zNumber << "!" << endl; return false; }

    if( done < size )
    { cout << "ATTENZIONE: accesi/spenti solo " << done << " aerotermi su " << size << " nella zona n. " << Zone.zNumber << "!" << endl; return true; }

    return true;
}

//////////////////////////////////////////////////
//                                              //
//  FindNextHumThreshold                        //
//                                              //
//////////////////////////////////////////////////
float CFloorCoord2::FindNextHumThreshold( const vector<t_FloorDeHum>& DHVector, unsigned int index, unsigned int* nextIndex )
{
    const float currentLevel = DHVector.at(index).Level;
    float nextLevel = 100.1; //se non trovato, ritorno 100.1 => qualunque valore sarà strettamente minore nella ricerca.

    const unsigned int size = DHVector.size(); //non faccio controlli.
    if( index >= (size-1) ) //ultimo elemento del vettore, inutile ciclare.
    {
        if( nextIndex != 0x0 ) { *nextIndex = size; } //= dim del vettore: in pratica non l'ho trovato.
        return 100.1;
    }

    float lLevel = currentLevel;
    for( unsigned int i = (index+1); i < size; i++ )
    {
        lLevel = DHVector.at(i).Level;
        if( lLevel > currentLevel ) //STRETTAMENTE maggiore.
        {
            nextLevel = lLevel;
            if( nextIndex != 0x0 ) { *nextIndex = i; }
            break;
        }
    }

    return nextLevel;//se nel ciclo non sono mai entrato nell'if, resta inizializzato a 100.1
}

//////////////////////////////////////////////////
//                                              //
//  FindNextTempDelta                           //
//                                              //
//////////////////////////////////////////////////
float CFloorCoord2::FindNextTempDelta( const vector<t_FloorDeHum>& DHVector, unsigned int index, unsigned int* nextIndex )
{
    const float currentDelta = DHVector.at(index).DeltaT;
    float nextDelta = TEMP_UPPERBOUND; //se non trovato, ritorno TEMP_UPPERBOUND => qualunque valore sarà strettamente minore nella ricerca.

    const unsigned int size = DHVector.size(); //non faccio controlli.
    if( index >= (size-1) ) //ultimo elemento del vettore, inutile ciclare.
    {
        if( nextIndex != 0x0 ) { *nextIndex = size; } //= dim del vettore: in pratica non l'ho trovato.
        return TEMP_UPPERBOUND;
    }

    float lDelta = currentDelta;
    for( unsigned int i = (index+1); i < size; i++ )
    {
        lDelta = DHVector.at(i).DeltaT;
        if( lDelta > currentDelta ) //STRETTAMENTE maggiore.
        {
            nextDelta = lDelta;
            if( nextIndex != 0x0 ) { *nextIndex = i; }
            break;
        }
    }

    return nextDelta;
}

//////////////////////////////////////////////////
//                                              //
//  CheckHysteresis                             //
//                                              //
//////////////////////////////////////////////////
short CFloorCoord2::CheckHysteresis(bool isSummer, float Value, float Centre, float Delta)
{
    const float upper = Centre + Delta;
    const float lower = Centre - Delta;

    short retValue = HYS_DO_NONE;
    //estate//
    if( isSummer)
    {
        if( Value <= upper && Value >= lower ) { retValue = HYS_DO_NONE; }
        else if( Value > upper ) { retValue = HYS_DO_ON; }
        else if( Value < lower ) { retValue = HYS_DO_OFF; }
        else { retValue = HYS_DO_NONE; }
    }
    //inverno//
    else
    {
        if( Value <= upper && Value >= lower ) { retValue = HYS_DO_NONE; }
        else if( Value > upper ) { retValue = HYS_DO_OFF; }
        else if( Value < lower ) { retValue = HYS_DO_ON; }
        else { retValue = HYS_DO_NONE; }
    }

    return retValue;
}

//////////////////////////////////////////////////
//                                              //
//  GetZoneNumberPtr                             //
//                                              //
//////////////////////////////////////////////////
t_FloorZone * CFloorCoord2::GetZoneNumberPtr(int zNumber)
{

    for (unsigned int i = 0; i < m_ZoneVector.size(); i++)
    {
        if (m_ZoneVector.at(i).zNumber == (unsigned int)zNumber)
        {
            return &(m_ZoneVector[i]);
        }
    }

    return 0x0;
}

//////////////////////////////////////////////////
//                                              //
//  GetAdvancedSetpoints                             //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::GetAdvancedSetpoints(unsigned int zone, float * setpoints)
{
    //solo variabile//
    t_FloorZone* pZone = GetZone( zone );

    if( pZone == 0x0 )
    { cout << "Attenzione: zona " << zone << " non trovata nel vettore. Impossibile settare SetPoint." << endl; return false; }
    else
    {
        //L'ordine e' il seguente:
        //summerSetpoint, winterSetpoint, waterSummerSetpoint, waterWinterSetpoint
        setpoints[0] = pZone->SetPoint.summerSetpoint;
        setpoints[1] = pZone->SetPoint.winterSetpoint;
        setpoints[2] = pZone->waterSummerSetpoint;
        setpoints[3] = pZone->waterWinterSetpoint;

        return true;
    }
}

//////////////////////////////////////////////////
//                                              //
//  SaveZoneAeroData                                //
//                                              //
//////////////////////////////////////////////////
bool CFloorCoord2::SaveZoneAeroData(t_FloorZone zone)
{
    CHECK_INI_BOOL()

    CString zoneKey = "ZONE";

    zoneKey+=zone.zNumber;

    m_hIni.Reload();
    m_hIni.SetInt("nOfAero",zone.Aerotherms.size(),"",zoneKey);

    for (unsigned int aeroIndex = 0; aeroIndex < zone.Aerotherms.size(); aeroIndex++)
    {
        CString aeroKey = "Aero";
        CString aeroConfig="";

        aeroKey+=(aeroIndex+1);
        for (unsigned int speedIndex = 0; speedIndex < zone.Aerotherms[aeroIndex].speedVector.size(); speedIndex++)
        {
            aeroConfig+="HUM";
            aeroConfig+=(speedIndex+1);
            aeroConfig+=":";
            aeroConfig+=zone.Aerotherms[aeroIndex].humiditySPVector.at(speedIndex);
            aeroConfig+=",";

            aeroConfig+="DELTAT";
            aeroConfig+=(speedIndex+1);
            aeroConfig+=CString(":")+zone.Aerotherms[aeroIndex].temperatureDeltaVector.at(speedIndex);
            aeroConfig+=",";

            aeroConfig+="ADDR";
            aeroConfig+=(speedIndex+1);
            aeroConfig+=CString(":")+zone.Aerotherms[aeroIndex].speedVector[speedIndex]->GetMemoryAddress();
            aeroConfig+=",";
        }

        aeroConfig+="DHT:";
        if (SEnum2Str(zone.Aerotherms[aeroIndex].seasonUsage) != 0x0)
        {
            aeroConfig+=SEnum2Str(zone.Aerotherms[aeroIndex].seasonUsage);
        }

        m_hIni.SetValue(aeroKey,aeroConfig,"",zoneKey);
    }

    m_hIni.Save();

    return true;

}

//////////////////////////////////////////////
//  UpdatePumpState
//////////////////////////////////////////////
void CFloorCoord2::UpdatePumpState()
{
    //Contiene gli indirizzi gia' controllati
    vector<unsigned int> checkedAddresses;

    //Ciclo su tutte le zone prendendo l'indirizzo della pompa
    for (unsigned int i = 0; i < m_ZoneVector.size(); i++)
    {
        t_FloorZone *mainZonePtr = 0x0, *checkZonePtr = 0x0;
        unsigned int mainZonePumpAddr, checkZonePumpAddr;
        bool pumpAlreadyKnown = false;
        bool turnOnPump = false;

        mainZonePtr = &(m_ZoneVector.at(i));

        //Controllo se c'e' la pompa
        if (!IS_CTRL(mainZonePtr->pPump))
        {
            continue;
        }

        mainZonePumpAddr = mainZonePtr->pPump->GetMemoryAddress();

        //Controllo se l'indirizzo della pompa lo conosco gia'
        pumpAlreadyKnown = false;
        for (unsigned int j = 0; j < checkedAddresses.size(); j++)
        {
            if (mainZonePumpAddr == checkedAddresses.at(j))
            {
                pumpAlreadyKnown = true;
                break;
            }
        }

        if (pumpAlreadyKnown)
        {
            continue;
        }

        //Salvo la pompa
        checkedAddresses.push_back(mainZonePtr->pPump->GetMemoryAddress());
        turnOnPump = mainZonePtr->IsTempOn;

        //Se la pompa deve accendersi su questa zona e' inutile ciclare sulle altre, altrimenti controllo tutte le zone
        if (turnOnPump)
        {
            //Accendi la pompa
            TurnPump(*mainZonePtr,true);
            continue;
        }

        //A questo punto ciclo sui restanti elementi del vettore delle zone
        for (unsigned int j = i+1; j< m_ZoneVector.size(); j++)
        {
            checkZonePtr = &(m_ZoneVector.at(j));

            if (!IS_CTRL(checkZonePtr->pPump))
            {
                continue;
            }

            checkZonePumpAddr = checkZonePtr->pPump->GetMemoryAddress();

            if (mainZonePumpAddr == checkZonePumpAddr)
            {
                //E' la stessa pompa, controllo che devo fare: se almeno una delle zone è accesa devo accendere la pompa
                turnOnPump = checkZonePtr->IsTempOn;
            }

            //Se la zona dice di accendere posso gia' uscire
            if (turnOnPump)
            {
                break;
            }
        }

        //Scrivo sulla pompa
        TurnPump(*mainZonePtr, turnOnPump);
    }
}

//////////////////////////////////////////////
//  UpdateWaterCtrlState
//////////////////////////////////////////////
void CFloorCoord2::UpdateWaterCtrlState()
{
    //Contiene gli indirizzi gia' controllati
    vector<unsigned int> checkedAddresses;

    //Ciclo su tutte le zone prendendo l'indirizzo della pompa
    for (unsigned int i = 0; i < m_ZoneVector.size(); i++)
    {
        t_FloorZone *mainZonePtr = 0x0, *checkZonePtr = 0x0;
        unsigned int mainZoneWaterCtrlAddr, checkZoneWaterCtrlAddr;
        bool waterCtrlAlreadyKnown = false;
        bool turnOnWaterCtrl = false;

        mainZonePtr = &(m_ZoneVector.at(i));

        //Controllo se c'e' la pompa
        if (!IS_CTRL(mainZonePtr->pPump))
        {
            continue;
        }

        if (mainZonePtr->waterCtrlType == WATER_PID)
        {
            mainZoneWaterCtrlAddr = mainZonePtr->pPid->GetMemoryAddress();
        }
        else if (mainZonePtr->waterCtrlType == WATER_3POINT)
        {
            mainZoneWaterCtrlAddr = mainZonePtr->digitalValveCtrl->GetMemoryAddress();
        }
        else
        {
            continue;
        }

        //Controllo se l'indirizzo del controllo acqua lo conosco gia'
        waterCtrlAlreadyKnown = false;
        for (unsigned int j = 0; j < checkedAddresses.size(); j++)
        {
            if (mainZoneWaterCtrlAddr == checkedAddresses.at(j))
            {
                waterCtrlAlreadyKnown = true;
                break;
            }
        }

        if (waterCtrlAlreadyKnown)
        {
            continue;
        }

        //Salvo la pompa
        checkedAddresses.push_back(mainZonePtr->pPump->GetMemoryAddress());
        turnOnWaterCtrl = mainZonePtr->IsTempOn;

        //Se il controllo deve accendersi su questa zona e' inutile ciclare sulle altre, altrimenti controllo tutte le zone
        if (turnOnWaterCtrl)
        {
            //Accendo il controllo
            TurnOnWaterCtrl(mainZonePtr,true);
            continue;
        }

        //A questo punto ciclo sui restanti elementi del vettore delle zone
        for (unsigned int j = i+1; j< m_ZoneVector.size(); j++)
        {
            checkZonePtr = &(m_ZoneVector.at(j));

            if (!IS_CTRL(checkZonePtr->pPump))
            {
                continue;
            }

            checkZoneWaterCtrlAddr = checkZonePtr->pPump->GetMemoryAddress();

            if (mainZoneWaterCtrlAddr == checkZoneWaterCtrlAddr)
            {
                //E' la stessa pompa, controllo che devo fare: se almeno una delle zone è accesa devo accendere la pompa
                turnOnWaterCtrl = checkZonePtr->IsTempOn;
            }

            //Se la zona dice di accendere posso gia' uscire
            if (turnOnWaterCtrl)
            {
                break;
            }
        }

        //Scrivo sulla pompa
        TurnOnWaterCtrl(mainZonePtr, turnOnWaterCtrl);
    }
}
//////////////////////////////////////////////
//  TurnOnWaterCtrl
//////////////////////////////////////////////
bool CFloorCoord2::TurnOnWaterCtrl(t_FloorZone* zonePtr, bool turnOn)
{
    CVController *waterCtrl = 0x0;
    if (zonePtr->waterCtrlType == WATER_3POINT)
    {
        waterCtrl = zonePtr->digitalValveCtrl;
    }
    else
    {
        waterCtrl = zonePtr->pPid;
    }

    if( !IS_CTRL( waterCtrl ) ) { return true; } //non c'è, quindi ok, aggiornata.
    else
    {
        waterCtrl->TurnOn(turnOn);
        return true;
    }
}
//////////////////////////////////////////////
//  ChangeZoneOnOffState
//////////////////////////////////////////////
bool CFloorCoord2::ChangeZoneOnOffState(int zone, bool turnOn)
{
    if (zone == -1)
    {
        //Accendo o spengo tutte le zone
        for (unsigned int i = 0; i < m_ZoneVector.size(); i++)
        {
            if (turnOn)
            {
                m_ZoneVector[i].OnOff.isOn = true;
            }
            else
            {
                TurnZoneOff(m_ZoneVector[i]);
            }
        }
    }
    else
    {
        t_FloorZone *zonePtr = GetZoneNumberPtr(zone);
    
        if ( zonePtr == 0x00 ) {
            return false;
        }
    
        if (turnOn) {
            //Per l'accensione basta cambiare il flag e al ciclo dopo si riaccende tutto
            zonePtr->OnOff.isOn = true;
        }
        else {
            TurnZoneOff(*zonePtr);
        }
    }

    return true;
}
//////////////////////////////////////////////
//  GetSpontaneousData
//////////////////////////////////////////////
CString CFloorCoord2::GetSpontaneousData(int lParam)
{
    CString retVal;
    
    if ((unsigned int)lParam >= m_ZoneVector.size())
    {
        return "";
    }
    
    Cmd message("DEVICE");
    message.putValue("TYPE", "FloorCoord");
    message.putValue("ADDRESS", GetMemoryAddress());
    message.putValue("ZONE", m_ZoneVector[lParam].zNumber);
    message.putValue("TEMP", m_ZoneVector[lParam].CurrentTemp);
    message.putValue("SETPOINT", m_ZoneVector[lParam].CurrentSetPoint);
    message.putValue("HUM",m_ZoneVector[lParam].CurrentHum);
    message.putValue("SUMMER", GetSummer());
    message.putValue("STATE", m_ZoneVector[lParam].IsTempOn);
    message.putValue("ON", m_ZoneVector[lParam].OnOff.isOn);

    retVal = message.getXMLValue();
    return retVal;
}



