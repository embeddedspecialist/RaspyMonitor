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
#include "fullutactrl.h"
#include "conewireengine.h"
#include "conewirenet.h"

#define IS_CTRL(pCtrl) pCtrl!=0x0

static const char* Uta_Config_Strings[] =
{
//PID
    "HeatPID",
    "ColdPID",
    "HumPID",
    "VOCPID",
    "CO2PID",
    "LMMAXPID",
    "LMMINPID",
    "POSTPID",
    "FreeCoolingPID",
    "DeUm",

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

//Main Logic
    "UseFreeCooling",
    "UseAirQuality",
    "Summer"
            
};


CFullUTACtrl::CFullUTACtrl(const char *configString, CTimer *timer): CVCoordinator(configString)
{
    //Allocate positions for the controllers
    m_ControllerList.resize(UTA_TOTPARAMETERS,0x0);

    m_AreShuttersOpen = false;
    m_TimeOfShutterOpening = 0;
    m_ShutterDelay = 60;
    m_PostHyst = 5.0;
    m_WinterSetPoint = 20.0;
    m_SummerSetPoint = 26.0;
    m_WinterHumSP = 45.0;
    m_SummerHumSP = 55.0;
    m_TempRegType = 1;
    m_IsSummer = true;
    m_TempMnd = m_TempRip = -100.0;
    m_MainPIDOutputVolt = 0;
    m_TempSetPoint = TEMP_ERRVAL;
    m_IsSummer = false;
    m_Humidity = ANALOG_ERRVAL;

    m_UseFreeCooling = false;
    m_UseAirQuality = false;

    if (configString != 0x0)
    {
        m_IniLib.GetConfigParamInt( configString, "CONFIGID", &m_ConfigID, -1);
    }

	//Aggiungo riga che secondo me mancava
	m_IniLib.GetConfigParamInt( configString, "TIMERID", &m_TimerID, -1);

    if ( (m_TimerID > 0) && (timer != 0x0) )
    {
        m_Timer = timer;
        m_UseTimer = true;
    }
    else
    {
        m_UseTimer = false;
    }

    m_MainPIDOutput = 255;
    m_HumidityPIDOutput = 255;
    m_TypeOfTimerVal = TIMERVAL_DIGITAL;
    m_ControllerType = DEV_FULLUTACTRL;
}


