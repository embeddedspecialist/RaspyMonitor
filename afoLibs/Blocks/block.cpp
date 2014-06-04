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
 
#include "block.h"
#include "vcontroller.h"
#include "conewireengine.h"


CBlock::CBlock(const char *configString)
{
    m_ConfigString = configString;
    m_ConfigBlockIndex = -1;
    m_ConfigSubSystemIndex = -1;
    m_NetPtr = 0x0;
    m_EngPtr = 0x0;
    m_BlockType = BLK_NONE;
    m_BlockAddress = -1;
}


CBlock::~CBlock()
{
}

void CBlock::ResetInputChannels(){
    m_InputVector.clear();
}
////////////////////////////////////////////////////////////////////
/////               SetInput
////////////////////////////////////////////////////////////////////
bool CBlock::SetInputChannel( CBlock * newInput, int newChannel, CVController* newController )
{
    t_BlockInputData newInputData;
    
    newInputData.sourceBlock = newInput;
    newInputData.sourceChannel = newChannel;
    newInputData.sourceController = newController;
    
    try
    {
        m_InputVector.push_back(newInputData);
        return true;
    }
    catch (...)
    {
        //TODO da gestire
        return false;
    }
}
////////////////////////////////////////////////////////////////////
/////               SetInputCommandChannel
////////////////////////////////////////////////////////////////////
bool CBlock::SetCommandChannel( CBlock * newInput, int newChannel, CVController* newController )
{
    t_BlockInputData newInputData;

    newInputData.sourceBlock = newInput;
    newInputData.sourceChannel = newChannel;
    newInputData.sourceController = newController;

    try
    {
        m_CommandInputVector.push_back(newInputData);
        return true;
    }
    catch (...)
    {
        //TODO da gestire
        return false;
    }
}
////////////////////////////////////////////////////////////////////
/////               SetupBlock
////////////////////////////////////////////////////////////////////
bool CBlock::SetupBlock(void *netPtr, void *engPtr )
{
    bool retVal = false;
    CString tempString;
    int i;
    COneWireNet *pNet = reinterpret_cast<COneWireNet*>(netPtr);

    m_NetPtr = netPtr;
    m_EngPtr = engPtr;
    
    try
    {
        //m_ConfigSubSystemIndex = 0;
        
        //First get the type
        m_LibIniReader.GetConfigParamString( m_ConfigString, "NAME", &tempString, "NODEV");
        
        for (i = BLK_NONE; i < BLK_NUMTOT; i++)
        {
            if (tempString == Block_strings[i])
            {
                break;
            }
        }
        
        if (i < BLK_NUMTOT)
        {
            m_BlockType = (e_BlockType)i;
        }
        
        m_LibIniReader.GetConfigParamInt( m_ConfigString, "ADDR", &m_BlockAddress, -1);
        
        for (i = 0; i < MAX_NUM_OUTPUT; i++)
        {
            CString outputString = "OUTPUT";
            outputString += i+1;
            
            m_LibIniReader.GetConfigParamString( m_ConfigString, outputString.c_str(), &tempString, "");
            
            if (tempString.size() == 0)
            {
                //No more outputs found, exit from the loop
                break;
            }
            else
            { 
                t_BlockOutputData newOutput;

                newOutput.outValue = -100.0;

                tempString.ToUpper();

                if (tempString == "STORE")
                {
//                    newOutput.outController = 0x0;
                    m_OutputVector.push_back(newOutput);
                }
                else if (tempString.at(0) == 'A')
                {
                    //Vettore degli indirizzi
                    vector<string> addressVector;
                    tempString.Split(addressVector,"-");

                    for (int i =0; i < addressVector.size(); i++){
                        //Devo cilcare se ho piu' uscite
                        int address = atoi (addressVector.at(i).substr(1).c_str());
                        CVController *ctrl = pNet->GetControllerHndlrByMemoryAddress(address);

                        if (ctrl != 0x0)
                        {
                            newOutput.outControllerList.push_back(ctrl);
                        }
                        else
                        {
                            //TODO da gestire
                        }
                    }

                    m_OutputVector.push_back(newOutput);
                }
            }
        }
        
        m_LibIniReader.GetConfigParamString( m_ConfigString, "COMMENT", &m_Comment, "NA");
        retVal = true;
    }
    catch (...)
    {
        //TODO Da gestire
    }

    return retVal;
}

