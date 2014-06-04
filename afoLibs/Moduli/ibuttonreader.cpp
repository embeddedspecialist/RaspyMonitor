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
 #include "ibuttonreader.h"
#include "conewirenet.h"

CIButtonReader::CIButtonReader(const char *configString, CTimer *timer): CVCoordinator(configString)
{
    if (configString != 0x0)
    {
        m_IniLib.GetConfigParamInt(configString,"GREENLIGHT",&m_GreenLightAddr,-1);
        m_IniLib.GetConfigParamInt(configString,"REDLIGHT",&m_RedLightAddr,-1);
    }
    
    m_LightControlEnabled = false;
    
    //Set controller type
    m_ControllerType = DEV_IBUTT_RDR;
}


CIButtonReader::~CIButtonReader()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool CIButtonReader::Update(bool updateData)
{
    //Cerco gli iButton nella NET a cui appartengo... anche se, di fatto, se ho un'altra NET con la stessa porta non li so distinguere
    bool retVal = false;
    uchar iButtonSN[10][8];
    int numDevFound = 0;
    T_Net *net;
    COneWireNet* netPtr = reinterpret_cast<COneWireNet*> ( m_NetPtr );

    net = netPtr->GetNetHandler(this->m_NetNumber-1);

    if ( (net != 0x0) && (net->isAcquired))
    {
        numDevFound = net->master->FindDevices(net->portHandler, &iButtonSN[0], DS1990A_FN, 9);
    
        //Li trasformo in stringhe:
        m_SerialNumberVector.clear();
        for (int i = 0 ; i < numDevFound; i++)
        {
            CString tempString;
            char buffer[32];
            ConvertSN2Str(buffer,&(iButtonSN[i][0]));
            tempString = buffer;
            m_SerialNumberVector.push_back(tempString);
        }
        
        retVal = true;
    }

    return retVal;
}

bool CIButtonReader::ConnectControllers()
{
    COneWireNet* netPtr = reinterpret_cast<COneWireNet*> ( m_NetPtr );
    int netIndex, ctrlIndex;
    
    //Esco subito perchè non ho il puntatore alla NET
    if ( netPtr == 0x0 )
    {
        cout << "Attenzione il coordinatore di indirizzo: " << m_Address << " NON è connesso al conewireNET"<<endl;
        msDelay(2000);
        return false;
    }
    
    if ((m_GreenLightAddr < 0) || (m_RedLightAddr < 0))
    {
        cout << "Attenzione il coordinatore di indirizzo: "<<m_Address << " NON ha il controllo luce abilitato"<<endl;
        msDelay(2000);
        return true; //Non è proprio un errore
    }
    
    netIndex = netPtr->GetNetByMemoryAddress(m_GreenLightAddr);
    ctrlIndex = netPtr->GetDeviceIndexByMemoryAddress(netIndex, m_GreenLightAddr);

    if ( (netIndex<0) || (ctrlIndex < 0) || (!netPtr->CheckControllerType(netIndex, ctrlIndex, DEV_DIDO)) )
    {
        cout << "Attenzione: coordinatore di indirizzo: " << m_Address<< " campo GREENLIGHT NON valido"<<endl;
        msDelay(1000);
        m_InitOk = true;
        return true;
    }
    
    m_GreenLight = (CDigitalIO*)(netPtr->GetNetHandler ( netIndex )->CtrlList[ctrlIndex]);
    
    netIndex = netPtr->GetNetByMemoryAddress(m_RedLightAddr);
    ctrlIndex = netPtr->GetDeviceIndexByMemoryAddress(netIndex, m_RedLightAddr);

    if ( (netIndex<0) || (ctrlIndex < 0) || (!netPtr->CheckControllerType(netIndex, ctrlIndex, DEV_DIDO)) )
    {
        cout << "Attenzione: coordinatore di indirizzo: " << m_Address<< " campo REDLIGHT NON valido"<<endl;
        msDelay(1000);
        m_InitOk = true;
        return true;
    }
    
    m_RedLight = (CDigitalIO*)(netPtr->GetNetHandler ( netIndex )->CtrlList[ctrlIndex]);
    
    m_LightControlEnabled = true;
    m_InitOk = true;
    
    return true;
    
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CIButtonReader::SetLight(bool greenLight, bool turnOn)
{
    CDigitalIO* lightPtr;
    if (!m_LightControlEnabled)
    {
        return;
    }
    
    //Le spengo entrambe
    m_GreenLight->SetState(false, true);
    m_RedLight->SetState(false, true);
    if (greenLight)
    {
        lightPtr = m_GreenLight;
    }
    else
    {
        lightPtr = m_RedLight;
    }
    
    lightPtr->SetState(turnOn, false);
}


