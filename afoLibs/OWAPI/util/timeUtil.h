/***************************************************************************
 *   Copyright (C) 2005 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it                                           *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 ***************************************************************************/
//Generic time related utilities
#ifndef _TIMEUTIL_H
#define _TIMEUTIL_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <termios.h>
#include <errno.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EPOCH_0 1970
#define SECS_IN_YEAR 31536000
#define SECS_IN_DAY 86400
#define SECS_IN_HOUR 3600
   

unsigned long msGettick(void);
void msDelay(int len);

//Controlla se un dato anno è bisestile
char IsLeapYear (unsigned short year);
unsigned int ConvertDateTM2Secs(unsigned int*seconds, struct tm *time);
int ConvertSecs2DateTM(unsigned int seconds, struct tm *pDateTime);

//Dest string deve essere almeno di 13 byte
//Il formato della data è MMDDhhmmYYYY
int ConvertSecs2DateStr(unsigned int seconds, char *destString, int stringLen);

#ifdef __cplusplus
}
#endif

#endif
