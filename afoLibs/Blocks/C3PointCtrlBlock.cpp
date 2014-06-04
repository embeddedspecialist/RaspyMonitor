/* 
 * File:   C3PointCtrlBlock.cpp
 * Author: amirrix
 * 
 * Created on 19 gennaio 2010, 17.44
 *
 */

#include "C3PointCtrlBlock.h"
#include "conewireengine.h"

C3PointCtrlBlock::C3PointCtrlBlock(const char* configString): CBlock(configString)
{
    CString tempString;

    //Read required parameters

    m_Setpoint = m_LibIniReader.GetConfigParamFloat(configString, "SP", 0x0, 20.0);
    m_SetpointH = m_LibIniReader.GetConfigParamFloat(configString, "LIMH", 0x0, 35.0);
    m_SetpointL = m_LibIniReader.GetConfigParamFloat(configString, "LIML", 0x0, 15.0);

    m_MoveTimeOut = m_LibIniReader.GetConfigParamFloat(configString, "MOVETIME", 0x0, 1.0);
    m_NullZone = m_LibIniReader.GetConfigParamFloat(configString, "HYST", 0x0, 1.0);

    m_LibIniReader.GetConfigParamBool( configString, "SUMMER", &m_IsSummer, 0);

    m_TimeOfOpening = m_TimeOfClosing = 0;

    m_ClockIdx = mainClockManager.GetClock();

    mainClockManager.ResetClock(m_ClockIdx);

    //TODO
//    m_LibIniReader.GetConfigParamFloat(configString, "MFACTOR1",  &m_normalizeM1, 1.0);
//    m_LibIniReader.GetConfigParamFloat( configString, "QFACTOR1",  &m_normalizeQ1, 0.0);
//
//    m_LibIniReader.GetConfigParamFloat(configString, "MFACTOR2",  &m_normalizeM2, 1.0);
//    m_LibIniReader.GetConfigParamFloat( configString, "QFACTOR2",  &m_normalizeQ2, 0.0);

}

C3PointCtrlBlock::~C3PointCtrlBlock() {
}

bool C3PointCtrlBlock::Update(){
    bool retVal = false;

    for (int i = 0; i < 4; i++){
        if (!IsInputValid(i)){
            SetOutputVal(0,-100.0,false);
            SetOutputVal(1,-100.0,false);
            return false;
        }
    }

    GetSummer();
    GetSetpoint();
    
    if (IsInputConnected(C3P_LMD_IN)){
        return UpdateLMDControl();
    }
    else {
        return UpdateSimpleControl();
    }
}

bool C3PointCtrlBlock::UpdateSimpleControl(){

    float error, input;
    bool retVal;
    
    GetDataFromInput(C3P_DIRECT_IN, &input);
    
    error = GetError(input, m_Setpoint, false);

    if (error > m_NullZone/2.0)
    {
        retVal = Open();
    }
    else if (error < (-m_NullZone/2.0))
    {
        retVal = Close();
    }
    else
    {
        //Area nulla, fermo la valvola
        retVal = Stop();
    }

    return retVal;
}

bool C3PointCtrlBlock::UpdateLMDControl(){
    float errorDirect, errorH, errorL;
    float inputDirect, inputLMD;
    bool retVal = false;

    GetDataFromInput(C3P_DIRECT_IN, &inputDirect);
    GetDataFromInput(C3P_LMD_IN, &inputLMD);

    errorDirect = GetError(inputDirect, m_Setpoint, false);
    errorH = GetError(inputLMD, m_SetpointH, true);
    //Cambio di segno per ottenere sempre un valore positivo
    errorL = (-1.0)*(GetError(inputLMD, m_SetpointL, true));

    //Calcolo cosa devo fare: se l'errore sui limiti di mandata si attiva devo dare precedenza a lui
    if ((errorH > 0.0) || (errorL > 0.0))
    {
        //Ho superato il limite di mandata superiore: chiudo la valvola
        retVal = Close();
    }
    else
    {
        if (errorDirect > m_NullZone/2.0)
        {
            retVal = Open();
        }
        else if (errorDirect < (-m_NullZone/2.0))
        {
            retVal = Close();
        }
        else
        {
            //Area nulla, fermo la valvola
            retVal = Stop();
        }
    }

    return retVal;
}

void C3PointCtrlBlock::GetSummer(){
    float summer;
    GetDataFromInput(C3P_SUMMER_IN, &summer);

    m_IsSummer = (bool)summer;
}

void C3PointCtrlBlock::GetSetpoint(){

    GetDataFromInput(C3P_SETPOINT_IN, &m_Setpoint);
}

bool C3PointCtrlBlock::Open(){
    
    bool retVal = false;
    int actTime = mainClockManager.GetTimeElapsed(m_ClockIdx, false);

    if (m_TimeOfOpening == 0)
    {
        //Devo aprire la valvola
        if( SetOutputVal(C3P_CLOSE,0.0,true))
        {
            retVal = SetOutputVal(C3P_OPEN,1.0,true);
        }

                    //Tutto OK
        if (retVal)
        {
            m_TimeOfOpening = mainClockManager.GetTimeElapsed(m_ClockIdx, false);
            m_TimeOfClosing = 0;
//            m_IsFullClosed = false;
//            m_ControllerStatus = OPENING;
        }
    }
    else
    {
        if ( (actTime - m_TimeOfOpening) >= m_MoveTimeOut)
        {
            //Fermo al valvola perche' e' da troppo tempo che ho il comando attivo
            retVal = SetOutputVal(C3P_OPEN,0.0,true);
//            m_IsFullOpen = true;
//            m_IsFullClosed = false;
//            m_ControllerStatus = STOPPED;
        }
        else
        {
//            m_IsFullOpen = false;
//            m_IsFullClosed = false;
            retVal = true;
        }
    }

    return retVal;
}

