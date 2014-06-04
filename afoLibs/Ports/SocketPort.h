////////////////////////////////////////////////////////////////////////////////
//!\class  CSocketPort
//!                      SocketPort
//!The SocketPort is used to perform I/O operations using sockets. It works
//!under QNX and under Windows.<br>
//!The config string format is the following:<br>
//! - <B>PortType:XXXSocket,</B>      - Type of port, could be ClientSocket or ServerSocket
//! - <B>ServerPort:XXXX,</B>         - Port to which to connect, or listen if server
//! - <B>ServerIPAddr:xx.xx.xx.xx,</B>- Address to which to connect
//////////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOCKETPORT_H__D644EAC1_3086_11D6_9796_00C0DF0976D6__INCLUDED_)
#define AFX_SOCKETPORT_H__D644EAC1_3086_11D6_9796_00C0DF0976D6__INCLUDED_
//////////////////////////////////////////////////////////////////////
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////// QNX Environment /////////////////////////////
#ifdef __QNX__

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h>
#include <string.h>
#define INVALID_SOCKET -1
#define SD_SEND         1
#define SD_BOTH         2
#define SOCKET_ERROR   -1
typedef int SOCKET ;
typedef unsigned int usize_t ;
typedef unsigned int size_of_addr ;
#endif //__QNX__

#ifdef __linux__

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <string>
#include <sys/ioctl.h> 
#include <errno.h>
#include <iostream>
#include <netinet/tcp.h>                 
#define INVALID_SOCKET -1
#define SD_SEND         1
#define SD_BOTH         2
#define SOCKET_ERROR   -1
typedef int SOCKET ;
typedef unsigned int usize_t ;
typedef unsigned int size_of_addr ;

using namespace std;
#endif // _linux_

/////////////////////////////////////////////////////

/////// Windows environment /////////////////////////////
#ifdef _WIN32
#include <Winsock2.h>
typedef int size_of_addr ;
#endif //_WIN32
/////////////////////////////////////////////////////

//Common Includes
#include "VPort.h"
#include "LibIniFile.h"
//////////////////////////////////////////////////////////////////////
#define SOCK_BUFFERLENGTH 4096
#define DEFAULT_PORT 5001
#define CREATING_MODE   0 // NotifyError modes
#define LISTENING_MODE  1
#define SENDING_MODE    2
#define ACCEPTING_MODE  3
#define RECEIVING_MODE  4
#define CLIENT_SOCKET 1
#define SERVER_SOCKET 2


class CSocketPort : public CVPort  
{
public:
/////////////////////////////////////////////////////////////////////////////
//
//! Constructor
//
//!\param void
//! 
//!\return void
//!
//!  <b>Actions :</b><br>
//!    Instantiate a new CSocketPort object that is invalid and not connected.
//!   
/////////////////////////////////////////////////////////////////////////////
   CSocketPort(void);


/////////////////////////////////////////////////////////////////////////////
//
//! Destructor
//
//!\param void
//! 
//!\return void
//!
//!  <b>Actions :</b><br>
//!    Disconnect the current instance (see Disconnect() member function) and
//!    destruct the current instance of CSocketPort object.
//!   
/////////////////////////////////////////////////////////////////////////////
   ~CSocketPort(void);

/////////////////////////////////////////////////////////////////////////////
//
// Init
//!Initialize Port
//
//!\param char *    -   Configuration string
//! 
//!\return true if port succesfully initialized, false otherwise
//!
//!  <b>Actions :</b><br>
//!    Sets the current instance's port number, IP address and socket type
//!    using the provided Configuration String. The Configuration String
//!    must be of the following format:<br>
//! <br>
//!       PortType:Socket,ServerPort:PortNumber,ServerIPAddr:IPAddress,<br>
//! <br>
//!    Where Socket, PortNumber and IPAddress are the values from which
//!    configure the current instance. Socket must be "ClientSocket"
//!    or "ServerSocket". In case of absence, the following default values 
//!    are used:<br>
//!    -  PortType:     ClientSocket
//!    -  ServerPort:   8000
//!    -  ServerIPAddr: 127.0.0.1 (local address)
//!
//!    For now, this function always returns true.
//!   
/////////////////////////////////////////////////////////////////////////////
   bool Init(const char *);

/////////////////////////////////////////////////////////////////////////////
//
// Connect
//!Connect the port
//
//!\param void
//! 
//!\return true if port succesfully connected, false otherwise
//!
//!  <b>Actions :</b><br>
//!    Connects the current instance using the instance's configuration
//!    see Init() function. If the port type is client, the socket is 
//!    connected using socket() and connect() functions in sys/socket.h
//!    library. If the port type is server, the socket is created using
//!    socket(), a name is binded to it using bind() and the socket is
//!    then put in listenning mode using listen(). The socket is kept in
//!    listening mode until a call to socket() to connect to that server
//!    socket or a call to accept() occurs. The function returns true if 
//!    connection was performed fine, false otherwise.
//!
///////////////////////////////////////////////////////////////////////////////
   bool Connect(void);

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
//!    Disconnects the current instance if it is connected and invalidates
//!    m_hDriver member socket, puts the current instance 
//!    in disconnected status. No effects if the current instance is not 
//!    connected. The disconnect is done using shutdown() and close() 
//!    functions from sys/socket.h library.
//!
///////////////////////////////////////////////////////////////////////////////
   bool Disconnect(void);

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
//!    Reads on the current socket for pending data and puts it in the Output
//!    Buffer for a maximum of Output Buffer Size, returns the number of 
//!    chars read from the socket, 0 if error occured or no chars were read.
//!    If the first argument is NULL the read just checks for a connection
//!
///////////////////////////////////////////////////////////////////////////////
   unsigned long Read(char *, int);
    unsigned long Read(std::string&);

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
//!    Writes the provided Buffer on the current socket as long as the sent
//!    bytes number is less than Buffer Size. If no Buffer Size is provided,
//!    or if the Buffer Size is negative, the Buffer is written until its
//!    first null character. Returns the number of written characters that 
//!    were successfully written in the socket. 
//!
///////////////////////////////////////////////////////////////////////////////
   unsigned long Write(const char *, int);

