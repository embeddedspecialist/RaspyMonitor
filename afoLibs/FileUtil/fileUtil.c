/***************************************************************************
 *   Copyright (C) 2008 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it                                           *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 *************************************************************************/

#include <unistd.h>

#include "fileUtil.h"


#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////
//                        TrimFile
//////////////////////////////////////////////////////////
int TrimFile(const char* fileName, int bytesToTrim, int maxFileSize)
{
    int retVal = 0;
    char *fileBuffer = 0x0, *pCutBegin, *pCutEnd;
    FILE *fileHandler = 0x0;
    unsigned long curPos = 0;

    if (fileName == NULL)
    {
        return retVal;
    }
    else if (bytesToTrim > maxFileSize)
    {
        //Error the number of bytes to insert is greater than the file size... dunno wht to do
        return 1;
    }

    //     cout << "Trim: Apro il file:"<< fileName<<endl;

    //Controllo dimensione file
    //Open file
    fileHandler = fopen(fileName, "a+");

    //Get file position
    fseek(fileHandler,0, SEEK_END);
    curPos = ftell(fileHandler);

    //     cout<<"La posizione corrente è: "<<curPos<<" E i byte da aggiungere sono: "<<bytesToTrim<<endl;


    if ((curPos + bytesToTrim) > maxFileSize)
    {
        //Devo riscrivere il file: leggo tutto il contenutolo
        fseek (fileHandler, 0, SEEK_SET);
        fileBuffer = (char*)(calloc(curPos+1,1));

        if (fileBuffer == NULL)
        {
            fclose (fileHandler);
            return 0;
        }
        
        fread(fileBuffer, sizeof(char), curPos, fileHandler);

        //chiudo e riapro in scrittura
        fclose (fileHandler);
        fileHandler = fopen(fileName, "w");

        //Mi posizono al primo log e al secondo
        pCutBegin = strchr(fileBuffer,'<');
        pCutEnd = strchr(pCutBegin+1,'<');

        if (pCutBegin == 0x0)
        {
            if (fileBuffer != 0x0)
            {
                free (fileBuffer);
            }
            
            fclose(fileHandler);
            return 0;
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

            retVal = 1;
        }
    }
    else
    {
        retVal = 1;
    }

    //Close the file
    fclose(fileHandler);

    if (fileBuffer != 0x0)
    {
        free (fileBuffer);
    }

    return retVal;
}

int UpdateSystemFile(const char* configString, const char* filename)
{
    char *fileBuffer = 0x0, tokenToSearch[255], *lineBegin = 0x0, *lineEnd = 0x0;
    char *newFileBuffer = 0x0;
    FILE *fileHandler = 0x0;
    unsigned long curPos = 0;

    //Open file
    fileHandler = fopen(filename, "a+");

    //Get file position
    fseek(fileHandler,0, SEEK_END);
    curPos = ftell(fileHandler);

    //+5 per sicurezza
    fileBuffer = (char*)malloc (curPos+5);
    memset (fileBuffer,0x0,curPos+5);

    rewind(fileHandler);
    fread(fileBuffer, sizeof(char), curPos, fileHandler);
    fclose(fileHandler);

    //A questo punto cerco la stringa....
    //l'unico modo che ho è cercare se nel file c'è una riga che inizia uguale
    //a quella passata.

    //Estraggo il token
    sscanf(configString,"%s ",tokenToSearch);

    lineBegin = strstr(fileBuffer,tokenToSearch);

    if (lineBegin == 0x0)
    {
        //La stringa non c'è: la aggiungo ???
        fileHandler = fopen(filename, "a+");

        //Scrivo nel file
        fprintf (fileHandler, "%s", configString);
        fflush(fileHandler);
        fclose(fileHandler);
    }
    else
    {

        //Cerco la fine riga
        lineEnd = strchr(lineBegin,'\n');

        if (lineEnd == NULL)
        {
            //Aggiungo la stringa in fondo
            fileHandler = fopen(filename, "a+");

            //Scrivo nel file
            fprintf (fileHandler, "%s", configString);
            fflush(fileHandler);
            fclose(fileHandler);
        }
        else
        {
            //Creo spazio per il nuovo file:
            long newFileDimension = (lineBegin - fileBuffer)+strlen(configString)+strlen(lineEnd)+5;
            newFileBuffer = (char *)malloc(newFileDimension);
            memset (newFileBuffer,0x0, newFileDimension);

            memcpy(newFileBuffer,fileBuffer,lineBegin-fileBuffer);
            strcat(newFileBuffer,configString);
            if (lineEnd != 0x0)
            {
                strcat(newFileBuffer,lineEnd+1);
            }

            //La stringa non c'è: la aggiungo ???
            rewind (fileHandler);

            //Scrivo nel file
            fileHandler = fopen(filename, "w");
            fprintf (fileHandler, "%s", newFileBuffer);
            fflush(fileHandler);
            fclose(fileHandler);
        }
    }

    //Faccio un po' di inhouse cleaning
    if (fileBuffer != 0x0)
    {
        free(fileBuffer);
    }

    if (newFileBuffer != 0x0)
    {
        free(newFileBuffer);
    }
    
    return 1;
}

int ReplaceAfoFile(const char* filename,const char* newFile)
{
    char* pSubChar;
    FILE *fileHandler = 0x0;

    //nella CIC::ReadCommands gli a capo sono stati sotisutiti
    pSubChar = strchr(newFile,'$');
    while (pSubChar != NULL)
    {
        *pSubChar = '\n';
        pSubChar++;
        pSubChar = strchr(pSubChar,'$');
    }

    fileHandler = fopen(filename,"w");

    if (fileHandler == NULL){
        return 0;
    }

    //Scrivo nel file
    fprintf (fileHandler, "%s", newFile);
    fflush(fileHandler);
    fclose(fileHandler);

    return 1;
}

#ifdef __cplusplus
}
#endif
