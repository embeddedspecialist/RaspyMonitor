/***************************************************************************
 *   Copyright (C) 2008 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it                                                *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 ***************************************************************************/
#include "nthCnt.h"
#include "conewirenet.h"

#define NOF_LITER_VALS 6
static float literScaleArray[] = {1.0, 10.0, 100.0, 1000.0, 2000.0, 5000.0};

#define NOF_WATT_VALS 8
static float wattScaleArray[] = {0.001, 0.01, 0.1, 1.0, 10.0, 100.0, 1000.0, 10000.0 };

/////////////////////////////////////////////////////
//                   Standard costructor
/////////////////////////////////////////////////////
CNTH_CNT::CNTH_CNT(const char* configString, CTimer *timer): CUPID2(configString)
{

    memset (m_Counters, 0x0, 4*sizeof(t_Counter));

    //TBI: Al momento c'è una sola configurazione come contaenergia completo
    //Contaenergia
//    m_Counters[0].counterType = CNT_CAL;
//    m_Counters[1].counterType = CNT_WATER;
//    m_Counters[2].counterType = CNT_WATER;
//    m_Counters[3].counterType = CNT_WATT;

    if (configString != 0x0)
    {
        //19-05-2009 inserisco i funzionamenti per il farneto
        m_IniLib.GetConfigParamInt(configString, "TIN1", &m_TempControllerAddr[0], -1);
        m_IniLib.GetConfigParamInt(configString, "TIN2", &m_TempControllerAddr[1], -1);
        
        m_IniLib.GetConfigParamInt(configString, "RIT1", &m_AnalogIOAddr[0], -1);
        m_IniLib.GetConfigParamInt(configString, "RIT2", &m_AnalogIOAddr[1], -1);

        //27/10/2009 Inserisco l'inizializzazione dei counter
        m_IniLib.GetConfigParamInt(configString, "CNT0TYPE", (int*)(&m_Counters[0].counterType), CNT_CAL);
        m_IniLib.GetConfigParamInt(configString, "CNT0SCALE", &m_Counters[0].scaleIndex, 0);

        m_IniLib.GetConfigParamInt(configString, "CNT1TYPE", (int*)(&m_Counters[1].counterType), CNT_WATER);
        m_IniLib.GetConfigParamInt(configString, "CNT1SCALE", &m_Counters[1].scaleIndex, 0);

        m_IniLib.GetConfigParamInt(configString, "CNT2TYPE", (int*)(&m_Counters[2].counterType), CNT_WATER);
        m_IniLib.GetConfigParamInt(configString, "CNT2SCALE", &m_Counters[2].scaleIndex, 0);

        m_IniLib.GetConfigParamInt(configString, "CNT3TYPE", (int*)(&m_Counters[3].counterType), CNT_WATT);
        m_IniLib.GetConfigParamInt(configString, "CNT3SCALE", &m_Counters[3].scaleIndex, 0);
        //Fine inizializzazione counter

        m_IniLib.GetConfigParamBool(configString, "ENABLED", &m_IsEnabled, false);
        m_IniLib.GetConfigParamBool(configString, "SUMMER", &m_IsSummer, true);
    }

    //Controllo se sono o meno in configurazione Farneto
    if ( (m_TempControllerAddr[0]==-1) || (m_TempControllerAddr[1]==-1) || (m_AnalogIOAddr[0]==-1) || (m_AnalogIOAddr[1]==-1)) m_Farneto = false;
    else m_Farneto = true;

    //Controllo la correttezza dell'inizializzazione dei counter
    for ( int i=0; i<4 ; i++ )
    {
        switch (m_Counters[i].counterType)
        {
            case CNT_CAL:
            case CNT_WATER:
            case CNT_WATT:
                break;
            default:
            {
                switch (i)
                {
                    case 0:{
                            m_Counters[i].counterType = CNT_CAL;
                        }
                        break;
                    case 1:
                    case 2:{
                            m_Counters[i].counterType = CNT_WATER;
                        }
                        break;
                    case 3:{
                            m_Counters[i].counterType = CNT_WATT;
                        }
                        break;
                }
            }
        }
    }


    memset (m_TempController, 0x0, 2*sizeof(CTempCtrl*)); //sonde di temperatura
    memset (m_AnalogIO, 0x0, 2*sizeof(CAnalogIO*));
    
    m_Hyst = 1.0;
    m_NetPtr = 0x0;
    m_FarnetoMode = false;
    
    m_FirstUpdate = false;
    m_ModuleInitOK = false;

    m_TypeOfTimerVal = TIMERVAL_DIGITAL;
    m_ControllerType = DEV_CNT;
}

