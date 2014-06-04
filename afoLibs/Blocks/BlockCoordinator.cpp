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
 
#include "BlockCoordinator.h"

#include "conewireengine.h"
#include "conewirenet.h"
#include "muxBlock.h"
#include "triggerBlock.h"
#include "binaryEncoder.h"
#include "ClockBlock.h"



CBlockCoordinator::CBlockCoordinator(const char *configString, CTimer *timer)
: CVCoordinator(configString)
{
    CIniFileManager iniMngr;
    if (configString != 0x0)
    {
        iniMngr.GetConfigParamString(configString,"INIFILE",&m_IniFileName,BLOCKS_FILE);
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
    m_ControllerType = DEV_BLOCKCOORD;

    //Variabile per fargli eseguire un comando
    m_CodeRevision = 1;
    
}


CBlockCoordinator::~CBlockCoordinator()
{
    CBlock *block;
  
    while (m_SubSystemList.size())
    {
        while (m_SubSystemList.at(m_SubSystemList.size()-1).blockList.size())
        {
            int subSystemIDX = m_SubSystemList.size()-1;
            int blockIDX = m_SubSystemList.at(subSystemIDX).blockList.size()-1;
            
            block = m_SubSystemList.at(subSystemIDX).blockList.at(blockIDX);
            m_SubSystemList.at(m_SubSystemList.size()-1).blockList.pop_back();
            delete block;
        }
        
        m_SubSystemList.pop_back();
    }

}


////////////////////////////////////////////////////////////////////
/////Run
///////////////////////////////////////////////////////////////////
void CBlockCoordinator::Run()
{
    if (m_DebugLevel)
    {
        cout << "Freely Programmable Coordinator Ready..."<<endl;
    }
    
    ControlSystem();
}

////////////////////////////////////////////////////////////////////
/////InitSystem
///////////////////////////////////////////////////////////////////
bool CBlockCoordinator::InitSystem()
{
    CString error;
    try
    {
        if (LoadSubSystemData() <= 0)
        {
            throw "Errore in caricamento Sistemi";
        }

        for (int i = 0; i < m_SubSystemList.size(); i++)
        {
            if (!ConnectBlocksInputs( i ))
            {
                error = "Errore nella connessione ingressi del sottosistema numero ";
                error += i+1;
                throw error.c_str();
            }
        }

        if (!SetupSubsystems())
        {
            throw "Errore nel setup dei blocchi";
        }
    }
    catch (exception ex)
    {
        cout << ex.what()<<endl;
        sleep(5);
        return false;
    }
    catch (const char* e)
    {
        cout << e<<endl;
        sleep(5);
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////
/////LoadSubSystemData
////////////////////////////////////////////////////////////////////
int CBlockCoordinator::LoadSubSystemData( )
{
    int retVal = -1;
    int nOfSubSystems, nOfBlocks, blockIndex, subSystemIndex;
    CString subSystemIndexString, blockIndexString, blockConfigString;
    CString error;
    struct timeval actTime;
    unsigned long int actTimeMs;
    CIniFileHandler iniFileReader;

    gettimeofday(&actTime, NULL);
    actTimeMs = actTime.tv_sec*1000 + actTime.tv_usec/1000;

    try
    {
        iniFileReader.Load(m_IniFileName);
        nOfSubSystems = iniFileReader.GetInt( Block_Config_Strings[BLOCK_NOF_SUBSYSTEMS], "COMMON", 0);

        if (!nOfSubSystems)
        {
            error = "Numero totale sottosistemi mancante!!";
            retVal = 0;
            throw error.c_str();
        }

        for (subSystemIndex = 0; subSystemIndex < nOfSubSystems; subSystemIndex++)
        {
            t_SubSystemData newSubSystemData;

            subSystemIndexString = "SUBSYSTEM";
            subSystemIndexString += subSystemIndex+1;

            nOfBlocks = iniFileReader.GetInt(Block_Config_Strings[BLOCK_NOF_BLOCKS], subSystemIndexString, -1);

            if (nOfBlocks<0)
            {
                error = "Numero totale blocchi assente nel sottosistema ";
                error += subSystemIndex+1;

                throw error.c_str();
            }
            else if (nOfBlocks == 0){
                cout << "WARNING: Subsystem blocks number is 0"<<endl;
                sleep(5);
                continue;
            }

            if (m_DebugLevel)
            {
                cout << "Creating Subsystem number: " << subSystemIndex +1<<endl;
                fflush(stdout);
            }

            error = "Subsystem number: ";
            error += subSystemIndex+1;

            for (blockIndex = 0; blockIndex < nOfBlocks; blockIndex++)
            {
                blockIndexString = "Block";

                if (blockIndex < 9)
                {
                    blockIndexString += '0';
                    blockIndexString += blockIndex+1;
                }
                else
                {
                    blockIndexString += blockIndex+1;
                }

                blockConfigString = iniFileReader.GetString( blockIndexString, subSystemIndexString, "");

                if (blockConfigString.size() == 0)
                {
                    error += " Missing block number: ";
                    error += blockIndex+1;
                    throw error.c_str();
                }

                //Create the new entry in the subsystem list
                newSubSystemData.updateTime = iniFileReader.GetInt(Block_Config_Strings[BLOCK_UPTIME], subSystemIndexString, 0);

                if (m_DebugLevel)
                {
                    cout << "Creating block number: " << blockIndex + 1 << endl;
                    fflush(stdout);
                }

                if (CreateBlock( blockConfigString.c_str(), &newSubSystemData))
                {
                    newSubSystemData.blockList.at(newSubSystemData.blockList.size()-1)->SetConfigIndexes(subSystemIndex+1, blockIndex+1);
                }
                else
                {
                    error += " There was an error while creating block number : ";
                    error += blockIndex+1;
                    throw error.c_str();
                }
            }//For block index

            newSubSystemData.lastUpdateTime = actTimeMs;
            newSubSystemData.hasHSInput = false;
            m_SubSystemList.push_back(newSubSystemData);
        }//For subsystemindex

        retVal = 1;
    }
    catch (const char *e)
    {
        cout << "There was an error while loading subsystem: "<<endl<<e<<endl;
        cout.flush();
        sleep(5);
    }

    return retVal;
}
////////////////////////////////////////////////////////////////////
/////Setup Subsystems
////////////////////////////////////////////////////////////////////
bool CBlockCoordinator::SetupSubsystems()
{
    bool retVal = true;
    vector<t_SubSystemData>::iterator subIt;

    for (subIt = m_SubSystemList.begin(); subIt < m_SubSystemList.end(); subIt++)
    {
        vector<CBlock*>::iterator blkIt;
        for (blkIt = subIt->blockList.begin(); blkIt < subIt->blockList.end(); blkIt++)
        {
            retVal = (*blkIt)->SetupBlock(m_NetPtr, m_EnginePtr) && retVal;
            (*blkIt)->SetDebugLevel(m_DebugLevel);
        }
    }

    return retVal;
}
////////////////////////////////////////////////////////////////////
/////CreateBlock
////////////////////////////////////////////////////////////////////
bool CBlockCoordinator::CreateBlock( const char * configString, t_SubSystemData *subSystemData )
{
//    bool retVal = false;
    CString blockName;
    CBlock *newBlock;
    CIniFileManager iniMngr;

    iniMngr.GetConfigParamString( configString, "NAME", &blockName, "");

    if (blockName == Block_strings[BLK_PID])
    {
       newBlock = new CPIDBlock(configString);
    }
    else if (blockName == Block_strings[BLK_IF])
    {
        newBlock = new CIFBlock (configString);
    }
    else if (blockName == Block_strings[BLK_COSTANT])
    {
        newBlock = new CCostantBlock (configString);
    }
    else if (blockName == Block_strings[BLK_ARITHMETIC])
    {
        newBlock = new COPBlock (configString);
    }
    else if (blockName == Block_strings[BLK_LOGIC])
    {
        newBlock = new CLOGBlock (configString);
    }
    else if (blockName == Block_strings[BLK_SATURATION])
    {
        newBlock = new CSATBlock (configString);
    }
    else if (blockName == Block_strings[BLK_HYSTERESIS])
    {
        newBlock = new CHYSTBlock (configString);
    }
    else if (blockName == Block_strings[BLK_COUNTER])
    {
        newBlock = new CCONTBlock (configString);
    }
    else if (blockName == Block_strings[BLK_GATE])
    {
        newBlock = new CGateBlock (configString);
    }
    else if (blockName == Block_strings[BLK_TIMER])
    {
        newBlock = new CTimerBlock (configString);
    }
    else if (blockName == Block_strings[BLK_MUX])
    {
        newBlock = new CMUXBlock (configString);
    }
    else if (blockName == Block_strings[BLK_TRIGGER])
    {
        newBlock = new CTriggerBlock (configString);
    }
    else if (blockName == Block_strings[BLK_DELAY])
    {
        newBlock = new CDelayBlock (configString);
    }
    else if (blockName == Block_strings[BLK_BINARYENCDEC])
    {
        newBlock = new CBinaryEncDec (configString);
    }
    else if (blockName == Block_strings[BLK_CLOCK])
    {
        newBlock = new ClockBlock (configString);
    }
    else if (blockName == Block_strings[BLK_C3POINT])
    {
        newBlock = new C3PointCtrlBlock (configString);
    }
    else if (blockName == Block_strings[BLK_CLIMATIC])
    {
        newBlock = new ClimaticCurveBlock (configString);
    }
    else
    {
        throw "Impossibile creare il blocco!!";
        return false;
    }

    subSystemData->blockList.push_back(newBlock);
    return true;
}

////////////////////////////////////////////////////////////////////
/////ControlSystem
////////////////////////////////////////////////////////////////////
bool CBlockCoordinator::ControlSystem( )
{
    unsigned long int actTimeMs;
    struct timeval actTime;
    vector<t_SubSystemData>::iterator subSystemIt;
    
    gettimeofday (&actTime,0x0);
    
    actTimeMs = actTime.tv_sec * 1000 + actTime.tv_usec/1000;
    
    for (subSystemIt = m_SubSystemList.begin(); subSystemIt < m_SubSystemList.end(); subSystemIt++)
    {
        if (actTimeMs > subSystemIt->updateTime + subSystemIt->lastUpdateTime)
        {
//            if (m_DebugLevel){
//                    cout << "-------- Subsystem"<<(subSystemIt - m_SubSystemList.begin())+1<<" --------"<<endl;
//            }
//
//            if (RunSubSystem(subSystemIt))
//            {
//                subSystemIt->lastUpdateTime = actTimeMs;
//            }

            if (m_DebugLevel){
                    cout << "-------- Subsystem"<<(subSystemIt - m_SubSystemList.begin())+1<<" --------"<<endl;
            }

            RunSubSystem(subSystemIt);
        }
    }
    
    return true;
}

////////////////////////////////////////////////////////////////////
/////ConnectBlocks
////////////////////////////////////////////////////////////////////
bool CBlockCoordinator::ConnectBlocksInputs( int subSystemIndex )
{
    bool retVal = false;
    vector<CBlock*>::iterator blockIt;
    int inputindex = 0;
    CString configString, blockString, subSystemString, inputNameString, inputString;
    int address, channel;
    //vettore usato per riordinare i blocchi man mano che li sistemo
    vector<CBlock*> swapVector;
    vector<CBlock*>::iterator swap1It, swap2It;
    //Flag che segnalal se c'e' stato almeno uno swap dei blocchi
    bool blocksSwapped = false;
    CIniFileManager iniMngr(m_IniFileName);
   
    cout << "Connecting subsystem " << subSystemIndex +1 << " blocks "<< endl;
    fflush(stdout);
    
    try
    {
        //C'e' il rischio di loop algebrici... provo a mettere un limite
        int iterationLimit = 10;
        do
        {
            subSystemString = "SubSystem";
            subSystemString += subSystemIndex + 1 ;
            
            //Copio il vettore di swap
            swapVector = m_SubSystemList.at(subSystemIndex).blockList;
            //Resetto la varibile di ciclo
            blocksSwapped = false;
            
            for (blockIt = m_SubSystemList.at(subSystemIndex).blockList.begin(); blockIt < m_SubSystemList.at(subSystemIndex).blockList.end(); blockIt++)
            {
                if ((*blockIt)->GetConfigBlockIdx() < 10)
                {
                    blockString = "Block0";
                    blockString += (*blockIt)->GetConfigBlockIdx();
                }
                else
                {
                    blockString = "Block";
                    blockString += (*blockIt)->GetConfigBlockIdx();
                }
                
                configString = iniMngr.GetString( blockString, subSystemString, "");
                
                if (configString.size() == 0)
                {
                    //TODO da migliorare
                    throw "Blocco non trovato";
                }

                //Resetto gli ingressi del blocco
                (*blockIt)->ResetInputChannels();
                for (inputindex = 0; inputindex < MAX_NUM_INPUT; inputindex++)
                {
                    inputNameString = "INPUT";
                    inputNameString += inputindex +1;
                    
                    iniMngr.GetConfigParamString( configString.c_str(), inputNameString.c_str(), &inputString, "");
                    
                    if (inputString == "")
                    {
                        //No more inputs, break;
                        break;
                    }
                    
                    //Check for type of input:
                    if (inputString.at(0) == 'N')
                    {
                        //Ingresso "finto"
                        (*blockIt)->SetInputChannel (0x0, 0, 0x0);
                    }
                    else if (inputString.at(0) == 'A')
                    {
                        //01/03/2010 -- Aggiunti driver con piu' di una uscita quindi cerco se c'e' il canale
                        string::size_type channelSepIdx;
                        int inputBlockChannel;
                        CString channelStr;

                        channelSepIdx = inputString.find(CHANNEL_SEP);
                        if (channelSepIdx == string::npos)
                        {
                            //Channel index not found, assume 0
                            inputBlockChannel = 0;
                        }
                        else
                        {
                            //Decode the channel
                            channelStr = inputString.substr(channelSepIdx);
                            //Internally the blocks are numbered from 0
                            inputBlockChannel = atoi(channelStr.substr(1).c_str()) - 1;

                            if (inputBlockChannel < 0)
                            {
                                throw "Input Channel NOT valid";
                            }

                            //Substitute the CHANNEL_SEP with end of string
                            inputString.at(channelSepIdx) = '\0';
                        }

                        //It is an address in the memory map
                        address = atoi(inputString.substr(1).c_str());
                        COneWireNet *netPtr = reinterpret_cast<COneWireNet*>(m_NetPtr);
                        CVController* ctrl = netPtr->GetControllerHndlrByMemoryAddress(address);
                        if (ctrl != 0x0)
                        {
                            (*blockIt)->SetInputChannel (0x0, inputBlockChannel, ctrl);
                        }
                    }
                    else if (inputString.at(0) == 'B')
                    {
                        //it is an input from another block, search and validate
                        int inputBlockIndex, inputBlockChannel;
                        vector<CBlock*>::iterator subSystBlkIt;
                        string::size_type channelSepIdx;
                        int blockIndex, subSystBlkIdx;
                        CString channelStr;
                        
                        channelSepIdx = inputString.find(CHANNEL_SEP);
                        
                        if (channelSepIdx == string::npos)
                        {
                            //Channel index not found, assume 0
                            inputBlockChannel = 0;
                        }
                        else
                        {
                            //Decode the channel
                            channelStr = inputString.substr(channelSepIdx);
                            //Internally the blocks are numbered from 0
                            inputBlockChannel = atoi(channelStr.substr(1).c_str()) - 1;
                            
                            if (inputBlockChannel < 0)
                            {
                                throw "Input Channel NOT valid";
                            }
                            
                            //Substitute the CHANNEL_SEP with end of string
                            inputString.at(channelSepIdx) = '\0';
                        }
                        
                        inputBlockIndex = atoi(inputString.substr(1).c_str());
                        
                        for (subSystBlkIt = m_SubSystemList.at(subSystemIndex).blockList.begin(); subSystBlkIt < m_SubSystemList.at(subSystemIndex).blockList.end(); subSystBlkIt++)
                        {
                            if ((*subSystBlkIt)->GetConfigBlockIdx() == inputBlockIndex)
                            {
                                break;
                            }
                        }
                        
                        if (subSystBlkIt == m_SubSystemList.at(subSystemIndex).blockList.end())
                        {
                            //TODO da gestire meglio
                            throw "Input Not Found";
                        }
                        
                        //TODO attualmente (26/7/2006) i blocchi hanno una sola uscita, va messa a posto
                        (*blockIt)->SetInputChannel (*subSystBlkIt, inputBlockChannel, 0x0);
                        
                        //Cerco il blocco che sto esaminando nel vettore di swap
                        for (swap1It = swapVector.begin(); swap1It < swapVector.end(); swap1It++)
                        {
                            if ( (*swap1It)->GetConfigBlockIdx() == (*blockIt)->GetConfigBlockIdx())
                            {
                                break;
                            }
                        }
                        
                        //cerco il blocco a cui e' collegato nel vettore di swap
                        for (swap2It = swapVector.begin(); swap2It < swapVector.end(); swap2It++)
                        {
                            if ( (*swap2It)->GetConfigBlockIdx() == (*subSystBlkIt)->GetConfigBlockIdx())
                            {
                                break;
                            }
                        }
                        
                        //TODO forse da controllare di averli trovato... ma dovrebbe essere inutile
                        
                        //Verifico qui che il blocco a cui e' collegato viene dopo questo, se non e' cosi' => li swappo
                        blockIndex = swap1It - swapVector.begin();
                        subSystBlkIdx = swap2It - swapVector.begin();
                                
                        if (subSystBlkIdx > blockIndex)
                        {
                            CBlock* tempBlkPointer;
                    
                            tempBlkPointer = *swap1It;
                            
                            *swap1It = *swap2It;
                            *swap2It = tempBlkPointer;
                            blocksSwapped = true;
                        }
                        
                    }
                } //For inputIndex

            } //For block
            
            //Ricopio il vettore ordinato nel vettore del sottosistema
            m_SubSystemList.at(subSystemIndex).blockList = swapVector;
            
            retVal = true;
            iterationLimit--;
        } while ((blocksSwapped) && (iterationLimit > 0));


        if (iterationLimit == 0){
            cout << "*******************************************************"<<endl;
            cout << "*******************************************************"<<endl;
            cout << "************ ALGEBRIC LOOP  ***************************"<<endl;
            cout << "*******************************************************"<<endl;
            cout << "*******************************************************"<<endl;
        }
    }
    catch (...)
    {
        //TODO da gestire
    }
                    
    return retVal;
            
}



////////////////////////////////////////////////////////////////////
/////RunSubSystem
////////////////////////////////////////////////////////////////////
bool CBlockCoordinator::RunSubSystem(vector< t_SubSystemData >::iterator subSysIt)
{
    bool retVal = true;
    vector<CBlock*>::iterator blockIt;
    COneWireEngine *eng = reinterpret_cast<COneWireEngine*>(m_EnginePtr);

    int counter = 0;
    for (blockIt = subSysIt->blockList.begin();blockIt < subSysIt->blockList.end(); blockIt++)
    {      
        retVal = (*blockIt)->Update() && retVal;
        (*blockIt)->PrintBlockInfo();
        (*blockIt)->SendBlockInfo();
        if ((counter % 10) == 0){
            eng->CheckForCommands2();
        }
        counter++;

    }
    
    return retVal;
}

////////////////////////////////////////////////////////////////////
/////ExecCommand
////////////////////////////////////////////////////////////////////
bool CBlockCoordinator::ExecCommand(CXMLUtil* xmlUtil)
{
    bool retVal = false;

    if (xmlUtil->GetIntParam("ADDRESS") == m_Address)
    {
        //Controllo se e' un messaggio che devo passare ai blocchi o lo devo parsare io....
        CString comando = xmlUtil->GetStringParam("COMMAND");
        if ( comando == commandStrings[COMM_BLOCKCOMMAND]){
            //Controllo se c'e' l'indirizzo o meno per vedere se mi fermo al primo
            //blocco che ha interpretato il messaggio o meno
            bool stopOnFirst = xmlUtil->ExistsParam("SUBADDR");


            //Ciclo su tutti i sottosistemi e su tutti i blocchi
            vector< t_SubSystemData >::iterator subSysIt;
            vector<CBlock*>::iterator blockIt;

            for (subSysIt = m_SubSystemList.begin(); subSysIt < m_SubSystemList.end(); subSysIt++){
                bool stopExecution = false;
                for (blockIt = subSysIt->blockList.begin();blockIt < subSysIt->blockList.end(); blockIt++)
                {
                    //CommadnExecuted mi dice se il comando e' stato processato,
                    bool commandExecuted = (*blockIt)->ExecCommand(xmlUtil,&retVal);

                    if (commandExecuted && stopOnFirst){
                        stopExecution = true;
                        break;
                    }
                }
                
                if (stopExecution){
                    break;
                }
            }
        }
    }

    return retVal;
}