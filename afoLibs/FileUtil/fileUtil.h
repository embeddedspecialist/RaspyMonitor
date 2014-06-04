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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYS_CONFIG_FILE 0
#define SYS_TIMERS_FILE 1
#define SYS_BLOCKS_FILE 2
    
int TrimFile(const char* fileName, int bytesToTrim, int maxFileSize);

//Cerca in un file di configurazione del sistema la stringa che ha lo stesso inizio
//della stringa passata e la sostituisce
int UpdateSystemFile(const char* configString, const char* filename);

int ReplaceAfoFile(const char* fileName, const char* newFile);


#ifdef __cplusplus
}
#endif