CFullUTACtrl::~CFullUTACtrl()
{
}
//////////////////////////////////////////////////////////////////////
//              Update
//////////////////////////////////////////////////////////////////////
bool CFullUTACtrl::Update(bool updateData)
{
    bool retVal = false;
    //bool isUTAOn = false;
    time_t actTime;
    //Variabili che contengono i valori dei PID rigirati in scala da 0 a 255 anzichè da 255 a 0
    int coldPID = 0,heatPID = 0,lmminPID = 0,lmmaxPID = 0;

	m_IsUTAOn=true;	

    time (&actTime);

    
    if (!m_InitOk)
    {
        InitData();
        m_InitOk = true;
    }

    if (m_DebugLevel)
    {
        cout << "UTA Controller address: " << m_Address << " Uscite PIDs:\n";
    }

    //TODO set error management
    //First gather all data from the inputs
    //Acquisisco il setpoint e nella setSummer lo impongo
    if (AcquireTempSetPoint(&m_TempSetPoint))
    {
        m_WinterSetPoint = m_SummerSetPoint = m_TempSetPoint;
    }
    
    //Check for summer
    CheckSummer();

    //Then check for timers
    if (m_UseTimer && IsTimerEnabled())
    {
        m_IsUTAOn = GetValFromTimer();
    }
    else
    {
        if (!GetInput((int)UTA_ONOFF,&m_IsUTAOn))
        {
            m_IsUTAOn = true;
        }
    }

    //Metto qui la disabilitazione del controllo in modo che se lo disabilito si spegne tutto automaticamente
    m_IsUTAOn = m_IsUTAOn && m_IsActive;
    
    //Gather temperatures
    GetInput(UTA_TEMPMND, &m_TempMnd);
    GetInput(UTA_TEMPRIP, &m_TempRip);

    //Execute the calculus of the main logic by executing everyPID
    if ( (m_TempMnd != TEMP_ERRVAL) && (m_TempRip != TEMP_ERRVAL))
    {
        UpdatePID(UTA_HEATBAT, m_TempRip);
        UpdatePID(UTA_COLDBAT, m_TempRip);
        UpdatePID(UTA_LMMAX, m_TempMnd);
        UpdatePID(UTA_LMMIN, m_TempMnd);
    }

    GetInput((int)UTA_COLDBAT, &coldPID);
    GetInput((int)UTA_HEATBAT, &heatPID);
    GetInput((int)UTA_LMMIN, &lmminPID);
    GetInput((int)UTA_LMMAX, &lmmaxPID);

    coldPID = 255 - coldPID;
    heatPID = 255 - heatPID;
    lmminPID = 255 - lmminPID;
    lmmaxPID = 255 - lmmaxPID;


    //Now execute everything
    if (m_IsUTAOn)
    {
        if (!m_AreShuttersOpen)
        {
            ShuttersControl(m_TempRip);
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
            WriteOutput((int)UTA_FANMND, 1);
            WriteOutput((int)UTA_FANRIP, 1);

            //Adjust shutters
            ShuttersControl(m_TempRip);

            if (m_IsSummer)
            {
                //Accendo la pompa del freddo
                WriteOutput((int)UTA_COLDPUMP, 1);
                heatPID=255;
                
                //Close the hot battery
                WriteOutput((int)UTA_HEATBAT, 255);

                //Se ho il controllo umidità accendo la pompa del caldo -> lo rimando al controllo umidita'
                //Da questa funzione ricalcolo il valore del pid dell'umidita' e fisso la mandata se ho il post
                HumidityControl();

                //22/06/2009 -- la funzionalita' vecchia sbagliava se il setpoint e' maggiore della ripresa ma bisogna umidificare: in quel caso i due PID vanno in contrasto 
                //e la batteria fredda si chiude (non deumidifica) mentre la calda di post si apre per mantenere la mandata almeno uguale al setpoint e quindi
                //penso che buttasse dentro aria calda e umida
                
                if (m_HumidityPIDOutput > 0)
                {
                    //Apro la fredda in funzione dell'umiditia'
                    m_MainPIDOutput = 255 - m_HumidityPIDOutput;
                }
                else
                {
                    m_MainPIDOutput = 255 - (coldPID - lmminPID + lmmaxPID);
                }
                
                
//                 //Calculate the output...dubbio: il limite se agisce l'umidità non dovrebbe agire perchè altrimenti lavora "contro" la deumidifcazione
//                 if (m_HumidityPIDOutput > 0)
//                 {
//                     m_MainPIDOutput = 255 - (coldPID + lmmaxPID - m_HumidityPIDOutput);
//                 }
//                 else
//                 {
//                     m_MainPIDOutput = 255 - (coldPID - lmminPID + lmmaxPID);
//                 }

                if (m_MainPIDOutput > 255)
                {
                    m_MainPIDOutput = 255;
                }
                else if (m_MainPIDOutput < 0)
                {
                    m_MainPIDOutput = 0;
                }

                //Write on the output
                if (WriteOutput((int)UTA_COLDBAT,m_MainPIDOutput))
                {
                    m_MainPIDOutputVolt = (float)(( 255.0 - m_MainPIDOutput )/255.0 * 10.0);
                }
            }
            else //INVERNO
            {
                //Chiudo Batteria freddo
                WriteOutput((int)UTA_COLDBAT, 255);
                coldPID = 255;
                //Spengo la pompa freddo e accendo la pompa caldo
                WriteOutput((int)UTA_COLDPUMP,0);
                WriteOutput((int)UTA_HEATPUMP,1);

                //outVal = GetPIDOutput(PID_DIR_HB) + GetPIDOutput(PID_LM_MIN) - GetPIDOutput(PID_LM_MAX);
                m_MainPIDOutput = 255 - (heatPID + lmminPID - lmmaxPID);

                if (m_MainPIDOutput > 255)
                {
                    m_MainPIDOutput = 255;
                }
                else if (m_MainPIDOutput < 0)
                {
                    m_MainPIDOutput = 0;
                }

                //Write to the output
                if (WriteOutput((int)UTA_HEATBAT, m_MainPIDOutput))
                {
                    m_MainPIDOutputVolt = (float)(( 255.0 - m_MainPIDOutput )/255.0 * 10.0);
                }

                //Controllo umidita'
                HumidityControl();
            }
        }
    }
    else
    {
        //TODO manca tutta la logica failsafe: es. se NON si spengono i ventilatori NON chiudo le serrande
        //Turn off everything:
        //Turn Off fans
        WriteOutput((int)UTA_FANMND, 0);
        WriteOutput((int)UTA_FANRIP, 0);
        //Turn off post heating
        WriteOutput((int)UTA_POST1,0);
        WriteOutput((int)UTA_POST2,0);
        
        WriteOutput((int)UTA_HUMIDIFIER,0);

        //Close shutters
        WriteOutput((int)UTA_MAINSHUTT,255);
        WriteOutput((int)UTA_RECSHUTT,255);

        //Close batteries
        WriteOutput((int)UTA_HEATBAT,255);
        WriteOutput((int)UTA_COLDBAT,255);
        WriteOutput((int)UTA_HUMBAT,255);

        //Shutdown pumps
        WriteOutput((int)UTA_COLDPUMP,0);
        WriteOutput((int)UTA_HEATPUMP,0);
        
        m_MainPIDOutput = 255;
        m_MainPIDOutputVolt = 0;
        m_Humidity = 0.0;

    }

    if (m_DebugLevel)
    {
        cout << "UTA Controller address: " << m_Address << " Uscite PIDs:\n";
        cout << "Calda: "<<heatPID<<endl;
        cout << "Fredda: "<<coldPID<<endl;
        cout << "LMMIN: "<<lmminPID<<endl;
        cout << "LMMAX: "<<lmmaxPID<<endl;
        cout << "MainPID (Volt): " << m_MainPIDOutputVolt;
        cout << " Summer: "<< m_IsSummer <<endl;
        cout.flush();
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////////////////
bool CFullUTACtrl::SetIsUTAOn ( bool theValue )
{
    m_IsActive = theValue;
    return true;
}
    
bool CFullUTACtrl::SetSetPoint ( float valueSP )
{
    m_TempSetPoint = valueSP;
    m_SummerSetPoint = valueSP;
    m_WinterSetPoint = valueSP;
    return true;
}
    
bool CFullUTACtrl::InitData()
{
    //The humidity PID can perform also negative calculus in order to indicate if we have to add humidity (positive) or subtract humidity
    if (m_ControllerList[UTA_HUMBAT] != 0x0)
    {
        ((CPIDSimple*)(m_ControllerList.at(UTA_HUMBAT)))->m_AllowsNegativeCalc = true;
    }

    //Disabilito tutti i PID in modo che nel loop principale della conewirengine non siano aggiornati
    for (int i = UTA_HEATBAT; i < UTA_TEMPMND; i++)
    {
        if (m_ControllerList[i] != 0x0)
        {
            m_ControllerList[i]->ActivateController(false);
        }
    }
    
    return true;
}
////////////////////////////////////////////////////////////////////////////////
bool CFullUTACtrl::CheckSummer()
{

    GetInput((int)UTA_SUMMERSWITCH, &m_IsSummer);

    if (m_DebugLevel)
    {
        cout << "Summer: "<<m_IsSummer<<" ";
    }

    if (m_IsSummer)
    {
        SetPIDSetpoint((int)UTA_HEATBAT,m_SummerSetPoint);
        SetPIDSetpoint((int)UTA_COLDBAT,m_SummerSetPoint);
        SetPIDSetpoint((int)UTA_POSTPID,m_SummerSetPoint);
        SetPIDSetpoint((int)UTA_HUMBAT,m_SummerHumSP);
        SetPIDSetpoint((int)UTA_DEHUMPID, m_SummerHumSP);
    }
    else
    {

        SetPIDSetpoint((int)UTA_HEATBAT,m_WinterSetPoint);
        SetPIDSetpoint((int)UTA_COLDBAT,m_WinterSetPoint);
        SetPIDSetpoint((int)UTA_POSTPID,m_WinterSetPoint);
        SetPIDSetpoint((int)UTA_HUMBAT,m_WinterHumSP);
        SetPIDSetpoint((int)UTA_DEHUMPID, m_WinterHumSP);
    }

    //Aggiorno il pid dell'umidita'
    if (IS_CTRL(m_ControllerList[UTA_HUMBAT]))
    {
        ((CPIDSimple*)(m_ControllerList[UTA_HUMBAT]))->SetSummer(m_IsSummer);
    }
    
    if (IS_CTRL(m_ControllerList[UTA_DEHUMPID]))
    {
        ((CPIDSimple*)(m_ControllerList[UTA_DEHUMPID]))->SetSummer(m_IsSummer);
    }    
    
    return true;
}
////////////////////////////////////////////////////////////////////////////////
bool CFullUTACtrl::ConnectControllers()
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

    //Leggo dal file di configurazione (changeover.ini) tutte le chiavi e le zone e le istanzio, programmo i moduli di controllo accessi
    if ( !iniFileReader.Load ( "./UTACoord.ini" ) )
    {
        cout << "Attenzione: impossibile aprire il file UTACoord.ini"<<endl;
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
            cout << "Attenzione coordinatore di indirizzo: " << m_Address << " in UTAConfig.ini NON esiste la sezione"<<configIDString<<endl;
            return false;
        }   
    }
    
    //Carico le variabili e gli stati della logica principale
    m_UseFreeCooling = iniFileReader.GetBool(Uta_Config_Strings[UTA_USEFREECOOLING], configIDString, false);
    m_FreeCoolingPercentage = iniFileReader.GetInt("FreeCoolingMinUsage", configIDString, 15);
    m_UseAirQuality = iniFileReader.GetBool(Uta_Config_Strings[UTA_USEAIRQUALITY], configIDString, false);
    m_IsSummer = iniFileReader.GetBool(Uta_Config_Strings[UTA_SUMMERSTATE],configIDString,false);
    
    //Carico i controllori
    for (int i = UTA_HEATBAT; i < UTA_SUMMERTEMPSP; i++)
    {
        ctrlAddress = iniFileReader.GetInt(Uta_Config_Strings[i],configIDString,-1);
        netIndex = netPtr->GetNetByMemoryAddress(ctrlAddress);
        ctrlIndex = netPtr->GetDeviceIndexByMemoryAddress(netIndex, ctrlAddress);

        if ( (netIndex<0) || (ctrlIndex < 0))
        {
            cout << "Attenzione: coordinatore di indirizzo: " << m_Address<< " campo "<<Uta_Config_Strings[i]<<" NON valido in UTAConfig.ini"<<endl;
            msDelay(1000);
            continue;
        }

        //Vedo se il regolatore e' nostro o itk
        if (i == UTA_TEMPREG)
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

        if ((i == UTA_HUMBAT) || (i == UTA_DEHUMPID))
        {
            ((CPIDSimple*)(m_ControllerList[i]))->SetSummer(m_IsSummer);
        }
    }

    //Carico i setpoint
    m_SummerSetPoint = iniFileReader.GetFloat(Uta_Config_Strings[UTA_SUMMERTEMPSP], configIDString, -1.0);
    if (m_SummerSetPoint < 0)
    {
        m_SummerSetPoint = 26.0;
    }

    m_WinterSetPoint = iniFileReader.GetFloat(Uta_Config_Strings[UTA_WINTERTEMPSP], configIDString, -1.0);
    if (m_WinterSetPoint < 0)
    {
        m_WinterSetPoint = 20.0;
    }

    m_SummerHumSP = iniFileReader.GetFloat(Uta_Config_Strings[UTA_SUMMERHUMSP], configIDString, -1.0);
    if (m_SummerHumSP < 0)
    {
        m_SummerHumSP = 55.0;
    }

    m_WinterHumSP = iniFileReader.GetFloat(Uta_Config_Strings[UTA_WINTERHUMSP], configIDString, -1.0);
    if (m_WinterHumSP < 0)
    {
        m_WinterHumSP = 45.0;
    }

    m_PostHyst = iniFileReader.GetFloat(Uta_Config_Strings[UTA_POSTHYST], configIDString, -1.0);
    if (m_PostHyst < 0)
    {
        m_PostHyst = 5.0;
    }

    m_ShutterDelay = iniFileReader.GetInt(Uta_Config_Strings[UTA_SHUTTDELAY], configIDString, -1);
    if (m_PostHyst < 0)
    {
        m_ShutterDelay = 30;
    }

    return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CFullUTACtrl::AcquireTempSetPoint(float *setPoint)
{
    double angle;
    int currentRegister;
    
    if ((m_ControllerList[UTA_TEMPREG]==0x0) || (setPoint == 0x0))
    {
        return false;
    }

    if (m_TempRegType)
    {
        //Sonda NTH
        currentRegister = ((CAnalogIO*)(m_ControllerList.at(UTA_TEMPREG)))->ReadCurrentRegister();

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
            
            *setPoint = angle * 20.0 / 270.0 + 10.0;
    //         *setPoint = (normalizedRegister*25.0) / 10.0 + 5.0;

            if (m_DebugLevel)
            {
                cout << "Setpoint : "<<*setPoint<<endl;
                cout << "Angle : "<<angle<<endl;
                cout << "Registro : "<<currentRegister<<endl;
            }

            return true;
        }
    }
    else
    {
        float tempVolt;

        tempVolt = ((CAnalogIO*)(m_ControllerList[UTA_TEMPREG]))->GetLastValue();

        *setPoint = (tempVolt*25.0)/10.0+5.0;

        if (m_DebugLevel)
        {
            cout << "Setpoint Impostato: "<<*setPoint<<endl;
        }

        return true;
    }
        
    return false;
}

