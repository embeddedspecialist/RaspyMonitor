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
#include "wattmeterdrvem3.h"


CWattMeterDrvEM3::CWattMeterDrvEM3(COWNET *master, char *configString): CVAFOObject( configString )
{
    m_Master = master;
    m_IDX = 0xFF;
}


CWattMeterDrvEM3::~CWattMeterDrvEM3()
{
}

//////////////////////////////////////////////////////////
//                ReadParameters
//////////////////////////////////////////////////////////
bool CWattMeterDrvEM3::ReadParameters( float * destination )
{
    bool retVal = false;
    unsigned char requestMessage[16];
    unsigned char dataMessage[512];
    int reqLen = 0;
    int dataLen = 0, dataRead = 0, dataParsed = 0;
    int i = 0, j = 0, destinationIndex = 0;
    int counter = 10;
    unsigned char valBuffer[3];
    //*Array che contiene le lunghezze dei singoli dati nel messaggio di ritorno
    int valTypeLen[] = {2, 2, 2, 2, 2, 2, 2, 2 ,2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
    int valTypeIndex = 0;
    int destIndex = 0;
    
    memset (requestMessage, 0x0, 16*sizeof(unsigned char));
    memset (dataMessage, 0x0, 512*sizeof(unsigned char));
    
    requestMessage[reqLen++] = STX;                //Transmission start char
    requestMessage[reqLen++] = m_IDX;              //Destination address
    requestMessage[reqLen++] = READ_PARAMS;        //Command, parameters
    requestMessage[reqLen++] = 0xFF;               //Byte G0 of the parameters mask
    requestMessage[reqLen++] = 0xFF;               //Byte G1 of the parameters mask
    
    reqLen = AddCheck( requestMessage, reqLen);
    
    //Add end of message
    requestMessage[reqLen++] = ETX;
    
    //Write out message
    m_Master->WriteDirect2Port( 0, reqLen, requestMessage );
    
    while (counter > 0)
    {
        dataRead = m_Master->ReadPortDirect(0, 511 - dataLen, dataMessage); 
        
        if (dataRead == 0)
        {
            //Wait a bit and retry
            msDelay (500);
            counter--;
        }
        else
        {
            dataLen += dataRead;
            if (dataMessage[dataLen - 1] == ETX)
            {
                //We got all the message
                break;
            }
        }
    }
    
    //Start Parsing the message
    if ((dataMessage[0] == STX) && (dataMessage[1] == m_IDX))
    {
        //Inizio ciclo grappa (tolgo testa e coda)
        i = 2;
        while (i<dataLen-3)
        {
            for (j = 0; j < valTypeLen[valTypeIndex]; j++)
            {
                if (dataMessage[i] == DLE)
                {
                    //Carattere di escape: prendo il successivo complementato
                    valBuffer[j] = ~dataMessage[i+1];
                    i+=2;
                }
                else
                {
                    valBuffer[j] = dataMessage[i];
                    i++;
                }
                
                //Estraggo il numero
                if (valTypeIndex < 9)
                {
                    destination[destIndex] = ExtractVal( valBuffer );
                }
                else if (valTypeIndex < 12)
                {
                    destination[destIndex] = ExtractValCos( valBuffer );
                }
                else if (valTypeIndex < 23)
                {
                    destination[destIndex] = ExtractVal( valBuffer );
                }
                else if (valTypeIndex < 25)
                {
                    destination[destIndex] = ExtractValLong( valBuffer );
                }
                else if (valTypeIndex < 37)
                {
                    destination[destIndex] = ExtractVal( valBuffer );
                }
                else
                {
                    destination[destIndex] = ExtractValCos( valBuffer );
                }
                
                valTypeIndex++;
                destIndex++;
            }
        }
    }
    
    return retVal;
    
}

//////////////////////////////////////////////////////////
//                ExtractVal
//////////////////////////////////////////////////////////
float CWattMeterDrvEM3::ExtractVal( unsigned char * inBuffer )
{
    float retVal = ANALOG_ERRVAL;
    unsigned char decPoint, scale;
    float multiplier = 1.0, scaleMul = 1.0;
    
    //Maschero i bit per estrarre il punto decimale
    decPoint = inBuffer[1] & 0xFC;
    scale = inBuffer[1] & 0x0C;
    
    switch (decPoint)
    {
        case 0: multiplier = 1.0;break;
        case 1: multiplier = 0.1;break;
        case 2: multiplier = 0.01;break;
        default: return retVal;
    }
    
    switch (scale)
    {
        case 0: scaleMul = 1.0;break;
        case 1: scaleMul = 1000.0;break;
        case 2: scaleMul = 1000000.0;break;
        default: return retVal;
    }
    
    //metto tutto nela scala giusta per ottenere il moltiplicatore delle cifre significative
    multiplier = multiplier*scaleMul;
    
    //Ottengo retVal sommando le cifre del buffer moltiplicate per il fattore di scala e per il coefficiente relativo alla posizione
    retVal = (float)((((inBuffer[0]&0xF0)>>4)*multiplier*100.0) + ((inBuffer[0]&0x0F)*multiplier*10.0) + (((inBuffer[1]&0xF0)>>4)*multiplier));
    
    return retVal;
}

//////////////////////////////////////////////////////////
//                ExtractValLong
//////////////////////////////////////////////////////////
float CWattMeterDrvEM3::ExtractValLong( unsigned char * inBuffer )
{
    float retVal = ANALOG_ERRVAL;

    
    //Ottengo retVal sommando le cifre del buffer moltiplicate per il coefficiente relativo alla posizione
    retVal = (float)((((inBuffer[0]&0xF0)>>4)*10000.0) + ((inBuffer[0]&0x0F)*1000.0) + (((inBuffer[1]&0xF0)>>4)*100.0) + (((inBuffer[1]&0x0F))*10.0) + ((inBuffer[2]&0xF0)>>4) + (((inBuffer[2]&0x0F))*0.1));
    
    return retVal;
}

//////////////////////////////////////////////////////////
//                ExtractValCos
//////////////////////////////////////////////////////////
float CWattMeterDrvEM3::ExtractValCos( unsigned char * inBuffer )
{
    float retVal = ANALOG_ERRVAL;
    unsigned char decPoint;
    float multiplier = 1.0, scale = 1.0;
    
    //Maschero i bit per estrarre il punto decimale
    decPoint = inBuffer[1] & 0xFC;
    
    switch (decPoint)
    {
        case 0: multiplier = 1.0;break;
        case 1: multiplier = 0.1;break;
        case 2: multiplier = 0.01;break;
        default: return retVal;
    }
    
    //Ottengo retVal sommando le cifre del buffer moltiplicate per il fattore di scala e per il coefficiente relativo alla posizione
    retVal = (float)((((inBuffer[0]&0xF0)>>4)*multiplier*100.0) + ((inBuffer[0]&0x0F)*multiplier*10.0) + (((inBuffer[1]&0xF0)>>4)*multiplier));
    
    return retVal;
}

//////////////////////////////////////////////////////////
//                SetIDX
//////////////////////////////////////////////////////////
bool CWattMeterDrvEM3::SetIDX( int newIDX )
{
    bool retVal = false;
    unsigned char message[16];
    int messageIndex;
    int dataRead = 0, counter = 10;
    int i = 0;
    bool responseOK = false;
    
    memset (message, 0x0, 16*sizeof(unsigned char));
    
    message[messageIndex++] = STX;
    
    message[messageIndex++] = m_IDX;
    
    message[messageIndex++] = SET_IDX;
    
    if ((newIDX == STX) || (newIDX == ETX) || (newIDX == DLE))
    {
        message[messageIndex++] = DLE;
        message[messageIndex++] = ~newIDX;
    }
    else
    {
        message[messageIndex++] = newIDX;
    }
    
    messageIndex = AddCheck( message, messageIndex);
    
    message[messageIndex++]=ETX;
    
    //Write out message
    m_Master->WriteDirect2Port( 0, messageIndex, message );
    
    memset (message, 0x0, 16*sizeof(unsigned char));
    messageIndex = 0;
    
    while (counter > 0)
    {
        dataRead = m_Master->ReadPortDirect(0, 15 - messageIndex, message); 
        
        if (dataRead == 0)
        {
            //Wait a bit and retry
            msDelay (500);
            counter--;
        }
        else
        {
            messageIndex += dataRead;
            if (message[messageIndex - 1] == ETX)
            {
                //We got all the message
                responseOK = true;
                retVal = true;
                break;
            }
        }
    }
    
    //TBR
    cout << "Il dispositivo ha risposto: ";
    for (int i = 0; i < 4; i++)
    {
        cout << " %x" << message[i];
    }
    
    m_IDX = newIDX;
    return true;
}

int CWattMeterDrvEM3::AddCheck( unsigned char * outMessage, int checkStartIndex )
{
    int i = 0;
    
    //Calculating checksum
    for (i = 0; i < checkStartIndex; i++)
    {
        outMessage[checkStartIndex] += outMessage[i];
    }
    
    if ((outMessage[checkStartIndex] == STX) || (outMessage[checkStartIndex] == ETX))
    {
        //The checksum is equal to either STX or ETX, add 80H to the message
        outMessage[checkStartIndex+1] = outMessage[checkStartIndex];
        outMessage[checkStartIndex] = 0x80;
        outMessage[checkStartIndex+1] += outMessage[checkStartIndex];
        checkStartIndex+=2;
    }
    else
    {
        checkStartIndex++;
    }
    
    return checkStartIndex;
}

