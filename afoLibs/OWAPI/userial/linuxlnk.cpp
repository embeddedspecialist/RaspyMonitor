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
//---------------------------------------------------------------------------
//
//  linuxlnk.C - COM functions required by MLANLL.C, MLANTRNU, MLANNETU.C and
//           MLanFile.C for MLANU to communicate with the DS2480 based
//           Universal Serial Adapter 'U'.  Fill in the platform specific code.
//
//  Version: 1.02
//
//  History: 1.00 -> 1.01  Added function msDelay.
//
//           1.01 -> 1.02  Changed to generic OpenCOM/CloseCOM for easier
//                         use with other platforms.
//

//--------------------------------------------------------------------------
// Copyright (C) 1998 Andrea Chambers and University of Newcastle upon Tyne,
// All Rights Reserved.
//--------------------------------------------------------------------------
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
// IN NO EVENT SHALL THE UNIVERSITY OF NEWCASTLE UPON TYNE OR ANDREA CHAMBERS
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH
// THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//---------------------------------------------------------------------------
//
//  LinuxLNK.C - COM functions required by MLANLLU.C, MLANTRNU.C, MLANNETU.C
//             and MLanFile.C for MLANU to communicate with the DS2480 based
//             Universal Serial Adapter 'U'.  Platform specific code.
//
//  Version: 2.01
//  History: 1.02 -> 1.03  modifications by David Smiczek
//                         Changed to use generic OpenCOM/CloseCOM
//                         Pass port name to OpenCOM instead of hard coded
//                         Changed msDelay to handle long delays
//                         Reformatted to look like 'TODO.C'
//                         Added #include "ds2480.h" to use constants.
//                         Added function SetBaudCOM()
//                         Added function msGettick()
//                         Removed delay from WriteCOM(), used tcdrain()
//                         Added wait for byte available with timeout using
//                          select() in ReadCOM()
//
//           1.03 -> 2.00  Support for multiple ports. Include "ownet.h". Use
//                         'uchar'.  Reorder functions. Provide correct
//                         return values to OpenCOM.  Replace 'makeraw' call.
//                         Should now be POSIX.
//           2.00 -> 2.01  Added support for owError library.
//

#include "cownet.h"
#include "ownet.h"
#include "errno.h"


//---------------------------------------------------------------------------
// Attempt to open a com port.  Keep the handle in ComID.
// Set the starting baud rate to 9600.
//
// 'portnum'   - number 0 to MAX_PORTNUM-1.  This number provided will
//               be used to indicate the port number desired when calling
//               all other functions in this library.
//
//
// Returns: the port number if it was succesful otherwise -1
//
int COWNET::OpenCOMEx(char *port_zstr)
{
   int i;
   int portnum;

   if(!fd_init)
   {
      for(i=0; i<MAX_PORTNUM; i++)
         fd[i] = 0;
      fd_init = 1;
   }

   // check to find first available handle slot
   for(portnum = 0; portnum<MAX_PORTNUM; portnum++)
   {
      if(!fd[portnum])
         break;
   }
   //OWASSERT( portnum<MAX_PORTNUM, OWERROR_PORTNUM_ERROR, -1 );

   if(!OpenCOM(portnum, port_zstr))
   {
      return -1;
   }

   m_PortNames[portnum] = port_zstr;

   return portnum;
}