////////////////////////////////////////////////////////////////////
/////               GetOutput
////////////////////////////////////////////////////////////////////
bool CBlock::GetOutputData( float * destination, int outIndex )
{   
    try
    {
        if (m_OutputVector.at(outIndex).isValid)
        {
            *destination = m_OutputVector.at(outIndex).outValue;
            return true;
        }
        else
        {
            return false;
        }
    }
    catch (...)
    {
        return false;
    }
}

////////////////////////////////////////////////////////////////////
/////               GetDataFromInput
////////////////////////////////////////////////////////////////////
int CBlock::GetDataFromInput( int inputIndex, float *dest )
{
    int retVal = 0;
    unsigned long int timeOfData;
    
    try
    {
        if ((dest == 0x0) || (!IsInputConnected(inputIndex)))
        {
            return 0;
        }
        
        //Check if the input is another block or a controller
        if (m_InputVector.at(inputIndex).sourceController == 0x0)
        {
            retVal = m_InputVector.at(inputIndex).sourceBlock->GetOutputData( dest, m_InputVector.at(inputIndex).sourceChannel);
            //TODO da controllare questa logica: per ora copio il tempo del dato originale nel dato nuovo ma potrei anche ricalcolare il tempo
            //per riflettere il fatto che questo blocco ha acquisito il tempo in un momento diverso
//            m_InputVector.at(inputIndex).sourceBlock->GetOutputValidityAndTime(&(m_InputVector.at(inputIndex).timeOfData), inputIndex);
        }
        else
        {
            t_DataVal data;
            if (!IsInputValid(inputIndex))
            {
                return -1;
            }

            data = m_InputVector.at(inputIndex).sourceController->GetDataStructure();

            *dest = data.floatData[m_InputVector.at(inputIndex).sourceChannel];
//            *dest = m_InputVector.at(inputIndex).sourceController->GetData();
            retVal = 1;
        }
    }
    catch (...)
    {
        *dest = -100.0;
    }
    
    return retVal;
}
////////////////////////////////////////////////////////////////////
/////               GetDataFromCommand
////////////////////////////////////////////////////////////////////
int CBlock::GetDataFromCommand( int inputIndex, float *dest )
{
    int retVal = 0;
    unsigned long int timeOfData;

    try
    {
        if (dest == 0x0)
        {
            return 0;
        }

        //Check if the input is another block or a controller
        if (m_CommandInputVector.at(inputIndex).sourceController == 0x0)
        {
            retVal = m_CommandInputVector.at(inputIndex).sourceBlock->GetOutputData( dest, m_CommandInputVector.at(inputIndex).sourceChannel);
            //TODO da controllare questa logica: per ora copio il tempo del dato originale nel dato nuovo ma potrei anche ricalcolare il tempo
            //per riflettere il fatto che questo blocco ha acquisito il tempo in un momento diverso
//            m_InputVector.at(inputIndex).sourceBlock->GetOutputValidityAndTime(&(m_InputVector.at(inputIndex).timeOfData), inputIndex);
        }
        else
        {

            if (!IsCommandValid(inputIndex))
            {
                return -1;
            }

            *dest = m_CommandInputVector.at(inputIndex).sourceController->GetData();
            retVal = 1;
        }
    }
    catch (...)
    {
        *dest = -100.0;
    }

    return retVal;
}
////////////////////////////////////////////////////////////////////
/////               IsInputValid
////////////////////////////////////////////////////////////////////
bool CBlock::IsInputValid( int inputIndex )
{
    bool retVal = false;
    
    try
    {
        //Check if the input is another block or an address
        if (m_InputVector.at(inputIndex).sourceBlock != 0x0)
        {
            //TODO
            return m_InputVector.at(inputIndex).sourceBlock->GetOutputValidityAndTime( 0x0, m_InputVector.at(inputIndex).sourceChannel);
        }
        else if (m_InputVector.at(inputIndex).sourceController != 0x0)
        {
            retVal = m_InputVector.at(inputIndex).sourceController->IsDataValid() ;
        }
        else
        {
            //E' un ingresso "finto" quindi e' sempre valido
            return true;
        }
    }
    catch (...)
    {
       //TODO da riempire 
    }
    
    return retVal;
}

