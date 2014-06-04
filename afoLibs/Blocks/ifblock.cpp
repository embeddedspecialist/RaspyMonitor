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
 
 #include "ifblock.h"


CIFBlock::CIFBlock(const char* configString)
    : CBlock(configString)
{
    CString tempString;
    
    //Get IF type and parameters
    m_LibIniReader.GetConfigParamString( configString, "TYPE", &tempString, "EQ");

    m_IFType = IF_TYPE_NUM_TOT;
    
    for (int i = 0; i < IF_TYPE_NUM_TOT; i++)
    {
        if (tempString == IF_Type_Strings[i])
        {
            m_IFType = (e_IFType)i;
        }
    }
}


CIFBlock::~CIFBlock()
{
}

///////////////////////////////////////////////////
//              Update
///////////////////////////////////////////////////
bool CIFBlock::Update( )
{
    float input1 = 0.0, input2 = 0.0;
    bool retVal = false, isInputValid = false;
    
    //Check if inputs are valid
    isInputValid = IsInputValid(0) && IsInputValid( 1 );
    if (!isInputValid)
    {
        return false;
    }
    
    GetDataFromInput( 0, &input1);
    GetDataFromInput( 1, &input2);
    
    switch (m_IFType)
    {
        case     IF_TYPE_EQUAL:
        {
            retVal = SetOutputVal( 0, input1 == input2, isInputValid);
        };break;
        case    IF_TYPE_GREATTHAN:
        {
            retVal = SetOutputVal( 0, input1 > input2 , isInputValid);
        };break;
        case    IF_TYPE_LESSTHAN:
        {
            retVal = SetOutputVal( 0, input1 < input2, isInputValid);
        };break;
        case    IF_TYPE_GREAT_EQ:
        {
            retVal = SetOutputVal( 0, input1 >= input2, isInputValid);
        };break;
        case    IF_TYPE_LESS_EQ:
        {
            retVal = SetOutputVal( 0, input1 <= input2, isInputValid);
        };break;
        case IF_TYPE_NOT_EQUAL:
        {
            retVal = SetOutputVal( 0, input1 != input2, isInputValid);
        };break;
        default: retVal = false;
    }
    
    
    return retVal;
    
}


