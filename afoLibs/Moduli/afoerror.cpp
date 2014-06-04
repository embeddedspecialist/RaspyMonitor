/***************************************************************************
 *   Copyright (C) 2006 by Alessandro Mirri   *
 *   alessandro.mirri@newtohm.it   *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 *************************************************************************/
#include "afoerror.h"

CAfoErrorHandler::CAfoErrorHandler()
{
    m_DoDebug = false;
    m_EnableLog = false;
    m_MaxErrorFileSize = 0;
    m_SuppressErrors = false;
    
}


CAfoErrorHandler::~CAfoErrorHandler()
{
}


///////////////////////////////////////////////////
//             PushError
///////////////////////////////////////////////////
bool CAfoErrorHandler::PushError( e_AFOErrors error, int netIndex, int devIndex )
{
    t_Error newError;
    vector<t_Error>::iterator it;
    CString errorFileName = "./Error.Log";
    CString errorMessage;

    //Se errori soppressi non faccio niente
    if (m_SuppressErrors) {
        return true;
    }
    
    newError.errorType = error;
    newError.netIndex = netIndex;
    newError.deviceIndex = devIndex;
    
    time(&(newError.errorTime));
    
    
    //Build the message relative to the error
    errorMessage = "<";
    errorMessage += ctime(&(newError.errorTime));
    errorMessage.erase(errorMessage.end()-1);
    errorMessage += " NET:";
    errorMessage += netIndex;
    errorMessage += " Device:";
    errorMessage += devIndex;
    
#ifndef SMALL_MEMORY_TARGET
    
    errorMessage += " :";
    errorMessage += afoErrorsStrings[error];

#else
    
    errorMessage += "Codice Errore: ";
    errorMessage += error;

#endif
    
    
    if (m_DoDebug)
    {
        cout << errorMessage << endl;
        cout.flush();
    }

    //Now check if we have already the same error, this is done to not fill the error stack with the same message
    if (IsErrorAlreadyPresent( newError ))
    {
        return true;
    }
    
    //Vector is too big, remove the first error in queue
    if (m_ErrVector.size() >= MAX_NOF_AFOERRORS)
    {
        it = m_ErrVector.begin();
        m_ErrVector.erase(it);
    }
        
    try
    {
        m_ErrVector.push_back(newError);
    }
    catch (...)
    {
        if (m_DoDebug)
        {
            cout << "ATTENZIONE!! Impossibile eseguire push su vettore errori, resetto vettore. "<< endl;
            m_ErrVector.clear();
        }
    }
    
    //If Log enabled, save the error
    if (m_EnableLog)
    {
        if (TrimFile( errorFileName, errorMessage.size(), m_MaxErrorFileSize))
        {
            FILE *fileHandler = 0x0;
            
            fileHandler = fopen(errorFileName.c_str(), "a+");

            //Get file position
            fseek(fileHandler,0, SEEK_END);
            
            fprintf(fileHandler, "%s\r\n", errorMessage.c_str());
    
            //Close the file
            fclose(fileHandler);
        }
    }
    
    return true;
}


