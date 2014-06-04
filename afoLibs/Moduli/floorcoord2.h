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
#ifndef FLOORCOORD2_H
#define FLOORCOORD2_H

#include "vcoordinator.h"
#include "pid.h"
#include "floorzone2.h"
#include "tempCondensa.h"


using namespace std;

/**
	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CFloorCoord2 : public CVCoordinator
{
public:
    CFloorCoord2(const char *configString, CTimer *timer);
    ~CFloorCoord2();

    //Variabili statiche//
    //File configurazione//
    static const char* const ConfigFile; //definito nel .cpp
    //tipo//
    static const e_DeviceType CoordType = DEV_FLOORCOORD_2;
    //-----------------//

    //***Ereditate da CVCoordinator***//
    bool Update(bool updateData);
    bool Update2(bool updateData){return false;};    
    
    bool ConnectControllers();
    CString GetSpontaneousData(int lParam = 0);
    //********************************//

    void Destroy();

    //---Set---//
    bool SetSummer(bool isSummer = true, bool backup = true); //anche su file
    bool SetOnOff(unsigned int zone, bool on);
    bool SetIsMaintenance(unsigned int zone, bool on = true);
    bool SetTempMaintenance(unsigned int zone, float SummerTemp, float WinterTemp, bool backup=true);
    bool SetDeHumVal(unsigned int zone, unsigned int dhNum, float value, bool backup = true); //anche su file
    bool SetDeHumDeltaT(unsigned int zone, unsigned int dhNum, float value, bool backup = true); //anche su file
    bool SetSetPoint(unsigned int zone, float value);
    bool SetAdvancedSetpoints(unsigned int zone, float *setpoints);
    bool SetAeroData(t_Aerotherm* aero, const char* configString, const char* param);
    //---------//

    bool SetVal(float val){return false;};

    //---Get---//
    //Ritorna un riferimento costante al vettore delle zone//
    //fuori è possibile ciclare usando "const_iterator" e leggerne il contenuto ma non modificarlo//
    //uso: oggetto->GetZoneVectorRef()."funzione membro di vector"
    inline const vector<t_FloorZone>& GetZoneVectorRef() const
    { return ( const_cast < const vector<t_FloorZone>& > ( m_ZoneVector ) ); }

    //Ritorna un puntatore costante al vettore delle zone//
    //fuori è possibile ciclare usando "const_iterator" e leggerne il contenuto ma non modificarlo//
    //uso: oggetto->GetZoneVectorRef()->"funzione membro di vector"
    inline const vector<t_FloorZone>* GetZoneVectorPtr() const
    { return ( const_cast < const vector<t_FloorZone>* > ( &m_ZoneVector ) ); }

    bool GetSummer() const {return m_isSummer;};

    inline unsigned int GetNumberZone() const { return m_ZonesCount; } //numero di zone CARICATE (0 se non è stata chiamata la connectcontrollers o è fallita).
    inline bool IsConnected() const { return m_isConnected; } //se è stata chiamata la ConnectControllers ed è stata caricata almeno una zona.

    t_FloorZone* GetZoneNumberPtr(int zNumber);

    //---------//
    bool SaveZoneAeroData(t_FloorZone zone);
    
    
    bool GetAdvancedSetpoints(unsigned int zone, float *setpoints);

    /**
     * Funzione che controlla lo stato delle valvole di tutte le zone e spegne/accende le pompe relative 
     */
    void UpdatePumpState();

    /**
     * Cicla su tutte le zone per capire se è necessario spegnere il controllo acqua
     * */
    void UpdateWaterCtrlState();
    
    bool ChangeZoneOnOffState(int zone, bool turnOn);

