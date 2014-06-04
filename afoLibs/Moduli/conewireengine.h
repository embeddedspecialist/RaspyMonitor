/***************************************************************************
 *   Copyright (C) 2005 by Alessandro Mirri                                *
 *   amirri@deis.unibo.it                                                  *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 ***************************************************************************/
 /**\class COneWireEngine
This class is the main engine of the One Wire application.
It is used to create the appropiate objects , to manage the I/O and to implement the control law

 The configuration is done by reading an .ini type file, named onewire.ini, as follows:
[COMMON]                                                                     -- This section contains the common configuration data
InterfacePort=PortType:ServerSocket,ServerPort:10000,ServerIPAddr:127.0.0.1, -- The Interface Port used to communicate with the GUI
TotalNETS =                                                                  -- Total Number of onewire masters

[NET0]                                                                       -- Section relative to a particular master
COMPORT=/dev/ttyUSB0                                                         -- Port used to talk with devices
NofDev=1                                                                     -- Total number of devices under this master
Device01=                                                                    -- Configuration of the device, values separated by comma
 NAME     : Name of the device, i.e. DS18B20
 SN       : Serial Number, required, if not present the engine will provide to gather it from the device
 FN       : Family Number in Hexadecimal, i.e. 28,
 UPTIME:1 : Update time in seconds, optional, default 1 sec
 HASPOWER : Parasite or external power, optional, default HASPOWER

@author Alessandro Mirri
  * Rev. 0.11 - 22/06/2006
  * Cambiata la gestione dei dispositivi e controller
  * Aggiunta decodifica di ulteriori comandi
  * Risolti BUG che affligevano i loop principali
  * Inserite variabili di controllo per l'attivazione/disattivazione dei loop di controllo dei diversi controller
  * QUESTA VERSIONE SARA' PESANTEMENTE MODIFICATA NELLA GESTIONE DEI COMANDI E DEI DISPOSITIVI AL FINE DI:
  * 1 - Separare definitivamente controller e driver
  * 2 - Razionalizzare il protocollo e uniformarlo ad una struttura standard
  * 3 - Nazionalizzare le stringhe di lettura del file config.ini
  *
  * Rev. 0.10 - 28/04/2006
  * Aggiunta funzione UpdateINIFile per aggiornare i valori di default negli INIfile ricevendo un comando esterno. Attualmente lavora solo sui DO
  * Estesa la funzione per leggere i valori dei PID su comando anche ai PIDLMD
  * Aggiunta una sezione sperimentale nella GetCommand per aumentare la velocita' di risposta ai comandi
  *
  * Rev. 0.9 - 17-03-2006
  * Aggiunta la funzione per gestire gli switch
  * Aggiunta la sezione relativa agli switch nella OutputIntData per mandarli in esterno
  *
  * Rev. 0.8 - 11-03-2006
  * Aggiunti i comandi relativi ai PID semplici (ingresso temperatura digitale, uscita analogica)
  * Aggiunta funzione per la gestione dei PID
  * Modificate le funzioni di gestione dei degital IO
  *
  * Rev. 0.7b - 04/03/2006
  * Cambiate le funzioni OutputData e specializzate a seconda del TIPO di dato in uscita
  *
  * Rev. 0.7a - 20/02/2006
  * Modificati i comandi per gli switch
  * Aggiunto ulteriore comando GETSWITCHSTATE
  *
  * Rev. 0.7 - 10/02/2006
  * Aggiunta la gestione di sensori diversi da quelli di temperatura nella SetupSystem()
  *
  * Rev. 0.6 - 21/01/2006
  * Aggiunta la gestione dei segnali di uscita attraverso rele' in caso di allarme, denominati StrongSignal.
  * Cambiata la funzione OutputData(int netIndex, int numAlarms, int *alarms, bool isAlarm) per segnalare sia errori che allarmi sia verso l'interfaccia che attivando gli strong signal
  * Aggiunti i comandi per gestire l'abilitazione/disabilitazione degli allarmi di rete
  * Predisposti i comandi per recuperare lo stato della rete e le temperature di ogni sensore

  * Rev. 0.5d - 15/01/2006
  * Corretto un bug nella funzione ControlSystem() parte relativa alla reinizializzazione di reti non inizializzate: era sbagliata la logica.
  *
  * Rev. 0.5c - 14/01/2006
  * Cambiata la logica di partenza del programma: ora se una rete non viene vista viene "marcata" e si ritenta nella funzione ControlSystem di inizializzarla
  *
  * Rev. 0.5b - 12/01/2006
  * Aggiunta variabile m_DoDebug per regolare la quantita' di messaggi a video.
  * Eliminato il rilevamento del master nella funzione InitSystem e lasciato nella funzione SetupSystem: sembra andare un po' meglio se WL
  *
  * Rev. 0.5a - 09/01/2006
  * Eliminati gli allarmi di default per le temperature e spostati nella classe COneWireNet. Aggiunta la possibilitï¿½di memorizzare l'ultimo comando non eseguito per provare 1 volta a rieseguirlo.
  *
  * Rev. 0.5 - 07/01/2006
  * Riviste le chiamate per l'apertura e la chiusura delle reti in modo da scongiurare accessi a porte non aperte.
  * Commentate tutte le funzioni.
  *
  * Rev. 0.1 - 18/12/2005
  * Cambiata la funzione InitNET: ora c'ï¿½una separazione pi netta tra inizializzazione di una net e acquisizione della stessa:
  * le funzioni della classe COneWirenet si occupano solo di caricare i valori di inizializzazione della rete ma non la acquisiscono
  * Cambiata la decodifica del valore di ritorno della funzione AcquireNet di conewirenet: questa puï¿½ritornare sia 0 che -1 per indicare un errore.
  * Corretti alcuni bug nella funzione ExecCommands(): ora la rete si apre e si chiude correttamente prima di settare gli allarmi.

  */

