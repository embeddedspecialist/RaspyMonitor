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
 #include "digitalctrl.h"
#include "conewirenet.h"


DigitalCtrl::DigitalCtrl(const char* configString, CTimer* timer): CVMultiDIDO(configString, timer)
{
//     CLibIniFile configReader;
//     
//     m_IsInitOK = false;
//     m_ControllerType = DEV_DIGITAL;
//     m_IsInitOK = false;
// 
//     m_IsTimedDO = false;
//     m_TimeOutMilliSec = 0;
//     m_HasInput = false;
//     m_HasOutput = false;
// 
//     m_GetInputFromActivity = false;
// 
//     if (configString != 0x0)
//     {
//         configReader.GetConfigParamInt(configString,"TIMEOUT",&m_TimeOutMilliSec,-1);
//         if (m_TimeOutSec > 0)
//         {
//             m_IsTimedDO = true;
//         }
// 
//         configReader.GetConfigParamInt(configString,"ACTIVITY",&m_GetInputFromActivity,false);
// 
//     }
}


DigitalCtrl::~DigitalCtrl()
{
}
//////////////////////////////////////////////
//              ConnectController
//////////////////////////////////////////////
bool DigitalCtrl::ConnectController(const char* configString, void* netHandler)
{
    bool retVal = true;
    int netIndex, ctrlIndex, devIndex;
    COneWireNet* netPtr = reinterpret_cast<COneWireNet*> ( netHandler );
    CLibIniFile configReader;
    CVDevice *device;

    //Get Handle to input device
//     configReader.GetConfigParamInt( configString, "INPUT", &devIndex, -1);
// 
//     //Get the NET where this object belongs
//     netIndex = m_NetNumber-1;
//     
//     devIndex = netPtr->GetDeviceIndexByConfigNumber( netIndex, devIndex);
// 
//     if (devIndex > -1)
//     {
//         device = netPtr->m_NetList[netIndex].deviceList[devIndex];
// 
//         if ( (inputIndex == -1) || (device == 0x0) || ( !((device->GetFamNum() != DS2408_FN) || (inDevice->GetFamNum() != DS2405_FN) ) ) )
//         {
//             //TODO: messaggio di errore
//             cout << "Errore!! L'input specificato nel DigitalCtrl: " << m_NetNumber<<"-"<<m_DeviceNumber<<" NON è valido"<<endl;
//             retval = false;
//         }
//         else
//         {
//             SetInputDevice(device);
//             m_HasInput = true;
// 
//         }
//     }
//     else
//     {
//         //it does not have informations about inputs
//         m_HasInput = false;
//     }
// 
//     //Get Handle to output device
//     configReader.GetConfigParamInt( configString.c_str(), "OUTPUT", &devIndex, -1);
//     
//     devIndex = netPtr->GetDeviceIndexByConfigNumber( netIndex, devIndex);
// 
//     if (devIndex > -1)
//     {
//         device = netPtr->m_NetList[netIndex].deviceList[devIndex];
// 
//         if ( (inputIndex == -1) || (inDevice == 0x0) || ( !((inDevice->GetFamNum() != DS2408_FN) || (inDevice->GetFamNum() != DS2405_FN) ) ) )
//         {
//             //TODO messaggio di errore
//             cout << "Errore!! L'output specificato nel DigitalCtrl: " << m_NetNumber<<"-"<<m_DeviceNumber<<" NON è valido"<<endl;
//             retval = false;
//         }
//         else
//         {
//             SetOutputDevice(device);
//             m_HasOutput = true;
//         }
//     }
//     else
//     {
//         //m_IsRemoted è impostato dal costruttore della vmultidido
//         if (m_IsRemoted)
//         {
//             m_HasOutput = true;
//         }
//         else
//         {
//             //it does not have information about outputs
//             m_HasOutput = false;
//         }
//     }
// 
//     if ( (!m_HasInput) && (!m_HasOutput))
//     {
//         //TODO messaggio di errore
//         cout << "Errore!! Nessun I/O specificato nel DigitalCtrl: " << m_NetNumber<<"-"<<m_DeviceNumber<<endl;
//         retVal = false;
//     }

    return retVal;
    
    
}

