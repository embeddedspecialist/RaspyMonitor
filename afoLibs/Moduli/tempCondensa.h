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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

    //Calcola la temperatura del punto di rugiada data una temperatura ed una umidita'
    //ritorna la temp di rugiada e se dewPointTemp != 0x0 lo scrive anche in quella variabile
    float CalcDewPoint(const float airTemp, const float airHum, float *dewPointTemp);




#ifdef __cplusplus
}
#endif
