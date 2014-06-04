/* 
 * File:   triggerBlock.cpp
 * Author: amirrix
 * 
 * Created on 18 gennaio 2010, 17.28
 */

#include "triggerBlock.h"
#include "commonDefinitions.h"

CTriggerBlock::CTriggerBlock(const char* configString) : CBlock(configString) {
    m_LastInput = -1.0;

    //Vedo che trigger sono:
    CString tempString;

    //Get OP type and parameters
    m_LibIniReader.GetConfigParamString( configString, "TYPE", &tempString, "RISE");

    for (int i = 0; i < TRIGGER_NUM_TOT; i++)
    {
        if (tempString == TRIGGER_Type_Strings[i])
        {
            m_Type = (e_TRIGGERType)i;
        }
    }

}


CTriggerBlock::~CTriggerBlock() {
}

///////////////////////////////////////////////////
//              Update
///////////////////////////////////////////////////
bool CTriggerBlock::Update( )
{
    float input1 = 0.0;
    bool retVal = false, isInputValid = false;

    //Check if inputs are valid
    if (!IsInputValid(0))
    {
        SetOutputVal(0,-100.0,false);
        return false;
    }
    GetDataFromInput( 0, &input1);


    switch (m_Type)
    {
        case    TRIGGER_FALL:
        {
            if ((m_LastInput == 1.0) && (input1 == 0.0)){
                retVal = SetOutputVal( 0, 1.0, true);
            }
            else {
                retVal = SetOutputVal( 0, 0.0, true);
            }

        };break;
        case    TRIGGER_RISE:
        {
            if ((m_LastInput == 0.0) && (input1 == 1.0)){
                retVal = SetOutputVal( 0, 1.0, true);
            }
            else {
                retVal = SetOutputVal( 0, 0.0, true);
            }
        };break;
        case    TRIGGER_LOW:
        {
            if ((m_LastInput == 0.0) && (input1 == 0.0)){
                retVal = SetOutputVal( 0, 1.0, true);
            }
            else {
                retVal = SetOutputVal( 0, 0.0, true);
            }
        };break;
        case    TRIGGER_HI:
        {
            if ((m_LastInput == 1.0) && (input1 == 1.0)){
                retVal = SetOutputVal( 0, 1.0, true);
            }
            else {
                retVal = SetOutputVal( 0, 0.0, true);
            }
        };break;

        default: retVal = false;
    }

    m_LastInput = input1;

    return retVal;

}