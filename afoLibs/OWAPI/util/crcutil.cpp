

//---------------------------------------------------------------------------
// Copyright (C) 2000 Dallas Semiconductor Corporation, All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY,  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL DALLAS SEMICONDUCTOR BE LIABLE FOR ANY CLAIM, DAMAGES
// OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// Except as contained in this notice, the name of Dallas Semiconductor
// shall not be used except as stated in the Dallas Semiconductor
// Branding Policy.
//--------------------------------------------------------------------------
//
//  crcutil.c - Keeps track of the CRC for 16 and 8 bit operations
//  version 2.00

// Include files
#include "crcutil.h"


CRCUtil::CRCUtil( )
{
    memset (utilcrc16, 0, MAX_PORTNUM*sizeof(ushort));
    memset (utilcrc8, 0, MAX_PORTNUM*sizeof(uchar));
}
//--------------------------------------------------------------------------
// Reset crc16 to the value passed in
//
// 'reset' - data to set crc16 to.
//
void CRCUtil::setcrc16(int portnum, ushort reset)
{
   utilcrc16[portnum] = reset;
   return;
}

//--------------------------------------------------------------------------
// Reset crc8 to the value passed in
//
// 'portnum'  - number 0 to MAX_PORTNUM-1.  This number is provided to
//              indicate the symbolic port number.
// 'reset'    - data to set crc8 to
//
void CRCUtil::setcrc8(int portnum, uchar reset)
{
   utilcrc8[portnum] = reset;
   return;
}

//--------------------------------------------------------------------------
// Calculate a new CRC16 from the input data short.  Return the current
// CRC16 and also update the global variable CRC16.
//
// 'portnum'  - number 0 to MAX_PORTNUM-1.  This number is provided to
//              indicate the symbolic port number.
// 'data'     - data to perform a CRC16 on
//
// Returns: the current CRC16
//
ushort CRCUtil::docrc16(int portnum, ushort cdata)
{
   cdata = (cdata ^ (utilcrc16[portnum] & 0xff)) & 0xff;
   utilcrc16[portnum] >>= 8;

   if (oddparity[cdata & 0xf] ^ oddparity[cdata >> 4])
     utilcrc16[portnum] ^= 0xc001;

   cdata <<= 6;
   utilcrc16[portnum]   ^= cdata;
   cdata <<= 1;
   utilcrc16[portnum]   ^= cdata;

   return utilcrc16[portnum];
}

//--------------------------------------------------------------------------
// Update the Dallas Semiconductor One Wire CRC (utilcrc8) from the global
// variable utilcrc8 and the argument.
//
// 'portnum'  - number 0 to MAX_PORTNUM-1.  This number is provided to
//              indicate the symbolic port number.
// 'x'        - data byte to calculate the 8 bit crc from
//
// Returns: the updated utilcrc8.
//
uchar CRCUtil::docrc8(int portnum, uchar x)
{
   utilcrc8[portnum] = dscrc_table[utilcrc8[portnum] ^ x];
   return utilcrc8[portnum];
}
