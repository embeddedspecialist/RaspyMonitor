/***************************************************************************
 *   Copyright (C) 2006 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it                                           *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 *************************************************************************/
 #include "upid2.h"

#define N_OF_DIV 9
extern const float listaDivisori[] = {1.0, 2.0, 10.0, 100.0, 1000.0, 0.5, 0.1, 0.01, 0.001};

CUPID2::CUPID2(const char* configString, CTimer *timer):CVController(configString)
{
    m_Device = 0x0;
    m_InitOk = false;
    m_LastUpdateTime = 0;
    m_UpdateTime = 0;
    m_LastClockUpdate = 0;

    GenerateUpdateTime();

}


CUPID2::~CUPID2()
{
}

///////////////////////////////////////////////////
//              WriteToDevice
///////////////////////////////////////////////////
bool CUPID2::WriteToDevice(uchar *mapMem){

    uchar data = 0x0, commandCheck;
    bool retVal;
    int retries = 1;
    bool commOk = false, sameMessage = false, deviceNotPresent = false;
    //Metto 10 secondi di timeout
    unsigned int timeout = 2;
    unsigned int actTime, startTime;

    commandCheck = (~mapMem[0]) - 1;

    //Sopprimo gli errori perche' il modulo e' asincrono
    m_AfoErrorHandler->SetErrorSuppression(true);

    commOk = false;

    //TBR
//     cout << "Scrivo messaggio con intestazione: " << (int)mapMem[0]<<endl;

    while ((retries < 2) && (!commOk))
    {
        if (m_Device->WriteAllSRAM(mapMem))
        {
            if (m_Device->SetSpecialReg(0x80))
            {
                //Aspetto un po' per essere sicuro di beccare la risposta su messaggi "pesanti"
                //msDelay(1000);
                //Entro in un loop di 10 secondi, cerco di fare il prima possibile
                msDelay(100);

                actTime = time(0x0);
                startTime = actTime;

                while (actTime < startTime + timeout)
                {
                    actTime = time(0x0);
                    if (m_Device->GetSpecialReg(&data))
                    {
                        if (data == 0xFF)
                        {
                            //Rileggo tutta la memoria se è tutta 0xFF il dispositivo potrebbe essere ancora nelle mani della uPID2 o non funzionare
                            if (m_Device->GetSRAM(mapMem))
                            {
                                deviceNotPresent = true;
                                for (int i = 0; i < 16; i++)
                                {
                                    if (mapMem[i] != 0xFF)
                                    {
                                        deviceNotPresent = false;
                                        commOk = true;
                                        break;
                                    }
                                }

                                if ((commandCheck == mapMem[0]) && (!deviceNotPresent))
                                {
                                    commOk = true;
                                    break;
                                }
                                else
                                {
//                                     cout<<"Errore di CRC sul comando della uPID2 di indirizzo "<<m_Address<<endl;
                                    msDelay(100);
                                }
                            }
                            else
                            {
                                //TODO messaggio errore
                                //TBR
                                cout << "Impossibile leggere la RAM dalla uPID2"<<endl;
                                msDelay(100);
                            }
                        }
                        else
                        {
                            msDelay(100);
                        }
                    }
                    else
                    {
                        //TODO messaggio di errore
                        //TBR
                        cout << "Impossibile leggere lo stato del PIO dalla uPID2"<<endl;
                        msDelay(100);
                    }
                }//WHILE timeout
            }
            else
            {
                //TODO messaggio di errore
                cout << "Impossibile impostare lo stato del PIO della uPID2"<<endl;
                msDelay(100);
            }
        }
        else
        {
            //TODO messaggio di errore
            cout << "Impossibile scrivere sulla SRAM della uPID2"<<endl;
            msDelay(100);
        }

        retries++;
    }

        //Riabilito gli errori
    m_AfoErrorHandler->SetErrorSuppression(false);


    if (commandCheck == mapMem[0]){
        //Everything ok
        retVal = true;
    }
    else {
        if (sameMessage)
        {
            PushError( AFOERROR_CTRLUPID2_DEVICE_NOT_RESPONDING, m_NetNumber, m_DeviceNumber );
        }

        if (deviceNotPresent)
        {
            PushError( AFOERROR_CTRLUPID2_DEVICE_NOT_PRESENT, m_NetNumber, m_DeviceNumber );
        }
        
        retVal = false;
    }

    return retVal;
}

