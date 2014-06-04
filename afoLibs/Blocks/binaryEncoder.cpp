/* 
 * File:   binaryEncoder.cpp
 * Author: amirrix
 * 
 * Created on 18 gennaio 2010, 18.10
 */

#include "binaryEncoder.h"


CBinaryEncDec::CBinaryEncDec(const char* configString) : CBlock(configString) {
    CString tempString;

    //Get OP type and parameters
    m_LibIniReader.GetConfigParamString( configString, "TYPE", &tempString, "ENC");

    if (tempString == "ENC"){
        m_Type = BINARY_ENCODER;
    }
    else {
        m_Type = BINARY_DECODER;
    }
}


CBinaryEncDec::~CBinaryEncDec() {
}

bool CBinaryEncDec::Update(){

    float input1,input2,input3,input4;
    float output1,output2,output3,output4;
    int output=0;
    bool isInputValid = true;

    if (m_Type == BINARY_ENCODER){
        for (int i = 0; i < 4; i++){
            isInputValid = isInputValid && IsInputValid(i);
        }

        if (!isInputValid){
            SetOutputVal(0,-100.0,false);
            return false;
        }

        for (int i = 0; i < 4; i++){
            if (IsInputConnected(i))
            {
                GetDataFromInput(i,&input1);
                if (input1 != 0){
                    output |= (1<<i);
                }
            }
        }

        SetOutputVal(0,(float)output,true);
    }
    else {

        if (IsInputValid(0)){
            
            GetDataFromInput(0,&input1);

            for (int i = 0; i < 4; i++){
                if ( (((int)input1) & (1<<i)) ) {
                    SetOutputVal(i,1.0,true);
                }
                else {
                    SetOutputVal(i,0.0,true);
                }
            }
        }
        else {
            for (int i =0; i < 4; i++){
                SetOutputVal(i,-100,false);
                return false;
            }
        }

    }

    return true;
}