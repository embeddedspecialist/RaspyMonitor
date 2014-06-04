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
#include "conewireengine.h"
#include <time.h>
#include <pthread.h>

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

//////////////////////////////////////////////////////////
//                StandardConstructor                   //
//////////////////////////////////////////////////////////
COneWireEngine::COneWireEngine(const char *iniFileName)
{

   m_RunLevel = 0;

  //Profiling del ciclo di controllo
  m_TotalCycleTime = 0;
  m_NumberOfCycles = 1;
  m_MinCycleTime = 0;
  m_MaxNumberOfCycles = -1;
  m_SaveSystemState = false;

  if (iniFileName == NULL)
  {
    //No IniFile, exit, enter stop state
    m_RunLevel = 0;
    m_IniFileName = NULL;
    return;
  }

  //Do some initialization
  m_IniFileName = iniFileName;

  if (m_IniFileName.size() != 0)
  {
    m_RunLevel = m_RunLevel || 1;
  }
  else
  {
    cout << "Error!! Could not allocate space for INI file name" << endl;cout.flush();cout.flush();
  }

  m_PollTemperature = false;
  m_PollTempAlarms = false;
  m_PollDigitalIO = false;
  m_PollActivity = false;
  m_PollPID = false;
  m_EnableLog = false; 
  m_Net = 0x0;
  m_RepeatCommand = false;

  m_TemperatureConversionLaucnhed = false;
  m_TimeOfLastTempConv = 0;

}


//////////////////////////////////////////////////////////
//                        StandardDestructor            //
//////////////////////////////////////////////////////////
COneWireEngine::~COneWireEngine()
{
    vector<CIC*>::iterator cicIT;
    vector<t_CICData>::iterator cicDataIT;

    for (cicIT = m_InterfaceVector.begin(); cicIT != m_InterfaceVector.end(); cicIT++)
    {
        (*cicIT)->Terminate();
        delete *cicIT;
    }

    for (cicIT = m_InCommVector.begin(); cicIT != m_InCommVector.end(); cicIT++)
    {
        (*cicIT)->Terminate();
        delete *cicIT;
    }

    for (cicDataIT = m_OutCommVector.begin(); cicDataIT != m_OutCommVector.end(); cicDataIT++)
    {
        cicDataIT->port->Terminate();
        delete cicDataIT->port;
    }


    //Closing the net
    if (m_Net != NULL)
    {
        m_Net->ReleaseAllNets();
        delete m_Net;
    }

}


//////////////////////////////////////////////////////////
//                        RUN                           //
//////////////////////////////////////////////////////////
void COneWireEngine::Run( )
{
    //Checking actual state

    switch(m_RunLevel)
    {
        case 0: //Stop State
            cout << "Warning!! Engine in Stop State"<<endl;cout.flush();cout.flush();
            break;
        case 1: InitSystem();
            break;
        case 2: SetupSystem();
            break;
        case 3: ControlSystem2();
            break;

        default : break;
    }

//    if (m_RunLevel == 3)
//    {
//        //Update the Interface command list
//        UpdateServerPorts(true, true);
//
//        //Update the Incoming command list
//        UpdateServerPorts(false, true);
//
//        //Check for commands
//        GetCommands();
//
//    }

    if (m_SaveSystemState)
    {
        CString command;

        command = "date >>";
        command += m_SystemStateLogFileName;
        system(command.c_str());

        command = "free >> ";
        command += m_SystemStateLogFileName;
        system(command.c_str());
    }



    //Aggiunto per l'uscita di valgrind
    if (m_MaxNumberOfCycles > 0)
    {
        if (m_NumberOfCycles >= m_MaxNumberOfCycles)
        {
            //Exit
            m_RunLevel = -1;
        }
    }
}

//////////////////////////////////////////////////////////
//                        InitAllNets                       //
//////////////////////////////////////////////////////////

bool COneWireEngine::InitAllNets( )
{
    int netIndex = 0;

    cout << "Inizializzo le NET..." << endl;cout.flush();cout.flush();

    for (netIndex = 0; netIndex < m_TotalNets; netIndex++)
    {
            //Add the net to the NET object
        m_Net->AddNet();

        //Init and acquire. If not initialized, stop the system
        if (!m_Net->InitNet(netIndex))
        {
            return false;
        }

    }

    //Connetto i controllori
    for (netIndex = 0; netIndex < m_TotalNets; netIndex++)
    {
        if (!m_Net->ConnectCoordinators(netIndex))
        {
            return false;
        }

    }

    return true;
}

//////////////////////////////////////////////////////////
//                        InitNet                       //
//////////////////////////////////////////////////////////
bool COneWireEngine::InitNet(int netIndex)
{
    cout << "Inizializzo NET " << netIndex << endl;cout.flush();cout.flush();

    //Init and acquire
    if (!m_Net->InitNet(netIndex))
    {
        m_RunLevel = m_RunLevel && 0;
        return false;
    }
    else
    {
        return true;
    }
}

//////////////////////////////////////////////////////////
//                        ControlSystem                //
//////////////////////////////////////////////////////////
void COneWireEngine::ControlSystem( )
{
    int netIndex;
    struct timeval tpStart, tpStop;

    //Profiling del ciclo di controllo
    gettimeofday( &tpStart, 0x0 );
    //Start Main Cycle Loop
    for (netIndex = 0; netIndex < m_TotalNets; netIndex++)
    {
        if (m_DoDebug)
        {
            cout << endl << endl;
            cout.flush();
            cout << "******************************* Esamino NET" << netIndex+1 <<" **********************************"<< endl;
        }

        //Open the net
        if (!m_Net->AcquireNet(netIndex))
        {
            ManageErrors();
            if (m_DoDebug)
            {
                cout << "Impossibile aprire la NET" << netIndex+1<<endl;
            }
            continue;
        }

        //First check if the net has been correctly started
        //TODO da rimuovere perche'ora il setup va sempre a buon fine
        if (!m_Net->GetSetupState(netIndex))
        {
            if (m_DoDebug)
                cout << "NET non inizializzata correttamente, reinizializzo." << endl; cout.flush();

            //Try to re-set the temperature
            if (SetupNet(netIndex))
            {
                if (m_DoDebug)
                    cout << "NET" << netIndex+1 << " reinizializzata correttamente." << endl; cout.flush();
            }
            else
            {
                if (m_DoDebug)
                {
                    cout << "Si e' verificato un errore nella reinizializzazione della NET" << netIndex + 1<< endl; cout.flush();
                }

                ManageErrors();

                continue;
            }
        }


        //****************** START MANAGING ALL THE SENSORS******************************
        //Start managing temperatures
        if ( (m_PollTemperature) && m_Net->GetNetHandler( netIndex )->hasTempDevices)
        {
            if (m_DoDebug)
            {
                cout << "Aggiornamento temperature nella NET" << netIndex + 1<< endl;cout.flush();
            }

            if (!ManageTemperatureDevices(netIndex))
            {
//                 cout << "Errori in ManageTemperatureDevices( "<<endl;cout.flush();
                ManageErrors();
            }
        }

        CheckForCommands( netIndex );


        //*********************** MANAGE SWITCHES ********************************
        if ( (m_PollDigitalIO) && m_Net->GetNetHandler( netIndex )->hasDIDOs)
        {

            if (m_DoDebug)
            {
                cout << "Aggiornamento DIDO's nella NET" << netIndex +1 << endl;cout.flush();
            }

            if (!ManageDIDO(netIndex))
            {
                ManageErrors();
            }
//             if (!ManageSwitches( netIndex ))
//             {
//                 //Update the black list
// //                 cout << "Errori in ManageSwitches"<<endl;cout.flush();
//                 ManageErrors();
//             }
        }



        //*********************** MANAGE PID ********************************
        if ( (m_PollPID) && m_Net->GetNetHandler( netIndex )->hasPIDs)
        {
            if (m_DoDebug)
            {
                cout << "Aggiornamento PID nella NET" << netIndex +1<< endl;cout.flush();
            }

            if (!ManagePID( netIndex ))
            {
//                 cout << "Errori in ManagePID( "<<endl;cout.flush();
                ManageErrors();
            }
        }

        CheckForCommands( netIndex );

        //*********************** MANAGE HUM ********************************
        if ( (m_PollHum) && m_Net->GetNetHandler( netIndex )->hasHums )
        {
            if (m_DoDebug)
            {
                cout << "Aggiornamento Umidita' nella NET" << netIndex+1 << endl;cout.flush();
            }

            if (!ManageHum( netIndex ))
            {
//                 cout << "Errori in ManageHum( "<<endl;cout.flush();
                ManageErrors();
            }
        }

        CheckForCommands( netIndex );

        //*********************** MANAGE AIAO ********************************
        if ( (m_PollAnalogIO) && m_Net->GetNetHandler( netIndex )->hasAIAOs)
        {
            if (m_DoDebug)
            {
                cout << "Aggiornamento AIAO nella NET" << netIndex+1 << endl;cout.flush();
            }

            if (!ManageAnalogIO( netIndex ))
            {
//                 cout << "Errori in ManageAnalogIO( "<<endl;cout.flush();
                ManageErrors();
            }
        }

        //*********************** MANAGE AdvamcedControllers ********************************
        if (m_Net->GetNetHandler( netIndex )->hasAdvCtrls)
        {
            if (m_DoDebug)
            {
                cout << "Aggiornamento Advanced Controllers" << endl; cout.flush();
            }

            if (!ManageAdvancedController( netIndex ))
            {
                ManageErrors();
            }
        }

        //*********************** MANAGE Coordinators ********************************
        if (m_Net->GetNetHandler( netIndex )->hasCoordinators)
        {
            if (m_DoDebug)
            {
                cout << "Aggiornamento Coordinatori" << endl; cout.flush();
            }

            if (!ManageCoordinators( netIndex ))
            {
                ManageErrors();
            }
        }

        //Write log file
        WriteLog( netIndex );


        //Check if we have some commands to execute
        CheckForCommands( netIndex );


        //Update the blacklist
        if (m_ManageBlackList){
            m_Net->UpdateBlackList( netIndex );
        }


    } //FOR

    if (m_HasGeneralAlarmOutput)
    {
        ActivateGeneralAlarm(TEMPALARM_DATA);
    }

    gettimeofday( &tpStop, 0x0);

    unsigned int thisCycleTime = (tpStop.tv_sec - tpStart.tv_sec)*1000+(tpStop.tv_usec - tpStart.tv_usec)/1000;

    if (m_DoDebug > 0)
    {
        cout<<"\n\nTempo impiegato nel ciclo (ms): "<< thisCycleTime<<endl;
        m_TotalCycleTime += thisCycleTime;
        cout <<"Tempo medio impiegato per " << m_NumberOfCycles << " cicli (ms) :" << m_TotalCycleTime/m_NumberOfCycles << "\n\n" << endl;
        m_NumberOfCycles++;
    }

    //07/08/2008 -- può succedere se si cambia l'ora mentre gira il programma che il tempo di ciclo di "scasini"
    if (thisCycleTime < 0)
    {
        thisCycleTime = 0;
    }
    //12/06/2008 -- Rallento il ciclo nel caso sia troppo veloce
    if (thisCycleTime < m_MinCycleTime)
    {
        msDelay (m_MinCycleTime - thisCycleTime);
    }
}

