/***************************************************************************
 *   Copyright (C) 2007 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 ***************************************************************************/
 
#include "pidBlock.h"
#include "conewireengine.h"

CPIDBlock::CPIDBlock(const char* configString): CBlock(configString)
{
    CString tempString;
    
    //Read required parameters
    m_Parameters.push_back(m_LibIniReader.GetConfigParamFloat(configString, "KP", 0x0, 8.0));
    m_Parameters.push_back(m_LibIniReader.GetConfigParamFloat(configString, "KI", 0x0, 1.0));
    m_Parameters.push_back(m_LibIniReader.GetConfigParamFloat(configString, "KD", 0x0, 0.0));
    m_SetPoints.push_back(m_LibIniReader.GetConfigParamFloat(configString, "SP", 0x0, 20.0));
    m_IntegralState.push_back(0.0);
    m_DerivativeState.push_back(0.0);

    if (m_Parameters.at(1) != 0.0){
        m_MaxIntegralError.push_back(MAX_PID_OUTPUT/m_Parameters.at(1));
    }else {
        m_MaxIntegralError.push_back(MAX_PID_OUTPUT);
    }

    //read the parameters for the second PID
    //Questi li leggo comunque in questo modo i vettori hanno sempre la stessa dimensione
    m_Parameters.push_back(m_LibIniReader.GetConfigParamFloat(configString, "KP2", 0x0, 1.0));
    m_Parameters.push_back(m_LibIniReader.GetConfigParamFloat(configString, "KI2", 0x0, 0.1));
    m_Parameters.push_back(m_LibIniReader.GetConfigParamFloat(configString, "KD2", 0x0, 0.0));
    m_SetPoints.push_back(m_LibIniReader.GetConfigParamFloat(configString, "LIMH", 0x0, 20.0));
    m_SetPoints.push_back(m_LibIniReader.GetConfigParamFloat(configString, "LIML", 0x0, 10.0));
    m_IntegralState.push_back(0.0); //Due controlli per minima e massima
    m_IntegralState.push_back(0.0); //Due controlli per minima e massima

    m_DerivativeState.push_back(0.0);//Due controlli per minima e massima
    m_DerivativeState.push_back(0.0);//Due controlli per minima e massima
    if (m_Parameters.at(4) != 0.0){
        m_MaxIntegralError.push_back(MAX_PID_OUTPUT/m_Parameters.at(4));
        m_MaxIntegralError.push_back(MAX_PID_OUTPUT/m_Parameters.at(4));
    }
    else {
        m_MaxIntegralError.push_back(MAX_PID_OUTPUT);
        m_MaxIntegralError.push_back(MAX_PID_OUTPUT);

    }
    
    m_LibIniReader.GetConfigParamBool( configString, "SUMMER", &m_IsSummer, 0);
    
    m_LibIniReader.GetConfigParamFloat(configString, "MFACTOR1",  &m_normalizeM1, 1.0);
    m_LibIniReader.GetConfigParamFloat( configString, "QFACTOR1",  &m_normalizeQ1, 0.0);

    m_LibIniReader.GetConfigParamFloat(configString, "MFACTOR2",  &m_normalizeM2, 1.0);
    m_LibIniReader.GetConfigParamFloat( configString, "QFACTOR2",  &m_normalizeQ2, 0.0);

}


CPIDBlock::~CPIDBlock()
{
}

void CPIDBlock::GetSummer(){

    if (IsInputConnected(PID_SUMMER_INPUT)){
        if (IsInputValid(PID_SUMMER_INPUT)){
            float summer;
            GetDataFromInput(PID_SUMMER_INPUT, &summer);
            m_IsSummer = (bool)summer;
        }
    }
}
void CPIDBlock::GetSetpoint() {
    
    if (IsInputConnected(PID_SP_INPUT)){
        if (IsInputValid(PID_SP_INPUT)){
            GetDataFromInput(PID_SP_INPUT, &m_SetPoints[0]);
        }
    }
}
///////////////////////////////////////////////////
//              Update
///////////////////////////////////////////////////
bool CPIDBlock::Update( )
{
    GetSummer();
    GetSetpoint();

    if (IsInputConnected(PID2_INDEX)){
        return UpdateLMDControl();
    }
    else {
        return UpdateSimpleControl();
    }
}

///////////////////////////////////////////////////
//              Integrate
///////////////////////////////////////////////////
float CPIDBlock::Integrate( int pidIndex, float value )
{
    m_IntegralState[pidIndex]+=value;

    if (m_IntegralState[pidIndex] > m_MaxIntegralError[pidIndex]){

        m_IntegralState[pidIndex] = m_MaxIntegralError[pidIndex];
        
    } else if (m_IntegralState[pidIndex] < 0)
    {
        m_IntegralState[pidIndex] = 0;
    }
    
    return m_IntegralState[pidIndex];
}

