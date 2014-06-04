/***************************************************************************
*   Copyright (C) 2007 by Alessandro Mirri                                *
*   alessandro.mirri@newtohm.it                                           *
*                                                                         *
*   This program is NOT free software; you can NOT redistribute it and/or *
*   modify it in any way without the authorization of the author          *
*                                                                         *
*   This program is distributed WITHOUT ANY WARRANTY;                     *
*   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
*                                                                         *
***************************************************************************/
#include "di2ao.h"

CDI2AO::CDI2AO(const char* configString, CTimer *timer):CVController( configString)
{   
    if (configString != 0x0)
    {
        m_IniLib.GetConfigParamInt( configString, "TIMERID", &m_TimerID, -1);
        
        if ( (m_TimerID > 0) && (timer != 0x0) )
        {
            m_Timer = timer;
            m_UseTimer = true;
        }
        else
        {
            m_UseTimer = false;
        }
        
        //Parsing input string for functions
        m_ParseOK = ParseFunctions( configString );
        
        //Create Digital INs
        CreateDigital( configString );
        
        //Create Digital OUTs
        CreateAnalog( configString );
    }
    
    m_TypeOfTimerVal = TIMERVAL_DIGITAL;
    m_ControllerType = DEV_DI2AO;
}


CDI2AO::~CDI2AO()
{
    vector<CDigitalIO*>::iterator inIt;
    vector<CAnalogIO*>::iterator outIt;
    
    if (m_InVector.size() > 0)
    {
        for (inIt = m_InVector.begin(); inIt < m_InVector.end(); inIt++)
        {
            delete (*inIt);
            m_InVector.erase (inIt);
        }
    }
    
    if (m_OutVector.size() > 0)
    {
        for (outIt = m_OutVector.begin(); outIt < m_OutVector.end(); outIt++)
        {
            delete (*outIt);
            m_OutVector.erase (outIt);
        }
    }
}

bool CDI2AO::ParseFunctions( const char* configString )
{
    CString funString, funLabel, param;
    int index, totNFun, outputIDX;
    bool retVal = false;
    t_Function newFunction;
    string::size_type idx, idx2;
    
    m_IniLib.GetConfigParamInt( configString, "NFUN", &totNFun, 0);
    
    if (totNFun > 0)
    {
        for (index = 1; index < totNFun+1; index++)
        {
            funLabel = "FUN";
            funLabel += index;
            
            m_IniLib.GetConfigParamString( configString,  funLabel.c_str(), &funString, "");
            
            if (funString.size() == 0)
            {
                retVal = false;
                break;
            }
            
            //Extract Configuration
            idx = funString.find(CONF_SEP);
            
            if (idx == string::npos)
            {
                retVal = false;
                break;
            }
            
            param = funString.substr(0, idx);
            
            newFunction.configuration = atoi(param.c_str());
            
            idx2 = funString.find(OUT_SEP, idx);
            
            if (idx2 == string::npos)
            {
                //Separator not found, maybe we have only one output connected
                idx2 = funString.find (OUTVAL_SEP, idx);
                
                if (idx2 == string::npos)
                {
                    //Problem: we don't have the required informations
                    retVal = false;
                    break;
                }
                else
                {
                    //extract the output device
                    param = funString.substr(idx+1,idx2-idx-1);
                    
                    //Subtract one because in m_OutVector the elements are numbered from 0
                    outputIDX = atoi(param.c_str()) - 1;
                    
                    newFunction.outputIDXVector.push_back(outputIDX);
                    
                    //Extract the output value
                    param = funString.substr(idx2 + 1, funString.size() - idx2 - 1);

                    newFunction.output = atoi (param.c_str());
                }
            }
            else
            {
                while (idx2 != string::npos)
                {
                    //extract the output device
                    param = funString.substr(idx+1,idx2-idx-1);
                    
                    outputIDX = atoi(param.c_str()) - 1;
                    
                    newFunction.outputIDXVector.push_back(outputIDX);
                                        
                    idx = idx2;
                    
                    idx2 = funString.find(OUT_SEP, idx+1);
                }
                
                idx2 = funString.find (OUTVAL_SEP, idx);
                
                if  (idx2 != string::npos)
                {
                    //Extract the last output index
                    param = funString.substr(idx+1,idx2-idx-1);
                    
                    //Subtract one because in m_OutVector the elements are numbered from 0
                    outputIDX = atoi(param.c_str()) - 1;
                    
                    newFunction.outputIDXVector.push_back(outputIDX);
                    
                   //Extract the output value
                    param = funString.substr(idx2+1, funString.size() - idx2);
                    
                    newFunction.output = atoi (param.c_str());
                    
                    retVal = true;
                }
                else
                {
                    retVal = false;
                    break;
                } 
            }
            
            //Save the configuration
            m_FunctionVector.push_back(newFunction);
            
        }//FOR
    }//IF
                    
    
    return retVal;
}

