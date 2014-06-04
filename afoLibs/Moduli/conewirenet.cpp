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
#include "conewirenet.h"
#include "conewireengine.h"
#include "BlockCoordinator.h"

///////////////////////////////////////////////////
//              STANDARD CONSTRUCTOR
///////////////////////////////////////////////////
COneWireNet::COneWireNet ( CString iniFileName, CTimer *timer )
{
    m_LastAddress = -1;

    m_Timer = timer;
    m_NetList.reserve ( MAX_NUM_NET );
    m_AfoErrorHandler = 0x0;
    m_Master = 0x0;

    time ( &m_LastBLRecoveryTime );

}

///////////////////////////////////////////////////
//  STANDARDDESTRUCTOR
///////////////////////////////////////////////////
COneWireNet::~COneWireNet()
{
    int i = 0;
    vector<T_Net>::iterator netIt;

    //Release all the nets
    ReleaseAllNets();


    //Clean up all the device objects
    for ( netIt = m_NetList.begin(); netIt != m_NetList.end(); netIt++ )
    {
        RemoveDevices ( i );
        i++;
    }

    //destroy the ini file object
    if ( m_IniFile != NULL )
    {
        delete m_IniFile;
        m_IniFile = NULL;
    }

    if (m_Master != 0x0)
    {
        delete m_Master;
        m_Master = 0x0;
    }

}

///////////////////////////////////////////////////
//              ACQUIRENET
///////////////////////////////////////////////////
bool COneWireNet::AcquireNet ( int netIndex, const char *port, int portnum )
{
    int portH = -1, counter = 0, maxCount = 0;
    bool retval = false;
    unsigned int devIndex;


    if ( (netIndex < 0) || (netIndex > m_NetList.size()) )
    {
        return retval;
    }

    //Write in the master if we are wireless or not
    if ( ( m_NetList[netIndex].isWl ) || ( m_NetList[netIndex].isOverIp ) )
    {
//         cout << "NET "<<netIndex+1<<" Wireless" << endl;
        m_Master->SetWireless ( 1 );
    }
    else
    {
//         cout << "NET "<<netIndex+1<<" NON Wireless" << endl;
        m_Master->SetWireless ( 0 );
    }

    m_Master->SetNetDelay ( m_NetList[netIndex].netDelay );

    //If the net is wireless add more retries and reprogram the radio modem
    if ( m_NetList[netIndex].isWl )
    {
        if ( m_DoDebug )
        {
            cout << "Changing RF Address..." << endl;
        }

//         cout << "NET "<<netIndex+1<<" cambio programmazione dispositivo" << endl;
//         cout.flush();
        if ( ChangeWLAddr ( netIndex ) )
        {
            maxCount = WL_RETRIES_COUNT;
        }
        else
        {
            //Some error occurred in reprogramming the device, exit
            PushError ( AFOERROR_UNABLE_TO_REPROGRAM_RADIOMODEM, netIndex + 1 );
            return false;
        }
    }
    else if ( m_NetList[netIndex].isOverIp )
    {
        if ( m_DoDebug )
        {
            cout << "Changing IP Address..." << endl;
        }

//         cout << "NET "<<netIndex+1<<" cambio programmazione dispositivo" << endl;
//         cout.flush();

        if ( ChangeIPAddr ( netIndex ) )
        {
            maxCount = WL_RETRIES_COUNT;
        }
        else
        {
            //Some error occurred in reprogramming the device, exit
            PushError ( AFOERROR_UNABLE_TO_REPROGRAM_ETHERNET_CONV, netIndex + 1 );
            return false;
        }
    }
    else
    {
        maxCount = 1;
    }

    //Controllo doppio se la rete è acquisita perchè se il master la stacca per errori la net perde l'handler della porta
    if ( ( m_NetList[netIndex].isAcquired ) && (m_Master->IsPortOpen(m_NetList[netIndex].portHandler) ) )
    {
        //Controllare la porta ad ogni ciclo mi costa un sacco di tempo -> faccio un polling ogni tanto
        if (m_NetList[netIndex].recheckMaster){
            //Testo il master... a fare così perdo un sacco di tempo....
            if (m_NetList[netIndex].master->DS2480Detect ( m_NetList[netIndex].portHandler ))
            {
                m_NetList[netIndex].recheckMaster = false;
                return true;
            }
            else
            {
                //Ahi... il master sembra non esserci... stacco la rete e continuo
                ReleaseNet(netIndex);
            }
        }
        else {
            return true;
        }
    }

    //Controllo se la porta della NET è già aperta da qualcun'altro
    for (unsigned int i = 0; i < m_NetList.size(); i++)
    {
        if ((!m_NetList[i].netPortName.compare(m_NetList[netIndex].netPortName)) && (i != netIndex))
        {
            //ho trovato una NET che ha la stessa porta di quella che sto acquisendo: controllo se e' gia' aperta e se si la uso
            if (m_NetList[i].isAcquired && m_Master->IsPortOpen(m_NetList[netIndex].portHandler))
            {
                portH = m_NetList[i].portHandler;
                m_NetList[netIndex].portHandler = portH;
                m_NetList[netIndex].isAcquired = true;

                //Update the port number in all devices
                for ( devIndex = 0; devIndex < m_NetList[netIndex].deviceList.size(); devIndex++ )
                {
                    m_NetList[netIndex].deviceList[devIndex]->SetPortNum ( portH );
                }

                retval = true;
                break;
            }
        }
    }

    while (( counter < maxCount ) && (!retval))
    {
        if ( portnum < 0 )
        {
//             cout << "NET "<<netIndex+1<<" portnum<0, acquisisco porta: "<< (char*)port << endl;
            //portH = m_NetList[netIndex].master->owAcquireEx ( ( char* ) port );
            portH = m_Master->owAcquireEx ( ( char* ) port );

        }
        else
        {
            //FIXME:Riaggiustare questo: forse fa dei casini perche' owAcquire non ricordo cosa torna
            //port number specified, call owAcquire
            //portH = m_NetList[netIndex].master->owAcquire ( portnum, ( char* ) port );
            portH = m_Master->owAcquire ( portnum, ( char* ) port );
        }

        //Update net list
        if ( portH >= 0 )
        {
            m_NetList[netIndex].portHandler = portH;
            m_NetList[netIndex].isAcquired = true;

            //Update the port number in all devices
            for ( devIndex = 0; devIndex < m_NetList[netIndex].deviceList.size(); devIndex++ )
            {
                m_NetList[netIndex].deviceList[devIndex]->SetPortNum ( portH );
            }

            retval = true;

            break;
        }
        else
        {
            //Update the counter
            counter++;
        }
    }//While

    //Update error condition
    if ( !retval )
    {
        m_NetList.at ( netIndex ).nOfErrors++;

        if ( m_DoDebug )
        {
            cout << "IMPOSSIBILE aprire la NET" << netIndex+1 << endl;
            cout.flush();
        }
    }
    else
    {
        m_NetList.at ( netIndex ).nOfErrors = 0;
    }

    return retval;
}

///////////////////////////////////////////////////
//              ACQUIRENET (2)
///////////////////////////////////////////////////
bool COneWireNet::AcquireNet ( int netIndex )
{
    CString portName = "", net = "";

    portName = m_NetList[netIndex].netPortName;

    //Acquire the net
    return AcquireNet ( netIndex, portName.c_str() );

}

///////////////////////////////////////////////////
//              RELEASENET
///////////////////////////////////////////////////
bool COneWireNet::ReleaseNet ( int netIndex )
{
    int counter = 0;

    try
    {
        if ( netIndex < m_NetList.size() )
        {
            if ( m_NetList.at ( netIndex ).isAcquired )
            {
                //The net exists, release it
                while ( counter < 5 )
                {
                    //Try up to 5 times to close the port
                    if ( !m_NetList.at ( netIndex ).master->owRelease ( m_NetList.at ( netIndex ).portHandler ) )
                    {
                        //Port correctly released
                        m_NetList.at ( netIndex ).isAcquired = false;
                        if ( m_DoDebug )
                        {
                            cout << "NET Chiusa: " << netIndex+1 << endl;
                        }

                        //Qui cerco se ci sono altre NET con la stessa porta e le marco come chiuse
                        for (unsigned int i = 0; i < m_NetList.size(); i++)
                        {
                            if ((m_NetList.at(i).netPortName == m_NetList.at(netIndex).netPortName) && (i != netIndex))
                            {
                                //ho trovato una NET che ha la stessa porta di quella che sto rilaciando: la flaggo come non acquisita
                                m_NetList.at(i).isAcquired = false;
                                if ( m_DoDebug )
                                {
                                    cout << "NET Chiusa: " << i+1 << endl;
                                }
                            }
                        }

                        return true;
                    }
                    counter ++;

                    if ( m_DoDebug )
                        cout << "Error while releasing serial port " << m_NetList.at(netIndex).netPortName << " for NET"<<netIndex+1<<endl;

                    //TBM
                    //msDelay(1000);
                }
            }
        }
        else
        {
            if ( m_DoDebug )
                cout << "CONEWIRENET: ReleaseNet -- Indice fuori dai limiti" << endl;
        }
    }
    catch ( exception &e )
    {
        if ( m_DoDebug )
        {
            cout << "Si e' verificato un errore durante la chiusura della NET : "<< netIndex + 1<< " errore: " << e.what() << endl;
        }
    }

    return false;
}

///////////////////////////////////////////////////
//              ADDNET                       //
///////////////////////////////////////////////////
bool COneWireNet::AddNet()
{
    T_Net newNet;

    newNet.portHandler = 0;
    newNet.isWl = 0;
    newNet.wlAddr[0] = 0;
    newNet.wlAddr[1] = 0;
    newNet.isAcquired = false;
    newNet.correctlySet = false;
    newNet.areAlarmsEnabled = true;
    newNet.areTempAlarmsSW = false;
    newNet.timerID = -1;
    newNet.netTimer = m_Timer;
    newNet.isTimerOn = false;
    newNet.isOverIp = false;
    newNet.hasTempDevices = false;
    newNet.hasDIDOs = false;
    newNet.hasPIDs = false;
    newNet.hasHums = false;
    newNet.hasAIAOs = false;
    newNet.hasDIDOs = false;
    newNet.hasButtonDIDOs = false;
    newNet.hasAdvCtrls = false;
    newNet.master = m_Master;
    newNet.hasHotelRooms = false;
    newNet.hasAccessControl = false;
    newNet.hasIButtonReader = false;
    newNet.hasCoordinators = false;
    newNet.isVirtual=false;
    newNet.tempConversionLaunched = false;
    newNet.timeOfLastTempConversion = 0;
    newNet.saveDigitalState = true;
    newNet.recheckMaster = true;

    newNet.temperatureClock = mainClockManager.GetClock();
    newNet.didoClock = mainClockManager.GetClock();
    
    m_NetList.push_back ( newNet );

    return true;
}

///////////////////////////////////////////////////
//             SENDRESET                      //
///////////////////////////////////////////////////
bool COneWireNet::SendReset ( int netIndex )
{
    if ( CheckNetIndex ( netIndex ) )
    {
        //Send a reset on the line
        return m_NetList[netIndex].master->owTouchReset ( m_NetList[netIndex].portHandler );
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////
//             REMOVELASTNET                    //
///////////////////////////////////////////////////
bool COneWireNet::RemoveLastNet( )
{
    int nOfDev;

    if ( m_NetList.size() > 0 )
    {

        //Get the total number of devices to check if everithing has been removed
        nOfDev = m_NetList[m_NetList.size()-1].deviceList.size();

        //Start by deleting all the device objects
        if ( nOfDev == RemoveDevices ( m_NetList.size()-1 ) )
        {
            m_NetList.pop_back();
            return true;
        }
        else
        {
            //Something bad happened...
            return false;
        }
    }
    else
    {
        return false;
    }

}

///////////////////////////////////////////////////
//             REMOVEDEVICES            //
///////////////////////////////////////////////////
int COneWireNet::RemoveDevices ( int netIndex )
{
    int nDev = 0;
    vector<CVDevice*>::iterator devIt;
    vector<CVController*>::iterator ctrlIt;
    vector<CVController*>::iterator afoIt;

    try
    {
        //Delete all the objects from the given net
        for ( devIt=m_NetList.at ( netIndex ).deviceList.begin(); devIt!=m_NetList.at ( netIndex ).deviceList.end(); devIt++ )
        {
            delete *devIt;
        }

        for ( ctrlIt=m_NetList.at ( netIndex ).CtrlList.begin(); ctrlIt!=m_NetList.at ( netIndex ).CtrlList.end(); ctrlIt++ )
        {
            delete *ctrlIt;
        }

        for ( afoIt=m_NetList.at ( netIndex ).blackList.begin(); afoIt!=m_NetList.at ( netIndex ).blackList.end(); afoIt++ )
        {
            delete *afoIt;
        }
    }
    catch ( exception &e )
    {
        cout << "An Error occurred in deleting all the objects for NET: " << netIndex << " Error : " << e.what() << endl;
    }

    return nDev;

}


///////////////////////////////////////////////////
//             INITDEVICES            //
///////////////////////////////////////////////////
bool COneWireNet::InitDevices ( int netIndex )
{
    CString buffer = "", net = "";
    int nOfDev = 0;
    int devIndex = 0;

    //Prepare the net
    net="NET";
    net += ( netIndex+1 );

    //Create the devices
    //Get the total number of devices
    nOfDev = m_IniFile->GetInt ( Config_Strings[CONF_NETNUMBEROFDEVICES], net );

    if ( m_DoDebug )
    {
        cout << "OneWireNet: Numero dispositivi: "<< nOfDev <<endl;
    }

    if ( nOfDev == INT_MIN )
    {
        //Error in number of devices
        if ( m_DoDebug )
        {
            cout<<"CONEWIRENET:Errore!! Numero dispositivi mancante nel file INI" << endl;
        }

        return false;
    }


    if ( m_DoDebug )
    {
        cout << "OneWireNet: Inizializzo i dispositivi"<<endl;cout.flush();
    }

    for ( int i = 0; i < nOfDev; i++ )
    {
        char deviceName[16] ; //First device to search for

        sprintf ( deviceName, "Device%02d", i+1 );

        //Get the configuration string for the device and create the object
        buffer = m_IniFile->GetString ( deviceName, net );

        if ( buffer.size() == 0 )
        {
            if ( m_DoDebug )
            {
                cout << "CONEWIRENET:Errore!!! "<<deviceName<< " non trovato nel file INI" <<endl;
            }

            return false;
        }

        if ( CreateDevice ( netIndex, i, ( char* ) buffer.c_str() ) )
        {
            devIndex++;
        }
    }

    if ( !ConnectControllers ( netIndex ) )
    {
        return false;
    }

    return true;

//     if ( ConnectCoordinators ( netIndex ) )
//     {
//         return true;{{
//     }
//     else
//     {
//         return false;
//     }
}


///////////////////////////////////////////////////
//             CREATEDEVICE                //
///////////////////////////////////////////////////
bool COneWireNet::CreateDevice ( int netIndex, int objectNumber, const char* configString )
{
    char name[32];
    bool retval = false, isDevice = true;

    int lastDevice = m_NetList[netIndex].deviceList.size();
    int lastController = m_NetList[netIndex].CtrlList.size();

    int address2Check = 0;

    m_IniLib.GetConfigParamString ( configString, "NAME", name, 32, "NotFound" );

    m_IniLib.GetConfigParamInt ( configString, "ADDR", &address2Check, -1 );
    if ( !CheckAddressValidity ( address2Check ) )
    {
        if ( m_DoDebug )
        {
            cout << "Attenzione!! Indirizzo duplicato nel Device"<<objectNumber + 1<<endl;cout.flush();
        }

        return retval;
    }

    if ( ( !strcasecmp ( name, "NotFound" ) ) || ( m_NetList[netIndex].deviceList.size() == MAX_NUM_DEV_PER_NET ) )
    {
        //Device not found
        if ( m_DoDebug )
        {
            cout << "CONEWIRENET:Errore!! Device "<<objectNumber + 1<<", parametro NAME Non trovato." << endl;
        }
        return retval;
    }

    if ( !strncasecmp ( name, Device_strings[DEV_DS18B20], strlen ( Device_strings[DEV_DS18B20] ) ) )
    {
        m_NetList[netIndex].deviceList.push_back ( new CDS18X20 ( 0, m_NetList[netIndex].master, 'B', configString ) );
        retval = true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_DS18S20], strlen ( Device_strings[DEV_DS18S20] ) ) )
    {
        m_NetList[netIndex].deviceList.push_back ( new CDS18X20 ( 0, m_NetList[netIndex].master, 'S', configString ) );
        retval = true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_DS2438], strlen ( Device_strings[DEV_DS18S20] ) ) )
    {
        CDS2438* pippo = new CDS2438 ( 0, m_NetList[netIndex].master, configString );
        m_NetList[netIndex].deviceList.push_back ( pippo );
        retval = true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_DS2890], strlen ( Device_strings[DEV_DS2890] ) ) )
    {
        m_NetList[netIndex].deviceList.push_back ( new CDS2890 ( 0, m_NetList[netIndex].master, configString ) );
        retval = true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_DS2751], strlen ( Device_strings[DEV_DS2751] ) ) )
    {
        m_NetList[netIndex].deviceList.push_back ( new CDS2751 ( 0, m_NetList[netIndex].master, configString ) );
        retval = true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_DS2408], strlen ( Device_strings[DEV_DS2408] ) ) )
    {
        m_NetList[netIndex].deviceList.push_back ( new CDS2408 ( 0, m_NetList[netIndex].master, configString ) );
        retval = true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_HUMIDITY], strlen ( Device_strings[DEV_HUMIDITY] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CHumController ( configString, m_Timer ) );
        m_NetList[netIndex].hasHums=true;
        retval = true;
        isDevice = false;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_TEMPCTRLHYST], strlen ( Device_strings[DEV_TEMPCTRLHYST] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CTempCtrlHyst ( configString, m_Timer ) );

        m_NetList[netIndex].hasTempDevices = true;
        retval = true;
        isDevice = false;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_AIAO], strlen ( Device_strings[DEV_AIAO] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CAnalogIO ( configString, m_Timer ) );
        isDevice = false;
        m_NetList[netIndex].hasAIAOs = true;
        retval = true;

    }
    else if ( !strncasecmp ( name, Device_strings[DEV_THU], strlen ( Device_strings[DEV_THU] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CThu ( configString ) );
        isDevice = false;
        m_NetList[netIndex].hasAIAOs = true;
        retval = true;

    }
    else if ( !strncasecmp ( name, Device_strings[DEV_PIDSIMPLE], strlen ( Device_strings[DEV_PIDSIMPLE] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CPIDSimple ( configString, m_Timer ) );

        m_NetList[netIndex].hasPIDs = true;

        //It is a controller, skip the time2error
        isDevice = false;
        retval = true;

    }
    else if ( !strncasecmp ( name, Device_strings[DEV_PIDLMD], strlen ( Device_strings[DEV_PIDLMD] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CPIDLMD ( configString, m_Timer ) );

        m_NetList[netIndex].hasPIDs = true;

        //It is a controller, skip the time2error
        isDevice = false;
        retval = true;

    }
    else if ( !strncasecmp ( name, Device_strings[DEV_DIDO], strlen ( Device_strings[DEV_DIDO] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CDigitalIO ( configString, m_Timer ) );

        //It is a controller, skip the time2error
        m_NetList[netIndex].hasDIDOs=true;
        isDevice = false;
        retval = true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_TEMPCTRL], strlen ( Device_strings[DEV_TEMPCTRL] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CTempCtrl ( configString, m_Timer ) );

        //It is a controller, skip the time2error
        isDevice = false;
        retval = true;
        m_NetList[netIndex].hasTempDevices = true;

    }
    else if ( !strncasecmp ( name, Device_strings[DEV_REMOTEDIDO], strlen ( Device_strings[DEV_REMOTEDIDO] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CRemoteDIDO ( configString, m_Timer ) );

        //It is a controller
        isDevice = false;
        retval = true;
        m_NetList[netIndex].hasButtonDIDOs = true;
        m_NetList[netIndex].hasDIDOs=true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_PUMPCTRL], strlen ( Device_strings[DEV_PUMPCTRL] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CPumpController ( configString, m_Timer ) );

        //It is a coordinator
        m_NetList[netIndex].hasCoordinators=true;
        isDevice = false;
        retval = true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_VLVCTRL], strlen ( Device_strings[DEV_VLVCTRL] ) ) )
    {
#ifndef USE_ADV_VLV
        m_NetList[netIndex].CtrlList.push_back ( new CNTHVLV ( configString, m_Timer ) );
#else
        m_NetList[netIndex].CtrlList.push_back ( new CNTHVLV_ADV ( configString, m_Timer ) );
#endif

        //It is a controller
        m_NetList[netIndex].hasAdvCtrls=true;
        isDevice = false;
        retval = true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_AFOVLV], strlen ( Device_strings[DEV_AFOVLV] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CNTHVLV2 ( configString, m_Timer ) );

        //It is a controller
        m_NetList[netIndex].hasAdvCtrls=true;
        isDevice = false;
        retval = true;
    }
    else if ( !strncasecmp ( name, Device_strings[ DEV_AFOVLV_VAV], strlen ( Device_strings[DEV_AFOVLV_VAV] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CNTHVLV2_VAV ( configString, m_Timer ) );

        //It is a controller
        m_NetList[netIndex].hasAdvCtrls=true;
        isDevice = false;
        retval = true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_MGC], strlen ( Device_strings[DEV_MGC] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CNTHMGC ( configString, m_Timer ) );

        //It is a controller
        m_NetList[netIndex].hasAdvCtrls=true;
        m_NetList[netIndex].hasHotelRooms=true;
        isDevice = false;
        retval = true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_CNT], strlen ( Device_strings[DEV_CNT] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CNTH_CNT ( configString, m_Timer ) );

        //It is a controller
        m_NetList[netIndex].hasAdvCtrls=true;
        isDevice = false;
        retval = true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_BUTTONCTRL], strlen ( Device_strings[DEV_BUTTONCTRL] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CButtonController ( configString, m_Timer ) );
        isDevice = false;
        retval = true;
        m_NetList[netIndex].hasButtonDIDOs = true;
        m_NetList[netIndex].hasDIDOs = true;

    }
    else if ( !strncasecmp ( name, Device_strings[DEV_STEPDIGITALOUT], strlen ( Device_strings[DEV_STEPDIGITALOUT] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CStepDigitalOut ( configString, m_Timer ) );
        isDevice = false;
        retval = true;
        m_NetList[netIndex].hasDIDOs=true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_TAGCTRL], strlen ( Device_strings[DEV_TAGCTRL] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CTAGControl ( configString, m_Timer ) );
        isDevice = false;
        retval = true;
        m_NetList[netIndex].hasDIDOs=true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_UTACTRL], strlen ( Device_strings[DEV_UTACTRL] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CUtaCtrl ( configString, m_Timer ) );
        m_NetList[netIndex].hasPIDs = true;
        //It is a controller
        m_NetList[netIndex].hasAdvCtrls=true;

        isDevice = false;
        retval = true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_DI2AO], strlen ( Device_strings[DEV_DI2AO] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CDI2AO ( configString, m_Timer ) );

        //It is a controller
        m_NetList[netIndex].hasAdvCtrls=true;
        isDevice = false;
        retval = true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_ACC], strlen ( Device_strings[DEV_ACC] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CNTH_ACC ( configString, m_Timer ) );

        //It is a controller
        m_NetList[netIndex].hasAdvCtrls=true;
        m_NetList[netIndex].hasAccessControl = true;
        isDevice = false;
        retval = true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_FULLUTACTRL], strlen ( Device_strings[DEV_FULLUTACTRL] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CFullUTACtrl ( configString, m_Timer ) );

        //It is a controller
        m_NetList[netIndex].hasCoordinators=true;
        isDevice = false;
        retval = true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_FULLUTACTRL_2], strlen ( Device_strings[DEV_FULLUTACTRL_2] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CFullUTACtrl2 ( configString, m_Timer ) );

        //It is a controller
        m_NetList[netIndex].hasCoordinators=true;
        isDevice = false;
        retval = true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_CLIMATICOORD], strlen ( Device_strings[DEV_CLIMATICOORD] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CClimaticCurve ( configString, m_Timer ) );

        //It is a controller
        m_NetList[netIndex].hasCoordinators=true;
        isDevice = false;
        retval = true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_ALARMCTRL], strlen ( Device_strings[DEV_ALARMCTRL] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new AlarmCoordinator ( configString, m_Timer ) );

        //It is a controller
        m_NetList[netIndex].hasCoordinators=true;
        isDevice = false;
        retval = true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_CHOVER], strlen ( Device_strings[DEV_CHOVER] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new ChangeOverCoord ( configString, m_Timer ) );

        //It is a controller
        m_NetList[netIndex].hasCoordinators=true;
        isDevice = false;
        retval = true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_TIMEMARKER], strlen ( Device_strings[DEV_TIMEMARKER] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CTimeMarkerCtrl ( configString, m_Timer ) );

        //It is a controller
        m_NetList[netIndex].hasAdvCtrls=true;
        isDevice = false;
        retval = true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_FLOORCOORD_2], strlen ( Device_strings[DEV_FLOORCOORD_2] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CFloorCoord2 ( configString, m_Timer ) );

        //It is a controller
        m_NetList[netIndex].hasCoordinators=true;
        isDevice = false;
        retval = true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_C3POINT], strlen ( Device_strings[DEV_C3POINT] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new C3PointCtrl ( configString, m_Timer ) );

        //It is a controller
        m_NetList[netIndex].hasPIDs=true;
        isDevice = false;
        retval = true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_IBUTT_RDR], strlen ( Device_strings[DEV_IBUTT_RDR] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CIButtonReader ( configString, m_Timer ) );

        //It is a controller
        isDevice = false;
        retval = true;
    }
    else if ( !strncasecmp ( name, Device_strings[DEV_BLOCKCOORD], strlen ( Device_strings[DEV_BLOCKCOORD] ) ) )
    {
        m_NetList[netIndex].CtrlList.push_back ( new CBlockCoordinator ( configString, m_Timer ) );

        //It is a controller
        m_NetList[netIndex].hasCoordinators=true;
        isDevice = false;
        retval = true;
    }
    else
    {
        retval = false;
    }

    if ( ( retval ) && ( isDevice ) )
    {
        //Update the internal reference numbers so the device knows its list number
        m_NetList[netIndex].deviceList[lastDevice]->SetReferenceNumbers ( objectNumber + 1, netIndex + 1 );
        m_NetList[netIndex].deviceList[lastDevice]->SetErrorHandler ( m_AfoErrorHandler );
        m_NetList[netIndex].deviceList[lastDevice]->SetCRCUtil ( &m_CRCUtil );
        m_NetList[netIndex].deviceList[lastDevice]->SetEngPtr ( m_EnginePtr );
        m_NetList[netIndex].deviceList[lastDevice]->SetNetPtr ( (void*)this );
        
    }
    else if ( ( !isDevice ) && retval )
    {
        m_NetList[netIndex].CtrlList[lastController]->SetReferenceNumbers ( objectNumber + 1, netIndex + 1 );
        m_NetList[netIndex].CtrlList[lastController]->SetErrorHandler ( m_AfoErrorHandler );
        m_NetList[netIndex].CtrlList[lastController]->SetEngPtr ( m_EnginePtr );
        m_NetList[netIndex].CtrlList[lastController]->SetNetPtr ( (void*)this );
        //Ho fatto un bel bug nel parsing della riga di configurazione: se TIMERID:0 allora
        //non inizializzo il timer dell'oggetto e quindi successivamente non posso abilitarlo da remoto
        //Forzo allora qui l'impostazione dell'oggetto timer
        m_NetList[netIndex].CtrlList[lastController]->SetTimer(m_Timer);
    }


    if ( m_DoDebug )
    {
        if ( retval )
        {
            cout << name << " Creato Correttamente" << endl;
        }
        else
        {
            cout << "ERRORE: " << name << " NON CREATO!!! " << endl;
        }
    }

    return retval;
}