#ifndef CONEWIREENGINE_H
#define CONEWIREENGINE_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstdlib>
#include <float.h>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include <algorithm>

#include "vdevice.h"
#include "ic.h"
#include "conewirenet.h"
#include "LibIniFile.h"
#include "timeUtil.h"
#include "IniFileHandler.h"
#include "commonDefinitions.h"
#include "xmlutil.h"
#include "gio.h"
#include "timer.h"
#include "afoerror.h"
#include "cownet.h"
#include "tagcontrol.h"
#include "nthmgc.h"
#include "accessLog.h"
#include "nthacc.h"
#include "Cmd.h"
#include "remotedido.h"
#include "fullutactrl.h"
#include "fullutactrl2.h"

#define BILLION 1000000000
#define MILLION 1000000
#define MAX_LOG_SIZE 10240
//Tempo minimo di ciclo di controllo in ms
#define MIN_CYCLE_TIME 1000

//Salvo il file hotel ogni 30 minuti
#define HOTEL_FILE_SAVE_INTERVAL 1800
//using namespace std;
#include "version.h"
using namespace AutoVersion;

//////////////////////////////////////////////////////
//Interface data
//////////////////////////////////////////////////////
typedef struct CICData
{
    CICData(){port = 0x0; minAddr = -1; maxAddr = -1;};
    CIC* port;
    int minAddr;
    int maxAddr;
} t_CICData;

typedef enum GeneralAlarm
{
    TEMPALARM_DATA,
    NUMTOT_DATA
} e_GeneralAlarm;


class COneWireEngine{
public:
    COneWireEngine(const char *iniFileName=NULL);

    ~COneWireEngine();

    /**
     * Main running function: it checks the internal state and call the correct function:
     * m_RunLevel = 0 -- An error occurred the Object is in stop mode
     * m_RunLevel = 1 -- Call InitSystem() to initialize all the variables and the nets
     * m_RunLevel = 2 -- Call SetupSystem() to set the entire system (i.e. the alarms on the sensors)
     * m_RunLevel = 3 -- Call ControlSystem() to enter the normal working mode
     */
    void Run();

    /**
     * Function used to initialize all the internal variables and to init the NETS, it tries also to detect the master
     * If some error occurs the one wire engine goes in state 0 (STOP)
     */
    void InitSystem();

    /**
     * Returns the internal state of the engine
     * @return m_RunLevel
     */
    int GetRunLevel(){return m_RunLevel;} ;

    bool SetRunLevel(int newRunLevel) ;

    /**
     * Main working function: it is responsible of controlling all the system and of performing input output of the messages
     */
    void ControlSystem();
    
    void ControlSystem2();