bool CDI2AO::CreateDigital( const char* configString )
{
    int nOfDigitals = 0, digitalIndex, input, channel;
    CString idxString, chString;
    bool retVal = false;
    char configBuffer[255];
    int invertDevice, invertValue;
    
    m_IniLib.GetConfigParamInt( configString, "NIN", &nOfDigitals, 0);
    
    if (nOfDigitals == 0)
    {
        return retVal;
    }
    
    m_IniLib.GetConfigParamInt( configString, "INVERT", &invertValue, 0);
    
    for (digitalIndex = 1; digitalIndex < nOfDigitals+1; digitalIndex++)
    {
        idxString="IN";
        idxString+=digitalIndex;
        
        chString="IN";
        chString+=digitalIndex;
        chString+="CH";
        
        m_IniLib.GetConfigParamInt( configString, idxString.c_str(), &input, -1);
        m_IniLib.GetConfigParamInt( configString, chString.c_str(), &channel, -1);
        
        if ((input < 0) || (channel < 0))
        {
            retVal = false;
            break;
        }
        else
        {
            invertDevice = (invertValue & (0x01<<(digitalIndex)))>>(digitalIndex);
            memset (configBuffer, 0x0, 255*sizeof(char));
            sprintf (configBuffer,"INPUT:%d,CHANNEL:%d,INVERTOUT:%d,IO:1,TIMERID:-1",input,channel,invertDevice);
            m_InVector.push_back(new CDigitalIO(configBuffer, 0x0));
            retVal = true;
        }
    }
    
    return retVal;
}

bool CDI2AO::CreateAnalog( const char* configString )
{
    int nOfAnalogs = 0,analIDX;
    CString idxString;
    bool retVal = false;
    char configBuffer[255];
    
    m_IniLib.GetConfigParamInt( configString, "NOUT", &nOfAnalogs, 0);
    
    if (nOfAnalogs == 0)
    {
        return retVal;
    }
    
    //The AnalogIO does not have any particular parameter
    sprintf (configBuffer,"NAME:AnalogINOUT,STARTPOS:0");
    
    for (analIDX = 1; analIDX < nOfAnalogs+1; analIDX++)
    {
        m_OutVector.push_back(new CAnalogIO(configBuffer));
    }
    
    retVal = true;
    
    return retVal;
}

bool CDI2AO::Update( bool updateData )
{
    bool retVal = true;
    vector<t_Function>::iterator functionIt;
    vector<CDigitalIO*>::iterator digIt;
    vector<int>::iterator outputIt;
    int actualConfiguration = 0;
    int state, shift;
    
    try
    {
        if (updateData)
        {
            //TODO algoritmo NON ottimizzato: vado sempre a leggere tutti gli ingressi; bisognerebbe mettere su qualcosa tipo COneWireNet::UpdateDIDO(). Vedi anche engine::manage advanced controllers
            for (digIt = m_InVector.begin(); digIt < m_InVector.end(); digIt++)
            {
                state = (*digIt)->GetState();
                shift = (digIt - m_InVector.begin());
                
                actualConfiguration += (state<<shift);
            }
        }
        else
        {
            for (digIt = m_InVector.begin(); digIt < m_InVector.end(); digIt++)
            {
                actualConfiguration += ((*digIt)->GetLastState())<<(digIt - m_InVector.begin());
            }
        }
            
            
        for (functionIt = m_FunctionVector.begin(); functionIt < m_FunctionVector.end(); functionIt++)
        {
            if (((*functionIt).configuration) == actualConfiguration)
            {
                for (outputIt = (*functionIt).outputIDXVector.begin();outputIt < (*functionIt).outputIDXVector.end(); outputIt++)
                {
                    int pippo = *outputIt;
                    int pluto = (*functionIt).output;
                    
                    retVal = m_OutVector.at(pippo)->SetPosition(pluto);
                }
            }       
        }
    }
    catch (exception &e)
    {
        retVal = false;
    }
    
    
    if (retVal)
    {
        ClearError();
    }
    else
    {
        AddError();
    }
    
    return retVal;
}