///////////////////////////////////////////////////
//             CONNECTCONTROLLERS
///////////////////////////////////////////////////
bool COneWireNet::ConnectControllers ( int netIndex )
{
    vector<CVController*>::iterator ctrlIterator;
    e_DeviceType controllerType = DEV_NONE;
    int configNetIndex, configDevIndex;
    CString configDevString, configNETString;
    CString configString;
    bool retval = false;
    int inputIndex = -1, outputIndex = -1, channel = -1;
    CVDevice *inDevice, *outDevice;

    cout << "Inizio connessione ingressi/uscite dei controller..."<<endl;cout.flush();
    if ( m_NetList[netIndex].CtrlList.size() == 0 )
    {
        cout << "ERRORE!! Non è stato definito nessun controller, impossibile continuare" << endl;
    }

    for ( ctrlIterator = m_NetList[netIndex].CtrlList.begin(); ctrlIterator != m_NetList[netIndex].CtrlList.end(); ctrlIterator++ )
    {
        controllerType = ( *ctrlIterator )->GetControllerType();
        configDevIndex = ( *ctrlIterator )->GetConfigFileDevIndex();

        configNETString = "NET";
        configNetIndex = netIndex + 1;
        configNETString+=configNetIndex;
        configDevString = "Device";

        if ( configDevIndex < 10 )
        {
            configDevString+="0";
        }
        configDevString+=configDevIndex;
        configString = m_IniFile->GetString ( configDevString, configNETString );

        switch ( controllerType )
        {
            case DEV_HUMIDITY:
            {
                //Get Handle to input device
                m_IniLib.GetConfigParamInt ( configString.c_str(), "INPUT", &inputIndex, -1 );

                inputIndex = GetDeviceIndexByConfigNumber ( netIndex, inputIndex );

                if ( inputIndex > -1 )
                {
                    inDevice = m_NetList[netIndex].deviceList[inputIndex];

                    //Get handle to the output device
                    m_IniLib.GetConfigParamInt ( configString.c_str(), "OUTPUT", &outputIndex, -1 );
                    m_IniLib.GetConfigParamInt ( configString.c_str(), "CHANNEL", &channel, -1 );

                    //Adjust for internal settings: channels are internally numbered from 0 to 8
                    channel = channel - 1;

                    if ( ( inputIndex == -1 ) || ( inDevice == 0x0 ) || ( inDevice->GetFamNum() != DS2438_FN ) )
                    {
                        retval = false;
                    }
                    else
                    {
                        ( ( CHumController* ) ( *ctrlIterator ) )->SetInputDevice ( ( ( CDS2438* ) ( inDevice ) ) );
                        retval = true;


                        //Check if we have a full controller or not
                        if ( ( outputIndex > -1 ) && ( channel > -1 ) && ( channel < 8 ) )
                        {
                            //Set the output device
                            outputIndex = GetDeviceIndexByConfigNumber ( netIndex, outputIndex );

                            if ( ( outputIndex == -1 ) || ( m_NetList[netIndex].deviceList[outputIndex]->GetFamNum() != DS2408_FN ) )
                            {
                                retval = false;
                            }
                            else
                            {
                                outDevice = m_NetList[netIndex].deviceList[outputIndex];
                                retval = ( ( CHumController* ) ( *ctrlIterator ) )->SetOutputDevice ( ( ( CDS2408* ) ( outDevice ) ), channel );
                            }
                        }

                    }
                }
                else
                {
                    retval = false;
                }
            }
            ;break;
            case DEV_TEMPCTRLHYST:
            {
                //Get Handle to input device
                m_IniLib.GetConfigParamInt ( configString.c_str(), "INPUT", &inputIndex, -1 );

                inputIndex = GetDeviceIndexByConfigNumber ( netIndex, inputIndex );

                if ( inputIndex > -1 )
                {
                    inDevice = m_NetList[netIndex].deviceList[inputIndex];

                    //Get handle to the output device
                    m_IniLib.GetConfigParamInt ( configString.c_str(), "OUTPUT", &outputIndex, -1 );
                    m_IniLib.GetConfigParamInt ( configString.c_str(), "CHANNEL", &channel, -1 );

                    //Adjust for internal settings: channels are internally numbered from 0 to 8
                    channel = channel - 1;

                    if ( ( inputIndex == -1 ) || ( inDevice == 0x0 ) || ( ! ( ( inDevice->GetFamNum() != DS18S20_FN ) || ( inDevice->GetFamNum() != DS18B20_FN ) || ( inDevice->GetFamNum() != DS2438_FN ) ) ) )
                    {
                        retval = false;
                    }
                    else
                    {
                        ( ( CTempCtrlHyst* ) ( *ctrlIterator ) )->SetInputDevice ( inDevice );
                        retval = true;


                        //Check if we have a full controller or not
                        if ( ( outputIndex > -1 ) && ( channel > -1 ) && ( channel < 8 ) )
                        {
                            //Set the output device
                            outputIndex = GetDeviceIndexByConfigNumber ( netIndex, outputIndex );

                            if ( ( outputIndex == -1 ) || ( m_NetList[netIndex].deviceList[outputIndex]->GetFamNum() != DS2408_FN ) )
                            {
                                retval = false;
                            }
                            else
                            {
                                outDevice = m_NetList[netIndex].deviceList[outputIndex];

                                retval = ( ( CTempCtrlHyst* ) ( *ctrlIterator ) )->SetOutputDevice ( ( ( CDS2408* ) ( outDevice ) ), channel );
                            }
                        }

                    }
                }
                else
                {
                    retval = false;
                }
            }
            ;break;
            case DEV_AIAO:
            {
                //Get handle to the output device
                m_IniLib.GetConfigParamInt ( configString.c_str(), "INPUT", &inputIndex, -1 );

                if ( inputIndex > 0 )
                {
                    inputIndex = GetDeviceIndexByConfigNumber ( netIndex, inputIndex );
                    inDevice = m_NetList[netIndex].deviceList[inputIndex];
                    if ( ( inputIndex == -1 ) || ( inDevice == 0x0 ) || ( ! ( ( inDevice->GetFamNum() != DS2438_FN ) || ( inDevice->GetFamNum() != DS2890_FN ) ) ) )
                    {
                        retval = false;
                    }
                    else
                    {
                        ( ( CAnalogIO* ) ( *ctrlIterator ) )->SetInputDevice ( inDevice );
                        retval = true;
                    }
                }
                else
                {
                    retval = false;
                }
            }
            ;break;
            case DEV_THU:
            {
                //Get handle to the output device
                m_IniLib.GetConfigParamInt ( configString.c_str(), "INPUT", &inputIndex, -1 );

                if ( inputIndex > 0 )
                {
                    inputIndex = GetDeviceIndexByConfigNumber ( netIndex, inputIndex );
                    inDevice = m_NetList[netIndex].deviceList[inputIndex];
                    if ( ( inputIndex == -1 ) || ( inDevice == 0x0 ) || ( ! ( ( inDevice->GetFamNum() != DS2438_FN ) || ( inDevice->GetFamNum() != DS2890_FN ) ) ) )
                    {
                        retval = false;
                    }
                    else
                    {
                        ( ( CThu* ) ( *ctrlIterator ) )->SetInputDevice ( inDevice );
                        retval = true;
                    }
                }
                else
                {
                    retval = false;
                }
            }
            ;break;
            case DEV_PIDSIMPLE:
            {
                retval = (*ctrlIterator)->ConnectDevices(this);

            }
            ;break;
            case DEV_PIDLMD:
            {
                retval = (*ctrlIterator)->ConnectDevices(this);
            }
            ;break;
            case DEV_DIDO:
            {
                //Get Handle to the device
                m_IniLib.GetConfigParamInt ( configString.c_str(), "INPUT", &inputIndex, -1 );
                inputIndex = GetDeviceIndexByConfigNumber ( netIndex, inputIndex );

                if ( inputIndex > -1 )
                {
                    //Init the device pointer

                    inDevice = m_NetList[netIndex].deviceList[inputIndex];

                    if ( ( inputIndex == -1 ) || ( inDevice == 0x0 ) || ( ! ( ( inDevice->GetFamNum() != DS2408_FN ) || ( inDevice->GetFamNum() != DS2405_FN ) ) ) )
                    {
                        retval = false;
                    }
                    else
                    {

                        ( ( CDigitalIO* ) ( *ctrlIterator ) )->SetInputDevice ( inDevice );
                        retval = true;
                    }
                }
                else
                {
                    retval = false;
                }
            }
            ;break;
            case DEV_TEMPCTRL:
            {
                //Get Handle to the device
                m_IniLib.GetConfigParamInt ( configString.c_str(), "INPUT", &inputIndex, -1 );

                if ( inputIndex > -1 )
                {
                    //Init the device pointer
                    inputIndex = GetDeviceIndexByConfigNumber ( netIndex, inputIndex );

                    if (inputIndex == -1)
                    {
                        retval = false;
                    }
                    else
                    {
                    
                        inDevice = m_NetList[netIndex].deviceList[inputIndex];

                        if ( ( inDevice == 0x0 ) || ( ! ( ( inDevice->GetFamNum() != DS18S20_FN ) || ( inDevice->GetFamNum() != DS18B20_FN ) || ( inDevice->GetFamNum() != DS2438_FN ) ) ) )
                        {
                            retval = false;
                        }
                        else
                        {
                            CString label;
                            int nOfdevices, i;
                            bool retval2 = false;

                            ( ( CTempCtrl* ) ( *ctrlIterator ) )->SetInputDevice ( inDevice );

                            //Setup digital inputs
                            //Start from the defreeze
                            inputIndex = -1;
                            m_IniLib.GetConfigParamInt ( configString.c_str(), "INPUTDF", &inputIndex, -1 );
                            inputIndex = GetDeviceIndexByConfigNumber ( netIndex, inputIndex );

                            if ( inputIndex>0 )
                            {
                                ( ( CTempCtrl* ) ( *ctrlIterator ) )->m_DefreezeDevice->SetInputDevice ( m_NetList.at ( netIndex ).deviceList.at ( inputIndex ) );
                            }

                            m_IniLib.GetConfigParamInt ( configString.c_str(),"NIN",&nOfdevices,-1 );

                            if ( nOfdevices > 0 )
                            {
                                for ( i = 1; i < nOfdevices+1; i++ )
                                {
                                    label = "IN";
                                    label+=i;

                                    m_IniLib.GetConfigParamInt ( configString.c_str(), label.c_str(), &inputIndex, -1 );

                                    if ( inputIndex > 0 )
                                    {
                                        inputIndex = GetDeviceIndexByConfigNumber ( netIndex, inputIndex );

                                        if ( inputIndex > -1 )
                                        {
                                            inDevice = m_NetList.at ( netIndex ).deviceList.at ( inputIndex );

                                            if ( inDevice->GetFamNum() == DS2408_FN )
                                            {
                                                try
                                                {
                                                    retval = ( ( CTempCtrl* ) ( *ctrlIterator ) )->m_InVector.at ( i-1 )->SetInputDevice ( inDevice );
                                                }
                                                catch ( exception &e )
                                                {
                                                    cout << "Errore in creazione DEV_TEMPCTRL, DigitalIN: "<< e.what() << endl;
                                                }
                                            }
                                            else
                                            {
                                                retval = false;
                                                break;
                                            }
                                        }
                                        else
                                        {
                                            retval = false;
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        retval = false;
                                        break;
                                    }
                                }//FOR
                            }
                            else
                            {
                                //No digital input devices specified
                                retval = true;
                            }

                            //Setup digital outputs
                            m_IniLib.GetConfigParamInt ( configString.c_str(),"NOUT",&nOfdevices,-1 );
                            if ( nOfdevices > 0 )
                            {
                                for ( i = 1; i < nOfdevices+1; i++ )
                                {
                                    label = "OUT";
                                    label+=i;

                                    m_IniLib.GetConfigParamInt ( configString.c_str(), label.c_str(), &inputIndex, -1 );

                                    if ( inputIndex > 0 )
                                    {
                                        inputIndex = GetDeviceIndexByConfigNumber ( netIndex, inputIndex );

                                        if ( inputIndex > -1 )
                                        {
                                            inDevice = m_NetList.at ( netIndex ).deviceList.at ( inputIndex );

                                            if ( inDevice->GetFamNum() == DS2408_FN )
                                            {
                                                try
                                                {
                                                    retval2 = ( ( CTempCtrl* ) ( *ctrlIterator ) )->m_OutVector.at ( i-1 )->SetInputDevice ( inDevice );
                                                }
                                                catch ( exception &e )
                                                {
                                                    cout << "Errore in creazione DEV_TEMPCTRL, DigitalOUT: "<< e.what() << endl;
                                                }
                                            }
                                            else
                                            {
                                                retval2 = false;
                                                break;
                                            }
                                        }
                                        else
                                        {
                                            retval2 = false;
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        retval2 = false;
                                        break;
                                    }
                                }//FOR
                            }
                            else
                            {
                                retval2 = true;
                            }

                            retval = retval && retval2;
                        }
                    }

                }//IF inputIndex > -1
                else
                {
                    retval = false;
                }
            }
            ;break;
            case DEV_REMOTEDIDO:
            {
                //Get Handle to the device
                m_IniLib.GetConfigParamInt ( configString.c_str(), "INPUT", &inputIndex, -1 );

                if ( inputIndex > -1 )
                {
                    //Init the device pointer
                    inputIndex = GetDeviceIndexByConfigNumber ( netIndex, inputIndex );
                    inDevice = m_NetList[netIndex].deviceList[inputIndex];

                    if ( ( inputIndex == -1 ) || ( inDevice == 0x0 ) || ( inDevice->GetFamNum() != DS2408_FN ) )
                    {
                        retval = false;
                    }
                    else
                    {

                        ( ( CButtonController* ) ( *ctrlIterator ) )->SetInputDevice ( inDevice );
                        retval = true;
                    }
                }
                else
                {
                    retval = false;
                }

            };
            break;
            case DEV_PUMPCTRL:
            {
                int input1Index = 0, input2Index = 0, output1Index = 0, output2Index = 0;
                CVController *in1Device, *in2Device, *out1Device, *out2Device;

                //Get Handles to the devices
                m_IniLib.GetConfigParamInt ( configString.c_str(), "INPUTP1", &input1Index, -1 );
                m_IniLib.GetConfigParamInt ( configString.c_str(), "OUTPUTP1", &output1Index, -1 );
                m_IniLib.GetConfigParamInt ( configString.c_str(), "INPUTP2", &input2Index, -1 );
                m_IniLib.GetConfigParamInt ( configString.c_str(), "OUTPUTP2", &output2Index, -1 );

                if ( ( input1Index != -1 ) && ( output1Index != -1 ) && ( input2Index != -1 ) && ( output2Index != -1 ) )
                {
                    //There is a PID LMD controller, init the pointers
                    input1Index = GetCtrlIndexByConfigNumber ( netIndex, input1Index );
                    output1Index = GetCtrlIndexByConfigNumber ( netIndex, output1Index );
                    input2Index = GetCtrlIndexByConfigNumber ( netIndex, input2Index );
                    output2Index = GetCtrlIndexByConfigNumber ( netIndex, output2Index );

                    in1Device = m_NetList[netIndex].CtrlList[input1Index];
                    out1Device = m_NetList[netIndex].CtrlList[output1Index];
                    in2Device = m_NetList[netIndex].CtrlList[input2Index];
                    out2Device = m_NetList[netIndex].CtrlList[output2Index];

                    //Recheck arguments
                    if ( ( input1Index == -1 ) || ( input2Index == -1 ) || ( output1Index == -1 ) || ( output2Index == -1 ) || ( in1Device == 0x0 ) || ( in2Device == 0x0 ) || ( out1Device == 0x0 ) || ( out2Device == 0x0 ) ||
                            ( in1Device->GetControllerType() != DEV_DIDO ) || ( out1Device->GetControllerType() != DEV_DIDO ) ||
                            ( in2Device->GetControllerType() != DEV_DIDO ) || ( out2Device->GetControllerType() != DEV_DIDO ) )
                    {
                        retval = false;
                    }
                    else
                    {
                        ( ( CPumpController* ) ( *ctrlIterator ) )->SetIOs ( ( CDigitalIO* ) in1Device, ( CDigitalIO* ) in2Device, ( CDigitalIO* ) out1Device, ( CDigitalIO* ) out2Device );

                        retval = true;
                    }
                }
                else
                {
                    retval = false;
                }
            };
            break;
            case DEV_VLVCTRL:
            {
                //Get Handle to the device
                m_IniLib.GetConfigParamInt ( configString.c_str(), "INPUT", &inputIndex, -1 );

                if ( inputIndex > -1 )
                {
                    //Init the device pointer
                    inputIndex = GetDeviceIndexByConfigNumber ( netIndex, inputIndex );
                    inDevice = m_NetList[netIndex].deviceList[inputIndex];

                    if ( ( inputIndex == -1 ) || ( inDevice == 0x0 ) || ( inDevice->GetFamNum() != DS2751_FN ) )
                    {
                        retval = false;
                    }
                    else
                    {
#ifndef USE_ADV_VLV
                        ( ( CNTHVLV* ) ( *ctrlIterator ) )->SetInputDevice ( ( CDS2751* ) inDevice );
#else
                        ( ( CNTHVLV_ADV* ) ( *ctrlIterator ) )->SetInputDevice ( ( CDS2751* ) inDevice );
#endif
                        retval = true;
                    }
                }
                else
                {
                    retval = false;
                }
            }
            ;break;
            case DEV_AFOVLV:
            {
                //Get Handle to the device
                m_IniLib.GetConfigParamInt ( configString.c_str(), "INPUT", &inputIndex, -1 );

                if ( inputIndex > -1 )
                {
                    //Init the device pointer
                    inputIndex = GetDeviceIndexByConfigNumber ( netIndex, inputIndex );
                    inDevice = m_NetList[netIndex].deviceList[inputIndex];

                    if ( ( inputIndex == -1 ) || ( inDevice == 0x0 ) || ( inDevice->GetFamNum() != DS2751_FN ) )
                    {
                        retval = false;
                    }
                    else
                    {

                        ( ( CNTHVLV2* ) ( *ctrlIterator ) )->SetInputDevice ( ( CDS2751* ) inDevice );
                        
                        retval = ( ( CNTHVLV2* ) ( *ctrlIterator ) )->ConnectControllers(this, configString.c_str());
                    }
                }
                else
                {
                    retval = false;
                }
            }
            ;break;
            case DEV_AFOVLV_VAV:
            {
                //Get Handle to the device
                m_IniLib.GetConfigParamInt ( configString.c_str(), "INPUT", &inputIndex, -1 );

                if ( inputIndex > -1 )
                {
                    //Init the device pointer
                    inputIndex = GetDeviceIndexByConfigNumber ( netIndex, inputIndex );
                    inDevice = m_NetList[netIndex].deviceList[inputIndex];

                    if ( ( inputIndex == -1 ) || ( inDevice == 0x0 ) || ( inDevice->GetFamNum() != DS2751_FN ) )
                    {
                        retval = false;
                    }
                    else
                    {

                        ( ( CNTHVLV2_VAV* ) ( *ctrlIterator ) )->SetInputDevice ( ( CDS2751* ) inDevice );
                        
                        retval = ( ( CNTHVLV2_VAV* ) ( *ctrlIterator ) )->ConnectControllers(this, configString.c_str());
                    }
                }
                else
                {
                    retval = false;
                }
            }
            ;break;
            case DEV_BUTTONCTRL:
            {
                //Get Handle to input device
                m_IniLib.GetConfigParamInt ( configString.c_str(), "INPUT", &inputIndex, -1 );

                inputIndex = GetDeviceIndexByConfigNumber ( netIndex, inputIndex );

                if ( inputIndex > -1 )
                {
                    inDevice = m_NetList[netIndex].deviceList[inputIndex];

                    if ( ( inputIndex == -1 ) || ( inDevice == 0x0 ) || ( ! ( ( inDevice->GetFamNum() != DS2408_FN ) || ( inDevice->GetFamNum() != DS2405_FN ) ) ) )
                    {
                        retval = false;
                    }
                    else
                    {
                        char buffer[255];
                        memset ( buffer, 0x0, 255*sizeof ( char ) );

                        ( ( CButtonController* ) ( *ctrlIterator ) )->SetInputDevice ( inDevice );

                        //Get handle to the output device
                        m_IniLib.GetConfigParamInt ( configString.c_str(), "OUTPUT", &outputIndex, -1 );
                        m_IniLib.GetConfigParamString ( configString.c_str(), "OUTPUT", buffer, 255, "NA" );

                        //Check if the output is to all devices
                        if ( !strcasecmp ( buffer, "ALL" ) )
                        {
                            outputIndex = -1;
                        }

                        //Check if we have a full controller or not
                        if ( outputIndex > -1 )
                        {
                            //Set the output device
                            outputIndex = GetDeviceIndexByConfigNumber ( netIndex, outputIndex );

                            if ( ( outputIndex == -1 ) || ( ! ( ( m_NetList[netIndex].deviceList[outputIndex]->GetFamNum() != DS2408_FN ) || ( m_NetList[netIndex].deviceList[outputIndex]->GetFamNum() != DS2405_FN ) ) ) )
                            {
                                retval = false;
                            }
                            else
                            {
                                outDevice = m_NetList[netIndex].deviceList[outputIndex];

                                ( ( CButtonController* ) ( *ctrlIterator ) )->SetOutputDevice ( outDevice );
                                retval = true;
                            }
                        }
                        else if ( ( ! ( ( CVMultiDIDO* ) ( *ctrlIterator ) )->IsRemoted() ) && ( ! ( ( CButtonController* ) ( *ctrlIterator ) )->m_IsJolly ) )
                        {
                            cout << "Attenzione !! l'oggetto "<< Device_strings[DEV_BUTTONCTRL] << " NON ha parametri per l'uscita" << endl;
                        }
                        else
                        {
                            retval = true;
                        }
                    }
                }
                else
                {
                    retval = false;
                }

            }
            ;break;
            case DEV_STEPDIGITALOUT:
            {
                //Get Handle to input device
                m_IniLib.GetConfigParamInt ( configString.c_str(), "INPUT", &inputIndex, -1 );

                inputIndex = GetDeviceIndexByConfigNumber ( netIndex, inputIndex );

                if ( inputIndex > -1 )
                {
                    inDevice = m_NetList[netIndex].deviceList[inputIndex];

                    //Get handle to the output device
                    m_IniLib.GetConfigParamInt ( configString.c_str(), "OUTPUT", &outputIndex, -1 );

                    if ( ( inputIndex == -1 ) || ( inDevice == 0x0 ) || ( ! ( ( inDevice->GetFamNum() != DS2408_FN ) || ( inDevice->GetFamNum() != DS2405_FN ) ) ) )
                    {
                        retval = false;
                    }
                    else
                    {
                        ( ( CStepDigitalOut* ) ( *ctrlIterator ) )->SetInputDevice ( inDevice );

                        //Check if we have a full controller or not
                        if ( outputIndex > -1 )
                        {
                            //Set the output device
                            outputIndex = GetDeviceIndexByConfigNumber ( netIndex, outputIndex );

                            if ( ( outputIndex == -1 ) || ( ! ( ( m_NetList[netIndex].deviceList[outputIndex]->GetFamNum() != DS2408_FN ) || ( m_NetList[netIndex].deviceList[outputIndex]->GetFamNum() != DS2405_FN ) ) ) )
                            {
                                retval = false;
                            }
                            else
                            {
                                outDevice = m_NetList[netIndex].deviceList[outputIndex];
                                ( ( CStepDigitalOut* ) ( *ctrlIterator ) )->SetOutputDevice ( outDevice );
                                ( ( CVMultiDIDO* ) ( *ctrlIterator ) )->SetRemoted ( false );
                                retval = true;
                            }
                        }

                    }
                }
                else
                {
                    retval = false;
                }

            }
            ;break;
            case DEV_TAGCTRL:
            {
                //Get Handle to input device
                m_IniLib.GetConfigParamInt ( configString.c_str(), "INPUT", &inputIndex, -1 );

                inputIndex = GetDeviceIndexByConfigNumber ( netIndex, inputIndex );

                if ( inputIndex > -1 )
                {
                    inDevice = m_NetList[netIndex].deviceList[inputIndex];

                    //Get handle to the output device
                    m_IniLib.GetConfigParamInt ( configString.c_str(), "OUTPUT", &outputIndex, -1 );

                    if ( ( inputIndex == -1 ) || ( inDevice == 0x0 ) || ( ! ( ( inDevice->GetFamNum() != DS2408_FN ) || ( inDevice->GetFamNum() != DS2405_FN ) ) ) )
                    {
                        retval = false;
                    }
                    else
                    {
                        ( ( CTAGControl* ) ( *ctrlIterator ) )->SetInputDevice ( inDevice );

                        //Check if we have a full controller or not
                        if ( outputIndex > -1 )
                        {
                            //Set the output device
                            outputIndex = GetDeviceIndexByConfigNumber ( netIndex, outputIndex );

                            if ( ( outputIndex == -1 ) || ( ! ( ( m_NetList[netIndex].deviceList[outputIndex]->GetFamNum() != DS2408_FN ) || ( m_NetList[netIndex].deviceList[outputIndex]->GetFamNum() != DS2405_FN ) ) ) )
                            {
                                retval = false;
                            }
                            else
                            {
                                outDevice = m_NetList[netIndex].deviceList[outputIndex];
                                ( ( CTAGControl* ) ( *ctrlIterator ) )->SetOutputDevice ( outDevice );
                                retval = true;
                            }
                        }

                    }
                }
                else
                {
                    retval = false;
                }

            }
            ;break;
            case DEV_UTACTRL:
            {
                CString outString = "DOUT";
                int i;
                int input1Index = 0, input2Index = 0;
                CVDevice *in1Device, *in2Device;

                //Connect TAG
                m_IniLib.GetConfigParamInt ( configString.c_str(), "DIN1", &inputIndex, -1 );

                if ( inputIndex > -1 )
                {
                    inputIndex = GetDeviceIndexByConfigNumber ( netIndex, inputIndex );
                    inDevice = m_NetList[netIndex].deviceList[inputIndex];

                    if ( ( inDevice == 0x0 ) || ( ! ( ( inDevice->GetFamNum() != DS2408_FN ) || ( inDevice->GetFamNum() != DS2405_FN ) ) ) )
                    {

                        retval = false;
                        break;
                    }
                    else
                    {

                        ( ( CUtaCtrl* ) ( *ctrlIterator ) )->m_Tag->SetInputDevice ( inDevice );
                        retval = true;
                    }
                }
                else
                {
                    retval = false;
                    break;
                }

                //Connect all DOUT
                for ( i = 1; i < 5; i++ )
                {
                    outString="DOUT";
                    outString += i;
                    m_IniLib.GetConfigParamInt ( configString.c_str(), outString.c_str(), &inputIndex, -1 );

                    if ( inputIndex > -1 )
                    {
                        inputIndex = GetDeviceIndexByConfigNumber ( netIndex, inputIndex );
                        inDevice = m_NetList[netIndex].deviceList[inputIndex];

                        if ( ( inDevice == 0x0 ) || ( ! ( ( inDevice->GetFamNum() != DS2408_FN ) || ( inDevice->GetFamNum() != DS2405_FN ) ) ) )
                        {
                            retval = false;
                            break;
                        }
                        else
                        {

                            ( ( CUtaCtrl* ) ( *ctrlIterator ) )->m_OutDigitals[i-1]->SetInputDevice ( inDevice );
                            retval = true;
                        }
                    }
                    else if ( ( i!=1 ) && ( i != 2 ) && ( i != 4 ) ) //we have optional and required parameters
                    {
                        retval = false;
                        break;
                    }
                }

                if ( !retval )
                {
                    break;
                }

                //Get temps and analogs and connect them in the UTACtrl class

                for ( i = 0; i < 2; i++ )
                {
                    outString = "TEMP";
                    outString += i+1;
                    m_IniLib.GetConfigParamInt ( configString.c_str(), outString.c_str(), &inputIndex, -1 );

                    inputIndex = GetDeviceIndexByConfigNumber ( netIndex, inputIndex );

                    if ( inputIndex > -1 )
                    {
                        inDevice = m_NetList[netIndex].deviceList[inputIndex];

                        if ( ( inputIndex == -1 ) || ( inDevice == 0x0 ) || ( ! ( ( inDevice->GetFamNum() != DS18S20_FN ) || ( inDevice->GetFamNum() != DS18B20_FN ) || ( inDevice->GetFamNum() != DS2438_FN ) ) ) )
                        {
                            retval = false;
                            break;
                        }
                        else
                        {

                            ( ( CUtaCtrl* ) ( *ctrlIterator ) )->m_TempControllers[i]->SetInputDevice ( inDevice );
                            retval = true;
                        }
                    }
                    else
                    {
                        retval = false;
                        break;
                    }
                }

                //Get Handles to the Analog Out devices

                //Get Handles to the devices
                m_IniLib.GetConfigParamInt ( configString.c_str(), "AN1OUT", &input1Index, -1 );
                m_IniLib.GetConfigParamInt ( configString.c_str(), "AN2OUT", &input2Index, -1 );

                if ( input1Index != -1 )
                {
                    //There is a PID LMD controller, init the pointers
                    input1Index = GetDeviceIndexByConfigNumber ( netIndex, input1Index );

                    if ( input1Index > -1 )
                    {
                        in1Device = m_NetList[netIndex].deviceList[input1Index];

                        //Recheck arguments
                        if ( ( in1Device == 0x0 ) ||
                                ( in1Device->GetFamNum() != DS2890_FN )
                           )
                        {
                            retval = false;
                        }
                        else
                        {
                            ( ( CUtaCtrl* ) ( *ctrlIterator ) )->m_Analogs[0] = ( CDS2890* ) ( in1Device );
                            retval = true;
                        }
                    }
                }
                else
                {
                    retval = false;
                    break;
                }

                if ( input2Index != -1 )
                {
                    //There is a PID LMD controller, init the pointers
                    input2Index = GetDeviceIndexByConfigNumber ( netIndex, input2Index );

                    in2Device = m_NetList[netIndex].deviceList[input2Index];

                    //Recheck arguments
                    if ( ( input2Index == -1 ) || ( in2Device == 0x0 ) ||
                            ( in2Device->GetFamNum() != DS2890_FN )
                       )
                    {
                        retval = false;
                    }
                    else
                    {
                        ( ( CUtaCtrl* ) ( *ctrlIterator ) )->m_Analogs[1] = ( CDS2890* ) ( in2Device );

                        retval = true;
                    }
                }
                else //Optional parameter
                {
                    ( ( CUtaCtrl* ) ( *ctrlIterator ) )->m_Analogs[1] = 0x0;

                    retval = true;
                }

                //Finish setup inside the module
                retval = ( ( CUtaCtrl* ) ( *ctrlIterator ) )->FinalizeSetup();

            }
            ;break;
            case DEV_DI2AO:
            {
                CString label;
                int nOfdevices, i;
                bool retval2 = false;

                //Setup digital inputs
                m_IniLib.GetConfigParamInt ( configString.c_str(),"NIN",&nOfdevices,-1 );

                if ( nOfdevices > 0 )
                {
                    for ( i = 1; i < nOfdevices+1; i++ )
                    {
                        label = "IN";
                        label+=i;

                        m_IniLib.GetConfigParamInt ( configString.c_str(), label.c_str(), &inputIndex, -1 );

                        if ( inputIndex > 0 )
                        {
                            inputIndex = GetDeviceIndexByConfigNumber ( netIndex, inputIndex );

                            if ( inputIndex > -1 )
                            {
                                inDevice = m_NetList.at ( netIndex ).deviceList.at ( inputIndex );

                                if ( inDevice->GetFamNum() == DS2408_FN )
                                {
                                    try
                                    {
                                        retval = ( ( CDI2AO* ) ( *ctrlIterator ) )->m_InVector.at ( i-1 )->SetInputDevice ( inDevice );
                                    }
                                    catch ( exception &e )
                                    {
                                        cout << "Errore in creazione DEV_DI2AO, DigitalIN: "<< e.what() << endl;
                                    }
                                }
                                else
                                {
                                    retval = false;
                                    break;
                                }
                            }
                            else
                            {
                                retval = false;
                                break;
                            }
                        }
                        else
                        {
                            retval = false;
                            break;
                        }
                    }//FOR
                }

                //Setup Analog Outputs
                m_IniLib.GetConfigParamInt ( configString.c_str(),"NOUT",&nOfdevices,-1 );
                if ( nOfdevices > 0 )
                {
                    for ( i = 1; i < nOfdevices+1; i++ )
                    {
                        label = "OUT";
                        label+=i;

                        m_IniLib.GetConfigParamInt ( configString.c_str(), label.c_str(), &inputIndex, -1 );

                        if ( inputIndex > 0 )
                        {
                            inputIndex = GetDeviceIndexByConfigNumber ( netIndex, inputIndex );

                            if ( inputIndex > -1 )
                            {
                                inDevice = m_NetList.at ( netIndex ).deviceList.at ( inputIndex );

                                if ( inDevice->GetFamNum() == DS2890_FN )
                                {
                                    try
                                    {
                                        retval2 = ( ( CDI2AO* ) ( *ctrlIterator ) )->m_OutVector.at ( i-1 )->SetInputDevice ( inDevice );
                                    }
                                    catch ( exception &e )
                                    {
                                        cout << "Errore in creazione DEV_DI2AO, AnalogOUT: "<< e.what() << endl;
                                    }

                                }
                                else
                                {
                                    retval2 = false;
                                    break;
                                }
                            }
                            else
                            {
                                retval2 = false;
                                break;
                            }
                        }
                        else
                        {
                            retval2 = false;
                            break;
                        }
                    }//FOR
                }

                retval = retval && retval2;

            }
            ;break;
            case DEV_MGC:
            {
                //Get Handle to input device
                m_IniLib.GetConfigParamInt ( configString.c_str(), "INPUT", &inputIndex, -1 );

                inputIndex = GetDeviceIndexByConfigNumber ( netIndex, inputIndex );

                if ( inputIndex > -1 )
                {
                    inDevice = m_NetList[netIndex].deviceList[inputIndex];

                    //Set input device
                    ( ( CNTHMGC* ) *ctrlIterator )->SetInputDevice ( ( CDS2751* ) inDevice );

                    //Check if the room has also a counter
                    m_IniLib.GetConfigParamInt ( configString.c_str(),"INPUT2", &inputIndex, -1 );
                    inputIndex = GetCtrlIndexByConfigNumber ( netIndex, inputIndex );

                    if ( inputIndex > -1 )
                    {
                        ( ( CNTHMGC* ) *ctrlIterator )->m_RoomCounter = ( CNTH_CNT* ) ( m_NetList[netIndex].CtrlList[inputIndex] );
                    }
                    else
                    {
                        cout << "Campo INPUT2 nel modulo numero :" << (*ctrlIterator)->GetConfigFileDevIndex()<< " NET:"<<netIndex+1<<" NON corretto"<<endl;
                        msDelay(5);
                        ( ( CNTHMGC* ) *ctrlIterator )->m_HasCounters = false;
                    }

                    retval = true;
                }
                else
                {
                    retval = false;
                }
            }
            ; break;
            case DEV_CNT:
            {
                //Get Handle to input device
                m_IniLib.GetConfigParamInt ( configString.c_str(), "INPUT", &inputIndex, -1 );

                inputIndex = GetDeviceIndexByConfigNumber ( netIndex, inputIndex );

                if ( inputIndex > -1 )
                {
                    inDevice = m_NetList[netIndex].deviceList[inputIndex];

                    //Set input device
                    ( ( CNTH_CNT* ) *ctrlIterator )->SetInputDevice ( ( CDS2751* ) inDevice );

                    retval = true;
                }
                else
                {
                    retval = false;
                }
            }
            ; break;
            case DEV_ACC:
            {
                CString inputType;
                //Controllo se l'ingresso è virtuale o reale
                m_IniLib.GetConfigParamString(configString.c_str(), "INPUT", &inputType, "");
                
                if (!strcasecmp("VIRTUAL", inputType.c_str()))
                {
                    //INgresso virtuale
                    ( ( CNTH_ACC* ) *ctrlIterator )->m_IsVirtual = true;
                    retval = true;
                }
                else
                {
                    //Get Handle to input device
                    m_IniLib.GetConfigParamInt ( configString.c_str(), "INPUT", &inputIndex, -1 );
    
                    inputIndex = GetDeviceIndexByConfigNumber ( netIndex, inputIndex );
    
                    if ( inputIndex > -1 )
                    {
                        inDevice = m_NetList[netIndex].deviceList[inputIndex];
    
                        //Set input device
                        ( ( CNTH_ACC* ) *ctrlIterator )->SetInputDevice ( ( CDS2751* ) inDevice );
    
                        retval = true;
                    }
                    else
                    {
                        retval = false;
                    }
                }
            }
            ; break;
            case DEV_TIMEMARKER:
            {
                //Get Handle to input device
                m_IniLib.GetConfigParamInt ( configString.c_str(), "INPUT", &inputIndex, -1 );

                inputIndex = GetDeviceIndexByConfigNumber ( netIndex, inputIndex );

                if ( inputIndex > -1 )
                {
                    inDevice = m_NetList[netIndex].deviceList[inputIndex];
                    
                    ( ( CTimeMarkerCtrl* ) ( *ctrlIterator ) )->SetInputDevice ( (CDS2751*)(inDevice) );
                    retval = true;
                }
                else
                {
                    retval = false;
                }
            }break;
            case DEV_C3POINT:
            {
                ( ( C3PointCtrl* ) ( *ctrlIterator ) )->m_NetPtr = this;
                retval = ( ( C3PointCtrl* ) ( *ctrlIterator ) )->ConnectControllers(netIndex, configString.c_str());
            };break;
            default: retval = true;
        }//Switch

        if ( !retval )
        {
            if ( m_DoDebug )
            {
                cout << "Attenzione !! Si e'verificato un problema nella connessione dell'oggetto "<< Device_strings[controllerType] << " di indice " << configDevIndex << " .Verificare i driver IN/OUT" << endl;
            }

            return retval;
        }
        else if ( m_DoDebug )
        {
            cout << "I/O del Controller di indirizzo " << ( *ctrlIterator )->GetMemoryAddress() << " connessi correttamente" << endl;cout.flush();
        }
    }//FOR CONTROLLER

    return retval;

}

///////////////////////////////////////////////////
//             CONNECTCOORDINATORS
///////////////////////////////////////////////////
bool COneWireNet::ConnectCoordinators ( int netIndex )
{
    vector<CVController*>::iterator ctrlIterator;
    e_DeviceType coordinatorType = DEV_NONE;
    int configNetIndex, configDevIndex;
    CString configDevString, configNETString;
    CString configString;
    bool retVal = true, isCoordinator = false;

    cout << "Inizio connessione ingressi/uscite dei coordinatori..."<<endl;cout.flush();
    if ( m_NetList[netIndex].CtrlList.size() == 0 )
    {
        cout << "ERRORE!! Non è stato definito nessun controller, impossibile continuare" << endl;
        return false;
    }

    for ( ctrlIterator = m_NetList[netIndex].CtrlList.begin(); ctrlIterator != m_NetList[netIndex].CtrlList.end(); ctrlIterator++ )
    {
        coordinatorType = ( *ctrlIterator )->GetControllerType();
        configDevIndex = ( *ctrlIterator )->GetConfigFileDevIndex();

        configNETString = "NET";
        configNetIndex = netIndex + 1;
        configNETString+=configNetIndex;
        configDevString = "Device";

        if ( configDevIndex < 10 )
        {
            configDevString+="0";
        }
        configDevString+=configDevIndex;
        configString = m_IniFile->GetString ( configDevString, configNETString );

        switch ( coordinatorType )
        {
            case DEV_FULLUTACTRL:
            {
                isCoordinator = true;
                //decodifico la stringa del coordinatore
                //NAME:FullUTACtrl,ADDR:YY,HEATBAT:,COLDBAT:,HUMBAT:,VOCPID:,CO2PID:,LMMAX:,LMMIN:,MAINSHUTT:,RECSHUTT:,HUMIDITY:,TEMPMND:,TEMPRIP:,POST1:,POST2,,FAN:,SUMMER:,SHUTTDELAY:,HUMMACHINE:,POSTSP,POSTHYST
                ( ( CVCoordinator* ) ( *ctrlIterator ) )->SetNetPtr( ( void* ) this);
                retVal = ( ( CFullUTACtrl* ) ( *ctrlIterator ) )->ConnectControllers();
            }
            ;break;
            case DEV_FULLUTACTRL_2:
            {
                isCoordinator = true;
                ( ( CVCoordinator* ) ( *ctrlIterator ) )->SetNetPtr(( void* ) this);
                retVal = ( ( CFullUTACtrl2* ) ( *ctrlIterator ) )->ConnectControllers();
            }
            ;break;
            case DEV_ALARMCTRL:
            {
                isCoordinator = true;
                ( ( AlarmCoordinator* ) ( *ctrlIterator ) )->SetNetPtr(( void* ) this);
                retVal = ( ( AlarmCoordinator* ) ( *ctrlIterator ) )->ConnectControllers();
            }
            ;break;
            case DEV_CHOVER:
            {
                isCoordinator = true;
                ( ( ChangeOverCoord* ) ( *ctrlIterator ) )->SetNetPtr( ( void* ) this);
                retVal = ( ( ChangeOverCoord* ) ( *ctrlIterator ) )->ConnectControllers();
            };break;
            case DEV_CLIMATICOORD:
            {
                isCoordinator = true;
                ( ( CClimaticCurve* ) ( *ctrlIterator ) )->SetNetPtr( ( void* ) this);
                retVal = ( ( CClimaticCurve* ) ( *ctrlIterator ) )->ConnectControllers();
            };break;
            case DEV_FLOORCOORD_2:
            {
                isCoordinator = true;
                ( ( CFloorCoord2* ) ( *ctrlIterator ) )->SetNetPtr((void*)this);
                retVal = ( ( CFloorCoord2* ) ( *ctrlIterator ) )->ConnectControllers();
            };break;
            case DEV_IBUTT_RDR:
            {
                isCoordinator = true;
                ( ( CIButtonReader* ) ( *ctrlIterator ) )->SetNetPtr(( void* ) this);
                retVal = ( ( CIButtonReader* ) ( *ctrlIterator ) )->ConnectControllers();
            };break;
            case DEV_BLOCKCOORD:
            {
                isCoordinator = true;
                retVal =  ( ( CBlockCoordinator* ) ( *ctrlIterator ) )->ConnectControllers();
            }
            default:
            {
                isCoordinator = false;
                retVal = true;
            }
        }

        if ( isCoordinator )
        {
            if ( !retVal )
            {
                if ( m_DoDebug )
                {
                    cout << "Attenzione !! Si e'verificato un problema nella connessione del coordinatore "<< Device_strings[coordinatorType] << " di indice " << configDevIndex << " .Verificare i controller IN/OUT" << endl;
                    msDelay(1000);
                }
            }
            else if ( m_DoDebug )
            {
                cout << "I/O del Coordinatore " << configDevIndex << " connessi correttamente" << endl;cout.flush();
            }
        }

    }//FOR

    return retVal;
}

///////////////////////////////////////////////////
//             INITNET                             //
///////////////////////////////////////////////////
bool COneWireNet::InitNet ( int netIndex )
{
    CString conf = "", net="", portName="";
    int param;
    char buf[32];
    char tmp[16];

    memset ( tmp, 0, 16*sizeof ( char ) );
    memset ( buf, 0, 32*sizeof ( char ) );

    net = "NET";
    net += ( netIndex+1 );

    //Check the index
    if ( ( netIndex >= m_NetList.size() ) || ( m_IniFile == 0x0 ) )
    {
        //Bail out
        return false;
    }

    cout << "OneWireNet: InitNet : "<< net << endl;cout.flush();

    //Start by getting if the net is wireless or not
    conf = m_IniFile->GetString ( Config_Strings[CONF_NETWL], net );

    if ( conf.size() != 0 )
    {
        m_IniLib.GetConfigParamInt ( conf.c_str(), "ISWL", &param, 0 );

        //FIXME aggiungere la gestione degli errori
        if ( param )
        {
            m_NetList[netIndex].master->SetWireless ( 1 );
            m_NetList[netIndex].isWl = true;

            //get the wireless net address
            m_IniLib.GetConfigParamInt ( conf.c_str(), "NETADDR", &param, -1 );
            m_NetList[netIndex].wlAddr[0] = param;

            m_IniLib.GetConfigParamInt ( conf.c_str(), "SUBNETADDR", &param, -1 );
            m_NetList[netIndex].wlAddr[1] = param;
        }
    }

    conf = m_IniFile->GetString ( Config_Strings[CONF_NETISOVERIP], net );

    if ( conf.size() != 0 )
    {
        m_IniLib.GetConfigParamInt ( conf.c_str(), "ISOIP", &param, 0 );

        //FIXME aggiungere la gestione degli errori
        if ( param )
        {
            m_NetList[netIndex].master->SetWireless ( 1 );
            m_NetList[netIndex].isOverIp = true;

            //get the wireless net address
            m_IniLib.GetConfigParamString ( conf.c_str(), "ADDR", buf, 32, "127.0.0.1" );
            m_NetList[netIndex].ipAddress = buf;

            m_IniLib.GetConfigParamInt ( conf.c_str(), "PORT", &param, -1 );
            m_NetList[netIndex].ipPort = param;
        }

    }

    //Get default alarms if any
    conf = m_IniFile->GetString ( Config_Strings[CONF_NETDEFAULTTEMPALARMS], net );

    m_IniLib.GetConfigParamInt ( conf.c_str(), "AllarmeMax", &param, -100 );
    m_NetList[netIndex].defAlMax = param;

    m_IniLib.GetConfigParamInt ( conf.c_str(), "AllarmeMin", &param, -100 );
    m_NetList[netIndex].defAlMin = param;

    //Get if ibuttonreader is present
    m_NetList[netIndex].hasIButtonReader = m_IniFile->GetBool ( Config_Strings[CONF_IBUTTONREADERPORT],net,false );

    //Get the Net Delay
    int netDel = m_IniFile->GetInt ( Config_Strings[CONF_NETDELAY], net,0 );

    //Set the delay on the comunications
    m_NetList[netIndex].netDelay = netDel;

    //Get Port Name
    portName = m_IniFile->GetString ( Config_Strings[CONF_NETCOMPORT], net );

    m_NetList[netIndex].netPortName = portName;

    //Add port to the master device
    //m_NetList[netIndex].master->SetPortName(portName);

    //Try to initialize the port
    param = m_NetList[netIndex].master->OpenPortDirect ( ( char* ) portName.c_str() );

    if ( param >= 0 )
    {
        m_NetList[netIndex].master->SetBaudCOMDirect ( param, PARMSET_9600 );

        //port correctly initialized
        if ( m_DoDebug )
        {
            cout << "OneWireNET: COMPORT inizializzata correttamente" << endl;
            cout.flush();
        }

        m_NetList[netIndex].master->ClosePortDirect ( param );
    }
    else
    {
        PushError ( OWERROR_OPENCOM_FAILED, netIndex + 1 );
    }

    //Check if the temperature alarms are managed by software or hardware
    if ( m_IniFile->GetBool ( Config_Strings[CONF_NETSWTEMPALARMS], net ) )
    {
        m_NetList[netIndex].areTempAlarmsSW = true;
    }
    else
    {
        m_NetList[netIndex].areTempAlarmsSW = false;
    }

    //************************** Flag per i salvataggi su file ini ***************
    m_NetList[netIndex].saveDigitalState = m_IniFile->GetBool(Config_Strings[CONF_SAVEDIGITALSTATE], net, true);
    
    //************************** NEW TIMER ***********************************
    param = m_IniFile->GetInt ( Config_Strings[CONF_NETTIMERID], net,-1 );

    if ( param > 0 )
    {
        m_NetList[netIndex].timerID = param;
        m_NetList[netIndex].netTimer = m_Timer;
        m_NetList[netIndex].isTimerOn = true;
    }
    else
    {
        m_NetList[netIndex].timerID = -1;
        m_NetList[netIndex].isTimerOn = false;
        m_NetList[netIndex].netTimer = 0x0;
    }

    //Init all the devices
    if ( InitDevices ( netIndex ) )
    {
        if ( m_DoDebug )
        {
            cout << "OneWireNet: Rete Inizializzata"<<endl;
            cout.flush();
        }

        return true;
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////
//             INITALLNETS                    //
///////////////////////////////////////////////////
bool COneWireNet::InitAllNets( )
{
    unsigned int i = 0;
    bool retVal = true;

    for ( i = 0; i < m_NetList.size(); i++ )
    {
        m_NetList.at ( 0 );
        if ( !InitNet ( i ) )
        {
            retVal = false;
        }
    }

    for ( i = 0; i < m_NetList.size(); i++ )
    {
        //Connetto i coordinatori DOPO tutte le net perche' lavorano sugli indirizzi assoluti
        if ( ConnectCoordinators ( i ) )
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    

    return retVal;
}

///////////////////////////////////////////////////
//             RELEASEALLNETS           //
///////////////////////////////////////////////////
bool COneWireNet::ReleaseAllNets( )
{
    unsigned int i = 0;
    bool retVal = true;


    for ( i = 0; i < m_NetList.size(); i++ )
    {
        if ( !ReleaseNet ( i ) )
        {
            retVal = false;
        }
    }

    return retVal;
}

///////////////////////////////////////////////////
//             UPDATE
///////////////////////////////////////////////////
bool COneWireNet::UpdateAllTemp ( int netIndex, unsigned char command )
{
    uchar outBuf[8];

    //Do usual controls
    if ( !CheckNetIndex ( netIndex ) )
    {
        return false;
    }

    switch ( command )
    {
        case CONVERT_T :
        {
            //It is a temperature update
            //First Try: send the ignore ROM command
            memset ( outBuf, 0, 8 );

            //Send a reset
//             if (!m_NetList[netIndex].master->owTouchReset(m_NetList[netIndex].portHandler))
//             {
//                 if (m_DoDebug)
//                     cout << "Error in owTouchReset" << endl;
//
//                 return false;
//             }

            //Wait a bit to let line settle
//             msDelay(5);

            //Skip ROM command
            outBuf[0] = SKIPROM;
            //ConvertT command
            outBuf[1] = CONVERT_T;


            //Write the command
            if ( !m_NetList[netIndex].master->owBlock ( m_NetList[netIndex].portHandler, true, outBuf, 2 ) )
            {
                if ( m_DoDebug )
                    cout << "Impossibile Scrivere sulla NET"<< ( netIndex+1 ) << " Per aggiornare le temperature" << endl;

                //Add error
//                 m_NetList[netIndex].errorHandler.AddError();

                return false;
            }

            //TODO da rimettere come era prima
            //Wait a bit for the conversion to be done
//             msDelay(1200);

            //Clear error
//             m_NetList[netIndex].errorHandler.ClearError();

            return true;
            break;
        }

        case CONVERT_V:
        {
            //FIXME: Da testare

            //It is a Voltage update

            //First Try: send the ignore ROM command
            memset ( outBuf, 0, 8 );

            //Skip ROM command
            outBuf[0] = SKIPROM;
            //ConvertT command
            outBuf[1] = CONVERT_T;

            //Write the command
            m_NetList[netIndex].master->owBlock ( m_NetList[netIndex].portHandler, true, outBuf, 2 );

            //Wait a bit for the conversion to be done
            msDelay ( 800 );

//             m_NetList[netIndex].errorHandler.ClearError();

            return true;
            break;
        }

        default: return false;
    }

}

///////////////////////////////////////////////////
//             SETALARMT                       //
///////////////////////////////////////////////////
bool COneWireNet::SetAlarmT ( int netIndex, int devIndex,  int maxAlarmLevel, int minAlarmLevel )
{
    bool retVal = false;

    //Usual controls
    if ( !CheckIndexes ( netIndex, devIndex, false ) )
    {
        return false;
    }

    //Check if it is a temperature controller
    if ( CheckControllerType ( netIndex, devIndex, DEV_TEMPCTRL ) )
    {
        retVal = ( ( CTempCtrl* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetAlarmLevel ( maxAlarmLevel, minAlarmLevel );
    }
    else
    {
        //Wrong device
        retVal = true;
    }

    return retVal;

}

///////////////////////////////////////////////////
//             GETTEMP
///////////////////////////////////////////////////
bool COneWireNet::GetTemp ( float * dest, int netIndex, int devIndex, bool updateFirst )
{

    *dest = GetTemp ( netIndex, devIndex, updateFirst );

    if ( *dest == TEMP_ERRVAL )
    {
        return false;
    }
    else
    {
        return true;
    }
}

///////////////////////////////////////////////////
//             GETTEMP
///////////////////////////////////////////////////
float COneWireNet::GetTemp ( int netIndex, int devIndex, bool updateFirst )
{
    float temp = TEMP_ERRVAL;
    bool resetOnError = false;
    int originalNetDelay=0;


    //Usual check
    if ( !CheckIndexes ( netIndex, devIndex, false ) )
    {
        //Error
        return TEMP_ERRVAL;
    }

    //If we are wireless try to slow down the reading of the sensors.
    //Please Note that the 40 ms delay is just a tentative value: it seems to work but there is no reason not to change it
    if ( m_NetList[netIndex].isWl )
    {
        //Get the NetDelay
        originalNetDelay = m_NetList[netIndex].master->GetNetDelay();

        //Set a slower NETDelay
        m_NetList[netIndex].master->SetNetDelay ( originalNetDelay + 40 );
    }

    if ( CheckControllerType ( netIndex, devIndex, DEV_TEMPCTRL ) )
    {
        //Read from the device
        if ( ! ( ( CTempCtrl* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->ReadTemperature ( updateFirst, &temp ) )
        {
            resetOnError = true;
        }
    }
    else if ( CheckControllerType ( netIndex, devIndex, DEV_TEMPCTRLHYST ) )
    {
        //Read from the device
        if ( ! ( ( CTempCtrlHyst* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->ReadTemperature ( updateFirst, &temp ) )
        {
            resetOnError = true;
        }
    }



    if ( m_NetList[netIndex].isWl )
    {
        //Restore original netDelay
        m_NetList[netIndex].master->SetNetDelay ( originalNetDelay );
    }

    //Some error occurred, reset the master
    if ( resetOnError )
    {
        if ( m_DoDebug )
        {
            cout << "Resetting Master" << endl;
        }

        m_NetList[netIndex].master->DS2480Detect ( m_NetList[netIndex].portHandler );
    }

    return temp;
}


///////////////////////////////////////////////////
//             GETALLTEMP
///////////////////////////////////////////////////
int COneWireNet::GetAllTemp ( int netIndex, bool upFirst, float dest[][2], int maxNum )
{
    int retVal = 0, index = 0, stopVal = 0;

    //Usual Check
    if ( ( !CheckNetIndex ( netIndex ) ) || ( dest == NULL ) || ( maxNum < 0 ) )
    {
        retVal = -1;
    }

    //Check if we have to update
    if ( ( upFirst ) && ( retVal != -1 ) )
    {
        if ( !UpdateAllTemp ( netIndex, CONVERT_T ) )
        {
            if ( m_DoDebug )
                cout << "Unable to update TEMP..." << endl;

            //An error occurred
            return retVal = -1;
        }
    }

    if ( maxNum < m_NetList[netIndex].CtrlList.size() )
    {
        stopVal = maxNum;
    }
    else
    {
        stopVal = m_NetList[netIndex].CtrlList.size();
    }


    //Start getting temps
    if ( retVal != -1 )
    {
        //Start cycle
        for ( index = 0;  index < stopVal; index++ )
        {
            //Check if the given device is a T sensor
            if ( CheckControllerType ( netIndex, index, DEV_TEMPCTRL ) )
            {
                //Update the device index
                dest[retVal][0] = index;
                dest[retVal][1] = GetTemp ( netIndex, index, false );

                retVal++;
            }
        }
    }

    return retVal;

}


///////////////////////////////////////////////////
//             SearchAlarmTemp            //
//////////////////////////////////////////////////
int COneWireNet::SearchAlarmTemp ( int netIndex, SMALLINT family, int * dest, int maxNum )
{
    int retVal = 0;
    int index = 0;
    int devIndex = 0;
    SMALLINT nextDev;
    uchar serialNum[8];
    int stopIndex = 0;

    //Usual Check
    if ( ( !CheckNetIndex ( netIndex ) ) || ( dest == NULL ) || ( maxNum < 0 ) )
    {
        retVal = -1;
        return retVal;
    }

    //Check if the timer is on and what it says
    if ( m_NetList[netIndex].isTimerOn && m_Timer->IsTimerEnabled ( m_NetList[netIndex].timerID ) )
    {
        //We have the timer, check if the timer says alarms on
        if ( !m_NetList[netIndex].netTimer->GetValue ( m_NetList[netIndex].timerID, TIMERVAL_ALARM ) )
        {
            if ( m_DoDebug )
            {
                cout << "Alarms Off by timer!!" << endl;
            }

            //Timer says alarms off
            retVal = 0;
            return retVal;
        }
    }

    if ( m_NetList[netIndex].CtrlList.size() < maxNum )
    {
        stopIndex = m_NetList[netIndex].CtrlList.size();
    }
    else
    {
        stopIndex = maxNum;
    }

    //check if we have to poll for the alarms or they are managed internally by the sensors (so called "software alarms" )
    if ( m_NetList[netIndex].areTempAlarmsSW )
    {
        retVal = 0;

        //Cycle through all tje sensor to check for alarms
        for ( devIndex = 0; devIndex < stopIndex; devIndex++ )
        {
            //Check if it is a temperature sensor
            if ( CheckControllerType ( netIndex, devIndex, DEV_TEMPCTRL ) )
            {
                //Update the status of the controller (alarms, Digitaloutputs, ...)
                ( ( CTempCtrl* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->Update ( false );

                //Update its alarm status and eventually store the total count of alarms
                if ( ( ( CTempCtrl* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->GetAlarmState() )
                {
                    if ( retVal < maxNum )
                    {
                        //Store the index
                        dest[retVal] = devIndex;

                        //Update total count
                        retVal++;
                    }
                    else
                    {
                        //Maximum number reached, break
                        break;
                    }
                }
            }
        }
    }
    else
    {

        //Start searching for all the temp devices with an alarm.
        m_NetList[netIndex].master->owFamilySearchSetup ( m_NetList[netIndex].portHandler, DS18S20_FN );

        nextDev = m_NetList[netIndex].master->owFirst ( m_NetList[netIndex].portHandler, true, true );

        //Find the first
        if ( nextDev < 0 )
        {
            //No devices found or an error occurred, return the search value
            if ( m_AfoErrorHandler->GetLastErrorNum() == 3 )
            {
                //Probably we don't have any error
                nextDev = 0;
            }
        }
        else if ( nextDev > 0 )
        {
            //Get the serial number of the device and the corresponding index in the NET
            m_NetList[netIndex].master->owSerialNum ( m_NetList[netIndex].portHandler, serialNum, true );


            //Check if the device is of the correct family
            if ( ( serialNum[0] & 0x7F ) == ( family & 0x7F ) )
            {
                index = GetIndexBySerial ( netIndex, serialNum );

                //Check for errors
                if ( index < 0 )
                {
                    //Some error occurred
                    return index;

                }

                //Store the value and update the counter
                dest[0] = index;
                retVal++;
            }

            //Start searching other devices
            nextDev = m_NetList[netIndex].master->owNext ( m_NetList[netIndex].portHandler, true, true );

            //Start searching for other devices
            while ( nextDev > 0 )
            {
                //Check if we have reached the maximum number of devices
                if ( retVal < maxNum )
                {
                    //Get the serial number of the device and the corresponding index in the NET
                    m_NetList[netIndex].master->owSerialNum ( m_NetList[netIndex].portHandler, serialNum, true );
                    index = GetIndexBySerial ( netIndex, serialNum );

                    if ( ( index >= 0 ) && ( ( serialNum[0] & 0x7F ) == ( family & 0x7F ) ) )
                    {
                        dest[retVal] = index;
                        retVal++;
                    }
                }
                else
                {
                    //Maximum number reached, exit
                    break;
                }

                //Update search
                nextDev = m_NetList[netIndex].master->owNext ( m_NetList[netIndex].portHandler, true, true );
            }
        }


        //Update the alarms inside the sensors
        UpdateTempAlarmState ( netIndex, dest, retVal );
    }

    return retVal;
}


///////////////////////////////////////////////////
//             SearchDIDOActivity             //
//////////////////////////////////////////////////
int COneWireNet::SearchDIDOActivity ( int portNumber, int *remoteDIDOList, int maxNum, vector<CVDevice*>& ds2408ToClear )
{
    int nOfRemoteDIDO = 0;
    int nOfActiveDevices = 0;
    uchar serialNum[8];
    uchar stateRegister[3] = {0,0,0};
    //Matrice che contiene in posizione zero il numero di serie e in posizione 1 il registro del ds2408
    uchar *activeDevicesMatrix[MAX_NUM_DEV_PER_NET][2];
    int devIndex = -1;
    int netIndex = -1;
    int didoIndex = -1;
    int nextDev = 0;

    memset ( activeDevicesMatrix, 0x0, MAX_NUM_DEV_PER_NET*2*sizeof ( uchar ) );

    //Start searching for all the DIDO devices with an activity
    m_Master->owFamilySearchSetup ( portNumber, DS2408_FN );

    nextDev = m_Master->owFirst ( portNumber, true, true );

    //Find the first
    if ( nextDev < 0 )
    {
        //No devices found or an error occurred, return the search value
        if ( m_AfoErrorHandler->GetLastErrorNum() == 3 )
        {
            //Probably we don't have any error
            //20080725 -- cambiata da nextDev = 0;
            return 0;
        }
    }
    else if ( nextDev > 0 )
    {
        //Get the serial number of the device and the corresponding index in the NET
        m_Master->owSerialNum ( portNumber, serialNum, true );


        //Check if the device is of the correct family
        if ( serialNum[0] == DS2408_FN )
        {
            netIndex = GetNetBySerial ( serialNum );
            devIndex = GetIndexBySerial ( netIndex, serialNum );

            if ( ( netIndex >= 0 ) || ( devIndex >= 0 ) )
            {
                //Store the value and update the counter
                m_NetList[netIndex].deviceList[devIndex]->GetSN ( serialNum );
                if (( ( CDS2408* ) ( m_NetList[netIndex].deviceList[devIndex] ) )->ReadRegister ( 0x88, stateRegister ))
                {
                    msDelay ( 10 );
    
                    if ( stateRegister[2] != 0 )
                    {
                        activeDevicesMatrix[nOfActiveDevices][0] = new uchar[8];
                        activeDevicesMatrix[nOfActiveDevices][1] = new uchar[3];
    
                        ( ( CDS2408* ) ( m_NetList[netIndex].deviceList[devIndex] ) )->ClearActivityLatch( );
    
                        if ( m_DoDebug == 3 )
                        {
                            cout << "SearchDIDOActivity(1) -- Resetto device indice " << m_NetList[netIndex].deviceList[devIndex]->GetConfigFileDevIndex() << " NET: "<<netIndex+1<<endl;
                        }
    
                        if ( ( activeDevicesMatrix[nOfActiveDevices][0] != 0x0 ) && ( activeDevicesMatrix[nOfActiveDevices][1] != 0x0 ) )
                        {
                            memcpy ( activeDevicesMatrix[nOfActiveDevices][0], serialNum, 8*sizeof ( uchar ) );
                            memcpy ( activeDevicesMatrix[nOfActiveDevices][1], stateRegister, 3*sizeof ( uchar ) );
                            nOfActiveDevices++;
                        }
                        else
                        {
                            //Probably we run out of memory: it is a serious problem, bail out
                            return -1;
                        }
                    }
                }
            }

        }

        //Start searching other devices
        nextDev = m_Master->owNext ( portNumber, true, true );

        //Start searching for other devices
        while ( nextDev > 0 )
        {
            //Get the serial number of the device and the corresponding index in the NET
            m_Master->owSerialNum ( portNumber, serialNum, true );


            //Check if the device is of the correct family
            if ( serialNum[0] == DS2408_FN  )
            {
                //Check if we have reached the maximum number of devices
                if ( nOfActiveDevices < MAX_NUM_DEV_PER_NET )
                {
                    netIndex = GetNetBySerial ( serialNum );
                    devIndex = GetIndexBySerial ( netIndex, serialNum );

                    if ( ( netIndex >= 0 ) && ( devIndex >= 0 ) )
                    {
                        //Store the value and update the counter
                        if (( ( CDS2408* ) ( m_NetList[netIndex].deviceList[devIndex] ) )->ReadRegister ( 0x88, stateRegister ))
                        {

                            //Questo non so perchè c'è
                            msDelay ( 10 );
    
                            //TODO da controllare perche' era stata messa a posto ma sembra che me la sia persa, vedere revisioni post 05/05/2007 o CVS
                            if ( stateRegister[2] != 0 )
                            {
                                activeDevicesMatrix[nOfActiveDevices][0] = new uchar[8];
                                activeDevicesMatrix[nOfActiveDevices][1] = new uchar[3];
    
                                ( ( CDS2408* ) ( m_NetList[netIndex].deviceList[devIndex] ) )->ClearActivityLatch( );
    
                                if ( m_DoDebug == 3 )
                                {
                                    cout << "SearchDIDOActivity(2) -- Resetto device indice " << m_NetList[netIndex].deviceList[devIndex]->GetConfigFileDevIndex() <<" NET:  "<<netIndex+1<< endl;
                                }
    
                                if ( ( activeDevicesMatrix[nOfActiveDevices][0] != 0x0 ) && ( activeDevicesMatrix[nOfActiveDevices][1] != 0x0 ) )
                                {
                                    memcpy ( activeDevicesMatrix[nOfActiveDevices][0], serialNum, 8*sizeof ( uchar ) );
                                    memcpy ( activeDevicesMatrix[nOfActiveDevices][1], stateRegister, 3*sizeof ( uchar ) );
                                    nOfActiveDevices++;
                                }
                                else
                                {
                                    //Probably we run out of memory: it is a serious problem, bail out
                                    return -1;
                                }
                            }
                        }
                    }
                    else
                    {
                        if ( m_DoDebug==3 )
                        {
                            cout << "Attenzione trovato dispositivo 2408 NON presente nel file di configurazione" << endl;
                        }
                    }

                }
                else
                {
                    //Maximum number reached, exit
                    break;
                }
            }//IF controllo famiglia

            //Update search
            nextDev = m_Master->owNext ( portNumber, true, true );
        } //While
    }
    

    if ( nOfActiveDevices == 0 )
    {
//         if (m_DoDebug)
//         {
//             cout << "NON ho trovato attivita'" << endl; cout.flush();
//         }

        return 0;
    }
    else
    {
//         if (m_DoDebug)
//         {
//             cout << "Ho trovato "<<nOfActiveDevices<<" Dispositivi con attività" << endl;cout.flush();
//         }
    }

    //Extract the relevant informations
    for ( netIndex = 0; netIndex < GetTotalNumberOfNets(); netIndex++ )
    {
        //Check if the NET has the same port number as the one used
        if ( m_NetList[netIndex].portHandler != portNumber )
        {
            continue;
        }

        for ( didoIndex = 0; didoIndex != m_NetList[netIndex].CtrlList.size(); didoIndex++ )
        {
            if ( ( m_NetList[netIndex].CtrlList[didoIndex]->GetControllerType() == DEV_REMOTEDIDO ) || ( m_NetList[netIndex].CtrlList[didoIndex]->GetControllerType() == DEV_BUTTONCTRL ) ||
                    ( m_NetList[netIndex].CtrlList[didoIndex]->GetControllerType() == DEV_STEPDIGITALOUT ) )
            {
                //Ciclo su tutti i numeri di serie gia'  acquisiti
                for ( devIndex = 0; devIndex < nOfActiveDevices; devIndex++ )
                {
                    if ( !memcmp ( ( char* ) ( activeDevicesMatrix[devIndex][0] ), ( char* ) ( ( ( CVMultiDIDO* ) ( m_NetList[netIndex].CtrlList[didoIndex] ) )->GetDeviceSN ( false ) ), 8 ) )
                    {
                        //I numeri di serie combaciano

                        //Imposto il registro internamente al DIDO
                        ( ( CVMultiDIDO* ) ( m_NetList[netIndex].CtrlList[didoIndex] ) )->SetStatusRegister ( false, activeDevicesMatrix[devIndex][1] );

                        if ( ( ( CVMultiDIDO* ) ( m_NetList[netIndex].CtrlList[didoIndex] ) )->GetActivity ( false ) )
                        {
                            remoteDIDOList[nOfRemoteDIDO] = m_NetList[netIndex].CtrlList[didoIndex]->GetMemoryAddress();
                            nOfRemoteDIDO++;
                        }

                        break;
                    }
                }
            }
            else if ( m_NetList[netIndex].CtrlList[didoIndex]->GetControllerType() == DEV_DIDO )
            {
                //Se non è un ingresso non mi interessa
                if ( !(( CDigitalIO* ) ( m_NetList[netIndex].CtrlList[didoIndex] ) )->IsInput() )
                {
                    continue;
                }
                
                for ( devIndex = 0; devIndex < nOfActiveDevices; devIndex++ )
                {
                    if ( !memcmp ( ( char* ) ( activeDevicesMatrix[devIndex][0] ), ( char* ) ( ( ( CDigitalIO* ) ( m_NetList[netIndex].CtrlList[didoIndex] ) )->GetDeviceSN() ),8 ) )
                    {
                        //I numeri di serie combaciano

                        //Imposto il registro internamente al DIDO
                        ( ( CDigitalIO* ) ( m_NetList[netIndex].CtrlList[didoIndex] ) )->SetStatusRegister ( activeDevicesMatrix[devIndex][1] );

                        if ( ( ( CDigitalIO* ) ( m_NetList[netIndex].CtrlList[didoIndex] ) )->GetActivity ( false ) )
                        {
                            remoteDIDOList[nOfRemoteDIDO] = m_NetList[netIndex].CtrlList[didoIndex]->GetMemoryAddress();
                            nOfRemoteDIDO++;
                        }

                        break;
                    }
                }
            }
        } //FOR DidoIndex
    }//FOR NetIndex

    //Clean up everything
    for ( devIndex = 0; devIndex < nOfActiveDevices; devIndex++ )
    {
        delete[] activeDevicesMatrix[devIndex][0];
        delete[] activeDevicesMatrix[devIndex][1];
    }

    return nOfRemoteDIDO;

}

///////////////////////////////////////////////////
//             GetIndexBySerial             //
//////////////////////////////////////////////////
int COneWireNet::GetIndexBySerial ( int netIndex, uchar *serNum )
{
    int retVal = -1, i = 0;

    //Usual Check
    if ( !CheckNetIndex ( netIndex ) )
    {
        return retVal;
    }

    for ( i = 0; i < m_NetList[netIndex].deviceList.size(); i++ )
    {
        if ( !memcmp ( ( char* ) serNum, ( char* ) ( m_NetList[netIndex].deviceList[i]->GetSN() ), 8 ) )
        {
            retVal = i;
            break;
        }
    }

    return retVal;

}

///////////////////////////////////////////////////
//             GetIndexBySerial             //
//////////////////////////////////////////////////
int COneWireNet::GetIndexBySerial ( int netIndex, CString serNum )
{
    int retVal = -1;
    uchar sn[8];

    if (serNum.length()<16)
    {
        return retVal;
    }
    
    memset (sn, 0x0, 8);
    ConvertSN2Hex ( serNum.c_str(), sn );
    retVal = GetIndexBySerial ( netIndex, sn );

    return retVal;
}

///////////////////////////////////////////////////
//             SETALLALARMT               //
//////////////////////////////////////////////////
bool COneWireNet::SetAllAlarmT ( int netIndex, int maxAlarmLevel, int minAlarmLevel )
{
    int i = 0;
    int numDev = 0;

    //Usual Check
    if ( !CheckNetIndex ( netIndex ) )
    {
        return false;
    }

    //Get the number of the actually stored devices
    numDev = m_NetList[netIndex].deviceList.size();

    //Check for temperature devices
    if ( !m_NetList[netIndex].hasTempDevices )
    {
        //No temperature devices, return true
        return true;
    }

    for ( i = 0; i < numDev; i++ )
    {
        //Check if it is a temp sensor
        if ( CheckDeviceFamily ( netIndex, i, DS18B20_FN ) ||
                CheckDeviceFamily ( netIndex, i, DS18S20_FN ) )
        {
            //Set the alarm
            if ( !SetAlarmT ( netIndex, i, maxAlarmLevel, minAlarmLevel ) )
            {
                //An error occurred writing the alarm
                return false;
            }
        }

        //Commented out to improve performance 3/5/2006
        //msDelay(50);
    }

    return true;

}

///////////////////////////////////////////////////
//             SETALLALARMT
///////////////////////////////////////////////////
bool COneWireNet::InitTempControllers ( int netIndex )
{
    int ctrlIndex = 0;
    bool retVal = false;

    if ( CheckNetIndex ( netIndex ) )
    {
        if ( m_NetList[netIndex].hasTempDevices )
        {
            for ( ctrlIndex = 0; ctrlIndex < m_NetList[netIndex].CtrlList.size(); ctrlIndex++ )
            {
                if ( CheckControllerType ( netIndex, ctrlIndex, DEV_TEMPCTRL ) )
                {
                    ( ( CTempCtrl* ) ( m_NetList[netIndex].CtrlList[ctrlIndex] ) )->InitTempController();

                    //Check if the sensor has its own alarms, in this case don't set the default
                    if ( ( ( ( CTempCtrl* ) ( m_NetList[netIndex].CtrlList[ctrlIndex] ) )->GetMinAlarmLevel() > -100 ) &&
                            ( ( ( CTempCtrl* ) ( m_NetList[netIndex].CtrlList[ctrlIndex] ) )->GetMaxAlarmLevel() > -100 ) )
                    {
                        retVal = ( ( CTempCtrl* ) ( m_NetList[netIndex].CtrlList[ctrlIndex] ) )->AutoSetAlarmLevel();
                    }
                    else
                    {
                        retVal = ( ( CTempCtrl* ) ( m_NetList[netIndex].CtrlList[ctrlIndex] ) )->SetAlarmLevel ( m_NetList[netIndex].defAlMax, m_NetList[netIndex].defAlMin );
                    }
                }
            }
        }
        else
        {
            retVal = true;
        }
    }

    return retVal;
}

///////////////////////////////////////////////////
//             GETNETBYSERIAL
///////////////////////////////////////////////////
int COneWireNet::GetNetBySerial ( uchar * serial )
{
    int retVal = -1, i;

    //Cycle on all nets and search for the device
    for ( i = 0; i < m_NetList.size(); i++ )
    {
        if ( GetIndexBySerial ( i, serial ) != -1 )
        {
            //Device found: get net index
            retVal = i;
            break;
        }
    }

    return retVal;

}

///////////////////////////////////////////////////
//             GETNETBYSERIAL
///////////////////////////////////////////////////
int COneWireNet::GetNetBySerial ( CString serial )
{
    int retVal = -1, i;

    //Cycle on all nets and search for the device
    for ( i = 0; i < m_NetList.size(); i++ )
    {
        if ( GetIndexBySerial ( i, serial ) != -1 )
        {
            //Device found: get net index
            retVal = i;
            break;
        }
    }

    return retVal;
}

///////////////////////////////////////////////////
//             CHECKDEVICEFAMILY
///////////////////////////////////////////////////
bool COneWireNet::CheckDeviceFamily ( int netIndex, int devIndex, SMALLINT FN )
{
    if ( !CheckIndexes ( netIndex, devIndex ) )
    {
        return false;
    }


    if ( m_NetList[netIndex].deviceList[devIndex]->GetFamNum() == FN )
    {
        return true;
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////
//             CheckControllerType
///////////////////////////////////////////////////
bool COneWireNet::CheckControllerType ( int netIndex, int ctrlIndex, e_DeviceType type )
{
    if ( !CheckIndexes ( netIndex, ctrlIndex, false ) )
    {
        return false;
    }


    if ( m_NetList[netIndex].CtrlList[ctrlIndex]->GetControllerType() == type )
    {
        return true;
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////
//             CHANGEIPADDR
///////////////////////////////////////////////////
bool COneWireNet::ChangeIPAddr ( int netIndex )
{
    uchar configBuffer[128];
    bool retVal = false;
    uchar recvBuffer[32];
    int portNum;
    int counter = 0;
    int ByteRecv = 0;
    bool relOnExit = false; //Flag indicating if we have to release the port prior to exit


    memset ( recvBuffer, 0, 32*sizeof ( uchar ) );
    memset ( configBuffer, 0, 128*sizeof ( uchar ) );

    if ( !CheckNetIndex ( netIndex ) )
    {
        return false;
    }

    //Check if we already had business with this net
    if ( m_LastAddress == netIndex )
    {
        return true;
    }

    //Check if the port is already opened
    if ( !m_NetList[netIndex].isAcquired )
    {
        cout << "Apro la porta direttamente per il cambio di indirizzo"<<endl;
        portNum = m_NetList[netIndex].master->OpenPortDirect ( ( char* ) ( m_NetList[netIndex].netPortName.c_str() ) );
        m_NetList[netIndex].master->SetBaudCOMDirect ( portNum, PARMSET_9600 );
        relOnExit = true;

        if ( portNum < 0 )
        {
            PushError ( OWERROR_OPENCOM_FAILED, netIndex + 1 );
            return false;
        }
    }
    else
    {
        portNum = m_NetList[netIndex].portHandler;
    }


    if ( m_DoDebug )
        cout << "Inizio Programmazione indirizzo di destinazione..." << endl;cout.flush();

    //TODO da spostare da un'altra parte
    //Disabilito l'eco dei comandi
    sprintf ( ( char* ) configBuffer,"ATE0\r" );
    m_NetList[netIndex].master->WriteDirect2Port ( portNum, strlen ( ( char* ) configBuffer ), configBuffer );
    ByteRecv = m_NetList[netIndex].master->ReadPortDirect ( portNum, 32, recvBuffer );

//     try 5 times to program the device
    while ( counter < 5 )
    {
        memset ( recvBuffer, 0x00, 32 );

        //La sequenza per entrare in modo comando è la seguente: attendere guardtime dall'ultimo invio, mandare +++, attendere altro guardtime
        msDelay ( 500 );
        configBuffer[0] = configBuffer[1] = configBuffer [2] = '+';
        m_NetList[netIndex].master->WriteDirect2Port ( portNum, 3, configBuffer );
        msDelay ( 500 );
        ByteRecv = m_NetList[netIndex].master->ReadPortDirect ( portNum, 32, recvBuffer );


//         if (!ByteRecv)
//         {
//             if (m_DoDebug)
//             {
//                 cout << "Nessuna risposta al tentativo di entrare in modo comando" << endl;
//             }
//             continue;
//         }
//         else
//         {
//             //Aggiungo un finestringa
//             recvBuffer[ByteRecv] = 0x0;
// //             if (m_DoDebug)
// //             {
// //                 cout << "GT+CC+GT -- Numero Caratteri ricevuti: " << ByteRecv << " stringa estratta: " << recvBuffer << endl;
// //             }
//
//             if (!strstr((char*)recvBuffer, "OK"))
//             {
//                 if (m_DoDebug)
//                 {
//                     cout << "Non ho ricevuto il comando corretto" << endl;
//                 }
//
//                 continue;
//             }
//             else
//             {
//                 memset(recvBuffer, 0x00, 32);
//             }
//         }

        //Butto giu' la linea in ogni modo
        sprintf ( ( char* ) configBuffer, "ATH\r" );
        m_NetList[netIndex].master->WriteDirect2Port ( portNum, strlen ( ( char* ) configBuffer ), configBuffer );

        msDelay ( 100 );
        //Provo a leggere dalla seriale l'acknowledge (\r\nOK\r\n)
        ByteRecv = m_NetList[netIndex].master->ReadPortDirect ( portNum, 32, recvBuffer );


//         if (!ByteRecv)
//         {
//             if (m_DoDebug)
//             {
//                 cout << "ATH--Nessuna risposta al tentativo di programmare l'indirizzo basso" << endl;
//             }
//             continue;
//         }
//         else
//         {
//             //Aggiungo un finestringa
//             recvBuffer[ByteRecv] = 0x0;
//             if (m_DoDebug)
//             {
//                 cout << "ATH -- Numero Caratteri ricevuti: " << ByteRecv << " stringa estratta: " << recvBuffer << endl;
//             }
//
//             if (!strstr((char*)recvBuffer, "OK"))
//             {
//                 if (m_DoDebug)
//                 {
//                     cout << "ATH--Non ho ricevuto la risposta corretta" << endl;
//                 }
//
//                 continue;
//             }
//             else
//             {
//                 memset(recvBuffer, 0x00, 32);
//             }
//         }


        //Imposto l'indirizzo
        sprintf ( ( char* ) configBuffer, "AT+PRIP=%s\r",m_NetList[netIndex].ipAddress.c_str() );
        m_NetList[netIndex].master->WriteDirect2Port ( portNum, strlen ( ( char* ) configBuffer ), configBuffer );

        msDelay ( 100 );
        //Provo a leggere dalla seriale l'acknowledge (\r\nOK\r\n)
        ByteRecv = m_NetList[netIndex].master->ReadPortDirect ( portNum, 6, recvBuffer );

        if ( !ByteRecv )
        {
            if ( m_DoDebug )
            {
                cout << "AT+PRIP--Nessuna risposta al tentativo di programmare l'indirizzo basso" << endl;
            }
            continue;
        }
        else
        {
            //Aggiungo un finestringa
            recvBuffer[ByteRecv] = 0x0;
//             if (m_DoDebug)
//             {
//                 cout << "AT+PRIP -- Numero Caratteri ricevuti: " << ByteRecv << " stringa estratta: " << recvBuffer << endl;
//             }

            if ( !strstr ( ( char* ) recvBuffer, "OK" ) )
            {
                if ( m_DoDebug )
                {
                    cout << "AT+PRIP--Non ho ricevuto la risposta corretta" << endl;
                }

                continue;
            }
            else
            {
                memset ( recvBuffer, 0x00, 32 );
            }
        }

        sprintf ( ( char* ) configBuffer, "AT+PRP=%d\r",m_NetList[netIndex].ipPort );
        m_NetList[netIndex].master->WriteDirect2Port ( portNum, strlen ( ( char* ) configBuffer ), configBuffer );

        msDelay ( 100 );
        //Provo a leggere dalla seriale l'acknowledge (\r\nOK\r\n)
        ByteRecv = m_NetList[netIndex].master->ReadPortDirect ( portNum, 6, recvBuffer );

        if ( !ByteRecv )
        {
            if ( m_DoDebug )
            {
                cout << "AT+PRP--Nessuna risposta al tentativo di programmare l'indirizzo basso" << endl;
            }
            continue;
        }
        else
        {
            //Aggiungo un finestringa
            recvBuffer[ByteRecv] = 0x0;
//             if (m_DoDebug)
//             {
//                 cout << "AT+PRP -- Numero Caratteri ricevuti: " << ByteRecv << " stringa estratta: " << recvBuffer << endl;
//             }

            if ( !strstr ( ( char* ) recvBuffer, "OK" ) )
            {
                if ( m_DoDebug )
                {
                    cout << "AT+PRP--Non ho ricevuto la risposta corretta" << endl;
                }

                continue;
            }
            else
            {
                memset ( recvBuffer, 0x00, 32 );
            }
        }

        if ( m_DoDebug )
        {
            cout << "Esco dalla configurazione" << endl;
        }
        sprintf ( ( char* ) configBuffer, "ATDT\r" );

        m_NetList[netIndex].master->WriteDirect2Port ( portNum, strlen ( ( char* ) configBuffer ), configBuffer );

        msDelay ( 100 );
        ByteRecv = m_NetList[netIndex].master->ReadPortDirect ( portNum, 32, recvBuffer );

        if ( !ByteRecv )
        {
            if ( m_DoDebug )
            {
                cout << "ATDT--Nessuna risposta al tentativo di uscire dalla programmazione" << endl;
            }
            continue;
        }
        else
        {
            //Aggiungo un finestringa
            recvBuffer[ByteRecv] = 0x0;
//             if (m_DoDebug)
//             {
//                 cout << "ATDT -- Numero Caratteri ricevuti: " << ByteRecv << " stringa estratta: " << recvBuffer << endl;
//             }

            if ( !strstr ( ( char* ) recvBuffer, "CONNECT" ) )
            {
                if ( m_DoDebug )
                {
                    cout << "ATDT--Non ho ricevuto la risposta corretta" << endl;
                }

                continue;
            }
        }

//         Store current address for later use
        m_LastAddress = netIndex;

        if ( m_DoDebug )
            cout << "Modulo RC correttamente riprogrammato" << endl;

        retVal = true;
        break;
    }//While

//     TODO inserire errore se non riesce a comunicare

    if ( relOnExit )
    {
        m_NetList[netIndex].master->ClosePortDirect ( portNum );
    }

    return retVal;
}

///////////////////////////////////////////////////
//             CHANGEWLADDR
///////////////////////////////////////////////////
bool COneWireNet::ChangeWLAddr ( int netIndex )
{
    uchar configBuffer[128];
    bool retVal = false;
    uchar recvBuffer[32];
    int portNum;
    int counter = 0;
    int ByteRecv = 0;
    bool relOnExit = false; //Flag indicating if we have to release the port prior to exit


    memset ( recvBuffer, 0, 32*sizeof ( uchar ) );
    memset ( configBuffer, 0, 128*sizeof ( uchar ) );

    if ( !CheckNetIndex ( netIndex ) )
    {
        return false;
    }

    //Check if we already had business with this net
    if ( m_LastAddress == netIndex )
    {
        return true;
    }

    //Check if the port is already opened
    if ( !m_NetList[netIndex].isAcquired )
    {
        cout << "Apro la porta direttamente per il cambio di indirizzo"<<endl;
        portNum = m_NetList[netIndex].master->OpenPortDirect ( ( char* ) ( m_NetList[netIndex].netPortName.c_str() ) );
        m_NetList[netIndex].master->SetBaudCOMDirect ( portNum, PARMSET_9600 );
        relOnExit = true;

        if ( portNum < 0 )
        {
            PushError ( OWERROR_OPENCOM_FAILED, netIndex + 1 );
            return false;
        }
    }
    else
    {
        portNum = m_NetList[netIndex].portHandler;
    }


    if ( m_DoDebug )
        cout << "Inizio Programmazione indirizzo di destinazione..." << endl;cout.flush();

//     try 5 times to program the device
    while ( counter < 5 )
    {
        memset ( recvBuffer, 0x00, 32 );

        //La sequenza per entrare in modo comando è la seguente: attendere guardtime dall'ultimo invio, mandare +++, attendere altro guardtime
        msDelay ( XBEE_GUARDTIME );
        configBuffer[0] = configBuffer[1] = configBuffer [2] = '+';
        m_NetList[netIndex].master->WriteDirect2Port ( portNum, 3, configBuffer );
        msDelay ( XBEE_GUARDTIME );

        //Provo a leggere dalla seriale l'acknowledge (OK\r)
        ByteRecv = m_NetList[netIndex].master->ReadPortDirect ( portNum, 3, recvBuffer );

        if ( !ByteRecv )
        {
            if ( m_DoDebug )
            {
                cout << "Nessuna risposta al tentativo di entrare in modo comando" << endl;
            }
        }
        else
        {
            //Aggiungo un finestringa
            recvBuffer[ByteRecv] = 0x0;
//             if (m_DoDebug)
//             {
//                 cout << "GT+CC+GT -- Numero Caratteri ricevuti: " << ByteRecv << " stringa estratta: " << recvBuffer << endl;
//             }

            if ( strcasecmp ( ( char* ) recvBuffer, "OK\r" ) )
            {
//                 if (m_DoDebug)
//                 {
//                     cout << "Non ho ricevuto il comando corretto" << endl;
//                 }

                continue;
            }
        }

        //Imposto il byte basso degli indirzzi
        sprintf ( ( char* ) configBuffer, "ATDL%X\r",m_NetList[netIndex].wlAddr[1] );
        m_NetList[netIndex].master->WriteDirect2Port ( portNum, strlen ( ( char* ) configBuffer ), configBuffer );

        //Provo a leggere dalla seriale l'acknowledge (OK\r)
        ByteRecv = m_NetList[netIndex].master->ReadPortDirect ( portNum, 3, recvBuffer );

        if ( !ByteRecv )
        {
            if ( m_DoDebug )
            {
                cout << "ATDL--Nessuna risposta al tentativo di programmare l'indirizzo basso" << endl;
            }
        }
        else
        {
            //Aggiungo un finestringa
            recvBuffer[ByteRecv] = 0x0;
//             if (m_DoDebug)
//             {
//                 cout << "ATDL -- Numero Caratteri ricevuti: " << ByteRecv << " stringa estratta: " << recvBuffer << endl;
//             }

            if ( strcasecmp ( ( char* ) recvBuffer, "OK\r" ) )
            {
                if ( m_DoDebug )
                {
                    cout << "ATDL--Non ho ricevuto la risposta corretta" << endl;
                }

                continue;
            }
        }

//         if (m_DoDebug)
//         {
//             cout << "Esco dalla configurazione" << endl;
//         }
        sprintf ( ( char* ) configBuffer, "ATCN\r" );

        m_NetList[netIndex].master->WriteDirect2Port ( portNum, strlen ( ( char* ) configBuffer ), configBuffer );

        ByteRecv = m_NetList[netIndex].master->ReadPortDirect ( portNum, 3, recvBuffer );

        if ( !ByteRecv )
        {
            if ( m_DoDebug )
            {
                cout << "ATCN--Nessuna risposta al tentativo di uscire dalla programmazione" << endl;
            }
        }
        else
        {
            //Aggiungo un finestringa
            recvBuffer[ByteRecv] = 0x0;
//             if (m_DoDebug)
//             {
//                 cout << "ATCN -- Numero Caratteri ricevuti: " << ByteRecv << " stringa estratta: " << recvBuffer << endl;
//             }

            if ( strcasecmp ( ( char* ) recvBuffer, "OK\r" ) )
            {
                if ( m_DoDebug )
                {
                    cout << "ATCN--Non ho ricevuto la risposta corretta" << endl;
                }

                continue;
            }
        }

//         Store current address for later use
        m_LastAddress = netIndex;

        if ( m_DoDebug )
            cout << "Modulo RC correttamente riprogrammato" << endl;

        retVal = true;
        break;
    }//While

//     TODO inserire errore se non riesce a comunicare

    if ( relOnExit )
    {
        m_NetList[netIndex].master->ClosePortDirect ( portNum );
    }

    return retVal;
}


///////////////////////////////////////////////////
//             RCENTERPROGRAMMODE
///////////////////////////////////////////////////
bool COneWireNet::RCEnterProgramMode ( int netIndex )
{
    int fdg; //Handlers to all ports
    uchar exitChar = 'X';
    int portNum = 0;
    EtraxGPIO gpio;

    if ( ( fdg = open ( "/dev/gpiog", O_RDWR ) ) <0 )
    {
        printf ( "Open error on /dev/gpiog\n" );
        return false;
    }


    //This pins must be accessed in group
    gpio.init_output ( fdg, LINE_8 | LINE_9 | LINE_10 | LINE_11 | LINE_12 | LINE_13 | LINE_14 | LINE_15 );
    gpio.init_output ( fdg, LED_1 );

    //Use line 13 for reprogramming the device and line 14 to send a reset
    gpio.set_output ( fdg, LINE_13 | LED_1 );
    gpio.set_output ( fdg, LINE_14 );

    //Get the handler to the port and send to the module the X char to force it to exit a previous
    //programming state
    portNum = m_NetList[netIndex].portHandler;
    m_NetList[netIndex].master->WriteDirect2Port ( portNum, 1, &exitChar );

    msDelay ( 200 );

    //FIXME provare a mettere la gestione del RESET
    //Send RESET
    //Clear RESET
    gpio.clear_output ( fdg, LINE_14 );

    msDelay ( 50 );

    gpio.set_output ( fdg, LINE_14 );


    //Per il config viene mosso il piedino 6 del connettore JP8
    //Low the pins
    gpio.clear_output ( fdg,  LINE_13 | LED_1 );

    //wait a bit
    msDelay ( 500 );

    //Raise the pins
    gpio.set_output ( fdg, LINE_13 | LED_1 );

    //Close port
    close ( fdg );

    return true;
}

///////////////////////////////////////////////////
//             SETSETUPSTATE
///////////////////////////////////////////////////
void COneWireNet::SetSetupState ( int netindex, bool setupState )
{
    if ( CheckNetIndex ( netindex ) )
    {
        m_NetList[netindex].correctlySet = setupState;
    }
}

///////////////////////////////////////////////////
//             GETSETUPSTATE
///////////////////////////////////////////////////
bool COneWireNet::GetSetupState ( int netIndex )
{
    if ( CheckNetIndex ( netIndex ) )
    {
        return m_NetList[netIndex].correctlySet;
    }
    else
    {
        //Default return value
        return false;
    }
}

///////////////////////////////////////////////////
//             GETNETALARMSTATE
///////////////////////////////////////////////////
bool COneWireNet::GetNetAlarmState ( int netIndex )
{
    if ( CheckNetIndex ( netIndex ) )
    {
        return m_NetList[netIndex].areAlarmsEnabled;
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////
//             UpdateTempAlarmState
///////////////////////////////////////////////////
void COneWireNet::UpdateTempAlarmState ( int netIndex, int * sensorList, int numSensors )
{
    int devIndex = 0;
    int sensorIndex = 0;

    //For each sensor on the NET check if it is a temperature sensor and if it has alarms and set the alarm state
    for ( devIndex = 0; devIndex < m_NetList[netIndex].CtrlList.size(); devIndex++ )
    {
        //Check if it is a temperature sensor
        if ( CheckControllerType ( netIndex, devIndex, DEV_TEMPCTRL ) )
        {
            //Cycle through the array containing the sensors in alarm
            for ( sensorIndex = 0; sensorIndex < numSensors; sensorIndex++ )
            {
                if ( sensorList[sensorIndex] == devIndex )
                {
                    break;
                }
            }

            if ( sensorIndex == numSensors )
            {
                //The selected device is not on the list, clear its alarm state
                ( ( CTempCtrl* ) ( m_NetList[netIndex].deviceList[devIndex] ) )->ClearAlarmState();
            }
            else
            {
                //Device found, set the alarm state
                ( ( CTempCtrl* ) ( m_NetList[netIndex].deviceList[devIndex] ) )->SetAlarmState();
            }
        }
    }
}

///////////////////////////////////////////////////
//             InitializeSwitches
///////////////////////////////////////////////////
bool COneWireNet::InitializeDOs ( int netIndex )
{
    bool retVal = true;
    int devIndex = 0;
    float switchTime = -1;
    char buffer[8];

    if ( !CheckNetIndex ( netIndex ) )
    {
        return false;
    }

    //Setup Switch Time
    sprintf ( buffer,"NET%d", netIndex+1 );
    switchTime = m_IniFile->GetFloat ( Config_Strings[CONF_NETDIGOUTDELAY], buffer );

    //Cycle through all devices to search for switches
    for ( devIndex = 0; devIndex < m_NetList[netIndex].CtrlList.size(); devIndex++ )
    {
        if ( CheckControllerType ( netIndex, devIndex, DEV_DIDO ) )
        {
            if ( switchTime != FLT_MIN )
            {
                ( ( CDigitalIO* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetSwitchTime ( ( int ) ( switchTime*1000 ) );
            }
            else
            {
                ( ( CDigitalIO* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetSwitchTime ( 0 );
            }

            //Setup the I/O channels
            if ( ! ( ( CDigitalIO* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->InitDevice() )
            {
                if ( m_DoDebug )
                {
                    cout<<"ONEWIRENET: Errore!! Setup impossibile per il DIDO di indirizzo: " << m_NetList[netIndex].CtrlList[devIndex]->GetMemoryAddress() <<endl;
                }

                retVal = false;
            }
            else
            {
                cout << "DIDO indirizzo :" << m_NetList[netIndex].CtrlList[devIndex]->GetMemoryAddress() << " Settato" << endl;
            }
        }
        else if ( ( CheckControllerType ( netIndex, devIndex, DEV_BUTTONCTRL ) ) || ( CheckControllerType ( netIndex, devIndex, DEV_STEPDIGITALOUT ) ) )
        {

            if ( switchTime != FLT_MIN )
            {
                ( ( CVMultiDIDO* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetSwitchTime ( ( int ) ( switchTime*1000 ) );
            }
            else
            {
                ( ( CVMultiDIDO* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetSwitchTime ( 0 );
            }

            //Setup the I/O channels
            if ( ! ( ( CVMultiDIDO* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->InitOutput() )
            {
                if ( m_DoDebug )
                {
                    cout<<"ONEWIRENET: Errore!! Setup impossibile per il MultiDIDO di indirizzo: " << m_NetList[netIndex].CtrlList[devIndex]->GetMemoryAddress() <<endl;
                }

                retVal = false;
            }
            else
            {
                cout << "MultiDIDO indirizzo :" << m_NetList[netIndex].CtrlList[devIndex]->GetMemoryAddress() << " Settato" << endl;
            }
        }
    }

    for ( devIndex = 0; devIndex < m_NetList[netIndex].deviceList.size(); devIndex++ )
    {
        if ( CheckDeviceFamily ( netIndex, devIndex, DS2408_FN ) )
        {
            cout<<"Inizializzo driver DIDO numero: "<< m_NetList[netIndex].deviceList[devIndex]->GetConfigFileDevIndex() <<endl;
            ( ( CDS2408* ) ( m_NetList[netIndex].deviceList[devIndex] ) )->InitDevice();
        }
        else if ( CheckDeviceFamily ( netIndex, devIndex, DS2405_FN ) )
        {
            //TODO da implementare
        }
    }

    return retVal;

}

///////////////////////////////////////////////////
//             InitializeAOs
///////////////////////////////////////////////////
bool COneWireNet::InitializeAOs ( int netIndex )
{
    bool retVal = true;
    int devIndex = 0;

    if ( !CheckNetIndex ( netIndex ) )
    {
        return false;
    }

    //Cycle through all devices to search for analog AIAO
    for ( devIndex = 0; devIndex < m_NetList[netIndex].CtrlList.size(); devIndex++ )
    {
        if ( CheckControllerType ( netIndex, devIndex, DEV_AIAO ) )
        {
            if ( ! ( ( CAnalogIO* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->IsInput() )
            {
                //Setup the Output channels
                retVal = ( ( CAnalogIO* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->InitDevice() && retVal;
            }
        }
    }

    return retVal;

}

///////////////////////////////////////////////////
//             InitializeCoordinators
///////////////////////////////////////////////////
bool COneWireNet::InitializeCoordinators ( int netIndex )
{
    bool retVal = true;
    int ctrlIndex = 0;

    if ( !CheckNetIndex ( netIndex ) )
    {
        return false;
    }

    for ( ctrlIndex = 0; ctrlIndex < m_NetList[netIndex].CtrlList.size(); ctrlIndex++ )
    {
        if ( CheckControllerType ( netIndex, ctrlIndex, DEV_ALARMCTRL ) )
        {
            retVal = ( ( AlarmCoordinator* ) ( m_NetList[netIndex].CtrlList[ctrlIndex] ) )->InitAlarmSystem();
        }
        else if ( CheckControllerType ( netIndex, ctrlIndex, DEV_CHOVER ) )
        {
            retVal = ( ( ChangeOverCoord* ) ( m_NetList[netIndex].CtrlList[ctrlIndex] ) )->InitChangeOver();
        }
    }

    return retVal;
}

///////////////////////////////////////////////////
//             InitializeAdvancedControllers
///////////////////////////////////////////////////
bool COneWireNet::InitializeAdvancedControllers ( int netIndex )
{
    bool retVal = true;
    int ctrlIndex = 0;

    if ( !CheckNetIndex ( netIndex ) )
    {
        return false;
    }

    //Cycle through all devices to search for analog AIAO
    for ( ctrlIndex = 0; ctrlIndex < m_NetList[netIndex].CtrlList.size(); ctrlIndex++ )
    {
        if ( CheckControllerType ( netIndex, ctrlIndex, DEV_VLVCTRL ) )
        {
            //Setup the device
#ifndef USE_ADV_VLV
            retVal = ( ( CNTHVLV* ) ( m_NetList[netIndex].CtrlList[ctrlIndex] ) )->InitDevice();
#else
            retVal = ( ( CNTHVLV_ADV* ) ( m_NetList[netIndex].CtrlList[ctrlIndex] ) )->InitDevice();
#endif

            if ( m_DoDebug )
            {
                cout << "Controllo valvole numero " << m_NetList[netIndex].CtrlList[ctrlIndex]->GetConfigFileDevIndex();
                if ( !retVal )
                {
                    cout << " NON";
                }

                cout << " inizializzato correttamente" << endl;
            }
        }
        else if ( CheckControllerType ( netIndex, ctrlIndex, DEV_UTACTRL ) )
        {
            //Setup the device
            retVal = ( ( CUtaCtrl* ) ( m_NetList[netIndex].CtrlList[ctrlIndex] ) )->Init();

            if ( m_DoDebug )
            {
                cout << "Controllo UTA " << m_NetList[netIndex].CtrlList[ctrlIndex]->GetConfigFileDevIndex();
                if ( !retVal )
                {
                    cout << " NON";
                }

                cout << " inizializzato correttamente" << endl;
            }
        }
        else if ( CheckControllerType ( netIndex, ctrlIndex, DEV_AFOVLV ) )
        {
            //Setup the device
            retVal = ( ( CNTHVLV2* ) ( m_NetList[netIndex].CtrlList[ctrlIndex] ) )->InitDevice();

            if ( m_DoDebug )
            {
                cout << "Controllo Valvole(2) " << m_NetList[netIndex].CtrlList[ctrlIndex]->GetConfigFileDevIndex();
                if ( !retVal )
                {
                    cout << " NON";
                }

                cout << " inizializzato correttamente" << endl;
            }
        }
        else if ( CheckControllerType ( netIndex, ctrlIndex, DEV_AFOVLV_VAV ) )
        {
            //Setup the device
            retVal = ( ( CNTHVLV2_VAV* ) ( m_NetList[netIndex].CtrlList[ctrlIndex] ) )->InitDevice();

            if ( m_DoDebug )
            {
                cout << "Controllo VAV " << m_NetList[netIndex].CtrlList[ctrlIndex]->GetConfigFileDevIndex();
                if ( !retVal )
                {
                    cout << " NON";
                }

                cout << " inizializzato correttamente" << endl;
            }
        }
        else if ( CheckControllerType ( netIndex, ctrlIndex, DEV_TIMEMARKER ) )
        {
            //Setup the device
            retVal = ( ( CTimeMarkerCtrl* ) ( m_NetList[netIndex].CtrlList[ctrlIndex] ) )->InitDevice();

            if ( m_DoDebug )
            {
                cout << "Marcatempo " << m_NetList[netIndex].CtrlList[ctrlIndex]->GetConfigFileDevIndex();
                if ( !retVal )
                {
                    cout << " NON";
                }

                cout << " inizializzato correttamente" << endl;
            }
        }
    }

    return retVal;
}

///////////////////////////////////////////////////
//             InitializeACC
///////////////////////////////////////////////////
bool COneWireNet::InitializeACC ( int netIndex)
{
    bool retVal = true;
    int ctrlIndex = 0;


    if ( !CheckNetIndex ( netIndex ) )
    {
        return false;
    }

    //Cycle through all devices to search for ACC
    for ( ctrlIndex = 0; ctrlIndex < m_NetList[netIndex].CtrlList.size(); ctrlIndex++ )
    {
        if ( CheckControllerType ( netIndex, ctrlIndex, DEV_ACC ) )
        {
            if (m_DoDebug)
            {
                cout<< "Inizializzo NTH-AFO-ACC  numero: " << m_NetList[netIndex].CtrlList[ctrlIndex]->GetConfigFileDevIndex() <<endl;
            }

            //Setup the device
            retVal = ( ( CNTH_ACC* ) ( m_NetList[netIndex].CtrlList[ctrlIndex] ) )->InitDevice ( ) && retVal;

            if ( m_DoDebug )
            {
                cout << "Modulo CACC numero " << m_NetList[netIndex].CtrlList[ctrlIndex]->GetConfigFileDevIndex();
                if ( !retVal )
                {
                    cout << " NON";
                }

                cout << " inizializzato correttamente" << endl;
            }
        }
    }

    return retVal;

}

///////////////////////////////////////////////////
//             InitializeMGC
///////////////////////////////////////////////////
bool COneWireNet::InitializeMGC ( int netIndex, bool skipInit )
{
    bool retVal = false;

    int ctrlIndex = 0;

    if ( !CheckNetIndex ( netIndex ) )
    {
        return false;
    }

    //Cycle through all devices to search for MGC
    for ( ctrlIndex = 0; ctrlIndex < m_NetList[netIndex].CtrlList.size(); ctrlIndex++ )
    {
        if ( CheckControllerType ( netIndex, ctrlIndex, DEV_MGC ) )
        {
            //TBR
            cout<< "Inizializzo NTH-AFO-MGC  numero: " << m_NetList[netIndex].CtrlList[ctrlIndex]->GetConfigFileDevIndex() <<endl;

            if ( skipInit )
            {
                //Evito tutta l'inizializzazione
                retVal = true;
                ( ( CNTHMGC* ) ( m_NetList[netIndex].CtrlList[ctrlIndex] ) )->m_InitOk = true;
            }
            else
            {
                //Setup the device
                retVal = ( ( CNTHMGC* ) ( m_NetList[netIndex].CtrlList[ctrlIndex] ) )->InitDevice ( );
            }


            if ( m_DoDebug )
            {
                cout << "Modulo della Camera numero " << ( ( CNTHMGC* ) ( m_NetList[netIndex].CtrlList[ctrlIndex] ) )->m_RoomNumber;
                if ( !retVal )
                {
                    cout << " NON";
                }

                cout << " inizializzato correttamente" << endl;
            }
        }
    }

    return retVal;
}

///////////////////////////////////////////////////
//             InitializeMGC
///////////////////////////////////////////////////
bool COneWireNet::InitializeCNT ( int netIndex )
{
    bool retVal = true;

    int ctrlIndex = 0;

    if ( !CheckNetIndex ( netIndex ) )
    {
        return false;
    }

    //Cycle through all devices to search for CNT
    for ( ctrlIndex = 0; ctrlIndex < m_NetList[netIndex].CtrlList.size(); ctrlIndex++ )
    {
        if ( CheckControllerType ( netIndex, ctrlIndex, DEV_CNT ) )
        {
            //TBR
            cout<< "Inizializzo NTH-AFO-CNT  numero: " << m_NetList[netIndex].CtrlList[ctrlIndex]->GetConfigFileDevIndex() <<endl;

            //Passo l'handler
            ( ( CNTH_CNT* ) ( m_NetList[netIndex].CtrlList[ctrlIndex] ) )->setNetPtr((void*)this);
            //Setup the device
            retVal = ( ( CNTH_CNT* ) ( m_NetList[netIndex].CtrlList[ctrlIndex] ) )->InitDevice() && retVal;

            if ( m_DoDebug )
            {
                cout << "Modulo Contabilizzatore numero " << m_NetList[netIndex].CtrlList[ctrlIndex]->GetConfigFileDevIndex();
                if ( !retVal )
                {
                    cout << " NON";
                }

                cout << " inizializzato correttamente" << endl;
            }
        }
    }

    return retVal;
}

///////////////////////////////////////////////////
//             GetAllLastTemp
///////////////////////////////////////////////////
int COneWireNet::GetAllLastTemp ( int netIndex, float dest[][2], int maxNum )
{
    int retVal = 0, devIndex = 0, stopVal = 0;

    //Usual Check
    if ( ( !CheckIndexes ( netIndex, devIndex, false ) ) || ( dest == NULL ) || ( maxNum < 0 ) )
    {
        retVal = -1;
    }

    if ( maxNum < m_NetList[netIndex].CtrlList.size() )
    {
        stopVal = maxNum;
    }
    else
    {
        stopVal = m_NetList[netIndex].CtrlList.size();
    }

    if ( retVal != -1 )
    {
        //Start cycle
        for ( devIndex = 0;  devIndex < stopVal; devIndex++ )
        {
            //Check if the given device is a T sensor
            if ( CheckControllerType ( netIndex, devIndex, DEV_TEMPCTRL ) )
            {
                //Update the device index
                dest[retVal][0] = devIndex;
                dest[retVal][1] = ( ( CTempCtrl* ) ( m_NetList[netIndex].deviceList[devIndex] ) )->GetLastTemp();

                retVal++;
            }
        }
    }

    return retVal;

}

///////////////////////////////////////////////////
//             GetNethandler
///////////////////////////////////////////////////
T_Net * COneWireNet::GetNetHandler ( int netIndex )
{
    if ( (netIndex >= 0) && (netIndex <= m_NetList.size()) )
    {
        return &m_NetList[netIndex];
    }
    else
    {
        return 0x0;
    }
}

///////////////////////////////////////////////////
//             GetDeviceIndexByMemoryAddress
///////////////////////////////////////////////////
int COneWireNet::GetDeviceIndexByMemoryAddress ( int netIndex, int address )
{
    int retVal = -1, i = 0;
    int devMemoryAddress = 0;


    //Usual Check
    if ( !CheckNetIndex ( netIndex ) )
    {
        return retVal;
    }

    for ( i = 0; i < m_NetList[netIndex].CtrlList.size(); i++ )
    {
        devMemoryAddress = m_NetList[netIndex].CtrlList[i]->GetMemoryAddress();

        if ( devMemoryAddress == address )
        {
            retVal = i;
            break;
        }
    }

    return retVal;
}

///////////////////////////////////////////////////
//             GetNetByMemoryAddress
///////////////////////////////////////////////////
int COneWireNet::GetNetByMemoryAddress ( int memoryAddress )
{
    int retVal = -1, i;

    //Cycle on all nets and search for the device
    for ( i = 0; i < m_NetList.size(); i++ )
    {
        if ( GetDeviceIndexByMemoryAddress ( i, memoryAddress ) != -1 )
        {
            //Device found: get net index
            retVal = i;
            break;
        }
    }

    return retVal;
}

///////////////////////////////////////////////////
//             SetPotentiometerValue
////////////////////////////////////////////////
bool COneWireNet::SetAnalogOutput ( int netIndex, int devIndex, uchar newPosition )
{
    bool retVal = false;
    int fileDevIndex;
    char buffer[8];
    CString newPos;


    //Usual Check
    if ( !CheckIndexes ( netIndex, devIndex, false ) )
    {
        return retVal;
    }


    retVal = ( ( CAnalogIO* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetPosition ( newPosition );

    if ( retVal )
    {
        fileDevIndex = m_NetList[netIndex].CtrlList[devIndex]->GetConfigFileDevIndex();
        sprintf ( buffer,"%d",newPosition );
        newPos = buffer;
        UpdateIniFile ( netIndex, fileDevIndex, "STARTPOS", newPos );
    }

    return retVal;
}

///////////////////////////////////////////////////
//             CheckIndexes
////////////////////////////////////////////////
bool COneWireNet::CheckIndexes ( int netIndex, int devIndex, bool isDevice )
{
    bool retVal = false;

    if ( ( netIndex >= 0 ) && ( netIndex < m_NetList.size() ) )
    {
        if ( isDevice )
        {
            if ( ( devIndex >= 0 ) && ( devIndex < m_NetList[netIndex].deviceList.size() ) )
            {
                retVal = true;
            }
        }
        else
        {
            if ( ( devIndex >= 0 ) && ( devIndex < m_NetList[netIndex].CtrlList.size() ) )
            {
                retVal = true;
            }
        }
    }

    return retVal;

}

///////////////////////////////////////////////////
//             GetDigitalInput
////////////////////////////////////////////////
int COneWireNet::GetDigitalInput ( int memoryAddress )
{

    int devIndex=-1, netIndex=-1;
    int retVal = -1;
    bool state = false;

    //First get netIndex and deviceIndex from the memory address
    netIndex = GetNetByMemoryAddress ( memoryAddress );

    if ( netIndex > -1 )
    {
        devIndex = GetDeviceIndexByMemoryAddress ( netIndex, memoryAddress );

        if ( devIndex > -1 )
        {
            //Get Value of digital Input
            state = ( ( CDigitalIO* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->GetState();
            if ( state )
            {
                retVal = 1;
            }
            else
            {
                retVal = 0;
            }
        }
    }

    return retVal;
}


///////////////////////////////////////////////////
//             SetDigitalOutput
///////////////////////////////////////////////////
bool COneWireNet::SetDigitalOutput ( int memoryAddress, bool turnOn )
{
    int netIndex = -1, devIndex = -1, fileDevIndex = -1;
    bool retVal = false;
    CString newVal;

    //First get the index of the NET and of the device
    netIndex = GetNetByMemoryAddress ( memoryAddress );

    if ( netIndex > -1 )
    {
        //Then get the switch index inside the NET
        devIndex = GetDeviceIndexByMemoryAddress ( netIndex, memoryAddress );

        if ( devIndex > -1 )
        {
            //Last but not least set the switch
            if ( CheckControllerType ( netIndex, devIndex, DEV_DIDO ) )
            {
                retVal = ( ( CDigitalIO* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetState ( turnOn, false );
            }
            else if ( CheckControllerType ( netIndex, devIndex, DEV_STEPDIGITALOUT ) || CheckControllerType ( netIndex, devIndex, DEV_BUTTONCTRL ) )
            {
                retVal = ( ( CVMultiDIDO* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetOutput ( turnOn );
            }

        }
    }

    if (( retVal ) && ( m_NetList[netIndex].saveDigitalState))
    {
        //Update the ini file
        fileDevIndex = m_NetList[netIndex].CtrlList[devIndex]->GetConfigFileDevIndex();

        if ( turnOn )
        {
            newVal = "1";
        }
        else
        {
            newVal = "0";
        }

        UpdateIniFile ( netIndex, fileDevIndex, "STARTV", newVal );
    }
    return retVal;
}

///////////////////////////////////////////////////
//             SetAllDigitalInputs
///////////////////////////////////////////////////
bool COneWireNet::SetAllDigitalOutputs ( int netIndex, bool newState )
{
    bool retVal = true;
    int devIndex = 0;
    int memoryAddress = 0;

    //Usual Check
    if ( !CheckNetIndex ( netIndex ) )
    {
        return false;
    }

    for ( devIndex = 0; devIndex < m_NetList[netIndex].CtrlList.size(); devIndex++ )
    {
        memoryAddress = GetControllerMemoryAddress ( netIndex, devIndex );

        if ( memoryAddress < 0 )
        {
            continue;
        }

        //Check if it is the device we are searching for
        if ( CheckControllerType ( netIndex, devIndex, DEV_DIDO ) )
        {
            if ( ! ( ( CDigitalIO* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->IsInput() )
            {
                retVal = SetDigitalOutput ( memoryAddress, newState )  && retVal;
            }
        }

        //Disabilito questa parte di comando perche' rischia di essere troppo "pesante" come nell'impianto FIEGI
//         else if (CheckControllerType( netIndex, devIndex, DEV_STEPDIGITALOUT) || CheckControllerType( netIndex, devIndex, DEV_BUTTONCTRL))
//         {
//             retVal = SetDigitalOutput(memoryAddress, newState)  && retVal;
//         }

    }

    return retVal;

}


///////////////////////////////////////////////////
//             SetAllAnalogOutput
///////////////////////////////////////////////////
bool COneWireNet::SetAllAnalogOutput ( int netIndex, uchar newPos )
{
    bool retVal = true;
    int devIndex = 0;

    //Usual Check
    if ( !CheckNetIndex ( netIndex ) )
    {
        return false;
    }

    for ( devIndex = 0; devIndex < m_NetList[netIndex].CtrlList.size(); devIndex++ )
    {
        //Check if it is the device we are searching for
        if ( CheckControllerType ( netIndex, devIndex, DEV_AIAO ) )
        {
            SetAnalogOutput ( netIndex, devIndex, newPos );
        }
    }

    retVal = true;

    return retVal;
}

///////////////////////////////////////////////////
//             GetNofDevices
///////////////////////////////////////////////////
int COneWireNet::GetNofDevices ( int netIndex )
{
    int nOfDevices = -1;

    if ( CheckNetIndex ( netIndex ) )
    {
        nOfDevices = m_NetList[netIndex].deviceList.size();
    }

    return nOfDevices;
}

///////////////////////////////////////////////////
//             GetNofControllers
///////////////////////////////////////////////////
int COneWireNet::GetNofControllers ( int netIndex )
{
    int nOfControllers = -1;

    if ( CheckNetIndex ( netIndex ) )
    {
        nOfControllers = m_NetList[netIndex].CtrlList.size();
    }

    return nOfControllers;
}

///////////////////////////////////////////////////
//             GetAllPotentiometers
///////////////////////////////////////////////////
int COneWireNet::GetAllAIAO ( int netIndex, float posArray[][2], int maxNum )
{
    int retVal = 0, index = 0, stopVal = 0;

    //Usual Check
    if ( ( !CheckNetIndex ( netIndex ) ) || ( posArray == NULL ) || ( maxNum < 0 ) )
    {
        retVal = -1;
    }


    //Check ohw many devices we can read
    if ( maxNum < m_NetList[netIndex].CtrlList.size() )
    {
        stopVal = maxNum;
    }
    else
    {
        stopVal = m_NetList[netIndex].CtrlList.size();
    }

    if ( retVal != -1 )
    {
        //Start cycle
        for ( index = 0;  index < stopVal; index++ )
        {
            //Check if the given device is a T sensor
            if ( CheckControllerType ( netIndex, index, DEV_AIAO ) )
            {
                //Update the device index
                posArray[retVal][0] = index;
                posArray[retVal][1] = ( ( CAnalogIO* ) ( m_NetList[netIndex].CtrlList[index] ) )->GetValue(true);

                if ( posArray[retVal][1] >= 0.0 )
                {
                    retVal++;
                }
                else
                {
                    if ( m_DoDebug )
                    {
                        cout << "Attenzione!! Errore nella lettura AnalogIO di indirizzo : " << m_NetList[netIndex].CtrlList[index]->GetMemoryAddress();
//                         OWERROR_DUMP(stdout);
                    }
                }
            }
        }
    }

    return retVal;
}

///////////////////////////////////////////////////
//             SetPIDParameters
///////////////////////////////////////////////////
bool COneWireNet::SetPIDParameters ( int address, float * parameters, int nOfParameters )
{
    int netIndex = 0;
    int pidIndex = 0;
    int devIndex = 0;
    bool retVal = false;
    CString newVal;
    CString paramString[6];
    int i;

    netIndex = GetNetByMemoryAddress ( address );
    pidIndex = GetDeviceIndexByMemoryAddress ( netIndex, address );

    if ( ( parameters == 0x0 ) || ( netIndex == -1 ) || ( pidIndex == -1 ) )
    {
        return false;
    }

    if ( !CheckIndexes ( netIndex, pidIndex, false ) )
    {
        return false;
    }
    else if ( CheckControllerType ( netIndex, pidIndex,DEV_PIDSIMPLE ) || CheckControllerType ( netIndex, pidIndex, DEV_PIDLMD ) )
    {
        retVal = ( ( CVPID* ) ( m_NetList[netIndex].CtrlList[pidIndex] ) )->SetParameters ( parameters, nOfParameters );
    }
    else if ( CheckControllerType ( netIndex, pidIndex,DEV_UTACTRL ) )
    {
        retVal = ( ( CUtaCtrl* ) ( m_NetList[netIndex].CtrlList[pidIndex] ) )->SetPIDParameters ( parameters, nOfParameters );
    }
    else if ( CheckControllerType ( netIndex, pidIndex,DEV_MGC ) )
    {
        retVal = ( ( CNTHMGC* ) ( m_NetList[netIndex].CtrlList[pidIndex] ) )->SetPIDParam ( parameters, ( int ) parameters[4] );
    }

    //Save parameters in INI file
    if ( retVal )
    {
        devIndex = m_NetList[netIndex].CtrlList[pidIndex]->GetConfigFileDevIndex();

        if ( CheckControllerType ( netIndex, pidIndex, DEV_PIDSIMPLE ) )
        {
            paramString[0] = "KP";
            paramString[1] = "Tint";
            paramString[2] = "Tder";
        }
        else if ( CheckControllerType ( netIndex, pidIndex, DEV_PIDLMD ) )
        {
            paramString[0] = "KP1";
            paramString[1] = "Tint1";
            paramString[2] = "Tder1";

            paramString[3] = "KP2";
            paramString[4] = "Tint2";
            paramString[5] = "Tder2";
        }
        else if ( CheckControllerType ( netIndex, pidIndex, DEV_UTACTRL ) )
        {
            paramString[0] = "KP1";
            paramString[1] = "TI1";
            paramString[2] = "TD1";

            paramString[3] = "KP2";
            paramString[4] = "TI2";
            paramString[5] = "TD2";

        }

        for ( i = 0; i<nOfParameters; i++ )
        {
            newVal ="";
            newVal+=parameters[i];
            UpdateIniFile ( netIndex, devIndex, paramString[i].c_str(), newVal );
        }
    }

    return retVal;
}

///////////////////////////////////////////////////
//             SetAllPIDSetpoint
///////////////////////////////////////////////////
bool COneWireNet::SetAllPIDSetpoint ( int netIndex, float *setPoints, int nOfSetPoints, CXMLUtil *xmlParser )
{
    bool retVal = true;
    int pidIndex = -1;

    if ( !CheckNetIndex ( netIndex ) )
    {
        return false;
    }

    for ( pidIndex = 0; pidIndex < m_NetList[netIndex].CtrlList.size(); pidIndex++ )
    {

        retVal = SetSetpoint ( netIndex, pidIndex, setPoints, nOfSetPoints,  xmlParser) && retVal;

    }

    return retVal;
}

///////////////////////////////////////////////////
//             SetAllPIDParameters
///////////////////////////////////////////////////
bool COneWireNet::SetAllPIDParameters ( int netIndex, float *parameters, int nOfParameters )
{
    bool retVal = true;
    int pidIndex = -1;
    int address = -1;

    if ( !CheckNetIndex ( netIndex ) || ( parameters == 0x0 ) )
    {
        return false;
    }

    for ( pidIndex = 0; pidIndex < m_NetList[netIndex].CtrlList.size(); pidIndex++ )
    {
        if ( CheckControllerType ( netIndex, pidIndex, DEV_PIDSIMPLE ) || CheckControllerType ( netIndex, pidIndex, DEV_PIDLMD ) || CheckControllerType ( netIndex, pidIndex, DEV_UTACTRL ) )
        {
            address = m_NetList[netIndex].CtrlList[pidIndex]->GetMemoryAddress();
            retVal = SetPIDParameters ( address, parameters, nOfParameters ) && retVal;
        }
    }

    return retVal;
}

///////////////////////////////////////////////////
//             SetPIDSetpoint
///////////////////////////////////////////////////
bool COneWireNet::SetSetpoint ( int address, float * parameters, int nOfVals, CXMLUtil *xmlParser )
{
    int netIndex, devIndex;

    netIndex = GetNetByMemoryAddress ( address );
    devIndex = GetDeviceIndexByMemoryAddress ( netIndex, address );

    if ( !CheckIndexes ( netIndex, devIndex, false ) )
    {
        return false;
    }

    return SetSetpoint ( netIndex, devIndex, parameters, nOfVals, xmlParser );
}

///////////////////////////////////////////////////
//             SetPIDSetpoint
///////////////////////////////////////////////////
bool COneWireNet::SetSetpoint ( int netIndex, int devIndex, float * parameters, int nOfVals, CXMLUtil *xmlParser )
{
    bool retVal = false;
    int configDevIndex = 0;
    char tempString[8];

    if ( parameters == 0x0 )
    {
        return false;
    }

    if ( !CheckIndexes ( netIndex, devIndex, false ) )
    {
        return false;
    }
    else if ( CheckControllerType ( netIndex, devIndex,DEV_PIDSIMPLE ) || CheckControllerType ( netIndex, devIndex, DEV_PIDLMD ) )
    {
        retVal = ( ( CVPID* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetSetPoint ( parameters, nOfVals );

        if ( retVal )
        {
            //Update the INI file
            //Get the index of the device in the configuration file
            configDevIndex = m_NetList[netIndex].CtrlList[devIndex]->GetConfigFileDevIndex();

            sprintf ( tempString, "%2.1f", parameters[0] );
            UpdateIniFile ( netIndex, configDevIndex, "SETPOINT", tempString );

            if ( nOfVals > 1 )
            {
                sprintf ( tempString, "%2.1f", parameters[1] );
                UpdateIniFile ( netIndex, configDevIndex, "SETPOINTH", tempString );

                sprintf ( tempString, "%2.1f", parameters[2] );
                UpdateIniFile ( netIndex, configDevIndex, "SETPOINTL", tempString );
            }
        }
    }
    else if ( CheckControllerType ( netIndex, devIndex,DEV_UTACTRL ) )
    {
        retVal = ( ( CUtaCtrl* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetPIDSetPoint ( parameters, nOfVals );

        if ( retVal )
        {
            //Update the INI file
            //Get the index of the device in the configuration file
            configDevIndex = m_NetList[netIndex].CtrlList[devIndex]->GetConfigFileDevIndex();

            sprintf ( tempString, "%2.1f", parameters[0] );
            UpdateIniFile ( netIndex, configDevIndex, "SP", tempString );

            if ( nOfVals > 1 )
            {
                sprintf ( tempString, "%2.1f", parameters[1] );
                UpdateIniFile ( netIndex, configDevIndex, "SPH", tempString );

                sprintf ( tempString, "%2.1f", parameters[2] );
                UpdateIniFile ( netIndex, configDevIndex, "SPL", tempString );
            }
        }
    }
    else if ( CheckControllerType ( netIndex, devIndex, DEV_MGC ) )
    {
        retVal = ( ( CNTHMGC* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetRoomSetPoint ( parameters[0] );
    }
    else if ( CheckControllerType ( netIndex, devIndex, DEV_AFOVLV ) )
    {
        // Modificato il 6/11/2009 perchè altrimenti cambiava solo il SetPoint2
        if (xmlParser != 0x0)
        {
            retVal = ( ( CNTHVLV2* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->ExecCommand ( xmlParser );
        }
        else
        {
            retVal = ( ( CNTHVLV2* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetPIDSetpoint ( 0,parameters[0] );
        }
    }
    else if ( CheckControllerType ( netIndex, devIndex, DEV_AFOVLV_VAV ) )
    {
        // Modificato il 6/11/2009 perchè altrimenti cambiava solo il SetPoint2
        if (xmlParser != 0x0)
        {
            retVal = ( ( CNTHVLV2_VAV* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->ExecCommand ( xmlParser );
        }
        else
        {
            retVal = ( ( CNTHVLV2_VAV* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetPIDSetpoint ( 0,parameters[0] );
        }
    }
    else if ( CheckControllerType ( netIndex, devIndex, DEV_VLVCTRL ) )
    {
#ifdef USE_ADV_VLV        
        retVal = ( ( CNTHVLV_ADV* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetSetPoint ( parameters[0] );
#else
        retVal = ( ( CNTHVLV* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetSetPoint ( parameters[0] );
#endif
    }
    else if ( CheckControllerType ( netIndex, devIndex, DEV_FLOORCOORD_2 ) )
    {
        //In parameters di 0 c'e' il setpoint, in parameters[1] la zona
        retVal = ( (CFloorCoord2*) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetSetPoint ( (int)(parameters[1]), parameters[0] );
    }

    return retVal;
}

///////////////////////////////////////////////////
//             GetPIDParameters
///////////////////////////////////////////////////
bool COneWireNet::GetPIDSetup ( int address, float * parameters, bool * isSummer, CString &type )
{
    int netIndex = 0;
    int pidIndex = 0;
    bool retVal = false;

    netIndex = GetNetByMemoryAddress ( address );
    pidIndex = GetDeviceIndexByMemoryAddress ( netIndex, address );

    if ( ( parameters == 0x0 ) || ( netIndex == -1 ) || ( pidIndex == -1 ) || ( isSummer == 0x0 ) )
    {
        return false;
    }

    if ( CheckControllerType ( netIndex, pidIndex, DEV_PIDSIMPLE ) || CheckControllerType ( netIndex, pidIndex, DEV_PIDLMD ) )
    {
        retVal = ( ( CVPID* ) ( m_NetList[netIndex].CtrlList[pidIndex] ) )->GetInfo ( parameters, isSummer, type );
    }
    else if ( CheckControllerType ( netIndex, pidIndex, DEV_UTACTRL ) )
    {
        retVal = ( ( CUtaCtrl* ) ( m_NetList[netIndex].CtrlList[pidIndex] ) )->GetPIDInfo ( parameters, isSummer, type );
    }
    else
    {
        retVal = false;
    }

    return retVal;
}

///////////////////////////////////////////////////
//             UpdateDIDOsRegisters
///////////////////////////////////////////////////
int COneWireNet::UpdateDIDOsRegisters ( int netIndex )
{
    int devIndex = 0;
    uchar stateRegister[3];

    bool isDigitalIO = false;
    bool isMultiDIDO = false;

    memset (stateRegister, 0x0, 3);
    if ( !CheckNetIndex ( netIndex ) )
    {
        return -1;
    }

    for ( devIndex = 0; devIndex < m_NetList[netIndex].deviceList.size(); devIndex++ )
    {
        if ( CheckDeviceFamily ( netIndex, devIndex, DS2408_FN ) )
        {
            ( ( CDS2408* ) ( m_NetList[netIndex].deviceList[devIndex] ) )->ReadRegister ( 0x88, stateRegister );

            //If the device has had activity, clear the latches
            if ( stateRegister[2] )
            {
                ( ( CDS2408* ) ( m_NetList[netIndex].deviceList[devIndex] ) )->ClearActivityLatch();
            }
        }
    }
    
    //Aggiorno il registro anche dentro il controllore
    for ( devIndex = 0; devIndex < m_NetList[netIndex].CtrlList.size(); devIndex++ )
    {
        isDigitalIO = CheckControllerType ( netIndex, devIndex, DEV_DIDO ) ;
        isMultiDIDO = CheckControllerType ( netIndex, devIndex, DEV_REMOTEDIDO ) || CheckControllerType ( netIndex, devIndex, DEV_BUTTONCTRL ) || CheckControllerType ( netIndex, devIndex, DEV_TAGCTRL ) || CheckControllerType ( netIndex, devIndex, DEV_STEPDIGITALOUT ) ;

        if (isDigitalIO)
        {
            //Rileggo dal ds2408 il registro e lo memorizzo nel controllore
            ( ( CVDIDO* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->GetStatusRegister(0x0, false);
        }
        else if (isMultiDIDO)
        {
            //Leggo gli stati di ingressi e uscite
            ( (CVMultiDIDO* ) (m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->GetStatusRegister(true,0x0,0,false);
            ( (CVMultiDIDO* ) (m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->GetStatusRegister(false,0x0,0,false);
        }
    }

    return 1;

}
    
///////////////////////////////////////////////////
//             UpdateDIDOs
///////////////////////////////////////////////////
int COneWireNet::UpdateDIDOs ( int netIndex, int inputMatrix[][2], int remoteDidoMatrix[], int *nOfRemoteDIDO )
{
//     int devIndex = 0;
//     int inputIndex = 0, remoteDidoIndex = 0;
//     uchar *switchMatrix[MAX_NUM_DEV_PER_NET][2];
//     int lastSwitchIndex = 0;
//     int switchIndex = 0;
//     bool updateOk = false;
//     uchar stateRegister[3];
//     uchar serialNumber[8];
//     bool didoState;
//     vector<int> ds2408Vector;
//     vector<int> activityds2408;
//     vector<int>::iterator activityIndexesIt;
// 
//     bool isDigitalIO = false;
//     bool isMultiDIDO = false;
// 
// //     cout << "Pippo" << endl;
//     memset ( switchMatrix, 0x0, MAX_NUM_DEV_PER_NET*2*sizeof ( uchar ) );
// 
//     if ( !CheckIndexes ( netIndex ) )
//     {
//         return -1;
//     }
// 
//     for ( devIndex = 0; devIndex < m_NetList[netIndex].deviceList.size(); devIndex++ )
//     {
//         //TBR
// //         cout << "Leggo Registro Dispositivo numero:" << m_NetList.at(netIndex).deviceList.at(devIndex)->GetConfigFileDevIndex() << endl;
// 
//         if ( CheckDeviceFamily ( netIndex, devIndex, DS2408_FN ) )
//         {
//             switchMatrix[lastSwitchIndex][0] = ( uchar* ) malloc ( 8*sizeof ( uchar ) );
//             switchMatrix[lastSwitchIndex][1] = ( uchar* ) malloc ( 3*sizeof ( uchar ) );
// 
//             m_NetList[netIndex].deviceList[devIndex]->GetSN ( serialNumber );
//             if ( ( ( CDS2408* ) ( m_NetList[netIndex].deviceList[devIndex] ) )->ReadRegister ( 0x88, stateRegister ) )
//             {
//                 ds2408Vector.push_back ( devIndex );
// 
//                 if ( ( switchMatrix[lastSwitchIndex][0] != 0x0 ) && ( switchMatrix[lastSwitchIndex][1] != 0x0 ) )
//                 {
//                     memcpy ( switchMatrix[lastSwitchIndex][0], serialNumber, 8*sizeof ( uchar ) );
//                     memcpy ( switchMatrix[lastSwitchIndex][1], stateRegister, 3*sizeof ( uchar ) );
//                     lastSwitchIndex++;
//                 }
//                 else
//                 {
//                     //Probably we run out of memory: it is a serious problem, bail out
//                     return -1;
//                 }
//             }
// 
//             //If the device has had activity, clear the latches
//             if ( stateRegister[2] )
//             {
//                 ( ( CDS2408* ) ( m_NetList[netIndex].deviceList[devIndex] ) )->ClearActivityLatch();
//             }
//         }
//     }
// 
//     for ( switchIndex = 0; switchIndex < lastSwitchIndex; switchIndex++ )
//     {
//         //TBR
// //             cout << "Aggiorno Device Indirizo :" << m_NetList.at(netIndex).CtrlList.at(devIndex)->GetMemoryAddress() << endl;
//         for ( devIndex = 0; devIndex < m_NetList[netIndex].CtrlList.size(); devIndex++ )
//         {
//             char SN[8];
//             updateOk = false;
//             isDigitalIO = CheckControllerType ( netIndex, devIndex, DEV_DIDO ) ;
//             isMultiDIDO = CheckControllerType ( netIndex, devIndex, DEV_REMOTEDIDO ) || CheckControllerType ( netIndex, devIndex, DEV_BUTTONCTRL ) || CheckControllerType ( netIndex, devIndex, DEV_TAGCTRL ) || CheckControllerType ( netIndex, devIndex, DEV_STEPDIGITALOUT ) ;
// 
//             //Check if it is a digitalIO or a remoteDIDO
//             if ( isDigitalIO  || isMultiDIDO )
//             {
//                 if ( isDigitalIO )
//                 {
//                     ( ( ( CVDIDO* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->GetDeviceSN ( ( uchar* ) SN ) );
//                 }
//                 else
//                 {
//                     //TODO da controllare qual'e' l'input che prendo
//                     //For multiDIDO, since they have to correlate Input and Output, we get the serial number of the input
//                     ( ( CVMultiDIDO* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->GetDeviceSN ( false, ( uchar* ) SN );
//                 }
// 
//                 //Check if the serials are the same
//                 if ( !memcmp ( ( char* ) ( switchMatrix[switchIndex][0] ), SN, 8 ) )
//                 {
//                     //Set the controller internal register
//                     if ( isDigitalIO )
//                     {
//                         ( ( CVDIDO* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->SetStatusRegister ( switchMatrix[switchIndex][1] );
//                     }
//                     else
//                     {
//                         ( ( CVMultiDIDO* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->SetStatusRegister ( false, switchMatrix[switchIndex][1] );
//                     }
// 
//                     //Record the states of the different controllers
//                     if ( CheckControllerType ( netIndex, devIndex, DEV_REMOTEDIDO ) )
//                     {
//                         if ( ( ( CVMultiDIDO* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->GetActivity ( false ) )
//                         {
//                             remoteDidoMatrix[remoteDidoIndex] = m_NetList.at ( netIndex ).CtrlList.at ( devIndex )->GetMemoryAddress();
//                             remoteDidoIndex++;
//                         }
//                         updateOk = true;
//                     }
//                     else if ( CheckControllerType ( netIndex, devIndex, DEV_STEPDIGITALOUT ) )
//                     {
//                         if ( ( ( CStepDigitalOut* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->Update ( false ) )
//                         {
//                             updateOk = true;
//                         }
// 
//                         if ( ( ( CVMultiDIDO* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->GetActivity ( false ) )
//                         {
//                             if ( m_DoDebug == 3 )
//                             {
//                                 cout << "UpdateDIDO -------- Attivita' rilevata sul dispositivo di indirizzo "<< m_NetList.at ( netIndex ).CtrlList.at ( devIndex )->GetMemoryAddress() <<endl;
//                             }
//                         }
// 
//                         inputMatrix[inputIndex][0] = devIndex;
//                         inputMatrix[inputIndex][1] = ( ( CVMultiDIDO* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->GetInputState ( false );
//                         //Update counter
//                         inputIndex++;
//                     }
//                     else if ( CheckControllerType ( netIndex, devIndex, DEV_DIDO ) )
//                     {
//                         //Store the value
//                         inputMatrix[inputIndex][0] = devIndex;
// 
//                         //If it is an input we have to get the "level state", else we have to get the "latch state"
//                         if ( ( ( CDigitalIO* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->IsInput() )
//                         {
//                             inputMatrix[inputIndex][1] = ( ( CVDIDO* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->GetChannelLevel ( false );
//                         }
//                         else
//                         {
//                             inputMatrix[inputIndex][1] = ( ( CVDIDO* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->GetChannelLatch ( false );
//                         }
// 
//                         //Update the output
//                         if ( ( ( CDigitalIO* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->Update ( true ) )
//                         {
//                             updateOk = true;
//                         }
// 
//                         //Check for errors in the state
//                         if ( ( ! ( ( CDigitalIO* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->IsInput() ) && ( ( ( CDigitalIO* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->GetStateCheck() ) )
//                         {
//                             //Get state recorded inside the device to check if the current state is the one we want
//                             didoState = ( ( CDigitalIO* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->GetLastState();
// 
//                             if ( didoState != inputMatrix[inputIndex][1] )
//                             {
//                                 PushError ( AFOERROR_DIDO_STATE_DIFFERS_FROM_INTERNAL_STATE, m_NetList[netIndex].CtrlList[devIndex]->GetConfigFileNetIndex(), m_NetList[netIndex].CtrlList[devIndex]->GetConfigFileDevIndex() );
// 
//                                 if ( m_DoDebug )
//                                 {
//                                     cout << "Indirizzo DIDO : "<< ( ( CDigitalIO* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->GetMemoryAddress() << " Stato Interno: "<<didoState<<" Stato reale : "<< inputMatrix[inputIndex][1] << endl;
//                                 }
// 
// 
//                                 //Something strange is happening: last state differs from what we got, try to force it again
//                                 if ( ! ( ( CDigitalIO* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetState ( didoState ) )
//                                 {
//                                     int netConfigIndex, devConfigIndex;
// 
//                                     devConfigIndex = m_NetList[netIndex].CtrlList[devIndex]->GetConfigFileDevIndex();
//                                     netConfigIndex = m_NetList[netIndex].CtrlList[devIndex]->GetConfigFileNetIndex();
// 
//                                     PushError ( AFOERROR_UNABLE_TO_SET_DIDO, netConfigIndex, devConfigIndex );
//                                     if ( m_DoDebug )
//                                     {
//                                         cout << "Impossibile impostare il DIDO di indirizzo: "<< m_NetList[netIndex].CtrlList[devIndex]->GetMemoryAddress() << endl;
//                                         cout.flush();
//                                     }
//                                 }
//                                 else
//                                 {
//                                     if ( m_DoDebug )
//                                     {
//                                         cout << "DIDO di indirizzo: " << m_NetList[netIndex].CtrlList[devIndex]->GetMemoryAddress() << "reimpostato a " << didoState << endl;
//                                         cout.flush();
//                                     }
//                                 }
//                             }
//                         } //If !IsInput (For timers)
// 
//                         //Update counter
//                         inputIndex++;
// 
//                     }//IF is digital io
//                     else if ( CheckControllerType ( netIndex, devIndex, DEV_BUTTONCTRL ) )
//                     {
//                         int outDevice = 0;
// 
//                         if ( ( ( CVMultiDIDO* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->GetActivity ( false ) )
//                         {
//                             if ( m_DoDebug == 3 )
//                             {
//                                 cout << "UpdateDIDO -------- Attivita' rilevata sul dispositivo di indirizzo "<< m_NetList.at ( netIndex ).CtrlList.at ( devIndex )->GetMemoryAddress() <<endl;
//                             }
// 
//                             int configDevIndex = -1;
//                             //TODO da cambiare perchè sono costretto a chiamare direttamente la classe buttoncontroller e non multidido
//                             if ( ( ( CButtonController* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->m_IsJolly )
//                             {
//                                 //Jolly device, apply changes to all devices belonging to the same NET
//                                 SetAllDigitalOutputs ( netIndex, ( ( CButtonController* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->m_JollyValue );
//                             }
//                             else
//                             {
//                                 if ( ( ( CVMultiDIDO* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->ChangeOutput() )
//                                 {
//                                     //Save the INI file
//                                     configDevIndex = m_NetList.at ( netIndex ).CtrlList.at ( devIndex )->GetConfigFileDevIndex();
//                                     if ( ( ( CButtonController* ) ( m_NetList.at ( netIndex ).CtrlList[devIndex] ) )->GetOutputState ( false ) )
//                                     {
//                                         UpdateIniFile ( netIndex, configDevIndex, "STARTV", "1" );
//                                     }
//                                     else
//                                     {
//                                         UpdateIniFile ( netIndex, configDevIndex, "STARTV", "0" );
//                                     }
//                                 }
//                                 else
//                                 {
//                                     //TODO mettere messaggio di errore
//                                 }
//                             }
// 
//                         }
//                         else
//                         {
//                             updateOk = ( ( CVMultiDIDO* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->Update ( false );
//                         }
// 
//                         //Search the output device
//                         if ( ( ( CVMultiDIDO* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->IOSameDevices() )
//                         {
//                             ( ( CVMultiDIDO* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->SetStatusRegister ( true, switchMatrix[switchIndex][1] );
//                         }
//                         else if ( ! ( ( CButtonController* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->m_IsJolly )
//                         {
//                             for ( outDevice = 0; outDevice < lastSwitchIndex; outDevice++ )
//                             {
//                                 if ( !memcmp ( ( char* ) ( ( CVMultiDIDO* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->GetDeviceSN ( true ), ( char* ) switchMatrix[outDevice][0], 8 ) )
//                                 {
//                                     break;
//                                 }
//                             }
// 
//                             if ( outDevice < lastSwitchIndex )
//                             {
//                                 ( ( CVMultiDIDO* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->SetStatusRegister ( true, switchMatrix[outDevice][1] );
//                             }
//                         }
// 
//                         //store the output value
//                         inputMatrix[inputIndex][0] = devIndex;
//                         inputMatrix[inputIndex][1] = ( ( CVMultiDIDO* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->GetOutputState ( false );
//                         //Update counter
//                         inputIndex++;
//                     }
//                     else if ( CheckControllerType ( netIndex, devIndex, DEV_TAGCTRL ) )
//                     {
// 
//                         ( ( CVMultiDIDO* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->Update ( false );
//                         updateOk = true;
//                         //store the Input value, or, in other words, the TAG state
//                         inputMatrix[inputIndex][0] = devIndex;
//                         inputMatrix[inputIndex][1] = ( ( CVMultiDIDO* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->GetInputState ( false );
// 
//                         if ( ( ( CVMultiDIDO* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->IsRemoted() )
//                         {
//                             remoteDidoMatrix[remoteDidoIndex] = m_NetList.at ( netIndex ).CtrlList.at ( devIndex )->GetMemoryAddress();
//                             remoteDidoIndex++;
// 
//                             //05/09/2008 -- Commentata perche' inutile, in attesa di rimozione dopo test
// //                             if ( inputMatrix[inputIndex][1] != ( ( CVMultiDIDO* ) ( m_NetList.at ( netIndex ).CtrlList.at ( devIndex ) ) )->GetOutputState ( false ) )
// //                             {
// //                                 //Salvo nella lista dei DIDO remoti SE input e output sono diversi, ovvero sempre in questo caso perchè lo stato dell'uscita NON E' MAI CAMBIATO DALL'INIZIALIZZAZIONE
// //                                 remoteDidoMatrix[remoteDidoIndex] = m_NetList.at ( netIndex ).CtrlList.at ( devIndex )->GetMemoryAddress();
// //                                 remoteDidoIndex++;
// //                             }
//                         }
// 
//                         //Update counter
//                         inputIndex++;
//                     }
// 
//                     if ( !updateOk )
//                     {
//                         int netConfigIndex, devConfigIndex;
// 
//                         devConfigIndex = m_NetList[netIndex].CtrlList[devIndex]->GetConfigFileDevIndex();
//                         netConfigIndex = m_NetList[netIndex].CtrlList[devIndex]->GetConfigFileNetIndex();
// 
//                         PushError ( AFOERROR_UNABLE_TO_UPDATE_DIDO_OUT, netConfigIndex, devConfigIndex );
//                     }
// 
//                 }//IF i numeri di serie combaciano
//             }//********IF isdigitalIO || isButtonDido
// 
//         }//For devIndex
// 
//     }//For switchIndex
// 
//     //Clean up everything
//     for ( switchIndex = 0; switchIndex < lastSwitchIndex; switchIndex++ )
//     {
//         free ( switchMatrix[switchIndex][0] );
//         free ( switchMatrix[switchIndex][1] );
//     }
// 
// 
//     *nOfRemoteDIDO = remoteDidoIndex;

//     return inputIndex;
    return -1;
}

///////////////////////////////////////////////////
//             ClearAllActivityLatches
///////////////////////////////////////////////////
bool COneWireNet::ClearAllActivityLatches ( int netIndex )
{
    int devIndex = 0;

    //Reset all the activity latches
    for ( devIndex = 0; devIndex < m_NetList[netIndex].deviceList.size(); devIndex++ )
    {
        if ( CheckDeviceFamily ( netIndex, devIndex, DS2408_FN ) )
        {
            ( ( CDS2408* ) ( m_NetList[netIndex].deviceList[devIndex] ) )->ClearActivityLatch();
        }
        else if ( CheckDeviceFamily ( netIndex, devIndex, DS2405_FN ) )
        {
            //TODO da implementare
        }

    }

    return true;
}

///////////////////////////////////////////////////
//             ChangeNetTimerState
///////////////////////////////////////////////////
bool COneWireNet::ChangeNetTimerState ( int netIndex, bool newState )
{
    bool retVal = false;

    if ( !CheckNetIndex ( netIndex ) )
    {
        return retVal;
    }

    m_NetList[netIndex].isTimerOn = newState;

    return true;
}

///////////////////////////////////////////////////
//             ChangeNetTimerID
///////////////////////////////////////////////////
bool COneWireNet::ChangeNetTimerID ( int netIndex, int newID )
{
    bool retVal = false;

    //Usual check
    if ( !CheckNetIndex ( netIndex ) )
    {
        return false;
    }

    if ( m_NetList[netIndex].netTimer != 0x0 )
    {
        m_NetList[netIndex].timerID = newID;
        retVal = true;
    }

    return retVal;
}

///////////////////////////////////////////////////
//             SetSummer
///////////////////////////////////////////////////
bool COneWireNet::SetSummer ( bool isSummer, int address )
{
    int netIndex = -1;
    int devIndex = -1;
    bool retVal = false;
    CString newVal = "0";
    bool updateIniFile = true;


    //Get net of the device
    netIndex = GetNetByMemoryAddress ( address );

    if ( netIndex != -1 )
    {
        devIndex = GetDeviceIndexByMemoryAddress ( netIndex, address );

        if ( devIndex > -1 )
        {
            if ( CheckControllerType ( netIndex, devIndex, DEV_PIDSIMPLE ) || CheckControllerType ( netIndex, devIndex, DEV_PIDLMD ) )
            {
                retVal = ( ( CVPID* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetSummer ( isSummer );
            }
            else if ( CheckControllerType ( netIndex, devIndex, DEV_VLVCTRL ) )
            {
#ifndef USE_ADV_VLV
                retVal = ( ( CNTHVLV* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetSummer ( isSummer );
#else
                retVal = ( ( CNTHVLV_ADV* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetSummer ( isSummer );
#endif
            }
            else if ( CheckControllerType ( netIndex, devIndex, DEV_AFOVLV ) )
            {
                retVal = ( ( CNTHVLV2* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetSummer ( isSummer );
            }
            else if ( CheckControllerType ( netIndex, devIndex, DEV_AFOVLV_VAV ) )
            {
                retVal = ( ( CNTHVLV2_VAV* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetSummer ( isSummer );
            }
            else if ( CheckControllerType ( netIndex, devIndex, DEV_UTACTRL ) )
            {
                retVal = ( ( CUtaCtrl* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetSummer ( isSummer );
            }
            else if ( CheckControllerType ( netIndex, devIndex, DEV_MGC ) )
            {
                retVal = ( ( CNTHMGC* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetSummer ( isSummer );
            }
            else if ( CheckControllerType ( netIndex, devIndex, DEV_FULLUTACTRL ) )
            {
                retVal = ( ( CFullUTACtrl* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetSummer ( isSummer );
                updateIniFile = false;
            }
            else if ( CheckControllerType ( netIndex, devIndex, DEV_FULLUTACTRL_2 ) )
            {
                retVal = ( ( CFullUTACtrl2* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetSummer ( isSummer );
                updateIniFile = false;
            }
            else if (  CheckControllerType ( netIndex, devIndex, DEV_FLOORCOORD_2 ) )
            {
                retVal = ( ( CFloorCoord2* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetSummer ( isSummer );
                updateIniFile = false;
            }
            else if (  CheckControllerType ( netIndex, devIndex, DEV_C3POINT ) )
            {
                ( ( C3PointCtrl* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetSummer ( isSummer );
                updateIniFile = true;
                retVal = true;

            }
            else if (  CheckControllerType ( netIndex, devIndex, DEV_CNT ) )
            {
                ( ( CNTH_CNT* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetSummer ( isSummer );
                updateIniFile = true;
                retVal = true;

            }
            
            if (( retVal ) && (updateIniFile))
            {

                devIndex = m_NetList[netIndex].CtrlList[devIndex]->GetConfigFileDevIndex();

                if ( isSummer )
                {
                    newVal="1";
                }

                UpdateIniFile ( netIndex, devIndex, "SUMMER", newVal );
            }

        }
    }


    return retVal;

}

///////////////////////////////////////////////////
//            SetAllSummer
///////////////////////////////////////////////////
bool COneWireNet::SetAllSummer ( int netIndex, bool isSummer )
{
    int devIndex = 0;
    int ctrlIndex = 0;
    CString newVal = "0";
    bool retVal = false;

    if ( !CheckIndexes ( netIndex, devIndex, false ) )
    {
        return false;
    }

    for ( ctrlIndex = 0; ctrlIndex < m_NetList[netIndex].CtrlList.size(); ctrlIndex++ )
    {
        SetSummer(isSummer, m_NetList[netIndex].CtrlList[ctrlIndex]->GetMemoryAddress());
//        if ( CheckControllerType ( netIndex, ctrlIndex, DEV_PIDSIMPLE ) || CheckControllerType ( netIndex, ctrlIndex, DEV_PIDLMD ) )
//        {
//            retVal = ( ( CVPID* ) ( m_NetList[netIndex].CtrlList[ctrlIndex] ) )->SetSummer ( isSummer );
//        }
//        else if ( CheckControllerType ( netIndex, ctrlIndex, DEV_VLVCTRL ) )
//        {
//#ifndef USE_ADV_VLV
//            retVal = ( ( CNTHVLV* ) ( m_NetList[netIndex].CtrlList[ctrlIndex] ) )->SetSummer ( isSummer );
//#else
//            retVal = ( ( CNTHVLV_ADV* ) ( m_NetList[netIndex].CtrlList[ctrlIndex] ) )->SetSummer ( isSummer );
//#endif
//        }
//        else if ( CheckControllerType ( netIndex, ctrlIndex, DEV_AFOVLV ) )
//        {
//            retVal = ( ( CNTHVLV2* ) ( m_NetList[netIndex].CtrlList[ctrlIndex] ) )->SetSummer ( isSummer );
//        }
//        else if ( CheckControllerType ( netIndex, ctrlIndex, DEV_AFOVLV_VAV ) )
//        {
//            retVal = ( ( CNTHVLV2_VAV* ) ( m_NetList[netIndex].CtrlList[ctrlIndex] ) )->SetSummer ( isSummer );
//        }
//        else if ( CheckControllerType ( netIndex, devIndex, DEV_UTACTRL ) )
//        {
//            retVal = ( ( CUtaCtrl* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetSummer ( isSummer );
//        }
//        else if ( CheckControllerType ( netIndex, devIndex, DEV_MGC ) )
//        {
//            retVal = ( ( CNTHMGC* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetSummer ( isSummer );
//        }
//
//        if ( retVal )
//        {
//
//            devIndex = m_NetList[netIndex].CtrlList[ctrlIndex]->GetConfigFileDevIndex();
//
//            if ( isSummer )
//            {
//                newVal="1";
//            }
//
//            UpdateIniFile ( netIndex, devIndex, "SUMMER", newVal );
//        }
    }

    return true;
}


///////////////////////////////////////////////////
//             GetHumidty(1)
///////////////////////////////////////////////////
bool COneWireNet::GetHumidity ( int netIndex, int devIndex, float * absH, float * relH )
{
    bool retVal = false;

    //TBRRimini
//     cout << "GetHumidity - Leggo umidita'  da sensore: \nnetIndex: "<< netIndex << " devIndex: "<< devIndex<<endl;cout.flush();
    if ( ( !CheckIndexes ( netIndex, devIndex, false ) ) || ( !CheckControllerType ( netIndex, devIndex, DEV_HUMIDITY ) ) )
    {
        *absH = -1.0;
        *relH = -1.0;
        return retVal;
    }

    if ( ( ( CHumController* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->GetHumidity ( absH, relH ) )
    {
        retVal = true;
    }
    else
    {
        if ( m_DoDebug )
        {
            cout << "Attenzione!! Impossibile leggere umidita'  dal sensore di indirizzo : " << m_NetList[netIndex].CtrlList[devIndex]->GetMemoryAddress() << endl; cout.flush();
//             OWERROR_DUMP(stdout);
        }
    }


    return retVal;
}

///////////////////////////////////////////////////
//             GetHumidty(2)
///////////////////////////////////////////////////
bool COneWireNet::GetHumidity ( int address, float *absH, float *relH )
{
    bool retVal = false;
    int netIndex, devIndex;

    netIndex = GetNetByMemoryAddress ( address );
    devIndex = GetDeviceIndexByMemoryAddress ( netIndex, address );

    retVal = GetHumidity ( netIndex, devIndex, absH, relH );

    return retVal;

}

///////////////////////////////////////////////////
//             SetHumAutoControl
///////////////////////////////////////////////////
bool COneWireNet::SetHumAutoControl ( int address, bool state )
{
    bool retVal = false;
    int netIndex, devIndex, configDevIndex;
    CString stateStr;

    netIndex = GetNetByMemoryAddress ( address );
    devIndex = GetDeviceIndexByMemoryAddress ( netIndex, address );

    if ( ( netIndex != -1 ) && ( devIndex != -1 ) )
    {
        retVal = ( ( CHumController* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetAutoControl ( state );

        if ( retVal )
        {
            stateStr+=state;
            configDevIndex = m_NetList[netIndex].CtrlList[devIndex]->GetConfigFileDevIndex();
            UpdateIniFile ( netIndex, configDevIndex, "AUTO", stateStr );
        }
    }

    return retVal;
}

///////////////////////////////////////////////////
//             SetAllHumAutoControl
///////////////////////////////////////////////////
bool COneWireNet::SetAllHumAutoControl ( int netIndex, bool state )
{
    int humIndex = 0;

    if ( !CheckNetIndex ( netIndex ) )
    {
        return false;
    }

    for ( humIndex = 0; humIndex < m_NetList[netIndex].CtrlList.size(); humIndex++ )
    {
        if ( CheckControllerType ( netIndex, humIndex, DEV_HUMIDITY ) )
        {
            ( ( CHumController* ) ( m_NetList[netIndex].CtrlList[humIndex] ) )->SetAutoControl ( state );
        }
    }

    return true;

}

///////////////////////////////////////////////////
//             SetHumSetPoint
///////////////////////////////////////////////////
bool COneWireNet::SetHumSetPoint ( int address, float setPoint )
{
    bool retVal = false;
    int netIndex, devIndex, configDevIndex;
    CString setpointStr;

    netIndex = GetNetByMemoryAddress ( address );
    devIndex = GetDeviceIndexByMemoryAddress ( netIndex, address );

    if ( ( netIndex != -1 ) && ( devIndex != -1 ) )
    {
        retVal = ( ( CHumController* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetSetPoint ( setPoint );

        if ( retVal )
        {
            setpointStr+=setPoint;
            configDevIndex = m_NetList[netIndex].CtrlList[devIndex]->GetConfigFileDevIndex();
            UpdateIniFile ( netIndex, configDevIndex, "AUTO", setpointStr );
        }
    }

    return retVal;

}

///////////////////////////////////////////////////
//             SetAllHumSetPoint
///////////////////////////////////////////////////
bool COneWireNet::SetAllHumSetPoint ( int netIndex, float setPoint )
{
    unsigned int humIndex = 0;

    if ( !CheckNetIndex ( netIndex ) )
    {
        return false;
    }

    for ( humIndex = 0; humIndex < m_NetList[netIndex].CtrlList.size(); humIndex++ )
    {
        if ( CheckControllerType ( netIndex, humIndex, DEV_HUMIDITY ) )
        {
            ( ( CHumController* ) ( m_NetList[netIndex].CtrlList[humIndex] ) )->SetSetPoint ( setPoint );
        }
    }

    return true;
}

///////////////////////////////////////////////////
//             GetHumiditySettings
///////////////////////////////////////////////////
bool COneWireNet::GetHumiditySettings ( int address, float * setPoint, bool * autoControlState, float* hysteresis )
{
    bool retVal = false;
    int netIndex, devIndex;

    netIndex = GetNetByMemoryAddress ( address );
    devIndex = GetDeviceIndexByMemoryAddress ( netIndex, address );

    if ( ( netIndex != -1 ) && ( devIndex != -1 ) )
    {
        retVal = true;
        *setPoint = ( ( CHumController* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->GetSetPoint();
        *autoControlState = ( ( CHumController* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->GetAutoControl();
        *hysteresis = ( ( CHumController* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->GetHysteresis();
    }

    return retVal;

}

///////////////////////////////////////////////////
//             SetHumidityHysteresis
///////////////////////////////////////////////////
bool COneWireNet::SetHumidityHysteresis ( int address, float newHysteresis )
{
    bool retVal = false;
    int netIndex, devIndex, configDevIndex;
    CString hystStr;

    netIndex = GetNetByMemoryAddress ( address );
    devIndex = GetDeviceIndexByMemoryAddress ( netIndex, address );

    if ( ( netIndex != -1 ) && ( devIndex != -1 ) )
    {
        retVal = ( ( CHumController* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->SetHysteresis ( newHysteresis );

        if ( retVal )
        {
            hystStr+=newHysteresis;
            configDevIndex = m_NetList[netIndex].CtrlList[devIndex]->GetConfigFileDevIndex();
            UpdateIniFile ( netIndex, configDevIndex, "AUTO", hystStr );
        }
    }

    return retVal;
}

///////////////////////////////////////////////////
//             SetAllHumidityHysteresis
///////////////////////////////////////////////////
bool COneWireNet::SetAllHumidityHysteresis ( int netIndex, float newHysteresis )
{
    int humIndex = 0;

    if ( !CheckNetIndex ( netIndex ) )
    {
        return false;
    }

    for ( humIndex = 0; humIndex < m_NetList[netIndex].CtrlList.size(); humIndex++ )
    {
        if ( CheckControllerType ( netIndex, humIndex, DEV_HUMIDITY ) )
        {
            ( ( CHumController* ) ( m_NetList[netIndex].CtrlList[humIndex] ) )->SetHysteresis ( newHysteresis );
        }
    }

    return true;
}

///////////////////////////////////////////////////
//             GetAnalogInput
///////////////////////////////////////////////////
bool COneWireNet::GetAnalogIO ( int address, float *inputVal, bool *isCurrent )
{
    bool retVal = false;
    int netIndex, devIndex;

    netIndex = GetNetByMemoryAddress ( address );
    devIndex = GetDeviceIndexByMemoryAddress ( netIndex, address );

    if ( ( netIndex != -1 ) && ( devIndex != -1 ) )
    {

        *inputVal = ( ( CAnalogIO* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->GetValue(true);
        //Check if it is an input
        if ( ( ( CAnalogIO* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->IsInput() )
        {
            *isCurrent = ( ( CAnalogIO* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->GetReadCurrentState();
        }
        else
        {
            *isCurrent = false;
        }


        if ( *inputVal > ANALOG_ERRVAL )
        {
            retVal = true;
        }
    }

    return retVal;
}

///////////////////////////////////////////////////
//             SetAnalogInput
///////////////////////////////////////////////////
bool COneWireNet::SetAnalogInput ( int address, bool readCurrent )
{
    bool retVal = false;
//     int netIndex, devIndex;
//
//     netIndex = GetNetByMemoryAddress( address );
//     devIndex = GetDeviceIndexByMemoryAddress( netIndex, address);
//
//     if ((netIndex != -1) && (devIndex != -1))
//     {
//         retVal = ((CAnalogInput*)(m_NetList[netIndex].CtrlList[devIndex]))->SetInputCurrent(readCurrent);
//     }

    return retVal;
}

///////////////////////////////////////////////////
//             GetDeviceIndexByConfigNumber
///////////////////////////////////////////////////
int COneWireNet::GetDeviceIndexByConfigNumber ( int netIndex, int configNumber )
{
    char deviceKey[32];
    CString deviceConfigStr, snString;
    CString netName;

    if ((configNumber < 0) || (netIndex > m_NetList.size()) || (netIndex < 0))
    {
        return -1;
    }
    
    //TODO da controllare perche' sembra che manchi il controllo del confignumber < 10 o >= 10
    sprintf ( deviceKey, "Device%02d", configNumber );

    netName = "NET";
    netName += ( netIndex+1 );

    deviceConfigStr = m_IniFile->GetString ( deviceKey, netName );

    m_IniLib.GetConfigParamString ( deviceConfigStr.c_str(), "SN", &snString, "" );

    if (snString.length() < 16)
    {
        return -1;
    }

    return GetIndexBySerial ( netIndex, snString );
}

///////////////////////////////////////////////////
//             GetAllHumidities
///////////////////////////////////////////////////
int COneWireNet::GetAllHumidities ( int netIndex, float dest[][3], int maxNum )
{

    int retVal = -1, index = 0, stopVal = 0;

    //Usual Check
    if ( ( !CheckNetIndex ( netIndex ) ) || ( dest == NULL ) || ( maxNum < 0 ) )
    {
        retVal = -1;
    }
    else
    {
        retVal = 0;
    }

    if ( maxNum < m_NetList[netIndex].CtrlList.size() )
    {
        stopVal = maxNum;
    }
    else
    {
        stopVal = m_NetList[netIndex].CtrlList.size();
    }

    if ( retVal != -1 )
    {
        //Start cycle
        for ( index = 0;  index < stopVal; index++ )
        {
            //Check if the given device is a T sensor
            if ( CheckControllerType ( netIndex, index, DEV_HUMIDITY ) )
            {
                //Update the device index
                dest[retVal][0] = index;
                if ( GetHumidity ( netIndex, index, &dest[retVal][1], &dest[retVal][2] ) )
                {
                    retVal++;
                }
            }
        }
    }

    return retVal;
}

///////////////////////////////////////////////////
//             UpdateIniFile
///////////////////////////////////////////////////
bool COneWireNet::UpdateIniFile ( int netIndex, int devIndex, CString subKey, CString newVal )
{
    string configStr;
    CString key, section, deviceConfigurationString;
    char tempBuffer[32];

    memset ( tempBuffer, 0x0, 32 );

    //Update the default value in the INI file
    sprintf ( tempBuffer, "NET%d",  netIndex+1 );
    section = tempBuffer;

    memset ( tempBuffer, 0x0, 32 );
    sprintf ( tempBuffer, "Device%02d", devIndex );

    key = tempBuffer;

    configStr = m_IniFile->GetString ( key, section );

    m_IniLib.SetConfigParamString ( &configStr, subKey.c_str(), newVal.c_str() );

    m_IniFile->SetValue ( key, configStr, "", section );

    m_IniFile->Save();

    return true;
}

///////////////////////////////////////////////////
//             EnableNetTimer
///////////////////////////////////////////////////
bool COneWireNet::EnableNetTimer ( int netIndex, bool timerState )
{
    if ( CheckNetIndex ( netIndex ) )
    {
        m_NetList[netIndex].isTimerOn = timerState;
        return true;
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////
//             EnableAllTimers
///////////////////////////////////////////////////
bool COneWireNet::EnableAllTimers ( int netIndex, bool timerState )
{
    int controllerIndex = 0;

    if ( CheckNetIndex ( netIndex ) )
    {
        for ( controllerIndex = 0; controllerIndex < m_NetList[netIndex].CtrlList.size(); controllerIndex++ )
        {
            m_NetList[netIndex].CtrlList[controllerIndex]->UseTimer ( timerState );
        }

        return true;
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////
//             UseTimer
///////////////////////////////////////////////////
bool COneWireNet::UseTimer ( int netIndex, int ctrlIndex, bool timerState )
{
    if ( CheckIndexes ( netIndex, ctrlIndex, false ) )
    {
        m_NetList[netIndex].CtrlList[ctrlIndex]->UseTimer ( timerState );
        return true;
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////
//             SetNetTimerID
///////////////////////////////////////////////////
bool COneWireNet::SetNetTimerID ( int netIndex, int timerID )
{
    if ( CheckNetIndex ( netIndex ) && ( timerID <= m_Timer->GetNofTimers() ) )
    {
        m_NetList[netIndex].timerID = timerID;
        return true;
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////
//             SetAllTimerID
///////////////////////////////////////////////////
bool COneWireNet::SetAllTimerID ( int netIndex, int timerID )
{
    int controllerIndex = 0;

    if ( CheckNetIndex ( netIndex ) && ( timerID <= m_Timer->GetNofTimers() ) )
    {
        for ( controllerIndex = 0; controllerIndex < m_NetList[netIndex].CtrlList.size(); controllerIndex++ )
        {
            m_NetList[netIndex].CtrlList[controllerIndex]->ChangeTimerID ( timerID );
        }

        return true;
    }
    else
    {
        return false;
    }

}

///////////////////////////////////////////////////
//             SetTimerID
///////////////////////////////////////////////////
bool COneWireNet::SetTimerID ( int netIndex, int ctrlIndex, int timerID )
{
    if ( CheckNetIndex ( netIndex ) && ( timerID <= m_Timer->GetNofTimers() ) )
    {
        m_NetList[netIndex].CtrlList[ctrlIndex]->ChangeTimerID ( timerID );
        
        return true;
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////
//             GetControllerMemoryAddress
///////////////////////////////////////////////////
int COneWireNet::GetControllerMemoryAddress ( int netIndex, int ctrlIndex )
{
    if ( CheckIndexes ( netIndex, ctrlIndex, false ) )
    {
        return m_NetList[netIndex].CtrlList[ctrlIndex]->GetMemoryAddress();
    }
    else
    {
        return -1;
    }
}

///////////////////////////////////////////////////
//             GetAlarmT
///////////////////////////////////////////////////
bool COneWireNet::GetAlarmT ( int netIndex, int devIndex, int * maxAlarmLevel, int * minAlarmLevel )
{
    if ( CheckIndexes ( netIndex, devIndex, false ) )
    {
        return ( ( CTempCtrl* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->GetAlarmLevel ( maxAlarmLevel, minAlarmLevel );
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////
//             GetAllAlarmT
///////////////////////////////////////////////////
int COneWireNet::GetAllAlarmT ( int netIndex, int alarmArray[][3], int maxNumOfAlarms )
{
    int retVal = 0, index = 0, stopVal = 0;

    //Usual Check
    if ( ( !CheckNetIndex ( netIndex ) ) || ( alarmArray == 0x0 ) || ( maxNumOfAlarms < 0 ) )
    {
        retVal = -1;
    }

    if ( maxNumOfAlarms < m_NetList[netIndex].CtrlList.size() )
    {
        stopVal = maxNumOfAlarms;
    }
    else
    {
        stopVal = m_NetList[netIndex].CtrlList.size();
    }

    if ( retVal != -1 )
    {
        //Start cycle
        for ( index = 0;  index < stopVal; index++ )
        {
            //Check if the given device is a T sensor
            if ( CheckControllerType ( netIndex, index, DEV_TEMPCTRL ) )
            {
                //Update the device index
                alarmArray[retVal][0] = index;
                GetAlarmT ( netIndex, index, &alarmArray[retVal][1], &alarmArray[retVal][2] );
                retVal++;
            }
        }
    }

    return retVal;
}

///////////////////////////////////////////////////
//             IsNetAcquired
///////////////////////////////////////////////////
bool COneWireNet::IsNetAcquired ( int netIndex )
{
    bool retVal = false;

    if ( CheckNetIndex ( netIndex ) )
    {
        if ( m_NetList[netIndex].isAcquired )
        {
            retVal = true;
        }
    }

    return retVal;
}

///////////////////////////////////////////////////
//             UpdateBlackList
///////////////////////////////////////////////////
bool COneWireNet::UpdateBlackList ( int netIndex )
{
    vector<CVController*>::iterator ctrlIt;
    vector<CVController*>::iterator blackListIt;
    unsigned int blackListIndex = 0;
    time_t actTime;

    try
    {
        //Get Actual Time
        time ( &actTime );

        //Compare it with last blacklist recovery
        if ( ( actTime > m_LastBLRecoveryTime + m_BLRecoveryInterval ) && ( m_NetList.at ( netIndex ).blackList.size() ) )
        {
            m_LastBLRecoveryTime = actTime;

            blackListIt = m_NetList.at ( netIndex ).blackList.begin();

            //Start Checking if some devices of the black list are back online again
            for ( blackListIndex =  0; blackListIndex < m_NetList.at ( netIndex ).blackList.size(); blackListIndex++ )
            {
                if ( ( *blackListIt )->VerifyIOPresence() )
                {
                    //Device back online, reinsert it in the correct position
                    //Reset error condition on controller
                    ( *blackListIt )->ClearError();

                    m_NetList.at ( netIndex ).CtrlList.push_back ( *blackListIt );

                    if ( m_DoDebug )
                    {
                        cout << "Attenzione!! NET: " << ( *blackListIt )->GetConfigFileNetIndex() << " Controller: " << ( *blackListIt )->GetConfigFileDevIndex() << " e' stato rimosso dalla blacklist" << endl;
                    }

                    //Remove it from the balckList
                    m_NetList.at ( netIndex ).blackList.erase ( blackListIt );

                }
                else
                {
                    //The device is not working, send an error
                    PushError ( AFOERROR_CONTROLLER_BLACKLISTED, ( *blackListIt )->GetConfigFileNetIndex(), ( *blackListIt )->GetConfigFileDevIndex() );
                    blackListIt++;
                }
            }
        }

        ctrlIt = m_NetList.at ( netIndex ).CtrlList.begin();

        while ( ctrlIt != m_NetList.at ( netIndex ).CtrlList.end() )
        {

            if ( ( *ctrlIt )->GetNofErrors() >= m_MAXNofErrors )
            {
                PushError ( AFOERROR_CONTROLLER_BLACKLISTED, netIndex + 1, ( *ctrlIt )->GetConfigFileDevIndex() );

                //Add the controller to the blacklist
                m_NetList.at ( netIndex ).blackList.push_back ( ( *ctrlIt ) );

                m_NetList.at ( netIndex ).CtrlList.erase ( ctrlIt );
            }
            else
            {
                ctrlIt++;
            }
        }

        return true;
    }
    catch ( exception &e )
    {
        if ( m_DoDebug )
        {
            cout << "Si e'verificato un errore nell'analisi della black list :" << netIndex << " Errore: "<< e.what() <<endl;
        }

        return false;
    }
}

///////////////////////////////////////////////////
//             UpdateAllBlackLists
///////////////////////////////////////////////////
bool COneWireNet::UpdateAllBlackLists( )
{
    unsigned int netIndex = 0;

    for ( netIndex = 0; netIndex < m_NetList.size(); netIndex++ )
    {
        UpdateBlackList ( netIndex );
    }

    return true;
}

///////////////////////////////////////////////////
//             NetHasTempDevices
///////////////////////////////////////////////////
bool COneWireNet::NetHasTempDevices ( int netIndex )
{
    bool retVal = false;

    try
    {
        retVal = m_NetList.at ( netIndex ).hasTempDevices;
    }
    catch ( exception &e )
    {
        retVal = false;
    }

    return retVal;
}

///////////////////////////////////////////////////
//             NetHasDIDOs
///////////////////////////////////////////////////
bool COneWireNet::NetHasDIDOs ( int netIndex )
{
    bool retVal = false;

    try
    {
        retVal = m_NetList.at ( netIndex ).hasDIDOs;
    }
    catch ( exception &e )
    {
        retVal = false;
    }

    return retVal;
}

///////////////////////////////////////////////////
//             NetHasPIDs
///////////////////////////////////////////////////
bool COneWireNet::NetHasPIDs ( int netIndex )
{
    bool retVal = false;

    try
    {
        retVal = m_NetList.at ( netIndex ).hasPIDs;
    }
    catch ( exception &e )
    {
        retVal = false;
    }

    return retVal;
}

///////////////////////////////////////////////////
//             NetHasHums
///////////////////////////////////////////////////
bool COneWireNet::NetHasHums ( int netIndex )
{
    bool retVal = false;

    try
    {
        retVal = m_NetList.at ( netIndex ).hasHums;
    }
    catch ( exception &e )
    {
        retVal = false;
    }

    return retVal;
}

///////////////////////////////////////////////////
//             NetHasAIAOs
///////////////////////////////////////////////////
bool COneWireNet::NetHasAIAOs ( int netIndex )
{
    bool retVal = false;

    try
    {
        retVal = m_NetList.at ( netIndex ).hasAIAOs;
    }
    catch ( exception &e )
    {
        retVal = false;
    }

    return retVal;
}

bool COneWireNet::NetHasButtonDIDOs ( int netIndex )
{
    bool retVal = false;

    try
    {
        retVal = m_NetList.at ( netIndex ).hasButtonDIDOs;
    }
    catch ( exception &e )
    {
        retVal = false;
    }

    return retVal;
}

///////////////////////////////////////////////////
//             CheckAddressValidity
///////////////////////////////////////////////////
bool COneWireNet::CheckAddressValidity ( int addr2Check )
{
    bool retVal = true;
    int address = -1;
    vector<T_Net>::iterator netIt;
    vector<CVController*>::iterator ctrlIt;

    if ( addr2Check < 0 )
    {
        return true;
    }

    try
    {
        for ( netIt = m_NetList.begin(); netIt != m_NetList.end(); netIt++ )
        {
            for ( ctrlIt = netIt->CtrlList.begin(); ctrlIt != netIt->CtrlList.end(); ctrlIt++ )
            {
                address = ( *ctrlIt )->GetMemoryAddress();

                if ( address == addr2Check )
                {
                    retVal = false;
                }
            }
        }
    }
    catch ( exception &e )
    {
        if ( m_DoDebug )
        {
            cout << "Si e'verificato un errore nel controllo indirizzi: " << e.what() <<endl;
        }
        retVal = false;
    }

    return retVal;
}

///////////////////////////////////////////////////
//             UpdateAllTempCtrlHyst
///////////////////////////////////////////////////
bool COneWireNet::UpdateAllHysteresisCtrl ( e_DeviceType controllerType, int netIndex, bool updateData )
{
    bool retVal = true;
    vector<CVController*>::iterator ctrlIt;

    if ( !CheckNetIndex ( netIndex ) )
    {
        return false;
    }

    for ( ctrlIt = m_NetList[netIndex].CtrlList.begin(); ctrlIt < m_NetList[netIndex].CtrlList.end(); ctrlIt++ )
    {
        if ( ( *ctrlIt )->GetControllerType() == controllerType )
        {
            retVal = ( *ctrlIt )->Update ( updateData ) && retVal;
            //TODO forse c'e'da mettere una segnalazione di errore in questa posizione
        }
    }

    return retVal;

}

///////////////////////////////////////////////////
//             SetActivitySearch
///////////////////////////////////////////////////
bool COneWireNet::SetActivitySearch ( int netIndex )
{
    int devIndex = 0;

    //Set activity search
    for ( unsigned devIndex = 0; devIndex < m_NetList[netIndex].deviceList.size(); devIndex++ )
    {
        if ( CheckDeviceFamily ( netIndex, devIndex, DS2408_FN ) )
        {
            ( ( CDS2408* ) ( m_NetList[netIndex].deviceList[devIndex] ) )->SetActivityConditionalSearch();
        }
        else if ( CheckDeviceFamily ( netIndex, devIndex, DS2405_FN ) )
        {
            //TODO da implementare
        }

    }

    return true;
}

/*!
    \fn COneWireNet::GetCtrlIndexByConfigNumber()
 */
int COneWireNet::GetCtrlIndexByConfigNumber ( int netIndex, int configNumber )
{
    int retVal = -1;
    unsigned int ctrlIndex = 0;
    int confIndex;

    for ( ctrlIndex = 0; ctrlIndex < m_NetList[netIndex].CtrlList.size(); ctrlIndex++ )
    {
        confIndex = m_NetList[netIndex].CtrlList[ctrlIndex]->GetConfigFileDevIndex();
        if ( confIndex ==  configNumber )
        {
            break;
        }
    }

    if ( ctrlIndex < m_NetList[netIndex].CtrlList.size() )
    {
        retVal = ctrlIndex;
    }

    return retVal;

}

/////////////////////////////////////////////////////
//InitializePIDs
/////////////////////////////////////////////////////
bool COneWireNet::InitializePIDs ( int netIndex )
{
    bool retVal = true;
    unsigned int devIndex = 0;

    if ( !CheckNetIndex ( netIndex ) )
    {
        return false;
    }

    //Cycle through all devices to search for analog AIAO
    for ( devIndex = 0; devIndex < m_NetList[netIndex].CtrlList.size(); devIndex++ )
    {
        if ( CheckControllerType ( netIndex, devIndex, DEV_PIDSIMPLE ) || CheckControllerType ( netIndex, devIndex, DEV_PIDLMD ) )
        {
            //Setup the Output channels
            retVal = ( ( CVPID* ) ( m_NetList[netIndex].CtrlList[devIndex] ) )->InitPID() && retVal;
        }
    }

    return retVal;
}

/////////////////////////////////////////////////////
//SetInitialParameters
/////////////////////////////////////////////////////
void COneWireNet::SetInitialParameters ( CIniFileHandler * handlr )
{
    m_IniFile = handlr;

    if ( m_IniFile != 0x0 )
    {
        //Get the debug variable
        m_DoDebug = m_IniFile->GetInt ( Config_Strings[CONF_DODEBUG], "COMMON" );

        m_BLRecoveryInterval = m_IniFile->GetInt ( Config_Strings[CONF_BLACKLISTTIMEOUT],"COMMON" );

        if ( m_BLRecoveryInterval < 0 )
        {
            m_BLRecoveryInterval = 0;
        }

        m_MAXNofErrors = m_IniFile->GetInt ( Config_Strings[CONF_MAXNOFERRORS],"COMMON" );

        if ( m_MAXNofErrors <= 0 )
        {
            m_MAXNofErrors = 5;
        }
    }

}

/////////////////////////////////////////////////////
//ManageAccKeys
/////////////////////////////////////////////////////
bool COneWireNet::ManageAccKeys ( bool addKey,CString name, uchar* snum, unsigned int expDateSec, int channel, bool sendToRemote )
{
    bool retVal = true;
    vector<CVController*>::iterator ctrlIt;

    for (unsigned int netIndex = 0; netIndex < m_NetList.size(); netIndex++ )
    {
        if ( m_NetList.at ( netIndex ).hasAccessControl )
        {
            for ( ctrlIt = m_NetList.at ( netIndex ).CtrlList.begin(); ctrlIt < m_NetList.at ( netIndex ).CtrlList.end();ctrlIt++ )
            {
                if ( ( *ctrlIt )->GetControllerType() == DEV_ACC )
                {
                    t_AccessData keyData;
                    char serialNumber[32];

                    keyData.name = name;
                    keyData.expireDateSec = expDateSec; //TBI
                    ConvertSN2Str(serialNumber, snum);
                    keyData.keySN = serialNumber;
                    keyData.channel = 2;
                    
                    if ( addKey )
                    {    
                        retVal = ( ( CNTH_ACC* ) ( *ctrlIt ) )->AddKey ( keyData ) && retVal;
                    }
                    else
                    {
                        retVal = ( ( CNTH_ACC* ) ( *ctrlIt ) )->RemoveKey ( keyData ) && retVal;
                    }
                    
                    if (retVal && sendToRemote) {
                        ( ( CNTH_ACC* ) ( *ctrlIt ) )->SendInfoToRemoteAcc(addKey, keyData);
                    }
                }
            }
        }
    }

    return retVal;
}


/*!
    \fn COneWireNet::GetDeviceHndlrByConfigNumber(int netIndex, int devIndex)
 */
CVDevice* COneWireNet::GetDeviceHndlrByConfigNumber(int netIndex, int configDevIndex)
{
    int devIndex;
    
    //TODO da controllare perche' a senso netIndex va da 0 a m_NetList.size()-1 quindi dovrebbe esserci anche un uguale
    if (m_NetList.size() < netIndex)
    {
        return 0x0;
    }

    devIndex = GetDeviceIndexByConfigNumber(netIndex, configDevIndex);

    if (devIndex == -1)
    {
        return 0x0;
    }

    return m_NetList[netIndex].deviceList[devIndex];
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVController* COneWireNet::GetControllerHndlrByMemoryAddress(int memoryAddress)
{
    int netIndex = GetNetByMemoryAddress(memoryAddress);
    if (netIndex < 0)
    {
        return 0x0;
    }
    for (unsigned int i = 0; i < m_NetList[netIndex].CtrlList.size(); i++)
    {
        if (m_NetList[netIndex].CtrlList[i]->GetMemoryAddress() == memoryAddress)
        {
            return m_NetList[netIndex].CtrlList[i];
        }
    }

    return 0x0;
}