    /**
     * Function used to initialize one NET: it sets up all the NET's variables and tries to acquire the Master
     * @param netIndex Index of the NET
     * @return TRUE if master correctly detected
     */
    bool InitNet(int netIndex);

    /**
     * Init all the known nets via the InitNet() function
     * @return TRUE if every NET is correctly initialized, FALSE otherwise
     */
    bool InitAllNets();

    /**
     * Gets the command from the XMLParser object and detects which command is
     * @return The integer value of the command (see commonDefinitions.h for the command codes)
     */
    int ParseCommand();

    /**
     * This function is used to setup the various nets before starting normal operations. A typical example can be to setup all the alarms
     * @return TRUE if successful.
     */
    bool SetupSystem();

    /**
     * Gets the messages coming from the interface and pass them to the XMLParser object. Calls the ExecCommand() function to
     * Execute the command
     * @return TRUE if all comands received have been executed, FALSE otherwise
     */
    bool GetCommands();

    /**
     * Executes the commands received from the interface
     * @return TRUE if command correctly executed
     */
    bool ExecCommand();

    /**
     * Function used to Output to the SocketPort the alarms or the errors present on one NET. It also checks for conditions in which we have to issue a Strong Alarm Signal
     * @param netIndex Index of the NET
     * @param numAlarms Total number of alarms present
     * @param alarms Array containing the index fo the devices in alarm
     * @return TRUE if everything went well
    */
    bool OutputData(int netIndex, int nOfDevices, int *devices);


    /**
     * Function used to Ouput to the SocketPort the temperatures measured by the devices
     * @param deviceType type of device (/sa commondefinitions.h)
     * @param netIndex index of the net
     * @param numTemps Total number of temperatures to ouput
     * @param temps[][] Matrix containing the index of the devices and the temperature
     * @return TRUE if the messages are correctly written out
     */
    bool OutputFloatData(int netIndex, int nOfValues, float values[MAX_NUM_DEV_PER_NET][2] );


    /**
     * Funzione che scrive sulle porte i dati passati in ingresso
     * @param nOfValues  numero dati da scrivere
     * @param values[][] matrice che contiene in posizione 0 l'indirizzo e in posizione 1 il dato da scrivere
     * @return TRUE se va tutto bene
     */
    bool OutputFloatDataByAddress(int nOfValues, float values[MAX_NUM_DEV_PER_NET][2] );

    //TODO da cambiare nome e modo di funzionare in modo da avere UNA sola funzione per l'output di valori float
    bool OutputFloatData(int netIndex, int nOfValues, float values[MAX_NUM_DEV_PER_NET][3] );

    bool OutputIntData(e_DeviceType deviceType, int netIndex, int nOfValues, int values[MAX_NUM_DEV_PER_NET][2] );
    bool OutputIntData(e_DeviceType deviceType, int netIndex, int nOfValues, int values[MAX_NUM_DEV_PER_NET][3] );

    /**
     * Function used to write the informations on the LOG file
     * @param netIndex Index of the NET
     * @return TRUE if temperatures correctly written in the LOG file
     */
    bool WriteLog(int netIndex);

    /**
     * This function checks for different kinds of data on the given net that are used to activate a signal that is common to all the system
     * the system is specified in the ini file under the common section     *
     * @param alarmType type of data to check
     * @return
     */
    bool ActivateGeneralAlarm(e_GeneralAlarm alarmType);

    /**
     * Sets up all the devices in the same NET, updates the setup flag inside the NET
     * @param netIndex index of the NET to setup
     * @return TRUE if setup was successfull.
     */
    bool SetupNet (int netIndex);

    bool ManageTemperatureDevices (int netIndex);

    bool ManageSwitches (int netIndex);

    bool ManageDIDO( int netIndex );

    bool ManagePID (int netIndex);

    bool ManageHum (int netIndex);

    bool ManageAnalogIO (int netIndex);

    bool ManageAdvancedController(int netIndex);

    bool ManageCoordinators(int netIndex);

    bool ManageErrors ();

    bool CheckForCommands(int netIndex);
    //Uguale alla precedente ma SENZA netIndex che tanto non serve più da quando le NET non si chiudono.
    bool CheckForCommands2();

    bool SendCommandToRemoteDO (int maxNum, int *remoteDidoList);

