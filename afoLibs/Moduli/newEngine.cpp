/***************************************************************************
 *   Copyright (C) 2009 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it                                           *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 *************************************************************************/
 //Questo file è un'estensione della COneWireEngine che contiene nuove funzioni di gestione dei driver, controller e coordinator
#include "conewireengine.h"

bool COneWireEngine::UpdateTempDrivers(unsigned int netIndex)
{
    T_Net *net;
    net = m_Net->GetNetHandler(netIndex);

    if (!net->tempConversionLaunched)
    {
        //Invalido i dati
        for (unsigned int i = 0; i < net->deviceList.size(); i++)
        {
            if ( (net->deviceList[i]->GetDeviceType() == DEV_DS18B20) || 
                  (net->deviceList[i]->GetDeviceType() == DEV_DS18S20) )
            {
                net->deviceList[i]->InvalidateData();
            }
        }
        
        //Update all the temps
        if (!m_Net->UpdateAllTemp( netIndex, CONVERT_T))
        {
            m_AfoErrorHandler.PushError( AFOERROR_UNABLE_TO_UPDATE_ALL_TEMP, netIndex+1);

            return false;
        }
        else
        {
            net->tempConversionLaunched = true;
            mainClockManager.ResetClock(net->temperatureClock);
            return true;
        }
    }
    else if (mainClockManager.GetTimeElapsed(net->temperatureClock,false) < TEMP_CONVERSION_TIME_MS/1000.0)
    {
        //Per sicurezza (?)
        if (mainClockManager.GetTimeElapsed(net->temperatureClock,false) < 0)
        {
            //E' successo qualcosa di strano: l'ora di convrsione è minore di quella attuale
            net->tempConversionLaunched=false;
        }
        return true;
    }

    net->tempConversionLaunched = false;
    
    for (unsigned int i = 0; i < net->deviceList.size(); i++)
    {
        if ( (net->deviceList[i]->GetDeviceType() == DEV_DS18B20) || 
              (net->deviceList[i]->GetDeviceType() == DEV_DS18S20) )
        {
            ((CDS18X20*)(net->deviceList[i]))->ReadTemperature(false);

            if (i%10 == 0){
                CheckForCommands2();
            }
        }
    }

    //Qui devo rilanciare la conversione per non perdere dei cicli di aggiornamento
    //Update all the temps
    if (!m_Net->UpdateAllTemp( netIndex, CONVERT_T))
    {
        
        m_AfoErrorHandler.PushError( AFOERROR_UNABLE_TO_UPDATE_ALL_TEMP, netIndex+1);

        return false;
    }
    else
    {
        net->tempConversionLaunched = true;
        mainClockManager.ResetClock(net->temperatureClock);
        return true;
    }
    
    return true;
}
//////////////////////////////////////////////////////////////////////////////
bool COneWireEngine::UpdateDIDODrivers(unsigned int netIndex)
{
    vector<CVDevice*>::iterator deviceIt;
    bool digitalOutPolled = false;
    unsigned int counter = 0;
    
    T_Net *net;
    unsigned long int actTime = msGettick();
    net = m_Net->GetNetHandler(netIndex);

    //Qui aggiorno tutti i moduli
    for (deviceIt = net->deviceList.begin(); deviceIt < net->deviceList.end(); deviceIt++)
    {
        if ((*deviceIt)->GetDeviceType() == DEV_DS2408)
        {
            counter++;
            if ( (((CDS2408*)(*deviceIt))->m_ModuleType == 0) || (((CDS2408*)(*deviceIt))->m_ModuleType == 2) )
            {
                ((CDS2408*)(*deviceIt))->UpdateData();
            }
            else if (mainClockManager.GetTimeElapsed(net->didoClock,false) > DIGITAL_OUT_CHECK_INTERVAL/1000.0)
            {
                ((CDS2408*)(*deviceIt))->UpdateData();
                digitalOutPolled = true;
            }

            //Controllo i comandi ogni tanto
            if (counter%10 == 0){
                CheckForCommands2();
            }
        }
    }

    if (digitalOutPolled == true)
    {
        mainClockManager.ResetClock(net->didoClock);
    }

    return true;
}
//////////////////////////////////////////////////////////////////////////////
bool COneWireEngine::UpdateAIDrivers(unsigned int netIndex)
{
    bool retVal = true;
    T_Net *net;
    unsigned int counter=0;

    net = m_Net->GetNetHandler(netIndex);
    
    if (net == 0x0)
    {
        return false;
    }
    
    for (unsigned int i = 0; i < net->deviceList.size(); i++)
    {
        if (net->deviceList[i]->GetDeviceType() == DEV_DS2438)
        {
            ((CDS2438*)(net->deviceList[i]))->ReadAllData();
            //Controllo i comandi ogni tanto
            if ((counter++)%10){
                CheckForCommands2();
            }
        }
    }
    
    return true;
}
/////////////////////////////////////////////////////////////////////////////////
bool COneWireEngine::UpdateAODrivers(unsigned int netIndex)
{
    bool retVal = true;
    T_Net *net;
    unsigned int counter =0;

    net = m_Net->GetNetHandler(netIndex);
    
    if (net == 0x0)
    {
        return false;
    }
    
    for (unsigned int i = 0; i < net->deviceList.size(); i++)
    {
        if (net->deviceList[i]->GetDeviceType() == DEV_DS2890)
        {
            ((CDS2890*)(net->deviceList[i]))->UpdateState();

            //Controllo i comandi ogni tanto
            if ((counter++)%10){
                CheckForCommands2();
            }
        }
    }
    
    return true;
}
////////////////////////////////////////////////////////////////////////
void COneWireEngine::ControlSystem2()
{
    int netIndex;
    struct timeval tpStart, tpStop;
    T_Net* net = 0x0;
    vector<CVController*>::iterator ctrlIt;
    
    //Profiling del ciclo di controllo
    gettimeofday( &tpStart, 0x0 );

    if (m_DoDebug)
    {
        cout << endl << endl;
        cout.flush();
        cout << "*******************************************************"<<endl;
        cout << "***************** Aggiornamento Driver ****************"<<endl;
        cout << "*******************************************************"<<endl;
    }
    //Aggiorno i driver di tutte le NET
    for (netIndex = 0; netIndex < m_TotalNets; netIndex++)
    {
        if (m_DoDebug)
        {
            cout << "+++++++++++ NET"<<netIndex+1<<"++++++++++"<<endl;
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

        //Aggiorno ds18x20
        UpdateTempDrivers(netIndex);

        CheckForCommands2( );

        //Aggiorno ds2408
        UpdateDIDODrivers(netIndex);

        CheckForCommands2( );

        //Aggiorno ds2438
        UpdateAIDrivers(netIndex);

        CheckForCommands2(  );

        //Aggiorno ds2890
        UpdateAODrivers(netIndex);

    }

    if (m_DoDebug)
    {
        cout << endl << endl;
        cout.flush();
        cout << "*******************************************************"<<endl;
        cout << "*************** Aggiornamento Controller **************"<<endl;
        cout << "*******************************************************"<<endl;
    }

    CheckForCommands2();

    //Aggiorno i controller di tutte le NET
    for (netIndex = 0; netIndex < m_TotalNets; netIndex++)
    {
        CheckForCommands2( );

        if (m_DoDebug)
        {
            cout << "+++++++++++ NET"<<netIndex+1<<" ++++++++++"<<endl;
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

        net = m_Net->GetNetHandler(netIndex);

        unsigned int counter = 0;
        for (ctrlIt = net->CtrlList.begin(); ctrlIt < net->CtrlList.end(); ctrlIt++)
        {
            if (!(*ctrlIt)->IsCoordinator())
            {
                (*ctrlIt)->Update2(false);

                if ((counter++)%10 == 0){
                    CheckForCommands2();
                }
            }


            //Aggiorno DIDO -- OK

            //Aggiorno AI -- OK

            //Aggiorno AO -- OK

            //Aggiorno PID -- OK

            //Aggiorno UTA_CTRL

            //Aggiorno DI2AO

            //Queste non seguono lo schema classico driver-controller: le aggiorno come sempre
            //Aggiorno VLV varie tipologie


            //Aggiorno MGC

        }

        //TODO da finire con le altri classi, per ora metto questo per continuare a gestire
        ManageAdvancedController(netIndex);
    }

    //Aggiorno i coordinatori
    if (m_DoDebug)
    {
        cout << endl << endl;
        cout.flush();
        cout << "************************************************************"<<endl;
        cout << "************* Aggiornamento Coordinatori *******************"<<endl;
        cout << "************************************************************"<<endl;
    }

    for (netIndex = 0; netIndex < m_TotalNets; netIndex++)
    {
        CheckForCommands2();

        if (m_DoDebug)
        {
            cout << "+++++++++++ NET"<<netIndex+1<<"++++++++++"<<endl;
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

        //TODO da rivedere i coordinatori nella nuova ottica, soprattutto con i nuovi PID
        ManageCoordinators(netIndex);
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