///////////////////////////////////////////////////////////////////////////
//                  HumidityControl
//Il controllo umidita' viene fatto nel seguente modo: se e' estate ed e' troppo
//umido aggiungo il valore del PID umidita' alla valvola principale per fare
//freddissimo e condensare sulla batteria freddo, contemporaneamente il pid del
//post aprira' il post per mantenere costante la mandata. Nasce il problema
//se il post non ce la fa a mantenere la mandata...
///////////////////////////////////////////////////////////////////////////
bool CFullUTACtrl::HumidityControl()
{
    bool isPOSTPresent = false;       //Indica se c'è il POST
    bool isPOSTDigital = false;     //Indica se il POST è digitale o analogico
    bool areDoublePOST = false;     //Indica se ci sono due batterie di POST
    bool isHumidifierPresent = false; //L'umidificatore puo' essere sia modulante che digitale
    bool isHumidifierDigital = false;
    
    int postPIDOutput = 255;

    if (m_ControllerList[UTA_HUMIDITY] != 0x0)
    {
        //Gather humidity
        m_Humidity = ((CAnalogIO*)(m_ControllerList.at(UTA_HUMIDITY)))->GetLastValue();

        //Compenso errori di lettura
        if (m_Humidity <0.0)
        {
            m_Humidity = 0.0;
        }

    }
    else
    {
        //Non c'e' sonda di umidita', esco subito
        return true;
    }

    //Controllo se esiste l'umidificatore e di che tipo e'
    if (m_ControllerList[UTA_HUMIDIFIER] != 0x0)
    {
        isHumidifierPresent = true;

        if (m_ControllerList[UTA_HUMBAT] != 0x0)
        {
            isHumidifierDigital = false;
        }
        else
        {
            isHumidifierDigital = true;
        }
    }

    //Controllo il tipo di POST
    if (m_ControllerList[UTA_POSTPID] != 0x0)
    {
        //Il post è analogico
        isPOSTPresent = true;
        isPOSTDigital = false;

        //Accendo la pompa del caldo
        WriteOutput((int)UTA_HEATPUMP,1);
    }
    else
    {
        //Controllo se esite almeno una batteria di POSt riscaldo elettrica
        if (m_ControllerList[UTA_POST1] == 0x0)
        {
            isPOSTPresent = false;
        }
        else
        {
            isPOSTPresent = true;
            isPOSTDigital = true;

            //Controllo se e' a doppi accensione
            if (m_ControllerList[UTA_POST2] != 0x0)
            {
                areDoublePOST = true;
            }
        }
    }
           
    
    if (m_IsSummer)
    {
        //Aggiorno il PID de-umidificatore
        if (UpdatePID(UTA_DEHUMPID,m_Humidity))
        {
            GetInput((int)UTA_DEHUMPID, &m_HumidityPIDOutput);
        }
        else
        {
            m_HumidityPIDOutput = 255;
        }
        
        //Spengo umidificatore se esiste
        if (isHumidifierPresent)
        {
            if (isHumidifierDigital)
            {
                ((CDigitalIO*)(m_ControllerList.at(UTA_HUMIDIFIER)))->SetState(0);
            }
            else
            {
                ((CPIDSimple*)(m_ControllerList.at(UTA_HUMBAT)))->WritePIDOutput(255);
            }
        }

        //Se e' estate aggiungo il valore del pid alla batteria del freddo
        //Il valore del pid umidità va aggiunto alla batteria fredda insieme a tutto il resto
        //Lo riscalo nel range giusto
        m_HumidityPIDOutput = 255 - m_HumidityPIDOutput;

        //Se l'umidita' va bene e' inutile che controlli i POST in mandata
        
        //Se ho POST digitale faccio isteresi sulla temperatura di mandata, altrimenti uso il PID
        if (isPOSTDigital && isPOSTPresent && (m_HumidityPIDOutput > 0))
        {
            //Il controllo della umidita' con i POST lo faccio solo di estate
            //Isteresi della seconda accensione
            if (m_TempMnd < (m_TempSetPoint - m_PostHyst))
            {
                WriteOutput(UTA_POST1,1);
            }
            else if (m_TempMnd > (m_TempSetPoint + m_PostHyst))
            {
                WriteOutput(UTA_POST1,0);
            }

            //Isteresi prima accensione
            if ((m_TempMnd < (m_TempSetPoint - (m_PostHyst/2)))  && (areDoublePOST))
            {
                WriteOutput(UTA_POST2,1);
            }
            else if ((m_TempMnd > (m_TempSetPoint + m_PostHyst/2))  && (areDoublePOST))
            {
                WriteOutput(UTA_POST2,0);
            }
        }
        else if (isPOSTPresent && (m_HumidityPIDOutput > 0))
        {
            //Aggiorno PID del post
            UpdatePID(UTA_POSTPID,m_TempMnd);
            GetInput((int)UTA_POSTPID, &postPIDOutput);

            if (postPIDOutput > 255)
            {
                postPIDOutput = 255;
            }
            else if (postPIDOutput < 0)
            {
                postPIDOutput = 0;
            }

            WriteOutput(UTA_POSTPID, postPIDOutput);
        }
        else
        {
            m_HumidityPIDOutput = 0;
        }
    }
    else  //Winter
    {
        //Aggiorno il PID umidificatore
        if (UpdatePID(UTA_HUMBAT,m_Humidity))
        {
            GetInput((int)UTA_HUMBAT, &m_HumidityPIDOutput);
        }
        else
        {
            m_HumidityPIDOutput = 255;
        }
        
        //Turn on humidifier
        if (isHumidifierPresent)
        {
            if (isHumidifierDigital)
            {
                //L'umidificatore e' digitale: implemento isteresi
                if (m_Humidity < (m_WinterHumSP - m_PostHyst))
                {
                    ((CDigitalIO*)(m_ControllerList.at(UTA_HUMIDIFIER)))->SetState(1);
                }
                else if (m_Humidity > (m_WinterHumSP + m_PostHyst))
                {
                    ((CDigitalIO*)(m_ControllerList.at(UTA_HUMIDIFIER)))->SetState(0);
                }
            }
            else
            {
                ((CDigitalIO*)(m_ControllerList.at(UTA_HUMIDIFIER)))->SetState(1);

                WriteOutput(UTA_HUMIDIFIER, m_HumidityPIDOutput);
            }
        }

        if (isPOSTDigital && isPOSTPresent)
        {
            //Turn off post heater
            WriteOutput(UTA_POST1, 0);

            if (areDoublePOST)
            {
                WriteOutput(UTA_POST2, 0);
            }
        }
        else if (isPOSTPresent)
        {
            //Chiudo la valvola del PID del POST ?? In realtà a via Rivani non ce la faccio a fare abbastanza caldo ->
            //ATTENZIONE: SOLO per via rivani dove hanno invertito le batterie aggiorno il pid con la temperatura ambiente 
            //Aggiorno PID del post
//             UpdatePID(UTA_POSTPID,m_TempRip);
//             GetInput((int)UTA_POSTPID, &postPIDOutput);
// 
//             if (postPIDOutput > 255)
//             {
//                 postPIDOutput = 255;
//             }
//             else if (postPIDOutput < 0)
//             {
//                 postPIDOutput = 0;
//             }
// 
//             WriteOutput(UTA_POSTPID, postPIDOutput);
            WriteOutput(UTA_POSTPID, 255);
        }
            
    }

    if (m_DebugLevel)
    {
        cout << "PID - Umidificatore: "<<m_HumidityPIDOutput<<endl;
        cout << "PID - POST : "<<postPIDOutput;
    }

    return true;
}

