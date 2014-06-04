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
#include "fullutactrl2.h"
#include "conewireengine.h"
#include "conewirenet.h"
    
static const char* Uta_Config_Strings_2[] =
{
    //PID params
    "PIDDirect",
    "PIDLimitMax",
    "PIDLimitMin",
    "PIDHumidity",
    "PIDFreeCooling",
    "PIDAirQuality",
    "PIDPost",

//Sensors
    "TempMNDSens",
    "TempRIPSens",
    "TempExtSens",
    "HumiditySens",
    "VOCSens",
    "CO2Sens",
    "SummerSwitch",
    "ONOFFSwitch",
    "TempRegulator",

//Actuators
    "HeatBattery",
    "ColdBattery",
    "Humidifier",
    "POST1",
    "POST2",
    "FANMND",
    "FANRIP",
    "MAINSHUTT",
    "RECSHUTT",
    "HumidityMachine",
    "HeatPump",
    "ColdPump",
            

//SetPoints
    "SummerTempSP",
    "WinterTempSP",
    "SummerHumSP",
    "WinterHumSP",
    "POSTHYST",
    "SHUTTDELAY",
    "TempLimMax",
    "TempLimMin",

//Main Logic
    "UseFreeCooling",
    "UseAirQuality",
    "Summer"
            
};


CFullUTACtrl2::CFullUTACtrl2(const char *configString, CTimer *timer): CVCoordinator(configString)
{
    //Allocate positions for the controllers
    m_ControllerList.resize(FULLUTA_TOTPARAMETERS,0x0);

    m_AreShuttersOpen = false;
    m_TimeOfShutterOpening = 0;
    m_ShutterDelay = 60;
    m_PostHyst = 5.0;
    m_HumHyst = 5.0;
    m_WinterSetPoint = 20.0;
    m_SummerSetPoint = 26.0;
    m_WinterHumSP = 45.0;
    m_SummerHumSP = 55.0;
    m_TempRegType = 1;
    m_IsSummer = true;
    m_TempMnd.floatData[0] = TEMP_ERRVAL;
    m_TempMnd.isValid = false;
    m_TempRip.floatData[0] = TEMP_ERRVAL;
    m_TempRip.isValid = false;
    m_MainPIDOutputVolt = 0.0;
    m_MainPIDOutput = 255;
    m_TempSetPoint.floatData[0] = TEMP_ERRVAL;
    m_TempSetPoint.isValid = false;
    m_IsSummer = false;
    m_Humidity.floatData[0] = ANALOG_ERRVAL;
    m_Humidity.isValid = false;

    m_UseFreeCooling = false;
    m_UseAirQuality = false;
    m_TimerID = -1;
    
    m_POST1_On = false;
    m_POST2_On = false;

    if (configString != 0x0)
    {
        m_IniLib.GetConfigParamInt( configString, "CONFIGID", &m_ConfigID, -1);
        m_IniLib.GetConfigParamInt( configString, "TIMERID", &m_TimerID, -1);
    }

    if ( (m_TimerID > 0) && (timer != 0x0) )
    {
        m_Timer = timer;
        m_UseTimer = true;
    }
    else
    {
        m_UseTimer = false;
    }

    m_TypeOfTimerVal = TIMERVAL_DIGITAL;
    m_ControllerType = DEV_FULLUTACTRL_2;

    //Variabile per fargli eseguire un comando
    m_CodeRevision = 1;
}