//////////////////////////////////////////////////////////
//                        InitSystem                    //
//////////////////////////////////////////////////////////
void COneWireEngine::InitSystem( )
{
    int waitOnStartup = 0;
    int nOfPorts = 0, portIndex = 0;
    CString portString;
    CIC* tempInterface;
    t_CICData tempCIC;

    CString buffer = CString("");
    m_TotalNets = -1;

    //Create the ini file object
    m_IniFile = new CIniFileHandler(m_IniFileName);

    if (m_IniFile == 0x0)
    {
        //Problems in init: stop
        m_RunLevel = 0;

        cout << "Errore nella creazione del gestore INI file" << endl;cout.flush();
        return;
    }


    //Load if we have to log system state
    m_SystemStateLogFileName = m_IniFile->GetString("SystemLog","COMMON","");

    if (m_SystemStateLogFileName.size() == 0)
    {
        m_SaveSystemState = false;
    }
    else
    {
        m_SaveSystemState = true;
    }
    //Load if we have to repeat the commands
    m_RepeatCommand = m_IniFile->GetBool(Config_Strings[CONF_REPEATCOMMAND],"COMMON",false);



    //Load data for generalAlarm
    buffer = m_IniFile->GetString( Config_Strings[CONF_GENERALALARM], "COMMON");

    if (buffer.size())
    {
        m_IniLib.GetConfigParamInt( buffer.c_str(), "NET", &m_GeneralAlarmNet, -1);
        m_IniLib.GetConfigParamInt( buffer.c_str(), "DEV", &m_GeneralAlarmDevice, -1);
        m_IniLib.GetConfigParamInt( buffer.c_str(), "CH", &m_GeneralAlarmChannel, -1);
        m_IniLib.GetConfigParamBool( buffer.c_str(), "INV", &m_GeneralAlarmInverted, false);

        if ((m_GeneralAlarmNet == -1) || (m_GeneralAlarmDevice == -1) || (m_GeneralAlarmChannel == -1))
        {
            m_HasGeneralAlarmOutput = false;
        }
        else
        {
            m_GeneralAlarmNet -= 1;
            m_GeneralAlarmChannel -= 1;
            m_HasGeneralAlarmOutput = true;
        }
    }
    else
    {
        m_HasGeneralAlarmOutput = false;
    }

    //Get DODebug variable
    m_DoDebug = m_IniFile->GetInt(Config_Strings[CONF_DODEBUG], "COMMON");

    if (m_DoDebug)
    {
        cout << "INI file aperto" << endl;cout.flush();
    }

    m_AfoErrorHandler.SetDebugMode( m_DoDebug );

    //See if we have to wait a bit
    m_MinCycleTime = (unsigned int)(m_IniFile->GetInt( Config_Strings[CONF_UPDATETIME], "COMMON",MIN_CYCLE_TIME ));
    waitOnStartup = m_IniFile->GetInt( Config_Strings[CONF_WAITONSTARTUP], "COMMON" );
    if (waitOnStartup > 0)
    {
        if (m_DoDebug)
        {
            cout << "Inizializzo sistema, attendere..." << endl; cout.flush();
        }

        msDelay( 1000*waitOnStartup);
    }

    //Get total number of nets
    m_TotalNets = m_IniFile->GetInt(Config_Strings[CONF_NUMBEROFNETS], "COMMON");

    if (m_TotalNets == INT_MIN)
    {
        cout << "Errore!!! Numero totale reti mancante" << endl;cout.flush();
        m_RunLevel = m_RunLevel && 0;
        return;
    }


    //Controllo se devo gestire la blacklist
    if (m_IniFile->GetInt(Config_Strings[CONF_BLACKLISTTIMEOUT], "COMMON") > 0){
        m_ManageBlackList = true;
    }
    else {
        m_ManageBlackList = false;
    }

    //Create The Interface
    //Get Interface configuration string
    nOfPorts = m_IniFile->GetInt( Config_Strings[CONF_NOFINTERFACEPORT], "COMMON");

    if (nOfPorts > 0)
    {
        for (portIndex = 0; portIndex < nOfPorts; portIndex++)
        {
            portString = Config_Strings[CONF_INTERFACEPORT];
            portString+=portIndex+1;
            buffer = m_IniFile->GetString(portString, "COMMON");

            //Checking if valid
            if (buffer.size() == 0)
            {
                //Error, interface port not found
                m_RunLevel = 0;
                cout << "Errore!!! Parametri porta interfaccia numero "<<portIndex+1<<" non trovati nel file INI" << endl;cout.flush();
                return;
            }

            //Init interface
            tempInterface = new CIC();

            if (tempInterface == NULL)
            {
                if (m_DoDebug)
                {
                    cout << "Errore!!! Impossibile creare l'oggetto Interfaccia Esterna numero "<< portIndex+1 << endl;
                    cout.flush();
                }
                m_RunLevel = 0;
                return;
            }

            tempInterface->m_EngPtr = this;

            tempInterface->SetDebugLevel( m_DoDebug );

            if (m_DoDebug)
            {
                cout << "Interfaccia Esterna numero " <<portIndex+1<<" creata" << endl;cout.flush();
            }

            tempInterface->Initialize((char*)buffer.c_str());

            if (!tempInterface->Startup())
            {
                if (m_DoDebug)
                    cout << "ENGINE: Errore!!! Impossibile aprire la porta Interfaccia esterna numero " << portIndex+1 << " controllare i parametri" << endl;

                //A Problem occurred in opening the port, could not continue
                m_RunLevel = 0;

                //Delete the object
                delete tempInterface;

                return;
            }
            else
            {
                m_InterfaceVector.push_back(tempInterface);
            }
        }//FOR
    }//IF nOfPorts
    else
    {
        cout << "Attenzione non e'stata definita nessuna porta di interfaccia" << endl; cout.flush();
    }


    if (m_IniFile->GetInt( Config_Strings[CONF_USEWATCHDOG], "COMMON") > 0)
    {
        //Create hidden port for the watchdog
        tempInterface = new CIC();

        if (tempInterface->Initialize("PortType:ServerSocket,ServerPort:65000,ServerIPAddr:127.0.0.1") && tempInterface->Startup())
        {
            //Tutto OK, slava la porta
            m_InterfaceVector.push_back(tempInterface);

            //Create the watchdog
           system ("./afowatchdog &");

            cout << "Watchdog avviato..." << endl;
            cout.flush();
        }
        else
        {
            cout << "Errore non e' stato possibile definire la porta per il watchdog" << endl;
            cout.flush();
        }
    }

    //++++++++++++++++++++++++++++++Porte comandi ingresso/uscita/Interfaccia+++++++++++++++++++++++++++++++++++++++

    //In Commands Port
    nOfPorts = m_IniFile->GetInt( Config_Strings[CONF_NOFCOMMPORTIN], "COMMON");

    if (nOfPorts > 0)
    {
        for (portIndex = 0; portIndex < nOfPorts; portIndex++)
        {

            tempInterface = new CIC();

            if (tempInterface == NULL)
            {
                if (m_DoDebug)
                {
                    cout << "Errore!!! Impossibile creare l'oggetto Porta Comandi in ingresso numero "<< portIndex+1 << endl;
                    cout.flush();
                }
                m_RunLevel = 0;
                return;
            }

            tempInterface->SetDebugLevel( m_DoDebug );

            //Get configuration string for the input command interface
            portString = Config_Strings[CONF_COMMPORTIN];
            portString+=portIndex+1;
            buffer = m_IniFile->GetString(portString, "COMMON");

            //Checking if valid
            if (buffer.size() == 0)
            {
                if (m_DoDebug)
                {
                    cout << "Attenzione!! Parametri Porta Comandi in ingresso numero " << portIndex+1<<" non trovati. Uso porta di default" << endl;
                }

                m_RunLevel = 0;
                return;
            }

           tempInterface->Initialize((char*)buffer.c_str());

            if (!tempInterface->Startup())
            {
                if (m_DoDebug)
                    cout << "Errore!!! Impossibile aprire la porta per ingresso comandi numero " << portIndex+1 << ", controllare i parametri" << endl;

                //A Problem occurred in opening the port, could not continue
                m_RunLevel = 0;

                //Delete the object
                delete tempInterface;

                return;
            }
            else
            {
                m_InCommVector.push_back(tempInterface);
            }
        }//FOR
    }
    else
    {
        cout << "Attenzione non e'stata definita nessuna Porta Comandi in ingresso" << endl; cout.flush();
    }


    //Setup command output port
    //Get Interface configuration string
    nOfPorts = m_IniFile->GetInt( Config_Strings[CONF_NOFCOMMPORTOUT], "COMMON");

    if (nOfPorts > 0)
    {
        for (portIndex = 0; portIndex < nOfPorts; portIndex++)
        {
            tempCIC.port = new CIC();

            if (tempCIC.port == NULL)
            {
                if (m_DoDebug)
                {
                    cout << "Errore!!! Impossibile creare l'oggetto Porta Comandi in Uscita numero "<< portIndex+1 << endl;
                    cout.flush();
                }
                m_RunLevel = 0;
                return;
            }

            tempCIC.port->SetDebugLevel( m_DoDebug );

            //Get configuration string for the input command interface
            portString = Config_Strings[CONF_COMMPORTOUT];
            portString+=portIndex+1;
            buffer = m_IniFile->GetString(portString, "COMMON");


            //Checking if valid
            if (buffer.size() == 0)
            {
                if (m_DoDebug)
                {
                    cout << "Errore!!! Parametri porta comandi in uscita numero " << portIndex+1<< " non trovati!!" << endl;
                }

                m_RunLevel = 0;
                return;
            }

            m_IniLib.GetConfigParamInt( buffer.c_str(), "MINADDR", &tempCIC.minAddr, 0 );
            m_IniLib.GetConfigParamInt( buffer.c_str(), "MAXADDR", &tempCIC.maxAddr, 100000 );

            tempCIC.port->Initialize((char*)buffer.c_str());

            //TODO forse da tolgiere perche'i client socket possono tentare di collegarsi ogni volta che sono usati
            if (!tempCIC.port->Startup())
            {
                if (m_DoDebug)
                    cout << "Attenzione!!! Impossibile avviare la porta per comandi in uscita numero "<<portIndex+1<<", controllare i parametri" << endl;
            }

            m_OutCommVector.push_back(tempCIC);

        }//FOR
    }//IF nOf ports
    else
    {
        cout << "Attenzione non e'stata definita nessuna Porta Comandi in Uscita" << endl; cout.flush();
    }


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //Init log file
    m_EnableLog = m_IniFile->GetBool( Config_Strings[CONF_ENABLELOG], "COMMON");

        //Get the maximum log file size
    m_MaxFileSize = (long)(m_IniFile->GetInt(Config_Strings[CONF_MAXFILEDIM], "COMMON",10240));

    if (m_EnableLog)
    {
        //Check if the file entry exists
        buffer = m_IniFile->GetString(Config_Strings[CONF_FILENAME], "COMMON");

        //Checking if valid
        if (buffer.size() == 0)
        {
            //Error, file port not found
            m_RunLevel = 0;
            cout << "Errore!!! Impossibile trovare i parametri del file di LOG, disabilito il salvataggio." << endl;cout.flush();
            m_EnableLog = false;
        }
        else
        {
            FILE* testFile = fopen(buffer.c_str(), "a+");
            if (testFile == 0x00)
            {
                //Il file non esiste, lo creo
                cout << "ATTENZIONE: Il file di log non esiste, lo creo"<<endl;
                CString createFile = "touch ";
                createFile+=buffer;
                system(createFile.c_str());
            }
        }

        //Retrieve Log Time
        m_LogInterval = m_IniFile->GetInt( Config_Strings[CONF_LOGINTERVAL], "COMMON",600 );

        m_LastLogTime = -1;

    }
    else
    {
        m_LastLogTime = -2;
    }

    //Init errors log
    if (m_IniFile->GetBool( Config_Strings[CONF_LOGERRORS], "COMMON"))
    {
        m_AfoErrorHandler.EnableLog( true );
    }
    else
    {
        m_AfoErrorHandler.EnableLog( false );
    }


    if (m_IniFile->GetInt(Config_Strings[CONF_MAXDIMERR], "COMMON") > 0 )
    {
        m_AfoErrorHandler.SetMaxFileSize( m_IniFile->GetInt(Config_Strings[CONF_MAXDIMERR], "COMMON") );
    }
    else
    {
        m_AfoErrorHandler.SetMaxFileSize( 10240 );
    }

    //++++++++++++++++ Creazione NET +++++++++++++++++++++++++++++++++++++++++++++

    //Create the main NET object
    m_Net = new COneWireNet(m_IniFileName, &m_Timer);
    m_Net->m_Master = new COWNET(m_IniFile, "COMMON");
    m_Net->m_Master->SetCRCUtil( &(m_Net->m_CRCUtil));
    m_Net->SetEnginePtr(reinterpret_cast<void*>(this));

    m_Net->SetInitialParameters(m_IniFile);
    m_Net->SetErrorHandler( &m_AfoErrorHandler );

    //Try to acquire the NET
    if (!InitAllNets())
    {
        m_RunLevel = m_RunLevel && 0;
        cout << "ENGINE: Errore!!! Si e'verificato un errore nell'inizializzazione delle reti" << endl;cout.flush();

        return;
    }

    //Get polling for devices
    m_PollDigitalIO = m_IniFile->GetBool( Config_Strings[CONF_POLLDIGITAL], "COMMON");
    m_PollActivity = m_IniFile->GetBool( Config_Strings[CONF_POLLACTIVITY], "COMMON");
    m_PollTemperature = m_IniFile->GetBool( Config_Strings[CONF_POLLTEMP], "COMMON" );
    m_PollTempAlarms = m_IniFile->GetBool( Config_Strings[CONF_POLLTEMPALARMS], "COMMON" );
    m_PollPID = m_IniFile->GetBool(Config_Strings[CONF_POLLPID], "COMMON");
    m_PollHum = m_IniFile->GetBool( Config_Strings[CONF_POLLHUMID], "COMMON");
    m_PollAnalogIO = m_IniFile->GetBool( Config_Strings[CONF_POLLANALOG], "COMMON" );

    if (!m_Timer.LoadTimers())
    {
        cout << "ENGINE: Errore!!! Impossibile caricare il file dei timer." << endl;
        m_RunLevel = 0;
        return;
    }

    if (m_DoDebug)
    {
        cout << "************** SISTEMA INIZIALIZZATO ***************" << endl;cout.flush();
    }

    cout << "Inizio Setup del Sistema (Modo 2)" << endl;cout.flush();

    msDelay(1000);

    m_RunLevel = 2 ;

}

//////////////////////////////////////////////////////////
//                        SetupSystem                   //
//////////////////////////////////////////////////////////
bool COneWireEngine::SetupSystem( )
{
    int i = 0;
    CString message;
    bool doSetup;


    //Check if we want to setup the net
    doSetup = m_IniFile->GetBool( Config_Strings[CONF_DOSETUP], "COMMON");


    for (i = 0; i < m_TotalNets; i++)
    {
        cout << "Setup NET: " << i +1 <<"..." << endl;cout.flush();

        //Open the NET
        if (m_Net->AcquireNet(i))
        {
            if (m_DoDebug)
            {
                cout << "SETUP: Rete Rilevata" << endl; cout.flush();
            }


            if (doSetup)
            {
                //Set up the NET
                SetupNet( i );
            }
            else
            {
                //Don't perform setup, just record a correct init in the NET
                m_Net->SetSetupState( i, true);
            }
        }
        else
        {
            m_AfoErrorHandler.PushError( AFOERROR_UNABLE_TO_ACQUIRE_NET, i+1);
        }

        //Wait a bit, just in case...
        msDelay(200);
    }

    //Setup finished: Going on...
    m_RunLevel = 3;

    //TBR
    cout << "**************************** SISTEMA SETTATO ***************************" << endl;
    cout << "Inizio a regolare il sistema (Modo 3)" << endl;cout.flush();

    return true;

}

//////////////////////////////////////////////////////////
//                        GetCommands                  //
//////////////////////////////////////////////////////////
bool COneWireEngine::GetCommands( )
{
    bool retVal = true;
    CString commandStr;
    vector<CString>::iterator stringIt;
    vector<CIC*>::iterator portIt;
    //TBR
    int nOfCommands = 0;

    //First try to rexecute commands pending, and then delete them to avoid blocking situations
    //TODO fare qualcosa di meglio perche' non mi piace moltissimo...
    stringIt = m_CommandQueue.begin();
    while( stringIt != m_CommandQueue.end())
    {
        if (m_DoDebug)
        {
            cout << "Executing pending commands..." << endl; cout.flush();
        }

        m_XMLParser.ParseXML( *stringIt );

        ExecCommand();

        m_CommandQueue.erase(stringIt);

    }

    //Get all the commands from the interface ports
    for (portIt = m_InterfaceVector.begin(); portIt!=m_InterfaceVector.end(); portIt++)
    {
        commandStr = (*portIt)->GetCommand();
        while ( commandStr.size() )
        {
            m_CommandQueue.push_back(commandStr);
            commandStr = (*portIt)->GetCommand();
        }
    }

    //Get all the commands coming from other devices
    for (portIt = m_InCommVector.begin(); portIt!=m_InCommVector.end(); portIt++)
    {
        commandStr = (*portIt)->GetCommand();
        //TBR
//         cout << "Comando Ricevuto: " << commandStr << endl;
        while ( commandStr.size() )
        {
            m_CommandQueue.push_back(commandStr);
            commandStr = (*portIt)->GetCommand();

            //TBR
//             cout << "Comando Ricevuto: " << commandStr << endl;
        }
    }

    //Start executing commands
    stringIt = m_CommandQueue.begin();
    while( stringIt != m_CommandQueue.end())
    {
        nOfCommands++;

        //TBR
//         cout << "Eseguo comando: " << *stringIt << endl;

        if (m_XMLParser.ParseXML( *stringIt ))
        {
            if (ExecCommand())
            {
                m_CommandQueue.erase(stringIt);
            }
            else
            {
                //TODO aggiungere gestione errore
                if (m_DoDebug)
                {
                    cout << "Warning command : " << *stringIt << " NOT EXECUTED" << endl;
                }

                //Se non devo ripetere i comandi cancello anche da qui
                if (!m_RepeatCommand)
                {
                    m_CommandQueue.erase(stringIt);
                }
                else
                {
                    stringIt++;
                }
            }
        }
        else
        {
            cout << "Warning command: " << *stringIt << " NON VALIDO" << endl;
            //Lo cancello
            m_CommandQueue.erase(stringIt);
        }
    }

    return retVal;
}



//////////////////////////////////////////////////////////
//                        OutputData                   //
//////////////////////////////////////////////////////////
bool COneWireEngine::OutputData(int netIndex, int nOfDevices, int *devices)
{
    int devIndex = 0, i = 0;
    CString message;
    int address;
    T_Net *net;
    char TBuffer[8], AddressBuffer[32];
    bool retVal = false;


    memset (AddressBuffer, 0, 32*sizeof(char));
    memset (TBuffer, 0, 8*sizeof(char));

    //Get Handler to the net
    net = m_Net->GetNetHandler(netIndex);

    if (net == 0x0)
    {
        //NET index was out of bounds
        return false;
    }
    else if (nOfDevices == 0)
    {
        //No values to output
        return true;
    }

    //We have found some alarms, writeout the corresponding message
    for (i = 0; i < nOfDevices; i++)
    {
        devIndex = devices[i];

        if (devIndex < 0)
        {
            //Index not valid for some reason
            continue;
        }

        //Get the ROM code and convert it to char
        address = net->CtrlList[devIndex]->GetMemoryAddress();
        sprintf (AddressBuffer, "%d", address);

        //Check the type of sensor to compose the correct message
        switch (net->CtrlList[devIndex]->GetControllerType())
        {
            case DEV_TEMPCTRL:
            {
                //Get the last temp
                sprintf(TBuffer, "%2.1f", ((CTempCtrl*)(net->CtrlList[devIndex]))->GetLastTemp());

                //Compose the message
                message = m_XMLParser.CreateMessage("DEVICE", 4, "DS18X20", AddressBuffer, TBuffer, "1");
            }
            break;
            default:
                message="<ERROR />";
        }


        //Write it out
        UpdateServerPorts(true, false);
        WriteOnInterfacePorts(message.c_str(), message.size());
    }

    retVal = true;

    return retVal;
}

