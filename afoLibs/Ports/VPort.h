////////////////////////////////////////////////////////////////////////////////
//!\class  CVPort
//!                      VPort
//!The CVPort class is the port's common interface class: all the other
//!ports derive from this.<br>
//!
//! GENERAL NOTES: 
//! -# Remember to insert base class constructor in init list of derived classes.
//! -# A Port object has to be initialized via an explicit call to the
//!    Init( configString ) method after the construction 
//! <br>
//!       ConfigString format examples: <br>
//!       PORT00=PortType:ServerSocket, ServerPort:10000, ServerIPAddr:137.204.98.224,<br>
//!       PORT00=PortType:ClientSocket, ServerPort:10000, ServerIPAddr:137.204.98.224,<br>
//!       PORT02=PortType:File, FileNamePath:ADSOutData.txt, Mode:w+,ReadMode:1,<br>
//!       PORT10=PortType:Serial, PortName:COM1, BaudRate:38400, Parity:0, ByteSize:8, StopBits:2, Xonoff_in:0, Xonoff_out:0,<br>
//! <br>
//! -# Ports are designed to implement only basic interface functionalities,
//!    things like: allocating buffers where to put read data and 
//!    data serialization/unserialization are to be implemented in user application
//! -# To handle status message coming from the Port: User application must 
//!    instantiate an object of a class inheriting from CMsgStream()
//!    and pass its pointer in an explicit call to SetMsgStream(CMsgStream*) method.
//!
//! NOTES for files: 
//! -# Write-read mode is configurated through Mode and ReadMode item in configString.
//!    For read files, if ReadMode = 0 the pointer is resetted to the starting point after 
//!    each read() operation. Otherwise each read operation reads only one line from file.
//!    For write files override mode is used.
//! 
//! NOTES for sockets: 
//! -# Windows applications must register to socket stuff dll 
//!    via WSAStartUp() call and must WSACleanUp() at exit.
//!    This is up to the user application.
//!
//! NOTES for server socket: 
//! -# It is initialized and setted in listening mode by the Connect() method,
//!    then it checks for incoming calls in read() method: hence user must 
//!    provide a cyclic call to read() to poll client connections
//! -# If the client disconnects, the server restores its listening mode.
//! -# To force the connection down (disconnect) from the server side
//!    restoring the listen mode, the user must call in sequence
//!    Disconnect() and Connect() methods WITHOUT any call to read() between.
////////////////////////////////////////////////////////////////////////////////
#if !defined(AFX_VPORT_H__CC291339_304C_11D6_9796_00C0DF0976D6__INCLUDED_)
#define AFX_VPORT_H__CC291339_304C_11D6_9796_00C0DF0976D6__INCLUDED_
//////////////////////////////////////////////////////////////////////
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//////////////////////////////////////////////////////////////////////
#include "MsgStream.h"
#include "LibIniFile.h"
//////////////////////////////////////////////////////////////////////
class CVPort  
{
public:
///////////////////////////////////////////////////////////////////////////////
//
//! Constructor
//
//!\param msg = NULL    -   The message stream to use for messages
//!
//!\return nothing
//!
//! <b>Actions :</b><br>
//!   Sets up the message stream of the port.
//!
///////////////////////////////////////////////////////////////////////////////   
   CVPort( CMsgStream* msg = 0x0 )
      : m_MsgStream( msg )
   {
       m_IsConnected = false ;
       m_MsgStream = msg ;
   }

///////////////////////////////////////////////////////////////////////////////
//
//! Destructor
//
//!\param void
//!
//!\return nothing
//!
//! <b>Actions :</b><br>
//!   See derived classes for details.
//!
///////////////////////////////////////////////////////////////////////////////
   virtual ~CVPort(){} ;  

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
//!   See derived classes for details.
//
///////////////////////////////////////////////////////////////////////////////
   virtual bool Init( const char * )   = 0;
   
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
//!   Connects the Port object and prepares for I/O operations.
//!   See derived classes for details.
//!
///////////////////////////////////////////////////////////////////////////////
   virtual bool Connect()        = 0;
   
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
//!   Disconnects the Port closing the I/O.
//!   See derived classes for details.
//!
///////////////////////////////////////////////////////////////////////////////
   virtual bool Disconnect()     = 0;
   
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
//!   See derived classes for details.
//!
///////////////////////////////////////////////////////////////////////////////
   virtual unsigned long Read(char*, int)  = 0;
   
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
//!   See derived classes for details.
//!
///////////////////////////////////////////////////////////////////////////////
   virtual unsigned long Write(const char*, int) = 0;

///////////////////////////////////////////////////////////////////////////////
//
// DisplayMsg
//!Common interface to MsgStream
//
//!\param buff  -   Buffer containing messages to output
//!
//!\return void
//!
//! <b>Actions :</b><br>
//!   It checks if the MsgStream is valid and outputs data either to it or to 
//!   the stdout.
//!
///////////////////////////////////////////////////////////////////////////////
   void DisplayMsg(const char* buff )
   {
       if (m_MsgStream != NULL)
           m_MsgStream->Write(buff);
       else{
           printf ("%s\r\n",buff);
           fflush(stdout);
       }
           
//       if(m_MsgStream == NULL)
//          printf( "%s\n",buff );
//       else 
//          m_MsgStream->Write(buff);
   }

///////////////////////////////////////////////////////////////////////////////
//
// IsConnected
//!Check if the port is connected
//
//!\param void
//!
//!\return true if port is connectde, false otherwise
//!
//! <b>Actions :</b><br>
//!   Returns the connection status of the port
//!
///////////////////////////////////////////////////////////////////////////////    
   bool IsConnected(){ return m_IsConnected; }

///////////////////////////////////////////////////////////////////////////////
//
// SetMsgStream
//!Assign a message stream to the port
//
//!\param msg       -   The new MessageStream to use
//!
//!\return void
//!
//! <b>Actions :</b><br>
//!   Connects the internal msgStream to the provided one
//!
/////////////////////////////////////////////////////////////////////////////// 
   void SetMsgStream(CMsgStream* msg) { m_MsgStream = msg; }

protected:
   //!Connection status
   bool m_IsConnected;
   CLibIniFile m_IniLib;

private:
   //!Class used to output messages
   CMsgStream* m_MsgStream;
   
};

#endif // !defined(AFX_VPORT_H__CC291339_304C_11D6_9796_00C0DF0976D6__INCLUDED_)