//---------------------------------------------------------------------------
// Attempt to open a com port.
// Set the starting baud rate to 9600.
//
// 'portnum'   - number 0 to MAX_PORTNUM-1.  This number provided will
//               be used to indicate the port number desired when calling
//               all other functions in this library.
//
// 'port_zstr' - zero terminate port name.  For this platform
//               use format COMX where X is the port number.
//
//
// Returns: true  - success, COM port opened
//          false - failure, could not open specified port
//
int COWNET::OpenCOM(int portnum, char *port_zstr)
{
    struct termios t;               // see man termios - declared as above
    int rc;

    if(!fd_init)
    {
        int i;
        for(i=0; i<MAX_PORTNUM; i++)
            fd[i] = 0;
        fd_init = 1;
    }

    if (!( (portnum<MAX_PORTNUM) && (portnum>=0) && (!fd[portnum])))
    {
        PushError( OWERROR_PORTNUM_ERROR, m_NetNumber );
        return false;
    }

    fd[portnum] = open(port_zstr, O_RDWR|O_NONBLOCK);
    if (fd[portnum]<0)
    {
        PushError(OWERROR_GET_SYSTEM_RESOURCE_FAILED, m_NetNumber);
        //Riazzero lo slot per renderlo disponibile per altre aperture
        fd[portnum] = 0;
        return false;  // changed (2.00), used to return fd;
    }

// #define ARM_TARGET
// #ifdef ARM_TARGET
//     /*set serial interface: RS-232*/
//     int interface = 232;
//     if(ioctl(fd[portnum], 0xe002, &interface) != 0) {
//         printf("set UART type: %d...Failed, errno: %s\r\n", interface, strerror(errno));
//         close(fd[portnum]);
//         return 0;
//     }
// #endif

    rc = tcgetattr (fd[portnum], &t);
    if (rc < 0)
    {
        int tmp;
        tmp = errno;
        close(fd[portnum]);
        errno = tmp;
        PushError(OWERROR_SYSTEM_RESOURCE_INIT_FAILED, m_NetNumber);
        return false; // changed (2.00), used to return rc;
    }

    cfsetospeed(&t, B9600);
    cfsetispeed (&t, B9600);

   // Save original settings.
    origterm = t;

   // Set to non-canonical mode, and no RTS/CTS handshaking
    t.c_iflag &= ~(BRKINT|ICRNL|IGNCR|INLCR|INPCK|ISTRIP|IXON|IXOFF|PARMRK);
    t.c_iflag |= IGNBRK|IGNPAR;
    t.c_oflag &= ~(OPOST);
    t.c_cflag &= ~(CRTSCTS|CSIZE|HUPCL|PARENB);

    t.c_cflag |= (CLOCAL|CS8|CREAD);
    t.c_lflag &= ~(ECHO|ECHOE|ECHOK|ECHONL|ICANON|IEXTEN|ISIG);

    t.c_cc[VMIN] = 0;
    t.c_cc[VTIME] = 3;

    rc = tcsetattr(fd[portnum], TCSAFLUSH, &t);
    tcflush(fd[portnum],TCIOFLUSH);

    if (rc < 0)
    {
        int tmp;
        tmp = errno;
        close(fd[portnum]);
        errno = tmp;
        PushError(OWERROR_SYSTEM_RESOURCE_INIT_FAILED, m_NetNumber);
        return false; // changed (2.00), used to return rc;
    }

    //TBR
//     cout << "Porta aperta" << endl; cout.flush();

    return true; // changed (2.00), used to return fd;
}


//---------------------------------------------------------------------------
// Closes the connection to the port.
//
// 'portnum'  - number 0 to MAX_PORTNUM-1.  This number was provided to
//              OpenCOM to indicate the port number.
//
int COWNET::CloseCOM(int portnum)
{
    int retVal = 0;

   // restore tty settings
//    tcsetattr(fd[portnum], TCSAFLUSH, &origterm);

   FlushCOM(portnum);

   retVal = close(fd[portnum]);

   if (retVal<0)
   {
       cout << "Si è verificato un errore nella chiusura del descrittore numero "<<portnum<< " :"<<strerror(errno)<<endl;
       
   }

   fd[portnum] = 0;

   return retVal;
}


//--------------------------------------------------------------------------
// Write an array of bytes to the COM port, verify that it was
// sent out.  Assume that baud rate has been set.
//
// 'portnum'   - number 0 to MAX_PORTNUM-1.  This number provided will
//               be used to indicate the port number desired when calling
//               all other functions in this library.
// Returns 1 for success and 0 for failure
//
SMALLINT COWNET::WriteCOM(int portnum, int outlen, uchar *outbuf)
{
   long count = outlen;
   int i = 0;
   char endPck[2] = {0x0D, 0x0E};


   i = write(fd[portnum], outbuf, outlen);

   //If we are wireless add end of packet
   if (m_Wireless)
   {
     //Add end of packet
     write(fd[portnum], &endPck, 2);
   }
   else
   {
     tcdrain(fd[portnum]);
   }

      //Eventually wait a bit
      msDelay(m_NetDelay);

   return (i == count);
}


//--------------------------------------------------------------------------
// Read an array of bytes to the COM port, verify that it was
// sent out.  Assume that baud rate has been set.
//
// 'portnum'  - number 0 to MAX_PORTNUM-1.  This number was provided to
//              OpenCOM to indicate the port number.
// 'outlen'   - number of bytes to write to COM port
// 'outbuf'   - pointer ot an array of bytes to write
//
// Returns:  TRUE(1)  - success
//           FALSE(0) - failure
//
int COWNET::ReadCOM(int portnum, int inlen, uchar *inbuf)
{
   fd_set         filedescr;
   struct timeval tval;   

   int nOfCharRead = 0;
   int nOfCharsRemaining = inlen;
   int retries = 0;
   int maxRetries;

   //Eventually wait a bit
   msDelay( m_NetDelay );

   maxRetries = inlen + 10;

//    for (cnt = 0; cnt < inlen; cnt++)
//    {
//       // set a descriptor to wait for a character available
//        FD_ZERO(&filedescr);
//        FD_SET(fd[portnum],&filedescr);
//       // set timeout to 10ms
//        tval.tv_sec = 0;
//        tval.tv_usec = 10000;
// 
//       // if byte available read or return bytes read
//        if (select(fd[portnum]+1,&filedescr,NULL,NULL,&tval) != 0)
//        {
//            if (read(fd[portnum],&inbuf[cnt],1) != 1)
//                return cnt;
//        }
//        else
//            return cnt;
//    }

   while ((nOfCharsRemaining > 0) && (retries < maxRetries))
   {
        FD_ZERO(&filedescr);
        FD_SET(fd[portnum],&filedescr);

        nOfCharRead = 0;

        // set timeout to 10ms
        tval.tv_sec = 0;
        tval.tv_usec = 10000;

        if (select(fd[portnum]+1,&filedescr,NULL,NULL,&tval) > 0)
        {
            if (FD_ISSET(fd[portnum],&filedescr))
            {
                nOfCharRead = read(fd[portnum],&(inbuf[inlen-nOfCharsRemaining]),1);

                if (nOfCharRead > 0)
                {
                    nOfCharsRemaining -= nOfCharRead;
                }
                else if (nOfCharRead == -1)
                {
                    //there is an error on Read: exit
                    break;
                }   
                else
                {
                    //No characters available
                    msDelay(10);
                    retries++;
                }
            }
            else
            {
                msDelay(10);
                retries++;
            }
        }
        else
        {
            msDelay(10);
            retries++;
        }
   }

   // success, so return number of bytes read
   return (inlen - nOfCharsRemaining);
}


