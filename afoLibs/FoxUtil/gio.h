/***************************************************************************
 * gio.h  - ver 1.01 - 18 July 2005                                        *                                  
 *   Copyright (C) 2005 by CapeSoft Software (Pty) Ltd                     *
 *   1 July 2005 - Bruce Johnson                                           *
 *   This code can be used in any way without restriction.                 *
 *   www.capefox.com                                                       *
 *                                                                         *
 *   This code is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 *                                                                         *
 ***************************************************************************/
// designed to be used with the Acme Systems FOX computer
// running Linux 2.6 and SDK ver 2

#ifndef __GIO_H__
#define __GIO_H__
#include "stdio.h"      
#include "sys/ioctl.h"   
#include "etraxgpio.h"
#include <string.h>

#define LINE_0  (1<<0)
#define LINE_1  (1<<1)
#define LINE_2  (1<<2)
#define LINE_3  (1<<3)
#define LINE_4  (1<<4)
#define LINE_5  (1<<5)
#define LINE_6  (1<<6)
#define LINE_7  (1<<7)
#define LINE_8  (1<<8)
#define LINE_9  (1<<9)
#define LINE_10 (1<<10)
#define LINE_11 (1<<11)
#define LINE_12 (1<<12)
#define LINE_13 (1<<13)
#define LINE_14 (1<<14)
#define LINE_15 (1<<15)
#define LINE_16 (1<<16)
#define LINE_17 (1<<17)
#define LINE_18 (1<<18)
#define LINE_19 (1<<19)
#define LINE_20 (1<<20)
#define LINE_21 (1<<21)
#define LINE_22 (1<<22)
#define LINE_23 (1<<23)
#define LINE_24 (1<<24)
#define LINE_25 (1<<25)
#define LINE_26 (1<<26)
#define LINE_27 (1<<27)
#define LINE_28 (1<<28)
#define LINE_29 (1<<29)
#define LINE_30 (1<<30)
#define LINE_31 (1<<31)

//Defines to reprogram the radiocraft module
#define LED_1 1<<25

#ifndef IO_SETGET_INPUT
#define IO_SETGET_INPUT   0x12
#endif

#ifndef IO_SETGET_OUTPUT
#define IO_SETGET_OUTPUT  0x13
#endif

#ifndef IOMASK_A
#define IOMASK_A LINE_0
#endif

#define IOMASK_B LINE_4 | LINE_6 | LINE_7

#define IOMASK_G LINE_8 | LINE_9 | LINE_10 | LINE_11 | LINE_12 | LINE_13 | LINE_14 | LINE_15 | LINE_16 | LINE_17 | LINE_18 | LINE_19 | LINE_20 | LINE_21 | LINE_22 | LINE_23 | LINE_24

class EtraxGPIO
{
public:
	EtraxGPIO(){};
	~EtraxGPIO(){};

	// pins must be inited for either input or output. Note that some pins 
	// need to be inited on a "complete 8 bit block" basis. 
	int init_output(int pid,int mask);
	int init_input(int pid,int mask);
	
	// set output pins high or low.
	void set_output(int pid,int mask);
	void clear_output(int pid,int mask);
	
	// get input pins
	int get_input(int pid,int mask);
	
	// helpful for debugging - displays a 32 bit int as a string eg
	// 00110011 01010101 11110000 00001111
	void bitprintf(int i);
	
	// some pins on port G need to be set in blocks of 8. Calling fullmask will
	// flesh out the mask accordingly.
	int fullmask(int mask);
	
	// version history
	// 1.01 init_output, init_input returns state of bits that were set.
	//      fullmask function added.
};
#endif