///////////////////////////////////////////////////
//              UpdateDateTime
///////////////////////////////////////////////////
bool CUPID2::UpdateDateTime(time_t newTime){

    uchar mapMem[16];

    memset (mapMem, 0xFF, 16*sizeof(uchar));
    mapMem[0] = 0x02;
    mapMem[1] = 0x01;
    memcpy(&(mapMem[2]), (unsigned int*)(&newTime), 4);

    mapMem[15] = CalcCRC(mapMem);
    if (!WriteToDevice(mapMem)){
        return false;
    }

    return true;
}

///////////////////////////////////////////////////
//              SetDevice
///////////////////////////////////////////////////
void CUPID2::SetInputDevice(CDS2751* theValue){
    m_Device = theValue;
}

///////////////////////////////////////////////////
//              GetDevice
///////////////////////////////////////////////////
CDS2751* CUPID2::GetDevice() const {
    return m_Device;
}

///////////////////////////////////////////////////
//              VerifyIOPresence
///////////////////////////////////////////////////
bool CUPID2::VerifyIOPresence( )
{
    return m_Device->VerifyPresence();
}

///////////////////////////////////////////////////
//              GenerateUpdateTime
///////////////////////////////////////////////////
void CUPID2::GenerateUpdateTime()
{
    long int randomNumber;
    double randomNumberDbl;
    time_t seed;

    //Genero il tempo di aggiornamento cone distribuzione uniforme tra 20 e 600 sec.
    time(&seed);
    srandom(seed);
    randomNumber = random();
    randomNumberDbl = ((double)randomNumber)/((double)RAND_MAX);
    m_UpdateTime = (unsigned int)(20.0 + randomNumberDbl*(600.0-20.0));
}

///////////////////////////////////////////////////
//              UpdateCommonData
///////////////////////////////////////////////////
bool CUPID2::UpdateCommonData()
{
    bool retVal = false;
    time_t actDateSec;
    unsigned int devTimeSec;
    uchar mapMem[16];

    memset (mapMem, 0xFF, 16*sizeof(uchar));
    time(&actDateSec);

    //Controllo l'ora nella schedina
    if (actDateSec > m_LastClockUpdate + CLOCK_UPDATE_INTERVAL)
    {
        mapMem[0] = 0x05;

        mapMem[15] = CalcCRC(mapMem);
        //Aggiorno il modulo
        if (WriteToDevice(mapMem)){
            devTimeSec = 0;

            memcpy (&devTimeSec, &(mapMem[1]), 4);

            if (labs(actDateSec - devTimeSec) > MAX_TIME_ERROR){
                if (UpdateDateTime(actDateSec))
                {
                    m_LastClockUpdate = actDateSec;
                    retVal = true;
                }
            }
            else
            {
                retVal = true;
            }
        }
    }
    else
    {
        retVal = true;
    }

    

    return retVal;
}

bool CUPID2::CheckCRC(uchar * mapmem)
{
    unsigned char crc = mapmem[15];
    unsigned char computedCrc = 0;

    for (int i =0; i < 15; i++)
    {
        computedCrc+=mapmem[i];
    }

    computedCrc+=1;

    if (computedCrc == crc)
    {
        return true;
    }
    else
    {
        if (m_DebugLevel){
            cout << "uPID2 address:"<<m_Address<<" CRC Errato!!"<<endl;
        }

        return false;
    }
}

unsigned char CUPID2::CalcCRC(uchar * mapmem)
{
    unsigned char computedCRC = 0;

    for (int i = 0; i < 15; i++)
    {
        computedCRC+=mapmem[i];
    }

    computedCRC+=1;

    return computedCRC;
}

