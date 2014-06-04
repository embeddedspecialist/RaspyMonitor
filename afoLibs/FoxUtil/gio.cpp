// gio.c  - ver 1.00 - 18 July 2005
//   Copyright (C) 2005 by CapeSoft Software (Pty) Ltd
// see gio.h for comments and license.
// functions for general io pins

#include "gio.h"

int EtraxGPIO::init_output(int pid,int mask)
{ 
  int m; 
  m = mask;
  #ifdef CRIS
  ioctl(pid, _IO(ETRAXGPIO_IOCTYPE, IO_SETGET_OUTPUT),&mask);
  #endif
  return(mask & m);
};

void EtraxGPIO::set_output(int pid,int mask)
{
  #ifdef CRIS
  ioctl(pid, _IO(ETRAXGPIO_IOCTYPE, IO_SETBITS),mask);
  #endif
};

void EtraxGPIO::clear_output(int pid,int mask)
{
  #ifdef CRIS
  ioctl(pid, _IO(ETRAXGPIO_IOCTYPE, IO_CLRBITS), mask);
  #endif
};

int EtraxGPIO::init_input(int pid,int mask)
{
  int m;
  m = mask;
  #ifdef CRIS
  ioctl(pid, _IO(ETRAXGPIO_IOCTYPE, IO_SETGET_INPUT),&mask);
  #endif
  return (mask & m);  
};

int EtraxGPIO::get_input(int pid,int mask)
{ 
  int result = 0;

  #ifdef CRIS
  result = ioctl(pid, _IO(ETRAXGPIO_IOCTYPE, IO_READBITS)) & mask;
  #endif
  
  return result;
  
};

int EtraxGPIO::fullmask(int mask)
{ 
  if (mask | 0xFF00) 
    mask = mask | 0xFF00;
  if (mask | 0xFF0000)
    mask = mask | 0xFF0000;    
  return(mask);    
};

void EtraxGPIO::bitprintf(int i)
{ 
  int b;
  
  for (b=0;b<32;b++) 
  {
    if (i&(0x80000000>>b)) printf("1");
    else printf ("0");
    if (((b+1)%8)==0) printf(" ");
  }
  printf("\n"); 
}
