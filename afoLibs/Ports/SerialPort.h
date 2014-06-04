////////////////////////////////////////////////////////////////////////////////
//!\class  CSerialPort
//!                      SerialPort
//!The SerialPort is used to perform I/O operations using serial ports. It works
//!under  - QNX and under Windows. Since the two environments ,manages the serial
//!connection in a totally different way the code is completely divided and no
//!effort to integrate, as in CSocketPort class, has been made.<br>
//!The config string format is the following:<br>
//! - <B>PortType:Serial,</B>  -- Type of port
//! - <B>PortName=XXX,</B>     -- Name of port, i.e. COMXX under WIN32, /dev/serXX under QNX
//!
//!Following items are standard serial parameters:<br>
//!BaudRate=115200, Parity=0, ByteSize=8, StopBits=2, Xonoff_in=0, Xonoff_out=0,
//////////////////////////////////////////////////////////////////////////////////
#if !defined(AFX_SERIALPORT_H__CC29133A_304C_11D6_9796_00C0DF0976D6__INCLUDED_)
#define AFX_SERIALPORT_H__CC29133A_304C_11D6_9796_00C0DF0976D6__INCLUDED_
//////////////////////////////////////////////////////////////////////
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//////////////////////////////////////////////////////////////////////
#include "VPort.h"
#include <string.h>

//////////////////////////////////////////////////////////////////////

#ifdef _WIN32
typedef HANDLE SERIAL ;
#endif

#ifdef __QNX__
#include <sys/time.h>
typedef FILE* SERIAL ;
#endif // __QNX__

#ifdef __linux__
#include <sys/time.h>
typedef int SERIAL ; // file descriptor
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#endif // __linux__

//////////////////////////////////////////////////////////////////////

#define XON 0x11
#define XOFF 0x13
#define NO_PARITY 0
#define ODD_PARITY 1
#define EVEN_PARITY 2
#define MARK_PARITY 3
#define SPACE_PARITY 4
#define ONE_STOPBIT 1
#define ONEPOINTFIVE_STOPBIT 1.5
#define TWO_STOPBITS 2

class CSerialPort : public CVPort
{

    public:
///////////////////////////////////////////////////////////////////////////////
//
//! Constructor
//
//!\param void
//!
//!\return nothing
//!
//! <b>Actions :</b><br>
//!   Standard constructor. <br>
//! - WIN32 -- Calls the constructor of the base class<br>
//!
///////////////////////////////////////////////////////////////////////////////   
    CSerialPort();
    
///////////////////////////////////////////////////////////////////////////////
//
//! Destructor
//
//!\param void
//!
//!\return nothing
//!
//! <b>Actions :</b><br>
//!   Calls Disconnect() to close the port.
//!
///////////////////////////////////////////////////////////////////////////////
    ~CSerialPort();

///////////////////////////////////////////////////////////////////////////////
//
// Init
//!Initialize Port
//
//!\param char *    -   Pointer to the configuration string
//!
//!\return true if port succesfully initialized, false otherwise
//!
//! <b>Actions :</b><br>
//!   Reads from the parameter the actual configuration and sets up the
//!   port environment.
//!
///////////////////////////////////////////////////////////////////////////////
    bool Init( char* ) ;
    
///////////////////////////////////////////////////////////////////////////////
//
// Connect
//!Connect the port
//
//!\param void
//!
//!\return true if port succesfully connected, false otherwise
//!
//! <b>Actions :</b><br>
//!   Creates the actual data stream to the port and sets it up according to
//!   the configuration.
//!
///////////////////////////////////////////////////////////////////////////////
    bool Connect() ;

///////////////////////////////////////////////////////////////////////////////
//
// Disconnect
//!Disconnect the port
//
//!\param void
//!
//!\return true if port succesfully disconnected, false otherwise
//!
//! <b>Actions :</b><br>
//!   Disconnects the Port closing the I/O stream.<br>
//! - WIN32 -- Calls CloseHandle() on handle<br>
//! - QNX   -- Calls close() on handle<br>
//!
///////////////////////////////////////////////////////////////////////////////    
    bool Disconnect() ;
    
///////////////////////////////////////////////////////////////////////////////
//
// Read
//!Perform a Read operation
//
//!\param char*     -   Buffer where to store data
//!\param int       -   Maximum number of bytes to read
//!
//!\return The number of read bytes
//!
//! <b>Actions :</b><br>
//!   Reads from the port up to int bytes putting them in the provided buffer.
//! - WIN32 -- Calls ReadFile() on handle<br>
//! - QNX   -- Uses the FD structures and select to check if data is waiting in the buffer
//!         then performs the read.
///////////////////////////////////////////////////////////////////////////////
    unsigned long Read(char *,int ) ;
    
///////////////////////////////////////////////////////////////////////////////
//
// Write
//!Perform a Write operation
//
//!\param char*     -   Buffer containing data
//!\param int       -   Number of bytes to write
//!
//!\return The number of written bytes
//!
//! <b>Actions :</b><br>
//!   Writes to the port int bytes reading them from the provided buffer.
//!   If the second parameter is not provided it checks for first parameter 
//!   length.
//!
///////////////////////////////////////////////////////////////////////////////
    unsigned long Write(const char *,int ) ;
    
#ifdef __QNX__
//!Return the hardware handle to the port: QNX Version
    SERIAL GethDriver() {return m_Serial->_Handle ;} ;
#else // __linux__ & WI32
//!Return the hardware handle to the port: Linux & Win Version
    SERIAL GethDriver() {return m_Serial ;} ;
#endif // 
    
//!Return the name of the port, i.e. COM1, /dev/ser1....
    char* GetPort()     {return m_Port      ;} ;
    int GetBaudRate()   {return m_BaudRate  ;} ;
    int GetParity()     {return m_Parity    ;} ;
    int GetByteSize()   {return m_ByteSize  ;} ;
    int GetStopBits()   {return m_StopBits  ;} ;
    bool GetXonoff_in() {return m_Xonoff_in ;} ;
    bool GetXonoff_out(){return m_Xonoff_out;} ;

//!Set the name of the port, i.e. COM1, /dev/ser1....
    void SetPort(char* Port)         {strcpy(m_Port, Port)      ;} ;
    void SetParity(int Parity)       {m_Parity = Parity         ;} ;
    void SetStopBits(int StopBits)   {m_StopBits = StopBits     ;} ;
    void SetByteSize(int ByteSize)   {m_ByteSize = ByteSize     ;} ;
    void SetBaudRate(int BaudRate)   {m_BaudRate = BaudRate     ;} ;
    void SetXonoff_in(bool X_in)     {m_Xonoff_in = X_in        ;} ;
    void SetXonoff_out(bool X_out)   {m_Xonoff_out = X_out      ;} ;

    private:

    //!Port hardware handle
    SERIAL m_Serial;
    
    //!Name of the port, i.e. COM1, /dev/ser1....
    char m_Port[16] ;
    int m_BaudRate  ;
    int m_Parity  ;
    int m_ByteSize  ;
    int m_StopBits  ;
    bool m_Xonoff_in  ;
    bool m_Xonoff_out ;
    
#ifdef __linux__
    struct termios m_Options;
#endif
    
};

#endif // !defined(AFX_SERIALPORT_H__CC29133A_304C_11D6_9796_00C0DF0976D6__INCLUDED_)
