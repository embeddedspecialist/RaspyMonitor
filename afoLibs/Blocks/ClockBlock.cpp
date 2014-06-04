/* 
 * File:   ClockBlock.cpp
 * Author: amirrix
 * 
 * Created on 26 gennaio 2010, 17.13
 */

#include "ClockBlock.h"
#include "block.h"
#include "ClockManager.h"

ClockBlock::ClockBlock( const char* configString)
    : CBlock( configString){

    m_ClockIdx = mainClockManager.GetClock();

}

ClockBlock::~ClockBlock() {



}
///////////////////////////////////////////////////////////////////////////////
bool ClockBlock::Update(){

    if (IsInputConnected(0) && IsInputValid(0)){
        float reset;

        GetDataFromInput(0, &reset);

        if (reset != 0.0){
            mainClockManager.ResetClock(m_ClockIdx);
        }
    }

    //Mi basta una risoluzione in secondi
    float timeElapsed = mainClockManager.GetTimeElapsed(m_ClockIdx,false);
    
    return SetOutputVal(0, timeElapsed, true);
}
///////////////////////////////////////////////////////////////////////////////
bool ClockBlock::ExecCommand(CXMLUtil *xmlUtil, bool *commandRetCode){

    if (!IsCommandForMe(xmlUtil)){
        return false;
    }

    //Ora interpreto il messaggio
    CString command = xmlUtil->GetStringParam("BLOCKEXEC");
    if (command.ToUpper() == "RESET"){
        mainClockManager.ResetClock(m_ClockIdx);
        *commandRetCode = true;
    }
    else {
        *commandRetCode = false;
    }

    return true;

}
