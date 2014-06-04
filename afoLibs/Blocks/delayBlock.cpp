/* 
 * File:   delayBlock.cpp
 * Author: amirrix
 * 
 * Created on 18 gennaio 2010, 20.57
 */

#include "delayBlock.h"
#include "conewireengine.h"

CDelayBlock::CDelayBlock(const char* configString) : CBlock(configString) {

    //Get OP type and parameters
    m_LibIniReader.GetConfigParamFloat( configString, "TIME", &m_Delay, 1.0);
    m_Status = DELAY_STOPPED;
    m_ClockIdx = mainClockManager.GetClock();
}

CDelayBlock::~CDelayBlock() {
}

bool CDelayBlock::Update(){
    float input;

    if (!(IsInputValid(DELAY_IN_START)&&
          IsInputValid(DELAY_IN_RESET)&&
          IsInputValid(DELAY_IN_LOAD)))
    {
        SetOutputVal(0,-100.0,false);
        return false;
    }

    if (m_Status == DELAY_STOPPED){
        GetDataFromInput(DELAY_IN_START,&input);
        //Qui controllo se arriva l'impulso di partenza:
        if (input){
            //Devo partire: controllo se devo caricare un valore dal canale 2
            if (IsInputConnected(2)){
                GetDataFromInput(2,&m_Delay);
            }

            SetOutputVal(0,0.0,true);
            //Resetto il clock
            mainClockManager.ResetClock(m_ClockIdx);

            m_Status = DELAY_RUNNNING;
        }
        else {
            //Controllo se e' arrivato il reset ?
            GetDataFromInput(DELAY_IN_RESET,&input);
            if (input){
                SetOutputVal(0,0.0,true);
            }
        }
    }
    else {
        //Controllo se arrivo il reset o se e' scaduto il tempo
        GetDataFromInput(DELAY_IN_RESET,&input);

        if (input){
            //Mi fermo e resetto
            SetOutputVal(0,0.0,true);
            m_Status = DELAY_STOPPED;
        }
        else {
            int timeElapsed = mainClockManager.GetTimeElapsed(m_ClockIdx, false);
            if (timeElapsed >= m_Delay){
                //Finito...
                SetOutputVal(0,1.0,true);
//                m_Status = DELAY_STOPPED;
            }
        }
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool CDelayBlock::ExecCommand(CXMLUtil *xmlUtil, bool *commandRetCode){
    COneWireEngine *eng = reinterpret_cast<COneWireEngine*>(m_EngPtr);

    if (!IsCommandForMe(xmlUtil)){
        return false;
    }

     *commandRetCode = false;
    //Ora interpreto il messaggio
     CString command = xmlUtil->GetStringParam("BLOCKEXEC");
    if (command.ToUpper() == "GETDELAY"){
        Cmd com("BLOCK");
        com.putValue("ADDRESS",m_BlockAddress);
        com.putValue("BLOCKEXEC","Delay");

        com.putValue("VAL",m_Delay);

        CString message = com.getXMLValue();
        eng->WriteOnInterfacePorts(message.c_str(), message.size());

        *commandRetCode = true;

    }
    else if (command.ToUpper() == "SETDELAY"){
        *commandRetCode = ParseCommandArgument(xmlUtil,"VAL",&m_Delay) || (*commandRetCode);

    }

    return true;

}