///////////////////////////////////////////////////
//              Derivate
///////////////////////////////////////////////////
float CPIDBlock::Derivate( int pidIndex, float value )
{
    float retVal = 0.0;

    retVal = value - m_DerivativeState[pidIndex];
    
    m_DerivativeState[pidIndex] = value;
    
    return retVal;
    
}


///////////////////////////////////////////////////
//              ExecCommand
///////////////////////////////////////////////////
bool CPIDBlock::ExecCommand(CXMLUtil* xmlUtil, bool* commandRetCode){

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
        com.putValue("BLOCKEXEC","PidParameters");

        com.putValue("SP",m_SetPoints.at(0));
        com.putValue("SPH", m_SetPoints.at(1));
        com.putValue("SPL",m_SetPoints.at(2));

        com.putValue("KP",m_Parameters.at(0));
        com.putValue("KI",m_Parameters.at(1));
        com.putValue("KD",m_Parameters.at(2));

        com.putValue("KP2",m_Parameters.at(3));
        com.putValue("KI2",m_Parameters.at(4));
        com.putValue("KD2",m_Parameters.at(5));

        com.putValue("SUMMER", m_IsSummer);

        com.putValue("MFACTOR1",m_normalizeM1);
        com.putValue("QFACTOR1", m_normalizeQ1);

        com.putValue("MFACTOR2", m_normalizeM2);
        com.putValue("QFACTOR2", m_normalizeQ2);

        CString message = com.getXMLValue();
        eng->WriteOnInterfacePorts(message.c_str(), message.size());

    }
    else if (command.ToUpper() == "SETPARAMETERS"){

        *commandRetCode = ParseCommandArgument(xmlUtil,"SP",&(m_SetPoints[0])) || (*commandRetCode);
        *commandRetCode = ParseCommandArgument(xmlUtil,"LIMH",&(m_SetPoints[1])) || (*commandRetCode);
        *commandRetCode = ParseCommandArgument(xmlUtil,"LIML",&(m_SetPoints[2])) || (*commandRetCode);
        *commandRetCode = ParseCommandArgument(xmlUtil,"KP",  &(m_Parameters[0])) || (*commandRetCode);
        *commandRetCode = ParseCommandArgument(xmlUtil,"KI",  &(m_Parameters[1])) || (*commandRetCode);
        *commandRetCode = ParseCommandArgument(xmlUtil,"KD",  &(m_Parameters[2])) || (*commandRetCode);
        *commandRetCode = ParseCommandArgument(xmlUtil,"KP2", &(m_Parameters[3])) || (*commandRetCode);
        *commandRetCode = ParseCommandArgument(xmlUtil,"KI2", &(m_Parameters[4])) || (*commandRetCode);
        *commandRetCode = ParseCommandArgument(xmlUtil,"KD2", &(m_Parameters[5])) || (*commandRetCode);
        float tempVal = -1.0;
        *commandRetCode = ParseCommandArgument(xmlUtil,"SUMMER",(float*)(&tempVal)) || (*commandRetCode);
        if (tempVal == 0.0){
            m_IsSummer = false;
        }
        else if (tempVal == 1.0){
            m_IsSummer = true;
        }

        *commandRetCode = ParseCommandArgument(xmlUtil,"MFACTOR1",&m_normalizeM1) || (*commandRetCode);
        *commandRetCode = ParseCommandArgument(xmlUtil,"QFACTOR1",&m_normalizeQ1) || (*commandRetCode);
        *commandRetCode = ParseCommandArgument(xmlUtil,"MFACTOR2",&m_normalizeM2) || (*commandRetCode);
        *commandRetCode = ParseCommandArgument(xmlUtil,"QFACTOR2",&m_normalizeQ2) || (*commandRetCode);

        if (m_Parameters.at(1) != 0.0){
            m_MaxIntegralError.at(0) = MAX_PID_OUTPUT/m_Parameters.at(1);
        }
        else {
            m_MaxIntegralError.at(0) = MAX_PID_OUTPUT;
        }

        if (m_Parameters.at(4) != 0.0){
            m_MaxIntegralError.at(1) = MAX_PID_OUTPUT/m_Parameters.at(4);
            m_MaxIntegralError.at(2) = MAX_PID_OUTPUT/m_Parameters.at(4);
        }
        else {
            m_MaxIntegralError.at(1) = MAX_PID_OUTPUT;
            m_MaxIntegralError.at(2) = MAX_PID_OUTPUT;
        }
    }

    return true;
    
}