//Funzione originale commentata il 16/12/2008
// bool CFullUTACtrl::HumidityControl()
// {
//     bool isPOSTPresent = false;       //Indica se c'è il POST
//     bool isPOSTDigital = false;     //Indica se il POST è digitale o analogico
//     bool areDoublePOST = false;     //Indica se ci sono due batterie di POST
//     bool isHumidifierPresent = false; //L'umidificatore puo' essere sia modulante che digitale
//     bool isHumidifierDigital = false;
//     
//     int humPIDOutput = 255, postPIDOutput = 255;
//     
//     
// 
//     if (m_ControllerList[UTA_HUMIDITY] != 0x0)
//     {
//         //Gather humidity
//         m_Humidity = ((CAnalogIO*)(m_ControllerList.at(UTA_HUMIDITY)))->GetLastValue();
// 
//         //Compenso errori di lettura
//         if (m_Humidity <0.0)
//         {
//             m_Humidity = 0.0;
//         }
// 
//     }
//     else
//     {
//         //Non c'e' sonda di umidita', esco subito
//         return true;
//     }
// 
//     //Controllo se esiste l'umidificatore e di che tipo e'
//     if (m_ControllerList[UTA_HUMBAT] != 0x0)
//     {
//         isHumidifierPresent = true;
//         isHumidifierDigital = false;
//     }
//     else
//     {
//         if (m_ControllerList[UTA_HUMIDIFIER] != 0x0)
//         {
//             isHumidifierPresent = true;
//             isHumidifierDigital = true;
//         }
//         else
//         {
//             isHumidifierPresent = false;
//         }
//     }
// 
//     //Controllo il tipo di POST
//     if (m_ControllerList[UTA_POSTPID] != 0x0)
//     {
//         //Il post è analogico
//         isPOSTPresent = true;
//         isPOSTDigital = false;
// 
//         //Accendo la pompa del caldo
//         WriteOutput((int)UTA_HEATPUMP,1);
//     }
//     else
//     {
//         //Controllo se esite almeno una batteria di POSt riscaldo elettrica
//         if (m_ControllerList[UTA_POST1] == 0x0)
//         {
//             isPOSTPresent = false;
//         }
//         else
//         {
//             isPOSTPresent = true;
//             isPOSTDigital = true;
// 
//             //Controllo se e' a doppi accensione
//             if (m_ControllerList[UTA_POST2] != 0x0)
//             {
//                 areDoublePOST = true;
//             }
//         }
//     }
//            
// 
//     if (m_IsSummer)
//     {
//         //Spengo umidificatore se esiste
//         if (isHumidifierPresent)
//         {
//             ((CDigitalIO*)(m_ControllerList.at(UTA_HUMIDIFIER)))->SetState(0);
//             if (!isHumidifierDigital)
//             {
//                 ((CPIDSimple*)(m_ControllerList.at(UTA_HUMBAT)))->WritePIDOutput(255);
//             }
//             humPIDOutput = 255;
//         }
// 
//         //Se ho POST digitale faccio isteresi, altrimenti uso il PID
//         if (isPOSTDigital && isPOSTPresent)
//         {
//             //Il controllo della umidita' con i POST lo faccio solo di estate
//             //Isteresi della seconda accensione
//             if (m_Humidity < (m_SummerHumSP - m_PostHyst))
//             {
//                 ((CDigitalIO*)(m_ControllerList.at(UTA_POST1)))->SetState(0);
//             }
//             else if (m_Humidity > (m_SummerHumSP + m_PostHyst))
//             {
//                 ((CDigitalIO*)(m_ControllerList.at(UTA_POST1)))->SetState(1);
//             }
// 
//             //Isteresi prima accensione
//             if ((m_Humidity < (m_SummerHumSP - (m_PostHyst/2)))  && (areDoublePOST))
//             {
//                 ((CDigitalIO*)(m_ControllerList.at(UTA_POST2)))->SetState(0);
//             }
//             else if ((m_Humidity > (m_SummerHumSP + m_PostHyst/2))  && (areDoublePOST))
//             {
//                 ((CDigitalIO*)(m_ControllerList.at(UTA_POST2)))->SetState(1);
//             }
//         }
//         else if (isPOSTPresent)
//         {
//             //Aggiorno PID del post
//             ((CPIDSimple*)(m_ControllerList.at(UTA_POSTPID)))->Update(m_Humidity);
//             postPIDOutput = ((CPIDSimple*)(m_ControllerList.at(UTA_POSTPID)))->m_PIDOutput;
// 
//             if (postPIDOutput > 255)
//             {
//                 postPIDOutput = 255;
//             }
//             else if (postPIDOutput < 0)
//             {
//                 postPIDOutput = 0;
//             }
// 
//             ((CPIDSimple*)(m_ControllerList.at(UTA_POSTPID)))->WritePIDOutput(postPIDOutput);
//                 
//         }
//             
//     }
//     else  //Winter
//     {
//         //Turn on humidifier
//         if (isHumidifierPresent)
//         {
//             if (isHumidifierDigital)
//             {
//                 //L'umidificatore e' digitale: implemento isteresi
//                 if (m_Humidity < (m_WinterHumSP - m_PostHyst))
//                 {
//                     ((CDigitalIO*)(m_ControllerList.at(UTA_HUMIDIFIER)))->SetState(1);
//                 }
//                 else if (m_Humidity > (m_WinterHumSP + m_PostHyst))
//                 {
//                     ((CDigitalIO*)(m_ControllerList.at(UTA_HUMIDIFIER)))->SetState(0);
//                 }
//             }
//             else
//             {
//                 ((CDigitalIO*)(m_ControllerList.at(UTA_HUMIDIFIER)))->SetState(1);
//                 
//                 //Aggiorno PID umidificatore
//                 ((CPIDSimple*)(m_ControllerList.at(UTA_HUMBAT)))->Update(m_Humidity);
//                 
//                 //Check for humidity
//                 humPIDOutput = ((CPIDSimple*)(m_ControllerList.at(UTA_HUMBAT)))->m_PIDOutput;
//                 if (humPIDOutput <= 0)
//                 {
//                         //Devo umidificare
//                     ((CPIDSimple*)(m_ControllerList.at(UTA_HUMBAT)))->WritePIDOutput(-humPIDOutput);
//                 }
//             }
//         }
// 
//         if (isPOSTDigital && isPOSTPresent)
//         {
//             //Turn off post heater
//             //TODO da controllare il tipo di POST e fare il check su quelli che ci sono o meno
//             //Il tipo può essere analogico o digitale
//             ((CDigitalIO*)(m_ControllerList.at(UTA_POST1)))->SetState(0);
// 
//             if (areDoublePOST)
//             {
//                 ((CDigitalIO*)(m_ControllerList.at(UTA_POST2)))->SetState(0);
//             }
//         }
//         else if (isPOSTPresent)
//         {
//             //Chiudo la valvola del PID
//             ((CPIDSimple*)(m_ControllerList.at(UTA_POSTPID)))->WritePIDOutput(255);
// 
//             postPIDOutput = 255;
//         }
//             
//     }
// 
//     if (m_DebugLevel)
//     {
//         cout << "PID - Umidificatore: "<<humPIDOutput<<endl;
//         cout << "PID - POST : "<<postPIDOutput;
//     }
// 
//     return true;
// }
///////////////////////////////////////////////////////////////////
//                  AirQualityControl
////////////////////////////////////////////////////////////////////
bool CFullUTACtrl::AirQualityControl()
{
    int co2PID, vocPID;
    bool isCO2Present = false,isVOCPresent = false;
    float voc = 0.0, co2 = 0.0;
    int aqPIDOutput;

    if (m_ControllerList[UTA_CO2PID] != 0x0)
    {
        isCO2Present = true;
    }

    if (m_ControllerList[UTA_VOCPID] != 0x0)
    {
        isVOCPresent = true;
    }

    if ( (!isCO2Present) && (!isVOCPresent) )
    {
        //Non c'e' controllo della qualita' dell'aria
        return true;
    }
    
    //Sistemo le serrande: normalmente la serranda di ricircolo è aperta, se il pid della qualità mi
    //dice diversamente la chiudo e apro l'aria esterna
    
    //Gather VOC and CO2
    if (isVOCPresent)
    {
        voc = ((CAnalogIO*)(m_ControllerList.at(UTA_VOC)))->GetLastValue();
        if (voc >= 0)
        {
            ((CPIDSimple*)(m_ControllerList.at(UTA_VOCPID)))->Update(voc);
        }
        
        vocPID = ((CPIDSimple*)(m_ControllerList.at(UTA_VOCPID)))->m_PIDOutput;
    }
    else
    {
        vocPID = 0;
    }

    if (isCO2Present)
    {
        co2 = ((CAnalogIO*)(m_ControllerList.at(UTA_CO2)))->GetLastValue();

        if (co2 >= 0)
        {
        //TODO forse qui va rallentato questo PID perchè la sonda è molto lenta
 
            ((CPIDSimple*)(m_ControllerList.at(UTA_CO2PID)))->Update(co2);
        }

        co2PID = ((CPIDSimple*)(m_ControllerList.at(UTA_CO2PID)))->m_PIDOutput;
    }
    else
    {
        co2PID = 0;
    }
    

    if ( co2PID < vocPID)
    {
        aqPIDOutput = ((CPIDSimple*)(m_ControllerList.at(UTA_CO2PID)))->m_PIDOutput;
    }
    else
    {
        aqPIDOutput = ((CPIDSimple*)(m_ControllerList.at(UTA_VOCPID)))->m_PIDOutput;
    }

    if (aqPIDOutput > 255)
    {
        aqPIDOutput = 0;
    }
    else if (aqPIDOutput < 0)
    {
        aqPIDOutput = 0;
    }

    //Alla serranda di ricircolo mando il segnale del PID invertito (0, cioè tutto aperto, se va tutto bene) per tenerla aperta,
    //alle serrande esterne mando il segnale invertito
    ((CAnalogIO*)(m_ControllerList.at(UTA_RECSHUTT)))->SetPosition(255 - aqPIDOutput);
    ((CAnalogIO*)(m_ControllerList.at(UTA_MAINSHUTT)))->SetPosition(aqPIDOutput);

    if (m_DebugLevel)
    {
        cout << "AirQualityPID: "<<aqPIDOutput<<endl;
    }

    return true;
}