private:

    //Quando usare i deumidificatori per la temperatura//
    static const char* str_DHTSeason[]; //definita in floorcoord.cpp, array di stringhe costanti.
    //funzioni di conversione stringa-e_DHTSeason//
    static e_DHTSeason SStr2Enum(const char* Season);//definita in floorcoord.cpp
    static const char* SEnum2Str(e_DHTSeason Season);//definita in floorcoord.cpp; ritorna 0x0 se non trovato
    
    inline static bool IsDHTActive( const t_FloorZone & Zone ) { return ( (Zone.DHTSeason != DHT_INVALID) && (Zone.DHTSeason != DHT_NEVER) ); }
    //--------------------------------------------------//

    CIniFileHandler m_hIni;
    bool m_isIniLoaded;

    //chiavi uniche per tutte le zone//
    bool m_isSummer;
    CDigitalIO* m_pSummerSwitch;

    void LoadCommons(); //valori di default
    //-------------------------------//

    //vettore zone//
    vector <t_FloorZone> m_ZoneVector; 
    unsigned int m_ZonesCount; //numero delle zone effettivamente caricate correttamente, non quello letto da ini!!!
    bool m_isVectorLoaded;
    bool m_isConnected;
    //------------//

    CVController* RetrieveCtrlEx( int devAddr, const char* devName, const char* Zone, bool showMsg = true ); //funzione già definita nella classe base.
                                                                                        //qui, implementata anche la messaggistica
    //---Letture da INI---//
    //lettura zone e riempimento del vettore m_ZoneVector//
    bool LoadZones(); //ritorna true se il vettore non è vuoto, anche se la dimensione dovesse essere inferiore al numero di zone riportato in [COMMON]
                      //verificare m_ZonesCount o m_ZoneVector.size() per sapere il numero di zone
                      //m_isVectorLoaded: true se almeno un elemento nel vettore, false altrimenti

    //lettura della singola zona da file .ini
    bool LoadSingleZone(unsigned int zoneIndex, t_FloorZone & Dest);

    //Sono void le load che non pregiudicano il caricamento della zone o del singolo elemento: Dest viene settato a valorei di default.    

    //lettura singole righe//
    void LoadOnOff(const char* sectionZone, t_FloorOnOff & Dest);//Attenzione: il bool isOn non viene letto da file, inizializzato a volore di default.
    void LoadMaintenance(const char* sectionZone, t_FloorMaintenance & Dest); //attenzione: valore impostato a UNSET_TEMPMAINT se non trovato TempMatenance
                                                                            //nell'INI può non esserci e non è in assoluto un errore (se non c'è il maintenimento), 
                                                                            //tuttavia se il valore del bool viene impostato a true con la set
                                                                            //tale valore deve esserci.
    bool LoadSingleDeHum(const char* sectionZone, unsigned int dhIndex, t_FloorDeHum & Dest);//ritorna false se non trovato
    void LoadSetPoint(const char* sectionZone, t_FloorSetPoint & Dest); 
    void LoadDHTemp(const char* sectionZone, e_DHTSeason & DHTSeason); //DHT_INVALID se non trovato, usa la IsDHTActive.
    void LoadPump(const char* sectionZone, CDigitalIO* & ptrDido);//NULL se non trovato.
    void LoadWaterCtrl(const char* sectionZone, t_FloorZone & Zone);
    void LoadWaterTempProbe(const char* sectionZone, t_FloorZone &Zone);


    //Carica gli aerotermi
    bool LoadAero(CString sectionName, t_FloorZone& Dest);
    

    //Funzioni DEPRECATE 25/09/2009
    bool LoadPid(const char* sectionZone, CPIDSimple* ptrPid);  //NULL se non trovato
                                                                  //restituisce una variabile puntatore a CPIDSimple, analogo di PIDSimple**
    bool Load3PointValve(const char* sectionZone, C3PointCtrl* Dest); //Prova a caricare i dati per il controllo valvola a 3 punti

    //lettura singoli campi di t_FloorZone
    //per tutte, se manca l'intero campo ed il vettore risulterebbe vuoto, ritornato false e quindi fanno fallire la LoadSingleZone
    //altrimenti, ciè che manca non viene aggiunto
    bool LoadValves(const char* sectionZone, vector <CDigitalIO*> & vDest);//DEVONO ESSERCI, nessun default
    void LoadHums(const char* sectionZone, vector <CAnalogIO*> & vDest);
    bool LoadTemps(const char* sectionZone, vector <CTempCtrl*> & vDest); //DEVONO ESSERCI, nessun default
    void LoadDeHums(const char* sectionZone, vector <t_FloorDeHum> & vDest);
    //----------------------------------//

    //---Ricerca nei vettori---//
    t_FloorZone* GetZone(unsigned int zoneNum, unsigned int* vectorPos = 0x0); //ritorna un puntatore alla zona cercandola nel vettore dal numero
                                                                                //ritorna se richiesta la posizione nel vettore
                                                                                //NULL se non trovato
    t_FloorDeHum* GetDeHum(unsigned int zoneNum, unsigned int num, unsigned int* vectorPos = 0x0);
    //-------------------------//

    //ordinamento dei deumidificatori//
    static bool SortDeHum( vector <t_FloorDeHum> & Vector ); //ordina in ordine CRESCENTE per SOGLIE!
    static bool VALPredicate( const t_FloorDeHum & dh1, const t_FloorDeHum & dh2 );
    //-------------------------------//

    //------ Update ------//
    bool UpdateZone( t_FloorZone & Zone );//Attenzione: false indica un fallimento. Se la zona è off, esco comunque con true.

    bool UpdateZTemp( t_FloorZone & Zone );//ATTENZIONE: setta anche CurrentTemp e CurrentSetPoint della struttura se letti correttamente.
                                           //Se no, restano i valori precedenti.
    bool UpdateZHum( t_FloorZone & Zone ); 

    //utilizzo deumidificatori per controllo di temperatura.
    //NothingDone: false se non ho dovuto accendere nulla!!!
    bool UpdateZDHT( t_FloorZone & Zone, bool& NothingDone );

    //Aggiornamento variabile di controllo nel PID della valvola di zona
    //DEPRECATA E NON IN UTILIZZO 25/09/2009
    bool UpdateZWaterPid(t_FloorZone &Zone);

    /**
     * Accende o spegne il controllo della valvola dell'acqua del collettore
     * */
    bool TurnOnWaterCtrl(t_FloorZone* zonePtr, bool turnOn);

    //Aggiornamento impostazioni estate/inverno e setpoint per il pid della valvola di zona
    //DEPRECATA
    bool UpdateZMainWaterValveSettings( t_FloorZone & Zone );

    bool RetrieveIsSummer();

    //letture dai controllori//
    static float ReadController( CVController* pCtrl ); //dal tipo decido in cosa downcastarlo e che funzione usare. Ritorno sempre un float (il tipo più generco), sarà 1/0 in caso di bool.
                                                 //ritorna INVALID_FLOAT in caso d'errore.
                                                 //ATTENZIONE: la correttezza del valore letto va verificata FUORI!!! es. temp = -100.0, um >100 ecc..

    bool RetrieveOnOff( t_FloorZone & Zone );
    bool RetrieveIsMaintenance( t_FloorZone & Zone, float* Temp = NULL ); //true/false se c'è o non c'è: se si è richiesto il valore, controllarlo sempre anche se ha ritornato true!!!
    static float RetrieveMeanTemp( t_FloorZone & Zone ); //ritorna TEMP_ERRVAL in caso di errore
    float RetrieveSetPoint( t_FloorZone & Zone );
    static float RetrieveMeanHum( t_FloorZone & Zone ); //ritorna ANALOG_ERRVAL in caso d'errore
    
    static bool IsHum( const t_FloorZone & Zone ); //verifica che ci siano le sonde e quindi i deumidificatori per entrare nell'update della zona.
                                                   //se tirotna false, inutile entrare nelle istruzioni per gestirle nella update.
                                                   //la UpdateZHum deve ritornare true in questo caso!

    static bool CheckValvesOn( const vector <CDigitalIO*> & Valves ); //true se almeno una valvola è accesa;
    static bool CheckDeHumOn( const vector <t_FloorDeHum> & DeHum ); //true se almeno un aerotermo è acceso;
    static bool CheckPumpOn( CDigitalIO* pPump ); //true se poma accesa
    //-----------------------//

    static bool TurnDeHumByHum( t_FloorZone & Zone, float Level ); //do per scontato che il vettore sia stato ordinato per VAL!
    static bool TurnDeHumByTemp( t_FloorZone & Zone, bool isSummer, bool *Nothing = 0x0 ); //do per scontato che il vettore sia stato ordinato per DELTAT!
                                                                    //CurrentTemp e CurrentSetPoint membri della zone
                                                                    // Nothing: true se, per controllare la temperatura, non ho fatto nulla perchè sono già ok!
                                                                    //Passare 0x0 se non serve saperlo.
                                                                    // in tal caso, passa all'umifità.

    bool TurnAeroByHum ( t_FloorZone & Zone, float Level ); //do per scontato che il vettore sia stato ordinato per VAL!
    bool TurnAeroByTemp( t_FloorZone & Zone, bool isSummer, bool *Nothing = 0x0 ); //do per scontato che il vettore sia stato ordinato per DELTAT!
                                                                    //CurrentTemp e CurrentSetPoint membri della zone
                                                                    // Nothing: true se, per controllare la temperatura, non ho fatto nulla perchè sono già ok!
                                                                    //Passare 0x0 se non serve saperlo.
                                                                    // in tal caso, passa all'umifità.

    static bool TurnPump( t_FloorZone & Zone, bool newState ); //attenzione: se la pompa non c'è, ritorna TRUE, perchè è previsto che non ci sia, non è un fallimento.  

    static float FindNextHumThreshold( const vector<t_FloorDeHum>& DHVector, unsigned int index, unsigned int* nextIndex = 0x0 ); 
                                                                                                       //mi serve nella FindDeHum: siccome le soglie possono ripetersi,
                                                                                                       //mi cerca la prossima diversa da quella attuale.
                                                                                                       //ATT: ritorna 100.1 se non trovato, qualunque valore sarà strettamente minore.
    static float FindNextTempDelta( const vector<t_FloorDeHum>& DHVector, unsigned int index, unsigned int* nextIndex = 0x0 ); 
                                                                                        //Analoga della precedente ma per DeltaT.
                                                                                        //ATT: mi rotorna TEMP_UPPERBOUND se non trovato, qualunque valore sarà strettamente minore. 

    static short CheckHysteresis(bool isSummer, float Value, float Centre, float Delta); //0 non fare nulla, 1 accendi, -1 spegni

    static bool TurnZoneOff( t_FloorZone & Zone ); //setta anche IsTempOn e IsDeHumOn
    static bool TurnValvesOff( t_FloorZone & Zone );
    static bool TurnAeroOff( t_FloorZone & Zone );

    bool AcquireZTempSetPoint(t_FloorZone &Zone);

};

#endif
