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
 #include "digitalin2out.h"

CDigitalIN2OUT::CDigitalIN2OUT(const char* configString, CTimer* timer): CVMultiDIDO(configString, timer)
{
}


CDigitalIN2OUT::~CDigitalIN2OUT()
{
}