bool CFullUTACtrl::WriteOutput(int device, int value)
{
    bool retVal = false;

    switch(device)
    {
        case UTA_HEATBAT:
        case UTA_COLDBAT:
        case UTA_HUMBAT:
        case UTA_VOCPID:
        case UTA_CO2PID:
        case UTA_LMMAX:
        case UTA_LMMIN:
        case UTA_POSTPID:
        {
            if (m_ControllerList[device] != 0x0)
            {
                ((CPIDSimple*)(m_ControllerList.at(device)))->WritePIDOutput(value);
                retVal = true;
            }

            
            break;
        };
        case UTA_POST1:
        case UTA_POST2:
        case UTA_FANMND:
        case UTA_FANRIP:
        case UTA_HUMIDIFIER:
        case UTA_HEATPUMP:
        case UTA_COLDPUMP:
        {
            if (m_ControllerList[device] != 0x0)
            {
                ((CDigitalIO*)(m_ControllerList.at(device)))->SetState((int)value);
                retVal = true;
            }

            
            break;
        };
        case UTA_MAINSHUTT:
        case UTA_RECSHUTT:
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
bool CFullUTACtrl::GetInput(int device, float * value)
{
    bool retVal = false;

    switch (device)
    {
        case UTA_TEMPMND:
        case UTA_TEMPRIP:
        case UTA_TEMPEXT:
        {
            if (m_ControllerList[device] != 0x0)
            {
                *value = ((CTempCtrl*)(m_ControllerList.at(device)))->GetLastTemp();
                retVal = true;
            }
            
            break;
        }
        case UTA_HUMIDITY:
        {
            if (m_ControllerList[device] != 0x0)
            {
                *value = ((CAnalogIO*)(m_ControllerList.at(UTA_HUMIDITY)))->GetLastValue()*10;
                retVal = true;
            }
            break;
        }
        case UTA_VOC:
        case UTA_CO2:
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
bool CFullUTACtrl::GetInput(int device, bool * value)
{
    bool retVal = false;
    
    switch (device)
    {
        case UTA_SUMMERSWITCH:
        case UTA_ONOFF:
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
//                  GetInput (int)
/////////////////////////////////////////////////////////////////////////////////////////////////
bool CFullUTACtrl::GetInput(int device, int * value)
{
    bool retVal = false;
    
    switch (device)
    {
        case UTA_HEATBAT:
        case UTA_COLDBAT:
        case UTA_HUMBAT:
        case UTA_VOCPID:
        case UTA_CO2PID:
        case UTA_LMMAX:
        case UTA_LMMIN:
        case UTA_POSTPID:
        case UTA_DEHUMPID:
        {
            if (m_ControllerList[device] != 0x0)
            {
                *value = ((CPIDSimple*)(m_ControllerList.at(device)))->m_PIDOutput;
                retVal = true;
            }

            break;
        };
    }

    return retVal;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//                  UpdatePID
/////////////////////////////////////////////////////////////////////////////////////////////////
bool CFullUTACtrl::UpdatePID(int device, float value)
{
    
    if (m_ControllerList[device] == 0x0)
    {
        return false;
    }

    if (m_ControllerList[device]->GetControllerType() != DEV_PIDSIMPLE)
    {
        return false;
    }
    
    ((CPIDSimple*)(m_ControllerList.at(device)))->Update(value);

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//                  SetPIDSetpoint
/////////////////////////////////////////////////////////////////////////////////////////////////
bool CFullUTACtrl::SetPIDSetpoint(int device, float value)
{
    if (m_ControllerList[device] == 0x0)
    {
        return false;
    }

    if (m_ControllerList[device]->GetControllerType() != DEV_PIDSIMPLE)
    {
        return false;
    }
    
    ((CPIDSimple*)(m_ControllerList[device]))->SetSetPoint(&value);

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//                  ShuttersControl
/////////////////////////////////////////////////////////////////////////////////////////////////
bool CFullUTACtrl::ShuttersControl(float tempRip)
{
    //TODO se le serrande sono digitali presuppongo il ritorno a molla, altrimenti va implementato
    //TODO da vedere se il controllo va separato per serranda mandata e ripresa... raro ma possibile ? (Vd Mercatone)
    //TODO per ora gli algorimti free cooling e airquality sono separati: o va uno o va l'altro
    bool isRecShuttPresent = false;
    bool areShuttDigital = false;

    if (m_ControllerList[UTA_MAINSHUTT] == 0x0)
    {
        //Direi che ho già finito... non c'è il controllo delle serrande principali
        return false;
    }
    else
    {
        if (m_ControllerList[UTA_MAINSHUTT]->GetControllerType() == DEV_DIDO)
        {
            areShuttDigital = true;
        }
    }

    if (m_ControllerList[UTA_RECSHUTT] == 0x0)
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
            WriteOutput((int)UTA_MAINSHUTT, 1);
            WriteOutput((int)UTA_RECSHUTT, 0);
        }
        else
        {
            WriteOutput((int)UTA_MAINSHUTT, 0);
            WriteOutput((int)UTA_RECSHUTT, 255);
        }

        time(&m_TimeOfShutterOpening);
        m_AreShuttersOpen = true;

        return true;
    }
            
    //Se le serrande sono digitali al momento l'unico controllo che mi viene in mente è il free cooling, rimane come TODO
    if (m_UseFreeCooling)
    {
        float tempExt = 0.0;
        //Algoritmo free cooling
        //Per il free cooling uso un algoritmo becero: se inverno e la temp Ext è maggiore del SP e la ripresa è inferiore al SP apro le serrande principali e chiudo il ricircolo
        //viceversa d'estate. In ricircolo, invece, tengo un 10% di serranda principale aperta per garantire il ricambio d'aria

        GetInput(UTA_TEMPEXT, &tempExt);

        if (tempExt < -50.0)
        {
            //Errore in letturra
            return true;
        }
                    
                
        if (m_IsSummer)
        {
            if ((tempExt <= m_SummerSetPoint) &&
                (tempRip > m_SummerSetPoint)
               )
            {
                //Vado in free cooling
                WriteOutput(UTA_MAINSHUTT, 0);
                WriteOutput(UTA_RECSHUTT, 255);
            }
            else
            {
                //Tengo aperto al 10%
                WriteOutput(UTA_MAINSHUTT, (int)(255 - (m_FreeCoolingPercentage/100.0)*255));
                WriteOutput(UTA_RECSHUTT, (int)((m_FreeCoolingPercentage/100.0)*255));
            }
        }
        else
        {
            if ((tempExt >= m_WinterSetPoint) &&
                (tempRip < m_WinterSetPoint)
               )
            {
                //Vado in free cooling
                WriteOutput(UTA_MAINSHUTT, 0);
                WriteOutput(UTA_RECSHUTT, 255);
            }
            else
            {
                //Tengo aperto al 10%
                WriteOutput(UTA_MAINSHUTT, (int)(255 - (m_FreeCoolingPercentage/100.0)*255));
                WriteOutput(UTA_RECSHUTT, (int)((m_FreeCoolingPercentage/100.0)*255));
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
bool CFullUTACtrl::SetSummer(bool isSummer)
{
    CIniFileHandler iniFileReader;
    CString configIDString;

    m_IsSummer = isSummer;
    
    if (!iniFileReader.Load ( "./UTACoord.ini" ))
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

    iniFileReader.SetBool(Uta_Config_Strings[UTA_SUMMERSTATE],isSummer,"",configIDString);

    iniFileReader.Save();

    //Aggiorno il pid dell'umidita'
    if (IS_CTRL(m_ControllerList[UTA_HUMBAT]))
    {
        ((CPIDSimple*)(m_ControllerList[UTA_HUMBAT]))->SetSummer(m_IsSummer);
    }

    return true;
}