/////////////////////////////////////////////////////
//                   Standard destructor
/////////////////////////////////////////////////////
CNTH_CNT::~CNTH_CNT()
{
}

/////////////////////////////////////////////////////
//                   InitDevice
/////////////////////////////////////////////////////
bool CNTH_CNT::InitDevice(CIniFileHandler *iniFileHandler)
{
    bool retVal = true;
    COneWireNet* netPtr = reinterpret_cast<COneWireNet*> ( m_NetPtr );

    //19-05-2009 -- Funzionamenti per il farneto
    if ((netPtr==0x0) || (m_Farneto==false) )
    {
        m_FarnetoMode = false;
    }
    else
    {
        int index = -1, netIdx = -1;
        
        for (int i = 0; i < 2; i++)
        {
            netIdx = netPtr->GetNetByMemoryAddress(m_TempControllerAddr[i]);
            index = netPtr->GetDeviceIndexByMemoryAddress(netIdx, m_TempControllerAddr[i]);
            
            if ((index > -1) && (netPtr->CheckControllerType(netIdx, index, DEV_TEMPCTRL)))
            {
                m_TempController[i] = ((CTempCtrl*)(netPtr->GetNetHandler(netIdx)->CtrlList[index]));
            }
        }
        
        for (int i = 0; i < 2; i++)
        {
            netIdx = netPtr->GetNetByMemoryAddress(m_AnalogIOAddr[i]);
            index = netPtr->GetDeviceIndexByMemoryAddress(netIdx, m_AnalogIOAddr[i]);
            
            if ((index > -1) && (netPtr->CheckControllerType(netIdx, index, DEV_AIAO)))
            {
                m_AnalogIO[i] = ((CAnalogIO*)(netPtr->GetNetHandler(netIdx)->CtrlList[index]));
            }
        }
        
        //Controllo se è tutto a posto
        if ( (!m_TempController[0]) || (!m_TempController[1]) || (!m_AnalogIO[0]) || (!m_AnalogIO[1]))
        {
            cout << "Attenzione si è verificato un errore nel collegamento degli I/O per il Farneto nel modulo di address: "<<m_Address<<endl;
            msDelay(2000);
            m_FarnetoMode = false;
            return false;
        }
        else
        {
            m_FarnetoMode = true;
        }
    }
    
//    //Leggo tutto quello che posso dalla scheda:
//    memset (mapMem, 0x0, 16*sizeof(uchar));
//
//    mapMem[0] = 0x01;
//
//    if (WriteToDevice(mapMem)){
//        for (int i = 0; i < 4; i++){
//            DecodeCounterConfig(i, mapMem[6+i]);
//        }
//    }
//    else {
//        retVal = false;
//    }

    if (!ReadCounters()) {
        retVal = false;
    }

    if (!UpdateCommonData()){
        retVal = false;
    }

    //Update the init flag
    m_ModuleInitOK = retVal;

    return retVal;
}

