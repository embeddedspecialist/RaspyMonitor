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
#include "alarmcoordinator.h"

//Includo qui i riferimenti alla engine e alla net
#include "conewireengine.h"
#include "conewirenet.h"


AlarmCoordinator::AlarmCoordinator ( const char *configString, CTimer *timer )
        : CVCoordinator ( configString )
{
    if ( configString != 0x0 )
    {
    }

    m_NetPtr = 0x0;

    m_ErrorNumber = 0;

    if ( ( m_TimerID > 0 ) && ( timer != 0x0 ) )
    {
        m_Timer = timer;
        m_UseTimer = true;
    }
    else
    {
        m_UseTimer = false;
    }

    m_TypeOfTimerVal = TIMERVAL_DIGITAL;
    m_ControllerType = DEV_ALARMCTRL;
}


AlarmCoordinator::~AlarmCoordinator()
{
}
////////////////////////////////////////////////////////////////////////////////////////
bool AlarmCoordinator::Update ( bool updateData )
{
    bool retVal = false;
    unsigned int zoneIndex = 0, accIndex=0;
    uchar mapmem[24];
    COneWireNet* netPtr = reinterpret_cast<COneWireNet*> ( m_NetPtr );
    vector<CIButtonReader*>::iterator iBtnIt;


    if ( netPtr == 0x0 )
    {
        //TODO errore!!!
        return false;
    }

    if (m_DebugLevel)
    {
        cout <<"Gestore Allarmi -- Address: "<<m_Address<<" Aggiornamento..."<<endl;
    }
    //Ciclo che legge e decodifica i campi di una scheda di controllo accessi alla volta, qui in mezzo devo anche controllare gli ingressi digitali
    for ( zoneIndex = 0; zoneIndex < m_ZoneVector.size(); zoneIndex++ )
    {
        for ( accIndex = 0; accIndex < m_ZoneVector[zoneIndex].accCtrlList.size(); accIndex++ )
        {
            memset ( mapmem,0xFF,24 );
            mapmem[0] = 0x1;
            mapmem[15] = m_ZoneVector[zoneIndex].accCtrlList[accIndex].accController->CalcCRC( mapmem );

            if (( m_ZoneVector[zoneIndex].accCtrlList[accIndex].accController->WriteToDevice ( mapmem ) ) &&
                  (m_ZoneVector[zoneIndex].accCtrlList[accIndex].accController->CheckCRC(mapmem))
               )
            {
                int channel = m_ZoneVector[zoneIndex].accCtrlList[accIndex].channel;

                //Decodifico se c'è una chiave valida nel canale che mi interessa
                if ( mapmem[10] & ( channel ) )
                {
                    m_ZoneVector[zoneIndex].isAlarmActive = !m_ZoneVector[zoneIndex].isAlarmActive;

                    ChangeZoneAlarmState ( zoneIndex );
                }
            }
            else
            {
                //TODO inserire errore "ufficiale"
                //ERRORE!!! Che faccio ?
                if ( m_DebugLevel )
                {
                    cout << "ERRORE!!! IMPOSSIBILE RICEVERE DATI DAL CONTROLLO ACCESSI DI INDIRIZZO: "<<
                    m_ZoneVector[zoneIndex].accCtrlList[accIndex].accController->GetMemoryAddress() << " Errore numero: " << endl;
                    m_ErrorNumber++;
                    //VerifyErrors()
                }
            }

            //Per ogni dispositivo faccio una ricerca di attivita'
            SearchForActivity();
            
            //Controllo se ho un ingresso di attivazione
            CheckInputActivation(zoneIndex);
        }
        
        //Ricilo sui lettori iButton
        for (iBtnIt = m_ZoneVector[zoneIndex].ibtnrdrList.begin(); iBtnIt < m_ZoneVector[zoneIndex].ibtnrdrList.end(); iBtnIt++)
        {
            //Leggo se ci sono delle chiavi nel/nei lettori
            int nOfIButtons = (*iBtnIt)->m_SerialNumberVector.size();
            
            for (int i = 0; i < nOfIButtons; i++)
            {
                if (CheckKeyInZone((*iBtnIt)->m_SerialNumberVector.at(i), zoneIndex))
                {
                    //La chiave esiste
                    m_ZoneVector[zoneIndex].isAlarmActive = !m_ZoneVector[zoneIndex].isAlarmActive;

                    ChangeZoneAlarmState ( zoneIndex );
                }
                else
                {
                    //La chiave non è riconosciuta
                    //TODO
                }
                   
            }
            
            if (nOfIButtons)
            {
                //Mi fermo un attimo per non leggere due volte la chiave
                sleep(5);
            }
            
            //Per ogni dispositivo faccio una ricerca di attivita'
            SearchForActivity();
            
             //Controllo se ho un ingresso di attivazione
            CheckInputActivation(zoneIndex);
        }
        
        //Se non ho iButton e controlli accessi non ho ancora controllato cosa succede...
        if ( (m_ZoneVector[zoneIndex].accCtrlList.size() == 0) && (m_ZoneVector[zoneIndex].ibtnrdrList.size() == 0))
        {
            //Per ogni dispositivo faccio una ricerca di attivita'
            SearchForActivity();
            
             //Controllo se ho un ingresso di attivazione
            CheckInputActivation(zoneIndex);
        }
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////////////////////
bool AlarmCoordinator::ConnectControllers()
{
    CIniFileHandler iniFileReader;
    COneWireNet* netPtr = reinterpret_cast<COneWireNet*> ( m_NetPtr );
    int netIndex, ctrlIndex, zoneIndex, itemIndex;
    int nOfZones = 0, nOfItems = 0;
    t_AccController newAccCtrl;
    t_AccessData newAccessData;
    t_Zone newZone;
    CString sectionName, deviceName;
    int devAddr, accChannel;
    CString configString;


    //Esco subito perchè non ho il puntatore alla NET
    if ( netPtr == 0x0 )
    {
        cout << "Attenzione il coordinatore di indirizzo: " << m_Address << " NON è connesso al conewireNET"<<endl;
        return false;
    }

    //Leggo dal file di configurazione (alarmControl.ini) tutte le chiavi e le zone e le istanzio, programmo i moduli di controllo accessi
    if ( !iniFileReader.Load ( "./alarmControl.ini" ) )
    {
        cout << "Attenzione: impossibile aprire il file alarm.ini"<<endl;
        return false;
    }

    //Carico Numero zone
    nOfZones = iniFileReader.GetInt ( "nOfZones", "COMMON" ,0 );

    //For tutte le zone
    for ( zoneIndex = 0; zoneIndex < nOfZones; zoneIndex++ )
    {
        ResetZone(&newZone);
        newZone.zoneNumber = zoneIndex;

        sectionName = "ZONE";

        sectionName+=zoneIndex+1;
        
        //Controllo subito se ho un controllo remoto
        newZone.remoteAddr = iniFileReader.GetInt ( "RemoteNotify", sectionName, -1 );

        //mi collego ai dispositivi della zona
        nOfItems = iniFileReader.GetInt ( "nOfAccDev", sectionName, 0 );

        if ( nOfItems == 0 )
        {
            cout << "Attenzione: la zona numero " << zoneIndex+1 << "NON contiene dispositivi di accesso"<<endl;
            sleep ( 5 );
//             continue;
        }

        for ( itemIndex = 0; itemIndex < nOfItems; itemIndex++ )
        {
            deviceName = "Device";
            if ( (itemIndex+1)<10 )
            {
                deviceName+=0;
            }

            deviceName+=itemIndex+1;

            configString = iniFileReader.GetString ( deviceName, sectionName, "" );

            if ( configString.size() == 0 )
            {
                cout << "ATTENZIONE: manca il " << deviceName << ", zona "<<sectionName<<" nel file alarmControl.ini\nContinuo..."<<endl;
                sleep ( 5 );
                continue;
            }

            m_IniLib.GetConfigParamInt ( configString.c_str(), "ADDR", &devAddr, -1 );

            if ( devAddr < 0 )
            {
                cout << "ATTENZIONE: manca ADDR nel " << deviceName << ", zona "<<sectionName<<" nel file alarmControl.ini\nContinuo..."<<endl;
                sleep ( 5 );
                continue;
            }

            m_IniLib.GetConfigParamInt ( configString.c_str(), "CHANNEL", &accChannel, 2 );

            //Recupero l'handler del dispositivo
            netIndex = netPtr->GetNetByMemoryAddress ( devAddr );
            ctrlIndex = netPtr->GetDeviceIndexByMemoryAddress ( netIndex, devAddr );

            if ( ( netIndex < 0 ) || ( ctrlIndex < 0 ))
            {
                cout << "ATTENZIONE: l'indirizzo " << devAddr << ", zona "<< sectionName <<" non esiste nel sistema\nContinuo..."<<endl;
                sleep ( 5 );
                continue;
            }
            
            if (netPtr->CheckControllerType(netIndex,ctrlIndex,DEV_ACC) )
            {
                //Ora ho tutti i dati:
                newAccCtrl.channel = accChannel;
                newAccCtrl.accController = ( CNTH_ACC* ) ( ( netPtr->GetNetHandler ( netIndex ) )->CtrlList[ctrlIndex] );
                newZone.accCtrlList.push_back ( newAccCtrl );
            }
            else if (netPtr->CheckControllerType(netIndex,ctrlIndex,DEV_IBUTT_RDR) )
            {
                CIButtonReader* rdr = ( CIButtonReader* ) ( ( netPtr->GetNetHandler ( netIndex ) )->CtrlList[ctrlIndex] );
                newZone.ibtnrdrList.push_back (rdr);
            }
            else
            {
                cout << "ATTENZIONE: l'indirizzo " << devAddr << ", zona "<< sectionName <<" non e' valido come dispositivo di accesso.\nContinuo..."<<endl;
                sleep ( 5 );
                continue;
            }
        } //For acc Index

        //Leggo un eventuale comando verso una centrale esterna
        deviceName = "OutputToAlarm";

        configString = iniFileReader.GetString ( deviceName, sectionName, "" );

        if (configString.size() != 0)
        {
            m_IniLib.GetConfigParamInt ( configString.c_str(), "ADDR", &devAddr, -1 );
            //Prendo i riferimenti del dispositivo
            netIndex = netPtr->GetNetByMemoryAddress ( devAddr );
            ctrlIndex = netPtr->GetDeviceIndexByMemoryAddress ( netIndex, devAddr );

            if ( ( netIndex < 0 ) || ( ctrlIndex < 0 ) )
            {
                cout << "ATTENZIONE: l'indirizzo dell'uscita " << devAddr << ", zona "<< sectionName <<" non esiste nel sistema\nContinuo..."<<endl;
                sleep ( 5 );
            }
            else
            {
                newZone.outDido = ( CDigitalIO* ) ( ( netPtr->GetNetHandler ( netIndex ) )->CtrlList[ctrlIndex] );
            }
        }
        
        //Leggo un eventuale comando verso una centrale esterna
        deviceName = "InputForAlarm";

        configString = iniFileReader.GetString ( deviceName, sectionName, "" );

        if (configString.size() != 0)
        {
            m_IniLib.GetConfigParamInt ( configString.c_str(), "ADDR", &devAddr, -1 );
            //Prendo i riferimenti del dispositivo
            netIndex = netPtr->GetNetByMemoryAddress ( devAddr );
            ctrlIndex = netPtr->GetDeviceIndexByMemoryAddress ( netIndex, devAddr );

            if ( ( netIndex < 0 ) || ( ctrlIndex < 0 ) )
            {
                cout << "ATTENZIONE: l'indirizzo dell'ingresso per allarme " << devAddr << ", zona "<< sectionName <<" non esiste nel sistema\nContinuo..."<<endl;
                sleep ( 5 );
            }
            else
            {
                newZone.inDido = ( CDigitalIO* ) ( ( netPtr->GetNetHandler ( netIndex ) )->CtrlList[ctrlIndex] );
                m_IniLib.GetConfigParamInt(configString.c_str(), "LOGIC", &(newZone.inDidoLogic), 0);
            }
        }
        
        
        //Carico eventuali comandi esterni
        nOfItems = iniFileReader.GetInt ( "nOfCommands", sectionName, 0 );
        for (int i = 0; i < nOfItems; i++)
        {
            t_Command newCommand;
            deviceName = "Command";
            deviceName+=(i+1);
            
            configString = iniFileReader.GetString ( deviceName, sectionName, "" );
            
            if (configString.size() != 0)
            {
                m_IniLib.GetConfigParamString(configString.c_str(),"TYPE",&(newCommand.type), "");
                m_IniLib.GetConfigParamString(configString.c_str(),"FIELD",&(newCommand.field), "");
                m_IniLib.GetConfigParamString(configString.c_str(),"VALUE",&(newCommand.value), "");
                m_IniLib.GetConfigParamString(configString.c_str(),"FIELD2",&(newCommand.field2), "");
                m_IniLib.GetConfigParamString(configString.c_str(),"VALUE2",&(newCommand.value2), "");
                m_IniLib.GetConfigParamInt(configString.c_str(),"ADDR",&(newCommand.address), -1);
                m_IniLib.GetConfigParamInt(configString.c_str(),"WHEN",&(newCommand.when), 0);
                newZone.commandList.push_back(newCommand);
            }
            else
            {
                cout<<"ATTENZIONE: AlarmCoordinator -- manca comando numero "<<i+1<<endl;
                sleep(2);
            }
        }

        //Salvo la zona
        m_ZoneVector.push_back ( newZone );

        //Carico le chiavi della zona
        if ( !LoadKeys ( zoneIndex, sectionName, &iniFileReader ) )
        {
            cout << "Impossibile caricare le chiavi di accesso per la "<< sectionName<<endl;
            sleep ( 5 );
            continue;
        }
    }

    //carico e collego digitali ingresso
    if ( !ConnectDIDO ( "DIGITALINPUTS", &iniFileReader, true ) )
    {
        cout << "Impossibile connettere gli ingressi digitali per il controllore allarmi"<<endl;
        sleep ( 5 );
    }

    if ( !ConnectDIDO ( "DIGITALOUTPUTS", &iniFileReader, false ) )
    {
        cout << "Impossibile connettere le uscite digitali per il controllore allarmi"<<endl;
        sleep ( 5 );
    }


    return true;
}
///////////////////////////////
/**
 *
 * @param zoneListString
 * @param zoneNumbers
 * @return
 */
int AlarmCoordinator::DecodeZones ( CString zoneListString, int * zoneNumbers, int maxNofZones, t_AlarmInputs *alarmInput=0x0 )
{
    int nOfZones = 0;
    string::size_type idx=0, idx2=0;
    CString param;
    bool finished = false;
    char sep='|';

    //Cerco il separatore: se non sto trattando degli ingressi può essere il solito '-'
    if (alarmInput == 0x0)
    {
        sep = '-';
    }
    else
    {
        idx = zoneListString.find ( sep,idx );
        if (idx == string::npos)
        {
            //NOn è un OR, allora è un AND
            sep = '&';
            alarmInput->zoneOperator = OP_AND;
        }
        else
        {
            alarmInput->zoneOperator = OP_OR;
        }
    }

    idx = 0;
        
    while ( ( !finished ) && ( nOfZones < maxNofZones ) )
    {
        nOfZones++;
        idx2 = zoneListString.find ( sep,idx );
        if ( idx2 == string::npos )
        {
            idx2 = zoneListString.length();
            finished = true;
        }

        param = zoneListString.substr ( idx, idx2 - idx );
        zoneNumbers[nOfZones-1] = atoi ( param.c_str() );
        if (alarmInput != 0x0)
        {
            alarmInput->zones.push_back(zoneNumbers[nOfZones-1] - 1);
        }
        idx = idx2+1;
    }

    return nOfZones;
}

/**
 *
 * @param sectionName
 * @param iniFileReader
 * @param inVector
 * @return
 */
bool AlarmCoordinator::ConnectDIDO ( CString sectionName, CIniFileHandler* iniFileReader,bool isInput )
{
    bool retVal = false;
    COneWireNet* netPtr = reinterpret_cast<COneWireNet*> ( m_NetPtr );
    int ctrlIndex = 0 , netIndex = 0, itemIndex = 0, zoneIndex = 0;
    int nOfItems = 0;
    int devAddr = 0;
    CString deviceName, configString;
    int zoneArray[MAX_NOF_ZONES];
    char zoneListString[255];
    int nOfZones = 0;
    CLibIniFile configReader;
    CDigitalIO* didoPtr;

    if ( netPtr == 0x0 )
    {
        return retVal;
    }

    nOfItems = iniFileReader->GetInt ( "nOfDev", sectionName, 0 );

    for ( itemIndex = 0; itemIndex < nOfItems; itemIndex++ )
    {
        t_AlarmInputs newAlarmInput;
        
        deviceName = "Device";
        if ( (itemIndex+1) < 10 )
        {
            deviceName+="0";
        }

        deviceName += itemIndex+1;

        configString = iniFileReader->GetString ( deviceName, sectionName, "" );

        if ( configString.size() == 0 )
        {
            cout << "ATTENZIONE: manca il " << deviceName << ", sezione " << sectionName <<", nel file alarmControl.ini\nContinuo..."<<endl;
            sleep ( 5 );
            continue;
        }

        m_IniLib.GetConfigParamInt ( configString.c_str(), "ADDR", &devAddr, -1 );

        if ( devAddr < 0 )
        {
            cout << "ATTENZIONE: manca ADDR nel " << deviceName << ", sezione " << sectionName <<", nel file alarmControl.ini\nContinuo..."<<endl;
            sleep ( 5 );
            continue;
        }

        //Prendo i riferimenti del dispositivo
        netIndex = netPtr->GetNetByMemoryAddress ( devAddr );
        ctrlIndex = netPtr->GetDeviceIndexByMemoryAddress ( netIndex, devAddr );

        if ( ( netIndex <0 ) || ( ctrlIndex < 0 ) )
        {
            //TODO generazione di errore
            cout << "ATTENZIONE: errrore di indirizzo nel dispositivo: "<< deviceName <<" "<< sectionName<<", nel file alarmControl.ini\nContinuo..."<<endl;
            sleep ( 5 );
            continue;
        }

        //Devo controllare che il controllore sia davvero un controllore digitale
        didoPtr = ( CDigitalIO* ) ( ( netPtr->GetNetHandler ( netIndex ) )->CtrlList[ctrlIndex] );

        if ( !netPtr->CheckControllerType ( netIndex, ctrlIndex, DEV_DIDO ) )
        {
            //TODO errore
            cout << "ATTENZIONE: il dispositivo: "<< deviceName << " "<<sectionName<<"NON E' UN DIGITALIO, nel file alarmControl.ini\nContinuo..."<<endl;
            sleep ( 5 );
            continue;
        }

        newAlarmInput.digitalInput = didoPtr;

        //Qui devo decodificare le zone
        memset ( zoneListString,0,255 );
        m_IniLib.GetConfigParamString ( configString.c_str(), "ZONES", zoneListString, 255,"" );
        
        //I dati extrazona sugli ingressi sono già memorizzati, faccio una push se si parla di ingressi
        if (isInput)
        {
            nOfZones = DecodeZones ( zoneListString, zoneArray, MAX_NOF_ZONES, &newAlarmInput );
            m_AlarmInputsVector.push_back(newAlarmInput);
        }
        else
        {
            nOfZones = DecodeZones ( zoneListString, zoneArray, MAX_NOF_ZONES);
        }

        for ( zoneIndex = 0; zoneIndex < nOfZones; zoneIndex++ )
        {

            if ( ( zoneArray[zoneIndex]<0 ) || ( zoneArray[zoneIndex]>(int)m_ZoneVector.size() ) )
            {
                //TODO Errore!!!!!
                sleep ( 5 );
                continue;
            }

            if ( isInput )
            {
                m_ZoneVector.at ( zoneArray[zoneIndex] - 1 ).digINList.push_back ( didoPtr );
            }
            else
            {
                m_ZoneVector.at ( zoneArray[zoneIndex] - 1 ).digOUTList.push_back ( didoPtr );
            }
        }
    }

    return true;
}
//////////////////////////////////////////////////////////////////////////////////////
bool AlarmCoordinator::LoadKeys ( int zoneIndex, CString zoneName, CIniFileHandler *iniReader )
{
    bool retVal = true;
    t_AccessData newKey;
    int nOfKeys = 0, keyIndex = 0;
    CString keyName, keyConfigString;
    CLibIniFile configParser;

    nOfKeys = iniReader->GetInt ( "nOfKeys", zoneName, 0 );

    for ( keyIndex = 0; keyIndex < nOfKeys; keyIndex++ )
    {
        ResetAccessData(&newKey);
        keyName = "key";
        if ( (keyIndex+1)<10 )
        {
            keyName+="0";
        }
        keyName+=keyIndex+1;

        keyConfigString = iniReader->GetString ( keyName, zoneName, "" );

        if ( keyConfigString.size() == 0 )
        {
            cout << "ATTENZIONE!! Manca la configurazione per la chiave numero "<<keyIndex+1<<" zona "<<zoneName<<"\nContinuo..."<<endl;
            sleep ( 5 );
            continue;
        }

        configParser.GetConfigParamString ( keyConfigString.c_str(),"NAME", & ( newKey.name ), "NA" );
        configParser.GetConfigParamString ( keyConfigString.c_str(),"SN", & ( newKey.keySN ), "NA" );
        if ( newKey.keySN.length() < 16 )
        {
            cout << "ATTENZIONE!! Manca il numero di serie per la chiave numero "<<keyIndex+1<<" zona "<<zoneName<<"\nContinuo..."<<endl;
            sleep ( 5 );
            continue;
        }
        configParser.GetConfigParamBool ( keyConfigString.c_str(),"ALARMENABLED", & ( newKey.enablesAlarms ), false );

        m_ZoneVector[zoneIndex].keysList.push_back(newKey);
    }

    return retVal;
}
//////////////////////////////////////////////////////////////////////////////////////
bool AlarmCoordinator::SearchForActivity()
{
    bool retVal = true;
    int nOfActivityDIDO = 0;
    int activityDIDOList[MAX_NUM_DEV_PER_NET];
    int masterPortIndex = 0, nOfPorts = 0, didoIndex = 0;
    vector<CVDevice*> ds2408ToClear;
    COneWireNet* netPtr = reinterpret_cast<COneWireNet*> ( m_NetPtr );
    int didoAddress;

    if ( netPtr == 0x0 )
    {
        return false;
    }

    vector<t_Zone>::iterator zoneIt;
    vector<CDigitalIO*>::iterator digitalIt;
    
    for (zoneIt = m_ZoneVector.begin(); zoneIt < m_ZoneVector.end(); zoneIt++)
    {
        for (digitalIt = zoneIt->digINList.begin(); digitalIt < zoneIt->digINList.end(); digitalIt++)
        {
            if (((*digitalIt)->GetActivity(false)) || (*digitalIt)->GetLastState())
            {
                didoAddress = (*digitalIt)->GetMemoryAddress();
                //Allarme
                if ( FindZoneInAlarm ( didoAddress ) )
                {
                    //21/09/2009 -- Aggiunta per debug
                    if (m_DebugLevel)
                    {
                        cout << "Allarme!! Zona:"<<zoneIt->zoneNumber<<" sensore:"<<(*digitalIt)->GetMemoryAddress()<<endl;
                    }
                    retVal = SendAlarms();
                }
            }
        }
    }
    
    //Agosto 2009 -- Provato a commentare dopo cambiamenti introdotti
//     nOfPorts = netPtr->GetMasterHandler()->GetNofPortsAcquired();
// 
//     for ( masterPortIndex = 0; masterPortIndex < nOfPorts; masterPortIndex++ )
//     {
//         memset ( activityDIDOList, 0x0, MAX_NUM_DEV_PER_NET*sizeof ( int ) );
//         nOfActivityDIDO = netPtr->SearchDIDOActivity ( masterPortIndex, activityDIDOList, MAX_NUM_DEV_PER_NET, ds2408ToClear );
// 
//         if ( nOfActivityDIDO == 0 )
//         {
//             return true;
//         }
// 
//         //Decodifico se c'è un elemento della mia rete ed eventualmente lancio l'allarme
//         for ( didoIndex = 0; didoIndex < nOfActivityDIDO; didoIndex++ )
//         {
//             didoAddress = activityDIDOList[didoIndex];
// 
//             //Devo ciclare su tutte le zone per vedere a quale appartiene il dido
//             if ( FindZoneInAlarm ( didoAddress ) )
//             {
//                 retVal = SendAlarms();
//             }
//         }
//     }

   
    return retVal;
}

/**
 * Finds all the zones associated to the given address and sets their alarm flag
 * @param address address of the digital input in alarm
 * @return number of alarmed zones
 */
int AlarmCoordinator::FindZoneInAlarm ( int address )
{
    int retVal = 0;
    int zoneIndex = 0;
    int didoIndex = 0;
    int i;
    bool activateAlarm;

    //Controllo i digitali di ingresso dal vettore degli allarmi
    for (didoIndex = 0; didoIndex < (int)m_AlarmInputsVector.size(); didoIndex++)
    {
        if (m_AlarmInputsVector[didoIndex].digitalInput->GetMemoryAddress() == address)
        {
            //Inizializzo il valore dell'attivazione in modo tale da ciclare indipendentemente
            //dal numero delle zone associate all'ingresso.
            if (m_AlarmInputsVector[didoIndex].zoneOperator == OP_AND)
            {
                activateAlarm = true;
            }
            else if (m_AlarmInputsVector[didoIndex].zoneOperator == OP_OR)
            {
                activateAlarm = false;
            }
            else
            {
                break;
            }

            for (i = 0; i < (int)m_AlarmInputsVector[didoIndex].zones.size(); i++)
            {
                zoneIndex = m_AlarmInputsVector[didoIndex].zones[i];
                if (m_AlarmInputsVector[didoIndex].zoneOperator == OP_AND)
                {
                    activateAlarm = activateAlarm && m_ZoneVector[zoneIndex].isAlarmActive;
                }
                else
                {
                    activateAlarm = activateAlarm || m_ZoneVector[zoneIndex].isAlarmActive;
                }
            }

            //A questo punto activateAlarm mi da' lo stato globale dell'allarme delle zone sorvegliate da quell'ingresso:
            //quindi devo riportarlo nelle zone solo se activateAlarm è attivo, altrimenti esco perche' staccherei l'allarme
            //anche dovuto ad altri input
            if (!activateAlarm)
            {
                return 0;
            }
            
            if (m_DebugLevel)
            {
                cout << "Allarme nella zona "<< zoneIndex+1<<" -- sensore di indirizzo "<<address<<endl;
            }
            
            for (i = 0; i < (int)m_AlarmInputsVector[didoIndex].zones.size(); i++)
            {
                zoneIndex = m_AlarmInputsVector[didoIndex].zones[i];
                m_ZoneVector[zoneIndex].isInAlarm = activateAlarm;
                retVal++;
            }

            break;
        }
    }
    
    return retVal;
}

/**
 * Sets all the digital outputs of the given zone to the given state
 * @param zone index of the zone to alarm
 * @param state state of the dido to write
 * @return true if everything went well
 */
bool AlarmCoordinator::SetZoneDIGOUT ( int state, int zone )
{
    bool retVal = true;
    vector<CDigitalIO*>::iterator didoIt;


    for ( didoIt = m_ZoneVector[zone].digOUTList.begin(); didoIt < m_ZoneVector[zone].digOUTList.end(); didoIt++ )
    {
        retVal = ( *didoIt )->SetState ( state ) && retVal;
    }

    return retVal;
}

/**
 * Sends the alarms to the outputs of the zones
 * @return true if alarm was sent
 */
bool AlarmCoordinator::SendAlarms()
{
    bool retVal = true;
    int zoneIndex;

    for ( zoneIndex = 0; zoneIndex < (int)m_ZoneVector.size(); zoneIndex++ )
    {
        if ( m_ZoneVector[zoneIndex].isInAlarm )
        {
          
          SendCommands(zoneIndex, false);
          
          retVal = SetZoneAlarm ( zoneIndex ) && retVal;
        }
    }

    return retVal;
}

/**
 * Changes the reader led based on the state of the alarms of the nets
 * @param  index of the zone to which we have to change the led color
 * @return false on error, true otherwise
 */
bool AlarmCoordinator::ChangeZoneAlarmState ( int zoneIndex )
{
    bool retVal = true;
    uchar mapmem[24];
    vector<t_AccController>::iterator accIt;
    vector<CIButtonReader*>::iterator iBtnIt;
    COneWireEngine* engPtr = reinterpret_cast<COneWireEngine*> ( m_EnginePtr );

    for ( accIt = m_ZoneVector[zoneIndex].accCtrlList.begin(); accIt < m_ZoneVector[zoneIndex].accCtrlList.end(); accIt++ )
    {
        memset ( mapmem,0xFF,24 );

        mapmem[0] = 0x02;
        mapmem[1] = accIt->channel<<1;

        if (accIt->channel & 0x1)
        {
            mapmem[6] = (char)(m_ZoneVector[zoneIndex].isAlarmActive);
        }

        if (accIt->channel & 0x02)
        {
            mapmem[7] = (char)(m_ZoneVector[zoneIndex].isAlarmActive);
        }
        
        mapmem[15] = accIt->accController->CalcCRC( mapmem );
        retVal = accIt->accController->WriteToDevice ( mapmem ) && retVal;
    }
    
    //Controllo se devo cambiare anche degli iButton direttamente
    for (iBtnIt = m_ZoneVector[zoneIndex].ibtnrdrList.begin(); iBtnIt < m_ZoneVector[zoneIndex].ibtnrdrList.end(); iBtnIt++)
    {
        if (m_ZoneVector[zoneIndex].isAlarmActive)
        {
            //Accendo la luce rossa
            (*iBtnIt)->SetLight(false, true);   
        }
        else
        {
            //Accendo la luce verde
            (*iBtnIt)->SetLight(true, true);
        }
    }

    //Cambio anche l'uscita remota
    if (m_ZoneVector[zoneIndex].outDido != 0x0)
    {
        if (m_ZoneVector[zoneIndex].isAlarmActive)
        {
            m_ZoneVector[zoneIndex].outDido->SetState(1);
        }
        else
        {
            m_ZoneVector[zoneIndex].outDido->SetState(0);
        }
    }
    
    //Mando comando fisso
    if ((m_ZoneVector[zoneIndex].remoteAddr > -1) && (engPtr != 0x0))
    {
        Cmd com("DEVICE");
        
        com.putValue("TYPE", "AlarmState");
        com.putValue("ZONE", zoneIndex);
        com.putValue("ADDRESS", m_ZoneVector[zoneIndex].remoteAddr);
        com.putValue("STATE", m_ZoneVector[zoneIndex].isAlarmActive);
       
        engPtr->WriteOnOutputPorts(com.toString(), m_ZoneVector[zoneIndex].remoteAddr);
    }

    SendCommands(zoneIndex, true);

    //Se stacco l'allarme resetto le uscite
    if (!m_ZoneVector[zoneIndex].isAlarmActive)
    {
        ClearZoneAlarm(zoneIndex);
    }
            

    return retVal;
}
///////////////////////////////////////////////////////////////////
bool AlarmCoordinator::InitAlarmSystem()
{
    bool retVal = true;
    uchar mapmem[24];
    vector<t_AccController>::iterator accIt;
    vector<t_Zone>::iterator zoneIt;
    vector<CDigitalIO*>::iterator didoIt;
    vector<CIButtonReader*>::iterator iBtnIt;

    for ( zoneIt = m_ZoneVector.begin(); zoneIt < m_ZoneVector.end(); zoneIt++ )
    {
        for ( accIt = zoneIt->accCtrlList.begin(); accIt < zoneIt->accCtrlList.end(); accIt++ )
        {
            //Prima aggiorno il tempo
            if ( ! ( accIt->accController->UpdateCommonData() ) )
            {
                //TBI messaggi errore
                cout << "Errore nel modulo di controllo accessi: "<< accIt->accController->GetMemoryAddress() << "Impossibile aggiornare orario"<<endl;
                continue;
            }

            //Prima cancello dalla scheda tutte le chiavi del canale che vado a riprogrammare
            memset ( mapmem, 0xFF, 24 );
            mapmem[0] = 0x03;
            mapmem[1] = ( accIt->channel ) <<3;
            mapmem[15] = accIt->accController->CalcCRC(mapmem);

            retVal = accIt->accController->WriteToDevice ( mapmem ) && retVal;

            if ( !retVal )
            {
                //Questo è un errore grave: non sono riuscito a cancellare le chiavi -> non ne registro di nuove
                //TODO Messaggio errore
                continue;
            }

            retVal = ProgramAccessDevice ( zoneIt, accIt ) && retVal;
        }
        
        //Resetto i lettori iButton
        for (iBtnIt = zoneIt->ibtnrdrList.begin(); iBtnIt <zoneIt->ibtnrdrList.end(); iBtnIt++)
        {
            //Accendo la luce verde
            (*iBtnIt)->SetLight(true, true);
        }

        for (didoIt = zoneIt->digOUTList.begin(); didoIt < zoneIt->digOUTList.end(); didoIt++)
        {
            (*didoIt)->InitDevice();
        }

        ChangeZoneAlarmState ( zoneIt - m_ZoneVector.begin() );

        
    }

    return retVal;
}
///////////////////////////////////////////////////////////////////
bool AlarmCoordinator::ResetAlarm ( int zone )
{
    bool retVal = true;
    int zoneStart, zoneStop, zoneIndex;

    if ( !ExtractCycleLimits ( zone, &zoneStart, &zoneStop ) )
    {
        return false;
    }

    for ( zoneIndex = zoneStart; zoneIndex<zoneStop; zoneIndex++ )
    {
        m_ZoneVector[zoneIndex].isInAlarm = false;
        retVal = ClearZoneAlarm ( zoneIndex ) && retVal;
    }

    return retVal;
}
///////////////////////////////////////////////////////////////////
int AlarmCoordinator::FindKeyInZones ( CString keySN, int* zoneList, int maxZones )
{
    int nOfKeys = 0;
    vector<t_Zone>::iterator zoneIt;
    vector<t_AccessData>::iterator accessIt;

    for ( zoneIt = m_ZoneVector.begin(); zoneIt < m_ZoneVector.end(); zoneIt++ )
    {
        for ( accessIt = zoneIt->keysList.begin(); accessIt < zoneIt->keysList.end(); accessIt++ )
        {
            if ( keySN == accessIt->keySN )
            {
                //TODO -- Da testare
                zoneList[nOfKeys] = zoneIt - m_ZoneVector.begin();
                nOfKeys++;

                //A questo punto potrei saltare alla zona successiva tanto ogni chiave deve essere presente una sola volta
                break;
            }
        }
    }

    return nOfKeys;
}
//////////////////////////////////////////////////////////////////////////////
bool AlarmCoordinator::EraseZoneKeysFromFile ( int zoneNumber )
{
    bool retVal = true;
    int nOfkeys = 0;
    CIniFileHandler iniFileReader;
    CString zoneName, keyName;
    int zoneStart, zoneStop, zoneIndex, keyIndex;

    if ( ( !iniFileReader.Load ( "./alarmControl.ini" ) ) || ( !ExtractCycleLimits ( zoneNumber,&zoneStart,&zoneStop ) ) )
    {
        return false;
    }

    for ( zoneIndex = zoneStart; zoneIndex < zoneStop; zoneIndex++ )
    {
        zoneName = "ZONE";
        zoneName += zoneIndex+1;

        nOfkeys = iniFileReader.GetInt ( "nOfKeys", zoneName, 0 );

        for ( keyIndex = 0 ; keyIndex < nOfkeys; keyIndex++ )
        {
            keyName = "key";
            if ( (keyIndex+1) < 10 )
            {
                keyName+="0";
            }
            keyName+=keyIndex+1;

            retVal = iniFileReader.DeleteKey ( keyName, zoneName ) && retVal;
        }

        //Aggiorno il numero delle chiavi
        retVal = iniFileReader.SetInt ( "nOfKeys",0, "", zoneName );
    }

    //Salvo
    iniFileReader.Save();

    return retVal;
}
////////////////////////////////////////////////////////////////////////////////////
bool AlarmCoordinator::EraseKeyFromFile ( CString keySN, int zoneIndex )
{
    CIniFileHandler iniFileReader;
    CString zoneName, keyName;
    int numZones, zoneIdx;
    int zoneArray[MAX_NOF_ZONES];
    vector<t_AccessData>::iterator keyIt;
    int arrayIndex;

    if ( !iniFileReader.Load ( "./alarmControl.ini" ) )
    {
        return false;
    }

    memset ( zoneArray, 0, MAX_NOF_ZONES*sizeof ( int ) );

    if ( zoneIndex < 0 )
    {
        //Cerco la chiave in tutte le zone
        numZones = FindKeyInZones ( keySN, zoneArray, MAX_NOF_ZONES );
    }
    else
    {
        numZones = 1;
        zoneArray[0] = zoneIndex;
    }

    for ( zoneIdx = 0; zoneIdx < numZones; zoneIdx++ )
    {
        arrayIndex = zoneArray[zoneIdx];
        //LA cancello dall'array
        for ( keyIt = m_ZoneVector[arrayIndex].keysList.begin(); keyIt < m_ZoneVector[arrayIndex].keysList.end(); keyIt++ )
        {
            if ( keyIt->keySN == keySN )
            {
                //Chiave trovata
                m_ZoneVector[arrayIndex].keysList.erase ( keyIt );
                break;
            }
        }

        EraseZoneKeysFromFile ( zoneIdx );
        WriteZoneKeysOnFile ( zoneIdx );
    }

    iniFileReader.Save();

    return true;
}
////////////////////////////////////////////////////////////////////////////////////////
bool AlarmCoordinator::ExtractCycleLimits ( int zone, int * startIndex, int * stopIndex )
{
    if ( zone > (int)m_ZoneVector.size() )
    {
        return false;
    }

    if ( zone < 0 )
    {
        *startIndex = 0;
        *stopIndex = m_ZoneVector.size();
    }
    else
    {
        *startIndex = zone;
        *stopIndex = zone+1;
    }

    return true;
}
////////////////////////////////////////////////////////////////////////////////////////
bool AlarmCoordinator::AddKey ( CString name, CString keySN, bool enablesAlarms, int zoneIndex )
{
    bool retVal = true;
    int startZone, stopZone, zoneIdx;
    t_AccessData newKey;
    int keyZones[MAX_NOF_ZONES], nOfKeyZones;

    //Riporto l'indice di zona a partire da 0
    zoneIndex--;
    
    if ( !ExtractCycleLimits ( zoneIndex, &startZone, &stopZone ) )
    {
        return false;
    }

    ResetAccessData(&newKey);

    newKey.keySN = keySN;
    newKey.name = name;
    newKey.enablesAlarms = enablesAlarms;

    //Cerco se la chiave è già presente in qualche zona
    nOfKeyZones = FindKeyInZones ( keySN, keyZones, MAX_NOF_ZONES );

    for ( zoneIdx = startZone; zoneIdx<stopZone; zoneIdx++ )
    {
        int i = 0;
        bool keyExists = false;
        
        for ( i = 0; i < nOfKeyZones; i++ )
        {
            if ( zoneIdx == keyZones[i] )
            {
                //La chiave è già presente in questa zona
                keyExists = true;
                break;
            }
        }

        if (!keyExists)
        {
            vector<t_Zone>::iterator zoneIt;
            vector<t_AccController>::iterator accIt;

            //La chiave appartiene alla zona considerata
            //La aggiungo alla zona
            m_ZoneVector[zoneIdx].keysList.push_back ( newKey );
            zoneIt = m_ZoneVector.begin();
            //TODO da testare l'aritmetica dell'iteratore
            zoneIt+=zoneIdx;

            for ( accIt = m_ZoneVector[zoneIdx].accCtrlList.begin(); accIt <m_ZoneVector[zoneIdx].accCtrlList.end(); accIt++ )
            {
                //Devo aggiungerla al dispositivo!!
                ProgramAccessDevice ( zoneIt, accIt );
            }
        }
    }

    //Scrivo tutta la situazione
    WriteZoneKeysOnFile ( zoneIndex );

    return retVal;
}
////////////////////////////////////////////////////////////////////////////////////////
bool AlarmCoordinator::RemoveKey ( CString keySN, int zoneIndex )
{
    bool retVal = true;
    int startZone, stopZone, zoneIdx;
    t_AccessData newKey;
    int keyZones[MAX_NOF_ZONES], nOfKeyZones;

    //Riporto l'indice di zona a partire da 0
    zoneIndex--;
    
    if ( !ExtractCycleLimits ( zoneIndex, &startZone, &stopZone ) )
    {
        return false;
    }

    nOfKeyZones = FindKeyInZones ( keySN, keyZones, MAX_NOF_ZONES );

    if ( nOfKeyZones == 0 )
    {
        //la chiave non esiste da nessuna parte
        return true;
    }

    for ( zoneIdx = startZone; zoneIdx<stopZone; zoneIdx++ )
    {
        int i = 0;
        for ( i = 0; i < nOfKeyZones; i++ )
        {
            if ( zoneIdx == keyZones[i] )
            {
                vector< t_AccessData >::iterator keyIt;
//                 vector<t_Zone>::iterator zoneIt;
                vector<t_AccController>::iterator accIt;

                for ( keyIt = m_ZoneVector[zoneIdx].keysList.begin(); keyIt < m_ZoneVector[zoneIdx].keysList.end(); keyIt++ )
                {
                    if ( keyIt->keySN == keySN )
                    {
                        for ( accIt = m_ZoneVector[zoneIdx].accCtrlList.begin(); accIt <m_ZoneVector[zoneIdx].accCtrlList.end(); accIt++ )
                        {
                            //Devo aggiungerla al dispositivo!!
                            RemoveKeyFromDevice ( keyIt, accIt );
                        }

                        //Elimino la chiave dalla zona
                        m_ZoneVector[zoneIdx].keysList.erase ( keyIt );
                        //Anche qui posso uscire dal loop interno perchè ogni chiave dovrebbe esserci una volta sola
                        break;
                    }
                }
            }
        }

        EraseZoneKeysFromFile ( zoneIdx );
        WriteZoneKeysOnFile ( zoneIdx );
    }

    return retVal;
}
////////////////////////////////////////////////////////////////////////////////////////
bool AlarmCoordinator::WriteZoneKeysOnFile ( int zoneIndex )
{
    bool retVal = true;
    vector<t_AccessData>::iterator keyIt;
    CIniFileHandler iniFileReader;
    CString zoneName, keyName, keyConfig;
    int zoneStart, zoneStop, zoneIdx;

    //Riporto l'indice di zona nel range a partire da 0
//     zoneIndex--;
    
    if ( ( !iniFileReader.Load ( "./alarmControl.ini" ) ) || ( !ExtractCycleLimits ( zoneIndex,&zoneStart,&zoneStop ) ) )
    {
        return false;
    }

    for ( zoneIdx = zoneStart; zoneIdx < zoneStop; zoneIdx++ )
    {
        zoneName = "ZONE";
        zoneName+=zoneIdx+1;

        //Salvo il numero chiavi
        iniFileReader.SetInt("nOfKeys",m_ZoneVector[zoneIdx].keysList.size(),"",zoneName);
        
        for ( keyIt = m_ZoneVector[zoneIdx].keysList.begin(); keyIt < m_ZoneVector[zoneIdx].keysList.end(); keyIt++ )
        {
            keyName = "key";
            if ( (( keyIt - m_ZoneVector[zoneIdx].keysList.begin() )+1) < 10 )
            {
                keyName+="0";
            }
            keyName+=(keyIt-m_ZoneVector[zoneIdx].keysList.begin())+1;

            keyConfig="NAME:";
            keyConfig+=keyIt->name;
            keyConfig+=",SN:";
            //NAME:pippo,SN:123456789ABCDEF,ALARMENABLED:1,EXP:0
            keyConfig+=keyIt->keySN;
            keyConfig+=",ALARMENABLED:";
            keyConfig+=keyIt->enablesAlarms;
            keyConfig+=",EXP:0";
            iniFileReader.SetValue ( keyName, keyConfig,"",zoneName );
        }
    }

    iniFileReader.Save();

    return retVal;
}
////////////////////////////////////////////////////////////////////////////////////////
bool AlarmCoordinator::ProgramAccessDevice ( vector< t_Zone >::iterator zoneIt, vector< t_AccController >::iterator accIt )
{
    bool retVal = true;
    uchar mapmem[24];
    vector <t_AccessData>::iterator keyIt;
    uchar hexSerNum[16];


    for ( keyIt = zoneIt->keysList.begin(); keyIt < zoneIt->keysList.end(); keyIt++ )
    {
        memset ( mapmem, 0xFF, 24 );
        memset ( hexSerNum, 0, 16 );
        mapmem[0] = 0x03;
        mapmem[1] = 0x02;

        //TODO qui andrebbe la scadenza
        memset(&mapmem[2],0x0,4);
        
        ConvertSN2Hex ( keyIt->keySN.c_str(), hexSerNum );
        memcpy ( & ( mapmem[6] ), hexSerNum, 8 );
        //I canali della scheda sono numerati come 0, 1 e 2 per entrambi
        mapmem[14] = accIt->channel - 1;

//         if ( keyIt->enablesAlarms )
//         {
//             mapmem[15] = 1;
//         }
        mapmem[15] = accIt->accController->CalcCRC(mapmem);

        //TODO da mettere errore in caso di fallimento
        retVal = accIt->accController->WriteToDevice ( mapmem ) && retVal;
    }

    return retVal;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AlarmCoordinator::RemoveKeyFromDevice ( vector< t_AccessData >::iterator keyIt, vector< t_AccController >::iterator accIt )
{
    bool retVal = false;
    uchar mapmem[24];

    memset ( mapmem,0xFF,24 );

    mapmem[0] = 0x03;
    mapmem[1] = 0x04;
    ConvertSN2Hex ( keyIt->keySN.c_str(), &mapmem[6] );
    mapmem[14] = accIt->channel - 1;
    mapmem[15] = accIt->accController->CalcCRC(mapmem);

    retVal = accIt->accController->WriteToDevice ( mapmem );

    return retVal;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AlarmCoordinator::ResetZone(t_Zone * zone)
{
    zone->keysList.clear();
    zone->accCtrlList.clear();
    zone->ibtnrdrList.clear();
    zone->digINList.clear();
    zone->digOUTList.clear();
    zone->zoneNumber = 0;
    zone->isAlarmActive = false;
    zone->isInAlarm = false;
    zone->outDido = 0x0;
    zone->inDido = 0x0;
    zone->inDidoLogic = 0;
    zone->lastInputState = -1;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AlarmCoordinator::ResetAccessData(t_AccessData * accessData)
{
    accessData->roomNumber = 0;
    accessData->expireDateSec = 0;
    accessData->category = NONE;
    accessData->enablesAlarms = false;
    accessData->channel = 0;   //E' il canale della scheda di controllo: 1, 2 o entrambi (3)
    
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AlarmCoordinator::SetZoneAlarmState(unsigned int zoneIndex, bool newState)
{
    bool retVal = false;
    
    if ( (zoneIndex >= 0) && (zoneIndex < m_ZoneVector.size()) )
    {
        m_ZoneVector[zoneIndex].isAlarmActive = newState;
        ChangeZoneAlarmState(zoneIndex);
        retVal = true;
    }

    return retVal;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
0 - ONOFF: l'allarme è inserito o disinserito a seconda dello stato, 
1 - SOLO INSERIMENTO Transizione basso/alto inserisce 
2 - SOLO INSERIMENTO Transizione alto/basso inserisce, 
3 - SOLO DISINSERIMENTO basso/alto, 
4 - SOLO DISINSERIMENTO alto/basso, 
5 - inserimento e disinserimento con transizione basso/alto e alto/basso, 
6 - inserimento e disisnerimento con transizione alto/basso, basso/alto
7 - Inserimento e disinserimento con transizione alto/basso, 
8 - inserimento e disinserimento con transizione basso/alto
*/
        
void AlarmCoordinator::CheckInputActivation(int zoneIndex)
{
    //Cerco se c'e' l'ingresso digitale e se questo e' attivo
    if (m_ZoneVector[zoneIndex].inDido != 0x0)
    {
        int state = m_ZoneVector[zoneIndex].inDido->GetState(false);
                
        if (m_ZoneVector[zoneIndex].lastInputState == -1)
        {
            m_ZoneVector[zoneIndex].lastInputState == state;
        }
                
        int lastState = m_ZoneVector[zoneIndex].lastInputState;
        //A questo punto controllo la logica
        switch (m_ZoneVector[zoneIndex].inDidoLogic)
        {
            case 0:
            {
                SetZoneAlarmState(zoneIndex, state);
            };break;
            case 1:
            {
                if (!m_ZoneVector[zoneIndex].isAlarmActive)
                {
                    if ((lastState == 0) && (state == 1))
                    {
                        SetZoneAlarmState(zoneIndex, true);
                    }
                }   
            };break;
            case 2:
            {
                if (!m_ZoneVector[zoneIndex].isAlarmActive)
                {
                    if ((lastState == 1) && (state == 0))
                    {
                        SetZoneAlarmState(zoneIndex, true);
                    }
                }
            };break;
            case 3:
            {
                if (m_ZoneVector[zoneIndex].isAlarmActive)
                {
                    if ((lastState == 0) && (state == 1))
                    {
                        SetZoneAlarmState(zoneIndex, false);
                    }
                }   
            };break;
            case 4:
            {
                if (m_ZoneVector[zoneIndex].isAlarmActive)
                {
                    if ((lastState == 1) && (state == 0))
                    {
                        SetZoneAlarmState(zoneIndex, false);
                    }
                }
            };break;
            case 5:
            {
                //Controllo se devo attivare o disattivare l'allarme
                if ((state == 1) && (m_ZoneVector[zoneIndex].lastInputState == 0))
                {
                    //Attivo l'allarme
                    SetZoneAlarmState(zoneIndex, true);
                }
                else if ((state == 0) && (m_ZoneVector[zoneIndex].lastInputState == 1))
                {
                    SetZoneAlarmState(zoneIndex, false);
                }
            };break;
            case 6:
            {
                //Controllo se devo attivare o disattivare l'allarme
                if ((state == 0) && (m_ZoneVector[zoneIndex].lastInputState == 1))
                {
                    //Attivo l'allarme
                    SetZoneAlarmState(zoneIndex, true);
                }
                else if ((state == 1) && (m_ZoneVector[zoneIndex].lastInputState == 0))
                {
                    SetZoneAlarmState(zoneIndex, false);
                }
            };break;
            case 7:
            {
                if ((state == 0) && (m_ZoneVector[zoneIndex].lastInputState == 1))
                {
                    //Attivo l'allarme
                    SetZoneAlarmState(zoneIndex, !m_ZoneVector[zoneIndex].isAlarmActive);
                }
            };break;
            case 8:
            {
                if ((state == 1) && (m_ZoneVector[zoneIndex].lastInputState == 0))
                {
                    //Attivo l'allarme
                    SetZoneAlarmState(zoneIndex, !m_ZoneVector[zoneIndex].isAlarmActive);
                }
            };break;
        }
                
        m_ZoneVector[zoneIndex].lastInputState = state;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool AlarmCoordinator::CheckKeyInZone(CString keySN, int zoneIndex)
{
    //TODO da testare perchè non sono sicuro che l'operatore == funzioni sulle stringhe
    vector<t_AccessData>::iterator accessIt;
    
    if ((zoneIndex<0) || (zoneIndex > m_ZoneVector.size()))
    {
        return false;
    }
    
    for ( accessIt = m_ZoneVector[zoneIndex].keysList.begin(); accessIt < m_ZoneVector[zoneIndex].keysList.end(); accessIt++ )
    {
        if ( keySN == accessIt->keySN )
        {
            return true;
        }
    } 
    
    return false;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AlarmCoordinator::SendCommands(int zoneIndex, bool alarmState)
{
  COneWireEngine* engPtr = reinterpret_cast<COneWireEngine*> ( m_EnginePtr );
  
  //Controllo se ho comandi da inviare
  if ((m_ZoneVector[zoneIndex].commandList.size() > 0) && (engPtr != 0x0))
  {
    vector<t_Command>::iterator commIt;
    for (commIt = m_ZoneVector[zoneIndex].commandList.begin(); commIt < m_ZoneVector[zoneIndex].commandList.end(); commIt++)
    {
      bool sendCommand = false;
            
      if (alarmState)
      {
        if ( commIt->when == 0)
        {
            sendCommand = true;
        }
        else if ( ( commIt->when == 1) && (m_ZoneVector[zoneIndex].isAlarmActive))
        {
            sendCommand = true;
        }
        else if (( commIt->when == 2) && (!m_ZoneVector[zoneIndex].isAlarmActive))
        {
            sendCommand = true;
        }
      }
      else if (( commIt->when == 3) && (m_ZoneVector[zoneIndex].isInAlarm))
      {
        sendCommand = true;
      }

            
      if (sendCommand)
      {
        Cmd com("DEVICE");
        com.putValue("COMMAND",commIt->type);
                //se non c'è il campo value mando lo stato dell'allarme
        if (commIt->value.size())
        {
          com.putValue(commIt->field, commIt->value);
        }
        else
        {
          com.putValue(commIt->field,m_ZoneVector[zoneIndex].isAlarmActive);
        }
        
        if (commIt->field2.size())
        {
            com.putValue(commIt->field2, commIt->value2);
        }   
                 
                //Controllo l'indirizzo
        if (commIt->address > -1)
        {
          com.putValue("ADDRESS",commIt->address);
        }
                
        engPtr->WriteOnOutputPorts(com.toString());
      }
    }
  }   
}