bool C3PointCtrlBlock::Close(){
    bool retVal;
    int actTime;

    actTime = mainClockManager.GetTimeElapsed(m_ClockIdx, false);

    if (m_TimeOfClosing == 0)
    {
        //Devo chiudere la valvola
        if( SetOutputVal(C3P_OPEN,0.0,true))
        {
            retVal = SetOutputVal(C3P_CLOSE,1.0,true);
        }

        //Tutto OK
        if (retVal)
        {
            m_TimeOfClosing = mainClockManager.GetTimeElapsed(m_ClockIdx, false);
            m_TimeOfOpening = 0;
//            m_IsFullOpen = false;
//            m_ControllerStatus = CLOSING;
        }
    }
    else
    {
        if ( (actTime - m_TimeOfClosing) >= m_MoveTimeOut)
        {
            //Fermo al valvola perche' e' da troppo tempo che ho il comando attivo
            retVal = SetOutputVal(C3P_CLOSE,0.0,true);
//            m_IsFullClosed = true;
//            m_ControllerStatus = STOPPED;
        }
        else
        {
//            m_IsFullOpen = false;
//            m_IsFullClosed = false;
            retVal = true;
        }
    }

    return retVal;
}

bool C3PointCtrlBlock::Stop(){
    bool retVal;
    retVal = SetOutputVal(C3P_CLOSE,0.0,true);
    retVal = SetOutputVal(C3P_OPEN,0.0,true) && retVal;
    m_TimeOfOpening = 0;
    m_TimeOfClosing = 0;
//    m_ControllerStatus = STOPPED;

    return retVal;
}

/////////////////////////////////////////////////////////
//  GetError
/////////////////////////////////////////////////////////
float C3PointCtrlBlock::GetError(float temp, float setpoint, bool isLmd)
{
    if (isLmd)
    {
        return temp - setpoint;
    }
    else if (m_IsSummer)
    {
        return temp - setpoint;
    }
    else
    {
        return setpoint - temp;
    }
}

///////////////////////////////////////////////////
//              ExecCommand
///////////////////////////////////////////////////
bool C3PointCtrlBlock::ExecCommand(CXMLUtil* xmlUtil, bool* commandRetCode){

    COneWireEngine *eng = reinterpret_cast<COneWireEngine*>(m_EngPtr);

    if (!IsCommandForMe(xmlUtil)){
        return false;
    }

    //KP, KI, KD, SP, ADDR, SUMMER, SPH, SPL, MFACTOR1,QFACTOR1,MFACTOR2,QFACTOR2,COMMENT
     *commandRetCode = false;
    //Ora interpreto il messaggio
     CString command = xmlUtil->GetStringParam("BLOCKEXEC");
    if (command.ToUpper() == "GETPARAMETERS"){
        Cmd com("BLOCK");
        com.putValue("ADDRESS",m_BlockAddress);
        com.putValue("BLOCKEXEC","C3PointParameters");

        com.putValue("SP",m_Setpoint);
        com.putValue("LIMH", m_SetpointH);
        com.putValue("LIML",m_SetpointL);

        com.putValue("HYST",m_NullZone);
        com.putValue("MOVETIME",m_MoveTimeOut);


        com.putValue("SUMMER", m_IsSummer);

        com.putValue("MFACTOR1",m_NormalizeM1);
        com.putValue("QFACTOR1", m_NormalizeQ1);

        com.putValue("MFACTOR2", m_NormalizeM2);
        com.putValue("QFACTOR2", m_NormalizeQ2);

        CString message = com.getXMLValue();
        eng->WriteOnInterfacePorts(message.c_str(), message.size());

    }
    else if (command.ToUpper() == "SETPARAMETERS"){

        *commandRetCode = ParseCommandArgument(xmlUtil,"SP",&m_Setpoint) || (*commandRetCode);
        *commandRetCode = ParseCommandArgument(xmlUtil,"LIMH",&m_SetpointH)|| (*commandRetCode);
        *commandRetCode = ParseCommandArgument(xmlUtil,"LIML",&m_SetpointL)|| (*commandRetCode);
        *commandRetCode = ParseCommandArgument(xmlUtil,"HYST",&m_NullZone)|| (*commandRetCode);
        *commandRetCode = ParseCommandArgument(xmlUtil,"MOVETIME",&m_MoveTimeOut)|| (*commandRetCode);
        float tempVal = -1.0;
        *commandRetCode = ParseCommandArgument(xmlUtil,"SUMMER",(float*)(&tempVal)) || (*commandRetCode);
        if (tempVal == 0.0){
            m_IsSummer = false;
        }
        else if (tempVal == 1.0){
            m_IsSummer = true;
        }
        *commandRetCode = ParseCommandArgument(xmlUtil,"MFACTOR1",&m_NormalizeM1)|| (*commandRetCode);
        *commandRetCode = ParseCommandArgument(xmlUtil,"QFACTOR1",&m_NormalizeQ1)|| (*commandRetCode);
        *commandRetCode = ParseCommandArgument(xmlUtil,"MFACTOR2",&m_NormalizeM2)|| (*commandRetCode);
        *commandRetCode = ParseCommandArgument(xmlUtil,"QFACTOR2",&m_NormalizeQ2)|| (*commandRetCode);
        
    }

    return true;

}