///////////////////////////////////////////////////
//             HasErrors
///////////////////////////////////////////////////
bool CAfoErrorHandler::HasErrors( )
{
    if (m_ErrVector.size() > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

///////////////////////////////////////////////////
//             PopError
///////////////////////////////////////////////////
bool CAfoErrorHandler::PopError(t_Error *retError )
{
    bool retVal = false;
    vector<t_Error>::iterator it;
    unsigned int i;
    
//     cout << "Dimensione vettore errori: " << m_ErrVector.size() << endl; cout.flush();
    if (m_ErrVector.size() != 0)
    {
        it = m_ErrVector.begin();
        for (i = 0; i < m_ErrVector.size() - 1; i++)
        {
            it++;
        }
        
        *retError = *it;
        
        m_ErrVector.erase(it);
        
        retVal = true;
    }
    else
    {
        retVal = false;
    }
        
    
    return retVal;
}

///////////////////////////////////////////////////
//             GetNofErrors
///////////////////////////////////////////////////
int CAfoErrorHandler::GetNofErrors( )
{
    return m_ErrVector.size();
}

///////////////////////////////////////////////////
//             GetError
///////////////////////////////////////////////////
t_Error CAfoErrorHandler::GetError( int errIndex )
{
    t_Error newError;
    
    if (errIndex < (int)m_ErrVector.size())
    {
        newError = m_ErrVector[errIndex];
    }
    else
    {
        newError.errorType = OWERROR_NO_ERROR_SET;
        newError.netIndex = -1;
        newError.deviceIndex = -1;
    }
    
    return newError;
}

///////////////////////////////////////////////////
//             GetLastError
///////////////////////////////////////////////////
t_Error CAfoErrorHandler::GetLastError( )
{
    return m_ErrVector[m_ErrVector.size()-1];
}
///////////////////////////////////////////////////
//             GetLastErrorString
///////////////////////////////////////////////////
CString CAfoErrorHandler::GetLastErrorString(){
#ifdef SMALL_MEMORY_TARGET
    return "";
#else
    int errIndex = m_ErrVector[m_ErrVector.size()-1].errorType;
    CString errStr = afoErrorsStrings[errIndex];
    return errStr;
#endif
}

///////////////////////////////////////////////////
//             IsErrorAlreadyPresent
///////////////////////////////////////////////////
bool CAfoErrorHandler::IsErrorAlreadyPresent(t_Error error2Check)
{
    bool retVal = false;
    vector<t_Error>::iterator errIt;
    int errorTimeout = 60; //If the same error returns after a minute push it anyway
    
    try
    {
        if (m_ErrVector.size() == 0)
        {
            return false;
        }
        
//         for (errorIndex = 0; errorIndex < m_ErrVector.size(); errorIndex++)
//         {
//             if ( (m_ErrVector.at(errorIndex).errorType == error2Check.errorType) &&
//                   (m_ErrVector.at(errorIndex).netIndex == error2Check.netIndex) &&
//                   (m_ErrVector.at(errorIndex).deviceIndex == error2Check.deviceIndex)
//                )                        
//             {
        for (errIt = m_ErrVector.end()-1; errIt >= m_ErrVector.begin(); errIt--)
        {
            if ( ((*errIt).errorType == error2Check.errorType) &&
                    ((*errIt).netIndex == error2Check.netIndex) &&
                    ((*errIt).deviceIndex == error2Check.deviceIndex)
               )
            {
                if (error2Check.errorTime > (*errIt).errorTime + errorTimeout)
                {
                    //The error is present but it is an old one so push it again
                    retVal = false;
                    break;
                }
                else
                {
                    retVal = true;
                    break;
                }   
            }
        }
    }
    catch (exception &e)
    {
        cout << "Errore nella gestione della lista errori: " << e.what() << endl;
    }
    
    return retVal;
}

//////////////////////////////////////////////////////////
//                        TrimFile
//////////////////////////////////////////////////////////
bool CAfoErrorHandler::TrimFile(CString fileName, int bytesToTrim, int maxFileSize)
{
    bool retVal = false;
    char *fileBuffer, *pCutBegin, *pCutEnd;
    CString fileConfigString;
    FILE *fileHandler = 0x0;
    long curPos = 0;
    
    if (fileName.size() == 0)
    {
        return retVal;
    }
    else if (bytesToTrim > maxFileSize)
    {
        //Error the number of bytes to insert is greater than the file size... dunno wht to do
        return true;
    }
    
    //Controllo dimensione file
    //Open file
    fileHandler = fopen(fileName.c_str(), "a+");

    //Get file position
    fseek(fileHandler,0, SEEK_END);
    curPos = ftell(fileHandler);
    
    if ((curPos + bytesToTrim) > maxFileSize)
    {
        //Devo riscrivere il file: leggo tutto il contenutolo 
        fseek (fileHandler, 0, SEEK_SET);
        fileBuffer = (char*)(calloc(curPos+1,1));
        fread(fileBuffer, sizeof(char), curPos, fileHandler);
        
        //chiudo e riapro in scrittura
        fclose (fileHandler);
        fileHandler = fopen(fileName.c_str(), "w");
        
        //Mi posizone al primo log e al secondo
        pCutBegin = strchr(fileBuffer,'<');
        pCutEnd = strchr(pCutBegin+1,'<');
        
        if (pCutBegin == 0x0)
        {
            return false;
        }
        else if (pCutEnd != 0x00)
        {
            //Se c'e' almeno un altro log cerco di liberare lo spazio che mi serve, altrimenti (pCutEnd == 0x00) sovrascrivo e basta
            //Finche' la dimensione del blocco da togliere e' tale per cui togliendolo il messaggio non ci sta aumento il blocco
            while ( curPos + bytesToTrim - (pCutEnd - pCutBegin) > maxFileSize )
            {
                pCutEnd = strchr(pCutEnd +1, '<');
                if (pCutEnd == 0x00)
                {
                    //non ci sono altri messaggi, posiziono il cursore all'inizio e sovrascrivo tutto
                    pCutEnd = fileBuffer;
                    break;
                }
            }
                
            //sposto il blocco all'inizio
            memmove (fileBuffer, pCutEnd, curPos - (pCutEnd - fileBuffer));
            
            //Azzero il resto
            memset (fileBuffer + curPos - (pCutEnd - pCutBegin), 0x0, pCutEnd - pCutBegin);
            
            //Scrivo nel file
            fprintf (fileHandler, "%s", fileBuffer);
            
            free (fileBuffer);
            
            retVal = true;
        }
    }
    else
    {
        retVal = true;
    }

    //Close the file
    fclose(fileHandler);
    
    return retVal;
}
