/***************************************************************************
 *   Copyright (C) 2005 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it                                           *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 ***************************************************************************/

#include "conewireengine.h"

        


bool COneWireEngine::ExecCommand( )
{
    CString messContent, addressStr, netStr;
    bool toNet = false, toAddress = false, toAllAddresses = false, toEveryOne = false;
    int netIndex = -1, devIndex = -1, address = -1, startIndex = -1, stopIndex = -1;
    bool commandExecuted = false;
    int command;
    int totalNumberOfNets = -1;
    //Se true viene inibita la risposta sull'esecuzione del comando
    bool suppressReturnMessage = false;

    command = ParseCommand();

    //get the content of the message
    messContent = m_XMLParser.GetContent();

    //Check the receivers of the command:
    if (m_XMLParser.GetStringParam("NET").size() != 0)
    {
        toNet = true;
        //Internal base for NETS is 1 not 0
        netIndex = m_XMLParser.GetIntParam("NET") - 1 ;
        netStr = "NET";
        netStr += netIndex;
    }

    if (m_XMLParser.GetStringParam("ADDRESS").size() != 0)
    {
        if (m_XMLParser.GetStringParam("ADDRESS") == "ALL")
        {
            toAllAddresses = true;
        }
        else
        {
            toAddress = true;
            address = m_XMLParser.GetIntParam("ADDRESS");
            addressStr = m_XMLParser.GetStringParam("ADDRESS");

            //Get also the netIndex for the current address
            netIndex = m_Net->GetNetByMemoryAddress(address);

            devIndex = m_Net->GetDeviceIndexByMemoryAddress(netIndex, address);

            //Check if netIndex and devIndex are correct
            if ( ( (netIndex < 0) || (devIndex < 0) ) && ((command != COMM_SETBUTTONCODE) && (command != COMM_CHECKOUTROOM)
                    && (command != COMM_SETIPADDR)) )
            {
                command = COMM_NUMTOT;
            }
        }
    }

    if ( (!toAddress) && (!toNet) )
    {
        toEveryOne = true;
        totalNumberOfNets = m_Net->GetTotalNumberOfNets();
    }

    //Setup Indexes for the various loop cycles
    if (toEveryOne)
    {
        startIndex = 0;
        stopIndex = totalNumberOfNets;
    }
    else if (toNet)
    {
        startIndex = netIndex;
        stopIndex = netIndex+1;
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    //NUOVO CODICE PER LA GESTIONE DEI COMANDI
    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    //Per ora lo metto solo per l'esecuzione diretta del comando
    if (toAddress)
    {
        T_Net* net = m_Net->GetNetHandler(netIndex);

        if ( (net != 0x0) && (m_Net->AcquireNet( netIndex )) )
        {
            if ( net->CtrlList[devIndex]->GetCodeRevision() > 0 )
            {
                commandExecuted = net->CtrlList[devIndex]->ExecCommand(&m_XMLParser);

                if (commandExecuted)
                {
                    messContent = this->m_XMLParser.AcknowledgeCommand( commandStrings[command], commandExecuted);
                    UpdateServerPorts(true, false);
                    WriteOnInterfacePorts( messContent.c_str(), messContent.size() );
                    return true;
                }

                //Altrimenti vado alla sezione sotto per vedere se può essere eseguito...
            }
        }
    }
    else
    {
        //TBD
    }

    

    switch (command)
    {
        case COMM_CHANGEINIFILE:{

            int fieldCounter = 1;
            bool rebootOnExit = false;

            CString newFile = m_XMLParser.GetStringParam(CString("INIFILE")+fieldCounter);
            CString fileName = m_XMLParser.GetStringParam(CString("FILENAME")+fieldCounter);
            fieldCounter++;

            while ((newFile.length() > 0) && (fileName.length() > 0)) {
                
                //Qui lo devo prendeere e salvare....
                if (ReplaceAfoFile(fileName.c_str(),newFile.c_str()))
                {
                    rebootOnExit = true;
                }

                newFile = m_XMLParser.GetStringParam(CString("INIFILE")+fieldCounter);
                fileName = m_XMLParser.GetStringParam(CString("FILENAME")+fieldCounter);
                fieldCounter++;
            }

            if (rebootOnExit)
            {
                commandExecuted = true;
#if (defined(CRIS)||defined(ARM))
                system("reboot");
#else
                cout << "Cambia file" << fileName<<" lanciato!!" <<endl;
                sleep(5);
                this->SetRunLevel(0);
                break;
#endif
            }

        };break;
        case COMM_SETIPADDR:{
            //Ricevo l'IP
            CString newIP = m_XMLParser.GetStringParam("IPADDRESS");
            CIniFileManager iniFile;

            if (!iniFile.Load(CONFIG_FILE)){
                break;
            }

            char *nOfInterfacesKeys[] = {"NPorteInterfaccia","NPorteComIn","NPorteComOut"};
            char *interfacesKeys[] = {"PortaInterfaccia","PortaComunicazioneIn","PortaComunicazioneOut"};
            //Qui memorizzo l'indirizzo attuale per poter cercare tra le porte out
            //se ce n'è una uguale alla mia
            CString originalAddress;
            iniFile.GetConfigParamString(iniFile.GetString("PortaInterfaccia1","COMMON","").c_str(),
                    "ServerIPAddr",&originalAddress,"192.168.0.90");

            
            //Ora devo:
            //Cambiare tutte le interfacce nel config.ini
            for (int interfaceType = 0; interfaceType< 3; interfaceType++ )
            {
                int nOfInterfaces = iniFile.GetInt(nOfInterfacesKeys[interfaceType],"COMMON",0);

                for (int interfaceNumber = 1; interfaceNumber <= nOfInterfaces; interfaceNumber++){
                    //Carico la porta e confronto
                    CString portKey = interfacesKeys[interfaceType];
                    CString portAddressStr;

                    portKey+=interfaceNumber;
                    CString portString = iniFile.GetString(portKey,"COMMON","");
                    if (portString.length() == 0){
                        //C'e' qualcosa che non va
                        continue;
                    }

                    iniFile.GetConfigParamString(portString.c_str(),"ServerIPAddr",&portAddressStr,"");

                    if (!strcasecmp(portAddressStr.c_str(),originalAddress.c_str())){
                        //Ok cambio il campo ServerIPAddr
                        iniFile.SetConfigParamString(&portString,"ServerIPAddr",newIP.c_str());
                        iniFile.SetValue(portKey,portString,"","COMMON");
                    }
                }
            }

            iniFile.Save();

            //Aggiornare le  impostazioni di rete del cervelletto
            CString newSystemConfigString;

#ifdef ARM
            newSystemConfigString = "ifconfig eth0 ";
            newSystemConfigString += newIP;
            newSystemConfigString += " netmask 255.255.255.0\n";
            char filename[]={"/etc/rc"};
            UpdateSystemFile(newSystemConfigString.c_str(), filename);
            system("reboot");
#elif defined(CRIS)
            //TODO
            break;
#else
            //Questo e' solo per test
            newSystemConfigString = "ifconfig eth0 ";
            newSystemConfigString += newIP;
            newSystemConfigString += " netmask 255.255.255.0\n";
            char filename[] = {"./rc"};
            UpdateSystemFile(newSystemConfigString.c_str(), filename);
#endif

        };break;
        case COMM_CHANGEDRIVERDATA:
        {
            int netIndex = m_XMLParser.GetIntParam("NET");
            int devIndex = m_XMLParser.GetIntParam("DEVINDEX");

            CVDevice *device = m_Net->GetDeviceHndlrByConfigNumber(netIndex-1,devIndex);

            if (device != 0x0){
                CString param;

                param = m_XMLParser.GetStringParam("COMMENT");
                device->SetComment(param);
                m_Net->UpdateIniFile(netIndex-1, devIndex,"COMMENT",param);

                param = m_XMLParser.GetStringParam("SN");
                device->SetSN(param.c_str());
                m_Net->UpdateIniFile(netIndex-1, devIndex,"SN",param);

                commandExecuted = true;

                if ( (device->GetDeviceType()==DEV_DS18B20) || (device->GetDeviceType()==DEV_DS18S20) ){
                    //Aggiorno internamente il tipo di dispositivo
                    ((CDS18X20*)device)->SetFamilyAndDevice();
                }
            }
        };break;
        case COMM_SETTALARM: //SetAlarm
        {
            int maxAlarm, minAlarm;

            //Get the alarm values
            maxAlarm = m_XMLParser.GetIntParam("ALLMAX");
            minAlarm = m_XMLParser.GetIntParam("ALLMIN");

            if ( toEveryOne || toNet)
            {
                //We have to change the setting for all sensors of all nets

                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    if (m_Net->NetHasTempDevices( netIndex ) )
                    {
                        //Open net
                        if (m_Net->AcquireNet(netIndex))
                        {
                            //Set the alarms
                            commandExecuted = m_Net->SetAllAlarmT(netIndex, maxAlarm, minAlarm) && commandExecuted;

                            //Close NET
                            //m_Net->ReleaseNet(netIndex);
                        }
                        else
                        {
                            commandExecuted = false;
                        }
                    }
                }


            }
            else if (toAddress)
            {
                //Acquire the NET
                if (m_Net->AcquireNet(netIndex))
                {
                    //Set the alarms
                    if (!m_Net->SetAlarmT(netIndex, devIndex, maxAlarm, minAlarm))
                    {
                        cout << "An error occurred in setting the new temp!!" << endl; cout.flush();

                        commandExecuted = false;
                    }
                    else
                    {
                        cout << "New Alarm correcly set" << endl; cout.flush();

                        //Send a reset
                        m_Net->SendReset(netIndex);

                        //Close the net
                        //m_Net->ReleaseNet(netIndex);

                        commandExecuted = true;
                    }
                }
                else
                {
                    commandExecuted = false;
                }
            }
        }
        break;
        case COMM_GETTEMP://Gettemp
        {
            float temps[MAX_NUM_DEV_PER_NET][2];
            int numTemps = 0;
            bool updateFirst = false;

            memset (temps, 0x0, MAX_NUM_DEV_PER_NET*2*sizeof(float));

            updateFirst = m_XMLParser.GetBoolParam("UPDATE");

            if (toAddress)
            {
                if (m_Net->AcquireNet( netIndex ))
                {
                    temps[0][0] = devIndex;
                    temps[0][1] = m_Net->GetTemp(netIndex, devIndex, updateFirst);
                    numTemps = 1;
                    commandExecuted = OutputFloatData( netIndex, numTemps, temps);

                    //m_Net->ReleaseNet( netIndex );
                }
            }
            else if (toEveryOne || toNet)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    if (m_Net->NetHasTempDevices( netIndex ))
                    {
                        if (m_Net->AcquireNet( netIndex ))
                        {
                            if (updateFirst)
                            {
                                numTemps = m_Net->GetAllTemp( netIndex, updateFirst, temps, MAX_NUM_DEV_PER_NET );
                            }
                            else
                            {
                                numTemps = m_Net->GetAllLastTemp( netIndex, temps, MAX_NUM_DEV_PER_NET );
                            }

                            if (numTemps > 0)
                            {
                                commandExecuted = OutputFloatData( netIndex, numTemps, temps) && commandExecuted;
                            }
                            else
                            {
                                commandExecuted = false;
                            }

                            //Reset the array
                            memset (temps, 0x0, MAX_NUM_DEV_PER_NET*2*sizeof(float));

                            //m_Net->ReleaseNet( netIndex );
                        }
                        else
                        {
                            commandExecuted = false;
                        }
                    }
                }
            }

        }
        break;
        case COMM_ENABLETIMER:
        {
            bool newState;

            newState = m_XMLParser.GetBoolParam("STATE");

            if (toEveryOne)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    commandExecuted = m_Net->EnableAllTimers(netIndex, newState) && commandExecuted;
                    commandExecuted = m_Net->EnableNetTimer(netIndex, newState) && commandExecuted;
                }
            }
            else if (toNet)
            {
                //Check if we have to change the timer state of all controllers in the NET
                if (toAllAddresses)
                {
                    //Change the timer state of ALL controllers in the NET
                    commandExecuted = m_Net->EnableAllTimers(netIndex, newState);
                }
                else
                {
                    //Change timerState for the NET
                    commandExecuted = m_Net->EnableNetTimer(netIndex, newState);
                }
            }
            else if (toAddress)
            {
                //Change timer state for controller at address
                m_Net->UseTimer(netIndex, devIndex, newState);
            }
        };
        break;
        case COMM_SETTIMERID:
        {
            bool newID;

            newID = m_XMLParser.GetIntParam( "TIMERID" );

            if (toEveryOne)
            {
                //Change timer state to ALL objects
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    commandExecuted = m_Net->SetAllTimerID(netIndex, newID) && commandExecuted;
                    commandExecuted = m_Net->SetNetTimerID(netIndex, newID) && commandExecuted;
                }
            }
            else if (toNet)
            {
                //Check if we have to change the timer ID of all controllers in the NET
                if (toAllAddresses)
                {
                    //Change the timer state of ALL controlloers in the NET
                    commandExecuted = m_Net->SetAllTimerID(netIndex, newID);
                }
                else
                {
                    //Change timerID for the NET
                    commandExecuted = m_Net->SetNetTimerID(netIndex, newID);
                }
            }
            else if (toAddress)
            {
                //Change timer state for controller at address
                m_Net->SetTimerID(netIndex, devIndex, newID);
            }
        }
        break;
        case COMM_CHANGETIMERSETTINGS:
        {

            CString day, newSettings;
            int timerID;

            //first get the timer ID
            timerID = m_XMLParser.GetIntParam("TIMERID");
            //then get the day
            day = m_XMLParser.GetStringParam( "DAY" );
            //Then get the settings
            newSettings = m_XMLParser.GetStringParam( "SETTING" );

            if ((timerID < 0) || (day.size() == 0) || (newSettings.size() == 0))
            {
                commandExecuted = false;
            }
            else
            {
                //Check if we have to set it up for everyday of the week
                if (day == "ALL" )
                {
                    int dayIndex = 0;

                    //Set commandExecuted to true to check the following loop correctly
                    commandExecuted = true;

                    for (dayIndex = 0; dayIndex < 7; dayIndex++)
                    {
                        commandExecuted = m_Timer.UpdateTimerSettings(timerID, daysStrings[dayIndex], newSettings ) && commandExecuted;
                    }
                }
                else
                {
                    commandExecuted = m_Timer.UpdateTimerSettings(timerID, day, newSettings );
                }
            }

            //Faccio la push dei comandi per riforzare l'invio dei dati timer
//             CString newCommand = "<DEVICE COMMAND=\"GetTimerSettings\" />";
//             m_CommandQueue.push_back(newCommand);

        }
        break;
        case COMM_CHANGETIMERLEVELS:
        {
            CString l1Settings, l2Settings, l3Settings;
            int timerID;

            //Get the levels
            l1Settings = m_XMLParser.GetStringParam("LEVEL1");
            l2Settings = m_XMLParser.GetStringParam("LEVEL2");
            l3Settings = m_XMLParser.GetStringParam("LEVEL3");

            if ( (l1Settings.size() != 0) || (l2Settings.size() != 0) || (l3Settings.size() != 0 ))
            {
                if (m_XMLParser.GetStringParam( "TIMERID" ) == "ALL")
                {
                //Levels modification for all the timers
                //TODO da implementare perche' non so se ha senso
                }
                else
                {
                    timerID = m_XMLParser.GetIntParam("TIMERID");

                    //Set commandExecuted to true to check the following instrctions
                    commandExecuted = true;
                    if (l1Settings.size() != 0)
                    {
                        commandExecuted = m_Timer.UpdateTimerLevels(timerID, 1, l1Settings) && commandExecuted;
                    }

                    if (l2Settings.size() != 0)
                    {
                        commandExecuted = m_Timer.UpdateTimerLevels(timerID, 2, l2Settings) && commandExecuted;
                    }

                    if (l3Settings.size() != 0)
                    {
                        commandExecuted = m_Timer.UpdateTimerLevels(timerID, 3, l3Settings) && commandExecuted;
                    }
                }

                //Faccio la push dei comandi per riforzare l'invio dei dati timer
//                 CString newCommand = "<DEVICE COMMAND=\"GetTimerSettings\" />";
//                 m_CommandQueue.push_back(newCommand);
            }
        };
        break;
      case COMM_SETDIGITALOUTPUT:
      {
        bool value;

        if (!m_XMLParser.GetBoolParam("STATE", &value))
        {
          m_XMLParser.GetBoolParam( "VAL", &value);
        }

        if (toAddress)
        {
          if (m_Net->AcquireNet( netIndex ))
          {
//                     net = m_Net->GetNetHandler( netIndex );
//                     if (((CVDIDO*)(net->CtrlList.at(devIndex)))->IsOutputInverted())
//                     {
//                         value = !value;
//                     }

            commandExecuted = m_Net->SetDigitalOutput( address, value);
                    //m_Net->ReleaseNet(netIndex);
          }
          else
          {
            commandExecuted = false;
          }
        }
        else if ((toEveryOne) || toNet)
        {
          commandExecuted = true;

          for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
          {
            if (m_Net->NetHasDIDOs( netIndex ))
            {
              if ( m_Net->AcquireNet( netIndex ) )
              {
                            //We have to change the state of every device
                commandExecuted = m_Net->SetAllDigitalOutputs( netIndex, value);
              }
              else
              {
                commandExecuted = false;
              }
            }
          }
        }
      };
        break;
        case COMM_GETDIDO:
        {
            int didoMatrix[MAX_NUM_DEV_PER_NET][2];
            int value, nOfDIDOs, didoIndex;
            CString valueStr;

            memset (didoMatrix, 0, MAX_NUM_DEV_PER_NET*2*sizeof(int));

            if (toAddress)
            {
                if (m_Net->AcquireNet( netIndex ))
                {
                    value = m_Net->GetDigitalInput( address );
                    //m_Net->ReleaseNet(netIndex);

                    if (value)
                    {
                        valueStr = "1";
                    }
                    else
                    {
                        valueStr = "0";
                    }

                    messContent = m_XMLParser.CreateMessage( "DEVICE", 4, "DIDO", "NA", addressStr.c_str(), valueStr.c_str() );

                    //Write out the message
                    UpdateServerPorts(true, false);
                    WriteOnInterfacePorts( messContent.c_str(), messContent.size());

                }
                else
                {
                    commandExecuted = false;
                }
            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    if ( m_Net->NetHasDIDOs( netIndex ) )
                    {
                        if (  m_Net->AcquireNet( netIndex ) )
                        {
                            //We have to change the state of every device
                            nOfDIDOs = m_Net->UpdateDIDOs( netIndex, didoMatrix);

                            //m_Net->ReleaseNet( netIndex );

                            for (didoIndex = 0; didoIndex < nOfDIDOs; didoIndex++ )
                            {
                                char addrBuffer[32], stateBuffer[8];
                                int addr;

                                addr = m_Net->GetControllerMemoryAddress(netIndex, didoMatrix[didoIndex][0]);

                                if (addr > -1)
                                {
                                    sprintf (addrBuffer, "%d", addr);
                                    sprintf (stateBuffer, "%d", didoMatrix[didoIndex][1]);
                                    messContent = m_XMLParser.CreateMessage( "DEVICE", 4, "DIDO", "NA", addrBuffer, stateBuffer );

                                    //Write out the message
                                    UpdateServerPorts(true, false);
                                    WriteOnInterfacePorts( messContent.c_str(), messContent.size());

                                    commandExecuted = true;
                                }
                                else
                                {
                                    commandExecuted = false;
                                }
                            }
                        }
                    }
                    else
                    {
                        commandExecuted = true;
                    }
                }
            }
        }
        break;
        case COMM_SETPIDPARAM:
        {
            float parameters[6];
            bool isLMD = false;
            bool isMGC = false;
            bool isC3Point = false;

            //Get the parameters
            parameters[0] = m_XMLParser.GetFloatParam("KAPPA");
            parameters[1] = m_XMLParser.GetFloatParam("TINT");
            parameters[2] = m_XMLParser.GetFloatParam("TDER");

            //Check if it is a PID LMD
            if (m_XMLParser.GetFloatParam( "KAPPA2", &parameters[3]) && m_XMLParser.GetFloatParam( "TINT2", &parameters[4]) && m_XMLParser.GetFloatParam( "TDER2", &parameters[5]))
            {
                isLMD = true;
            }
            else if (m_XMLParser.GetFloatParam( "KDIV", &parameters[3])){
                isMGC = true;
            }
            else if (m_XMLParser.GetFloatParam("NULLZONE",parameters)){
                isC3Point = true;
            }

            if (toAddress)
            {
                if (isLMD)
                {
                    //Set all six parameters
                    commandExecuted = m_Net->SetPIDParameters( address, parameters, 6 );
                }
                else if (isMGC){
                    commandExecuted = m_Net->SetPIDParameters( address, parameters, 4 );
                }
                else if (isC3Point){
                    T_Net *net = m_Net->GetNetHandler(netIndex);

                    if (m_Net->CheckControllerType(netIndex,devIndex,DEV_C3POINT)){
                        ((C3PointCtrl*)(net->CtrlList[devIndex]))->m_NullZoneAmplitude = parameters[0];
                        int configIndex = m_Net->GetControllerHndlrByMemoryAddress(address)->GetConfigFileDevIndex();
                        m_Net->UpdateIniFile(netIndex,configIndex,"NULLZONE",CString("")+parameters[0]);
                        if (m_XMLParser.GetFloatParam("MOVEMENT",&parameters[1]))
                        {
                            ((C3PointCtrl*)(net->CtrlList[devIndex]))->m_MovementTimeOut = (int)parameters[1];
                            m_Net->UpdateIniFile(netIndex,configIndex,"MOVETIMEOUT",CString("")+parameters[1]);
                        }

                        
                        
                        commandExecuted = true;
                    }
                }
                else
                {
                    //Set just the primary controller parameters
                    commandExecuted = m_Net->SetPIDParameters( address, parameters, 3 );
                }
            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    if (m_Net->NetHasPIDs( netIndex ))
                    {
                        if ( isLMD )
                        {
                            commandExecuted = m_Net->SetAllPIDParameters( address, parameters, 6 ) && commandExecuted;
                        }
                        else if (isMGC){
                            commandExecuted = m_Net->SetAllPIDParameters( address, parameters, 4 ) && commandExecuted;
                        }
                        else
                        {
                            commandExecuted = m_Net->SetAllPIDParameters( address, parameters, 3 ) && commandExecuted;
                        }
                    }
                }
            }
        }
        break;
        case COMM_GETPIDINFO:
        {
            float parameters[10];
            bool isSummer;
            CString pidType;
            int ctrlIndex=-1, nOfControllers = -1;
            T_Net *net;

            if (toAddress)
            {
                    //Get handler to the NET
                    net = m_Net->GetNetHandler( netIndex );

                    if (m_Net->AcquireNet( netIndex ))
                    {
                        if (m_Net->CheckControllerType( netIndex, devIndex, DEV_PIDSIMPLE) )
                        {
                           commandExecuted = ( (CPIDSimple*) (net->CtrlList[devIndex] ) )->ExecCommand(&m_XMLParser);
                        }
                        else if (m_Net->CheckControllerType( netIndex, devIndex, DEV_PIDLMD) )
                        {
                            commandExecuted = ( (CPIDLMD*) (net->CtrlList[devIndex] ) )->ExecCommand(&m_XMLParser);
                        }
                        else
                        {
                            commandExecuted = false;
                        }
                    }

                //Get all six parameters
//                commandExecuted = m_Net->GetPIDSetup( address, parameters, &isSummer, pidType );
//
//                if (commandExecuted)
//                {
//                    if (pidType == Device_strings[DEV_PIDLMD])
//                    {
//                        messContent = m_XMLParser.CreateMessage( "DEVICE", 14, Device_strings[DEV_PIDLMD], addressStr.c_str(), "Settings", parameters[0],
//                                parameters[1],
//                                parameters[2],
//                                parameters[3],
//                                parameters[4],
//                                parameters[5],
//                                parameters[6],
//                                parameters[7],
//                                parameters[8],
//                                parameters[9],
//                                isSummer);
//                    }
//                    else if (pidType == Device_strings[DEV_PIDSIMPLE])
//                    {
//                        messContent = m_XMLParser.CreateMessage( "DEVICE", 9, Device_strings[DEV_PIDSIMPLE], addressStr.c_str(),"Settings", parameters[0], parameters[1], parameters[2], parameters[3], parameters[4], isSummer);
//                    }
//                    else
//                    {
//                        messContent = "Errore!! Il dispositivo NON e' un PID";
//                    }
//
//                    UpdateServerPorts(true, false);
//                    WriteOnInterfacePorts( messContent.c_str(), messContent.size() );
//                }
            }
            else if ((toEveryOne) || toNet)
            {
                CString outMessage;

                commandExecuted = true;


                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    nOfControllers = m_Net->GetNofControllers(netIndex);

                    for (ctrlIndex = 0; ctrlIndex < nOfControllers; ctrlIndex++)
                    {
                        if (m_Net->CheckControllerType(netIndex, ctrlIndex, DEV_PIDLMD) ||
                            m_Net->CheckControllerType(netIndex, ctrlIndex, DEV_PIDSIMPLE) ||
                            m_Net->CheckControllerType(netIndex, ctrlIndex, DEV_UTACTRL)
                           )
                        {
                            outMessage = "";
                            address = m_Net->GetControllerMemoryAddress( netIndex, ctrlIndex);
                            addressStr="";
                            addressStr+=address;

                            if (m_Net->GetPIDSetup( address, parameters, &isSummer, pidType ))
                            {
                                outMessage="<DEVICE TYPE=\"PIDInfo\" ";
                                outMessage+=" ADDRESS=\"";
                                outMessage+=address;
                                outMessage+="\" SUMMER=\"";
                                outMessage+=(int)isSummer;
                                outMessage+="\" KAPPA=\"";
                                outMessage+=parameters[0];
                                outMessage+="\" TINT=\"";
                                outMessage+=parameters[1];
                                outMessage+="\" TDER=\"";
                                outMessage+=parameters[2];
                                outMessage+="\" ";

                                if (pidType == Device_strings[DEV_PIDLMD])
                                {

                                    outMessage+="KAPPA2=\"";
                                    outMessage+=parameters[3];
                                    outMessage+="\" TINT2=\"";
                                    outMessage+=parameters[4];
                                    outMessage+="\" TDER2=\"";
                                    outMessage+=parameters[5];
                                    outMessage+="\" SETPOINT=\"";
                                    outMessage+=parameters[6];
                                    outMessage+="\" SETPOINTH=\"";
                                    outMessage+=parameters[7];
                                    outMessage+="\" SETPOINTL=\"";
                                    outMessage+=parameters[8];
                                    outMessage+="\" TEMP=\"";
                                    outMessage+=parameters[9];

//                                     messContent = m_XMLParser.CreateMessage( "DEVICE", 14, Device_strings[DEV_PIDLMD], addressStr.c_str(), "Settings", parameters[0],
//                                             parameters[1],
//                                             parameters[2],
//                                             parameters[3],
//                                             parameters[4],
//                                             parameters[5],
//                                             parameters[6],
//                                             parameters[7],
//                                             parameters[8],
//                                             parameters[9],
//                                             isSummer);
                                }
                                else
                                {

                                    outMessage+="SETPOINT=\"";
                                    outMessage+=parameters[3];
                                    outMessage+="\" TEMP=\"";
                                    outMessage+=parameters[4];

//                                     messContent = m_XMLParser.CreateMessage( "DEVICE", 9, Device_strings[DEV_PIDSIMPLE], addressStr.c_str(),"Settings", parameters[0], parameters[1], parameters[2], parameters[3], parameters[4], isSummer);
                                }

                                outMessage+= "\" />";

                                UpdateServerPorts(true, false);
                                WriteOnInterfacePorts( outMessage.c_str(), outMessage.size() );
                            }
                        }
                    }
                }
            }
        }
        break;
        case COMM_SETPIDSETPOINT:
        case COMM_SETSETPOINT:
        {
            float parameters[3];
            CString pidType;
            T_Net* net;

            parameters[0] = m_XMLParser.GetFloatParam( "SETPOINT" );


            if (toAddress)
            {
                net = m_Net->GetNetHandler(netIndex);

                if ( (net != 0x0) && (m_Net->AcquireNet( netIndex )) )
                {
                    if ((m_Net->CheckControllerType(netIndex, devIndex, DEV_PIDSIMPLE)) || (m_Net->CheckControllerType ( netIndex, devIndex, DEV_MGC ) ) )
                    {
                        commandExecuted = m_Net->SetSetpoint( netIndex, devIndex, parameters, 1) ;
                    }
                    else if ((m_Net->CheckControllerType(netIndex, devIndex, DEV_PIDLMD) ) ||  (m_Net->CheckControllerType ( netIndex, devIndex,DEV_UTACTRL ) ) )
                    {
                        if ( m_XMLParser.GetFloatParam( "SETPOINTH", &parameters[1] ) && m_XMLParser.GetFloatParam( "SETPOINTL", &parameters[2] ))
                        {
                            commandExecuted = m_Net->SetSetpoint( netIndex, devIndex, parameters, 3) ;
                        }
                        else
                        {
                            commandExecuted = m_Net->SetSetpoint( netIndex, devIndex, parameters, 1) ;
                        }
                    }
                    else if (m_Net->CheckControllerType(netIndex, devIndex, DEV_FLOORCOORD_2) )
                    {
                        int linkedZone = -1;
                        //Controllo se ci sono altri parametri:
                        //Controllo se va a piu' di una zona
                        parameters[1] = m_XMLParser.GetFloatParam("ZONE"); //Questo e' per il controllo pavimenti
                        commandExecuted = ( (CFloorCoord2*) (net->CtrlList[devIndex] ) )->SetSetPoint ( (int)(parameters[1]), parameters[0] );
                        linkedZone = ( (CFloorCoord2*) (net->CtrlList[devIndex] ) )->GetZoneNumberPtr((int)parameters[1])->linkedZone;

                        if (linkedZone > 0)
                        {
                            ( (CFloorCoord2*) (net->CtrlList[devIndex] ) )->SetSetPoint ( linkedZone, parameters[0] );
                        }
                    }
                    else if (m_Net->CheckControllerType(netIndex, devIndex, DEV_AFOVLV) )
                    {
                        commandExecuted = ( (CNTHVLV2*) (net->CtrlList[devIndex] ) )->ExecCommand ( &m_XMLParser );
//                        //Se c'è questo parametro differenzio i setpoint, altrimenti no
//                        if (m_XMLParser.ExistsParam("SETPOINT2"))
//                        {
//                            commandExecuted = ( (CNTHVLV2*) (net->CtrlList[devIndex] ) )->SetPIDSetpoint ( 1, parameters[0] );
//                            parameters[0] = m_XMLParser.GetFloatParam("SETPOINT2");
//                            commandExecuted = ( (CNTHVLV2*) (net->CtrlList[devIndex] ) )->SetPIDSetpoint ( 2, parameters[0] ) && commandExecuted;
//                        }
//                        else
//                        {
//                            commandExecuted = ( (CNTHVLV2*) (net->CtrlList[devIndex] ) )->SetPIDSetpoint ( 0, parameters[0] );
//                        }
                    }

                    //Aggiunto il 6/11/2009
                    else if (m_Net->CheckControllerType(netIndex, devIndex, DEV_AFOVLV_VAV) )
                    {
                        commandExecuted = ( (CNTHVLV2_VAV*) (net->CtrlList[devIndex] ) )->ExecCommand ( &m_XMLParser );
                        //Se c'è questo parametro differenzio i setpoint, altrimenti no
//                        if (m_XMLParser.ExistsParam("SETPOINT2"))
//                        {
//                            commandExecuted = ( (CNTHVLV2_VAV*) (net->CtrlList[devIndex] ) )->SetPIDSetpoint ( 1, parameters[0] );
//                            parameters[0] = m_XMLParser.GetFloatParam("SETPOINT2");
//                            commandExecuted = ( (CNTHVLV2_VAV*) (net->CtrlList[devIndex] ) )->SetPIDSetpoint ( 2, parameters[0] ) && commandExecuted;
//                        }
//                        else
//                        {
//                            commandExecuted = ( (CNTHVLV2_VAV*) (net->CtrlList[devIndex] ) )->SetPIDSetpoint ( 0, parameters[0] );
//                        }
                    }

                    else if (m_Net->CheckControllerType(netIndex, devIndex, DEV_FULLUTACTRL) )
                    {
                        commandExecuted = ( (CFullUTACtrl*) (net->CtrlList[devIndex] ) )->SetSetPoint ( parameters[0] );   
                    }
                    else if (m_Net->CheckControllerType(netIndex, devIndex, DEV_FULLUTACTRL_2) )
                    {
                        commandExecuted = ( (CFullUTACtrl2*) (net->CtrlList[devIndex] ) )->SetSetPoint ( parameters[0] );   
                    }
                    else if (m_Net->CheckControllerType(netIndex, devIndex, DEV_C3POINT) )
                    {
                        ( (C3PointCtrl*) (net->CtrlList[devIndex] ) )->SetSetpoint ( parameters[0] );

                        if ( (( (C3PointCtrl*) (net->CtrlList[devIndex] ) )->m_IsLMD) &&
                                 m_XMLParser.GetFloatParam( "SETPOINTH", &parameters[1] ) &&
                                 m_XMLParser.GetFloatParam( "SETPOINTL", &parameters[2] ))
                        {
                            ( (C3PointCtrl*) (net->CtrlList[devIndex]) )->SetLMDSetpoint( parameters[1], parameters[2]) ;
                        }

                        commandExecuted = true;
                    }
                    else if (m_Net->CheckControllerType(netIndex, devIndex, DEV_VLVCTRL))
                    {
#ifndef USE_ADV_VLV
                        commandExecuted = ((CNTHVLV*)(net->CtrlList[devIndex]))->SetSetPoint(parameters[0]);
#else
                        commandExecuted = ((CNTHVLV_ADV*)(net->CtrlList[devIndex]))->SetSetPoint(parameters[0]);
#endif
                    }
                }
            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;
                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    if ( m_XMLParser.GetFloatParam( "SETPOINTH", &parameters[1] ) && m_XMLParser.GetFloatParam( "SETPOINTL", &parameters[2] ))
                    {
                        commandExecuted = m_Net->SetAllPIDSetpoint( netIndex, parameters, 3) && commandExecuted;

                    }
                    else
                    {
                        commandExecuted = m_Net->SetAllPIDSetpoint( netIndex, parameters, 1,&m_XMLParser) && commandExecuted;
                    }
                }
            }
        }
        break;
        case COMM_SETSUMMER:
        {
            bool isSummer = false;

            if (m_XMLParser.ExistsParam("VAL")) {
                isSummer = m_XMLParser.GetBoolParam( "VAL" );
            }
            else if (m_XMLParser.ExistsParam("SUMMER")) {
                isSummer = m_XMLParser.GetBoolParam( "SUMMER" );
            }
            else {
                //Esco tanto non so che fare
                break;
            }

            if (toAddress)
            {
                commandExecuted = m_Net->SetSummer( isSummer, address);
            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    commandExecuted = m_Net->SetAllSummer(netIndex, isSummer);
                }
            }
        };
        break;
        case COMM_SETPOLLMODE:
        {
            CString typeOfDevice;

            typeOfDevice = m_XMLParser.GetStringParam( "TYPE" );

            if (typeOfDevice == "TEMP")
            {
                m_PollTemperature = m_XMLParser.GetBoolParam( "ENABLE" );
                commandExecuted = true;
            }
            else if (typeOfDevice == "DIGITALIO" )
            {
                m_PollDigitalIO = m_XMLParser.GetBoolParam( "ENABLE" );
                commandExecuted = true;
            }
            else if (typeOfDevice == "ALARMS")
            {
                m_PollTempAlarms = m_XMLParser.GetBoolParam( "ENABLE" );
                commandExecuted = true;
            }
            else if (typeOfDevice == "HUM")
            {
                m_PollHum = m_XMLParser.GetBoolParam( "ENABLE" );
                commandExecuted = true;
            }
            else if (typeOfDevice == "PID")
            {
                m_PollPID = m_XMLParser.GetBoolParam( "ENABLE" );
                commandExecuted = true;
            }
            else if (typeOfDevice == "ANALOGIO")
            {
                m_PollAnalogIO = m_XMLParser.GetBoolParam( "ENABLE" );
                commandExecuted = true;
            }

        };
        break;
        case COMM_GETHUMIDITY:
        {
            float humidities[MAX_NUM_DEV_PER_NET][3];
            int numHums = 0;

            if (toAddress)
            {
                if (m_Net->AcquireNet( netIndex ))
                {
                    numHums = 1;
                    humidities[0][0] = devIndex;
                    commandExecuted = m_Net->GetHumidity(netIndex, devIndex, &humidities[0][1], &humidities[0][2]);
                    if (commandExecuted)
                    {
                        OutputFloatData( netIndex, numHums, humidities);
                    }
                }

            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    if (m_Net->NetHasHums( netIndex ))
                    {
                        if (m_Net->AcquireNet( netIndex ))
                        {
                            numHums = m_Net->GetAllHumidities(netIndex, humidities, numHums);

                            if (numHums < 0)
                            {
                                commandExecuted = false;
                            }
                            else if (numHums > 0)
                            {
                                OutputFloatData(netIndex, numHums, humidities);
                            }
                        }
                        else
                        {
                            commandExecuted = false;
                        }
                    }
                }
            }
        };
        break;
        case COMM_SETHUMIDITYSETTINGS:
        {
            CString tempString;
            float setpoint= -1.0, hysteresis = -1.0;
            bool enableAutoControl = false;
            bool setAutoControl = false, setSetPoint = false, setHysteresis = false;

            //GetParameters
            //Set state of the automatic control
            tempString = m_XMLParser.GetStringParam( "AUTO" );
            if (tempString.size() != 0)
            {
                setAutoControl = true;
                enableAutoControl = m_XMLParser.GetBoolParam( "AUTO" );

            }

            //Set setpoint
            tempString = m_XMLParser.GetStringParam( "SETPOINT" );
            if (tempString.size() != 0)
            {
                setSetPoint = true;
                setpoint = m_XMLParser.GetFloatParam( "SETPOINT" );

            }

            //Set hysteresis
            tempString = m_XMLParser.GetStringParam( "HYST" );
            if (tempString.size() != 0)
            {
                setHysteresis = true;
                hysteresis = m_XMLParser.GetFloatParam( "HYST" );

            }

            if (toAddress)
            {
                if (m_Net->AcquireNet( netIndex))
                {
                    commandExecuted = true;

                    if (setAutoControl)
                    {
                        commandExecuted = m_Net->SetHumAutoControl( address, enableAutoControl) && commandExecuted;
                    }

                    if ( setSetPoint )
                    {
                        commandExecuted = m_Net->SetHumSetPoint( address, setpoint) && commandExecuted;
                    }

                    if ( setHysteresis )
                    {
                        commandExecuted = m_Net->SetHumidityHysteresis( address, hysteresis ) && commandExecuted;
                    }

                    //m_Net->ReleaseNet( netIndex );
                }
            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    if (m_Net->NetHasHums( netIndex ))
                    {
                        if (m_Net->AcquireNet( netIndex ))
                        {
                            if (setAutoControl)
                            {
                                commandExecuted = m_Net->SetAllHumAutoControl(netIndex, enableAutoControl) && commandExecuted;
                            }

                            if ( setSetPoint )
                            {
                                commandExecuted = m_Net->SetAllHumSetPoint( netIndex, setpoint) && commandExecuted;
                            }

                            if ( setHysteresis )
                            {
                                commandExecuted = m_Net->SetAllHumidityHysteresis(netIndex, hysteresis ) && commandExecuted;
                            }

                            //m_Net->ReleaseNet( netIndex );
                        }
                        else
                        {
                            commandExecuted = false;
                        }
                    }
                }
            }
        }
        break;
        case COMM_GETHUMIDITYSETTINGS:
        {
            float setPoint= -1.0, hysteresis = -1.0;
            bool autoControlState = false;

            if (toAddress)
            {
                if (m_Net->AcquireNet( netIndex))
                {
                    if (m_Net->GetHumiditySettings( address, &setPoint, &autoControlState, &hysteresis))
                    {
                        messContent = m_XMLParser.CreateMessage( "DEVICE", 6, Device_strings[DEV_HUMIDITY], addressStr.c_str(), "settings", setPoint, autoControlState, hysteresis);

                        UpdateServerPorts(true, false);
                        WriteOnInterfacePorts( messContent.c_str(), messContent.size() );

                        commandExecuted = true;
                    }

                    //m_Net->ReleaseNet( netIndex );
                }
            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    //TODO Aggiungere le funzioni con All
                }
            }
        }
        break;
        case COMM_GETAIAO:
        {
            float analogInput = -1.0;
            bool isCurrent = false;
            int numCtrl = 0;
            int analogIndex = 0;

            //Check if analog device is input or output

            if (toAddress)
            {
                if (m_Net->AcquireNet( netIndex ))
                {
                    if (m_Net->GetAnalogIO(address, &analogInput, &isCurrent))
                    {
                        commandExecuted = true;
                        messContent = m_XMLParser.CreateMessage( "DEVICE", 4, Device_strings[DEV_AIAO], addressStr.c_str(), analogInput, isCurrent);

                        UpdateServerPorts(true, false);
                        WriteOnInterfacePorts( messContent.c_str(), messContent.size() );
                    }

                    //m_Net->ReleaseNet( netIndex );
                }
            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    if (m_Net->NetHasAIAOs( netIndex ))
                    {
                        if (m_Net->AcquireNet( netIndex ))
                        {
                            numCtrl = m_Net->GetNofControllers(netIndex);

                            for (analogIndex = 0; analogIndex < numCtrl; analogIndex++)
                            {
                                if (m_Net->CheckControllerType( netIndex, analogIndex, DEV_AIAO))
                                {
                                    address = m_Net->GetControllerMemoryAddress( netIndex, analogIndex);

                                    if (m_Net->GetAnalogIO(address, &analogInput, &isCurrent))
                                    {
                                        commandExecuted = commandExecuted && true;
                                        messContent = m_XMLParser.CreateMessage( "DEVICE", 4, Device_strings[DEV_AIAO], addressStr.c_str(), analogInput, isCurrent);

                                        UpdateServerPorts(true, false);
                                        WriteOnInterfacePorts( messContent.c_str(), messContent.size() );
                                    }
                                    else
                                    {
                                        commandExecuted = false;
                                    }
                                }
                            }

                            //m_Net->ReleaseNet( netIndex );
                        }
                        else
                        {
                            commandExecuted = false;
                        }
                    }
                }
            }
        };
        break;
        case COMM_SETANALOGOUTPUT:
        {
            int newPosVolt = 0;
            T_Net *net=0x0;

            newPosVolt = m_XMLParser.GetIntParam( "VAL" );

            if (toAddress)
            {
                if (m_Net->AcquireNet( netIndex ))
                {
                    net = m_Net->GetNetHandler(netIndex);
                    
                    commandExecuted = ((CAnalogIO*)(net->CtrlList[devIndex]))->SetVOutput(newPosVolt);

                    //m_Net->ReleaseNet( netIndex );
                }

            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    if (m_Net->NetHasAIAOs( netIndex ))
                    {
                        if (m_Net->AcquireNet( netIndex ))
                        {
                            net = m_Net->GetNetHandler(netIndex);
                            
                            for (unsigned int analIdx = 0; analIdx < net->CtrlList.size(); analIdx++)
                            {
                                if (m_Net->CheckControllerType(netIndex,analIdx,DEV_AIAO) && (!((CAnalogIO*)(net->CtrlList[analIdx]))->IsInput()) )
                                {
                                    commandExecuted = ((CAnalogIO*)(net->CtrlList[analIdx]))->SetVOutput(newPosVolt);
                                }
                            }
                        }
                        else
                        {
                            commandExecuted = false;
                        }
                    }
                }
            }
        };
        break;
        case COMM_GETTEMPALSETTINGS:
        {
            int maxAlarm = (int)TEMP_ERRVAL, minAlarm = (int)TEMP_ERRVAL;
            char maxAlBuffer[8], minAlBuffer[8];
            int numOfAlarms;
            int alarmArray[MAX_NUM_DEV_PER_NET][3];

            if (toAddress)
            {
                if (m_Net->CheckControllerType( netIndex, devIndex, DEV_TEMPCTRL) && (m_Net->GetAlarmT(netIndex, devIndex, &maxAlarm, &minAlarm)) )
                {
                    sprintf (maxAlBuffer, "%d", maxAlarm);
                    sprintf (minAlBuffer, "%d", minAlarm);

                    messContent = m_XMLParser.CreateMessage("DEVICE", 5, Device_strings[DEV_TEMPCTRL], addressStr.c_str(), "AlarmLevels", minAlBuffer, maxAlBuffer);

                    UpdateServerPorts(true, false);
                    WriteOnInterfacePorts( messContent.c_str(), messContent.size() );

                    commandExecuted = true;
                }
                else
                {
                    commandExecuted = false;
                }
            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    if (m_Net->NetHasTempDevices( netIndex ))
                    {
                        numOfAlarms = m_Net->GetAllAlarmT(netIndex, alarmArray, MAX_NUM_DEV_PER_NET);

                        if (numOfAlarms > 0)
                        {
                            commandExecuted = OutputIntData (DEV_TEMPCTRL, netIndex, numOfAlarms, alarmArray) && commandExecuted;
                        }
                    }
                }
            }
        };
        break;
        case COMM_GETHYSTTEMPCTRLSETTINGS:
        {
            T_Net *net=0x0;
            //Settings are auto, setpoint, hysteresis
            CString autoCtrl, setpoint, hyst;
            float tempFloat;
            vector <CVController*>::iterator ctrlIt;

            if (toAddress)
            {
                //Get handler to the NET
                net = m_Net->GetNetHandler( netIndex );

                tempFloat = (float)(((CVHystController*)(net->CtrlList[devIndex]))->GetAutoControl());
                autoCtrl+=tempFloat;

                tempFloat = ((CVHystController*)(net->CtrlList[devIndex]))->GetSetPoint();
                setpoint+=tempFloat;

                tempFloat = ((CVHystController*)(net->CtrlList[devIndex]))->GetHysteresis();
                hyst+=tempFloat;

                messContent = m_XMLParser.CreateMessage( "DEVICE", 5, Device_strings[DEV_TEMPCTRLHYST], addressStr.c_str(), autoCtrl.c_str(), setpoint.c_str(), hyst.c_str());

                UpdateServerPorts(true, false);
                WriteOnInterfacePorts( messContent.c_str(), messContent.size() );

                commandExecuted = true;

            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    if (m_Net->NetHasTempDevices( netIndex ))
                    {
                        net = m_Net->GetNetHandler( netIndex );
                        for (ctrlIt = net->CtrlList.begin(); ctrlIt!=net->CtrlList.end(); ctrlIt++)
                        {
                            if ((*ctrlIt)->GetControllerType() == DEV_TEMPCTRLHYST)
                            {
                                address = (*ctrlIt)->GetMemoryAddress();
                                addressStr+=address;

                                tempFloat = (float)(((CVHystController*)(*ctrlIt))->GetAutoControl());
                                autoCtrl+=tempFloat;

                                tempFloat = ((CVHystController*)(*ctrlIt))->GetSetPoint();
                                setpoint+=tempFloat;

                                tempFloat = ((CVHystController*)(*ctrlIt))->GetHysteresis();
                                hyst+=tempFloat;

                                messContent = m_XMLParser.CreateMessage( "DEVICE", 5, Device_strings[DEV_TEMPCTRLHYST], addressStr.c_str(), autoCtrl.c_str(), setpoint.c_str(), hyst.c_str());

                                UpdateServerPorts(true, false);
                                WriteOnInterfacePorts( messContent.c_str(), messContent.size() );

                                commandExecuted = true;
                            }
                        }
                    }
                }
            }
        };
        break;
        case COMM_SETHYSTTEMPCTRLSETTINGS:
        {
            T_Net *net=0x0;
            //Settings are auto, setpoint, hysteresis
            float setpoint, hyst;
            int autoCtrl;
            vector <CVController*>::iterator ctrlIt;
            CString tempString;
            int configNetIndex, configDevIndex;

            autoCtrl = m_XMLParser.GetIntParam( "AUTO");
            setpoint = m_XMLParser.GetFloatParam( "SETPOINT" );
            hyst = m_XMLParser.GetFloatParam( "HYSTERESIS");

            if (toAddress)
            {
                //Get handler to the NET
                net = m_Net->GetNetHandler( netIndex );
                configNetIndex = net->CtrlList[devIndex]->GetConfigFileNetIndex();
                configDevIndex = net->CtrlList[devIndex]->GetConfigFileDevIndex();

                if (autoCtrl > MINVAL)
                {
                    ((CVHystController*)(net->CtrlList[devIndex]))->SetAutoControl(autoCtrl);
                    commandExecuted = true;
                    tempString+=autoCtrl;
                    m_Net->UpdateIniFile( netIndex, configDevIndex, "AUTO", tempString );
                    tempString.erase();
                }

                if (setpoint > MINVAL)
                {
                    ((CVHystController*)(net->CtrlList[devIndex]))->SetSetPoint(setpoint);
                    commandExecuted = true;
                    tempString+=setpoint;
                    m_Net->UpdateIniFile( netIndex, configDevIndex, "SETPOINT", tempString );
                    tempString.erase();
                }

                if (hyst > MINVAL)
                {
                    ((CVHystController*)(net->CtrlList[devIndex]))->SetHysteresis(hyst);
                    commandExecuted = true;
                    tempString+=hyst;
                    m_Net->UpdateIniFile( netIndex, configDevIndex, "HYST", tempString );
                    tempString.erase();
                }
            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = false;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    if (m_Net->NetHasTempDevices( netIndex ))
                    {
                        net = m_Net->GetNetHandler( netIndex );

                        for (ctrlIt = net->CtrlList.begin(); ctrlIt!=net->CtrlList.end(); ctrlIt++)
                        {

                            if ((*ctrlIt)->GetControllerType() == DEV_TEMPCTRLHYST)
                            {
                                configNetIndex = (*ctrlIt)->GetConfigFileNetIndex();
                                configDevIndex = (*ctrlIt)->GetConfigFileDevIndex();

                                if (autoCtrl > MINVAL)
                                {
                                    ((CVHystController*)(*ctrlIt))->SetAutoControl(autoCtrl);
                                    commandExecuted = true;
                                    tempString+=autoCtrl;
                                    m_Net->UpdateIniFile( netIndex, configDevIndex, "AUTO", tempString );
                                    tempString.erase();
                                }

                                if (setpoint > MINVAL)
                                {
                                    ((CVHystController*)(*ctrlIt))->SetSetPoint(setpoint);
                                    commandExecuted = true;
                                    tempString+=setpoint;
                                    m_Net->UpdateIniFile( netIndex, configDevIndex, "SETPOINT", tempString );
                                    tempString.erase();
                                }

                                if (hyst > MINVAL)
                                {
                                    ((CVHystController*)(*ctrlIt))->SetHysteresis(hyst);
                                    commandExecuted = true;
                                    tempString+=hyst;
                                    m_Net->UpdateIniFile( netIndex, configDevIndex, "HYST", tempString );
                                    tempString.erase();
                                }
                            }
                        }
                    }
                }
            }
        };
        break;
        case COMM_ENABLETIMERBYID:
        {
            int timerID = -1;
            bool enableTimer = false;

            timerID = m_XMLParser.GetIntParam( "TIMERID" );
            enableTimer = m_XMLParser.GetBoolParam( "STATE" );

            if (timerID > 0)
            {
                m_Timer.EnableTimer( timerID, enableTimer);

                commandExecuted = true;
            }

        };
        break;
        case COMM_GETDATEANDTIME:
        {
            struct tm timeStruct;
            time_t actTime;
            int month;
            int day;
            int hour;
            int minute;
            int year;
            string version = AutoVersion::FULLVERSION_STRING;
            char message[128];

            //Usare localtime, gettimeofday, clock_gettime
            //Get time
            time(&actTime);
            localtime_r( &actTime, &timeStruct );

            month = timeStruct.tm_mon+1;
            day = timeStruct.tm_mday;
            hour = timeStruct.tm_hour;

//#ifndef CRIS
//            //Messoper compensare la differenza con il fuso orario Greenwich e CEST
//            hour +=1;
//#endif
//            if (timeStruct.tm_isdst > 0){
//                hour+=1;
//            }

            minute = timeStruct.tm_min;
            year = timeStruct.tm_year+1900;
            
            //messContent = m_XMLParser.CreateMessage( "STATUS", 6, "DATE", timeStruct.tm_mon+1, timeStruct.tm_mday,hour,timeStruct.tm_min,timeStruct.tm_year+1900,"VERSION",version);

            //15-12-2009 Modificato il messaggio in modo che così posso aggiungere la versione del programma
            commandExecuted = true;

            sprintf(message,"%02d%02d%02d%02d%d",month,day,hour,minute,year);
            
            Cmd com("STATUS");
            com.putValue("TYPE","DateAndTime");
            com.putValue("DATE",message);
            com.putValue("VERSION",version);


            UpdateServerPorts(true, false);
            commandExecuted = WriteOnInterfacePorts(com.toString().c_str(), (int)com.toString().size());

        };
        break;
        case COMM_SETDATEANDTIME:
        {
            CString execString;
            CString dateString;

            dateString = m_XMLParser.GetStringParam( "DATE");
            if (dateString.size() > 0)
            {
                //Set date and time
                execString = "date ";
                execString+=dateString;
                system (execString.c_str());

                //Store information on the RTC
                execString = "hwclock -w";
                system(execString.c_str());

                commandExecuted = true;
            }

            //Riazzero le variabili temporali in tutti i dispositivi
            for (int i = 0; i < m_TotalNets; i++)
            {
                T_Net* net = m_Net->GetNetHandler(i);

                net->ResetTimers();

                for (unsigned int j = 0; j < net->CtrlList.size(); j++)
                {
                    e_DeviceType type = net->CtrlList[j]->GetControllerType();
                    if ( (type == DEV_AFOVLV) ||
                         (type == DEV_AFOVLV_VAV) ||
                         (type == DEV_TIMEMARKER) ||
                         (type == DEV_ACC) ||
                         (type == DEV_MGC) ||
                         (type == DEV_CNT) )
                    {
                        ((CUPID2*)(net->CtrlList[j]))->ResetTimers();
                    }
                }
            }
        };
        break;
        case COMM_GETTIMERSETTINGS:
        {
            int timerID;
            CString dayOfWeek;

            //Get if it is required for one timer or for all
            timerID = m_XMLParser.GetIntParam( "TIMERID" );
            dayOfWeek = m_XMLParser.GetStringParam( "DAY" );

            if (timerID == INT_MIN)
            {
                //TODO da implementare
            }
            else
            {
                //Get info on all timers
                int nOfTimers = m_Timer.GetNofTimers();
                int timerIndex = 0;
                bool isTimerEnabled;
                CIniFileHandler timerFileHndlr;
                CString outMessage;
                CString timerString;
                CString tempString;

                timerFileHndlr.Load( "timers.ini" );
                commandExecuted = true;

                for (timerIndex = 0; timerIndex < nOfTimers; timerIndex++)
                {
                    timerString = "TIMER";
                    timerString+=timerIndex+1;
                    outMessage = "<DEVICE TYPE=\"TimerSettings\" TIMERID=\"";
                    outMessage+=timerIndex+1;
                    outMessage+="\" ";

                    outMessage+="SUN=\"";
                    tempString = timerFileHndlr.GetString( "Sun", timerString);
                    outMessage+= tempString;
                    outMessage+="\" ";

                    outMessage+="MON=\"";
                    outMessage+=timerFileHndlr.GetString( "Mon", timerString);
                    outMessage+="\" ";

                    outMessage+="TUE=\"";
                    outMessage+=timerFileHndlr.GetString( "Tue", timerString);
                    outMessage+="\" ";

                    outMessage+="WED=\"";
                    outMessage+=timerFileHndlr.GetString( "Wed", timerString);
                    outMessage+="\" ";

                    outMessage+="THU=\"";
                    outMessage+=timerFileHndlr.GetString( "Thu", timerString);
                    outMessage+="\" ";

                    outMessage+="FRI=\"";
                    outMessage+=timerFileHndlr.GetString( "Fri", timerString);
                    outMessage+="\" ";

                    outMessage+="SAT=\"";
                    outMessage+=timerFileHndlr.GetString( "Sat", timerString);
                    outMessage+="\" ";

                    outMessage+="LEVEL1=\"";
                    outMessage+=timerFileHndlr.GetString( "Level1", timerString);
                    outMessage+="\" ";

                    outMessage+="LEVEL2=\"";
                    outMessage+=timerFileHndlr.GetString( "Level2", timerString);
                    outMessage+="\" ";

                    outMessage+="LEVEL3=\"";
                    outMessage+=timerFileHndlr.GetString( "Level3", timerString);
                    outMessage+="\" ";

                    isTimerEnabled = m_Timer.IsTimerEnabled( timerIndex + 1 );
                    outMessage+="ENABLED=\"";
                    outMessage+=isTimerEnabled;
                    outMessage+="\" ";

                    outMessage+="/>";

                    commandExecuted = WriteOnInterfacePorts( outMessage.c_str(), outMessage.size() ) && commandExecuted;
                }
            }

        };
        break;
      case COMM_CHANGEDOSTATE:
      {
        T_Net *net=0x0;
        int configDevIndex;
        bool isDIDO = false;
        bool isButton = false;
        bool isStateOn = false;
        vector<CVController*>::iterator ctrlIt;

        isDIDO = m_Net->CheckControllerType( netIndex, devIndex, DEV_DIDO);
        isButton = m_Net->CheckControllerType( netIndex, devIndex, DEV_BUTTONCTRL) || m_Net->CheckControllerType( netIndex, devIndex, DEV_STEPDIGITALOUT);

        if (toAddress)
        {
                //Get handler to the NET
          net = m_Net->GetNetHandler( netIndex );

          if ( m_Net->AcquireNet( netIndex ) && (isButton || isDIDO) )
          {
            if (isDIDO)
            {
              commandExecuted = ((CVDIDO*)(net->CtrlList[devIndex]))->ChangeOutput();
            }
            else
            {
              commandExecuted = ((CVMultiDIDO*)(net->CtrlList[devIndex]))->ChangeOutput();
            }

                    //Update the INI file
            if (commandExecuted && m_EnableLog)
            {
                        //TODO qui c'e' un marone per il salvataggio deigli stati degli stepdigitaslout
              configDevIndex = net->CtrlList[devIndex]->GetConfigFileDevIndex();

              if (isDIDO)
              {
                isStateOn = ((CDigitalIO*)(net->CtrlList[devIndex]))->GetState();
              }
              else
              {
                isStateOn = ((CVMultiDIDO*)(net->CtrlList[devIndex]))->GetInputState(true);
              }

              if (net->saveDigitalState)
              {
                if (isStateOn)
                {
                  m_Net->UpdateIniFile(netIndex, configDevIndex, "STARTV", "1");
                }
                else
                {
                  m_Net->UpdateIniFile(netIndex, configDevIndex, "STARTV", "0");
                }
              }
            }
          }

        }
        else if ((toEveryOne) || toNet)
        {
          commandExecuted = true;

          for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
          {
            if ( m_Net->AcquireNet( netIndex ) )
            {
              net = m_Net->GetNetHandler( netIndex );
              for (ctrlIt = net->CtrlList.begin(); ctrlIt!=net->CtrlList.end(); ctrlIt++)
              {
                isDIDO = (*ctrlIt)->GetControllerType() == DEV_DIDO;
                isButton = ( ((*ctrlIt)->GetControllerType() == DEV_STEPDIGITALOUT) || ((*ctrlIt)->GetControllerType() == DEV_BUTTONCTRL));

                if (isDIDO)
                {
                    if (!((CDigitalIO*)((*ctrlIt)))->IsInput())
                    {
                        commandExecuted = ((CVDIDO*)((*ctrlIt)))->ChangeOutput() && commandExecuted;
                    }
                }
                else if(isButton)
                {
                  commandExecuted = ((CVMultiDIDO*)((*ctrlIt)))->ChangeOutput() && commandExecuted;
                }
              }
            }
          }
          return commandExecuted;
        }
      }; 
      break;
        case COMM_GETPUMPCTRLSETTINGS:
        {
            T_Net *net=0x0;
            int pump1State = 0, pump2State = 0, swapInterval = 0;
            CString outMessage, pump1StateStr, pump2StateStr;
            vector<CVController*>::iterator ctrlIt;

            if (toAddress)
            {
                //Get handler to the NET
                net = m_Net->GetNetHandler( netIndex );

                ((CPumpController*)(net->CtrlList[devIndex]))->GetSettings(&pump1State, &pump2State, &swapInterval);

                switch (pump1State)
                {
                    case 0: pump1StateStr = "Off";break;
                    case 1: pump1StateStr = "On";break;
                    case 2: pump1StateStr = "Alarm";break;
                    default:pump1StateStr="NA";
                }

                switch (pump2State)
                {
                    case 0: pump2StateStr = "Off";break;
                    case 1: pump2StateStr = "On";break;
                    case 2: pump2StateStr = "Alarm";break;
                    default:pump2StateStr="NA";
                }

                outMessage = "<DEVICE TYPE=\"PumpControllerSettings\" ADDRESS=\"";
                outMessage+=addressStr;
                outMessage+="\" PUMP1STATE=\"";
                outMessage+=pump1StateStr;
                outMessage+="\" PUMP2STATE=\"";
                outMessage+=pump2StateStr;
                outMessage+="\" />";

                commandExecuted = WriteOnInterfacePorts( outMessage.c_str(), outMessage.size() );

            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    net = m_Net->GetNetHandler( netIndex );

                    for (ctrlIt = net->CtrlList.begin(); ctrlIt!=net->CtrlList.end(); ctrlIt++)
                    {

                        if ((*ctrlIt)->GetControllerType() == DEV_PUMPCTRL)
                        {
                            address = (*ctrlIt)->GetMemoryAddress();

                            ((CPumpController*)(*ctrlIt))->GetSettings(&pump1State, &pump2State, &swapInterval);

                            switch (pump1State)
                            {
                                case 0: pump1StateStr = "Off";break;
                                case 1: pump1StateStr = "On";break;
                                case 2: pump1StateStr = "Alarm";break;
                                default:pump1StateStr="NA";
                            }

                            switch (pump2State)
                            {
                                case 0: pump2StateStr = "Off";break;
                                case 1: pump2StateStr = "On";break;
                                case 2: pump2StateStr = "Alarm";break;
                                default:pump2StateStr = "NA";
                            }

                            outMessage = "<DEVICE TYPE=\"PumpControllerSettings\" ADDRESS=\"";
                            outMessage+=address;
                            outMessage+="\" PUMP1STATE=\"";
                            outMessage+=pump1StateStr;
                            outMessage+="\" PUMP2STATE=\"";
                            outMessage+=pump2StateStr;
                            outMessage+="\" />";

                            commandExecuted = WriteOnInterfacePorts( outMessage.c_str(), outMessage.size() ) && commandExecuted;


                        }
                    }
                }//FOR netIndex
            }
        };
        break;
        case COMM_SETPUMPCONTROLLERSWAPTIME:
        {
            T_Net *net=0x0;
            time_t swapTime = 0;
            int configDevIndex = -1;
            CString swapTimeStr;
            vector<CVController*>::iterator ctrlIt;

            swapTime = m_XMLParser.GetIntParam( "SWAPTIME" );

            swapTimeStr+=swapTime;

            if (swapTime < 0)
            {
                break;
            }

            if (toAddress)
            {
                //Get handler to the NET
                net = m_Net->GetNetHandler( netIndex );

                ((CPumpController*)(net->CtrlList[devIndex]))->SetSwapTime(swapTime);
                configDevIndex = net->CtrlList[devIndex]->GetConfigFileDevIndex();

                commandExecuted = m_Net->UpdateIniFile( netIndex, configDevIndex, "SWAPTIME", swapTimeStr);

            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    net = m_Net->GetNetHandler( netIndex );

                    for (ctrlIt = net->CtrlList.begin(); ctrlIt!=net->CtrlList.end(); ctrlIt++)
                    {
                        if ((*ctrlIt)->GetControllerType() == DEV_PUMPCTRL)
                        {
                            configDevIndex = (*ctrlIt)->GetConfigFileDevIndex();

                            ((CPumpController*)(net->CtrlList[devIndex]))->SetSwapTime(swapTime);

                            commandExecuted = m_Net->UpdateIniFile( netIndex, configDevIndex, "SWAPTIME", swapTimeStr) && commandExecuted;
                        }
                    }
                }//For netIndex
            }
        };
        break; //-----------------COMM_SETPUMPCONTROLLERSWAPTIME------------//
        case COMM_GETVOWCTRLSETTINGS :
        {
            uchar vowConfiguration[32];
            T_Net *net;
            CString outMessage;
            bool isLMD = false;
            vector<CVController*>::iterator ctrlIt;

            if (toAddress)
            {
                //Get handler to the NET
                net = this->m_Net->GetNetHandler( netIndex );

                if (this->m_Net->AcquireNet( netIndex ))
                {
                    if (this->m_Net->CheckControllerType(netIndex, devIndex, DEV_VLVCTRL))
                    {
#ifndef USE_ADV_VLV
                        ((CNTHVLV*)(net->CtrlList[devIndex]))->GetStateVector(vowConfiguration);
#else
                        ((CNTHVLV_ADV*)(net->CtrlList[devIndex]))->GetStateVector(vowConfiguration);
#endif

                        outMessage="<DEVICE TYPE=\"VOWCtrlSettings\" ADDRESS=\"";
                        outMessage+=addressStr;

                        outMessage+="\" SETPOINT=\"";
                        outMessage+=(float)(vowConfiguration[9])* 20.0/255.0 + 10.0; //Re-transform it from 0-255 to 10C - 30C;

                        outMessage+="\" KAPPA1=\"";
                        outMessage+=(float)(vowConfiguration[3]);

                        outMessage+="\" TINT1=\"";
                        outMessage+=(float)(vowConfiguration[4]);

                        outMessage+="\" TDER1=\"";
                        outMessage+=(float)(vowConfiguration[5]);

                        isLMD = vowConfiguration[13] & 0x0C;
                        if (isLMD)
                        {
                            outMessage+="\" KAPPA2=\"";
                            outMessage+=(float)(vowConfiguration[6]);

                            outMessage+="\" TINT2=\"";
                            outMessage+=(float)(vowConfiguration[7]);

                            outMessage+="\" TDER2=\"";
                            outMessage+=(float)(vowConfiguration[8]);

                            outMessage+="\" SETPOINTH=\"";
                            outMessage+=(float)(vowConfiguration[14]);

                            outMessage+="\" SETPOINTL=\"";
                            outMessage+=(float)(vowConfiguration[15]);
                        }

                        outMessage+="\" TEMPINT=\"";

                        //Check sign of temps
                        if (vowConfiguration[2] & 0x0F)
                        {
                            outMessage+="-";
                        }

                        outMessage+=(float)(vowConfiguration[0]);

                        outMessage+="\" TEMPEXT=\"";

                        //Check sign of temps
                        if (vowConfiguration[2] & 0xF0)
                        {
                            outMessage+="-";
                        }

                        outMessage+=(float)(vowConfiguration[1]);

                        outMessage+="\" />";

                        commandExecuted = WriteOnInterfacePorts( outMessage.c_str(), outMessage.size() );

                        //m_Net->ReleaseNet( netIndex );
                    }
                }
            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    net = m_Net->GetNetHandler( netIndex );

                    if (net->hasAdvCtrls && m_Net->AcquireNet( netIndex ))
                    {
                        for (ctrlIt = net->CtrlList.begin(); ctrlIt!=net->CtrlList.end(); ctrlIt++)
                        {
                            if ((*ctrlIt)->GetControllerType() == DEV_VLVCTRL)
                            {
#ifndef USE_ADV_VLV
                                ((CNTHVLV*)(*ctrlIt))->GetStateVector(vowConfiguration);
#else
                                ((CNTHVLV_ADV*)(*ctrlIt))->GetStateVector(vowConfiguration);
#endif

                                outMessage="<DEVICE TYPE=\"VOWCtrlSettings\" ADDRESS=\"";
                                addressStr="";
                                addressStr += (*ctrlIt)->GetMemoryAddress();
                                outMessage+=addressStr;

                                outMessage+="\" SETPOINT=\"";
                                outMessage+=(float)(vowConfiguration[9]*0.5);

                                outMessage+="\" KAPPA1=\"";
                                outMessage+=(float)(vowConfiguration[3]);

                                outMessage+="\" TINT1=\"";
                                outMessage+=(float)(vowConfiguration[4]);

                                outMessage+="\" TDER1=\"";
                                outMessage+=(float)(vowConfiguration[5]);

                                isLMD = vowConfiguration[13] & 0x0C;
                                if (isLMD)
                                {
                                    outMessage+="\" KAPPA2=\"";
                                    outMessage+=(float)(vowConfiguration[6]);

                                    outMessage+="\" TINT2=\"";
                                    outMessage+=(float)(vowConfiguration[7]);

                                    outMessage+="\" TDER2=\"";
                                    outMessage+=(float)(vowConfiguration[8]);

                                    outMessage+="\" SETPOINTH=\"";
                                    outMessage+=(float)(vowConfiguration[14]);

                                    outMessage+="\" SETPOINTL=\"";
                                    outMessage+=(float)(vowConfiguration[15]);
                                }

                                outMessage+="\" TEMPINT=\"";

                                //Check sign of temps
                                if (vowConfiguration[2] & 0x0F)
                                {
                                    outMessage+="-";
                                }

                                outMessage+=(float)(vowConfiguration[0])*0.5;

                                outMessage+="\" TEMPEXT=\"";

                                //Check sign of temps
                                if (vowConfiguration[2] & 0xF0)
                                {
                                    outMessage+="-";
                                }

                                outMessage+=(float)(vowConfiguration[1])*0.5;

                                outMessage+="\" />";

                                commandExecuted = WriteOnInterfacePorts( outMessage.c_str(), outMessage.size() );
                            }
                        }
                    }

                    //m_Net->ReleaseNet( netIndex );
                }//For netIndex
            }
        };break;//-----------------COMM_GETVOWCTRLSETTINGS------------//
        case COMM_SETVOWCTRLSETTINGS:
        {
            uchar vowConfiguration[17];
            int vowNewConfiguration[17];
            int i;
            T_Net *net;
            vector<CVController*>::iterator ctrlIt;
            int changeSummerStatus = 0;
            CString tempString;
            int activateOutputCommand = -1;
            unsigned int newOutputState = 0;


            activateOutputCommand = m_XMLParser.GetIntParam("ACTIVATEDIGITAL");
            newOutputState = m_XMLParser.GetIntParam("DIGOUT");

            //Extract values from the device
            //FLAG_START : not used
            vowNewConfiguration[0] = 0;

            vowNewConfiguration[1] = vowNewConfiguration[2] = -1;

            vowNewConfiguration[3] = m_XMLParser.GetIntParam( "KAPPA1" );
            vowNewConfiguration[4] = m_XMLParser.GetIntParam( "TINT1" );
            vowNewConfiguration[5] = m_XMLParser.GetIntParam( "TDER1" );

            vowNewConfiguration[6] = m_XMLParser.GetIntParam( "KAPPA2" );
            vowNewConfiguration[7] = m_XMLParser.GetIntParam( "TINT2" );
            vowNewConfiguration[8] = m_XMLParser.GetIntParam( "TDER2" );

            vowNewConfiguration[9] =  m_XMLParser.GetIntParam( "SETPOINT" );

            if (vowNewConfiguration[9] >= 0)
            {
                //Check setpoint value
                if (vowNewConfiguration[9] < 10)
                {
                    vowNewConfiguration[9] = 10;
                }

                if (vowNewConfiguration[9] > 30)
                {
                    vowNewConfiguration[9] = 30;
                }

                //Transform it in a 0 - 255 range
                vowNewConfiguration[9] = 255*(vowNewConfiguration[9] - 10)/20;
            }

            vowNewConfiguration[10] = 0xFF;

            //DAC A and B
            vowNewConfiguration[11] = vowNewConfiguration[12] = -1;

            //FLAG state
            vowNewConfiguration[13] = -1;

            //SetpointH and L
            vowNewConfiguration[14] = m_XMLParser.GetIntParam( "SETPOINTH");
            vowNewConfiguration[15] = m_XMLParser.GetIntParam( "SETPOINTL");

            //Get if the summer parameter has been specified
            changeSummerStatus = m_XMLParser.GetIntParam( "SUMMER" );

            if (toAddress)
            {
                //Get handler to the NET
                net = m_Net->GetNetHandler( netIndex );

                if (m_Net->AcquireNet( netIndex ))
                {
                    if (m_Net->CheckControllerType(netIndex, devIndex, DEV_VLVCTRL))
                    {
                        //Get the statevector of the device
    #ifndef USE_ADV_VLV
                        ((CNTHVLV*)(net->CtrlList[devIndex]))->GetStateVector(vowConfiguration);
    #else
                        ((CNTHVLV_ADV*)(net->CtrlList[devIndex]))->GetStateVector(vowConfiguration);
    #endif
    
                        //Load the new configuration into the old one
                        for (i = 0; i < 13; i++)
                        {
                            if (vowNewConfiguration[i]>=0)
                            {
                                vowConfiguration[i] = vowNewConfiguration[i];
                            }
                        }
    
                        //Setup the summer/winter switch
                        if (changeSummerStatus >= 0)
                        {
                            if (changeSummerStatus == 0)
                            {
                                //go in winter mode
                                vowConfiguration[13] = vowConfiguration[13] | 0x02 ;
                            }
                            else if (changeSummerStatus > 0)
                            {
                                //Go in summer mode
                                vowConfiguration[13] = vowConfiguration[13] & 0xFD ;
                            }
                        }
    
                        if (vowNewConfiguration[14] >= 0)
                        {
                        vowConfiguration[14] = (uchar)(vowNewConfiguration[14]);
                        }
    
                        if (vowNewConfiguration[15] >= 0)
                        {
                        vowConfiguration[15] = (uchar)(vowNewConfiguration[15]);
                        }
    
                        //Write on the device
                        bool operationOk=false;
    #ifndef USE_ADV_VLV
                        operationOk = ((CNTHVLV*)(net->CtrlList[devIndex]))->WriteStateVector(vowConfiguration);
    #else
                        operationOk = ((CNTHVLV_ADV*)(net->CtrlList[devIndex]))->WriteStateVector(vowConfiguration);
    #endif
                        if (operationOk)
                        {
                            commandExecuted = true;
    
                            //Save in the INI file the new parameters
                            devIndex = net->CtrlList[devIndex]->GetConfigFileDevIndex();
    
                            tempString = "";
                            tempString+= vowConfiguration[3];
                            m_Net->UpdateIniFile( netIndex, devIndex, "KAPPA1", tempString);
                            tempString = "";
                            tempString+= vowConfiguration[4];
                            m_Net->UpdateIniFile( netIndex, devIndex, "TINT1", tempString);
                            tempString = "";
                            tempString+= vowConfiguration[5];
                            m_Net->UpdateIniFile( netIndex, devIndex, "TDER1", tempString);
    
                            tempString = "";
                            tempString+= vowConfiguration[6];
                            m_Net->UpdateIniFile( netIndex, devIndex, "KAPPA2", tempString);
                            tempString = "";
                            tempString+= vowConfiguration[7];
                            m_Net->UpdateIniFile( netIndex, devIndex, "TINT2", tempString);
                            tempString = "";
                            tempString+= vowConfiguration[8];
                            m_Net->UpdateIniFile( netIndex, devIndex, "TDER2", tempString);
    
                            tempString = "";
                            tempString+= (int)round((vowConfiguration[9]* 20.0/255.0 + 10.0));
                            m_Net->UpdateIniFile( netIndex, devIndex, "SETPOINT", tempString);
    
                            if (changeSummerStatus >= 0)
                            {
                                if (changeSummerStatus == 0)
                                {
                                    //Save Winter mode
                                    tempString = "0";
                                    m_Net->UpdateIniFile( netIndex, devIndex, "SUMMER", tempString);
                                }
                                else if (changeSummerStatus > 0)
                                {
                                    //Save summer mode
                                    tempString = "1";
                                    m_Net->UpdateIniFile( netIndex, devIndex, "SUMMER", tempString);
                                }
                            }
    
                            tempString = "";
                            tempString+= vowConfiguration[14];
                            m_Net->UpdateIniFile( netIndex, devIndex, "SETPOINTH", tempString);
    
                            tempString = "";
                            tempString+= vowConfiguration[15];
                            m_Net->UpdateIniFile( netIndex, devIndex, "SETPOINTL", tempString);
    
                        }
    
                        if (activateOutputCommand > 0)
                        {
    #ifndef USE_ADV_VLV
                            ((CNTHVLV*)(net->CtrlList[devIndex]))->ChangeDOOutput((bool)newOutputState);
    #else
                            ((CNTHVLV_ADV*)(net->CtrlList[devIndex]))->ChangeDOOutput((bool)newOutputState);
    #endif
                        }
                    }

                    //m_Net->ReleaseNet( netIndex );
                }//IF AcquireNet
            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    net = m_Net->GetNetHandler( netIndex );

                    if (net->hasAdvCtrls && m_Net->AcquireNet( netIndex ))
                    {
                        for (ctrlIt = net->CtrlList.begin(); ctrlIt!=net->CtrlList.end(); ctrlIt++)
                        {
                            if ((*ctrlIt)->GetControllerType() == DEV_VLVCTRL)
                            {
#ifndef USE_ADV_VLV
                                ((CNTHVLV*)(*ctrlIt))->GetStateVector(vowConfiguration);
#else
                                ((CNTHVLV_ADV*)(*ctrlIt))->GetStateVector(vowConfiguration);
#endif
                                //Load the new configuration into the old one
                                for (i = 0; i < 17; i++)
                                {
                                    if (vowNewConfiguration[i]>=0)
                                    {
                                        vowConfiguration[i] = vowNewConfiguration[i];
                                    }
                                }

                                //Setup the summer/winter switch
                                if (changeSummerStatus >= 0)
                                {
                                    if (changeSummerStatus == 0)
                                    {
                                        //go in winter mode
                                        vowConfiguration[13] = vowConfiguration[13] & 0xFD;
                                    }
                                    else if (changeSummerStatus > 0)
                                    {
                                        //Go in summer mode
                                        vowConfiguration[13] = vowConfiguration[13] | 0x02;
                                    }
                                }

                                bool operationOk = false;
#ifndef USE_ADV_VLV
                                operationOk = ((CNTHVLV*)(*ctrlIt))->WriteStateVector(vowConfiguration);
#else
                                operationOk = ((CNTHVLV_ADV*)(*ctrlIt))->WriteStateVector(vowConfiguration);
#endif
                                if (operationOk)
                                {
                                    commandExecuted = true && commandExecuted;

                                     //Save in the INI file the new parameters
                                    devIndex = (*ctrlIt)->GetConfigFileDevIndex();

                                    tempString = "";
                                    tempString+= vowConfiguration[3];
                                    m_Net->UpdateIniFile( netIndex, devIndex, "KAPPA1", tempString);
                                    tempString = "";
                                    tempString+= vowConfiguration[4];
                                    m_Net->UpdateIniFile( netIndex, devIndex, "TINT1", tempString);
                                    tempString = "";
                                    tempString+= vowConfiguration[5];
                                    m_Net->UpdateIniFile( netIndex, devIndex, "TDER1", tempString);

                                    tempString = "";
                                    tempString+= vowConfiguration[6];
                                    m_Net->UpdateIniFile( netIndex, devIndex, "KAPPA2", tempString);
                                    tempString = "";
                                    tempString+= vowConfiguration[7];
                                    m_Net->UpdateIniFile( netIndex, devIndex, "TINT2", tempString);
                                    tempString = "";
                                    tempString+= vowConfiguration[8];
                                    m_Net->UpdateIniFile( netIndex, devIndex, "TDER2", tempString);

                                    tempString = "";
                                    tempString+= (int)(vowConfiguration[9]* 20.0/255.0 + 10.0);
                                    m_Net->UpdateIniFile( netIndex, devIndex, "SETPOINT", tempString);

                                    if (changeSummerStatus >= 0)
                                    {
                                        if (changeSummerStatus == 0)
                                        {
                                             //Save Winter mode
                                            tempString = "0";
                                            m_Net->UpdateIniFile( netIndex, devIndex, "SUMMER", tempString);
                                        }
                                        else if (changeSummerStatus > 0)
                                        {
                                             //Save summer mode
                                            tempString = "1";
                                            m_Net->UpdateIniFile( netIndex, devIndex, "SUMMER", tempString);
                                        }
                                    }

                                    tempString = "";
                                    tempString+= vowConfiguration[14];
                                    m_Net->UpdateIniFile( netIndex, devIndex, "SETPOINTH", tempString);

                                    tempString = "";
                                    tempString+= vowConfiguration[15];
                                    m_Net->UpdateIniFile( netIndex, devIndex, "SETPOINTL", tempString);
                                }
                            }
                        }
                    }

                    //m_Net->ReleaseNet( netIndex );
                }//For netIndex
            }
        };break; //------------------ SETVOWCTRLSETTINGS -------------------- //
        case COMM_ENABLETEMPALARMS:
        {
            bool enableAlarms = false;
            T_Net *net;
            vector<CVController*>::iterator ctrlIt;

            enableAlarms = m_XMLParser.GetIntParam("ENABLE");

            if (toAddress)
            {
                if (m_Net->CheckControllerType( netIndex, devIndex, DEV_TEMPCTRL) )
                {
                    //Get handler to the NET
                    net = m_Net->GetNetHandler( netIndex );

                    if (m_Net->AcquireNet( netIndex ))
                    {
                        ((CTempCtrl*)(net->CtrlList[devIndex]))->EnablePhoneAlarm(enableAlarms);

                        commandExecuted = true;
                    }
                }
                else
                {
                    commandExecuted = false;
                }
            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    net = m_Net->GetNetHandler( netIndex );

                    if ( (m_Net->NetHasTempDevices( netIndex )) && (m_Net->AcquireNet( netIndex )) )
                    {
                        for (ctrlIt = net->CtrlList.begin(); ctrlIt!=net->CtrlList.end(); ctrlIt++)
                        {
                            if ((*ctrlIt)->GetControllerType() == DEV_TEMPCTRL)
                            {

                                ((CTempCtrl*)(*ctrlIt))->EnablePhoneAlarm( enableAlarms );

                                commandExecuted = true && commandExecuted;
                            }
                        }
                    }
                }
            }
        };
        break;
        case COMM_GETTEMPALARMSTATE:
        {
            int alarmsEnabled = 0;
            T_Net *net;
            vector<CVController*>::iterator ctrlIt;
            CString outMessage, addressStr;

            if (toAddress)
            {
                if (m_Net->CheckControllerType( netIndex, devIndex, DEV_TEMPCTRL) )
                {
                    //Get handler to the NET
                    net = m_Net->GetNetHandler( netIndex );

                    if (m_Net->AcquireNet( netIndex ))
                    {
                        outMessage = "<DEVICE TYPE=\"TempController\" STATE=\"";
                        alarmsEnabled = ((CTempCtrl*)(net->CtrlList[devIndex]))->m_AlarmEnabled;
                        outMessage+=alarmsEnabled;
                        outMessage+="\" ADDRESS=\"";
                        addressStr="";
                        addressStr += (*ctrlIt)->GetMemoryAddress();
                        outMessage+=addressStr;
                        outMessage+="\" />";

                        commandExecuted = WriteOnInterfacePorts( outMessage.c_str(), outMessage.size() );
                    }
                }
                else
                {
                    commandExecuted = false;
                }
            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    net = m_Net->GetNetHandler( netIndex );

                    if (m_Net->NetHasTempDevices( netIndex ))
                    {
                        if (m_Net->AcquireNet( netIndex ))
                        {
                            for (ctrlIt = net->CtrlList.begin(); ctrlIt!=net->CtrlList.end(); ctrlIt++)
                            {
                                if ((*ctrlIt)->GetControllerType() == DEV_TEMPCTRL)
                                {

                                    outMessage = "<DEVICE TYPE=\"TempAlarmState\" STATE=\"";
                                    alarmsEnabled = ((CTempCtrl*)(*ctrlIt))->m_AlarmEnabled;
                                    outMessage+=alarmsEnabled;
                                    outMessage+="\" ADDRESS=\"";
                                    addressStr += (*ctrlIt)->GetMemoryAddress();
                                    outMessage+=addressStr;
                                    outMessage+="\" />";

                                    commandExecuted = true && WriteOnInterfacePorts( outMessage.c_str(), outMessage.size() );
                                }
                            }
                        }
                    }
                }
            }
        };
        break;
        case COMM_GETROOMDATA:{
            T_Net *net;
            CString outMessage;
            vector<CVController*>::iterator ctrlIt;

            if (toAddress)
            {
                if (m_Net->CheckControllerType( netIndex, devIndex, DEV_MGC) )
                {
                    //Get handler to the NET
                    net = m_Net->GetNetHandler( netIndex );

                    if (m_Net->AcquireNet( netIndex ))
                    {
                        outMessage = "<DEVICE TYPE=\"RoomData\" ";
                        outMessage+="ADDRESS=\"";
                        outMessage+=net->CtrlList[devIndex]->GetMemoryAddress();
                        outMessage+="\" NOFKEYS=\"";
                        outMessage+=((CNTHMGC*)(net->CtrlList[devIndex]))->m_GuestArray.size();

                        outMessage+="\" STATUS=\"";
                        outMessage+=((CNTHMGC*)(net->CtrlList[devIndex]))->m_IsRoomOn;

                        outMessage+="\" CHECKIN=\"";

                        if (((CNTHMGC*)(net->CtrlList[devIndex]))->m_IsRoomFree) {
                            struct tm timeStruct;
                            time_t actTime;

                            //Get time
                            time(&actTime);

                            //Convert it to broken down time+
                            gmtime_r(&actTime, &timeStruct);

                            if ( (timeStruct.tm_mon+1) < 10){
                                outMessage+="0";
                            }
                            outMessage+=(timeStruct.tm_mon+1);

                            if (timeStruct.tm_mday < 10){
                                outMessage+="0";
                            }
                            outMessage+=timeStruct.tm_mday;

                            if (timeStruct.tm_hour < 10){
                                outMessage+="0";
                            }
                            outMessage+=timeStruct.tm_hour;

                            if (timeStruct.tm_min < 10){
                                outMessage+="0";
                            }
                            outMessage+=timeStruct.tm_min;

                            outMessage+=(timeStruct.tm_year+1900);

                        }
                        else {
                            unsigned int roomTimeSec = 0;
                            char roomTimeStr[32];

                            //Leggo il checkin dalla camera
                            roomTimeSec = ((CNTHMGC*)(net->CtrlList[devIndex]))->m_CheckInDateSec;

                            //Lo convertop a formato normale
                            ConvertSecs2DateStr(roomTimeSec, roomTimeStr, 32);

                            //lo metto nel checkin
                            outMessage+=roomTimeStr;

                            outMessage+="\" CHECKOUT=\"";

                            roomTimeSec = ((CNTHMGC*)(net->CtrlList[devIndex]))->m_CheckOutDateSec;

                            ConvertSecs2DateStr(roomTimeSec, roomTimeStr, 32);

                            outMessage+=roomTimeStr;
                        }

                        outMessage+="\" AIRCOND=\"";
                        outMessage+=(int)(((CNTHMGC*)(net->CtrlList[devIndex]))->m_IsAirCondEnabled);

                        //Aggiorno i dati dal contabilizzatore
                        //Leggo i contatori assoluti, per ogni contatore l'ordine dei dati è il seguente
                        //Per bedazzo l'ordine è
                        //Contatore 0 -- ContaCalorie,
                        //Contatore 1 -- Acqua calda
                        //Contatore 2 -- Acqua fredda
                        //Contatore 3 -- Watt
                        //Nei float che leggo ho i seguenti valori:
                        //*Valore della grandezza contata (litri o watt)
                        //*tempMnd
                        //*tempRip
                        //*Calorie
                        //*Frigorie
                        float absCounters[25];
                        bool isSummer = ((CNTHMGC*)(net->CtrlList[devIndex]))->m_PIDIsSummer;
                        char floatStr[8];

                        ((CNTHMGC*)(net->CtrlList[devIndex]))->GetCounterReadings(absCounters);

                        if (!((CNTHMGC*)(net->CtrlList[devIndex]))->m_IsRoomFree)
                        {
                            //Mando i contatori assoluti
                            outMessage+="\" HOTWATER=\"";
                            outMessage+=absCounters[5] - ((CNTHMGC*)(net->CtrlList[devIndex]))->m_CheckInCounters[0];//Acqua calda

                            outMessage+="\" COLDWATER=\"";
                            outMessage+=absCounters[10] - ((CNTHMGC*)(net->CtrlList[devIndex]))->m_CheckInCounters[1];//Acqua freddda

                            outMessage+="\" POW=\"";
                            sprintf (floatStr, "%.3f", absCounters[15] - ((CNTHMGC*)(net->CtrlList[devIndex]))->m_CheckInCounters[2]);
                            outMessage+=floatStr; //WATT

                            outMessage+="\" HEAT=\"";
                            if (isSummer)
                            {
                                sprintf (floatStr, "%.3f", absCounters[4] - ((CNTHMGC*)(net->CtrlList[devIndex]))->m_CheckInCounters[3]);
                                outMessage+=floatStr;//Frigorie
                            }
                            else
                            {
                                sprintf (floatStr, "%.3f", absCounters[3] - ((CNTHMGC*)(net->CtrlList[devIndex]))->m_CheckInCounters[4]);
                                outMessage+=floatStr; //Calorie
                            }
                        }
                        else
                        {
                            //Devo mandare i consumi relativi al checkin
                            //Mando i contatori assoluti
                            outMessage+="\" HOTWATER=\"";
                            outMessage+=absCounters[5];

                            outMessage+="\" COLDWATER=\"";
                            outMessage+=absCounters[10];

                            outMessage+="\" POW=\"";
                            sprintf(floatStr, "%.3f", absCounters[15]);
                            outMessage+=floatStr;

                            outMessage+="\" HEAT=\"";
                            if (isSummer)
                            {
                                sprintf(floatStr, "%.3f", absCounters[04]);
                                outMessage+=absCounters[4];
                            }
                            else
                            {
                                sprintf(floatStr, "%.3f", absCounters[3]);
                                outMessage+=absCounters[3];
                            }
                        }

                        outMessage+="\" />";

                        commandExecuted = WriteOnInterfacePorts( outMessage.c_str(), outMessage.size() );
                    }
                }
                else
                {
                    commandExecuted = false;
                }
            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    net = m_Net->GetNetHandler( netIndex );

                    if (m_Net->NetHasTempDevices( netIndex ))
                    {
                        if (m_Net->AcquireNet( netIndex ))
                        {
                            for (ctrlIt = net->CtrlList.begin(); ctrlIt!=net->CtrlList.end(); ctrlIt++)
                            {
                                if ((*ctrlIt)->GetControllerType() == DEV_MGC)
                                {

                                    outMessage = "<DEVICE TYPE=\"RoomData\" ";
                                    outMessage+="ADDRESS=\"";
                                    outMessage+=net->CtrlList[devIndex]->GetMemoryAddress();
                                    outMessage+="\" NOFKEYS=\"";
                                    outMessage+=((CNTHMGC*)(net->CtrlList[devIndex]))->m_GuestArray.size();

                                    outMessage+="\" STATUS=\"";
                                    outMessage+=((CNTHMGC*)(net->CtrlList[devIndex]))->m_IsRoomOn;

                                    outMessage+="\" CHECKIN=\"";

                                    if (((CNTHMGC*)(net->CtrlList[devIndex]))->m_IsRoomFree) {
                                        struct tm timeStruct;
                                        time_t actTime;

                                        //Get time
                                        time(&actTime);

                                        //Convert it to broken down time+
                                        gmtime_r(&actTime, &timeStruct);

                                        if ( (timeStruct.tm_mon+1) < 10){
                                            outMessage+="0";
                                        }
                                        outMessage+=(timeStruct.tm_mon+1);

                                        if (timeStruct.tm_mday < 10){
                                            outMessage+="0";
                                        }
                                        outMessage+=timeStruct.tm_mday;

                                        if (timeStruct.tm_hour < 10){
                                            outMessage+="0";
                                        }
                                        outMessage+=timeStruct.tm_hour;

                                        if (timeStruct.tm_min < 10){
                                            outMessage+="0";
                                        }
                                        outMessage+=timeStruct.tm_min;

                                        outMessage+=(timeStruct.tm_year+1900);

                                    }
                                    else {
                                        unsigned int roomTimeSec = 0;
                                        char roomTimeStr[32];

                                        //Leggo il checkin dalla camera
                                        roomTimeSec = ((CNTHMGC*)(net->CtrlList[devIndex]))->m_CheckInDateSec;

                                        //Lo convertop a formato normale
                                        ConvertSecs2DateStr(roomTimeSec, roomTimeStr, 32);

                                         //lo metto nel checkin
                                        outMessage+=roomTimeStr;

                                        outMessage+="\" CHECKOUT=\"";

                                        roomTimeSec = ((CNTHMGC*)(net->CtrlList[devIndex]))->m_CheckOutDateSec;

                                        ConvertSecs2DateStr(roomTimeSec, roomTimeStr, 32);

                                        outMessage+=roomTimeStr;
                                    }

                                    outMessage+="\" AIRCOND=\"";
                                    outMessage+=(int)(((CNTHMGC*)(net->CtrlList[devIndex]))->m_IsAirCondEnabled);

                                    //Aggiorno i dati dal contabilizzatore
                                    //Leggo i contatori assoluti, per ogni contatore l'ordine dei dati è il seguente
                                    //Per bedazzo l'ordine è
                                    //Contatore 0 -- ContaCalorie,
                                    //Contatore 1 -- Acqua calda
                                    //Contatore 2 -- Acqua fredda
                                    //Contatore 3 -- Watt
                                    //Nei float che leggo ho i seguenti valori:
                                    //*Valore della grandezza contata (litri o watt)
                                    //*tempMnd
                                    //*tempRip
                                    //*Calorie
                                    //*Frigorie
                                    float absCounters[20];
                                    bool isSummer = ((CNTHMGC*)(net->CtrlList[devIndex]))->m_PIDIsSummer;
                                    char floatStr[8];

                                    ((CNTHMGC*)(net->CtrlList[devIndex]))->GetCounterReadings(absCounters);

                                    if (!((CNTHMGC*)(net->CtrlList[devIndex]))->m_IsRoomFree)
                                    {
                                         //Mando i contatori assoluti
                                        outMessage+="\" HOTWATER=\"";
                                        outMessage+=absCounters[5] - ((CNTHMGC*)(net->CtrlList[devIndex]))->m_CheckInCounters[0];//Acqua calda

                                        outMessage+="\" COLDWATER=\"";
                                        outMessage+=absCounters[10] - ((CNTHMGC*)(net->CtrlList[devIndex]))->m_CheckInCounters[1];//Acqua freddda

                                        outMessage+="\" POW=\"";
                                        sprintf (floatStr, "%.3f", absCounters[15] - ((CNTHMGC*)(net->CtrlList[devIndex]))->m_CheckInCounters[2]);
                                        outMessage+=floatStr; //WATT

                                        outMessage+="\" HEAT=\"";
                                        if (isSummer)
                                        {
                                            sprintf (floatStr, "%.3f", absCounters[4] - ((CNTHMGC*)(net->CtrlList[devIndex]))->m_CheckInCounters[3]);
                                            outMessage+=floatStr;//Frigorie
                                        }
                                        else
                                        {
                                            sprintf (floatStr, "%.3f", absCounters[3] - ((CNTHMGC*)(net->CtrlList[devIndex]))->m_CheckInCounters[4]);
                                            outMessage+=floatStr; //Calorie
                                        }
                                    }
                                    else
                                    {
                                        //Devo mandare i consumi relativi al checkin
                                        //Mando i contatori assoluti
                                        outMessage+="\" HOTWATER=\"";
                                        outMessage+=absCounters[5];

                                        outMessage+="\" COLDWATER=\"";
                                        outMessage+=absCounters[10];

                                        outMessage+="\" POW=\"";
                                        sprintf(floatStr, "%.3f", absCounters[15]);
                                        outMessage+=floatStr;

                                        outMessage+="\" HEAT=\"";
                                        if (isSummer)
                                        {
                                            sprintf(floatStr, "%.3f", absCounters[04]);
                                            outMessage+=absCounters[4];
                                        }
                                        else
                                        {
                                            sprintf(floatStr, "%.3f", absCounters[3]);
                                            outMessage+=absCounters[3];
                                        }
                                    }

                                    outMessage+="\" />";

                                    commandExecuted = WriteOnInterfacePorts( outMessage.c_str(), outMessage.size() );
                                }
                            }
                        }
                    }
                }
            }
        };break;
        case COMM_SETBUTTONCODE:{
            CString buttonCodeStr, name, checkInDateStr, checkOutDateStr;
            bool isGuest, removeCode;
            unsigned int checkInDateSec = 0, checkOutDateSec = 0;
            uchar hexSN[8];
            T_Net *net;
            CString outMessage;
            vector<CVController*>::iterator ctrlIt;
            struct tm timeConvStructure;
            e_DeviceType deviceType;

            //Estraggo i parametri comuni a tutti i messaggi
            name = m_XMLParser.GetStringParam("NAME");
            buttonCodeStr = m_XMLParser.GetStringParam("SERNUM");
            ConvertSN2Hex(buttonCodeStr.c_str(),hexSN);

            removeCode = m_XMLParser.GetBoolParam("REMOVE");

            //TBR l'operazione di OR
            //30/10/2008 -- Inserite sullo stesso comando altre funzionalita'
            if ((m_XMLParser.GetIntParam("ALARM") > 0) || m_Net->CheckControllerType( netIndex, devIndex, DEV_ALARMCTRL))
            {
                //E' per il controllo allarmi
                deviceType = DEV_ALARMCTRL;
            }
            else if ((m_XMLParser.GetIntParam("GATE") > 0) || m_Net->CheckControllerType( netIndex, devIndex, DEV_ACC))
            {
                //E' per il controllo cancello
                deviceType = DEV_ACC;
            }
            else if ( (m_XMLParser.GetIntParam("TIMEMARKER")>0) || m_Net->CheckControllerType( netIndex, devIndex, DEV_TIMEMARKER) )
            {
                //E' per il marcatempo
                deviceType = DEV_TIMEMARKER;
            }
            else if (m_Net->CheckControllerType( netIndex, devIndex, DEV_MGC))
            {
                deviceType = DEV_MGC;
            }
            
            //Decodifico tutti i parametri che conosco
            isGuest = m_XMLParser.GetBoolParam("GUEST");

            checkInDateStr = m_XMLParser.GetStringParam("CHECKIN");
            checkOutDateStr = m_XMLParser.GetStringParam("CHECKOUT");

            memset (&timeConvStructure, 0, sizeof(struct tm));
            if (strptime(checkInDateStr.c_str(), "%m%d%H%M%Y", &timeConvStructure) == NULL)
            {
                struct tm* tempTimeStruct;
                time_t actTime;
                time(&actTime);
                tempTimeStruct = localtime (&actTime);
                timeConvStructure = *tempTimeStruct;
            }

            ConvertDateTM2Secs(&checkInDateSec, &timeConvStructure);

            memset (&timeConvStructure, 0, sizeof(struct tm));
            if (strptime(checkOutDateStr.c_str(), "%m%d%H%M%Y", &timeConvStructure) == NULL)
            {
                struct tm* tempTimeStruct;
                time_t actTime;
                //Aggiungo un giorno alla data odierna perchè manca il checkout
                actTime+=86400;
                time(&actTime);
                tempTimeStruct = localtime (&actTime);
                timeConvStructure = *tempTimeStruct;
            }

            ConvertDateTM2Secs(&checkOutDateSec, &timeConvStructure);
            

            if (toAddress)
            {
                if (m_Net->CheckControllerType( netIndex, devIndex, deviceType) )
                {
                    //Get handler to the NET
                    net = m_Net->GetNetHandler( netIndex );

                    if (m_Net->AcquireNet( netIndex ))
                    {
                        //OK tutto torna

                        switch (deviceType)
                        {
                            case DEV_MGC:
                            {
                                //Controllo se cliente o personale
                                if (isGuest){
                                    //Controllo se aggiungo o tolgo la chiave
                                    //TBI devo anche aggiornare eventuali controlli accessi
                                    if (!removeCode){
                                        commandExecuted = ((CNTHMGC*)(net->CtrlList[devIndex]))->CheckInKey(name, hexSN, checkInDateSec, checkOutDateSec);
                                    }
                                    else {
                                        commandExecuted = ((CNTHMGC*)(net->CtrlList[devIndex]))->CheckOutKey(hexSN);
                                    }
                                }
                                else {
                                    //E' una chiave del personale
                                    //TBI devo gestirlo anche a livello di engine per aggiornare il file di configurazione
                                    //TBI devo anche aggiornare eventuali controlli accessi
                                    if (!removeCode){
                                        commandExecuted =((CNTHMGC*)(net->CtrlList[devIndex]))->ProgramRomCode(hexSN, false);
                                    }
                                    else {
                                        commandExecuted =((CNTHMGC*)(net->CtrlList[devIndex]))->EraseRomCode(hexSN, false);
                                    }
                                }

                                if (!isGuest)
                                {
                                    checkOutDateSec = 0;
                                }

                            };break;
                            case DEV_ALARMCTRL:
                            {
                                CString zoneStr;
                                int zoneNumber;

                                zoneStr = m_XMLParser.GetStringParam("ZONE");

                                if ((zoneStr == "") || (zoneStr=="ALL"))
                                {
                                    zoneNumber = -1;
                                }
                                else
                                {
                                    zoneNumber = m_XMLParser.GetIntParam("ZONE");
                                }

                                if (!removeCode)
                                {
                                    //TODO da gestire la possibilità che la chiave abiliti o meno gli allarmi
                                    commandExecuted = ((AlarmCoordinator*)(net->CtrlList[devIndex]))->AddKey ( name, buttonCodeStr, true, zoneNumber );
                                }
                                else
                                {
                                    commandExecuted = ((AlarmCoordinator*)(net->CtrlList[devIndex]))->RemoveKey ( buttonCodeStr, zoneNumber );
                                }

                            };break;
                            case DEV_TIMEMARKER:
                            {
                                if (!removeCode)
                                {
                                    commandExecuted = ((CTimeMarkerCtrl*)(net->CtrlList[devIndex]))->AddKey(name, buttonCodeStr);
                                }
                                else
                                {
                                    commandExecuted = ((CTimeMarkerCtrl*)(net->CtrlList[devIndex]))->RemoveKey(buttonCodeStr);
                                }
                            };break;
                            case DEV_ACC:
                            {
                                t_AccessData keyData;

                                keyData.name = name;
                                
                                //Lo rifaccio per interpretare correttamente il valore di checkout
                                if (m_XMLParser.ExistsParam("CHECKOUT")) {
                                    checkOutDateStr = m_XMLParser.GetStringParam("CHECKOUT");
                                    memset (&timeConvStructure, 0, sizeof(struct tm));
                                    if (strptime(checkOutDateStr.c_str(), "%m%d%H%M%Y", &timeConvStructure) == NULL)
                                    {
                                        keyData.expireDateSec = 0;
                                    }
                                    else {
                                        ConvertDateTM2Secs(&(keyData.expireDateSec), &timeConvStructure);
                                    }
                                }
                                else {
                                    keyData.expireDateSec = 0;
                                }
                                
                                keyData.keySN = buttonCodeStr;

                                //TODO da implementare la gestione della data di scadenza
                                int channel = m_XMLParser.GetIntParam("CHANNEL");

                                if (( channel< 0) || (channel >2))
                                {
                                    channel = 2; //Aggiungi ad entrambi i canali
                                }

                                if (!removeCode)
                                {
                                    commandExecuted = ((CNTH_ACC*)(net->CtrlList[devIndex]))->AddKey(keyData);
                                }
                                else
                                {
                                    commandExecuted = ((CNTH_ACC*)(net->CtrlList[devIndex]))->RemoveKey(keyData);
                                }
                            };break;
                            default:
                            {
                                commandExecuted = false;
                            }
                        }//Switch
                    }//If m_Net isAcquired
                }//If CheckControllertype

                if (deviceType == DEV_MGC)
                {
                    //Se la camera non esiste comunque controllo se c'e' il controllo accessi
                    //Lo devo salvare anche nel controllo accessi per basculante e cancellino
                    //Controllo se devo reinviare il messaggio ad altri controlli accessi
                    if (!m_XMLParser.ExistsParam("RELAYMSG"))
                    {
                        //Il comando arrivato non e' stato inviato da un altro controllo accessi
                        commandExecuted = m_Net->ManageAccKeys(!removeCode,name, hexSN, checkOutDateSec,2,true);
                    }
                    else {
                        commandExecuted = m_Net->ManageAccKeys(!removeCode,name, hexSN, checkOutDateSec,2,false);
                    }
                }
            }
            else if ((toEveryOne) || toNet)
            {
                //TODO da implementare per tutti i tipi
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    net = m_Net->GetNetHandler( netIndex );

                    if (m_Net->AcquireNet( netIndex ))
                    {
                        for (ctrlIt = net->CtrlList.begin(); ctrlIt!=net->CtrlList.end(); ctrlIt++)
                        {
                            if ((*ctrlIt)->GetControllerType() == DEV_MGC)
                            {
                                //Controllo se cliente o personale
                                if (isGuest){
                                    //Controllo se aggiungo o tolgo la chiave
                                    if (!removeCode){
                                        commandExecuted = ((CNTHMGC*)(*ctrlIt))->CheckInKey(name, hexSN, checkInDateSec, checkOutDateSec) && commandExecuted;
                                    }
                                    else {
                                        commandExecuted = ((CNTHMGC*)(*ctrlIt))->CheckOutKey(hexSN) && commandExecuted;
                                    }
                                }
                                else {
                                    //E' una chiave del personale
                                    if (!removeCode){
                                        commandExecuted =((CNTHMGC*)(*ctrlIt))->ProgramRomCode(hexSN, false) && commandExecuted;
                                    }
                                    else {
                                        commandExecuted =((CNTHMGC*)(*ctrlIt))->EraseRomCode(hexSN, false) && commandExecuted;
                                    }
                                }
                            }
                            else if ((*ctrlIt)->GetControllerType() == DEV_ACC)
                            {
                                //TBM andra' tutta modificata: per ora e' solo per Bedazzo
                                //Se non è un ospite la devo aggiungere ai moduli di controllo accesso
                                t_AccessData keyData;

                                keyData.name = name;
                                keyData.expireDateSec = 0; //TBI
                                keyData.keySN = buttonCodeStr;
                                keyData.channel = 2;

                                if (!isGuest)
                                {
                                    if (!removeCode){
                                        commandExecuted = ((CNTH_ACC*)(*ctrlIt))->AddKey(keyData) && commandExecuted;
                                    }
                                    else {
                                        commandExecuted = ((CNTH_ACC*)(*ctrlIt))->RemoveKey(keyData) && commandExecuted;
                                    }
                                    
                                    //Mando il comando anche ad un eventuale acc remoto
//                                     if (commandExecuted) {
//                                         ((CNTH_ACC*)(*ctrlIt))->SendInfoToRemoteAcc(!removeCode, keyData);
//                                     }
                                }

                            }
                        }
                    }
                }
            }

        };break;
        case COMM_GETPERSONNELLIST:{
            //Questo fa crashare il programma, da debuggare -- OK fissato
             //Questo lo gestisco da engine a livello di file
             CString outMessage;
             T_Net *net;


             outMessage = "<DEVICE TYPE=\"PersonnelList\" PERS=\"";

             if (toAddress)
             {
                 if (m_Net->CheckControllerType( netIndex, devIndex, DEV_TIMEMARKER) )
                 {
                    //Get handler to the NET
                     net = m_Net->GetNetHandler( netIndex );

                     if (m_Net->AcquireNet( netIndex ))
                     {
                         vector<t_TM_KeyData>::iterator keyIt;
                         for (keyIt = ((CTimeMarkerCtrl*)(net->CtrlList[devIndex]))->m_ValidKeys.begin(); keyIt < ((CTimeMarkerCtrl*)(net->CtrlList[devIndex]))->m_ValidKeys.end(); keyIt++){

                             outMessage+=keyIt->name;
                             outMessage+=":";
                             outMessage+=keyIt->serialNum;

                             if (keyIt < ((CTimeMarkerCtrl*)(net->CtrlList[devIndex]))->m_ValidKeys.end() - 1){
                                 outMessage+=",";
                             }
                         }
                     }
                 }
                 else if (m_Net->CheckControllerType( netIndex, devIndex, DEV_ALARMCTRL) )
                 {
                    //Get handler to the NET
                     net = m_Net->GetNetHandler( netIndex );

                     if (m_Net->AcquireNet( netIndex ))
                     {
                         vector<t_Zone>::iterator zoneIt;
                         for (zoneIt = ((AlarmCoordinator*)(net->CtrlList[devIndex]))->m_ZoneVector.begin();
                              zoneIt < ((AlarmCoordinator*)(net->CtrlList[devIndex]))->m_ZoneVector.end();
                              zoneIt++)
                         {
                             vector<t_AccessData>::iterator keyIt;

                             for (keyIt = zoneIt->keysList.begin(); keyIt < zoneIt->keysList.end(); keyIt++)
                             {
                                outMessage+=keyIt->name;
                                outMessage+=":";
                                outMessage+=keyIt->keySN;
                                outMessage+=",";
                             }
                         }
                     }
                 }
                 else if (m_Net->CheckControllerType( netIndex, devIndex, DEV_ACC) )
                 {
                     //Get handler to the NET
                     net = m_Net->GetNetHandler( netIndex );

                     if (m_Net->AcquireNet( netIndex ))
                     {
                         vector<t_AccessData> accVector;
                         vector<t_AccessData>::iterator keyIt;

                         accVector = ((CNTH_ACC*)(net->CtrlList[devIndex]))->getRegisteredKeysArray();
                         for (keyIt = accVector.begin(); keyIt < accVector.end(); keyIt++)
                         {
                             //PER BEDAZZO  LA AGGIUNGO ALLA LISTA SOLO SE E' DAVVERO UNA CHIAVE PASSPARTOUT (EXP = 0)
                             if (keyIt->expireDateSec != 0)
                             {
                                 continue;
                             }

                             outMessage+=keyIt->name;
                             outMessage+=":";
                             outMessage+=keyIt->keySN;

                             if (keyIt < accVector.end() - 1){
                                 outMessage+=",";
                             }
                         }
                     }
                 }
             }
             else
             {
                 vector<CVController*>::iterator ctrlIt;

                 commandExecuted = true;

                 for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                 {
                     net = m_Net->GetNetHandler( netIndex );

                     if (m_Net->AcquireNet( netIndex ))
                     {
                         for (ctrlIt = net->CtrlList.begin(); ctrlIt!=net->CtrlList.end(); ctrlIt++)
                         {
                             if ( (*ctrlIt)->GetControllerType() == DEV_ACC)
                             {
                                //Get handler to the NET
                                 net = m_Net->GetNetHandler( netIndex );

                                 if (m_Net->AcquireNet( netIndex ))
                                 {
                                     vector<t_AccessData> accVector;
                                     vector<t_AccessData>::iterator keyIt;

                                     accVector = ((CNTH_ACC*)(*ctrlIt))->getRegisteredKeysArray();
                                     for (keyIt = accVector.begin(); keyIt < accVector.end(); keyIt++)
                                     {
                                         //solo PER BEDAZZO: FACCIO IN MODO DI TRASMETTERE SOLO LE VERE CHIAVI PERSONALE

                                         if (keyIt->expireDateSec != 0)
                                         {
                                             continue;
                                         }
                                         outMessage+=keyIt->name;
                                         outMessage+=":";
                                         outMessage+=keyIt->keySN;

                                         if (keyIt < accVector.end() - 1){
                                             outMessage+=",";
                                         }
                                     }
                                 }
                             }
                         }
                     }

                 }
             }

             outMessage+="\" />";

            commandExecuted = WriteOnInterfacePorts( outMessage.c_str(), outMessage.size() );
            commandExecuted = true;

        };break;//---------------COMM_GETPERSONNELLIST--------------------------
        case COMM_CHECKOUTROOM:{
            T_Net *net;
            vector<CVController*>::iterator ctrlIt;
            vector<t_MGC_KeyData> guestArray;
            char buttonCodeStr[32];

            if (toAddress)
            {
                if (m_Net->CheckControllerType( netIndex, devIndex, DEV_MGC) )
                {
                    //Get handler to the NET
                    net = m_Net->GetNetHandler( netIndex );

                    if (m_Net->AcquireNet( netIndex ))
                    {
//                         !!!!!!!!!!!!ARGHHHHHHHHHHHH!!!!!!!!!!!!!!! Devo cancellare le chiavi associate a questa stanza anche nel modulo di controllo accessi
                       guestArray = ((CNTHMGC*)(net->CtrlList[devIndex]))->m_GuestArray;
                       commandExecuted =((CNTHMGC*)(net->CtrlList[devIndex]))->CheckOutRoom();
                       if (commandExecuted)
                       {
                           //Tolgo le chiavi associate a questa stanza dal controllo accessi
                           for (unsigned int keyIdx = 0; keyIdx < guestArray.size(); keyIdx++)
                           {
                               //Altri cervelli in rete sono aggiornati direttamente dal modulo acc virtuale se presente
                               commandExecuted = m_Net->ManageAccKeys(false,"",guestArray[keyIdx].keySN, 0xFFFFFFFF,2,true) && commandExecuted;
                               
                               memset (buttonCodeStr, 0x0, 32);
                               ConvertSN2Str(buttonCodeStr, guestArray[keyIdx].keySN);
                           }
                       }
                    }
                }
            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    net = m_Net->GetNetHandler( netIndex );

                    if (m_Net->NetHasTempDevices( netIndex ))
                    {
                        if (m_Net->AcquireNet( netIndex ))
                        {
                            for (ctrlIt = net->CtrlList.begin(); ctrlIt!=net->CtrlList.end(); ctrlIt++)
                            {
                                if ((*ctrlIt)->GetControllerType() == DEV_MGC)
                                {
                                    commandExecuted =((CNTHMGC*)(net->CtrlList[devIndex]))->CheckOutRoom() && commandExecuted;
                                }
                            }
                        }
                    }
                }
            }
        };break;
        case COMM_SETROOMSTATUS:{
            bool turnOn;
            T_Net *net;
            vector<CVController*>::iterator ctrlIt;

            turnOn = m_XMLParser.GetBoolParam("STATUS");

            if (toAddress)
            {
                if (m_Net->CheckControllerType( netIndex, devIndex, DEV_MGC) )
                {
                    //Get handler to the NET
                    net = m_Net->GetNetHandler( netIndex );

                    if (m_Net->AcquireNet( netIndex ))
                    {
                       commandExecuted =((CNTHMGC*)(net->CtrlList[devIndex]))->TurnOnRoom(turnOn);
                    }
                }
                else
                {
                    commandExecuted = false;
                }
            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    net = m_Net->GetNetHandler( netIndex );

                    if (m_Net->AcquireNet( netIndex ))
                    {
                        for (ctrlIt = net->CtrlList.begin(); ctrlIt!=net->CtrlList.end(); ctrlIt++)
                        {
                            if ((*ctrlIt)->GetControllerType() == DEV_MGC)
                            {
                              commandExecuted =((CNTHMGC*)(*ctrlIt))->TurnOnRoom(turnOn) && commandExecuted;
                            }
                        }
                    }
                }
            }
        };break;
        case COMM_SETAIRCOND:{
            bool enableAirCond;
            T_Net *net;
            vector<CVController*>::iterator ctrlIt;

            enableAirCond = m_XMLParser.GetBoolParam("STATUS");

            if (toAddress)
            {
                if (m_Net->CheckControllerType( netIndex, devIndex, DEV_MGC) )
                {
                    //Get handler to the NET
                    net = m_Net->GetNetHandler( netIndex );

                    if (m_Net->AcquireNet( netIndex ))
                    {
                       commandExecuted =((CNTHMGC*)(net->CtrlList[devIndex]))->SetAirCondEnable(enableAirCond);
                    }
                }
                else
                {
                    commandExecuted = false;
                }
            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    net = m_Net->GetNetHandler( netIndex );

                    if (m_Net->NetHasTempDevices( netIndex ))
                    {
                        if (m_Net->AcquireNet( netIndex ))
                        {
                            for (ctrlIt = net->CtrlList.begin(); ctrlIt!=net->CtrlList.end(); ctrlIt++)
                            {
                                if ((*ctrlIt)->GetControllerType() == DEV_MGC)
                                {
                                    commandExecuted =((CNTHMGC*)(net->CtrlList[devIndex]))->SetAirCondEnable(enableAirCond) && commandExecuted;
                                }
                            }
                        }
                    }
                }
            }
        };break;
        case COMM_GETPLANTSTATE: {

            T_Net* net;
            CString outMessage;
            vector<CVController*>::iterator ctrlIt;

            if (toAddress)
            {
                if (m_Net->CheckControllerType( netIndex, devIndex, DEV_CHOVER) )
                {
                    //Get handler to the NET
                    net = m_Net->GetNetHandler( netIndex );

                    if (m_Net->AcquireNet( netIndex ))
                    {
                        outMessage = "<DEVICE TYPE=\"PlantState\" ADDRESS=\"";
                        outMessage+=addressStr;
                        outMessage+="\" SUMMER=\"";
                        outMessage+=((ChangeOverCoord*)(net->CtrlList[devIndex]))->m_IsSummer;
                        outMessage+="\" ON=\"";
                        outMessage+=((ChangeOverCoord*)(net->CtrlList[devIndex]))->m_IsChangeOverStarted;
                        outMessage+="\" />";

                        commandExecuted = WriteOnInterfacePorts( outMessage.c_str(), outMessage.size() );
                    }
                }
                else
                {
                    commandExecuted = false;
                }
            }
            else if (toEveryOne || toNet)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    net = m_Net->GetNetHandler( netIndex );

                    if (m_Net->NetHasTempDevices( netIndex ))
                    {
                        if (m_Net->AcquireNet( netIndex ))
                        {
                            for (ctrlIt = net->CtrlList.begin(); ctrlIt!=net->CtrlList.end(); ctrlIt++)
                            {
                                if ((*ctrlIt)->GetControllerType() == DEV_CHOVER)
                                {
                                    outMessage = "<DEVICE TYPE=\"PlantState\" ADDRESS=\"";
                                    outMessage+=(*ctrlIt)->GetMemoryAddress();
                                    outMessage+="\" SUMMER=\"";
                                    outMessage+=( (ChangeOverCoord*)(*ctrlIt) )->m_IsSummer;
                                    outMessage+="\" ON=\"";
                                    outMessage+=( (ChangeOverCoord*)(*ctrlIt) )->m_IsChangeOverStarted;
                                    outMessage+="\" />";

                                    commandExecuted = WriteOnInterfacePorts( outMessage.c_str(), outMessage.size() );
                                }
                            }
                        }
                    }
                }
            }

        };break;
        case COMM_SETPLANTSTATE: {

            bool setSummer = false;
            T_Net *net;
            vector<CVController*>::iterator ctrlIt;

            setSummer = m_XMLParser.GetIntParam("SUMMER");

            if (toAddress)
            {
                if (m_Net->CheckControllerType( netIndex, devIndex, DEV_CHOVER) )
                {
                    //Get handler to the NET
                    net = m_Net->GetNetHandler( netIndex );

                    if (m_Net->AcquireNet( netIndex ))
                    {
                        commandExecuted = true;

                        //Se la nuova impostazione per l'estate è diversa dalla vecchia attivo il changeover
                        //NON lo faccio se c'è già un changeover in corso
                        if (  (!((ChangeOverCoord*)(net->CtrlList[devIndex]))->GetStartChangeOver()) &&
                            ( ((ChangeOverCoord*)(net->CtrlList[devIndex]))->m_IsSummer != setSummer)
                           )
                        {
                            ((ChangeOverCoord*)(net->CtrlList[devIndex]))->m_IsSummer = setSummer;
                            ((ChangeOverCoord*)(net->CtrlList[devIndex]))->StartChangeOver ( );
                        }
                    }
                }
                else
                {
                    commandExecuted = false;
                }
            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    net = m_Net->GetNetHandler( netIndex );

                    if (m_Net->NetHasTempDevices( netIndex ))
                    {
                        if (m_Net->AcquireNet( netIndex ))
                        {
                            for (ctrlIt = net->CtrlList.begin(); ctrlIt!=net->CtrlList.end(); ctrlIt++)
                            {
                                if ((*ctrlIt)->GetControllerType() == DEV_CHOVER)
                                {
                                    //Se la nuova impostazione per l'estate è diversa dalla vecchia attivo il changeover
                                    if ( ((ChangeOverCoord*)(*ctrlIt))->m_IsSummer != setSummer)
                                    {
                                        ((ChangeOverCoord*)(*ctrlIt))->m_IsSummer = setSummer;
                                        ((ChangeOverCoord*)(*ctrlIt))->StartChangeOver ();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        };break;
        case COMM_SETALARMSTATE:
        {
            T_Net *net;
            vector<CVController*>::iterator ctrlIt;
            int zoneIndex;
            bool newState;

            zoneIndex = m_XMLParser.GetIntParam("ZONE");
            newState = m_XMLParser.GetBoolParam("STATE");

            //Decremento l'indice di zona
            zoneIndex--;

            if (toAddress)
            {
                if (m_Net->CheckControllerType( netIndex, devIndex, DEV_ALARMCTRL) )
                {
                    //Get handler to the NET
                    net = m_Net->GetNetHandler( netIndex );

                    if (m_Net->AcquireNet( netIndex ))
                    {
                        commandExecuted = ((AlarmCoordinator*)(net->CtrlList[devIndex]))->SetZoneAlarmState(zoneIndex, newState);
                    }
                }
                else
                {
                    commandExecuted = false;
                }
            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    net = m_Net->GetNetHandler( netIndex );

                    if (m_Net->NetHasTempDevices( netIndex ))
                    {
                        if (m_Net->AcquireNet( netIndex ))
                        {
                            for (ctrlIt = net->CtrlList.begin(); ctrlIt!=net->CtrlList.end(); ctrlIt++)
                            {
                                if ((*ctrlIt)->GetControllerType() == DEV_ALARMCTRL)
                                {
                                    ( (AlarmCoordinator*)(*ctrlIt) )->SetZoneAlarmState(zoneIndex, newState);
                                }
                            }
                        }
                    }
                }
            }
        };break;
        case COMM_SETFLOORSETTINGS:
        {
            T_Net *net;
            vector<CString>aeroConfigVector;
            int zoneIndex = 0;
            int nOfAero = 0;
            float setpoints[4] = {-1.0,-1.0,-1.0,-1.0};

            zoneIndex = m_XMLParser.GetIntParam("ZONE");
            nOfAero = m_XMLParser.GetIntParam("NOFAERO");

            for (int aeroIndex = 0; aeroIndex < nOfAero; aeroIndex++)
            {
                CString aeroKey = CString("AERO")+(aeroIndex+1);
                CString aeroConfiguration;

                aeroConfiguration = m_XMLParser.GetStringParam(aeroKey);
                aeroConfigVector.push_back(aeroConfiguration);
            }

            setpoints[0] = m_XMLParser.GetFloatParam("SUMSP");
            setpoints[1] = m_XMLParser.GetFloatParam("WINSP");
            setpoints[2] = m_XMLParser.GetFloatParam("WATSUMSP");
            setpoints[3] = m_XMLParser.GetFloatParam("WATWINSP");

            if (toAddress)
            {
                if (m_Net->CheckControllerType( netIndex, devIndex, DEV_FLOORCOORD_2) )
                {
                    //Get handler to the NET
                    net = m_Net->GetNetHandler( netIndex );

                    if (m_Net->AcquireNet( netIndex ))
                    {
                        commandExecuted = true;
                        t_FloorZone* floorZone = ((CFloorCoord2*)(net->CtrlList[devIndex]))->GetZoneNumberPtr(zoneIndex);

                         //Controllo che la zona esista
                        if (floorZone != 0x0)
                        {
                            vector<CString>::iterator configIt;
                            configIt = aeroConfigVector.begin();
                            for (unsigned int aeroIndex = 0;aeroIndex < floorZone->Aerotherms.size(); aeroIndex++)
                            {
                                if (configIt == aeroConfigVector.end())
                                {
                                    //Esco perche' ho ricevuto meno stringhe degli aerotermi che ho istanziato
                                    break;
                                }

                                floorZone->Aerotherms[aeroIndex].clearHumidities();
                                floorZone->Aerotherms[aeroIndex].clearTemps();

                                ((CFloorCoord2*)(net->CtrlList[devIndex]))->SetAeroData(&(floorZone->Aerotherms[aeroIndex]), configIt->c_str(), "HUM");
                                ((CFloorCoord2*)(net->CtrlList[devIndex]))->SetAeroData(&(floorZone->Aerotherms[aeroIndex]), configIt->c_str(), "DELTAT");

                                sort(floorZone->Aerotherms[aeroIndex].humiditySPVector.begin(), floorZone->Aerotherms[aeroIndex].humiditySPVector.end());
                                sort(floorZone->Aerotherms[aeroIndex].temperatureDeltaVector.begin(), floorZone->Aerotherms[aeroIndex].temperatureDeltaVector.end());
                                configIt++;
                            }

                            ((CFloorCoord2*)(net->CtrlList[devIndex]))->SaveZoneAeroData(*floorZone);

                        }

                        if ((setpoints[0] > 0.0) && (setpoints[1] > 0.0) && (setpoints[2] > 0.0) && (setpoints[3] > 0.0))
                        {
                            ((CFloorCoord2*)(net->CtrlList[devIndex]))->SetAdvancedSetpoints(zoneIndex, setpoints);
                        }

                        //Riuso setpoints
                        setpoints[0] = m_XMLParser.GetFloatParam("SUMMAINTSP");
                        setpoints[1] = m_XMLParser.GetFloatParam("WINTMAINTSP");

                        if ((setpoints[0] > 0.0) && (setpoints[1] >0.0))
                        {
                            ((CFloorCoord2*)(net->CtrlList[devIndex]))->SetTempMaintenance(zoneIndex, setpoints[0], setpoints[1], true);
                        }
                    }

                }
                else
                {
                    commandExecuted = false;
                }
            }
//             else if ((toEveryOne) || toNet)
//             {
//                 commandExecuted = true;
//
//                 for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
//                 {
//                     net = m_Net->GetNetHandler( netIndex );
//
//                     if (m_Net->NetHasTempDevices( netIndex ))
//                     {
//                         if (m_Net->AcquireNet( netIndex ))
//                         {
//                             for (ctrlIt = net->CtrlList.begin(); ctrlIt!=net->CtrlList.end(); ctrlIt++)
//                             {
//                                 if ((*ctrlIt)->GetControllerType() == DEV_ALARMCTRL)
//                                 {
//                                     ( (AlarmCoordinator*)(*ctrlIt) )->SetZoneAlarmState(zoneIndex, newState);
//                                 }
//                             }
//                         }
//                     }
//                 }
//             }
        };break;
        case COMM_GETFLOORSETTINGS:
        {
            T_Net *net;
            vector<CVController*>::iterator ctrlIt;
            vector<CString>aeroConfigVector;
            float setpoints[4] = {-1.0,-1.0,-1.0,-1.0};


            if (toAddress)
            {
                if (m_Net->CheckControllerType( netIndex, devIndex, DEV_FLOORCOORD_2) )
                {
                    //Get handler to the NET
                    net = m_Net->GetNetHandler( netIndex );

                    if (m_Net->AcquireNet( netIndex ))
                    {

                        commandExecuted = true;

                        vector<t_FloorZone> zoneVector = ((CFloorCoord2*)(net->CtrlList[devIndex]))->GetZoneVectorRef();
                        vector<t_FloorZone>::iterator zoneIt;

                        //Itero sulle zone
                        for (zoneIt = zoneVector.begin(); zoneIt < zoneVector.end(); zoneIt++)
                        {
                            Cmd com("DEVICE");
                            com.putValue("TYPE","FloorCoordSettings");

                            com.putValue("ADDRESS",net->CtrlList[devIndex]->GetMemoryAddress());

                            com.putValue("ZONE",CString("")+zoneIt->zNumber);

                            vector<t_Aerotherm> aerotherms = zoneIt->Aerotherms;
                            vector<t_Aerotherm>::iterator aeroIt;

                            com.putValue("NOFAERO",(unsigned int)aerotherms.size());

                            //Itero sugli aerotermi
                            for (aeroIt = aerotherms.begin(); aeroIt < aerotherms.end(); aeroIt++)
                            {
                                CString aeroKey = "AERO";
                                CString aeroConfig = "";
                                aeroKey+=(aeroIt-aerotherms.begin())+1;

                                for (unsigned int i = 0; i < aeroIt->speedVector.size(); i++)
                                {
                                    aeroConfig+="HUM";
                                    aeroConfig+=(i+1);
                                    aeroConfig+=":";
                                    aeroConfig+=aeroIt->humiditySPVector[i];

                                    aeroConfig+=",";

                                    aeroConfig+="DELTAT";
                                    aeroConfig+=(i+1);
                                    aeroConfig+=":";
                                    aeroConfig+=aeroIt->temperatureDeltaVector[i];

                                    if (i < aeroIt->speedVector.size() -1)
                                    {
                                        aeroConfig += ",";
                                    }
                                }

                                com.putValue(aeroKey, aeroConfig);
                            }

                            ((CFloorCoord2*)(net->CtrlList[devIndex]))->GetAdvancedSetpoints(zoneIt->zNumber, setpoints);

                            com.putValue("SUMSP",setpoints[0]);
                            com.putValue("WINSP",setpoints[1]);
                            com.putValue("WATSUMSP", setpoints[2]);
                            com.putValue("WATWINSP", setpoints[3]);

                            com.putValue("SUMMAINTSP",zoneIt->Maintenance.SummerTempValue);
                            com.putValue("WINTMAINTSP",zoneIt->Maintenance.WinterTempValue);

                            CString outMessage = com.getXMLValue();
                            WriteOnInterfacePorts(outMessage.c_str(), outMessage.size());
                        }
                    }
                }
                else
                {
                    commandExecuted = false;
                }
            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;

                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    net = m_Net->GetNetHandler( netIndex );

                    if (m_Net->AcquireNet( netIndex ))
                    {
                        for (ctrlIt = net->CtrlList.begin(); ctrlIt!=net->CtrlList.end(); ctrlIt++)
                        {
                            if ((*ctrlIt)->GetControllerType() == DEV_FLOORCOORD_2)
                            {
                                commandExecuted = true;

                                const vector<t_FloorZone> zoneVector = ((CFloorCoord2*)(*ctrlIt))->GetZoneVectorRef();
                                vector<t_FloorZone>::const_iterator zoneIt;

                                //Itero sulle zone
                                for (zoneIt = zoneVector.begin(); zoneIt < zoneVector.end(); zoneIt++)
                                {
                                    Cmd com("DEVICE");
                                    com.putValue("TYPE","FloorCoordSettings");

                                    com.putValue("ZONE",CString("")+zoneIt->zNumber);

                                    com.putValue("ADDRESS",(*ctrlIt)->GetMemoryAddress());

                                    vector<t_Aerotherm> aerotherms = zoneIt->Aerotherms;
                                    vector<t_Aerotherm>::iterator aeroIt;

                                    com.putValue("NOFAERO",(unsigned int)aerotherms.size());

                                     //Itero sugli aerotermi
                                    for (aeroIt = aerotherms.begin(); aeroIt < aerotherms.end(); aeroIt++)
                                    {
                                        CString aeroKey = "AERO";
                                        CString aeroConfig = "";
                                        aeroKey+=(aeroIt-aerotherms.begin())+1;

                                        for (unsigned int i = 0; i < aeroIt->speedVector.size(); i++)
                                        {
                                            aeroConfig+="HUM";
                                            aeroConfig+=(i+1);
                                            aeroConfig+=":";
                                            aeroConfig+=aeroIt->humiditySPVector[i];

                                            aeroConfig+=",";

                                            aeroConfig+="DELTAT";
                                            aeroConfig+=(i+1);
                                            aeroConfig+=":";
                                            aeroConfig+=aeroIt->temperatureDeltaVector[i];

                                            if (i < aeroIt->speedVector.size() -1)
                                            {
                                                aeroConfig += ",";
                                            }
                                        }

                                        com.putValue(aeroKey, aeroConfig);
                                    }

                                    ((CFloorCoord2*)(*ctrlIt))->GetAdvancedSetpoints(zoneIt->zNumber, setpoints);

                                    com.putValue("SUMSP",setpoints[0]);
                                    com.putValue("WINSP",setpoints[1]);
                                    com.putValue("WATSUMSP", setpoints[2]);
                                    com.putValue("WATWINSP", setpoints[3]);

                                    com.putValue("SUMMAINTSP",zoneIt->Maintenance.SummerTempValue);
                                    com.putValue("WINTMAINTSP",zoneIt->Maintenance.WinterTempValue);

                                    CString outMessage = com.getXMLValue();
                                    WriteOnInterfacePorts(outMessage.c_str(), outMessage.size());
                                }
                            }
                        }
                    }

                }
            }
        };break;
        case COMM_GETVLV2SETTINGS:{
            T_Net *net;
            vector<CVController*>::iterator ctrlIt;

            if (toAddress)
            {
                if (m_Net->CheckControllerType( netIndex, devIndex, DEV_AFOVLV) )
                {
                    //Get handler to the NET
                    net = m_Net->GetNetHandler( netIndex );

                    if ( (net != 0x0) && (m_Net->AcquireNet( netIndex )) )
                    {
                        CNTHVLV2* vlvPtr = ((CNTHVLV2*)(net->CtrlList[devIndex]));
                        Cmd com("DEVICE");

                        com.putValue("ADDRESS",vlvPtr->GetMemoryAddress());
                        com.putValue("TYPE","VLV2State");
                        com.putValue("SP1",vlvPtr->m_SetPoint1);
                        com.putValue("KP1",vlvPtr->m_Pid1Parameters[0]);
                        com.putValue("TINT1",vlvPtr->m_Pid1Parameters[1]);
                        com.putValue("TDER1",vlvPtr->m_Pid1Parameters[2]);
                        com.putValue("DIV1",vlvPtr->m_PID1Divider);

                        com.putValue("SP2",vlvPtr->m_SetPoint2);
                        com.putValue("KP2",vlvPtr->m_Pid2Parameters[0]);
                        com.putValue("TINT2",vlvPtr->m_Pid2Parameters[1]);
                        com.putValue("TDER2",vlvPtr->m_Pid2Parameters[2]);
                        com.putValue("DIV2",vlvPtr->m_PID2Divider);

                        CString outMessage = com.getXMLValue();
                        commandExecuted = WriteOnInterfacePorts(outMessage.c_str(), outMessage.size());
                    }
                }
            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;
                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    net = m_Net->GetNetHandler( netIndex );

                    if ( (net != 0x0) && (m_Net->AcquireNet( netIndex )) )
                    {
                        for (ctrlIt = net->CtrlList.begin(); ctrlIt!=net->CtrlList.end(); ctrlIt++)
                        {
                            if ((*ctrlIt)->GetControllerType() == DEV_AFOVLV)
                            {
                                CNTHVLV2* vlvPtr = ((CNTHVLV2*)(*ctrlIt));
                                Cmd com("DEVICE");

                                com.putValue("ADDRESS",vlvPtr->GetMemoryAddress());
                                com.putValue("TYPE","VLV2Settings");
                                com.putValue("SP1",vlvPtr->m_SetPoint1);
                                com.putValue("KP1",vlvPtr->m_Pid1Parameters[0]);
                                com.putValue("TINT1",vlvPtr->m_Pid1Parameters[1]);
                                com.putValue("TDER1",vlvPtr->m_Pid1Parameters[2]);
                                com.putValue("DIV1",vlvPtr->m_PID1Divider);

                                com.putValue("SP2",vlvPtr->m_SetPoint2);
                                com.putValue("KP2",vlvPtr->m_Pid2Parameters[0]);
                                com.putValue("TINT2",vlvPtr->m_Pid2Parameters[1]);
                                com.putValue("TDER2",vlvPtr->m_Pid2Parameters[2]);
                                com.putValue("DIV2",vlvPtr->m_PID2Divider);

                                CString outMessage = com.getXMLValue();
                                commandExecuted = WriteOnInterfacePorts(outMessage.c_str(), outMessage.size());
                            }
                        }
                    }
                }
            }
        };break;
        case COMM_SETVLV2SETTINGS:{

            T_Net *net;
            float params[4] = {-1.0,-1.0,-1.0,-1.0};
            int pidDivider;
            bool isVAV = false;

            if (toAddress)
            {
                if (m_Net->CheckControllerType( netIndex, devIndex, DEV_AFOVLV)  || m_Net->CheckControllerType( netIndex, devIndex, DEV_AFOVLV_VAV)) 
                {
                    if (m_Net->CheckControllerType( netIndex, devIndex, DEV_AFOVLV_VAV))
                    {
                        isVAV = true;
                    }
                    
                    //Get handler to the NET
                    net = m_Net->GetNetHandler( netIndex );

                    if ( (net != 0x0) && (m_Net->AcquireNet( netIndex )) )
                    {
                        if (m_XMLParser.ExistsParam("SETDIGOUT"))
                        {
                            bool turnOn = m_XMLParser.GetBoolParam("SETDIGOUT");

                            if (isVAV)
                            {
                                commandExecuted = ((CNTHVLV2_VAV*)(net->CtrlList[devIndex]))->TurnOnModule(0,turnOn);
                            }
                            else
                            {
                                commandExecuted = ((CNTHVLV2*)(net->CtrlList[devIndex]))->TurnOnModule(0,turnOn);
                            }
                        }

                        for (int i = 1; i < 3; i++)
                        {
                            CString kp("KP");
                            CString tInt("TINT");
                            CString tDer("TDER");
                            CString div("DIV");
                            CString setpoint ("SETPOINT");

                            kp+=i;
                            tInt+=i;
                            tDer+=i;
                            div+=i;
                            if (i == 2)
                            {
                                setpoint+=i;
                            }

                            if (m_XMLParser.ExistsParam(kp)      &&
                                m_XMLParser.ExistsParam(tInt)    &&
                                m_XMLParser.ExistsParam(tDer)    &&
                                m_XMLParser.ExistsParam(div))
                            {
                                params[0] = m_XMLParser.GetFloatParam(kp);
                                params[1] = m_XMLParser.GetFloatParam(tInt);
                                params[2] = m_XMLParser.GetFloatParam(tDer);
                                pidDivider = m_XMLParser.GetIntParam(div);

                                if (isVAV)
                                {
                                    commandExecuted = ((CNTHVLV2_VAV*)(net->CtrlList[devIndex]))->SetPIDParam(i,params, pidDivider);
                                }
                                else
                                {
                                    commandExecuted = ((CNTHVLV2*)(net->CtrlList[devIndex]))->SetPIDParam(i,params, pidDivider);
                                }
                            }

                            if (m_XMLParser.ExistsParam(setpoint))
                            {
                                params[0] = m_XMLParser.GetFloatParam(setpoint);
                                if (isVAV)
                                {
                                    commandExecuted = ((CNTHVLV2_VAV*)(net->CtrlList[devIndex]))->SetPIDSetpoint(i,params[0]);
                                }
                                else
                                {
                                    commandExecuted = ((CNTHVLV2*)(net->CtrlList[devIndex]))->SetPIDSetpoint(i,params[0]);
                                }
                            }
                        }

                    }
                }
            }
//             else if ((toEveryOne) || toNet)
//             {
//                 commandExecuted = true;
//                 for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
//                 {
//                     net = m_Net->GetNetHandler( netIndex );
            //
//                     if (m_Net->NetHasTempDevices( netIndex ))
//                     {
//                         if (m_Net->AcquireNet( netIndex ))
//                         {
//                             for (ctrlIt = net->CtrlList.begin(); ctrlIt!=net->CtrlList.end(); ctrlIt++)
//                             {
//                                 if ((*ctrlIt)->GetControllerType() == DEV_ALARMCTRL)
//                                 {
//                                     ( (AlarmCoordinator*)(*ctrlIt) )->SetZoneAlarmState(zoneIndex, newState);
//                                 }
//                             }
//                         }
//                     }
//                 }
//             }

        };break;
        case COMM_SETZONESTATE: {
            T_Net *net;
            vector<CString>aeroConfigVector;
            int zoneIndex = 0;
            bool turnOn = false;

            zoneIndex = m_XMLParser.GetIntParam("ZONE");

            turnOn = m_XMLParser.GetBoolParam("ON");

            if (toAddress)
            {
                if (m_Net->CheckControllerType( netIndex, devIndex, DEV_FLOORCOORD_2) )
                {
                    //Get handler to the NET
                    net = m_Net->GetNetHandler( netIndex );

                    if (m_Net->AcquireNet( netIndex ))
                    {
                        commandExecuted = true;

                        //Se ricevo un zoneIndex == -1 significa tutte le zone
                        if (zoneIndex == -1)
                        {
                            commandExecuted = true;

                            ((CFloorCoord2*)(net->CtrlList[devIndex]))->ChangeZoneOnOffState(zoneIndex,turnOn);
                        }
                        else {

                            commandExecuted = true;
                            t_FloorZone* floorZone = ((CFloorCoord2*)(net->CtrlList[devIndex]))->GetZoneNumberPtr(zoneIndex);

                            //Controllo che la zona esista
                            if (floorZone != 0x0)
                            {
                                ((CFloorCoord2*)(net->CtrlList[devIndex]))->ChangeZoneOnOffState(zoneIndex,turnOn);


                            }
                        }
                    }
                }
                else
                {
                    commandExecuted = false;
                }
            }
        };break;
        case COMM_ACTIVATECONTROLLER:
        {
            T_Net *net;
            bool activate = false;

            activate = m_XMLParser.GetBoolParam("ACTIVATE");

            if (toAddress)
            {
                net = m_Net->GetNetHandler( netIndex );
                
                net->CtrlList[devIndex]->ActivateController(activate);
                
                commandExecuted = true;
            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;
                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    net = m_Net->GetNetHandler( netIndex );
            
                    net->CtrlList[devIndex]->ActivateController(activate);
                }
            }
        };break;
        case COMM_GETSPONTANEOUSDATA:
        {
            T_Net* net;
            commandExecuted = true;
            vector<CVController*>::iterator ctrlIt;
            
            if (toAddress)
            {
                CString message;
                
                net = m_Net->GetNetHandler( netIndex );
                
                //Tratto diversamente solo i controllori che mandano più di un messaggio
                if (m_Net->CheckControllerType( netIndex, devIndex, DEV_FLOORCOORD_2) )
                {
                    int nOfZones = ((CFloorCoord2*)net->CtrlList[devIndex])->GetNumberZone();
                    for (int zone = 0; zone < nOfZones; zone++)
                    {
                        message = net->CtrlList[devIndex]->GetSpontaneousData(zone);
                        if (CheckInterfacePortsForConnection())
                        {
                            WriteOnInterfacePorts(message.c_str(), message.size());
                        }
                    }
                }
                else if (m_Net->CheckControllerType( netIndex, devIndex, DEV_ALARMCTRL) )
                {
                    //TODO da implementare
                    message = net->CtrlList[devIndex]->GetSpontaneousData();
                    WriteOnInterfacePorts(message.c_str(), message.size());
                }
                else
                {
                    message = net->CtrlList[devIndex]->GetSpontaneousData();
                    WriteOnInterfacePorts(message.c_str(), message.size());
                }

            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;
                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    CString message;
                
                    net = m_Net->GetNetHandler( netIndex );
                
                    for (ctrlIt = net->CtrlList.begin(); ctrlIt!=net->CtrlList.end(); ctrlIt++)
                    {
                        //Tratto diversamente solo i controllori che mandano più di un messaggio
                        if ( (*ctrlIt)->GetControllerType() == DEV_FLOORCOORD_2 )
                        {
                            int nOfZones = ((CFloorCoord2*)(*ctrlIt))->GetNumberZone();
                            for (int zone = 0; zone < nOfZones; zone++)
                            {
                                message = (*ctrlIt)->GetSpontaneousData(zone);
                                if (CheckInterfacePortsForConnection())
                                {
                                    WriteOnInterfacePorts(message.c_str(), message.size());
                                }
                            }
                        }
                        else if (m_Net->CheckControllerType( netIndex, devIndex, DEV_ALARMCTRL) )
                        {
                        //TODO da implementare
                            message = (*ctrlIt)->GetSpontaneousData();
                            WriteOnInterfacePorts(message.c_str(), message.size());
                        }
                        else
                        {
                            message = (*ctrlIt)->GetSpontaneousData();
                            WriteOnInterfacePorts(message.c_str(), message.size());
                        }
                    }
                    
                    
                }
            }
            
        };break;
        case COMM_ENABLEFARNETO:
        {
            T_Net *net;
            bool activate = false;

            activate = m_XMLParser.GetBoolParam("ENABLE");

            if (toAddress)
            {
                net = m_Net->GetNetHandler( netIndex );
                
                if (m_Net->CheckControllerType( netIndex, devIndex, DEV_CNT) )
                {
                    ((CNTH_CNT*)(net->CtrlList[devIndex]))->SetEnabled(activate);
                
                    commandExecuted = true;
                }
            }
            else if ((toEveryOne) || toNet)
            {
                //TODO da implementare
//                 commandExecuted = true;
//                 for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
//                 {
//                     net = m_Net->GetNetHandler( netIndex );
//             
//                     net->CtrlList[devIndex]->ActivateController(activate);
//                 }
            }
        };break;
        case COMM_TURNON:
        {
            T_Net *net;
            bool turnOn = false;
            vector<CVController*>::iterator ctrlIt;

            turnOn = m_XMLParser.GetBoolParam("TURNON");

            if (toAddress)
            {
                net = m_Net->GetNetHandler( netIndex );
                
                if ((net != 0x0 ) && (m_Net->AcquireNet(netIndex)))
                {
                    net->CtrlList[devIndex]->TurnOn(turnOn);
                
                    commandExecuted = true;
                }
            }
            else if ((toEveryOne) || toNet)
            {
                commandExecuted = true;
                for (netIndex = startIndex; netIndex < stopIndex; netIndex++)
                {
                    net = m_Net->GetNetHandler( netIndex );
                    
                    if (m_Net->AcquireNet(netIndex))
                    {   
                        for (ctrlIt = net->CtrlList.begin(); ctrlIt!=net->CtrlList.end(); ctrlIt++)
                        {     
                            (*ctrlIt)->TurnOn(turnOn);
                        }
                    }
                }
            }
        };break;
        case COMM_NUMTOT:{
            //25/08/2008 -- sopprimo l'errore se il comando non ha indirizzo valido perchè nel caso hotel se ho più cervelletti i comandi sono mandati a tutti e
            //non si capisce la risposta
            suppressReturnMessage = true;
        };break;
        default:
            break;
    }//Switch

    if (!suppressReturnMessage)
    {
        messContent = this->m_XMLParser.AcknowledgeCommand( commandStrings[command], commandExecuted);
        UpdateServerPorts(true, false);
        WriteOnInterfacePorts( messContent.c_str(), messContent.size() );
    }

    //exit
    return commandExecuted;

}
