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
#include "timeUtil.h"

#ifdef __cplusplus
extern "C" {
#endif

//Durate dei mesi
static int RTC_MonthVal[]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
//--------------------------------------------------------------------------
// Get the current millisecond tick count.  Does not have to represent
// an actual time, it just needs to be an incrementing timer.
//
unsigned long msGettick(void)
{
  struct timezone tmzone;
  struct timeval  tmval;
  long ms;

  gettimeofday(&tmval,&tmzone);
  ms = (tmval.tv_sec & 0xFFFF) * 1000 + tmval.tv_usec / 1000;
  return ms;
}


//--------------------------------------------------------------------------
//  Description:
//     Delay for at least 'len' ms
//
void msDelay(int len)
{
  struct timespec s;              // Set aside memory space on the stack

  s.tv_sec = len / 1000;
  s.tv_nsec = (len - (s.tv_sec * 1000)) * 1000000;
  nanosleep(&s, NULL);
}

//--------------------------------------------------------------------------
//  Description:
//     Convert the given date in the struct tm to the number of seconds since epoch 0
//
unsigned int ConvertDateTM2Secs(unsigned int * seconds, struct tm * time)
{
    int i = 0;
    unsigned int nOfMinutes = 0;
    int yearsFromEpoch;

    //La struct TM memorizza gli anni a partire dal 1900 e NON dal 1970
    yearsFromEpoch = (time->tm_year+1900) - EPOCH_0;
    
    //Conto gli anni: nella struttura tm sono contati dal 1900
    for (i = 0; i < yearsFromEpoch; i++)
    {
        nOfMinutes += 365*24*60;
        if (IsLeapYear(i+EPOCH_0))
        {
      //Aggiungo un giorno
            nOfMinutes += 24*60;
        }
    }
  
    //Conto i mesi
    for (i = 0; i < time->tm_mon; i++)
    {
        nOfMinutes += RTC_MonthVal[i]*24*60;
    
        //Se c'e' il 29 febbraio in mezzo aggiungo un giorno
        if (IsLeapYear(yearsFromEpoch+EPOCH_0) && (i == 1))
        {
            nOfMinutes += 24*60;
        }
    }
  
    //Conto i giorni
    for (i = 1; i < time->tm_mday; i++)
    {
        nOfMinutes += 24*60;
    }
  
    //Conto le ore, parto da 0 
    for (i = 0; i < time->tm_hour; i++)
    {
        nOfMinutes += 60;
    }
  
    //Conto i minuti, parto da 0
    for (i = 0; i < time->tm_min; i++)
    {
        nOfMinutes += 1;
    }
  
    *seconds = nOfMinutes*60 + time->tm_sec;
  
    return *seconds;
}
////////////////////////////////////////////////////////////////////////////////
//Converte i secondi forniti in una data e ora (Dal 1/1/1970)
//ritorna 0 in caso di errore o 1 se va tutto bene
int ConvertSecs2DateTM(unsigned int seconds, struct tm *pDateTime)
{
  int retVal = 0;
  int i;
  unsigned int remainingSecs;
  int year = 0, month = 0, day = 0, hour = 0, minute = 0, sec = 0;
  
  remainingSecs = seconds;
  i = 0;
  
  //tolgo i secondi in un anno, se l'anno è bisesto tolgo un giorno in più
  while (remainingSecs >= SECS_IN_YEAR)
  {
    remainingSecs -= SECS_IN_YEAR;
    
    if (IsLeapYear(EPOCH_0 + i))
    {
      remainingSecs -= SECS_IN_DAY;
    }
    
    i++;
  }
  
  year += i;
  
  //Conto i mesi
  i = 0;
  while (remainingSecs >= RTC_MonthVal[i]*SECS_IN_DAY)
  {
    remainingSecs -= RTC_MonthVal[i]*86400;
    
    //Se è febbraio tolgo un altro giorno
    if (IsLeapYear(year+EPOCH_0) && (i == 1))
    {
        remainingSecs -= SECS_IN_DAY;
    }
    
    i++;
  }
  
  month = i;
  
  //Conto i giorni
  i = 1;
  while (remainingSecs >= SECS_IN_DAY)
  {
    remainingSecs -= SECS_IN_DAY;
    i++;
  }
  
  //I giorni sono contati a partire da "1"
  day = i;
  
  //Conto le ore
  i = 0;
  while (remainingSecs >= SECS_IN_HOUR)
  {
    remainingSecs -= SECS_IN_HOUR;
    i++;
  }
  
  hour = i;
  
  //Conto i minuti
  i = 0;
  while (remainingSecs >= 60)
  {
    remainingSecs -= 60;
    i++;
  }
  
  minute = i;
  //Conto i secondi... dovrebbe essere tutto quello che avanza
  sec = remainingSecs;
  
        
  if (pDateTime != NULL)
  {
    pDateTime->tm_year = year+EPOCH_0-1900;
    pDateTime->tm_mon = month;
    pDateTime->tm_mday = day;
    pDateTime->tm_hour = hour;
    pDateTime->tm_min = minute;
    pDateTime->tm_sec = sec;
    retVal = 1;
  }
  
  return retVal;
}
////////////////////////////////////////////////////////////////////////////////
int ConvertSecs2DateStr(unsigned int seconds, char *destString, int stringLen){
    struct tm tempTime;
    
    memset (destString, 0x0, stringLen);
    if (ConvertSecs2DateTM(seconds, &tempTime)){
        sprintf (destString+strlen(destString), "%02d", tempTime.tm_mon+1);
        sprintf (destString+strlen(destString), "%02d", tempTime.tm_mday);
        sprintf (destString+strlen(destString), "%02d", tempTime.tm_hour);
        sprintf (destString+strlen(destString), "%02d", tempTime.tm_min);
        sprintf (destString+strlen(destString), "%02d", tempTime.tm_year+1900);
        return 1;
    }
    
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
char IsLeapYear (unsigned short year)
{
    if ( (! ((year%4) && (year%100))) || (!(year%400)) )
        return 1;
    return 0;
}

#ifdef __cplusplus
}
#endif