//---------------------------------------------------------------------------
//  Description:
//     flush the rx and tx buffers
//
// 'portnum'  - number 0 to MAX_PORTNUM-1.  This number was provided to
//              OpenCOM to indicate the port number.
//
void COWNET::FlushCOM(int portnum)
{
   tcflush(fd[portnum], TCIOFLUSH);
}


//--------------------------------------------------------------------------
//  Description:
//     Send a break on the com port for at least 2 ms
//
// 'portnum'  - number 0 to MAX_PORTNUM-1.  This number was provided to
//              OpenCOM to indicate the port number.
//
void COWNET::BreakCOM(int portnum)
{
    int duration = 0;     // see man termios break may be too long

  if (m_Wireless)
  {
    uchar endSeq[3] = {0x00, 0x0D, 0x0E};
    uchar resetSeq[4] = {0x0D, 0x00, 0x00, 0x00};

    //Send first the end chars, just in case...
    //write (fd[portnum], &endSeq, 3);


    //Write the reset sequence
    write(fd[portnum], &resetSeq, 4);

    //Added 15-05-2006, it improves the ability to detect the master
    //TBM -- forse bisogna "calibrarlo" meglio
    msDelay( m_NetDelay );

    //Wait for at least 20 ms because this is the NTH-NRF-S board sends a break of 10 ms
    msDelay(310);

//     //TBR
//     cout<<"Break mandato!!! esco!!!!"<<endl;
//     exit(1);
  }
  else
  {
      int pippo;
      pippo = tcsendbreak(fd[portnum], 10);
  }

}

//--------------------------------------------------------------------------
//  Description:
//     Send 0x0D and 0x0E as end of the current packet
//
// 'portnum'  - number 0 to MAX_PORTNUM-1.  This number was provided to
//              OpenCOM to indicate the port number.
//
void COWNET::SendEndPck(int portnum)
{
  uchar endPck[] = {0x0D, 0x0E};

  //send the ending chars
  WriteCOM(portnum, 2, endPck);
}

//--------------------------------------------------------------------------
// Set the baud rate on the com port.
//
// 'portnum'   - number 0 to MAX_PORTNUM-1.  This number was provided to
//               OpenCOM to indicate the port number.
// 'new_baud'  - new baud rate defined as
// PARMSET_9600     0x00
// PARMSET_19200    0x02
// PARMSET_57600    0x04
// PARMSET_115200   0x06
//
void COWNET::SetBaudCOM(int portnum, uchar new_baud)
{
   struct termios t;
   int rc;
   speed_t baud=B9600;

   // read the attribute structure
   rc = tcgetattr(fd[portnum], &t);
   if (rc < 0)
   {
      close(fd[portnum]);
      return;
   }

   // convert parameter to linux baud rate
   switch(new_baud)
   {
      case PARMSET_4800:
        baud = B4800;
        break;

      case PARMSET_9600:
         baud = B9600;
         break;
      case PARMSET_19200:
         baud = B19200;
         break;
      case PARMSET_57600:
         baud = B57600;
         break;
      case PARMSET_115200:
         baud = B115200;
         break;
   }

   // set baud in structure
   cfsetospeed(&t, baud);
   cfsetispeed(&t, baud);

   // change baud on port
   rc = tcsetattr(fd[portnum], TCSANOW, &t);
   if (rc < 0)
   {
       owRelease(portnum);
   }
}

SMALLINT COWNET::WriteDirect2Port( int portnum, int outlen, uchar * outbuf )
{
    long count = outlen;
    int i = 0;

    i = write(fd[portnum], outbuf, outlen);

    return (i == count);
}

int COWNET::ReadPortDirect( int portnum, int inlen, uchar * inbuf )
{
  //call directly the internal function
    return ReadCOM(portnum, inlen, inbuf);
}