    void UpdateServerPorts(bool updateInterface, bool parseData);

    bool WriteOnInterfacePorts(const char* buffer, int bufferSize);
    bool WriteOnOutputPorts (CString message, int remoteAddr = -1, int remotePort = -1);

    bool CheckInterfacePortsForConnection();
    bool ManageActivityDIDO();

    bool TrimFile(CString fileName,unsigned int bytesToTrim, int maxFileSize);


    /**
     * Trova un dispositivo iButton connesso ad un lettore locale scandagliando tutte le NET
     * @param snBuffer Buffer in cui sarà riposto il numero di serie trovato
     * @return true se l'iButton è presente, false altrimenti
     */
    bool SearchIButton (uchar *snBuffer);

    //!The interface object
    vector<CIC*> m_InterfaceVector;
    vector<CIC*> m_InCommVector;
    vector<t_CICData> m_OutCommVector;

    //!Flag that indicates if in the configuration file there is specified an out command port
    bool m_IsOutCommandPortActive;

     //!The One Wire Net Object
    COneWireNet *m_Net;

    //!The Initialization File
    CString m_IniFileName;

    //!Last command received BUT NOT executed
    CString m_LastCommand;

    //!The internal State: 0=Stop, 1=Control, 2=Command
    int m_RunLevel;

    //!The ini file object
    CIniFileHandler *m_IniFile;

    //!The total number of nets
    int m_TotalNets;

    //!The one and unique parse of the XML messages
    CXMLUtil m_XMLParser;

    //!Main debug value
    int m_DoDebug;

    long m_MaxFileSize;

    bool m_PollTemperature, m_PollTempAlarms;
    bool m_PollDigitalIO;
    bool m_PollPID;
    bool m_PollHum;
    bool m_PollAnalogIO;
    bool m_PollActivity;

    bool m_EnableLog;
    int m_LogInterval, m_LastLogTime;

    CTimer m_Timer;
    CAfoErrorHandler m_AfoErrorHandler;

    vector<CString> m_CommandQueue;

    //TBR
    int m_TotalCycleTime, m_NumberOfCycles;

    CLibIniFile m_IniLib;

    int m_GeneralAlarmNet, m_GeneralAlarmDevice, m_GeneralAlarmChannel;
    int m_HasGeneralAlarmOutput;
    bool m_GeneralAlarmInverted;

    bool m_ManageBlackList;

    //TBR--Provo ad implementare una strategia diversa per le temperature: quando questo flag è 1 indica che la conversione ha avuto luogo
    bool m_TemperatureConversionLaucnhed;
    unsigned long int m_TimeOfLastTempConv;

    /**
     *25/04/2008 -- Inserite variabili per la gestione di:
     *-Camere Hotel
     *-Moduli di controllo accessi
     */
    //Handler del file di configurazione per Hotel
//     CIniFileHandler *m_HotelIniFileHandler;
//     //Nome e percorso del file di gestione hotel: questo file viene copiato nella /var/empty per non
//     //rovinare la flash del cervelletto e periodicamente è salvato in flash (ogni 60 minuti)
//     CString m_HotelFilePath;
//     CString m_HotelFileName;
//     unsigned long int m_TimeOfLastHotelFileSave;
//
//     //Classe di gestione degli accessi: controlla tutti i parametri comuni della gestione (nomi, chiavi, scadenze)
//     //Per gli hotel si occupa di manutenere la lista personale abilitato
//     CAccessLog *m_HotelAccessLog;

    //Flag che indica se i comandi devono essere ripetuti o meno in caso di fallimento (default = 1)
    bool m_RepeatCommand;

    unsigned int m_MinCycleTime;

    //Flag che indica il numero max di cicli, letto da riga di comando se non dato è infinito
    int m_MaxNumberOfCycles;

    //Debug purposes
    bool m_SaveSystemState;
    CString m_SystemStateLogFileName;
    
    ///////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    bool UpdateTempDrivers(unsigned int netIndex);
    bool UpdateDIDODrivers(unsigned int netIndex);
    bool UpdateAIDrivers(unsigned int netIndex);
    bool UpdateAODrivers(unsigned int netIndex);
    bool UpdateDIDOControllers(unsigned int netIndex);

};

#endif //CONEWIREENGINE_H