    unsigned long WriteClient(const char *Buffer, int BlockSize);
    
    /**
     * Returns the type of the socket used : client or server
     * @return 1 if he socket is client, 2 if the socket is server
     */
    int GetSocketType() { return m_SockType;};
    
    string GetSocketAddress() { return m_SrvIPAddr;};
    
    ///////////////////////////////////////////////////////////////////////////////
    //
    // ShutdownServer
    //!Shutdown if server
    //
    //!\param void
    //!
    //!\return void
    //!
    //! <b>Actions :</b><br>
    //!    Used to shutdown the server socket by the destructor.
    //!
    ///////////////////////////////////////////////////////////////////////////////
   void ShutdownServer(void);
   
    ///////////////////////////////////////////////////////////////////////////////
    //
    // flushInFlow
    //!Empty input data flow
    //
    //!\param void
    //!
    //!\return true if input data flow has been flushed, false if an error occurred
    //!\return use GetLastError() to retrieve the erro number.
    //!
    //! <b>Actions :</b><br>
    //! It flushes all data pending in the input before closing the socket.
    //!
    ///////////////////////////////////////////////////////////////////////////////
   bool flushInFlow();

   ///////////////////////////////////////////////////////////////////////////////
//
// GetCharsToRead
//!Retrieve the number of bytes pending on the socket
//
//!\param int *charsToRead      -   Where to store the number of bytes
//!
//!\return -1 if an error occurred, use GetLastError() to know which.
//!
//! <b>Actions :</b><br>
//! Common interface under QNX and Windows to get how many bytes are pending on the
//! input by calling the ioctl functions.
//!
///////////////////////////////////////////////////////////////////////////////
   int GetCharsToRead (int *charsToRead);


private:

///////////////////////////////////////////////////////////////////////////////
//
// checkForRequest
//!Checks the socket for incoming connections
//
//!\param void
//!
//!\return 0 if the connection has been accepted
//!\return 1 if an error during a select() occurred
//!\return 3 if an error during an accept() occurred
//!
//! <b>Actions :</b><br>
//! Checks the socket for incoming connections requests and tries to connect
//! the clients.
//!
///////////////////////////////////////////////////////////////////////////////
  int CheckForRequest(void);

///////////////////////////////////////////////////////////////////////////////
//
// GetLastError
//!Retrieve the last error number
//
//!\param void
//!
//!\return The code of the last occurred error
//!
//! <b>Actions :</b><br>
//! Common interface used to retrieve errors number under QNX and Windows
//!
///////////////////////////////////////////////////////////////////////////////
   int GetLastError(void);



///////////////////////////////////////////////////////////////////////////////
//
// SetNonBlocking
//!Set the Socket to non blocking
//
//!\param void
//!
//!\return void
//!
//! <b>Actions :</b><br>
//!    Used under windows and qnx to set the server socket to non blocking
//!
///////////////////////////////////////////////////////////////////////////////
   void SetNonBlocking();

///////////////////////////////////////////////////////////////////////////////
//
// HasConnectionDropped
//!Checks if the connection is still alive
//
//!\param void
//!
//!\return void
//!
//! <b>Actions :</b><br>
//!    Under Linux is used to check if the connection is alive or the remote socket
//!    has been closed abruptly
//!
///////////////////////////////////////////////////////////////////////////////
   bool HasConnectionDropped();



#ifdef __QNX__
///////////////////////////////////////////////////////////////////////////////
//
// closesocket(
//!Close the socket -- REDEFINED FOR QNX
//
//!\param void
//!
//!\return close() call return value
//!
//! <b>Actions :</b><br>
//! It closes the socket using a "window" interface.
//!
///////////////////////////////////////////////////////////////////////////////
   int closesocket( SOCKET m_hDriver ) { return close(m_hDriver) ;}

#ifndef socklen_t                  //QNX 6.1

   typedef unsigned int sock_addr_len;

#else                              //QNX 6.2

   typedef socklen_t sock_addr_len;  

#endif // socklen_t
#endif //__QNX__


#ifdef __linux__
   int closesocket( SOCKET m_hDriver ) { return close(m_hDriver) ;}
   typedef socklen_t sock_addr_len;
#endif // _linux_


#ifdef _WIN32
   typedef int sock_addr_len;
#endif

   //!Socket handles
   SOCKET m_hDriver, m_hLocalSrvr;

   //!Type of socket: client or server
   int m_SockType;

   //!Local and remote socket adresses
   sockaddr_in m_SockAddr, m_RemoteAddr; 

   //!???
   unsigned int m_RemoteAddrLen;

   //!???
   sock_addr_len m_SockAddrLen;

   //!Port number of server
   int m_SrvPort; 

   //!Address of server
   char m_SrvIPAddr[20];
   
   //!???
   bool m_ShutDownFlag ;

};



#endif // !defined(AFX_SOCKETPORT_H__D644EAC1_3086_11D6_9796_00C0DF0976D6__INCLUDED_)