/////////////////////////////////////////////////////
//                   ReadCounters
/////////////////////////////////////////////////////
bool CNTH_CNT::ReadCounters(){

    uchar mapMem[16];
    bool retVal1=true, retVal2=true;

    //Leggo i dati del Counter 0
    memset ( mapMem, 0x00, 16*sizeof(uchar));
    mapMem[0] = 0x04;
    mapMem[15] = CalcCRC(mapMem);

    if ( WriteToDevice(mapMem) )
    {
        memcpy(&(m_Counters[0].countValue), &(mapMem[1]), 4);
        memcpy(&(m_Counters[0].heatEnergy), &(mapMem[5]), 4);
        memcpy(&(m_Counters[0].coldEnergy), &(mapMem[9]), 4);
        m_Counters[0].tMnd = (float)(mapMem[13])/2.0;
        m_Counters[0].tRip = (float)(mapMem[14])/2.0;
    }
    else
    {
        retVal1 = false;
    }

    //Leggo i dati dei Counter 1, 2 e 3
    memset ( mapMem, 0x00, 16*sizeof(uchar));
    mapMem[0] = 0x03;
    mapMem[15] = CalcCRC(mapMem);

    if ( WriteToDevice(mapMem) )
    {
        unsigned char adr=1;

        for ( int i=1 ; i<4 ; i++ )
        {
            memcpy(&(m_Counters[i].countValue), &(mapMem[adr]), 4);
            adr = adr + 4;
        }
    }
    else
    {
        retVal2 = false;
    }

    //Elaboro i dati ricevuti dalla scheda
    for ( int i=0 ; i<4 ; i++ )
    {
        switch ( m_Counters[i].counterType )
        {
            case CNT_CAL:{
                    m_CounterValue[i].val1 = m_Counters[i].heatEnergy;
                    m_CounterValue[i].val2 = m_Counters[i].coldEnergy;
                }
                break;

            case CNT_WATER:{
                    m_CounterValue[i].val1 = m_Counters[i].countValue * literScaleArray[m_Counters[i].scaleIndex];
                    m_CounterValue[i].val2 = 0;
                }
                break;

            case CNT_WATT:{
                m_CounterValue[i].val1 = m_Counters[i].countValue * wattScaleArray[m_Counters[i].scaleIndex];
                m_CounterValue[i].val2 = 0;
            }
        }
    }

    return (retVal1 && retVal2);
}


/////////////////////////////////////////////////////
//                   Update
/////////////////////////////////////////////////////
bool CNTH_CNT::Update(bool updateData)
{
    time_t actDateSec;

    time (&actDateSec);

    //First check if the module has been correctly programmed
    if (!m_ModuleInitOK)
    {
        if (!InitDevice())
        {
            //Aggiungere errore sull'inizializzazione
            return false;
        }
    }

//    if (actDateSec < m_LastUpdateTime + m_UpdateTime)
//    {
//        return true;
//    }

    if (ReadCounters())
    {
//        m_LastUpdateTime = actDateSec;
//        GenerateUpdateTime();
//        return true;

        //Ho letto correttamente i valori dei Counter e ora li visualizzo
        if (m_DebugLevel)
        {
            cout << "NTH-CNT indirizzo: " << m_Address << endl;
            cout << "\n";
            for ( int i=0 ; i<4 ; i++ )
            {
                switch ( m_Counters[i].counterType )
                {
                    case CNT_CAL: {
                            cout << "   Counter n: " << i << " Calorie: " << m_CounterValue[i].val1 << " Frigorie: " << m_CounterValue[i].val2 << endl;
                            cout << "   Temp. Mandata: " << m_Counters[i].tMnd << endl;
                            cout << "   Temp. Ripresa: " << m_Counters[i].tRip << endl;
                            cout << "\n";
                        }
                        break;

                    case CNT_WATER:{
                            cout << "   Counter n: " << i << " Litri Conteggiati: " << m_CounterValue[i].val1 << endl;
                            cout << "\n";
                        }
                        break;

                    case CNT_WATT:{
                            cout << "   Counter n: " << i << " Watt Conteggiati: " << m_CounterValue[i].val1 << endl;
                            cout << "\n";
                        }
                        break;
                        
                    default:{
                            cout << "   Counter n: " << i << " Non utilizzato " << endl;
                            cout << "\n";
                        }
                        break;
                }
            }
            cout << "NTH-CNT indirizzo: " << m_Address << " Commento: " <<  m_Comment << endl;
        }
    }
    else
    {
        //Messaggio di errore durante la lettura dei valori dalla uPID2
        cout << "NTH-CNT indirizzo: " << m_Address << endl;
        cout << "\n";
        cout << "   *** Errore durante la lettura dei valori dei contabilizzatori... ***" << endl;
        cout << "\n";
        cout << "NTH-CNT indirizzo: " << m_Address << " Commento: " <<  m_Comment << endl;
    }
    
    if (m_FarnetoMode)
    {
        ManageFarnetoData();
    }
    
    return false;
}

