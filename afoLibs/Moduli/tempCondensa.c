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

#include "tempCondensa.h"

#ifdef __cplusplus
extern "C" {
#endif

    static float psicroData[20][14] = {
        {10.5,12.9,14.9,16.8,18.4,20.0,21.4,22.7,23.9,25.1,26.2,27.2,28.2,29.1},
        {9.7,12.0,14.0,15.9,17.5,19.0,20.4,21.7,23.0,24.1,25.2,26.2,27.2,28.1},
        {8.8,11.1,13.1,15.0,16.6,18.1,19.5,20.8,22.0,23.2,24.2,25.2,26.2,27.1},
        {7.1,9.4,11.4,13.2,14.8,16.3,17.6,18.9,20.1,21.2,22.3,23.3,24.2,25.1},
        {6.2,8.5,10.5,12.2,13.9,15.3,16.7,18.0,19.1,20.3,21.3,22.3,23.2,24.1},
        {5.4,7.6,9.6,11.3,12.9,14.4,15.8,17.0,18.2,19.3,20.3,21.3,22.3,23.1},
        {4.5,6.7,8.7,10.4,12.0,13.5,14.8,16.1,17.2,18.3,19.4,20.3,21.3,22.2},
        {3.6,5.9,7.8,9.5,11.1,12.5,13.9,15.1,16.3,17.4,18.4,19.4,20.3,21.2},
        {2.8,5.0,6.9,8.6,10.2,11.6,12.9,14.2,15.3,16.4,17.4,18.4,19.3,20.2},
        {1.9,4.1,6.0,7.7,9.3,10.7,12.0,13.2,14.4,15.4,16.4,17.4,18.3,19.2},
        {1.0,3.2,5.1,6.8,8.3,9.8,11.1,12.3,13.4,14.5,15.5,16.4,17.3,18.2},
        {0.2,2.3,4.2,5.9,7.4,8.8,10.1,11.3,12.5,13.5,14.5,15.4,16.3,17.2},
        {-0.6,1.4,3.3,5.0,6.5,7.9,9.2,10.4,11.5,12.5,13.5,14.5,15.3,16.2},
        {-1.4,0.5,2.4,4.1,5.6,7.0,8.2,9.4,10.5,11.6,12.6,13.5,14.4,15.2},
        {-2.2,-0.3,1.5,3.2,4.7,6.1,7.3,8.5,9.6,10.6,11.6,12.5,13.4,14.2},
        {-2.9,-1.0,0.6,2.3,3.7,5.1,6.4,7.5,8.6,9.6,10.6,11.5,12.4,13.2},
        {-3.7,-1.9,-0.1,1.3,2.8,4.2,5.5,6.6,7.7,8.7,9.6,10.5,11.4,12.2},
        {-4.5,-2.6,-1.0,0.4,1.9,3.2,4.5,5.7,6.7,7.7,8.7,9.6,10.4,11.2},
        {-5.2,-3.4,-1.8,-0.4,1.0,2.3,3.5,4.7,5.8,6.7,7.7,8.6,9.4,10.2},
        {-6.0,-4.2,-2.6,-1.2,0.1,1.4,2.6,3.7,4.8,5.8,6.7,7.6,8.4,9.2}
    };

    static int minTemp = 11, maxTemp = 30;
    static int minHum = 30, maxHum = 95;
    
float CalcDewPoint(const float airTemp, const float airHum, float *dewPointTemp)
{
    int row = 0;
    int column = 0;
    if ((int)airTemp <= minTemp)
    {
        row = 0;
    }
    else if ((int)airTemp >= maxTemp)
    {
        row = 18;
    }
    else
    {
        //Trovo la riga
        for (row = 0; row < 19; row++)
        {
            if ( ((int)airTemp <= maxTemp-row) && ((int)airTemp > maxTemp-row-1) )
            {
                break;
            }
        }
    }

    if (airHum <= minHum)
    {
        column = 0;
    }
    else if (airHum >= maxHum)
    {
        column = 13;
    }
    else
    {
        for (column = 0; column < 14; column++)
        {
            //Il passo delle umidità è 5%
            if ( ((int)airHum >= minHum+column*5) && ((int)airHum < minHum+(column+1)*5) )
            {
                break;
            }
        }
    }


    if (dewPointTemp != 0x0)
    {
        *dewPointTemp = psicroData[row][column];
    }
    
    return psicroData[row][column];

}

#ifdef __cplusplus
}
#endif