////////////////////////////////////////////////////////////////////
/////               IsCommandValid
////////////////////////////////////////////////////////////////////
bool CBlock::IsCommandValid( int commandIndex )
{
    bool retVal = false;

    try
    {
        //Check if the input is another block or an address
        if (m_CommandInputVector.at(commandIndex).sourceController == 0x0)
        {
            //TODO
            return m_CommandInputVector.at(commandIndex).sourceBlock->GetOutputValidityAndTime( 0x0, m_InputVector.at(commandIndex).sourceChannel);
        }
        else
        {
            retVal = m_CommandInputVector.at(commandIndex).sourceController->IsDataValid() ;
        }
    }
    catch (...)
    {
       //TODO da riempire
    }

    return retVal;
}


////////////////////////////////////////////////////////////////////
/////               SetOutputVal
////////////////////////////////////////////////////////////////////
bool CBlock::SetOutputVal( int outputIndex, float value, bool isValid )
{
    unsigned long int timeOfData;
    struct timeval actTime;
    
    gettimeofday(&actTime, NULL);
    timeOfData = actTime.tv_sec*1000 + actTime.tv_usec/1000;
    
    try
    {
        //The value must be stored locally
        m_OutputVector.at(outputIndex).outValue = value;
        m_OutputVector.at(outputIndex).isValid = isValid;
        
        //Se il dato non è valido non aggiorno il suo tempo
        if (isValid)
        {
            m_OutputVector.at(outputIndex).timeOfData = timeOfData;
        }

        if ((m_OutputVector.at(outputIndex).outControllerList.size() > 0) && isValid)
        {
            bool retVal = true;
            //TODO: inserisco il controllo per l'output veloce:
            for (int i = 0 ; i < m_OutputVector.at(outputIndex).outControllerList.size(); i++){
                retVal = m_OutputVector.at(outputIndex).outControllerList.at(i)->SetVal(value) && retVal;
            }

            return retVal;
        }
    }
    catch (...)
    {
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////
/////               SetConfigIndexes
////////////////////////////////////////////////////////////////////
void CBlock::SetConfigIndexes( int subSystemIdx, int blockIdx )
{
    m_ConfigSubSystemIndex = subSystemIdx;
    m_ConfigBlockIndex = blockIdx;
}

//////////////////////////////////////////////////////////////////////
///////               IsInputConnected
//////////////////////////////////////////////////////////////////////
bool CBlock::IsInputConnected(unsigned int inputIndex){
    if (m_InputVector.size() <= inputIndex) {
        return false;
    }

    if ((m_InputVector.at(inputIndex).sourceBlock == 0x0) &&
        (m_InputVector.at(inputIndex).sourceController == 0x0)) {
            return false;
        }
    
    return true;

}
//////////////////////////////////////////////////////////////////////
///////               AllInputsFromMemoryMap
//////////////////////////////////////////////////////////////////////
//bool CBlock::AreAllInputsFromMemoryMap( )
//{
//    bool retVal = true;
//    int inputIdx;
//
//    for (inputIdx = 0; inputIdx < m_InputVector.size(); inputIdx++)
//    {
//        retVal = IsInputFromMemoryMap (inputIdx) && retVal;
//    }
//
//    return retVal;
//}
//
////////////////////////////////////////////////////////////////////
/////               GetOutputValidity
////////////////////////////////////////////////////////////////////
bool CBlock::GetOutputValidityAndTime( unsigned long int *timeOfData, int outIndex )
{
    bool retVal = false;

    try
    {
        retVal = m_OutputVector.at(outIndex).isValid;

        if (timeOfData != 0x0)
        {
            *timeOfData = m_OutputVector.at(outIndex).timeOfData;
        }
    }
    catch (...)
    {
        //TODO da riempire
    }

    return retVal;
}


void CBlock::PrintBlockInfo(){

    CString infoString="";

    if (!m_DebugLevel){
        return;
    }

    infoString+=CString("Block:")+CString(Block_strings [m_BlockType])+CString(" ADDR:")+m_BlockAddress+CString(" ");
    int vectorSize = m_InputVector.size();
    for (int i = 0; i < vectorSize; i++){
        
        float data = -100.0;
        GetDataFromInput(i,&data);
        infoString+=CString("IN")+(i+1)+CString("=")+data+CString(" ");
    }

    vectorSize = m_OutputVector.size();
    for (int i = 0; i < vectorSize; i++) {
        float data = m_OutputVector.at(i).outValue;

        infoString+=CString("OUT")+(i+1)+CString("=")+data+CString(" ");
    }

    cout<<infoString<<endl;
}

void CBlock::SendBlockInfo(){
    COneWireEngine *eng = reinterpret_cast<COneWireEngine*>(m_EngPtr);

    CString infoString;

    if (!eng){
        return;
    }

    if (eng->CheckInterfacePortsForConnection())
    {
        Cmd com("BLOCK");
        com.putValue("TYPE",Block_strings [m_BlockType]);
        //com.putValue("SUBTYPE",Block_strings [m_BlockType]);
        com.putValue("ADDRESS",m_BlockAddress);

        int vectorSize = m_InputVector.size();
        for (int i = 0; i < vectorSize; i++){

            float data = -100.0;
            GetDataFromInput(i,&data);
            com.putValue(CString("IN")+(i+1),CString("")+data);
        }

        vectorSize = m_OutputVector.size();
        for (int i = 0; i < vectorSize; i++) {
            float data = m_OutputVector.at(i).outValue;

            com.putValue(CString("OUT")+(i+1),CString("")+data);
        }


        CString message = com.getXMLValue();
        eng->WriteOnInterfacePorts(message.c_str(), message.size());
    }
}

bool CBlock::IsCommandForMe(CXMLUtil* com){
    //Se non ha inidirizzo provo a parsarlo lo stesso
    if ((!com->ExistsParam("SUBADDR")) ||
        (com->GetIntParam("SUBADDR") == m_BlockAddress)
       )
    {
        return true;
    } else {
        return false;
    }
}

///////////////////////////////////////////////////
//             UpdateIniFile
///////////////////////////////////////////////////
bool CBlock::UpdateIniFile ( int subSystemIdx, int blockIndex, CString subKey, CString newVal )
{
    string configStr;
    CString key, section, deviceConfigurationString;
    char tempBuffer[32];
    CIniFileManager iniFileHndlr;

    if (!iniFileHndlr.Load(BLOCKS_FILE)){
        return false;
    }

    memset ( tempBuffer, 0x0, 32 );

    //Update the default value in the INI file
    section = "SUBSYSTEM";
    section +=subSystemIdx;

    memset ( tempBuffer, 0x0, 32 );
    sprintf ( tempBuffer, "Block%02d", blockIndex );
    key = tempBuffer;

    configStr = iniFileHndlr.GetString ( key, section );

    if (configStr.size() == 0){
        return false;
    }
    
    iniFileHndlr.SetConfigParamString ( &configStr, subKey.c_str(), newVal.c_str() );

    iniFileHndlr.SetValue ( key, configStr, "", section );

    iniFileHndlr.Save();

    return true;
}

bool CBlock::ParseCommandArgument(CXMLUtil* command, CString argument, float* destination, bool saveArgument)
{
    bool retVal = false;
    if (command->ExistsParam(argument.c_str())){

        *destination = command->GetFloatParam(argument.c_str());

        if (saveArgument)
        {
            CString valueStr = "";
            valueStr += command->GetFloatParam(argument.c_str());
            UpdateIniFile(m_ConfigSubSystemIndex, m_ConfigBlockIndex,argument,valueStr);
        }
        retVal = true;
    }

    return retVal;
}