/////////////////////////////////////////////////////
//                   ClearCounter
/////////////////////////////////////////////////////
bool CNTH_CNT::ClearCounter( unsigned char nCnt )
{
    bool retVal = true;
    uchar mapMem[16];

    memset ( mapMem, 0x00, 16*sizeof(uchar));
    mapMem[0] = 0x07;
    mapMem[1] = nCnt;
    mapMem[15] = CalcCRC(mapMem);

    if ( WriteToDevice(mapMem) )
    {
        return retVal;
    }
    else
    {
        retVal = false;
    }

    return retVal;
}

/**
*Ritorna i valori di contabilizzazione memorizzati dal contatore
*in un array di int nel seguente ordine:
*Valore della grandezza contata (litri o watt)
*tempMnd
*tempRip
*Calorie
*Frigorie
*/
void CNTH_CNT::GetData(int cntIdx, float *destArray){

    if ((m_Counters[cntIdx].counterType == CNT_CAL) || (m_Counters[cntIdx].counterType == CNT_WATER)){

        if (m_Counters[cntIdx].isLiterPerImpulse)
        {
            destArray[0] = literScaleArray[m_Counters[cntIdx].scaleIndex]*m_Counters[cntIdx].countValue;
        }
        else
        {
          //Ad ogni impulso corrisponde un numero di litri pari a 1/scala
          destArray[0] = 1/(literScaleArray[m_Counters[cntIdx].scaleIndex])*m_Counters[cntIdx].countValue;
        }
    }
    else if (m_Counters[cntIdx].counterType == CNT_WATT){
        //Scalo in KW
        destArray[0] = m_Counters[cntIdx].countValue/wattScaleArray[m_Counters[cntIdx].scaleIndex];
    }

    destArray[1] = m_Counters[cntIdx].tMnd;
    destArray[2] = m_Counters[cntIdx].tRip;

    destArray[3] = m_Counters[cntIdx].heatEnergy;
    destArray[4] = m_Counters[cntIdx].coldEnergy;
}

/*************************************************************************
 * Function Name: DecodeCounterConfig
 * Parameters: none
 * Return: none
 * Description: Decodifica il byte dato e lo usa per configurare il counter
 *
 *************************************************************************/
//void CNTH_CNT::DecodeCounterConfig(int cntIdx, char config){
//  char mask = 0x01;
//  t_Counter *pCnt = &(m_Counters[cntIdx]);
//
//  //Decodifico l'indice della scala di valori di contabilizzazione
//  pCnt->scaleIndex = (config&0xE0)>>5;
//
//  switch (cntIdx) {
//    case 0: {
//      //Tipo contatore
//      if (config & mask){
//        pCnt->counterType = CNT_CAL;
//        //Conto anche le frigorie ?
//        if (config & (mask<<2)){
//          pCnt->countOnlyHeat = 1;
//        }
//        else {
//          pCnt->countOnlyHeat = 0;
//        }
//      }
//      else {
//        if (config & (mask<<4)){
//          pCnt->counterType = CNT_WATT;
//        }
//        else {
//          pCnt->counterType = CNT_WATER;
//        }
//     }
//
//     if (config & (mask<<3)){
//       pCnt->isLiterPerImpulse = 1;
//     }
//     else {
//       pCnt->isLiterPerImpulse = 0;
//     }
//
//    };break;
//    case 1:
//    case 2:
//    case 3:{
//      if (config&mask){
//        pCnt->counterType = CNT_WATT;
//      }
//      else {
//        pCnt->counterType = CNT_WATER;
//      }
//
//      if (config & (mask<<1)){
//        pCnt->isLiterPerImpulse = 1;
//      }
//      else {
//        pCnt->isLiterPerImpulse = 0;
//      }
//    };break;
//  }
//}


/*************************************************************************
 * Function Name: EncodeCounterConfig
 * Parameters: none
 * Return: none
 * Description: Prende le informazioni del contatore dato e le usa per codificare
 * in un byte la configurazione
 *
 *************************************************************************/