//////////////////////////////////////////////////////////
//                        OutputFloatData                   //
//////////////////////////////////////////////////////////
bool COneWireEngine::OutputFloatData( int netIndex, int nOfValues, float values[MAX_NUM_DEV_PER_NET][2] )
{
    int devIndex = 0, i = 0;
    CString message;
    T_Net *net;
    int address;
    char TBuffer[8], AlBuffer[8], AddressBuffer[32];
    e_DeviceType controllerType;

    memset (AddressBuffer, 0, 32*sizeof(char));
    memset (TBuffer, 0, 8*sizeof(char));
    memset (AlBuffer, 0, 8*sizeof(char));

    //Get Handler to the net
    net = m_Net->GetNetHandler(netIndex);

    if (net == 0x0)
    {
        //NET index was out of bounds
        return false;
    }
    else if (nOfValues == 0)
    {
        //No values to output
        return true;
    }

    UpdateServerPorts(true, false);

    //Get the first device of the list to determine the family number
    devIndex = (int)values[0][0];
    controllerType = net->CtrlList[devIndex]->GetControllerType();

    switch (controllerType)
    {
        case DEV_TEMPCTRL:
        case DEV_TEMPCTRLHYST:
        {
                        //writeOut the temperatures
            for (i = 0; i < nOfValues; i++)
            {
                devIndex = (int)values[i][0];

                //Get the Address and convert it to char
                address = net->CtrlList[devIndex]->GetMemoryAddress();
                sprintf(AddressBuffer, "%d", address);

                //Convert temperature to string
                sprintf(TBuffer, "%2.1f", values[i][1]);

                //GetAlarmState
                if ((m_PollTempAlarms) && (net->CtrlList[devIndex]->GetControllerType() == DEV_TEMPCTRL))
                {
                    if ( ((CTempCtrl*)(net->CtrlList[devIndex]))->GetAlarmState() )
                    {
                        strcpy (AlBuffer, "1");
                    }
                    else
                    {
                        strcpy (AlBuffer, "0");
                    }
                }
                else
                {
                    strcpy(AlBuffer, "0");
                }

                //Compose the message
                message = m_XMLParser.CreateMessage("DEVICE", 5, Device_strings[DEV_TEMPCTRL], AddressBuffer, "Temperature", TBuffer, AlBuffer);

                //Write it out
                UpdateServerPorts(true, false);
                WriteOnInterfacePorts(message.c_str(), message.size());
            }
        }
        break;
        case DEV_AIAO:
        {
            for (i = 0; i < nOfValues; i++)
            {
                devIndex = (int)values[i][0];

                //Get the Address and convert it to char
                address = net->CtrlList[devIndex]->GetMemoryAddress();
                sprintf(AddressBuffer, "%d", address);

                //Convert Values to String
                sprintf(TBuffer, "%2.1f", values[i][1]);

                //GetAlarmState
                if  ( ((CAnalogIO*)(net->CtrlList[devIndex]))->IsInput())
                {
                    if ( ((CAnalogIO*)(net->CtrlList[devIndex]))->GetReadCurrentState() )
                    {
                        strcpy (AlBuffer, "1");
                    }
                    else
                    {
                        strcpy (AlBuffer, "0");
                    }
                }
                else
                {
                    strcpy(AlBuffer, "NA");
                }

                //Compose the message
                message = m_XMLParser.CreateMessage("DEVICE", 4, Device_strings[DEV_AIAO], AddressBuffer, TBuffer, AlBuffer);

                //Write it out
                UpdateServerPorts(true, false);
                WriteOnInterfacePorts(message.c_str(), message.size());
            }
        }
        break;
        case DEV_PIDSIMPLE:
        case DEV_PIDLMD:
        case DEV_UTACTRL:
        {
            int memoryAddr;
            CString pidName;
            char addrBuffer[32];
            char posBuffer[16];

            //writeOut positions
            for (i = 0; i < nOfValues; i++)
            {
                devIndex = (int)(values[i][0]);

                //Get memory address
                memoryAddr = net->CtrlList[devIndex]->GetMemoryAddress();
                sprintf (addrBuffer, "%d", memoryAddr);

                sprintf (posBuffer, "%2.1f", values[i][1]);

                pidName = net->CtrlList[devIndex]->GetName();

                //Compose the message
                message = m_XMLParser.CreateMessage("DEVICE", 4, pidName.c_str(), addrBuffer, "LastPosition",  posBuffer);

                //Write it out
                UpdateServerPorts(true, false);
                WriteOnInterfacePorts(message.c_str(), message.size());

//                 cout << "Scrivo dati UTA:" << message << endl;
                //wait a bit to not overflow the socket
                //msDelay(50);
            }

        };
        break;
        default:
        {
            message = "<ERROR TYPE=\"DEVICE_UNKNOWN\" />";

            UpdateServerPorts(true, false);
            WriteOnInterfacePorts( message.c_str(), message.size());
        }
    }

    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
bool COneWireEngine::OutputFloatDataByAddress(int nOfValues, float values[MAX_NUM_DEV_PER_NET][2])
{
    int devIndex = 0, netIndex = 0;
    CString message;
    T_Net *net;
    char TBuffer[8], AlBuffer[8], AddressBuffer[32];
    e_DeviceType controllerType = DEV_NONE;

    for (int valIndex = 0; valIndex < nOfValues; valIndex++)
    {
        controllerType = DEV_NONE;
        memset (AddressBuffer, 0, 32*sizeof(char));
        memset (TBuffer, 0, 8*sizeof(char));
        memset (AlBuffer, 0, 8*sizeof(char));

        UpdateServerPorts(true, false);

        //Get the first device of the list to determine the family number
        netIndex = m_Net->GetNetByMemoryAddress((int)values[valIndex][0]);

        if (netIndex > -1)
        {
            //Get Handler to the net
            net = m_Net->GetNetHandler(netIndex);

            devIndex = m_Net->GetDeviceIndexByMemoryAddress(netIndex, (int)values[valIndex][0]);

            if (devIndex > -1)
            {
                controllerType = net->CtrlList[devIndex]->GetControllerType();
            }
            else
            {
                continue;
            }
        }
        else
        {
            continue;
        }

        switch (controllerType)
        {
            case DEV_TEMPCTRL:
            case DEV_TEMPCTRLHYST:
            {
                //writeOut the temperatures
                //Get the Address and convert it to char
                sprintf(AddressBuffer, "%d", (int)values[valIndex][0]);

                //Convert temperature to string
                sprintf(TBuffer, "%2.1f", values[valIndex][1]);

                //GetAlarmState
                if ((m_PollTempAlarms) && (net->CtrlList[devIndex]->GetControllerType() == DEV_TEMPCTRL))
                {
                    if ( ((CTempCtrl*)(net->CtrlList[devIndex]))->GetAlarmState() )
                    {
                        strcpy (AlBuffer, "1");
                    }
                    else
                    {
                        strcpy (AlBuffer, "0");
                    }
                }
                else
                {
                    strcpy(AlBuffer, "0");
                }

                //Compose the message
                message = m_XMLParser.CreateMessage("DEVICE", 5, Device_strings[DEV_TEMPCTRL], AddressBuffer, "Temperature", TBuffer, AlBuffer);

                //Write it out
                UpdateServerPorts(true, false);
                WriteOnInterfacePorts(message.c_str(), message.size());
            }
            break;
            case DEV_AIAO:
            {
                //Get the Address and convert it to char
                sprintf(AddressBuffer, "%d", (int)values[valIndex][0]);

                //Convert Values to String
                sprintf(TBuffer, "%2.1f", values[valIndex][1]);

                //GetAlarmState
                if  ( ((CAnalogIO*)(net->CtrlList[devIndex]))->IsInput())
                {
                    if ( ((CAnalogIO*)(net->CtrlList[devIndex]))->GetReadCurrentState() )
                    {
                        strcpy (AlBuffer, "1");
                    }
                    else
                    {
                        strcpy (AlBuffer, "0");
                    }
                }
                else
                {
                    strcpy(AlBuffer, "NA");
                }

                //Compose the message
                message = m_XMLParser.CreateMessage("DEVICE", 4, Device_strings[DEV_AIAO], AddressBuffer, TBuffer, AlBuffer);

                //Write it out
                UpdateServerPorts(true, false);
                WriteOnInterfacePorts(message.c_str(), message.size());
            }
            break;
            case DEV_PIDSIMPLE:
            case DEV_PIDLMD:
            case DEV_UTACTRL:
            {
                CString pidName;
                char addrBuffer[32];
                char posBuffer[16];

                //writeOut positions
                //Get memory address
                sprintf (addrBuffer, "%d", (int)values[valIndex][0]);

                sprintf (posBuffer, "%2.1f", values[valIndex][1]);

                pidName = net->CtrlList[devIndex]->GetName();

                //Compose the message
                message = m_XMLParser.CreateMessage("DEVICE", 4, pidName.c_str(), addrBuffer, "LastPosition",  posBuffer);

                //Write it out
                UpdateServerPorts(true, false);
                WriteOnInterfacePorts(message.c_str(), message.size());

            };
            break;
            default:
            {
                message = "<ERROR TYPE=\"DEVICE_UNKNOWN\" />";

                UpdateServerPorts(true, false);
                WriteOnInterfacePorts( message.c_str(), message.size());
            }
        }
    }

    return true;

}

//////////////////////////////////////////////////////////
//                        OutputFloatData                   //
//////////////////////////////////////////////////////////
bool COneWireEngine::OutputFloatData( int netIndex, int nOfValues, float values[MAX_NUM_DEV_PER_NET][3] )
{
    int devIndex = 0, i = 0;
    CString message;
    e_DeviceType controllerType;
    T_Net *net;
    char addressBuffer[32], val1Buffer[8], val2Buffer[8];
    int address;

    memset (addressBuffer, 0, 32*sizeof(char));
    memset (val1Buffer, 0, 8);
    memset (val2Buffer, 0, 8);

    //Get Handler to the net
    net = m_Net->GetNetHandler(netIndex);

    UpdateServerPorts(true, false);

    if (net == 0x0)
    {
        //NET index was out of bounds
        return false;
    }
    else if (nOfValues <= 0)
    {
        //No values to output
        return true;
    }

    //Get the first device of the list to determine the family number
    devIndex = (int)values[0][0];
    controllerType = net->CtrlList[devIndex]->GetControllerType();

    switch (controllerType)
    {
        case DEV_HUMIDITY:
        {
            //writeOut the temperatures
            for (i = 0; i < nOfValues; i++)
            {
                devIndex = (int)values[i][0];

                //Get the ROM code and convert it to char
                address = m_Net->GetControllerMemoryAddress( netIndex, devIndex);
                sprintf (addressBuffer, "%d", address);

                //Compose the message
                message = m_XMLParser.CreateMessage("DEVICE", 5, Device_strings[DEV_HUMIDITY], addressBuffer, "humidity",  values[i][1], values[i][2]);

                //Write it out
                UpdateServerPorts(true, false);
                WriteOnInterfacePorts(message.c_str(), message.size());
            }
        }
        break;
        default:
        {
            message = "<ERROR TYPE=\"OutputFloatData -- DEVICE_UNKNOWN\" />";

            UpdateServerPorts(true, false);
            WriteOnInterfacePorts( message.c_str(), message.size());
        }
    }

    return true;
}

//////////////////////////////////////////////////////////
//                        OutputIntData                   //
//////////////////////////////////////////////////////////
bool COneWireEngine::OutputIntData( e_DeviceType deviceType, int netIndex, int nOfValues, int values[MAX_NUM_DEV_PER_NET][2] )
{
    int devIndex = 0, i = 0;
    CString message;
    T_Net *net;
    char AddrBuffer[32], posBuffer[8];

    memset (AddrBuffer, 0, 32);
    memset (posBuffer, 0, 8);

    //Get Handler to the net
    net = m_Net->GetNetHandler(netIndex);

    switch (deviceType)
    {
        case DEV_AIAO:
        {
//             //writeOut potentiometers positions
//             for (i = 0; i < nOfValues; i++)
//             {
//                 devIndex = values[i][0];
//
//                 //Get the ROM code and convert it to char
//                 net->deviceList[devIndex]->GetSN(serialCode);
//                 ConvertSN2Str(AddrBuffer, serialCode);
//
//                 //Convert temperature to string
//                 sprintf(posBuffer, "%d", values[i][1]);
//
//                 //Compose the message
//                 message = m_XMLParser.CreateMessage("DEVICE", 3, Device_strings[DEV_DS2890], AddrBuffer, posBuffer);
//
//                 //Write it out
//                 m_Interface->Update();
//                 WriteOnInterfacePorts(message.c_str(), message.size());
//
//                 //wait a bit to not overflow the socket
//                 //msDelay(50);
//             }
        }
        break;
        case DEV_DS2408:
        case DEV_DS2405:
        case DEV_DIDO:
        {
            int memoryAddr;

            //writeOut positions
            for (i = 0; i < nOfValues; i++)
            {
                devIndex = values[i][0];

                //Get memory address
                memoryAddr = net->CtrlList[devIndex]->GetMemoryAddress();
                sprintf (AddrBuffer, "%d", memoryAddr);

                //GetStatus
                if (values[i][1])
                {
                    strcpy (posBuffer, "1");
                }
                else
                {
                    strcpy (posBuffer, "0");
                }

                //Compose the message
                message = m_XMLParser.CreateMessage("DEVICE", 3, Device_strings[DEV_DIDO], AddrBuffer, posBuffer);

                //Write it out
                UpdateServerPorts(true, false);
                WriteOnInterfacePorts(message.c_str(), message.size());
            }

        }
        break;
        default:
        {
            message = "<ERROR TYPE=\"DEVICE_UNKNOWN\" />";

            UpdateServerPorts(true, false);
            WriteOnInterfacePorts( message.c_str(), message.size());
        }
    }

    return true;
}

bool COneWireEngine::OutputIntData( e_DeviceType deviceType, int netIndex, int nOfValues, int values[MAX_NUM_DEV_PER_NET][3] )
{
    int devIndex = 0, i = 0, address = -1;
    CString message;
    T_Net *net;
    char addressBuffer[32], val1Buffer[8], val2Buffer[8];
    int alMax = 0, alMin = 0;

    memset (addressBuffer, 0, 32*sizeof(char));
    memset (val1Buffer, 0, 8*sizeof(char));
    memset (val2Buffer, 0, 8*sizeof(char));

    //Get Handler to the net
    net = m_Net->GetNetHandler(netIndex);

    switch (deviceType)
    {
        case DEV_TEMPCTRL:
        {
            for (i = 0; i < nOfValues; i++)
            {
                devIndex = values[i][0];

                //Get the Address and convert it to char
                address = net->CtrlList[devIndex]->GetMemoryAddress();
                sprintf (addressBuffer, "%d", address);

                alMax = values[i][1];
                alMin = values[i][2];

                sprintf (val1Buffer, "%d", alMin); //Minimum alarm level
                sprintf (val2Buffer, "%d", alMax); //Maximum alarm level

                message = m_XMLParser.CreateMessage("DEVICE", 5, Device_strings[DEV_TEMPCTRL], addressBuffer, "AlarmLevels", val1Buffer, val2Buffer);

                //Write it out
                UpdateServerPorts(true, false);
                WriteOnInterfacePorts(message.c_str(), message.size());
            }

        };
        break;
        default:
        {
            message = "<ERROR >";
        }
    }

    return true;
}

//////////////////////////////////////////////////////////
//                        ParseCommand                   //
//////////////////////////////////////////////////////////
int COneWireEngine::ParseCommand( )
{
    int index;
    CString inCommand;

    //Get the command
    inCommand = m_XMLParser.GetCommand();

    if (m_DoDebug)
    {
        cout << "Comando ricevuto: " << inCommand << " -- " << m_XMLParser.GetContent() << endl;cout.flush();
    }

    for (index = COMM_NONE; index < COMM_NUMTOT; index = index +1)
    {
        if (!strcasecmp(commandStrings[index], inCommand.c_str()))
        {
            break;
        }
    }

    return index;
}

//////////////////////////////////////////////////////////
//                        WriteLog                     //
//////////////////////////////////////////////////////////
bool COneWireEngine::WriteLog( int netIndex )
{
    T_Net *net;
    CString message;
    unsigned int i = 0;
    char timeBuffer[32];
    CString fileConfigString;
    CString fileName;
    FILE *fileHandler = 0x0;
    e_DeviceType controllerType;
    time_t actTime;

    //Get actual time
    time(&actTime);

    //Init the last time we recorded the data
    if (m_LastLogTime == -1)
    {
        m_LastLogTime = actTime;
    }
    else if (m_LastLogTime == -2)
    {
        //no log
        return true;
    }

    //Check if we have to save the file or not
    if ((!m_EnableLog) || (actTime < m_LastLogTime + m_LogInterval))
    {
        return true;
    }

    m_LastLogTime = actTime;

    if (m_DoDebug)
        cout << "Saving to file\n" << endl;

    //Check if file size is too big, in this case, delete the file
    fileName = m_IniFile->GetString(Config_Strings[CONF_FILENAME], "COMMON");

    //Write in the log file the current time
    ctime_r(&actTime, timeBuffer);

    //Start char for log message
    message = "< Ora Registrazione: ";
    message += timeBuffer;

    //Get the NET
    net = m_Net->GetNetHandler(netIndex);

    //Columns are as follows: 1 - time, 2 - net, 3 - number of sensors, from 4 the temperature recorded by each sensor
    message += "NET";

    //Add the net index
    message +=netIndex;
    message +="\r\n";

    //Start cycling through all the sensors
    for (i = 0; i < net->CtrlList.size(); i++)
    {
        //get the family number
        controllerType = net->CtrlList.at(i)->GetControllerType();

        message+=net->CtrlList.at(i)->GetComment();
        message+="\t\t";
        message+=Device_strings[controllerType];
        message+="\t\t";

        switch (controllerType)
        {
            case DEV_TEMPCTRL:
            {
                if ((net->hasTempDevices)&&(m_PollTemperature))
                {
                    message+=((CTempCtrl*)(net->CtrlList.at(i)))->GetLastTemp();
                }
            }
            break;
            case DEV_DIDO:
            {
                if ((net->hasDIDOs)&&(m_PollDigitalIO))
                {
                    message+=((CDigitalIO*)(net->CtrlList.at(i)))->GetLastState();
                }
            }
            break;
            case DEV_HUMIDITY:
            {
                if ((net->hasHums)&&(m_PollHum))
                {
                    message+=((CHumController*)(net->CtrlList.at(i)))->GetLastAbsHumidity();
                }
            }
            break;
            case DEV_AIAO:
            {
                if ((net->hasAIAOs)&&(m_PollAnalogIO))
                {
                    message+=((CAnalogIO*)(net->CtrlList.at(i)))->GetLastValue();
                }
            }
            break;
            case DEV_PIDSIMPLE:
            case DEV_PIDLMD:
            {
                if ((net->hasPIDs)&&(m_PollPID))
                {
                    message +=((CVPID*)(net->CtrlList.at(i)))->GetLastOutputInVolt();
                    message += "\t";

                    message += ((CVPID*)(net->CtrlList.at(i)))->GetControlVariable();

                    if (controllerType == DEV_PIDLMD)
                    {
                        message += "\t";
                        message += ((CPIDLMD*)(net->CtrlList.at(i)))->GetLMDVariable();
                    }
                }
            }
            break;
            case DEV_UTACTRL:
            {
                try
                {
                    //message+="\tUscita :";
                    message +=((CUtaCtrl*)(net->CtrlList.at(i)))->m_PIDLMD->GetLastOutputInVolt();

                    //message+="\tTAG:";
                    message+="\t";
                    message +=((CUtaCtrl*)(net->CtrlList.at(i)))->m_Tag->GetLastState();

                    if (((CUtaCtrl*)(net->CtrlList.at(i)))->HasShutterCommand())
                    {
                        if (((CUtaCtrl*)(net->CtrlList.at(i)))->HasOneShutterCommand())
                        {
                            //message+="\tSerrande (Apertura):";
                            message+="\t";
                            message+=((CUtaCtrl*)(net->CtrlList.at(i)))->m_OutDigitals[0]->GetLastState();

    //                         message+="\tSerrande (Chiusura):NA";
                            message+="\tNA";
                        }
                        else
                        {
    //                         message+="\tSerrande (Apertura):";
                            message+="\t";
                            message+=((CUtaCtrl*)(net->CtrlList.at(i)))->m_OutDigitals[0]->GetLastState();

    //                         message+="\tSerrande (Chiusura):";
                            message+="\t";
                            message+=((CUtaCtrl*)(net->CtrlList.at(i)))->m_OutDigitals[1]->GetLastState();

                        }
                    }
                    else
                    {
    //                     message+="\tSerrande (Apertura):NA";
                        message+="\tNA";

    //                     message+="\tSerrande (Chiusura):NA";
                        message+="\tNA";
                    }

                    if (((CUtaCtrl*)(net->CtrlList.at(i)))->HasOneFanCommand())
                    {
    //                     message+="\tVentilanti (Mandata):";
                        message+="\t";
                        message+=((CUtaCtrl*)(net->CtrlList.at(i)))->m_OutDigitals[2]->GetLastState();

    //                     message+="\tVentilanti (Ripresa):NA";
                        message+="\tNA";
                    }
                    else
                    {
    //                     message+="\tVentilanti (Mandata):";
                        message+="\t";
                        message+=((CUtaCtrl*)(net->CtrlList.at(i)))->m_OutDigitals[2]->GetLastState();

    //                     message+="\tVentilanti (Ripresa):";
                        message+="\t";
                        message+=((CUtaCtrl*)(net->CtrlList.at(i)))->m_OutDigitals[3]->GetLastState();

                    }

    //                 message+="\tT mandata:";
                    message+="\t";
                    message+=((CUtaCtrl*)(net->CtrlList.at(i)))->m_PIDLMD->GetLMDVariable();

    //                 message+="\tT ripresa:";
                    message+="\t";
                    message+=((CUtaCtrl*)(net->CtrlList.at(i)))->m_PIDLMD->GetControlVariable();
                }
                catch (exception &e)
                {
                    if (m_DoDebug)
                    {
                        cout << "Errore in generazione Log per UTACtrl: "<<e.what()<<endl;
                    }

                    message += "ERRORE!! Impossibile leggere dati da dispositivo (indice sbagliato ?)";
                }

            }
            break;
            case DEV_DI2AO:
            {
                unsigned int inVectSize, outVectSize;
                int inputs = 0;

                try
                {
                    inVectSize = ((CDI2AO*)(net->CtrlList.at(i)))->m_InVector.size();
                    outVectSize = ((CDI2AO*)(net->CtrlList.at(i)))->m_InVector.size();

                    message += inVectSize;
                    message +="\t";

                    message += ((CDI2AO*)(net->CtrlList.at(i)))->m_InVector.size();
                    message += "\t";

                    //Get States of inputs
                    int lastState = 0;
                    for (i = 0; i < inVectSize; i++)
                    {
                        lastState = (((CDI2AO*)(net->CtrlList.at(i)))->m_InVector.at(i)->GetLastState());

                        inputs = (lastState<<i) | inputs;
                    }
                    message += inputs;
                    message += "\t";

                    //Get States of outputs
                    for (i = 0; i < outVectSize; i++)
                    {
                        message += ((CDI2AO*)(net->CtrlList.at(i)))->m_OutVector.at(i)->GetLastValue();
                        message += "\t";
                    }
                }
                catch (exception &e)
                {
                    //TODO da migliorare
                    cout << "Errore in generazione LOG per di2ao: "<< e.what() << endl;
                    message += "ERRORE!! Impossibile leggere dati da dispositivo (indice sbagliato ?)";
                }
            }
            break;
            case DEV_REMOTEDIDO:
            case DEV_BUTTONCTRL:
            case DEV_STEPDIGITALOUT:
            case DEV_TAGCTRL:
            {
                int nOfInputs, nOfOutputs, index;
                try
                {
                    nOfInputs = ((CVMultiDIDO*)(net->CtrlList.at(i)))->GetNofInputs();
                    nOfOutputs = ((CVMultiDIDO*)(net->CtrlList.at(i)))->GetNofOutputs();

                    message += nOfInputs;
                    message += "\t";

                    message += nOfOutputs;
                    message += "\t";

                    for (index = 0; index < nOfInputs; index++)
                    {
                        message += ((CVMultiDIDO*)(net->CtrlList.at(i)))->GetInputState(false, index);
                        message += "\t";
                    }

                    for (index = 0; index < nOfOutputs; index++)
                    {
                        message += ((CVMultiDIDO*)(net->CtrlList.at(i)))->GetOutputState(false, index);
                        message += "\t";
                    }
                }
                catch (exception &e)
                {
                    if (m_DoDebug)
                    {
                        cout << cout << "Errore in generazione Log per " << Device_strings[controllerType] << endl;
                    }

                    message += "ERRORE!! Impossibile leggere dati da dispositivo (indice sbagliato ?)";
                }
            }
            break;
            default:
            {
                message += "Dispositivo sconosciuto";
            }
        }

        message += "\r\n";
    }


    message+="\r\n";

//     cout <<"Trimmo il file"<<endl;

    if (!TrimFile(fileName, message.size(), m_MaxFileSize))
    {
        return false;
    }

    //Controllo dimensione file
    //Open file
    fileHandler = fopen(fileName.c_str(), "a+");

    //Get file position
    fseek(fileHandler,0, SEEK_END);

    fprintf(fileHandler, "%s", message.c_str());

    //Close the file
    fclose(fileHandler);

    return true;

}

//////////////////////////////////////////////////////////
//                        TrimFile
//////////////////////////////////////////////////////////
bool COneWireEngine::TrimFile(CString fileName,unsigned int bytesToTrim, int maxFileSize)
{
    bool retVal = false;
    char *fileBuffer = 0x0, *pCutBegin, *pCutEnd;
    CString fileConfigString;
    FILE *fileHandler = 0x0;
    unsigned long curPos = 0;

    if (fileName.size() == 0)
    {
        return retVal;
    }
    else if (bytesToTrim > maxFileSize)
    {
        //Error the number of bytes to insert is greater than the file size... dunno wht to do
        return true;
    }

//     cout << "Trim: Apro il file:"<< fileName<<endl;

    //Controllo dimensione file
    //Open file
    fileHandler = fopen(fileName.c_str(), "a+");

    //Get file position
    fseek(fileHandler,0, SEEK_END);
    curPos = ftell(fileHandler);

//     cout<<"La posizione corrente è: "<<curPos<<" E i byte da aggiungere sono: "<<bytesToTrim<<endl;


    if ((curPos + bytesToTrim) > maxFileSize)
    {
        //Devo riscrivere il file: leggo tutto il contenutolo
        fseek (fileHandler, 0, SEEK_SET);
        fileBuffer = (char*)(calloc(curPos+1,1));
        fread(fileBuffer, sizeof(char), curPos, fileHandler);

        //chiudo e riapro in scrittura
        fclose (fileHandler);
        fileHandler = fopen(fileName.c_str(), "w");

        //Mi posizone al primo log e al secondo
        pCutBegin = strchr(fileBuffer,'<');
        pCutEnd = strchr(pCutBegin+1,'<');

        if (pCutBegin == 0x0)
        {
            return false;
        }
        else if (pCutEnd != 0x00)
        {
            //Se c'e' almeno un altro log cerco di liberare lo spazio che mi serve, altrimenti (pCutEnd == 0x00) sovrascrivo e basta
            //Finche' la dimensione del blocco da togliere e' tale per cui togliendolo il messaggio non ci sta aumento il blocco
            while ( curPos + bytesToTrim - (pCutEnd - pCutBegin) > maxFileSize )
            {
                pCutEnd = strchr(pCutEnd +1, '<');
                if (pCutEnd == 0x00)
                {
                    //non ci sono altri messaggi, posiziono il cursore all'inizio e sovrascrivo tutto
                    pCutEnd = fileBuffer;
                    break;
                }
            }

            //sposto il blocco all'inizio
            memmove (fileBuffer, pCutEnd, curPos - (pCutEnd - fileBuffer));

            //Azzero il resto
            memset (fileBuffer + curPos - (pCutEnd - pCutBegin), 0x0, pCutEnd - pCutBegin);

            //Scrivo nel file
            fprintf (fileHandler, "%s", fileBuffer);

            retVal = true;
        }
    }
    else
    {
        retVal = true;
    }

    //Close the file
    fclose(fileHandler);

    if (fileBuffer != 0x0)
    {
        free (fileBuffer);
    }

    return retVal;
}

//////////////////////////////////////////////////////////
//                        SetState
//////////////////////////////////////////////////////////
bool COneWireEngine::SetRunLevel( int newRunLevel )
{
    if (newRunLevel < 4)
    {
        m_RunLevel = newRunLevel;
        return true;
    }
    else
    {
        return false;
    }
}

//////////////////////////////////////////////////////////
//                       SendStrongAlarm
//////////////////////////////////////////////////////////
bool COneWireEngine::ActivateGeneralAlarm( e_GeneralAlarm alarmType)
{
    int devIndex = 0;
    T_Net *net = 0x0;
    int netIndex = 0;
    vector<CVController*>::iterator ctrlIt;


    //TODO sarà riempita in seguito

    switch (alarmType)
    {
        case TEMPALARM_DATA:
        {
            bool activateGenAl = false;

            for (netIndex = 0; netIndex < m_TotalNets; netIndex++)
            {
                if (m_Net->AcquireNet( netIndex ))
                {
                    net = m_Net->GetNetHandler( netIndex );
                    for (ctrlIt = net->CtrlList.begin(); ctrlIt < net->CtrlList.end(); ctrlIt++)
                    {
                        if ((*ctrlIt)->GetControllerType() == DEV_TEMPCTRL)
                        {
                            if (m_DoDebug)
                            {
                                cout << "Controllo stato allarme combinatore per controller Indirizzo: "<< (*ctrlIt)->GetMemoryAddress()<<endl;
                            }

                            activateGenAl = activateGenAl || ((CTempCtrl*)(*ctrlIt))->GetPhoneAlarmState();

                            if (m_DoDebug && activateGenAl)
                            {
                                cout << "Allarme Generale Attivo!!!"<<endl;
                            }
                        }
                    }
                }
            }

            net = m_Net->GetNetHandler( m_GeneralAlarmNet );
            devIndex = m_Net->GetDeviceIndexByConfigNumber( m_GeneralAlarmNet, m_GeneralAlarmDevice);

            if (m_GeneralAlarmInverted)
            {
                activateGenAl = !activateGenAl;
            }



            if (devIndex >= 0)
            {
                if (m_Net->AcquireNet(m_GeneralAlarmNet))
                {
                    ((CDS2408*)(net->deviceList[devIndex]))->SetLatchState( m_GeneralAlarmChannel, activateGenAl );
                }
            }
        };break;
        default: break;
    }

    return true;
}


//////////////////////////////////////////////////////////
//                       SetupNet
//////////////////////////////////////////////////////////
bool COneWireEngine::SetupNet( int netIndex )
{
    bool isSetupOK = false;
    int counter = 0;
    T_Net *net;

    net = m_Net->GetNetHandler(netIndex);

    if (!net) {
        return false;
    }

    //If we are wireless try to setup the NET up to 5 times
    if (net->isWl || net->isOverIp )
    {
        //Write in the master the fact that we are wireless because we have only one master object
        m_Net->m_Master->SetWireless( 1 );
        m_Net->m_Master->SetNetDelay (net->netDelay);
        counter = 5;
    }
    else
    {
        //Write in the master the fact that we are not wireless because we have only one master object
        m_Net->m_Master->SetWireless( 0 );
        counter = 1;
    }

    while (counter > 0)
    {
        //Set the alarms and the switches
        isSetupOK = true;

        if ( m_Net->InitTempControllers( netIndex) )
        {
            if (m_DoDebug)
            {
                cout << "Sensori di temperatura Inizializzati" << endl; cout.flush();
            }

        }
        else
        {
            cout << "ERRORE!!! Impossibile inizializzare i sensori di temperatura" << endl; cout.flush();
            isSetupOK = false;
        }


        if ( m_Net->InitializeDOs(netIndex) )
        {
            if (m_DoDebug)
            {
                cout << "DIDOs Inizializzati" << endl; cout.flush();
            }
        }
        else
        {
            cout << "ERRORE!!! Impossibile inizializzare i DIDO" << endl; cout.flush();
            isSetupOK = false;
        }

        isSetupOK = m_Net->InitializeAOs( netIndex ) && isSetupOK;

        if ( isSetupOK )
        {
            if (m_DoDebug)
            {
                cout << "AIAOs Inizializzati" << endl; cout.flush();
            }
        }
        else
        {
            cout << "ERRORE!!! Impossibile inizializzare gli AIAO" << endl; cout.flush();
            isSetupOK = false;
        }

        if ( m_Net->InitializeAdvancedControllers( netIndex ))
        {
            if (m_DoDebug)
            {
                cout << "Controlli Valvole Inizializzati" << endl;
            }
        }
        else
        {
            cout << "ERRORE!!! Impossibile inizializzare i controlli valvole" << endl; cout.flush();
            isSetupOK = false;
        }

        if ( m_Net->InitializePIDs( netIndex ))
        {
            if (m_DoDebug)
            {
                cout << "PID Inizializzati" << endl;
            }
        }
        else
        {
            cout << "ERRORE!!! Impossibile inizializzare i PID" << endl; cout.flush();
            isSetupOK = false;
        }

        if ( m_Net->InitializeCoordinators( netIndex ))
        {
            if (m_DoDebug)
            {
                cout << "Coordinatori Inizializzati" << endl;
            }
        }
        else
        {
            cout << "ERRORE!!! Impossibile inizializzare i Coordinatori" << endl; cout.flush();
            isSetupOK = false;
        }

        //Controllo se la NET gestisce dei moduli camera e se il file ini è a posto
        if (net->hasHotelRooms){
            if (m_Net->InitializeMGC(netIndex)){
                if (m_DoDebug){
                    cout<<"MGC Inizializzati"<<endl;
                }
            }
            else {
                isSetupOK = false;
            }
        }

        if (net->hasAccessControl) {
            if (m_Net->InitializeACC(netIndex)) {
                if (m_DoDebug){
                    cout << "ACC Inizializzati"<<endl;
                }
                else {
                    isSetupOK = false;
                }
            }
        }

        if (m_Net->InitializeCNT( netIndex ))
        {
            if (m_DoDebug)
            {
                cout << "Moduli Contabilizzatori Inizializzati"<<endl;
            }
        }
        else
        {
            cout << "ERRORE!! Impossibile  inizializzare i moduli contabilizzatori"<<endl;
        }

        if (isSetupOK)
        {
            break;
        }

        m_Net->ClearAllActivityLatches( netIndex );

        //Decrease counter
        counter--;
    }



    m_Net->SetSetupState(netIndex, true);

    if (!isSetupOK)
    {
        m_AfoErrorHandler.PushError( AFOERROR_UNABLE_TO_SETUP_NET, netIndex + 1);

        //NET not initialized: update the blacklist
        if (m_ManageBlackList){
            m_Net->UpdateBlackList( netIndex );
        }

        //TODO c'e' da mettere a posto qualcosa perche'nel caso il setup non funzioni potremmo avere uno stato indefinito
//         m_Net->SetSetupState( netIndex, false);

    }

    return isSetupOK;

}

//////////////////////////////////////////////////////////
//               ManageTemperatureDevices
//////////////////////////////////////////////////////////
bool COneWireEngine::ManageTemperatureDevices( int netIndex )
{
    int i = 0, numAlarms = 0, numTemps = 0;
    float temps[MAX_NUM_DEV_PER_NET][2];
    int alarms [MAX_NUM_DEV_PER_NET];
    bool retVal = true;
    bool skipOutputData = false;
    bool noErrorOnSensor = true;
    T_Net *net;
    vector<CVController*>::iterator ctrlIt;
    bool forceCommandCheck = false;

    unsigned long int actTime;

    memset (temps, 0x0, MAX_NUM_DEV_PER_NET*2*sizeof(float));
//     memset (alarms, 0x0, MAX_NUM_DEV_PER_NET*sizeof(int));

    actTime = time(0x0);

//     //Update all the temps
//     if (!m_Net->UpdateAllTemp( netIndex, CONVERT_T))
//     {
//         m_AfoErrorHandler.PushError( AFOERROR_UNABLE_TO_UPDATE_ALL_TEMP, netIndex+1);
//         return false;
//     }


    //TBR -- da rimettere come era prima
    net = m_Net->GetNetHandler(netIndex);

    if (!net->tempConversionLaunched)
    {
        //Update all the temps
        if (!m_Net->UpdateAllTemp( netIndex, CONVERT_T))
        {
            m_AfoErrorHandler.PushError( AFOERROR_UNABLE_TO_UPDATE_ALL_TEMP, netIndex+1);

            return false;
        }
        else
        {
            net->tempConversionLaunched = true;
            net->timeOfLastTempConversion = time(0x0);
            return true;
        }
    }
    else if (actTime < net->timeOfLastTempConversion + 2)
    {
        return true;
    }

    net->tempConversionLaunched = false;

    //numTemps = m_Net->GetAllTemp(netIndex, true, temps, MAX_NUM_DEV_PER_NET);



    for (ctrlIt = net->CtrlList.begin(); ctrlIt < net->CtrlList.end(); ctrlIt++)
    {
        //Resetto il flag dei comandi
        forceCommandCheck = false;

        if ( ((*ctrlIt)->GetControllerType() == DEV_TEMPCTRL) || ((*ctrlIt)->GetControllerType() == DEV_TEMPCTRLHYST) )
        {
            int devIndex = ctrlIt - net->CtrlList.begin();

            //Aggiorno i comandi solo se faccio effettivamente qualcosa
            forceCommandCheck = true;

            temps[numTemps][0] = devIndex;
            noErrorOnSensor = m_Net->GetTemp(&temps[numTemps][1], netIndex, devIndex, false) && noErrorOnSensor;
            numTemps++;
        }

        if (forceCommandCheck)
        {
            CheckForCommands( netIndex );
        }
    }


    //If it is a temperature controller with hysteresis, update it
    m_Net->UpdateAllHysteresisCtrl( DEV_TEMPCTRLHYST, netIndex, false);

    if (numTemps < 0)
    {
        m_AfoErrorHandler.PushError( AFOERROR_UNABLE_TO_GET_TEMPS, netIndex+1);

        skipOutputData = true;

    }
    else if (numTemps == 0)
    {
        m_AfoErrorHandler.PushError( AFOERROR_TEMP_SENSORS_NOT_FOUND, netIndex+1);
        return false;
    }


    if (m_DoDebug)
        cout << "Cerco sensori in allarme" << endl;


    if (m_PollTempAlarms)
    {
        numAlarms = m_Net->SearchAlarmTemp(netIndex, DS18S20_FN, alarms, MAX_NUM_DEV_PER_NET);

        //Inserisco un ciclo di aggiornamento degli allarmi interni ai dispositivi
        for (ctrlIt = net->CtrlList.begin(); ctrlIt < net->CtrlList.end(); ctrlIt++)
        {
            if ((*ctrlIt)->GetControllerType() == DEV_TEMPCTRL)
            {
                ((CTempCtrl*)(*ctrlIt))->Update(false);
            }

            CheckForCommands( netIndex );
        }

    }
    else
    {
        numAlarms = 0;
    }

    if (numAlarms == 0)
    {
        if (m_DoDebug)
        {
            cout << "Allarmi non trovati" << endl;cout.flush();
        }
    }
    else if (numAlarms < 0)
    {
        m_AfoErrorHandler.PushError( AFOERROR_UNABLE_TO_GET_TEMPALARMS, netIndex +1);
    }
    else
    {
        if (m_DoDebug)
            cout << "Numero di sensori in allarme trovati: " << numAlarms << endl;cout.flush();
    }

    //Write out the logs and save to file
    if (m_DoDebug)
    {
        int devIndex = 0;

        cout << "Numero di temperature acquisite " << numTemps << " \nElenco i valori :" << endl;cout.flush();cout.flush();

        for (i = 0; i < numTemps; i++)
        {
            //Becca la rete
            net = m_Net->GetNetHandler(netIndex);
            devIndex = (int)temps[i][0];
            cout<< "devIndex : " << net->CtrlList[devIndex]->GetConfigFileDevIndex();
            cout << " Address: " << net->CtrlList[devIndex]->GetMemoryAddress();
            cout << "  Temp:  " <<  temps[i][1];
            cout << " Comment : "<<net->CtrlList[devIndex]->GetComment()<< endl;cout.flush();
        }
    }

    if (skipOutputData)
    {
        retVal = false;
    }
    else
    {
        //Send data acquired via LAN
        if (m_DoDebug)
            cout << "Invio le temperature e gli allarmi via rete" << endl;

        //perform a Read to check if the port is connected
        UpdateServerPorts(true, false);
        OutputFloatData(netIndex, numTemps, temps);

        retVal = true;
    }

    return (retVal && noErrorOnSensor);
}

//////////////////////////////////////////////////////////
//               ManagePID
//////////////////////////////////////////////////////////
bool COneWireEngine::ManagePID( int netIndex )
{
    bool retVal = false;
    T_Net *net;
    unsigned int pidIndex = 0;
    float lastPositionArray[MAX_NUM_DEV_PER_NET][2];
    int lastPosIndex = 0;
    bool forceCommandCheck = false;
    bool writeOnOutput = false;


    memset (lastPositionArray, 0, sizeof(int)*MAX_NUM_DEV_PER_NET);

    //Get NET handler
    net = m_Net->GetNetHandler( netIndex);

    //First convert all the temperatures, then proceed with the calculus
//     m_Net->UpdateAllTemp(netIndex, CONVERT_T);

    if (net != 0x0)
    {
        for (pidIndex = 0; pidIndex < net->CtrlList.size(); pidIndex++)
        {
            forceCommandCheck = false;

            if ( m_Net->CheckControllerType( netIndex, pidIndex, DEV_PIDSIMPLE) || m_Net->CheckControllerType( netIndex, pidIndex, DEV_PIDLMD) )
            {
                forceCommandCheck = true;
                //Update the PID
                if (net->CtrlList[pidIndex]->Update(false))
                {
//                     lastPositionArray[lastPosIndex][0] = pidIndex;
//                     lastPositionArray[lastPosIndex][1]= ((CVPID*)(net->CtrlList[pidIndex]))->GetLastOutputInVolt();
//                     lastPosIndex++;

                    if (m_DoDebug)
                    {
                        cout << "Indirizzo PID: " << net->CtrlList[pidIndex]->GetMemoryAddress();
                        cout << " SetPoint PID: " << ((CVPID*)(net->CtrlList[pidIndex]))->GetSetPoint();
                        cout << " Uscita PID: " << ((CVPID*)(net->CtrlList[pidIndex]))->GetLastOutputInVolt();
                        cout << " Summer PID: " << ((CVPID*)(net->CtrlList[pidIndex]))->GetSummer();
                        cout << " Comment : "<<net->CtrlList[pidIndex]->GetComment()<< endl;cout.flush();
                    }

                    Cmd com("DEVICE");

                    com.putValue("TYPE","PIDOutput");
                    com.putValue("ADDRESS",net->CtrlList[pidIndex]->GetMemoryAddress());
                    com.putValue("VAL",((CVPID*)(net->CtrlList[pidIndex]))->GetLastOutputInVolt());
                    com.putValue("SETPOINT",((CVPID*)(net->CtrlList[pidIndex]))->GetSetPoint());
                    com.putValue("SUMMER", ((CVPID*)(net->CtrlList[pidIndex]))->GetSummer());

                    CString message = com.getXMLValue();
                    WriteOnInterfacePorts(message.c_str(), message.size());

                    retVal = true;
                }
                else
                {
                    //TODO inserire messaggio di errore
                    if (m_DoDebug)
                    {
                        cout << "Impossibile aggiornare l'uscita del PID avente indirizzo :" << net->CtrlList[pidIndex]->GetMemoryAddress()<<endl;
                        cout.flush();
                    }

                    retVal = false;
                }
            }
            else if (m_Net->CheckControllerType( netIndex, pidIndex, DEV_UTACTRL))
            {
                forceCommandCheck = true;
                //Update the PID
                if (net->CtrlList[pidIndex]->Update(false))
                {
                    lastPositionArray[lastPosIndex][0] = pidIndex;
                    lastPositionArray[lastPosIndex][1]= ((CUtaCtrl*)(net->CtrlList[pidIndex]))->m_PIDLMD->GetLastOutputInVolt();
                    lastPosIndex++;

                    if (m_DoDebug)
                    {
                        cout << "Indirizzo PID(UTACtrl): " << net->CtrlList[pidIndex]->GetMemoryAddress();
                        cout << " Uscita PID(UTACtrl): " << lastPositionArray[lastPosIndex-1][1];
                        cout << " Comment : "<<net->CtrlList[pidIndex]->GetComment()<< endl;cout.flush();
                    }

                    writeOnOutput = true;
                    retVal = true;
                }
                else
                {
                    //TODO inserire messaggio di errore
                    if (m_DoDebug)
                    {
                        cout << "Impossibile aggiornare l'uscita del PID(UTACtrl) avente indirizzo :" << net->CtrlList[pidIndex]->GetMemoryAddress()<<endl;
                        cout.flush();
                    }

                    retVal = false;
                }

            }
            else if (m_Net->CheckControllerType( netIndex, pidIndex, DEV_C3POINT) )
            {
                C3PointCtrl *ctrlIt = (C3PointCtrl*)(net->CtrlList[pidIndex]);
                forceCommandCheck = true;
                ctrlIt->Update(false);

                Cmd com("DEVICE");
                com.putValue("TYPE","C3POINTCTRL");
                com.putValue("ADDRESS",net->CtrlList[pidIndex]->GetMemoryAddress());
                com.putValue("SUMMER",ctrlIt->m_IsSummer);
                com.putValue("STATUS", ctrlIt->m_ControllerStatus);
                com.putValue("SETPOINT", ctrlIt->m_Setpoint);
                com.putValue("FULLOPEN", ctrlIt->m_IsFullOpen);
                com.putValue("FULLCLOSED", ctrlIt->m_IsFullClosed);

                CString message = com.getXMLValue();
                WriteOnInterfacePorts(message.c_str(), message.size());

                retVal = true;
            }

            if (forceCommandCheck)
            {
                CheckForCommands( netIndex );
            }

        }

        if (lastPosIndex > 0)
        {
            if (writeOnOutput)
                OutputFloatData(netIndex, lastPosIndex, lastPositionArray);
        }
    }

    return retVal;
}

//////////////////////////////////////////////////////////
//               ManageSwitches
//////////////////////////////////////////////////////////
bool COneWireEngine::ManageDIDO( int netIndex )
{
    T_Net* net;
    int didoStates[MAX_NUM_DEV_PER_NET][2];
    vector<CVController*>::iterator ctrlIt;
    int nOfDevices=0;


    //Aggiorno i registri
    m_Net->UpdateDIDOsRegisters(netIndex);

    net = m_Net->GetNetHandler(netIndex);

    if (net == 0x0)
    {
        cout << "Handler non valido!!! NetIndex =  "<<netIndex<<endl;
        return false;
    }

    //Costruisco il vettore degli stati
    for (ctrlIt = net->CtrlList.begin(); ctrlIt < net->CtrlList.end(); ctrlIt++)
    {
        if ( (*ctrlIt)->GetControllerType() == DEV_DIDO )
        {
            int didoState;

            ((CDigitalIO*)(*ctrlIt))->Update(false);

            //Controllo di coerenza del DIDO: se per caso e' stato staccato il bus in questo modo lo riforzo
            didoState = ( ( CDigitalIO* ) (*ctrlIt ))->GetLastState();

            if ( (!(( CDigitalIO* )(*ctrlIt ))->IsDataAligned(false)) && (( CDigitalIO* ) (*ctrlIt ))->GetStateCheck() )
            {
                if ( m_DoDebug )
                {
                    cout << "Indirizzo DIDO : "<< ( ( CDigitalIO* ) ( *ctrlIt ) )->GetMemoryAddress() << " Stato Interno: "<<didoState<<" Stato reale : "<< !didoState << " Reimposto lo stato"<< endl;
                }

                ( ( CDigitalIO* ) (*ctrlIt ))->SetState(didoState, true);

            }


            didoStates[nOfDevices][0] = ctrlIt - net->CtrlList.begin();
            didoStates[nOfDevices][1] = ((CDigitalIO*)(*ctrlIt))->GetState(false);
            if (m_DoDebug==2)
            {
                cout << "Tipo: DIDO";
                cout << " Address: " << (*ctrlIt)->GetMemoryAddress() << " Stato:  " << didoStates[nOfDevices][1] ;
                cout << " Comment: " << (*ctrlIt)->GetComment() << endl;cout.flush();
            }

            nOfDevices++;
        }/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        else if ((*ctrlIt)->GetControllerType() == DEV_REMOTEDIDO )
        {
            int address = -1;
            if ( ( ( CVMultiDIDO* ) (*ctrlIt)  )->GetActivity ( false ) )
            {
                address = (*ctrlIt)->GetMemoryAddress();
                if (m_DoDebug==2)
                {
                    cout << "Tipo: RemoteDIDO";
                    cout << " Address: " << address << " Comment : "<<(*ctrlIt)->GetComment();
                    cout << "  Attivita' rilevata  " << endl;cout.flush();
                }

                SendCommandToRemoteDO( 1, &address);
            }
        }/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        else if ((*ctrlIt)->GetControllerType() == DEV_TAGCTRL )
        {
            int address = -1
                    ;
            ( ( CVMultiDIDO* ) ( *ctrlIt ) )->Update ( false );

            if ( ( ( CVMultiDIDO* ) ( *ctrlIt ) )->IsRemoted() )
            {
                address = (*ctrlIt)->GetMemoryAddress();
                if (m_DoDebug==3)
                {
                    cout << "Tipo: TAG Controller";
                    cout << " Address: " << address << " Comment : "<<(*ctrlIt)->GetComment();
                    cout << "  StatoIngresso:  " << ((CTAGControl*)(*ctrlIt))->GetInputState(false)<<endl;cout.flush();
                }

                SendCommandToRemoteDO( 1, &address);
            }
        }/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        else if ((*ctrlIt)->GetControllerType() == DEV_STEPDIGITALOUT )
        {
            //Se sto cercando attività questo funziona solo così altrimenti controlla lo stato
            if (m_PollActivity)
            {
                if ( ( ( CVMultiDIDO* ) (*ctrlIt ) )->GetActivity ( false ) )
                {
                    if ( m_DoDebug == 3)
                    {
                        cout << "Tipo:StepDigitalOutput";
                        cout << " Address: " << (*ctrlIt)->GetMemoryAddress() << " Comment : "<<(*ctrlIt)->GetComment();
                        cout << "  Attivita' rilevata  " << endl;cout.flush();
                    }

                    ((CStepDigitalOut*)(*ctrlIt))->ChangeOutput();
                }
            }
            else
            {
                ( ( CStepDigitalOut* ) ( *ctrlIt ) )->Update ( false );
            }
            
            //TODO qui devo inviare lo stato dell'ingresso
            Cmd com("DEVICE");
            com.putValue("TYPE","DIDO");
            com.putValue("ADDRESS",(*ctrlIt)->GetMemoryAddress());
            com.putValue("STATE",( ( CVMultiDIDO* ) (*ctrlIt ) )->GetInputState(false));
            com.putValue("VAL",( ( CVMultiDIDO* ) (*ctrlIt ) )->GetInputState(false));
            
            UpdateServerPorts(true, false);
            WriteOnInterfacePorts(com.getXMLValue().c_str(), com.getXMLValue().size());
        }/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        else if ((*ctrlIt)->GetControllerType() == DEV_BUTTONCTRL)
        {
            if ( ( ( CVMultiDIDO* ) ( *ctrlIt ) )->GetActivity ( false ) )
            {
                if ( m_DoDebug == 3 )
                {
                    cout << "UpdateDIDO -------- Attivita' rilevata sul dispositivo di indirizzo "<< (*ctrlIt)->GetMemoryAddress() <<endl;
                }

                //Controllo se e' remotato o meno
                if ( ( ( CVMultiDIDO* ) ( *ctrlIt ) )->IsRemoted() )
                {
                    int address = -1;

                    address = (*ctrlIt)->GetMemoryAddress();
                    SendCommandToRemoteDO( 1, &address);
                }
                else
                {
                    int configDevIndex = -1;

                    //TODO da cambiare perchè sono costretto a chiamare direttamente la classe buttoncontroller e non multidido
                    if ( ( ( CButtonController* ) ( *ctrlIt ) )->m_IsJolly )
                    {
                            //Jolly device, apply changes to all devices belonging to the same NET
                        m_Net->SetAllDigitalOutputs ( netIndex, ( ( CButtonController* ) ( *ctrlIt ) )->m_JollyValue );
                    }
                    else
                    {
                        if ( ( ( CVMultiDIDO* ) (  *ctrlIt ) )->ChangeOutput() )
                        {
                            if (net->saveDigitalState)
                            {
                                //Save the INI file
                                configDevIndex =  (*ctrlIt)->GetConfigFileDevIndex();
                                if ( ( ( CButtonController* ) (  *ctrlIt ))->GetOutputState ( false ) )
                                {
                                    m_Net->UpdateIniFile ( netIndex, configDevIndex, "STARTV", "1" );
                                }
                                else
                                {
                                    m_Net->UpdateIniFile ( netIndex, configDevIndex, "STARTV", "0" );
                                }
                            }
                        }
                        else
                        {
                                //TODO mettere messaggio di errore
                        }
                    }
                }
            }//IF BUTTONCTRL
        }
    }

//     gettimeofday( &tpStop, 0x0);
//
//     thisCycleTime = (tpStop.tv_sec - tpStart.tv_sec)*1000+(tpStop.tv_usec - tpStart.tv_usec)/1000;
//     cout<<"\n\nTempo impiegato nell'aggiornamento Dispositivi (ms): "<< thisCycleTime<<endl;
//
//     gettimeofday( &tpStart, 0x0);

    if (CheckInterfacePortsForConnection())
    {
        //Mando via web gli stati
        OutputIntData( DEV_DIDO, netIndex, nOfDevices, didoStates );
    }

//     gettimeofday( &tpStop, 0x0);
//
//     thisCycleTime = (tpStop.tv_sec - tpStart.tv_sec)*1000+(tpStop.tv_usec - tpStart.tv_usec)/1000;
//     cout<<"\n\nTempo impiegato nell'invio WEB (ms): "<< thisCycleTime<<endl;

    return true;

}

//////////////////////////////////////////////////////////
//               ManageSwitches
//////////////////////////////////////////////////////////
bool COneWireEngine::ManageSwitches( int netIndex )
{
    int switches[MAX_NUM_DEV_PER_NET][2];
    int remoteDIDOs[MAX_NUM_DEV_PER_NET];
    int nOfRemoteDIDOs = 0;
    bool retVal = false;
    int nOfDevices = 0;
    int i = 0;
    T_Net* net;

    memset (switches, -1, 2*MAX_NUM_DEV_PER_NET*sizeof(int));
    memset (switches, -1, MAX_NUM_DEV_PER_NET*sizeof(int));

    nOfDevices = m_Net->UpdateDIDOs (netIndex, switches, remoteDIDOs, &nOfRemoteDIDOs);

    if (nOfDevices > 0)
    {
        if (m_DoDebug == 2)
        {
            int devIndex = 0;

            cout << "Numero DIDO: " << nOfDevices << " \nValori letti :" << endl;cout.flush();cout.flush();

            for (i = 0; i < nOfDevices; i++)
            {
            //Becca la rete
                net = m_Net->GetNetHandler(netIndex);
                devIndex = (int)switches[i][0];
                cout << " Address: " << net->CtrlList[devIndex]->GetMemoryAddress() << " Comment : "<<net->CtrlList[devIndex]->GetComment();
                cout << "  Stato :  " <<  switches[i][1] << endl;cout.flush();
            }
        }

        OutputIntData( DEV_DIDO, netIndex, nOfDevices, switches );
        retVal = true;
    }
    else if ( nOfDevices <= 0)
    {
        m_AfoErrorHandler.PushError( AFOERROR_DIDO_NOT_FOUND, netIndex+1);
        retVal = false;
    }

    if (nOfRemoteDIDOs > 0)
    {
        if (m_DoDebug == 2)
        {
            int devIndex;
            cout << "Numero DIDO remotati: " << nOfRemoteDIDOs << " \nValori letti :" << endl;cout.flush();cout.flush();
            try
            {
                for (i = 0; i < nOfRemoteDIDOs; i++)
                {
                    devIndex = m_Net->GetDeviceIndexByMemoryAddress( netIndex, remoteDIDOs[i]);

                    //Becca la rete
                    cout << " Address: " << remoteDIDOs[i] << " Comment : "<<net->CtrlList.at(devIndex)->GetComment();
                    cout << "  Stato Attivita':  1" <<  endl;cout.flush();
                }
            }
            catch (exception &e)
            {
                cout << "Si e' verificato un errore durante la chiusura della NET : "<< netIndex + 1<< " errore: " << e.what() << endl;
            }
        }

        SendCommandToRemoteDO( nOfRemoteDIDOs, remoteDIDOs);
        retVal = true;
    }


    return retVal;
}

//////////////////////////////////////////////////////////
//               ManageHum
//////////////////////////////////////////////////////////
bool COneWireEngine::ManageHum( int netIndex )
{
    float hums[MAX_NUM_DEV_PER_NET][3];
    bool retVal = false;
    bool noErrorOnSensors = true;
    int numHums = 0;
    int i = 0;
    T_Net *net;
    vector<CVController*>::iterator ctrlIt;
    bool forceCommandCheck = false;

    //Get Humidities
    //numHums = m_Net->GetAllHumidities( netIndex, hums, MAX_NUM_DEV_PER_NET);
    net = m_Net->GetNetHandler( netIndex );

    net->master->owTouchReset( net->portHandler );

    for (ctrlIt = net->CtrlList.begin(); ctrlIt != net->CtrlList.end(); ctrlIt++)
    {
        forceCommandCheck = false;
        if ((*ctrlIt)->GetControllerType() == DEV_HUMIDITY)
        {
            int devIndex = ctrlIt - net->CtrlList.begin();

            forceCommandCheck = true;
            hums[numHums][0] = devIndex;
            noErrorOnSensors = m_Net->GetHumidity( netIndex, devIndex, &hums[numHums][1], &hums[numHums][2]) && noErrorOnSensors;
            numHums++;
        }

        if (forceCommandCheck)
        {
            CheckForCommands( netIndex );
        }
    }

    //Update the humidity controller
    m_Net->UpdateAllHysteresisCtrl( DEV_HUMIDITY, netIndex, false);

    if (numHums > 0)
    {
        OutputFloatData( netIndex, numHums, hums );
        retVal = true;

        if (m_DoDebug)
        {
            int devIndex = 0;

            cout << "Numero Sensori Umidita' : " << numHums << " \nValori letti :" << endl;cout.flush();cout.flush();

            for (i = 0; i < numHums; i++)
            {
                devIndex = (int)hums[i][0];
                cout<< "CtrlIndex : " << devIndex;
                cout << " Address: " << m_Net->GetControllerMemoryAddress( netIndex, devIndex);
                cout << "  absH :  " <<  hums[i][1] << " relH: " << hums[i][2] << endl;cout.flush();cout.flush();
            }
        }

    }
    else
    {
        m_AfoErrorHandler.PushError( AFOERROR_HUMS_NOT_FOUND, netIndex +1);
    }

    return (retVal && noErrorOnSensors);
}

//////////////////////////////////////////////////////////
//               ManageAnalogIO
//////////////////////////////////////////////////////////
bool COneWireEngine::ManageAnalogIO( int netIndex )
{
    float aiao[MAX_NUM_DEV_PER_NET][2];
    bool retVal = false, isCurrent = false;
    bool noErrorOnSensor = true;
    int numAIAO = 0, address = 0;
    int i = 0;
    T_Net *net;
    vector<CVController*>::iterator ctrlIt;


    net = m_Net->GetNetHandler( netIndex );

//     net->master->owTouchReset( net->portHandler );

    for (ctrlIt = net->CtrlList.begin(); ctrlIt != net->CtrlList.end(); ctrlIt++)
    {
        if ((*ctrlIt)->GetControllerType() == DEV_AIAO)
        {
            int devIndex = ctrlIt - net->CtrlList.begin();
            aiao[numAIAO][0] = devIndex;
            address = (*ctrlIt)->GetMemoryAddress();
            noErrorOnSensor = m_Net->GetAnalogIO(address, &aiao[numAIAO][1], &isCurrent) && noErrorOnSensor;
            numAIAO++;
        }

        CheckForCommands( netIndex );
    }

//     net->master->owTouchReset( net->portHandler );

    if (numAIAO > 0)
    {
        OutputFloatData( netIndex, numAIAO, aiao );
        retVal = true;

        if (m_DoDebug)
        {
            int devIndex = 0;
            T_Net* net;

            net = m_Net->GetNetHandler(netIndex);

            cout << "Numero AI/AO: " << numAIAO << " \nValori letti :" << endl;cout.flush();cout.flush();

            for (i = 0; i < numAIAO; i++)
            {
                devIndex = (int)aiao[i][0];
                cout << " Address: " << m_Net->GetControllerMemoryAddress( netIndex, devIndex);
                cout << " Comment: " << net->CtrlList[devIndex]->GetComment();
                cout << "  Valore Letto/Impostato :  " <<  aiao[i][1] << endl;cout.flush();
            }
        }

    }
    else
    {
        m_AfoErrorHandler.PushError( AFOERROR_AIAO_NOT_FOUND, netIndex + 1);
    }

    return (retVal && noErrorOnSensor);
}

//////////////////////////////////////////////////////////
//               ManageErrors
//////////////////////////////////////////////////////////
bool COneWireEngine::ManageErrors( )
{
    t_Error error;
    CString errMessage;

    //Check if the interface is connected or if we don't have errors
    if ( (m_AfoErrorHandler.GetNofErrors() == 0) || (!CheckInterfacePortsForConnection()) )
    {
        //No connection or no errors, return
        return true;
    }


    //Start extracting errors
    UpdateServerPorts(true, false);
    while (CheckInterfacePortsForConnection() && m_AfoErrorHandler.PopError( &error ))
    {

        Cmd com("ERROR");
        com.putValue("CODE",error.errorType);
#ifndef SMALL_MEMORY_TARGET
        if ((error.errorType > 0) && (error.errorType < AFOERROR_NUM_TOT_ERRORS))
        {
            com.putValue("DESCRIPTION",afoErrorsStrings[error.errorType]);
        }
#endif
        com.putValue("NETORIGIN",error.netIndex);
        com.putValue("DEVICEORIGIN",error.deviceIndex);

        //Convert time into string
        int tempInt;
        CString errorTime = ctime((time_t*)(&tempInt));

        //trim the last /n char
        errorTime.erase(errorTime.end()-1);
        com.putValue("TIME",errorTime);

        WriteOnInterfacePorts( com.getXMLValue().c_str(), strlen(com.getXMLValue().c_str()) );
        UpdateServerPorts(true, false);
    }

    return true;

}

//////////////////////////////////////////////////////////
//               CheckForCommands
//////////////////////////////////////////////////////////
bool COneWireEngine::CheckForCommands( int netIndex )
{
    bool retVal = true;
    vector <CIC*>::iterator portIt;
    bool commandsPending = false;


    //06/04/2010
    return true;

    //TODO aggiungere gestione errori
    UpdateServerPorts(true, true);
    for (portIt = m_InterfaceVector.begin(); portIt!=m_InterfaceVector.end(); portIt++)
    {
        if ( (*portIt)->GetNofCommands() > 0)
        {
            commandsPending = true;
            break;
        }
    }

    UpdateServerPorts(false, true);
    for (portIt = m_InCommVector.begin(); portIt != m_InCommVector.end(); portIt++)
    {
        if ( (*portIt)->GetNofCommands() > 0)
        {
            commandsPending = true;
            break;
        }
    }

    if (commandsPending)
    {
        //We have some commands pending, close the net and execute them
//         m_Net->ReleaseNet( netIndex );
        GetCommands();
        m_Net->AcquireNet( netIndex );
    }
    ManageErrors();


    return retVal;
}

//////////////////////////////////////////////////////////
//               CheckForCommands2
//////////////////////////////////////////////////////////
//bool COneWireEngine::CheckForCommands2()
//{
//    bool retVal = true;
//    vector <CIC*>::iterator portIt;
//    bool commandsPending = false;
//
//
//    //TODO aggiungere gestione errori
//    for (portIt = m_InterfaceVector.begin(); portIt!=m_InterfaceVector.end(); portIt++)
//    {
//        (*portIt)->ReadCommands2();
//    }
//
//    for (portIt = m_InCommVector.begin(); portIt != m_InCommVector.end(); portIt++)
//    {
//        (*portIt)->ReadCommands2();
//    }
//
//    ManageErrors();
//
//
//    return retVal;
//}

bool COneWireEngine::CheckForCommands2()
{
    bool retVal = true;
    vector <CIC*>::iterator portIt;
    bool commandsPending = false;


    //TODO aggiungere gestione errori
    UpdateServerPorts(true, true);
    for (portIt = m_InterfaceVector.begin(); portIt!=m_InterfaceVector.end(); portIt++)
    {
        if ((*portIt)->IsConnected()){
            if ( (*portIt)->GetNofCommands() > 0)
            {
                commandsPending = true;
                break;
            }
        }
    }

    UpdateServerPorts(false, true);
    for (portIt = m_InCommVector.begin(); portIt != m_InCommVector.end(); portIt++)
    {
        if ((*portIt)->IsConnected()){
            if ( (*portIt)->GetNofCommands() > 0)
            {
                commandsPending = true;
                break;
            }
        }
    }

    if (commandsPending)
    {
        GetCommands();
    }

    ManageErrors();


    return retVal;
}

//////////////////////////////////////////////////////////
//               SendOutputMessage
//////////////////////////////////////////////////////////
bool COneWireEngine::WriteOnOutputPorts (CString message, int remoteAddr, int remotePort) {
    
    vector <t_CICData>::iterator portIt;
    
    if (m_DoDebug)
    {
        cout <<"Output verso remoto" << endl;cout.flush();
    }
    
    if (remoteAddr > -1)
    {
        for (portIt = m_OutCommVector.begin(); portIt != m_OutCommVector.end(); portIt++)
        {
            portIt->port->Update();
            if (!(portIt->port->IsConnected()) )
            {
                            //Try to reconnect
                if (!portIt->port->Startup())
                {
                                //TODO inserire messaggio di errore
                    return false;
                }
            }

            if ((portIt->minAddr <= remoteAddr) && (portIt->maxAddr >= remoteAddr))
            {
                portIt->port->OutputData( message.c_str(), message.size());
            }
        }
    }
    else if (remotePort > -1)
    {
        m_OutCommVector.at(remotePort).port->Update();
        if (!(m_OutCommVector.at(remotePort).port->IsConnected()) )
        {
            //Try to reconnect
            if (!m_OutCommVector.at(remotePort).port->Startup())
            {
                //TODO inserire messaggio di errore
                return false;
            }
        }

        m_OutCommVector.at(remotePort).port->OutputData( message.c_str(), message.size());
    }
    else
    {
        //Lo scrivo su tutte le porte
        for (portIt = m_OutCommVector.begin(); portIt != m_OutCommVector.end(); portIt++)
        {
            portIt->port->Update();
            if (!(portIt->port->IsConnected()) )
            {
                            //Try to reconnect
                if (!portIt->port->Startup())
                {
                    //TODO inserire messaggio di errore
                    return false;
                }
            }

            portIt->port->OutputData( message.c_str(), message.size());
        }
    }
    
    return true;
}

//////////////////////////////////////////////////////////
//               SendCommandToRemoteDO
//////////////////////////////////////////////////////////
bool COneWireEngine::SendCommandToRemoteDO( int maxNum, int * remoteDidoList )
{
    bool retVal = true;
    int didoIndex = 0, netNumber = 0, devIndex = 0;
    CString outMessage;
    int remoteADDR, remotePort, remoteNet;
    T_Net *net;
    

    //TODO mettere gestione errori

    for (didoIndex = 0; didoIndex < maxNum; didoIndex++)
    {
        netNumber = m_Net->GetNetByMemoryAddress( remoteDidoList[didoIndex] );
        devIndex = m_Net->GetDeviceIndexByMemoryAddress( netNumber, remoteDidoList[didoIndex] );

        if ( (netNumber > -1) && (devIndex > -1) )
        {
            if (m_Net->CheckControllerType( netNumber, devIndex, DEV_BUTTONCTRL))
            {
                CString command;

                net = m_Net->GetNetHandler( netNumber );
                remoteADDR = ((CVMultiDIDO*)(net->CtrlList[devIndex]))->GetRemoteAddress();
                remoteNet = ((CVMultiDIDO*)(net->CtrlList.at(devIndex)))->GetRemoteNET();
                remotePort = ((CVMultiDIDO*)(net->CtrlList.at(devIndex)))->GetRemotePort();
                outMessage = "<DEVICE ";
                if (((CButtonController*)(net->CtrlList.at(devIndex)))->m_RemoteChangeState)
                {
                    command = "COMMAND=\"ChangeDOState\" ";
                }
                else
                {
                    command = "COMMAND=\"SetDigitalOutput\" ";

                    if (((CButtonController*)(net->CtrlList.at(devIndex)))->m_JollyValue)
                    {
                        command+=" STATE=\"1\" ";
                    }
                    else
                    {
                        command+=" STATE=\"0\" ";
                    }

                }

                outMessage+=command;
                if (remoteADDR > -1)
                {
                    outMessage+="ADDRESS=\"";
                    outMessage+=remoteADDR;
                    outMessage+="\"";
                }
                else if (remoteNet > -1)
                {
                    outMessage += "NET=\"";
                    outMessage += remoteNet;
                    outMessage += "\" ";
                }


                outMessage+=" />";

                if (m_DoDebug)
                {
                    cout << "Messaggio in uscita:\n" << outMessage << endl;
                }

                WriteOnOutputPorts(outMessage,remoteADDR, remotePort);
            }
            else if (m_Net->CheckControllerType( netNumber, devIndex, DEV_TAGCTRL))
            {
                int inputState, outputState;

                net = m_Net->GetNetHandler( netNumber );

                inputState = ((CTAGControl*)(net->CtrlList[devIndex]))->GetInputState(false);
                outputState = ((CTAGControl*)(net->CtrlList[devIndex]))->GetOutputState(false);

                if (inputState != outputState)
                {

                    remoteADDR = ((CVMultiDIDO*)(net->CtrlList[devIndex]))->GetRemoteAddress();
                    outMessage = "<DEVICE COMMAND=\"SetDigitalOutput\" ADDRESS=\"";
                    outMessage+=remoteADDR;
                    outMessage+="\" VAL=\"";
                    outMessage+=inputState;
                    outMessage+="\" />";

                    if (m_DoDebug)
                    {
                        cout << "Messaggio in uscita:\n" << outMessage << endl;
                    }

                    WriteOnOutputPorts(outMessage,remoteADDR, remotePort);
                }



            }
            else if (m_Net->CheckControllerType( netNumber, devIndex, DEV_REMOTEDIDO))
            {
                CString command;

                net = m_Net->GetNetHandler( netNumber );
                remoteADDR = ((CVMultiDIDO*)(net->CtrlList[devIndex]))->GetRemoteAddress();
                remoteNet = ((CVMultiDIDO*)(net->CtrlList.at(devIndex)))->GetRemoteNET();
                remotePort = ((CVMultiDIDO*)(net->CtrlList.at(devIndex)))->GetRemotePort();

                if ( ((CRemoteDIDO*)(net->CtrlList[devIndex]))->m_RemoteChangeState)
                {
                    outMessage = "<DEVICE COMMAND=\"ChangeDOState\" ";
                }
                else {
                    outMessage = "<DEVICE COMMAND=\"SetDigitalOutput\" ";
                    outMessage += "STATE=\"";
                    if ( ((CRemoteDIDO*)(net->CtrlList[devIndex]))->m_JollyValue ) {
                        outMessage += "1";
                    }
                    else {
                        outMessage += "0";
                    }
                    outMessage += "\" ";
                }

                if (remoteADDR > -1)
                {
                    outMessage+="ADDRESS=\"";
                    outMessage+=remoteADDR;
                    outMessage+="\"";
                }
                else if (remoteNet > -1)
                {
                    outMessage += "NET=\"";
                    outMessage += remoteNet;
                    outMessage += "\" ";
                }


                outMessage+=" />";

                if (m_DoDebug)
                {
                    cout << "Messaggio in uscita:\n" << outMessage << endl;
                }

                WriteOnOutputPorts(outMessage,remoteADDR, remotePort);
            }


        }
    }

    return retVal;
}

//////////////////////////////////////////////////////////
//               UpdateCommPorts
//////////////////////////////////////////////////////////
void COneWireEngine::UpdateServerPorts(bool updateInterface, bool parseData)
{
    vector<CIC*>::iterator portIt;

    if (updateInterface)
    {
        for (portIt = m_InterfaceVector.begin(); portIt != m_InterfaceVector.end(); portIt++)
        {
            (*portIt)->Update();

            if (parseData)
            {
                (*portIt)->ParseData();
            }
        }
    }
    else
    {
        for (portIt = m_InCommVector.begin(); portIt != m_InCommVector.end(); portIt++)
        {
            (*portIt)->Update();
            if (parseData)
            {
                (*portIt)->ParseData();
            }
        }
    }

}

//////////////////////////////////////////////////////////
//               WriteOnInterfacePorts
//////////////////////////////////////////////////////////
bool COneWireEngine::WriteOnInterfacePorts( const char * buffer, int bufferSize )
{
    bool retVal = true;
    vector<CIC*>::iterator portIt;

    pthread_mutex_lock(&mutex1);
    for (portIt = m_InterfaceVector.begin(); portIt != m_InterfaceVector.end(); portIt++)
    {
        (*portIt)->Update();
        (*portIt)->OutputData(buffer, bufferSize);
    }
    pthread_mutex_unlock(&mutex1);
    return retVal;
}

//////////////////////////////////////////////////////////
//               CheckInterfacePortsForConnection
//////////////////////////////////////////////////////////
bool COneWireEngine::CheckInterfacePortsForConnection( )
{
    bool retVal = false;
    vector <CIC*>::iterator portIt;

    //Check that at least one interface is connected
    for (portIt = m_InterfaceVector.begin(); portIt != m_InterfaceVector.end(); portIt++)
    {
        //Check if the only connected port is the watchdog port, in this case do not report it as a valid connected port
        if ( ((*portIt)->IsConnected()) && ((*portIt)->GetPortAddress() != "127.0.0.1") )
        {
            retVal = true;
            break;
        }
    }

    return retVal;
}

//////////////////////////////////////////////////////////
//               ManageCoordinators
//////////////////////////////////////////////////////////
bool COneWireEngine::ManageCoordinators( int netIndex )
{
    bool retVal = false;
    vector<CVController*>::iterator ctrlIt;
    T_Net *net;
    CString outMessage, addressStr;


    net = m_Net->GetNetHandler( netIndex );

    if (net != 0x0)
    {
        retVal = true;

        for (ctrlIt = net->CtrlList.begin(); ctrlIt != net->CtrlList.end(); ctrlIt++)
        {
            CheckForCommands2();
            
            addressStr="";
            addressStr+=(*ctrlIt)->GetMemoryAddress();

            if((*ctrlIt)->GetControllerType() == DEV_IBUTT_RDR)
            {
                (*ctrlIt)->Update(true);
                
                if (((CIButtonReader*)(*ctrlIt))->m_SerialNumberVector.size())
                {
                    //Li sparo tutti ??
                    for (int i = 0; i < ((CIButtonReader*)(*ctrlIt))->m_SerialNumberVector.size(); i++)
                    {
                        CString outMessage;
                        outMessage = "<DEVICE TYPE=\"ButtonRomCode\" SERNUM=\"";
    
                        outMessage+=((CIButtonReader*)(*ctrlIt))->m_SerialNumberVector.at(i);
    
                        outMessage+="\" />";
    
                        UpdateServerPorts( true, false);
                        WriteOnInterfacePorts( outMessage.c_str(), outMessage.size() );
                    }

                }
                else
                {
                    outMessage = "";
                    outMessage = "<DEVICE TYPE=\"ButtonRomCode\" SERNUM=\"";
                    outMessage+="Chiave NON presente";
                    outMessage+="\" />";

                    UpdateServerPorts( true, false);
                    WriteOnInterfacePorts( outMessage.c_str(), outMessage.size() );
                }
            }
            else if ((*ctrlIt)->GetControllerType() == DEV_FULLUTACTRL_2)
            {

                (*ctrlIt)->Update(true);

            }//FULL_UTACTRL
            else if ((*ctrlIt)->GetControllerType() == DEV_FULLUTACTRL)
            {
                int pidIndex = 0;

                (*ctrlIt)->Update(true);

                //Devo raccogliere le uscite dei PID e mandarle all'eventuale interfaccia
                for (pidIndex = UTA_HEATBAT; pidIndex <= UTA_FREECOOLINGPID; pidIndex++)
                {
                    if (((CFullUTACtrl*)(*ctrlIt))->m_ControllerList[pidIndex] != 0x0)
                    {
                        Cmd com("DEVICE");

                        com.putValue("TYPE","PIDOutput");
                        com.putValue("ADDRESS",((CFullUTACtrl*)(*ctrlIt))->m_ControllerList[pidIndex]->GetMemoryAddress());
                        com.putValue("VAL",((CPIDSimple*)(((CFullUTACtrl*)(*ctrlIt))->m_ControllerList[pidIndex]))->GetLastOutputInVolt());
                        com.putValue("SETPOINT",((CPIDSimple*)(((CFullUTACtrl*)(*ctrlIt))->m_ControllerList[pidIndex]))->GetSetPoint());
                        com.putValue("SUMMER", ((CPIDSimple*)(((CFullUTACtrl*)(*ctrlIt))->m_ControllerList[pidIndex]))->GetSummer());

                        CString message = com.getXMLValue();
                        WriteOnInterfacePorts(message.c_str(), message.size());
                        
                        
                        

//                         pidAddress = ((CFullUTACtrl*)(*ctrlIt))->m_ControllerList[pidIndex]->GetMemoryAddress();
//                         pidOutput =  ((CPIDSimple*)(((CFullUTACtrl*)(*ctrlIt))->m_ControllerList[pidIndex]))->m_PIDOutput;
                        //
//                         pidOutputArray[nOfPids][0] = pidAddress;
//                         pidOutputArray[nOfPids][1] = pidOutput;
//                         nOfPids++;
                    }
                }

                //OutputFloatDataByAddress(nOfPids,pidOutputArray);

                //Messaggio specifico del FullUTA ctrl
                outMessage = "<DEVICE TYPE=\"FullUTACtrl\" ADDRESS=\"";
                outMessage += (*ctrlIt)->GetMemoryAddress();
                outMessage +="\" OUTPUT=\"";
                outMessage += ((CFullUTACtrl*)(*ctrlIt))->m_MainPIDOutputVolt;
                outMessage +="\" TEMPMND=\"";
                outMessage +=((CFullUTACtrl*)(*ctrlIt))->m_TempMnd;
                outMessage += "\" TEMPRIP=\"";
                outMessage +=((CFullUTACtrl*)(*ctrlIt))->m_TempRip;
                outMessage += "\" SETPOINT=\"";
                outMessage +=((CFullUTACtrl*)(*ctrlIt))->m_TempSetPoint;
                outMessage +="\" SUMMER=\"";
                outMessage +=((CFullUTACtrl*)(*ctrlIt))->m_IsSummer;
                outMessage +="\" HUM=\"";
                outMessage +=((CFullUTACtrl*)(*ctrlIt))->m_Humidity;
                outMessage +="\" />";

                WriteOnInterfacePorts(outMessage.c_str(), outMessage.size());

            }//FULL_UTACTRL
            else if((*ctrlIt)->GetControllerType() == DEV_PUMPCTRL)
            {
                (*ctrlIt)->Update(true);
            }
            else if ((*ctrlIt)->GetControllerType() == DEV_ALARMCTRL)
            {
                vector<t_Zone>::iterator zoneIt;

                (*ctrlIt)->Update(true);

                if (m_DoDebug)
                {
                    cout <<"Gestione Allarmi -- ADDR: "<<(*ctrlIt)->GetMemoryAddress();
                }

                for (zoneIt = ( (AlarmCoordinator*)(*ctrlIt) )->m_ZoneVector.begin();
                     zoneIt < ( (AlarmCoordinator*)(*ctrlIt) )->m_ZoneVector.end();
                     zoneIt++
                    )
                {
                    outMessage="<DEVICE TYPE=\"AlarmState\" ADDR=\"";
                    outMessage+=(*ctrlIt)->GetMemoryAddress();
                    outMessage+="\" ZONE=\"";
                    outMessage+=zoneIt->zoneNumber+1;
                    outMessage+="\" STATE=\"";
                    outMessage+=zoneIt->isAlarmActive;
                    outMessage+="\" />";

                    WriteOnInterfacePorts(outMessage.c_str(), outMessage.size());

                    if (m_DoDebug)
                    {
                        cout <<" ZONA: "<<zoneIt->zoneNumber+1<<" STATO: "<<zoneIt->isAlarmActive;
                    }
                }
                
                cout<<endl;
            }
            else if ((*ctrlIt)->GetControllerType() == DEV_CHOVER)
            {
                (*ctrlIt)->Update(true);

                outMessage = "<DEVICE TYPE=\"PlantState\" ADDRESS=\"";
                outMessage+=addressStr;
                outMessage+="\" SUMMER=\"";
                outMessage+=((ChangeOverCoord*)(*ctrlIt))->m_IsSummer;
                outMessage+="\" ON=\"";
                outMessage+=((ChangeOverCoord*)(*ctrlIt))->m_IsChangeOverStarted;
                outMessage+="\" />";
                WriteOnInterfacePorts(outMessage.c_str(), outMessage.size());

            }
            else if ((*ctrlIt)->GetControllerType() == DEV_CLIMATICOORD)
            {
                (*ctrlIt)->Update(true);
            }
            else if ((*ctrlIt)->GetControllerType() == DEV_FLOORCOORD_2)
            {
                vector<t_FloorZone>::const_iterator zoneIt;
                (*ctrlIt)->Update(false);

                //Mando via un messaggio per zona
                for (zoneIt = ((CFloorCoord2*)(*ctrlIt))->GetZoneVectorRef().begin(); zoneIt < ((CFloorCoord2*)(*ctrlIt))->GetZoneVectorRef().end(); zoneIt++)
                {
                    Cmd message("DEVICE");
                    message.putValue("TYPE", "FloorCoord");
                    message.putValue("ADDRESS", (*ctrlIt)->GetMemoryAddress());
                    message.putValue("ZONE", zoneIt->zNumber);
                    message.putValue("TEMP", zoneIt->CurrentTemp);
                    message.putValue("SETPOINT", zoneIt->CurrentSetPoint);
                    message.putValue("HUM",zoneIt->CurrentHum);
                    message.putValue("SUMMER", ((CFloorCoord2*)(*ctrlIt))->GetSummer());
                    message.putValue("STATE", zoneIt->IsTempOn);
                    message.putValue("ON", zoneIt->OnOff.isOn);

                    outMessage = message.getXMLValue();
                    WriteOnInterfacePorts(outMessage.c_str(), outMessage.size());
                }
            }
             else if ((*ctrlIt)->GetControllerType() == DEV_BLOCKCOORD)
            {
                 (*ctrlIt)->Update(false);

                 //Non credo ci siano messaggi da mandare poiche' i risultati si vedono sulle uscite... ma e' da vedere.
                 //Forse, tipo labview, si potrebbe mandare un messaggio per blocco con il suo stato cosi' posso
                 //avere la mappa in realtime!! figata!!
             }


        } // FOR
    }

    return retVal;
}

//////////////////////////////////////////////////////////
//               ManageAdvancedController
//////////////////////////////////////////////////////////
bool COneWireEngine::ManageAdvancedController( int netIndex )
{
    bool retVal = false;
    vector<CVController*>::iterator ctrlIt;
    T_Net *net;
    CString outMessage, addressStr;
    uchar iButtonSN[8];
    bool forceCommandCheck = false;
    bool forceIButtonSearch = false;

    net = m_Net->GetNetHandler( netIndex );

    if (net != 0x0)
    {
        retVal = true;

        //TODO potrei ciclare per vedere se ci sono dei controller di2ao e, nel caso, convertire tutti i dido in una volta, da valutare

        for (ctrlIt = net->CtrlList.begin(); ctrlIt < net->CtrlList.end(); ctrlIt++)
        {
            forceCommandCheck = false;

            addressStr="";
            addressStr+=((*ctrlIt)->GetMemoryAddress());

            if ((*ctrlIt)->GetControllerType() == DEV_VLVCTRL)
            {
                float tempInt=0.0, tempExt=0.0, setpoint=0.0;
                bool isSummer=false;

                forceCommandCheck = true;
                forceIButtonSearch = true;
                (*ctrlIt)->Update(true);

                //Check if we are in debug mode or there is at least one interface open, otherwise we don't need to update the data
                UpdateServerPorts( true, false);
                if (CheckInterfacePortsForConnection() || (m_DoDebug))
                {
#ifndef USE_ADV_VLV
                    ((CNTHVLV*)(*ctrlIt))->GetBasicdata(&tempInt, &tempExt, &setpoint, &isSummer);
#else
                    ((CNTHVLV_ADV*)(*ctrlIt))->GetBasicdata(&tempInt, &tempExt, &setpoint, &isSummer);
#endif
                    if (m_DoDebug)
                    {
                        cout << "Modulo indirizzo: " << (*ctrlIt)->GetMemoryAddress() << " Impostazioni temperature: " << tempInt << ", " << tempExt << ", " << setpoint << endl;
                    }

                    Cmd com("DEVICE");

                    com.putValue("TYPE","VOWCtrlSettings");
                    com.putValue("ADDRESS",addressStr);
                    com.putValue("SETPOINT",setpoint);
                    com.putValue("TEMPINT",tempInt);
                    com.putValue("TEMPEXT",tempExt);
                    com.putValue("SUMMER",isSummer);
#ifndef USE_ADV_VLV
                    com.putValue("DIGOUT",((CNTHVLV*)(*ctrlIt))->getIsOutputOn());
#endif


                    outMessage = com.getXMLValue();

                    WriteOnInterfacePorts( outMessage.c_str(), outMessage.size() );
                }
            }//VLV_CTRL
            else if ((*ctrlIt)->GetControllerType() == DEV_DI2AO)
            {
                retVal = ((CDI2AO*)(*ctrlIt))->Update(true);
                forceCommandCheck = true;
                forceIButtonSearch = true;

                if (!retVal)
                {
                    //TODO aggiungere messaggi

                    if (m_DoDebug)
                    {
                        cout << "NET"<<netIndex+1<<" Errore nell'aggiornamento del DI2AO indice : " << ((*ctrlIt))->GetConfigFileDevIndex()<<endl;
                    }
                }
            }//NTH_MGC
            else if ((*ctrlIt)->GetControllerType() == DEV_MGC){

                forceCommandCheck = true;
                forceIButtonSearch = true;

                (*ctrlIt)->Update(true);

                //IN automatico viene mandato il messaggio ROOMSETTINGS
                outMessage = "<DEVICE TYPE=\"RoomSettings\" ADDRESS=\"";

                outMessage+=addressStr;

                outMessage+="\" ROOMSETPOINT=\"";
                outMessage+=((CNTHMGC*)(*ctrlIt))->m_RoomSetPoint;

                outMessage+="\" ROOMTEMP=\"";
                outMessage+=((CNTHMGC*)(*ctrlIt))->m_RoomTemp;

                outMessage+="\" NAME=\"";
                outMessage+=((CNTHMGC*)(*ctrlIt))->m_GuestName;

                outMessage+="\" SUMMER=\"";
                if (((CNTHMGC*)(*ctrlIt))->m_PIDIsSummer)
                {
                    outMessage+="1";
                }
                else
                {
                    outMessage+="0";
                }

                outMessage+="\" VACANCY=\"";
                outMessage+=((CNTHMGC*)(*ctrlIt))->m_PresenceType;

                outMessage+="\" />";

                if (m_DoDebug){
                    cout <<outMessage<<endl;
                }

                UpdateServerPorts( true, false);
                WriteOnInterfacePorts( outMessage.c_str(), outMessage.size() );

            }
            else if ((*ctrlIt)->GetControllerType() == DEV_CNT)
            {
                forceCommandCheck = true;
                forceIButtonSearch = true;

                (*ctrlIt)->Update(true);

                UpdateServerPorts( true, false);
                
                if (CheckInterfacePortsForConnection())
                {
                    for ( int i=0 ; i<4 ; i++ )
                    {
                        Cmd com("DEVICE");
                        com.putValue("TYPE","CNTState");
                        com.putValue("ADDRESS",addressStr);
                        
                        switch ( ((CNTH_CNT*)(*ctrlIt))->m_Counters[i].counterType )
                        {
                            case CNT_CAL: {
                                    com.putValue("INDEX",i);
                                    com.putValue("CNT","Calorie");
                                    com.putValue("CAL",((CNTH_CNT*)(*ctrlIt))->m_CounterValue[i].val1);
                                    com.putValue("FRG",((CNTH_CNT*)(*ctrlIt))->m_CounterValue[i].val2);
                                    com.putValue("TMND",((CNTH_CNT*)(*ctrlIt))->m_Counters[i].tMnd);
                                    com.putValue("TRIP",((CNTH_CNT*)(*ctrlIt))->m_Counters[i].tRip);
                                }
                                break;

                            case CNT_WATER:{
                                    com.putValue("INDEX",i);
                                    com.putValue("CNT","Litri");
                                    com.putValue("LTR",((CNTH_CNT*)(*ctrlIt))->m_CounterValue[i].val1);
                                }
                                break;

                            case CNT_WATT:{
                                    com.putValue("INDEX",i);
                                    com.putValue("CNT","Watt");
                                    com.putValue("WTT",((CNTH_CNT*)(*ctrlIt))->m_CounterValue[i].val1);
                                }
                                break;
                        }

                        outMessage = com.getXMLValue();
                        WriteOnInterfacePorts( outMessage.c_str(), outMessage.size() );
                    }
                }
            }
            else if ((*ctrlIt)->GetControllerType() == DEV_ACC)
            {
                forceCommandCheck = true;
                forceIButtonSearch = true;

                //Cerco se c'è un iButton nel lettore di net
                if (net->hasIButtonReader)
                {
                    if (SearchIButton(iButtonSN))
                    {
                        char snString[64];

                        outMessage = "";
                        outMessage = "<DEVICE TYPE=\"ButtonRomCode\" SERNUM=\"";

                        ConvertSN2Str(snString, iButtonSN);
                        outMessage+=snString;

                        outMessage+="\" />";

                        UpdateServerPorts( true, false);
                        WriteOnInterfacePorts( outMessage.c_str(), outMessage.size() );

                    }
                    else
                    {
                        outMessage = "";
                        outMessage = "<DEVICE TYPE=\"ButtonRomCode\" SERNUM=\"";
                        outMessage+="Chiave NON presente";
                        outMessage+="\" />";

                        UpdateServerPorts( true, false);
                        WriteOnInterfacePorts( outMessage.c_str(), outMessage.size() );
                    }
                }
            }
            else if ((*ctrlIt)->GetControllerType() == DEV_AFOVLV)
            {
                forceCommandCheck = true;
                forceIButtonSearch = true;

                (*ctrlIt)->Update(true);

                UpdateServerPorts( true, false);
                if (CheckInterfacePortsForConnection())
                {
                    outMessage = ((CNTHVLV2*)(*ctrlIt))->GetSpontaneousData(0);
                    WriteOnInterfacePorts( outMessage.c_str(), outMessage.size() );
                }
            }//VLV_CTRL
            else if ((*ctrlIt)->GetControllerType() == DEV_AFOVLV_VAV)
            {
                float tempInt, tempExt, setpoint1, setpoint2;
                bool summer;

                forceCommandCheck = true;
                forceIButtonSearch = true;

                (*ctrlIt)->Update(true);

                tempInt = ((CNTHVLV2_VAV*)(*ctrlIt))->m_Temp1;
                tempExt = ((CNTHVLV2_VAV*)(*ctrlIt))->m_Temp2;
                setpoint1 = ((CNTHVLV2_VAV*)(*ctrlIt))->m_SetPoint1;
                setpoint2 = ((CNTHVLV2_VAV*)(*ctrlIt))->m_SetPoint2;
                summer = ((CNTHVLV2_VAV*)(*ctrlIt))->m_PIDIsSummer;

                if (m_DoDebug)
                {
                    cout << "Modulo indirizzo: " << (*ctrlIt)->GetMemoryAddress() << " Impostazioni temperature: Temp1:" << tempInt
                            << ", TEMP2:" << tempExt << ", SP1:" << setpoint1<< ", SP2:" << setpoint2 << ", SUMMER:"<<(int)summer<<endl;
                }
                
                UpdateServerPorts( true, false);
                if (CheckInterfacePortsForConnection())
                {
                    Cmd com("DEVICE");

                    com.putValue("TYPE","VLV2State");
                    com.putValue("ADDRESS",addressStr);
                    com.putValue("SETPOINT",((CNTHVLV2_VAV*)(*ctrlIt))->m_SetPoint1);
                    com.putValue("TEMP1",((CNTHVLV2_VAV*)(*ctrlIt))->m_Temp1);
                    com.putValue("TEMP2",((CNTHVLV2_VAV*)(*ctrlIt))->m_Temp2);
                    com.putValue("SETPOINT2", ((CNTHVLV2_VAV*)(*ctrlIt))->m_SetPoint2);
                    com.putValue("SUMMER",((CNTHVLV2_VAV*)(*ctrlIt))->m_PIDIsSummer);
                    com.putValue("ANALOGOUT1", ((CNTHVLV2_VAV*)(*ctrlIt))->m_AnalogOut1);
                    com.putValue("ANALOGOUT2", ((CNTHVLV2_VAV*)(*ctrlIt))->m_AnalogOut2);
                    com.putValue("ANALOGIN1", ((CNTHVLV2_VAV*)(*ctrlIt))->m_AnalogIn1);
                    com.putValue("ANALOGIN2", ((CNTHVLV2_VAV*)(*ctrlIt))->m_AnalogIn2);
                    com.putValue("DIGOUT",((CNTHVLV2_VAV*)(*ctrlIt))->m_IsPID1On);


                    outMessage = com.getXMLValue();

                    WriteOnInterfacePorts( outMessage.c_str(), outMessage.size() );
                }
            }//AFOVLV_VAV
            else if ((*ctrlIt)->GetControllerType() == DEV_TIMEMARKER)
            {
                (*ctrlIt)->Update(true);
                forceCommandCheck = true;
                forceIButtonSearch = true;
            }




            if (forceCommandCheck)
            {
                CheckForCommands2();
            }

            //Cerco se c'è un iButton nel lettore di net
            if ((net->hasIButtonReader) && forceIButtonSearch)
            {
                forceIButtonSearch = false;
                if (SearchIButton(iButtonSN))
                {
                    char snString[64];

                    outMessage = "";
                    outMessage = "<DEVICE TYPE=\"ButtonRomCode\" SERNUM=\"";

                    ConvertSN2Str(snString, iButtonSN);
                    outMessage+=snString;

                    outMessage+="\" />";

                    UpdateServerPorts( true, false);
                    WriteOnInterfacePorts( outMessage.c_str(), outMessage.size() );

                }
                else
                {
                    outMessage = "";
                    outMessage = "<DEVICE TYPE=\"ButtonRomCode\" SERNUM=\"";
                    outMessage+="Chiave NON presente";
                    outMessage+="\" />";

                    UpdateServerPorts( true, false);
                    WriteOnInterfacePorts( outMessage.c_str(), outMessage.size() );
                }
            }

        } // FOR
    }

    return retVal;
}


/*!
    \fn COneWireEngine::ManageActivityDIDO(int netIndex)
 */
bool COneWireEngine::ManageActivityDIDO()
{
    bool retVal = false;
    int nOfActivityDIDO = 0;
    int activityDIDOList[MAX_NUM_DEV_PER_NET];
    int didoIndex = -1, devIndex = -1;
    T_Net *net = 0x0;
    int configDevIndex = -1;
    vector<CVDevice*> ds2408ToClear;
    ////////////
    int netIndex = -1;
    int masterPortIndex = 0;
    int nOfPorts;

    memset (activityDIDOList, 0x0, MAX_NUM_DEV_PER_NET*sizeof(int));
    nOfPorts = m_Net->GetMasterHandler()->GetNofPortsAcquired();

    //TBM
//     cout << "Numero porte acquisite: " << nOfPorts<<endl;

    //TODO Da rimettere a posto
    for (masterPortIndex = 0; masterPortIndex < nOfPorts; masterPortIndex++)
    {
       nOfActivityDIDO = m_Net->SearchDIDOActivity(masterPortIndex, activityDIDOList, MAX_NUM_DEV_PER_NET, ds2408ToClear );

        if (nOfActivityDIDO > 0)
        {
            retVal = true;

            for (didoIndex = 0; didoIndex < nOfActivityDIDO; didoIndex++)
            {
                netIndex = m_Net->GetNetByMemoryAddress( activityDIDOList[didoIndex] );
                devIndex = m_Net->GetDeviceIndexByMemoryAddress( netIndex, activityDIDOList[didoIndex] );

                if ( (netIndex < 0) || (devIndex < 0))
                {
                    //C'è un qualche errore, passa alla porta successiva
                    //TODO Sistemare la gestione dell'errore

                    continue;
                }

                net = m_Net->GetNetHandler( netIndex );

                if (m_Net->CheckControllerType( netIndex, devIndex, DEV_REMOTEDIDO))
                {
                    retVal = SendCommandToRemoteDO( 1, &activityDIDOList[didoIndex]);
                }
                else if (m_Net->CheckControllerType( netIndex, devIndex, DEV_BUTTONCTRL))
                {
                    if (m_DoDebug == 3)
                    {
                        cout << "Attivita' ButtonController rilevata sul dispositivo di indirizzo: " << net->CtrlList[devIndex]->GetMemoryAddress() << endl;
                    }

                    if (((CButtonController*)(net->CtrlList.at(devIndex)))->m_IsJolly)
                    {
                        //Jolly device, apply changes to all devices belonging to the same NET
                        retVal = m_Net->SetAllDigitalOutputs( netIndex, ((CButtonController*)(net->CtrlList.at(devIndex)))->m_JollyValue);
                    }
                    else
                    {
                        retVal = ((CButtonController*)(net->CtrlList[devIndex]))->ChangeOutput();
                        //Update the INI file
                        if ((retVal) && (net->saveDigitalState))
                        {
                            configDevIndex = ((CDigitalIO*)(net->CtrlList[devIndex]))->GetConfigFileDevIndex();
                            if (((CButtonController*)(net->CtrlList[devIndex]))->GetOutputState(false))
                            {
                                m_Net->UpdateIniFile(netIndex, configDevIndex, "STARTV", "1");
                            }
                            else
                            {
                                m_Net->UpdateIniFile( netIndex, configDevIndex, "STARTV", "0");
                            }
                        }
                    }

                    ManageSwitches(netIndex);
                }
                else if (m_Net->CheckControllerType( netIndex, devIndex, DEV_STEPDIGITALOUT))
                {
                    if (m_DoDebug == 3)
                    {
                        cout << "Attivita' STEP rilevata sul dispositivo di indirizzo: " << net->CtrlList[devIndex]->GetMemoryAddress() << endl;
                    }

                    retVal = ((CStepDigitalOut*)(net->CtrlList[devIndex]))->ChangeOutput();

                    //ManageSwitches(netIndex);
                }
            } //for didoIndex
        }

    } //For


    return retVal;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                  SearchIButton
///////////////////////////////////////////////////////////////////////////////////////////////////////////
bool COneWireEngine::SearchIButton(uchar * snBuffer)
{
    bool retVal = false;
    uchar iButtonSN[MAX_NUM_DEV_PER_NET][8];
    int numDevFound = 0;
    T_Net *net;
    int netIndex;

    for (netIndex = 0; netIndex < m_TotalNets; netIndex++)
    {
        net = m_Net->GetNetHandler(netIndex);

        if ( (net != 0x0) && (net->isAcquired) && (net->hasIButtonReader))
        {
            if (m_DoDebug)
            {
                cout<<"*************SearchIButton on NET "<< netIndex+1<<"***************"<<endl;
            }

            numDevFound = net->master->FindDevices(net->portHandler, &iButtonSN[0], DS1990A_FN, 1);

            if ((numDevFound > 0) && (iButtonSN[0][0]==0x01))
            {
                if (m_DoDebug)
                {
                    cout << "Chiave trovata!!"<<endl;
                }
                memcpy (snBuffer, &iButtonSN[0], 8*sizeof(uchar));
                retVal = true;
            }
        }
    }

    return retVal;
}