void CPIDBlock::SendBlockInfo()
{
    CBlock::SendBlockInfo();
}
////////////////////////////////////////////////////////////////////////////////
bool CPIDBlock::UpdateSimpleControl(){

    float input1Val, normInput1Val, error1;
    float pid1_Output;

    try {
        //Get the inputs
        if (IsInputValid(PID1_INDEX))
        {
            if (!GetDataFromInput(PID1_INDEX,  &input1Val))
            {
                throw "Impossibile leggere ingresso";
            }
        }
        else
        {
            //TODO da migliorare
            m_OutputVector.at(PID1_INDEX).isValid = false;
            throw "Ingresso non valido";
        }

        //Normalize the inputs
        normInput1Val = m_normalizeM1 * input1Val + m_normalizeQ1;

        //Calculate error
        if (m_IsSummer)
        {
            error1 = normInput1Val - m_SetPoints.at(0);
        }
        else
        {
            error1 = m_SetPoints.at(0) - normInput1Val;
        }

        //Apply the PID parameters
        pid1_Output = m_Parameters.at(0)*error1 + m_Parameters.at(1)*Integrate( PID1_INDEX, error1 ) + m_Parameters.at(2)*Derivate( PID1_INDEX, error1 );

        if (pid1_Output > MAX_PID_OUTPUT)
        {
            pid1_Output = MAX_PID_OUTPUT;
        }
        else if ( pid1_Output < 0 )
        {
            pid1_Output = 0.0;
        }


        return SetOutputVal(PID1_INDEX, pid1_Output);
    }
    catch(char const* ex){
        cout << ex<<endl;
        return SetOutputVal(PID1_INDEX, -100.0,false);
        return false;
    }

}
////////////////////////////////////////////////////////////////////////////////
bool CPIDBlock::UpdateLMDControl()
{

    float input1Val, normInput1Val, error1, input2Val, normInput2Val, error2, error3;
    float pid1_Output, pid2_Output, pid3_Output;
    bool retVal = false;

    try {
            //Get the inputs
        if (IsInputValid(PID1_INDEX))
        {
            if (!GetDataFromInput(PID1_INDEX,  &input1Val))
            {
                throw "Impossibile leggere ingresso";
            }
        }
        else
        {
            //TODO da migliorare
            m_OutputVector.at(PID1_INDEX).isValid = false;
            throw "Ingresso non valido";
        }

        //Normalize the inputs
        normInput1Val = m_normalizeM1 * input1Val + m_normalizeQ1;

        //TODO se è con compensazione esterna va cambiato il SP
        //Calculate error
        if (m_IsSummer)
        {
            error1 = normInput1Val - m_SetPoints.at(0);
        }
        else
        {
            error1 = m_SetPoints.at(0) - normInput1Val;
        }

        //Apply the PID parameters
        pid1_Output = m_Parameters.at(0)*error1 + m_Parameters.at(1)*Integrate( PID1_INDEX, error1 ) + m_Parameters.at(2)*Derivate( PID1_INDEX, error1 );

        //Set the analog output: 255 means the output is low, 0 is at Vmax
        if (pid1_Output > MAX_PID_OUTPUT)
        {
            pid1_Output = MAX_PID_OUTPUT;
        }
        else if ( pid1_Output < 0 )
        {
            pid1_Output = 0;
        }

        //Get the second input
        if (!GetDataFromInput(PID2_INDEX,  &input2Val))
        {
            throw "Impossibile leggere ingresso";
        }

        //Normalize the inputs
        normInput2Val = m_normalizeM2 * input2Val + m_normalizeQ2;

        //Controllo di non mandare aria troppo calda o troppo fredda nelle diverse stagioni
        //Il punto chiave e' che il PID2 dice sempre di chiudere mentre il PID3 dice di aprire
        if (m_IsSummer)
        {
            error2 = m_SetPoints.at(2) - normInput2Val;
            error3 = normInput2Val - m_SetPoints.at(1);
        }
        else
        {
            error2 = normInput2Val - m_SetPoints.at(1);
            error3 = m_SetPoints.at(2) - normInput2Val;
        }

        //Apply the PID parameters
        pid2_Output = m_Parameters.at(3)*error2 + m_Parameters.at(4)*Integrate( PID2_INDEX, error2 ) + m_Parameters.at(5)*Derivate( PID2_INDEX, error2 );
        pid3_Output = m_Parameters.at(3)*error3 + m_Parameters.at(4)*Integrate( PID3_INDEX, error3 ) + m_Parameters.at(5)*Derivate( PID3_INDEX, error3 );


        //Set the analog output: 255 means the output is low, 0 is at Vmax
        if (pid2_Output > MAX_PID_OUTPUT)
        {
            pid2_Output = MAX_PID_OUTPUT;
        }
        else if ( pid2_Output < 0 )
        {
            pid2_Output = 0;
        }

        if (pid3_Output > MAX_PID_OUTPUT)
        {
            pid3_Output = MAX_PID_OUTPUT;
        }
        else if ( pid3_Output < 0 )
        {
            pid3_Output = 0;
        }

        retVal = SetOutputVal( PID1_INDEX, pid1_Output - pid2_Output + pid3_Output);

    }
    catch (char const* ex)
    {

        cout<<ex<<endl;
        SetOutputVal( PID1_INDEX, -100.0,false);
        retVal = false;
    }

    return retVal;
}