//char CNTH_CNT::EncodeCounterConfig(int cntIdx){
//  char retval = 0;
//  t_Counter *pCnt = &(m_Counters[cntIdx]);
//
//  //Scala valori di contabilizzazione
//  retval = retval | ((pCnt->scaleIndex)<<5);
//
//  switch (cntIdx){
//
//    case 0: {
//      if (pCnt->counterType == CNT_CAL){
//        retval = retval | 0x1;
//        if (pCnt->countOnlyHeat){
//          retval = retval | 0x4;
//        }
//      }
//      else if (pCnt->counterType == CNT_WATT){
//        retval = retval | 0x10;
//      }
//
//      if (pCnt->isLiterPerImpulse){
//        retval = retval | 0x08;
//      }
//    };break;
//    case 1:
//    case 2:
//    case 3:{
//      if (pCnt->counterType == CNT_WATT){
//        retval = retval | 0x01;
//      }
//
//      if (pCnt->isLiterPerImpulse){
//        retval = retval | 0x02;
//      }
//    };break;
//  }
//
//  return retval;
//}

//////////////////////////////////////////////////
//bool CNTH_CNT::GetInternalParameters(int counterIndex, uchar *destArray)
//{
//    memset(destArray, 0x0, 16*sizeof(uchar));
//
//    destArray[0] = 0x04;
//
//    if (WriteToDevice(destArray))
//    {
//        return true;
//    }
//    else
//    {
//        return false;
//    }
//
//}

////////////////////////////////////////////////
void CNTH_CNT::ManageFarnetoData()
{
    uchar mapmem[16];
    bool stateChanged = false;
    //Controllo lo stato globale, se non abilitato
    if (m_IsEnabled)
    {
        for (int i = 0; i < 2; i++)
        {
            float temp = ANALOG_ERRVAL;
            float setpoint = ANALOG_ERRVAL;
            //Acquisisco le temperature
            temp = m_TempController[i]->GetLastTemp();
            
            //Acquisisco i setpoint
            setpoint = AcquireSetpoint(i);
            
            //controllo gli stati
            if ((temp != TEMP_ERRVAL) && (setpoint != ANALOG_ERRVAL))
            {
                float absVal = 0.0;
                
                if (m_IsSummer)
                {
                    absVal = temp-setpoint;
                }
                else
                {
                    absVal = setpoint - temp;
                }
                
                if ( (absVal) > m_Hyst)
                {
                    if (!m_IsOn[i])
                    {
                        m_IsOn[i] = true;
                        stateChanged = true;
                    }
                }
                else if ( (absVal) < -m_Hyst)
                {
                    if (m_IsOn[i])
                    {
                        m_IsOn[i] = false;
                        stateChanged = true;
                    }
                }
            }
        }
        
        //Se cambiati aggiorno altrimenti esco
        if (stateChanged)
        {
            memset (mapmem, 0xff, 16);
            mapmem[0] = 0x05;
            mapmem[1] = 0x07; //Gli ripasso lo stato di tutto
            mapmem[2] = 1;    //Scheda abilitata
            mapmem[3] = m_IsOn[0]; //Stato DO1
            mapmem[4] = m_IsOn[1]; //Stato DO2
            mapmem[15] = CalcCRC(mapmem);
            
            //TODO segnalazione errori
            WriteToDevice(mapmem);
        }
    }
    else
    {
        //mando il messaggio alla scheda di off delle uscite
        memset (mapmem, 0xff, 16);
        mapmem[0] = 0x05;
        mapmem[1] = 0x01; //Gli passo solo lo stato di abilitazione generale
        mapmem[15] = CalcCRC(mapmem);
            
        //TODO segnalazione errori
        WriteToDevice(mapmem);
    }
    
}

///////////////////////////////////////////////////////////////////////////////////////////////
float CNTH_CNT::AcquireSetpoint(int index)
{
    double angle;
    int currentRegister;
    float setpoint = ANALOG_ERRVAL;
    
    //Sonda NTH
    currentRegister = m_AnalogIO[index]->ReadCurrentRegister();

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
            
        setpoint = angle * 20.0 / 270.0 + 10.0;


        return setpoint;
    }
    
    return ANALOG_ERRVAL;
}