CFullUTACtrl2::~CFullUTACtrl2()
{
}
//////////////////////////////////////////////////////////////////////
//              Update
//////////////////////////////////////////////////////////////////////
bool CFullUTACtrl2::Update(bool updateData)
{
    bool retVal = false;
    //bool isUTAOn = false;
    time_t actTime;
    //Variabili che contengono i valori dei PID rigirati in scala da 0 a 255 anzichè da 255 a 0
    COneWireEngine* engPtr = reinterpret_cast<COneWireEngine*>(m_EnginePtr);

	
    time (&actTime);

    
    if (!m_InitOk)
    {
        InitData();
        m_InitOk = true;
    }

    if (m_DebugLevel)
    {
        cout << "Gestore UTA -- address: " << m_Address << " Aggiornamento..."<<endl;
    }

    //TODO set error management
    //First gather all data from the inputs
    if (AcquireTempSetPoint(&m_TempSetPoint))
    {
        m_WinterSetPoint = m_SummerSetPoint = m_TempSetPoint.floatData[0];
    }
    
    //Check for summer and setpoint
    CheckSummer();

    //Then check for timers
    if (m_UseTimer && IsTimerEnabled())
    {
        m_IsOn = GetValFromTimer();
    }
    else
    {
        GetInput((int)FULLUTA_ONOFF,&m_IsOn);
    }

    //Metto qui la disabilitazione del controllo in modo che se lo disabilito si spegne tutto automaticamente
    if (!m_IsActive)
    {
        if (engPtr->CheckInterfacePortsForConnection())
        {
            Cmd com("DEVICE");
            com.putValue("TYPE","FullUTACtrl");
            com.putValue("ADDRESS",m_Address);
            com.putValue("OUTPUT",m_MainPIDOutputVolt);
            com.putValue("TEMPMND",m_TempMnd.floatData[0]);
            com.putValue("TEMPRIP",m_TempRip.floatData[0]);
            com.putValue("TEMPEXT",m_TempExt.floatData[0]);
            com.putValue("SETPOINT",m_TempSetPoint.floatData[0]);
            com.putValue("HUM", m_Humidity.floatData[0]);
            com.putValue("SUMMER",m_IsSummer);
            com.putValue("ISACTIVE",m_IsActive);
            com.putValue("ISON",m_IsOn);
        
        
            engPtr->WriteOnInterfacePorts(com.toString().c_str(), com.toString().size());
        }
        //Se non è attiva esco senza fare niente
        return true;
    }
    
    //Gather temperatures
    GetInput(FULLUTA_TEMPMND, &m_TempMnd);
    GetInput(FULLUTA_TEMPRIP, &m_TempRip);
    GetInput(FULLUTA_HUMIDITY,&m_Humidity);
    GetInput(FULLUTA_TEMPEXT,&m_TempExt);

    //Now execute everything
    if (m_IsOn)
    {
        if (!m_AreShuttersOpen)
        {
            ShuttersControl();
        }

        if (actTime < m_TimeOfShutterOpening + m_ShutterDelay)
        {
            //The open shutter command is given but we have to wait a while before the shutters open
            return true;
        }
        else
        {
            //Main logic function
            //Turn On FANs
            WriteOutput((int)FULLUTA_FANMND, 1);
            WriteOutput((int)FULLUTA_FANRIP, 1);

            //Adjust shutters
            ShuttersControl();

            if (m_IsSummer)
            {
                //Spengo umidificatore
                ManageHumdifier(false);
                //Accendo pompa del freddo
                WriteOutput(FULLUTA_COLDPUMP,1);
                //Close the hot battery
                WriteOutput((int)FULLUTA_HEATBAT, (float)0.0);
                
                if (m_Humidity.floatData[0] > m_SummerHumSP + m_HumHyst)
                {
                    //Spalanco la valvola del freddo e regolo con il POST
                    WriteOutput(FULLUTA_COLDBAT, (float)10.0);
                    m_MainPIDOutputVolt = 10.0;
                    
                    ManagePOST(true);
                }
                else if (m_Humidity.floatData[0] < m_SummerHumSP + m_HumHyst)
                {
                    ManageMainPID();
                    
                    if (m_TempExt.isValid)
                    {
                        if ((m_PIDVector[FULLUTA_PID_DIRECT].GetPIDOutput() == 0.0) && (m_TempExt.floatData[0]  < m_TempSetPoint.floatData[0] ))
                        {
                            ManagePOST(true);
                        }
                        else
                        {
                            ManagePOST(false);
                        }
                    }
                    else
                    {
                        ManagePOST(false);
                    }
                }
            }
            else //INVERNO
            {
                //Chiudo Batteria freddo
                WriteOutput((int)FULLUTA_COLDBAT, (float)0.0);
                
                //Spengo la pompa freddo e accendo la pompa caldo
                WriteOutput((int)FULLUTA_COLDPUMP,0);
                WriteOutput((int)FULLUTA_HEATPUMP,1);

                //Spengo i POST
                ManagePOST(false);

                //Se umidità sotto setpoint accendo umdificatore o apro valvola
                ManageHumdifier(true);

                //Separatamente controllo di temperatura con limiti
                ManageMainPID();

            }
        }
    }
    else
    {
        //TODO manca tutta la logica failsafe: es. se NON si spengono i ventilatori NON chiudo le serrande
        //Turn off everything:
        //Turn Off fans
        WriteOutput((int)FULLUTA_FANMND, 0);
        WriteOutput((int)FULLUTA_FANRIP, 0);
        //Turn off post heating
        WriteOutput((int)FULLUTA_POST1,0);
        WriteOutput((int)FULLUTA_POST2,0);
        
        WriteOutput((int)FULLUTA_HUMIDIFIER, (float)0.0);

        //Close shutters
        WriteOutput((int)FULLUTA_MAINSHUTT,(float)0.0);
        WriteOutput((int)FULLUTA_RECSHUTT,(float)0.0);

        //Close batteries
        WriteOutput((int)FULLUTA_HEATBAT,(float)0.0);
        WriteOutput((int)FULLUTA_COLDBAT,(float)0.0);
        WriteOutput((int)FULLUTA_HUMBAT,(float)0.0);

        //Shutdown pumps
        WriteOutput((int)FULLUTA_COLDPUMP,0);
        WriteOutput((int)FULLUTA_HEATPUMP,0);
        
        m_MainPIDOutput = 255;
        m_MainPIDOutputVolt = 0.0;

    }

    if (m_DebugLevel)
    {
        
        cout << "Gestore UTA -- address: " << m_Address<<" ";
        cout << " Setpoint:"<<m_TempSetPoint.floatData[0]<<" Rip:"<<m_TempRip.floatData[0]<<" Mnd:"<<m_TempMnd.floatData[0]<<" Hum:"<<m_Humidity.floatData[0]<<" Summer:"<<m_IsSummer<<" On:"<<m_IsOn<<" MainPID:"<<m_MainPIDOutput<<endl;
        cout.flush();
    }
    
    if (engPtr->CheckInterfacePortsForConnection())
    {
        Cmd com("DEVICE");
        com.putValue("TYPE","FullUTACtrl");
        com.putValue("ADDRESS",m_Address);
        com.putValue("OUTPUT",m_MainPIDOutputVolt);
        com.putValue("TEMPMND",m_TempMnd.floatData[0]);
        com.putValue("TEMPRIP",m_TempRip.floatData[0]);
        com.putValue("TEMPEXT",m_TempExt.floatData[0]);
        com.putValue("SETPOINT",m_TempSetPoint.floatData[0]);
        com.putValue("HUM", m_Humidity.floatData[0]);
        com.putValue("SUMMER",m_IsSummer);
        com.putValue("ISACTIVE",m_IsActive);
        com.putValue("ISON",m_IsOn);
        
        
        engPtr->WriteOnInterfacePorts(com.toString().c_str(), com.toString().size());
    }

    return retVal;
}
//////////////////////////////////////////////////////////////////////////////////////////    
bool CFullUTACtrl2::SetSetPoint ( float valueSP )
{
    m_TempSetPoint.floatData[0] = valueSP;
    m_SummerSetPoint = valueSP;
    m_WinterSetPoint = valueSP;
    
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////    
bool CFullUTACtrl2::InitData()
{
    //Parto sempre acceso
    m_IsOn=true; 
    for (int i = FULLUTA_PID_DIRECT; i < FULLUTA_TEMPMND; i++)
    {
        m_PIDVector[i].InitPID();
    }

    WriteOutput((int)FULLUTA_FANMND, 0);
    WriteOutput((int)FULLUTA_FANRIP, 0);
    //Turn off post heating
    WriteOutput((int)FULLUTA_POST1,0);
    WriteOutput((int)FULLUTA_POST2,0);

    WriteOutput((int)FULLUTA_HUMIDIFIER, (float)0.0);

    //Close shutters
    WriteOutput((int)FULLUTA_MAINSHUTT,(float)0.0);
    WriteOutput((int)FULLUTA_RECSHUTT,(float)0.0);

    //Close batteries
    WriteOutput((int)FULLUTA_HEATBAT,(float)0.0);
    WriteOutput((int)FULLUTA_COLDBAT,(float)0.0);
    WriteOutput((int)FULLUTA_HUMBAT,(float)0.0);

    //Shutdown pumps
    WriteOutput((int)FULLUTA_COLDPUMP,0);
    WriteOutput((int)FULLUTA_HEATPUMP,0);
    
    return true;
}
////////////////////////////////////////////////////////////////////////////////
bool CFullUTACtrl2::CheckSummer()
{

    GetInput((int)FULLUTA_SUMMERSWITCH, &m_IsSummer);
    
    if (m_IsSummer)
    {
        m_TempSetPoint.floatData[0] = m_SummerSetPoint;
        SetPIDSetpoint((int)FULLUTA_PID_DIRECT,m_SummerSetPoint);
        SetPIDSetpoint((int)FULLUTA_PID_HUMD,m_SummerHumSP);
        SetPIDSetpoint((int)FULLUTA_PID_FREECOOLING, m_SummerSetPoint);
    }
    else
    {
        m_TempSetPoint.floatData[0] = m_WinterSetPoint;
        SetPIDSetpoint((int)FULLUTA_PID_DIRECT,m_WinterSetPoint);
        SetPIDSetpoint((int)FULLUTA_PID_FREECOOLING,m_WinterSetPoint);
        SetPIDSetpoint((int)FULLUTA_PID_HUMD,m_WinterHumSP);
    }
    
    m_PIDVector[FULLUTA_PID_DIRECT].SetSummer(m_IsSummer);
    
    return true;
}
////////////////////////////////////////////////////////////////////////////////
bool CFullUTACtrl2::ConnectControllers()
{
    CIniFileHandler iniFileReader;
    COneWireNet* netPtr = reinterpret_cast<COneWireNet*> ( m_NetPtr );
    CString configIDString;
    int netIndex, ctrlIndex,ctrlAddress;

    //Esco subito perchè non ho il puntatore alla NET
    if ( netPtr == 0x0 )
    {
        cout << "Attenzione il coordinatore di indirizzo: " << m_Address << " NON è connesso al conewireNET"<<endl;
        return false;
    }

    //Leggo dal file di configurazione (utacoord2.ini) tutte le chiavi e le zone e le istanzio, programmo i moduli di controllo accessi
    if ( !iniFileReader.Load ( UTA2_INIFILE ) )
    {
        cout << "Attenzione: impossibile aprire il file UTACoord2.ini"<<endl;
        return false;
    }

    if (m_ConfigID <= 0)
    {
        cout << "Attenzione il coordinatore di indirizzo: " << m_Address << " NON ha il campo corretto in CONFIGID"<<endl;
        return false;
    }
    else
    {
        configIDString="UTA";
        configIDString+=m_ConfigID;

        if (!iniFileReader.ExistSection(configIDString))
        {
            cout << "Attenzione coordinatore di indirizzo: " << m_Address << " in UTACoord2.ini NON esiste la sezione"<<configIDString<<endl;
            return false;
        }   
    }
    
    //Carico le variabili e gli stati della logica principale
    m_UseFreeCooling = iniFileReader.GetBool(Uta_Config_Strings_2[FULLUTA_USEFREECOOLING], configIDString, false);
    m_FreeCoolingPercentage = iniFileReader.GetInt("FreeCoolingMinUsage", configIDString, 15);
    m_UseAirQuality = iniFileReader.GetBool(Uta_Config_Strings_2[FULLUTA_USEAIRQUALITY], configIDString, false);
    m_IsSummer = iniFileReader.GetBool(Uta_Config_Strings_2[FULLUTA_SUMMERSTATE],configIDString,false);
    
    //Carico i parametri dei PID
    for (unsigned int i = FULLUTA_PID_DIRECT; i < FULLUTA_TEMPMND; i++)
    {
        CString config,  pidKey;
        float params[3] = {32.0, 8.0, 0.0 };
        
        config = iniFileReader.GetString(Uta_Config_Strings_2[i],configIDString,"");
        
        if (config.size() != 0)
        {
            m_IniLib.GetConfigParamFloat(config.c_str(), "KP", &params[0], 32.0);
            m_IniLib.GetConfigParamFloat(config.c_str(), "Tint", &params[1], 8.0);
            m_IniLib.GetConfigParamFloat(config.c_str(), "Tder", &params[2], 0.0);
        }
        
        m_PIDVector[i].SetPIDParams(params);
    }
    
    //Carico i controllori
    for (int i = FULLUTA_TEMPMND; i < FULLUTA_SUMMERTEMPSP; i++)
    {
        ctrlAddress = iniFileReader.GetInt(Uta_Config_Strings_2[i],configIDString,-1);
        netIndex = netPtr->GetNetByMemoryAddress(ctrlAddress);
        ctrlIndex = netPtr->GetDeviceIndexByMemoryAddress(netIndex, ctrlAddress);

        if ( (netIndex<0) || (ctrlIndex < 0))
        {
            cout << "Attenzione: coordinatore di indirizzo: " << m_Address<< " campo "<<Uta_Config_Strings_2[i]<<" NON valido in UTAConfig.ini"<<endl;
            msDelay(1000);
            continue;
        }

        //Vedo se il regolatore e' nostro o itk
        if (i == FULLUTA_TEMPREG)
        {
            CString config;
            
            config = iniFileReader.GetString("TempRegType",configIDString,"NTH");

            if (!strcasecmp(config.c_str(),"NTH"))
            {
                m_TempRegType = 1;
            }
            else
            {
                m_TempRegType = 0;
            }
        }

        
        m_ControllerList[i] = netPtr->GetNetHandler ( netIndex )->CtrlList[ctrlIndex];
    }

    //Carico i setpoint
    m_SummerSetPoint = iniFileReader.GetFloat(Uta_Config_Strings_2[FULLUTA_SUMMERTEMPSP], configIDString, 26.0);
    m_WinterSetPoint = iniFileReader.GetFloat(Uta_Config_Strings_2[FULLUTA_WINTERTEMPSP], configIDString, 20.0);
    m_SummerHumSP = iniFileReader.GetFloat(Uta_Config_Strings_2[FULLUTA_SUMMERHUMSP], configIDString, 55.0);
    m_WinterHumSP = iniFileReader.GetFloat(Uta_Config_Strings_2[FULLUTA_WINTERHUMSP], configIDString, 45.0);
    m_PostHyst = iniFileReader.GetFloat(Uta_Config_Strings_2[FULLUTA_POSTHYST], configIDString, 5.0);
    m_ShutterDelay = iniFileReader.GetInt(Uta_Config_Strings_2[FULLUTA_SHUTTDELAY], configIDString, 30);
    float sp;
    sp = iniFileReader.GetFloat(Uta_Config_Strings_2[FULLUTA_TEMPLIMMAX], configIDString, 35.0);
    SetPIDSetpoint(FULLUTA_PID_LMMAX, sp);
    m_PIDVector[FULLUTA_PID_LMMAX].SetSummer(true);
    sp = iniFileReader.GetFloat(Uta_Config_Strings_2[FULLUTA_TEMPLIMMIN], configIDString, 15.0);
    SetPIDSetpoint(FULLUTA_PID_LMMIN, sp);
    m_PIDVector[FULLUTA_PID_LMMIN].SetSummer(false);
    m_PIDVector[FULLUTA_PID_HUMD].SetSummer(false);
    m_PIDVector[FULLUTA_PID_POST].SetSummer(false);


    return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CFullUTACtrl2::AcquireTempSetPoint(t_DataVal *setPoint)
{
    double angle;
    int currentRegister;
    
    if ((m_ControllerList[FULLUTA_TEMPREG]==0x0) || (setPoint == 0x0))
    {
        setPoint->isValid = false;
        return false;
    }

    if (m_TempRegType)
    {
        //Sonda NTH
        currentRegister = (int)((CAnalogIO*)(m_ControllerList.at(FULLUTA_TEMPREG)))->GetLastValue();

        if (currentRegister >= 0)
        {
            if (currentRegister > 950)
            {
                currentRegister = 950;
            }

            //Cerco di linearizzare l'andamento del potenziometro dividendo i dati in due fasce
            if (currentRegister < 180)
            {
                angle = -0.0108*currentRegister*currentRegister
                        + 4.6356*currentRegister
                        - 356.03;
            }
            else if (currentRegister > 205)
            {
                angle = -0.0002*currentRegister*currentRegister
                        + 0.4482*currentRegister
                        +66.74;
            }
            else
            {
                //Lascio una fascia di intermezzo per i 20 gradi
                angle = 135.0;
            }
                
            if (angle < 0.0)
            {
                angle = 0.0;
            }
            else if (angle > 270.0)
            {
                angle = 270.0;
            }
            
            setPoint->floatData[0] = angle * 20.0 / 270.0 + 10.0;
            setPoint->isValid = true;

            return true;
        }
    }
    else
    {
        float tempVolt;

        tempVolt = ((CAnalogIO*)(m_ControllerList[FULLUTA_TEMPREG]))->GetLastValue();

        setPoint->floatData[0] = (tempVolt*25.0)/10.0+5.0;
        setPoint->isValid = true;

        return true;
    }
        
    return false;
}

// ///////////////////////////////////////////////////////////////////
// //                  AirQualityControl
// ////////////////////////////////////////////////////////////////////
// bool CFullUTACtrl2::AirQualityControl()
// {
//     int co2PID, vocPID;
//     bool isCO2Present = false,isVOCPresent = false;
//     float voc = 0.0, co2 = 0.0;
//     int aqPIDOutput;
// 
//     if (m_ControllerList[FULLUTA_CO2PID] != 0x0)
//     {
//         isCO2Present = true;
//     }
// 
//     if (m_ControllerList[FULLUTA_VOCPID] != 0x0)
//     {
//         isVOCPresent = true;
//     }
// 
//     if ( (!isCO2Present) && (!isVOCPresent) )
//     {
//         //Non c'e' controllo della qualita' dell'aria
//         return true;
//     }
//     
//     //Sistemo le serrande: normalmente la serranda di ricircolo è aperta, se il pid della qualità mi
//     //dice diversamente la chiudo e apro l'aria esterna
//     
//     //Gather VOC and CO2
//     if (isVOCPresent)
//     {
//         voc = ((CAnalogIO*)(m_ControllerList.at(FULLUTA_VOC)))->GetLastValue();
//         if (voc >= 0)
//         {
//             ((CPIDSimple*)(m_ControllerList.at(FULLUTA_VOCPID)))->Update(voc);
//         }
//         
//         vocPID = ((CPIDSimple*)(m_ControllerList.at(FULLUTA_VOCPID)))->m_PIDOutput;
//     }
//     else
//     {
//         vocPID = 0;
//     }
// 
//     if (isCO2Present)
//     {
//         co2 = ((CAnalogIO*)(m_ControllerList.at(FULLUTA_CO2)))->GetLastValue();
// 
//         if (co2 >= 0)
//         {
//         //TODO forse qui va rallentato questo PID perchè la sonda è molto lenta
//  
//             ((CPIDSimple*)(m_ControllerList.at(FULLUTA_CO2PID)))->Update(co2);
//         }
// 
//         co2PID = ((CPIDSimple*)(m_ControllerList.at(FULLUTA_CO2PID)))->m_PIDOutput;
//     }
//     else
//     {
//         co2PID = 0;
//     }
//     
// 
//     if ( co2PID < vocPID)
//     {
//         aqPIDOutput = ((CPIDSimple*)(m_ControllerList.at(FULLUTA_CO2PID)))->m_PIDOutput;
//     }
//     else
//     {
//         aqPIDOutput = ((CPIDSimple*)(m_ControllerList.at(FULLUTA_VOCPID)))->m_PIDOutput;
//     }
// 
//     if (aqPIDOutput > 255)
//     {
//         aqPIDOutput = 0;
//     }
//     else if (aqPIDOutput < 0)
//     {
//         aqPIDOutput = 0;
//     }
// 
//     //Alla serranda di ricircolo mando il segnale del PID invertito (0, cioè tutto aperto, se va tutto bene) per tenerla aperta,
//     //alle serrande esterne mando il segnale invertito
//     ((CAnalogIO*)(m_ControllerList.at(FULLUTA_RECSHUTT)))->SetPosition(255 - aqPIDOutput);
//     ((CAnalogIO*)(m_ControllerList.at(FULLUTA_MAINSHUTT)))->SetPosition(aqPIDOutput);
// 
// 
//     return true;
// }
///////////////////////////////////////////////////////////////////////
bool CFullUTACtrl2::WriteOutput(int device, int value)
{
    bool retVal = false;
    /*
    FULLUTA_HEATBAT,    //AnalogIO Batteria calda analogica o monobatteria
    FULLUTA_COLDBAT,    //AnalogIO Batteria fredda
    FULLUTA_HUMBAT,     //AnalogIO umidificatore
    FULLUTA_POST1,      //DigitalIO Accensione post o AnalogIO per POST
    FULLUTA_POST2,      //DigitalIO seconda accensione POST
    FULLUTA_FANMND,     //DigitalIO ventilatori mandata
    FULLUTA_FANRIP,     //DigitalIO Ventilatori ripresa
    FULLUTA_MAINSHUTT,  //AnalogIO Serrande mandata e ripresa
    FULLUTA_RECSHUTT,   //AnalogIO Serranda di ricircolo
    FULLUTA_HUMIDIFIER, //DigitalIO Comando umidificatore
    FULLUTA_HEATPUMP,   //DigitalIO comando pompa caldo
    FULLUTA_COLDPUMP,   //DigitalIO comando pompa freddo
    */
    switch(device)
    {
        case FULLUTA_HEATBAT:
        case FULLUTA_COLDBAT:
        case FULLUTA_HUMBAT:
        {
            if (m_ControllerList[device] != 0x0)
            {
                ((CAnalogIO*)(m_ControllerList.at(device)))->SetPosition((uchar)value);
                retVal = true;
            }

            
            break;
        };
        
        case FULLUTA_POST2:
        case FULLUTA_FANMND:
        case FULLUTA_FANRIP:
        case FULLUTA_HUMIDIFIER:
        case FULLUTA_HEATPUMP:
        case FULLUTA_COLDPUMP:
        {
            if (m_ControllerList[device] != 0x0)
            {
                ((CDigitalIO*)(m_ControllerList.at(device)))->SetState((int)value);
                retVal = true;
            }

            
            break;
        };
        case FULLUTA_POST1:
        case FULLUTA_MAINSHUTT:
        case FULLUTA_RECSHUTT:
        {
            if (m_ControllerList[device] != 0x0)
            {
                //Controllo se le serrande sono ON/OFF
                if (m_ControllerList[device]->GetControllerType() == DEV_DIDO)
                {
                    ((CDigitalIO*)(m_ControllerList.at(device)))->SetState((int)value);
                    retVal = true;
                }
                else if (m_ControllerList[device]->GetControllerType() == DEV_AIAO)
                {
                    ((CAnalogIO*)(m_ControllerList.at(device)))->SetPosition((uchar)value);
                    retVal = true;
                }
            }

            
            break;
        }
    }

    return retVal;
}
/////////////////////////////////////////////////////////////////////////////////////////////////
//                  GetInput (float)
/////////////////////////////////////////////////////////////////////////////////////////////////
bool CFullUTACtrl2::GetInput(int device, t_DataVal * value)
{
    bool retVal = false;
    
    switch (device)
    {
        case FULLUTA_TEMPMND:
        case FULLUTA_TEMPRIP:
        case FULLUTA_TEMPEXT:
        {
            if (m_ControllerList[device] != 0x0)
            {
                
                value->floatData[0] = ((CTempCtrl*)(m_ControllerList.at(device)))->GetLastTemp();
                value->isValid = ((CTempCtrl*)(m_ControllerList.at(device)))->IsDataValid();
            }
            
            break;
        }
        case FULLUTA_HUMIDITY:
        {
            if (m_ControllerList[device] != 0x0)
            {
                value->floatData[0] = ((CAnalogIO*)(m_ControllerList.at(FULLUTA_HUMIDITY)))->GetLastValue()*10.0;
                value->isValid = true;
                retVal = true;
            }
            break;
        }
        case FULLUTA_VOC:
        case FULLUTA_CO2:
        {
            if (m_ControllerList[device] != 0x0)
            {
                ((CAnalogIO*)(m_ControllerList.at(device)))->GetLastValue();
                retVal = true;
            }
            break;
        }
    }

    return retVal;
        
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//                  GetInput (bool)
/////////////////////////////////////////////////////////////////////////////////////////////////
bool CFullUTACtrl2::GetInput(int device, bool * value)
{
    bool retVal = false;
    
    switch (device)
    {
        case FULLUTA_SUMMERSWITCH:
        case FULLUTA_ONOFF:
        {
            if (m_ControllerList[device] != 0x0)
            {
                //TODO da rivedere il comando perchè gli stati dovrebbero essere a posto dopo le modifiche di inizio ottobre
//                 *value = ((CVDIDO*)(m_ControllerList.at(device)))->GetChannelLevel(false);
                //TODO da testare
                *value = ((CDigitalIO*)(m_ControllerList.at(device)))->GetState(false);

                retVal = true;
            }
            break;
        }
    }

    return retVal;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//                  UpdatePID
/////////////////////////////////////////////////////////////////////////////////////////////////
bool CFullUTACtrl2::UpdatePID(int device, float value)
{ 
    m_PIDVector[device].UpdatePID(value);
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//                  SetPIDSetpoint
/////////////////////////////////////////////////////////////////////////////////////////////////
bool CFullUTACtrl2::SetPIDSetpoint(int device, float value)
{   
    m_PIDVector[device].SetSetPoint(value);

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//                  ShuttersControl
/////////////////////////////////////////////////////////////////////////////////////////////////
bool CFullUTACtrl2::ShuttersControl()
{
    //TODO se le serrande sono digitali presuppongo il ritorno a molla, altrimenti va implementato
    //TODO da vedere se il controllo va separato per serranda mandata e ripresa... raro ma possibile ? (Vd Mercatone)
    //TODO per ora gli algorimti free cooling e airquality sono separati: o va uno o va l'altro
    bool isRecShuttPresent = false;
    bool areShuttDigital = false;

    if (m_ControllerList[FULLUTA_MAINSHUTT] == 0x0)
    {
        //Direi che ho già finito... non c'è il controllo delle serrande principali
        m_AreShuttersOpen = true;
        return false;
    }
    else
    {
        if (m_ControllerList[FULLUTA_MAINSHUTT]->GetControllerType() == DEV_DIDO)
        {
            areShuttDigital = true;
        }
    }

    if (m_ControllerList[FULLUTA_RECSHUTT] == 0x0)
    {
        isRecShuttPresent = false;
    }
    else
    {
        isRecShuttPresent = true;
    }

    if (!m_AreShuttersOpen)
    {
        //Apro le serrande esterne e chiudo il ricircolo come caso generale in partenza
        if (areShuttDigital)
        {
            WriteOutput((int)FULLUTA_MAINSHUTT, 1);
            WriteOutput((int)FULLUTA_RECSHUTT, 0);
        }
        else
        {
            WriteOutput((int)FULLUTA_MAINSHUTT, 0);
            WriteOutput((int)FULLUTA_RECSHUTT, 255);
        }

        time(&m_TimeOfShutterOpening);
        m_AreShuttersOpen = true;

        return true;
    }
            
    //Se le serrande sono digitali al momento l'unico controllo che mi viene in mente è il free cooling, rimane come TODO
    if (m_UseFreeCooling)
    {
        //Algoritmo free cooling
        //Per il free cooling uso un algoritmo becero: se inverno e la temp Ext è maggiore del SP e la ripresa è inferiore al SP apro le serrande principali e chiudo il ricircolo
        //viceversa d'estate. In ricircolo, invece, tengo un 10% di serranda principale aperta per garantire il ricambio d'aria

        if ((!m_TempExt.isValid) || (!m_TempRip.isValid))
        {
            //Errore in letturra
            return true;
        }
                    
        float tempExt = m_TempExt.floatData[0];        
        float tempRip = m_TempRip.floatData[0];
        
        if (m_IsSummer)
        {
            if ((tempExt <= m_SummerSetPoint) &&
                (tempRip > m_SummerSetPoint)
               )
            {
                //Vado in free cooling
                WriteOutput(FULLUTA_MAINSHUTT, 0);
                WriteOutput(FULLUTA_RECSHUTT, 255);
            }
            else
            {
                //Tengo aperto al 10%
                WriteOutput(FULLUTA_MAINSHUTT, (int)(255 - (m_FreeCoolingPercentage/100.0)*255));
                WriteOutput(FULLUTA_RECSHUTT, (int)((m_FreeCoolingPercentage/100.0)*255));
            }
        }
        else
        {
            if ((tempExt >= m_WinterSetPoint) &&
                (tempRip < m_WinterSetPoint)
               )
            {
                //Vado in free cooling
                WriteOutput(FULLUTA_MAINSHUTT, 0);
                WriteOutput(FULLUTA_RECSHUTT, 255);
            }
            else
            {
                //Tengo aperto al 10%
                WriteOutput(FULLUTA_MAINSHUTT, (int)(255 - (m_FreeCoolingPercentage/100.0)*255));
                WriteOutput(FULLUTA_RECSHUTT, (int)((m_FreeCoolingPercentage/100.0)*255));
            }
        }
    }
    else if ((m_UseAirQuality) && (!areShuttDigital))
    {
        AirQualityControl();
    }

    return true;

}
/////////////////////////////////////////////////////
bool CFullUTACtrl2::SetSummer(bool isSummer)
{
    CIniFileHandler iniFileReader;
    CString configIDString;

    m_IsSummer = isSummer;
    
    m_PIDVector[FULLUTA_PID_DIRECT].SetSummer(m_IsSummer);
    m_PIDVector[FULLUTA_PID_FREECOOLING].SetSummer(m_IsSummer);
    
    if (!iniFileReader.Load ( UTA2_INIFILE ))
    {
        return false;
    }
    
    if (m_ConfigID <= 0)
    {
        return false;
    }
    else
    {
        configIDString="UTA";
        configIDString+=m_ConfigID;

        if (!iniFileReader.ExistSection(configIDString))
        {
            return false;
        }
    }

    iniFileReader.SetBool(Uta_Config_Strings_2[FULLUTA_SUMMERSTATE],isSummer,"",configIDString);

    iniFileReader.Save();

    return true;
}
////////////////////////////////////////////////////////////////////////////////////////
void CFullUTACtrl2::ManageHumdifier(bool turnOn)
{
    if (turnOn)
    {
        WriteOutput(FULLUTA_HUMIDIFIER, 1);
        
        if (IS_CTRL(m_ControllerList[FULLUTA_HUMBAT]))
        {
            float output = m_PIDVector[FULLUTA_PID_HUMD].UpdatePID(m_Humidity.floatData[0])/100.0;
            
            WriteOutput(FULLUTA_HUMBAT, (float)output);
        }
    }
    else
    {
        WriteOutput(FULLUTA_HUMIDIFIER,0);
        WriteOutput(FULLUTA_HUMBAT,(float)(0.0));
        
    }
}
/////////////////////////////////////////////////////////////////////////////////////////
void CFullUTACtrl2::ManagePOST(bool turnOn)
{
    if ( (!IS_CTRL(m_ControllerList[FULLUTA_POST1])) || (!m_TempMnd.isValid))
    {
        return;
    }
    
    if (turnOn)
    {
        //Devo accendere la pompa del caldo se ce l'ho
        if (IS_CTRL(m_ControllerList[FULLUTA_HEATPUMP]))
        {
            WriteOutput(FULLUTA_HEATPUMP, 1);
        }
        
        //qui devo controllare se il POST è elettrico o PID
        if (m_ControllerList[FULLUTA_POST1]->GetControllerType() == DEV_AIAO)
        {
            //E' un analogico -> aggiorno il PID
            m_PIDVector[FULLUTA_PID_POST].SetSummer(false);
            m_PIDVector[FULLUTA_PID_POST].SetSetPoint(m_TempSetPoint.floatData[0]);
            //Divido per 10 per ottenere l'uscita in volt.
            float outputVolt = m_PIDVector[FULLUTA_PID_POST].UpdatePID(m_TempMnd.floatData[0])/100.0;
            
            //Scrivo sull'uscita il valore in volt del PID.
            WriteOutput(FULLUTA_POST1, outputVolt);
        }
        else if (m_ControllerList[FULLUTA_POST1]->GetControllerType() == DEV_DIDO)
        {
            //Lavoro sull'isteresi...
            if (m_TempMnd.floatData[0] < m_TempSetPoint.floatData[0] - m_PostHyst/2)
            {
                m_POST1_On = true;
            }
            else if (m_TempMnd.floatData[0] > m_TempSetPoint.floatData[0] + m_PostHyst/2)
            {
                m_POST2_On = false;
            }
            
            if (m_TempMnd.floatData[0] < m_TempSetPoint.floatData[0] - m_PostHyst)
            {
                m_POST2_On = true;
            }
            else if (m_TempMnd.floatData[0] > m_TempSetPoint.floatData[0] + m_PostHyst)
            {
                m_POST2_On = false;
            }
            
            WriteOutput(FULLUTA_POST1, (int)m_POST1_On);
            WriteOutput(FULLUTA_POST2, (int)m_POST2_On);
            
        }
    }
    else
    {
        WriteOutput(FULLUTA_HEATPUMP, 0);
        
        if (m_ControllerList[FULLUTA_POST1]->GetControllerType() == DEV_AIAO)
        {
            WriteOutput(FULLUTA_POST1, (float)0.0);
        }
        else if (m_ControllerList[FULLUTA_POST1]->GetControllerType() == DEV_DIDO)
        {
            WriteOutput(FULLUTA_POST1, 0);
            WriteOutput(FULLUTA_POST2, 0);
        }
            
        
    } 
}
/////////////////////////////////////////////////////////////////////////////////////////
void CFullUTACtrl2::ManageMainPID()
{
    /*
    FULLUTA_PID_DIRECT,
    FULLUTA_PID_LMMAX,
    FULLUTA_PID_LMMIN,
    */
    if ( (!m_TempRip.isValid) || (!m_TempMnd.isValid))
    {
        //Todo da mettere un errore ?
        return;
    }
    
    //Calcolo i 3 PID separatamente
    float pidRip = m_PIDVector[FULLUTA_PID_DIRECT].UpdatePID(m_TempRip.floatData[0] );
    float pidMax = m_PIDVector[FULLUTA_PID_LMMAX].UpdatePID(m_TempMnd.floatData[0] );
    float pidMin = m_PIDVector[FULLUTA_PID_LMMIN].UpdatePID(m_TempMnd.floatData[0] );

    //Se estate e supero il limite superiore devo aprire, se inverno e supero il limite inferiore devo aprire
    if (m_IsSummer)
    {
        m_MainPIDOutput = pidRip - pidMin + pidMax;
    }
    else
    {
        m_MainPIDOutput = pidRip + pidMin - pidMax;
    }
    
    if (m_MainPIDOutput < 0.0)
    {
        m_MainPIDOutput = 0.0;
    }
    else if (m_MainPIDOutput > 1000.0)
    {
        m_MainPIDOutput = 1000.0;
    }
    
    m_MainPIDOutputVolt = m_MainPIDOutput/100.0;
    
    if (m_IsSummer)
    {
        if (IS_CTRL(m_ControllerList[FULLUTA_COLDBAT]))
        {
            WriteOutput(FULLUTA_COLDBAT, m_MainPIDOutputVolt);
        }
        else
        {
            WriteOutput(FULLUTA_HEATBAT, m_MainPIDOutputVolt);
        }   
    }
    else
    {
        WriteOutput(FULLUTA_HEATBAT, m_MainPIDOutputVolt);
    }
    
}
/////////////////////////////////////////////////////////////////////////////////////////
bool CFullUTACtrl2::WriteOutput(int device, float value)
{
    bool retVal = false;
    /*
    FULLUTA_HEATBAT,    //AnalogIO Batteria calda analogica o monobatteria
    FULLUTA_COLDBAT,    //AnalogIO Batteria fredda
    FULLUTA_HUMBAT,     //AnalogIO umidificatore
    FULLUTA_POST1,      //DigitalIO Accensione post o AnalogIO per POST
    FULLUTA_POST2,      //DigitalIO seconda accensione POST
    FULLUTA_FANMND,     //DigitalIO ventilatori mandata
    FULLUTA_FANRIP,     //DigitalIO Ventilatori ripresa
    FULLUTA_MAINSHUTT,  //AnalogIO Serrande mandata e ripresa
    FULLUTA_RECSHUTT,   //AnalogIO Serranda di ricircolo
    FULLUTA_HUMIDIFIER, //DigitalIO Comando umidificatore
    FULLUTA_HEATPUMP,   //DigitalIO comando pompa caldo
    FULLUTA_COLDPUMP,   //DigitalIO comando pompa freddo
    */
    switch(device)
    {
        case FULLUTA_HEATBAT:
        case FULLUTA_COLDBAT:
        case FULLUTA_HUMBAT:
        {
            if (m_ControllerList[device] != 0x0)
            {
                ((CAnalogIO*)(m_ControllerList.at(device)))->SetVOutput(value);
                retVal = true;
            }
            break;
        };
        case FULLUTA_POST1:
        case FULLUTA_MAINSHUTT:
        case FULLUTA_RECSHUTT:
        {
            if (m_ControllerList[device] != 0x0)
            {
                //Controllo se le serrande sono ON/OFF o analogiche
                if (m_ControllerList[device]->GetControllerType() == DEV_AIAO)
                {
                    ((CAnalogIO*)(m_ControllerList.at(device)))->SetVOutput(value);
                    retVal = true;
                }
                else if (m_ControllerList[device]->GetControllerType() == DEV_DIDO)
                {
                    ((CDigitalIO*)(m_ControllerList.at(device)))->SetState((int)value);
                }
            }            
            break;
        }
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////
bool CFullUTACtrl2::ExecCommand(CXMLUtil* xmlUtil)
{
    bool retVal = false;
    e_CommandTypes command = (e_CommandTypes)ParseCommand(xmlUtil);
    COneWireEngine *engPtr = reinterpret_cast<COneWireEngine*>(m_EnginePtr);
    CIniFileHandler iniFile;

    switch (command)
    {
        case COMM_SETPIDPARAM:
        {
            unsigned int pidNumber = xmlUtil->GetIntParam("PIDNUMBER");
            float params[3] = {0.0, 0.0, 0.0};

            if (pidNumber > FULLUTA_PID_POST)
            {
                break;
            }

            if ( xmlUtil->GetFloatParam("KAPPA", &params[0]) &&
                 xmlUtil->GetFloatParam("TINT", &params[1]) &&
                 xmlUtil->GetFloatParam("TDER", &params[2])
               )
            {
                m_PIDVector[pidNumber].SetPIDParams(params);
                if (iniFile.Load(UTA2_INIFILE))
                {
                    CString section = "UTA";
                    section += m_ConfigID;
                    //KP:32.0,Tint:8.0,Tder:0.0
                    CString configLine = "KP:";
                    configLine += params[0];
                    configLine += ",Tint:";
                    configLine += params[1];
                    configLine +=",Tder:";
                    configLine += params[2];
                    CString key = Uta_Config_Strings_2[pidNumber];

                    if (iniFile.SetValue(key,configLine,"",section))
                    {
                        iniFile.Save();
                    }
                }

                if (xmlUtil->ExistsParam("SETPOINT"))
                {
                    float setpoint = xmlUtil->GetFloatParam("SETPOINT");
                    m_PIDVector[pidNumber].SetSetPoint(setpoint);
                    bool doSave =  true;
                    CString section = "UTA";
                    section += m_ConfigID;
                    CString key;
                    switch (pidNumber)
                    {
                        case FULLUTA_PID_LMMAX:
                        {
                            key = Uta_Config_Strings_2[FULLUTA_TEMPLIMMAX];
                        };break;
                        case FULLUTA_PID_LMMIN:
                        {
                            key = Uta_Config_Strings_2[FULLUTA_TEMPLIMMIN];
                        };break;
                        case FULLUTA_PID_HUMD:
                        {
                            if (m_IsSummer)
                            {
                                key = Uta_Config_Strings_2[FULLUTA_SUMMERHUMSP];
                                m_SummerHumSP = setpoint;
                            }
                            else
                            {
                                key = Uta_Config_Strings_2[FULLUTA_WINTERHUMSP];
                                m_WinterHumSP = setpoint;
                            }
                        };break;
                        case FULLUTA_PID_AIRQUALITY:
                        {
                            key = Uta_Config_Strings_2[FULLUTA_CO2];
                        };break;
                        default:
                        {
                            doSave = false;
                        }
                    }

                    if (doSave && iniFile.SetFloat(key,setpoint,"",section))
                    {
                        iniFile.Save();
                    }
                }
                retVal = true;
            }
        };break;
        case COMM_GETPIDINFO:
        {
            unsigned int pidNumber = xmlUtil->GetIntParam("PIDNUMBER");
            float params[3];

            if (pidNumber > FULLUTA_PID_POST)
            {
                break;
            }

            m_PIDVector[pidNumber].GetPIDParams(params);

            Cmd com("DEVICE");
            com.putValue("TYPE", "PIDInfo");
            com.putValue("ADDRESS", m_Address);
            com.putValue("KAPPA", params[0]);
            com.putValue("TINT", params[1]);
            com.putValue("TDER", params[2]);
            com.putValue("SETPOINT", m_PIDVector[pidNumber].GetSetPoint());
            com.putValue("PIDNUMBER", pidNumber);

            if (engPtr->CheckInterfacePortsForConnection())
            {
                engPtr->WriteOnInterfacePorts(com.toString().c_str(), com.toString().size());
            }

            retVal = true;

        };break;
        default:break;
    }

    return retVal;
}

