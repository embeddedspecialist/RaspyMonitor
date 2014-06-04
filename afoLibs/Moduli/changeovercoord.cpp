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
#include "changeovercoord.h"
#include "conewireengine.h"
#include "conewirenet.h"
#include "digitalio.h"

ChangeOverCoord::ChangeOverCoord(const char* configString, CTimer *timer): CVCoordinator(configString)
{
    if ( configString != 0x0 )
    {
    }

    m_EnginePtr = 0x0;
    m_NetPtr = 0x0;
    m_InitOk = false;
    m_AllStopped = false;
    m_IsChangeOverStarted = false;

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
    m_ControllerType = DEV_CHOVER;
}

////////////////////////////////////////////////////////////////////////////////
ChangeOverCoord::~ChangeOverCoord()
{
}

////////////////////////////////////////////////////////////////////////////////
bool ChangeOverCoord::Update(bool updateData)
{
    time_t actTime;

    //Get actual time
    time(&actTime);

    if (!m_InitOk)
    {
        InitChangeOver();
    }
    if (m_InputForSummer != -1)
    {
        int stateSummer = m_InSummer -> GetState(false);
        if ( stateSummer != m_IsSummer )
        {
            m_IsSummer = stateSummer;
            m_StartChangeOver = true;
        }
    }

    if (m_StartChangeOver)
    {
        if (!m_AllStopped)
        {
            m_AllStopped = StopAll();
        }
        else if (!m_IsChangeOverStarted)
        {
            if (Changeover())
            {
                m_ChangeOverStartTime = actTime;
                m_IsChangeOverStarted = true;
            }
        }
        else if (m_IsChangeOverStarted)
        {
            if (actTime > m_ChangeOverStartTime + m_ChangeOverTime)
            {
                if (StartAll())
                {
                    //Finito!!
                    m_StartChangeOver = false;
                    m_IsChangeOverStarted = false;
                    m_AllStopped = false;

                    //Salvo l'impostazione estate/inverno
                    CIniFileHandler iniFileReader;
                    CString configString;

                    //Leggo dal file di configurazione (changeover.ini) tutte le chiavi e le zone e le istanzio, programmo i moduli di controllo accessi
                    if ( !iniFileReader.Load ( "./changeover.ini" ) )
                    {
                        cout << "Attenzione: impossibile aprire il file changeover.ini"<<endl;
                        return false;
                    }

                    //Salvo l' impostazione estate
                    iniFileReader.SetBool("Summer",m_IsSummer,"","COMMON");

                    iniFileReader.Save();
                }
            }
        }
    }

    //Verifico se devo mandare dei comandi
    if ( m_OldSummer != m_IsSummer )
    {
        SendCommands();
        m_OldSummer = m_IsSummer;
    }
    
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////
bool ChangeOverCoord::ConnectControllers()
{
    int netIndex, ctrlIndex;
    CString deviceName;

    CIniFileHandler iniFileReader;
    COneWireNet* netPtr = reinterpret_cast<COneWireNet*> ( m_NetPtr );

    CString configString;

    //Esco subito perchè non ho il puntatore alla NET
    if ( netPtr == 0x0 )
    {
        cout << "Attenzione il coordinatore di indirizzo: " << m_Address << " NON è connesso al conewireNET"<<endl;
        return false;
    }

    //Leggo dal file di configurazione (changeover.ini) tutte le chiavi e le zone e le istanzio, programmo i moduli di controllo accessi
    if ( !iniFileReader.Load ( "./changeover.ini" ) )
    {
        cout << "Attenzione: impossibile aprire il file changeover.ini"<<endl;
        return false;
    }

    //Carico le impostazioni comuni
    m_IsSummer = iniFileReader.GetBool("Summer","COMMON",false);
    m_OldSummer = !(m_IsSummer);
    m_ChangeOverTime = iniFileReader.GetInt("ChangeoverTime","COMMON",3600);

    m_InputForSummer = iniFileReader.GetInt("InputForSummer","COMMON",-1);

    if ( m_InputForSummer!=-1 )
    {
        netIndex = netPtr->GetNetByMemoryAddress ( m_InputForSummer );
        ctrlIndex = netPtr->GetDeviceIndexByMemoryAddress ( netIndex, m_InputForSummer );
        if ( (netIndex!=-1) && (ctrlIndex!=-1) )
        {
            m_InSummer = ( CDigitalIO* ) ( ( netPtr->GetNetHandler ( netIndex ) )->CtrlList[ctrlIndex] );
            if ( ((m_InSummer)->GetControllerType() == DEV_DIDO) && ((m_InSummer)->IsInput()) )
            {
                cout << "Ingresso per la commutazione E / I correttamento collegato!!!" << endl;
            }
            else
            {
                cout << "Attenzione: L'ingresso per la commutazione E / I non è valido!!!" << endl;
                m_InputForSummer=-1;
            }
        }
        else
        {
            cout << "Attenzione: L'ingresso per la commutazione E / I non è valido!!!" << endl;
            m_InputForSummer=-1;
        }
    }
    else
    {
        cout << "Attenzione: Non sono stati impostati ingressi per la commutazione E / I nel file changeover.ini" << endl;
    }

    //Carico eventuali comandi esterni
    m_NumOfCommands = iniFileReader.GetInt ( "nOfCommands", "COMMAND", 0 );
    
    for (int i = 0; i < m_NumOfCommands; i++)
        {
            deviceName = "Command";
            deviceName+=(i+1);
            
            configString = iniFileReader.GetString ( deviceName, "COMMAND", "" );
            
            if (configString.size() != 0)
            {
                m_IniLib.GetConfigParamString(configString.c_str(),"TYPE",&(m_Commands[i].type), "");
                m_IniLib.GetConfigParamString(configString.c_str(),"FIELD",&(m_Commands[i].field), "");
                m_IniLib.GetConfigParamString(configString.c_str(),"VALUE",&(m_Commands[i].value), "");
                m_IniLib.GetConfigParamInt(configString.c_str(),"ADDR",&(m_Commands[i].address), -1);
            }
            else
            {
                cout<<"ATTENZIONE: AlarmCoordinator -- manca comando numero "<<i+1<<endl;
                sleep(2);
            }
        }
    
    
    //Carico le impostazioni delle uscite
    if (!ConnectIO(OUTPUT, &iniFileReader))
    {
        return false;
    }

    //Carico le impostazioni per i dispositivi del cambio stagionale
    if (!ConnectIO(CHOVER, &iniFileReader))
    {
        return false;
    }

    return true;
}
//////////////////////////////////////////////////////////////////////////////////////
bool ChangeOverCoord::ConnectIO(e_IOType ioType,  CIniFileHandler* iniFileReader)
{
    COneWireNet* netPtr = reinterpret_cast<COneWireNet*> ( m_NetPtr );
    int netIndex, ctrlIndex;
    int nOfItems = 0, itemIndex;
    t_ActiveControllers newController;
    CString sectionName, deviceName;
    CString configString, seasonString, keyName;
    int devAddr;

    newController.ctrlType = ioType;
    
    switch (ioType)
    {
        case OUTPUT:
        {
            sectionName = "OUTPUTS";
            keyName = "ADDR";

        };break;
        case CHOVER:
        {
            sectionName = "CHOVDEVS";
            keyName = "OUT";


        };break;
    };

    nOfItems = iniFileReader->GetInt("nOfDev",sectionName,0);

    for (itemIndex = 1; itemIndex <= nOfItems; itemIndex++)
    {
        deviceName = "Device";
        if (itemIndex < 10)
        {
            deviceName+="0";
        }

        deviceName+=itemIndex;

        configString = iniFileReader->GetString ( deviceName, sectionName, "" );

        if ( configString.size() == 0 )
        {
            cout << "ATTENZIONE: manca il " << deviceName << ", sezione " << sectionName <<", nel file changeover.ini\nContinuo..."<<endl;
            sleep ( 5 );
            continue;
        }

        m_IniLib.GetConfigParamInt ( configString.c_str(), keyName.c_str(), &devAddr, -1 );

        if ( devAddr < 0 )
        {
            cout << "ATTENZIONE: manca "<<keyName<<" nel " << deviceName << ", sezione " << sectionName <<", nel file changeover.ini\nContinuo..."<<endl;
            sleep ( 5 );
            continue;
        }

        
        //Carico se è remotato
        m_IniLib.GetConfigParamBool(configString.c_str(), "REMOTE", &(newController.isRemote),false);
        
        if (newController.isRemote)
        {
            //IL campo address mi serve solo come remoto
            newController.remoteAddress = devAddr;
            m_IniLib.GetConfigParamBool(configString.c_str(), "ISDIDO", &(newController.isRemoteDido),false);
        }
        else
        {
            //Prendo i riferimenti del dispositivo
            netIndex = netPtr->GetNetByMemoryAddress ( devAddr );
            ctrlIndex = netPtr->GetDeviceIndexByMemoryAddress ( netIndex, devAddr );
    
            if ( ( netIndex <0 ) || ( ctrlIndex < 0 ) )
            {
                cout << "ATTENZIONE: L'ingresso " << deviceName << ", sezione " << sectionName <<", nel file changeover.ini è errato!!\nContinuo..."<<endl;
                sleep ( 5 );
                continue;
            }
    
            //TODO da inserire il controllo se il controller è un ingresso o un'uscita
            newController.outHandler = netPtr->GetNetHandler ( netIndex )->CtrlList[ctrlIndex];
    
            if (ioType == CHOVER)
            {
                //Carico anche l'ingresso
                m_IniLib.GetConfigParamInt ( configString.c_str(), "IN", &devAddr, -1 );
    
                if ( devAddr < 0 )
                {
                    cout << "ATTENZIONE: manca il campo \"IN\" nel " << deviceName << ", sezione " << sectionName <<", nel file changeover.ini\nContinuo..."<<endl;
                    sleep ( 5 );
                    continue;
                }
    
                //Prendo i riferimenti del dispositivo
                netIndex = netPtr->GetNetByMemoryAddress ( devAddr );
                ctrlIndex = netPtr->GetDeviceIndexByMemoryAddress ( netIndex, devAddr );
    
                if ( ( netIndex <0 ) || ( ctrlIndex < 0 ) )
                {
                    cout << "ATTENZIONE: il campo IN di " << deviceName << ", sezione " << sectionName <<", nel file changeover.ini è errato!!\nContinuo..."<<endl;
                    sleep ( 5 );
                    continue;
                }
    
                //TODO da inserire il controllo se il controller è un ingresso o un'uscita
                newController.inHandler = netPtr->GetNetHandler ( netIndex )->CtrlList[ctrlIndex];
                
                //Carico l'impostazione estiva
                m_IniLib.GetConfigParamInt ( configString.c_str(), "SUMMERSET", &(newController.summerSet), 1 );
            }
        }

        m_IniLib.GetConfigParamString(configString.c_str(), "ACTIVE", &seasonString, "ever");

        if (!strcasecmp(seasonString.c_str(),"winter"))
        {
            newController.activeSeason = WINTER;
        }
        else if (!strcasecmp(seasonString.c_str(),"summer"))
        {
            newController.activeSeason = SUMMER;
        }
        else
        {
            newController.activeSeason = EVER;
        }

        m_ActiveControllers.push_back(newController);
    }

    return true;
}
//////////////////////////////////////////////////////////////////////////////////////
bool ChangeOverCoord::InitChangeOver()
{
    //Spengo le uscite
    if (StopAll())
    {
        if (Changeover())
        {
            if (StartAll())
            {
                m_StartChangeOver = false;
                m_IsChangeOverStarted = false;
                m_AllStopped = false;
                m_InitOk = true;
                cout << "Modulo changeover inizializzato" << endl;
            }
            else
            {
                //TBI -- errore
                cout << "Impossibile riavviare i dispositivi del changeover"<<endl;
                return false;
            }
        }
        else
        {
            //TBI -- errore
            cout << "Impossibile eseguire il changeover"<<endl;
            return false;
        }
    }
    else
    {
        //TBI -- errore
        cout << "Impossibile fermare i dispositivi del changeover"<<endl;
        return false;
    }

    return true;
}


/*!
    \fn ChangeOverCoord::StopAll()
 */
bool ChangeOverCoord::StopAll()
{
    unsigned int ctrlIndex = 0;
    CVController* controller;
    COneWireEngine* engPtr = reinterpret_cast<COneWireEngine*>(m_EnginePtr);

    //Spengo le uscite
    for (ctrlIndex = 0; ctrlIndex < m_ActiveControllers.size(); ctrlIndex++)
    {
        //Prendo l'handler
        controller = m_ActiveControllers[ctrlIndex].outHandler;

        if (m_ActiveControllers[ctrlIndex].ctrlType == OUTPUT)
        {
            cout << "ChangeOver address: "<< m_Address << " -- Fermo il dispositivo di address:"<<controller->GetMemoryAddress()<<endl;
            
            if (m_ActiveControllers[ctrlIndex].isRemote)
            {
                Cmd *com = new Cmd("DEVICE");
                com->putValue("COMMAND","ActivateController");
                com->putValue("ADDRESS",CString("")+m_ActiveControllers[ctrlIndex].remoteAddress);
                com->putValue("ACTIVATE","0");
                
                engPtr->WriteOnOutputPorts(com->getXMLValue(), m_ActiveControllers[ctrlIndex].remoteAddress,-1);
                
                delete com;
                
                //Ho il problema del timer: non so se nel remoto c'e' o meno il timer
                if (m_ActiveControllers[ctrlIndex].isRemoteDido)
                {
                    com = new Cmd("DEVICE");
                    com->putValue("ADDRESS",CString("")+m_ActiveControllers[ctrlIndex].remoteAddress);
                    com->putValue("COMMAND","SetDigitalOutput");
                    com->putValue("STATE","0");
                    engPtr->WriteOnOutputPorts(com->getXMLValue(), m_ActiveControllers[ctrlIndex].remoteAddress,-1);
                    delete com;
                }
            }
            else
            {            
                //Disabilito il timer
                controller->UseTimer(false);
    
                //Disabilito il controller
                controller->ActivateController(false);
                
                //Se è un dido devo spegnere l'uscita
                if (controller->GetControllerType() == DEV_DIDO)
                {
                    if ( ! ((CDigitalIO*)(controller))->SetState(0) )
                    {
                        //Se c'è un errore interrompo la procedura
                        //TBI -- messaggio di errore
                        cout << "ChangeOver address: "<< m_Address << " -- Errore nel fermare il dispositivo di address:"<<controller->GetMemoryAddress()<<endl;
                        return false;
                    }
                }
                else if ((controller->GetControllerType() == DEV_PIDSIMPLE) ||
                        (controller->GetControllerType() == DEV_PIDLMD))
                {
                    
                    //Dovrei chiudere gli AO e resettare i PID
                    ((CVPID*)(controller))->InitPID();
    
                    if (!((CVPID*)(controller))->WritePIDOutput(255))
                    {
                        //Se c'è un errore interrompo la procedura
                        //TBI -- messaggio di errore
                        return false;
                    }
                }
                else if (controller->GetControllerType() == DEV_AFOVLV)
                {
                    ((CNTHVLV2*)(controller))->TurnOnModule(3, false);
                }
            }
        }
    }

    return true;
}


/*!
    \fn ChangeOverCoord::StartAll()
 */
bool ChangeOverCoord::StartAll()
{
    unsigned int ctrlIndex = 0;
    CVController* controller;
    COneWireNet* netPtr = reinterpret_cast<COneWireNet*> ( m_NetPtr );
    COneWireEngine* engPtr = reinterpret_cast<COneWireEngine*> ( m_EnginePtr );

    //Esco subito perchè non ho il puntatore alla NET
    if ( netPtr == 0x0 )
    {
        cout << "Attenzione il coordinatore di indirizzo: " << m_Address << " NON è connesso al conewireNET"<<endl;
        return false;
    }

    for (ctrlIndex = 0; ctrlIndex < m_ActiveControllers.size(); ctrlIndex++)
    {
        //Prendo l'handler
        controller = m_ActiveControllers[ctrlIndex].outHandler;

        if (m_ActiveControllers[ctrlIndex].ctrlType == OUTPUT)
        {
            if ( ((m_ActiveControllers[ctrlIndex].activeSeason==SUMMER) && m_IsSummer) ||
                 ((m_ActiveControllers[ctrlIndex].activeSeason==WINTER) && (!m_IsSummer)) ||
                  (m_ActiveControllers[ctrlIndex].activeSeason==EVER)
               )
            {
                
                //Controllo se remotato o meno
                if (m_ActiveControllers[ctrlIndex].isRemote)
                {
                    cout << "ChangeOver address: "<< m_Address << " -- Attivo il dispositivo REMOTO di address:"<<m_ActiveControllers[ctrlIndex].remoteAddress<<endl;
                    
                    //Riattivo il dispositivo
                    Cmd *com = new Cmd("DEVICE");
                    com->putValue("ADDRESS",CString("")+m_ActiveControllers[ctrlIndex].remoteAddress);
                    com->putValue("COMMAND","ActivateController");
                    com->putValue("ACTIVATE","1");
                    engPtr->WriteOnOutputPorts(com->getXMLValue(), m_ActiveControllers[ctrlIndex].remoteAddress, -1);
                    delete com;
                    
                    com = new Cmd("DEVICE");
                    com->putValue("ADDRESS",CString("")+m_ActiveControllers[ctrlIndex].remoteAddress);
                    
                    if (controller->GetControllerType() == DEV_DIDO)
                    {    
                        com->putValue("COMMAND","SetDigitalOutput");
                        com->putValue("STATE","1");
                    }
                    else 
                    {
                        com->putValue("COMMAND","SetSummer");
                        com->putValue("SUMMER",CString("")+(int)(m_IsSummer));
                    }
                    
                    engPtr->WriteOnOutputPorts(com->getXMLValue(), m_ActiveControllers[ctrlIndex].remoteAddress,-1);
                    delete com;
                }
                else
                {
                    cout << "ChangeOver address: "<< m_Address << " -- Attivo il dispositivo di address:"<<controller->GetMemoryAddress()<<endl;
                    //Abilito il timer se e' il caso
                    if (controller->GetTimerID() > 0)
                    {
                        controller->UseTimer(true);
                    }

                    //Abilito il controller
                    controller->ActivateController(true);
                    if ((controller->GetControllerType() == DEV_PIDSIMPLE) || (controller->GetControllerType() == DEV_PIDLMD))
                    {
                        netPtr->SetSummer(m_IsSummer, controller->GetMemoryAddress());
                    }
                    else if (controller->GetControllerType() == DEV_FULLUTACTRL)
                    {
                        ((CFullUTACtrl*)(controller))->SetSummer(m_IsSummer);
                    }
                    else if (controller->GetControllerType() == DEV_FULLUTACTRL_2)
                    {
                        ((CFullUTACtrl2*)(controller))->SetSummer(m_IsSummer);
                    }
                    else if (controller->GetControllerType() == DEV_DIDO)
                    {
                        if ( ! ((CDigitalIO*)(controller))->SetState(1) )
                        {
                            //Se c'è un errore interrompo la procedura
                            cout << "ChangeOver address: "<< m_Address << " -- Errore nell'avviare il dispositivo di address:"<<controller->GetMemoryAddress()<<endl;
                            return false;
                        }
                    }
                    else if (controller->GetControllerType() == DEV_AFOVLV)
                    {
                        ((CNTHVLV2*)(controller))->SetSummer(m_IsSummer);
                    }
                    else if (controller->GetControllerType() == DEV_VLVCTRL)
                    {
    #ifdef USE_ADV_VLV  
                        ((CNTHVLV_ADV*)(controller))->SetSummer(m_IsSummer);
    #else
                        ((CNTHVLV*)(controller))->SetSummer(m_IsSummer);
    #endif
                    }
                    else if (controller->GetControllerType() == DEV_C3POINT)
                    {
                        ((C3PointCtrl*)(controller))->SetSummer(m_IsSummer);
                    }
                }
            }
        }
    }

    return true;
}


/*!
    \fn ChangeOverCoord::Changeover()
 */
bool ChangeOverCoord::Changeover()
{
    unsigned int ctrlIndex = 0;
    CDigitalIO* outController;
    CDigitalIO* inController;
    int summerSet;

    for (ctrlIndex = 0; ctrlIndex< m_ActiveControllers.size(); ctrlIndex++)
    {
        if (m_ActiveControllers[ctrlIndex].ctrlType == CHOVER)
        {
            //Prendo gli handler
            outController = (CDigitalIO*)(m_ActiveControllers[ctrlIndex].outHandler);
            inController = (CDigitalIO*)(m_ActiveControllers[ctrlIndex].inHandler);
            summerSet = m_ActiveControllers[ctrlIndex].summerSet;

            //Controllo lo stato degli ingressi a seconda della impostazione summer e della stagione
            if (m_IsSummer)
            {
                if (inController->GetState(true) != summerSet)
                {
                    cout << "ChangeOver address: "<< m_Address << " -- Cambio dispositivo per changeover di address:"<<outController->GetMemoryAddress()<<endl;
                    //L'impostazione estate è diversa da quella del dispositivo, esegui il cambiamento
                    if (!outController->ChangeOutput())
                    {
                        cout << "ChangeOver address: "<< m_Address << " -- Errore nel change over del dispositivo di address:"<<outController->GetMemoryAddress()<<endl;
                        return false;
                    }
                }
            }
            else
            {
                if (inController->GetState(true) != (!summerSet))
                {
                    cout << "ChangeOver address: "<< m_Address << " -- Cambio dispositivo per changeover di address:"<<outController->GetMemoryAddress()<<endl;
                    //L'impostazione estate è diversa da quella del dispositivo, esegui il cambiamento
                    if (!outController->ChangeOutput())
                    {
                        cout << "ChangeOver address: "<< m_Address << " -- Errore nel change over del dispositivo di address:"<<outController->GetMemoryAddress()<<endl;
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

///////////////////////////////////////////////////////
// Invio Comandi al cambio dello stato di m_IsSummer //
///////////////////////////////////////////////////////
void ChangeOverCoord::SendCommands()
{
    for (int i=0 ; i < m_NumOfCommands ; i++)
    {
        COneWireEngine* engPtr = reinterpret_cast<COneWireEngine*> ( m_EnginePtr );

        Cmd com("DEVICE");
        com.putValue("COMMAND",m_Commands[i].type);

        if (m_Commands[i].value.size())
            {
              com.putValue(m_Commands[i].field, m_Commands[i].value);
            }
            else
            {
              com.putValue(m_Commands[i].field,m_IsSummer);
            }

        //Controllo l'indirizzo
        if (m_Commands[i].address > -1)
        {
          com.putValue("ADDRESS",m_Commands[i].address);
        }

        engPtr->WriteOnOutputPorts(com.toString());
    }
}
