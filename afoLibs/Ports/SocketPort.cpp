//////////////////////////////////////////////////////////////////////
//                   SocketPort.cpp
//              See SocketPort.h for details
//////////////////////////////////////////////////////////////////////


#include "SocketPort.h"

#ifdef __QNX__

// Error management
#include <errno.h>

// Socket library
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#endif

#ifdef __linux__

#define strnicmp strncasecmp

#endif // __linux__

//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////

#define SELECT_TIMEOUT_MICROSECS 100
//////////////////////////////////////////////////////////////////////
// Constructor, see .h file for description
//////////////////////////////////////////////////////////////////////
CSocketPort::CSocketPort()
:CVPort( NULL ) // base class constructor
{
    m_hDriver = INVALID_SOCKET ;
    m_hLocalSrvr = INVALID_SOCKET ;
    m_IsConnected = false ;
}
//////////////////////////////////////////////////////////////////////
// Destructor, see .h file for description
//////////////////////////////////////////////////////////////////////
CSocketPort::~CSocketPort()
{
    if(this->IsConnected())
    {
        this->Disconnect();
    }
}

//////////////////////////////////////////////////////////////////////
// Init function, see .h file for description
//////////////////////////////////////////////////////////////////////
bool CSocketPort::Init(const char* ConfigString )
{
    char SType[32];
    CLibIniFile libIni;

    // Get socket configuration
    libIni.GetConfigParamInt(ConfigString, "ServerPort", &m_SrvPort , 8000);
    libIni.GetConfigParamString(ConfigString, "ServerIPAddr", m_SrvIPAddr, 20, "127.0.0.1");
    libIni.GetConfigParamString(ConfigString, "PortType", SType, 32, "ClientSocket");

    m_ShutDownFlag = true ;

    if ( strnicmp( "ServerSocket", SType, 32) == 0 )
//    if ( strncasecmp( "ServerSocket", SType, 32) == 0 )
    {
        //Socket is server type 
        m_SockType = SERVER_SOCKET;
    }
    else
    {
        //Default value for socket type
        m_SockType = CLIENT_SOCKET;
    }
 
    return true;
}

//////////////////////////////////////////////////////////////////////
// Connect function, see .h file for description
//////////////////////////////////////////////////////////////////////
bool CSocketPort::Connect()
{
    int ret = 0;
    bool retval = false;
    char buf[512] ;
    buf[0] = '\0' ;

//    m_ShutDownFlag = true ;

    m_SockAddr.sin_addr.s_addr = inet_addr(m_SrvIPAddr);  
    m_SockAddr.sin_family = AF_INET; 
    m_SockAddr.sin_port = htons(m_SrvPort); // Port MUST be in Network Byte Order 
    memset(&(m_SockAddr.sin_zero), 0, 8); // zero the rest of the struct
    m_SockAddrLen = sizeof(sockaddr);

    if( m_SockType == CLIENT_SOCKET )
    {
        m_hDriver = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP); 
        if (m_hDriver == INVALID_SOCKET)
        { 
            sprintf(buf,
                "failure creating socket (%d: %s)\n",
                this->GetLastError(),
                strerror(this->GetLastError())
                );

            DisplayMsg(buf);
            return false; 
        }
        
#ifdef __linux__
        int on = 1;
        if ( setsockopt ( m_hDriver, SOL_SOCKET, SO_REUSEADDR, &on, sizeof ( on ) ) == -1 )
        {

          sprintf(buf,
                  "An Error occurred in setting options for socket (%d - %s)",
                  this->GetLastError(),
                  strerror(this->GetLastError())
                 );

          DisplayMsg(buf);
          return false;
        }
#endif

        ret = connect(m_hDriver,(sockaddr*) &m_SockAddr, m_SockAddrLen);
        if ( ret == -1 ) 
        {
            sprintf(buf,
                "failure connecting socket to \"%s:%d\" (%d: %s)",
                m_SrvIPAddr,
                m_SrvPort,
                this->GetLastError(),
                strerror(this->GetLastError())
                );

            DisplayMsg(buf);
            return false;
        }
        else
        {
            //Per il client questo NON funziona: c'e' comunque un timeout interno di ca 16 minuti su Linux
//             //Provo ad attivare il keep alive con invio ad un minuto
//             setsockopt ( m_hDriver, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof ( on ) );
//             int firstKeepAlive = 60, nextKeepAlive=5 ;
//             //Il primo keepalive viene mandato dopo un minuto di inattivita'
//             setsockopt ( m_hDriver, IPPROTO_TCP, TCP_KEEPIDLE, &firstKeepAlive, sizeof ( firstKeepAlive ) );
//             //Se non c'è risposta invio un messaggio ogni 5 secondi
//             setsockopt ( m_hDriver, IPPROTO_TCP, TCP_KEEPINTVL, &nextKeepAlive, sizeof ( nextKeepAlive ) );
            
            sprintf(buf,"Connected to \"%s:%d\" ",m_SrvIPAddr, m_SrvPort) ;
            m_IsConnected = 1;
            DisplayMsg(buf);
            return true;
        }
    }
    else 
    {
        if( (m_SockType == SERVER_SOCKET ) && (m_hLocalSrvr == INVALID_SOCKET) )
        {
           //m_hLocalSrvr = socket( AF_INET, SOCK_STREAM, 0);
            //TBC
            m_hLocalSrvr = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP);
#ifdef __linux__
          int on = 1;
          //if ( setsockopt ( m_hLocalSrvr, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on, sizeof ( on ) ) == -1 )
          //TBC
          if ( setsockopt ( m_hLocalSrvr, SOL_SOCKET, SO_REUSEADDR, &on, sizeof ( on ) ) == -1 )
          {              
                sprintf(buf,
                        "An Error occurred in setting options for socket (%d - %s)",
                        this->GetLastError(),
                        strerror(this->GetLastError())
                      );

                DisplayMsg(buf);
                return false;
          }
#endif
            if (m_hLocalSrvr == INVALID_SOCKET)
            {
                sprintf(buf,
                    "failure creating socket (%d: %s)",
                    this->GetLastError(),
                    strerror(this->GetLastError())
                    );
                retval = 0 ; 
            }

           if (m_hLocalSrvr != INVALID_SOCKET)
            {
               SetNonBlocking();
            }

            int ret = bind(m_hLocalSrvr,(sockaddr*) &m_SockAddr, m_SockAddrLen );

            if ( ret == -1 )
            {
                sprintf(buf,
                    "failure binding socket to \"%s:%d\" (%d: %s)",
                    m_SrvIPAddr,
                    m_SrvPort,
                    this->GetLastError(), 
                    strerror(this->GetLastError())
                    );

                retval = false; 
            }
            else
            {
                // Listen only if bind succeeded
                ret = listen(m_hLocalSrvr, 5)  ;
                if ( ret == -1 )
                {
                    sprintf(buf,
                        "failure listening to socket \"%s:%d\" (%d: %s)",
                        m_SrvIPAddr,
                        m_SrvPort,
                        this->GetLastError(),
                        strerror(this->GetLastError())
                        );

                    retval = false; 
                }
                else
                {
                    sprintf(buf,
                        "Listening on \"%s:%d\" ",
                        m_SrvIPAddr,
                        m_SrvPort);
 
                    retval = true; 
                }
            }
        }
    }
  
    if( strlen(buf) != 0 )
    {
      DisplayMsg(buf) ;
    }

    return retval ;

}


//////////////////////////////////////////////////////////////////////
// Disconnect function, see .h file for description
//////////////////////////////////////////////////////////////////////
bool CSocketPort::Disconnect()
{
    bool retval = true;
    char buf[512] ; buf[0] = '\0' ;
    
    if(m_IsConnected == false)
    {
        DisplayMsg("Socket already Closed");
    }
    else
    {
        if(m_hDriver == INVALID_SOCKET)
        {
        DisplayMsg("socket invalid in Disconnect") ;
        retval = false;
        }
        else
        {
        if( m_ShutDownFlag )
        {
            if( shutdown( m_hDriver, SD_BOTH ) == SOCKET_ERROR ) 
            {
                sprintf(buf,
                    "Error Shutting Down Server: (%d: %s)",
                    this->GetLastError(),
                    strerror(this->GetLastError()) );
                DisplayMsg(buf);
                retval = false;
            }
        }
    
        flushInFlow();
        if( closesocket( m_hDriver ) == SOCKET_ERROR)
        {
            sprintf(buf,
                "Error Closing socket: (%d: %s)",
                this->GetLastError(),
                strerror(this->GetLastError()) );
            DisplayMsg(buf);
            retval = false;
        }
        m_hDriver = INVALID_SOCKET ;
        }
    }
    
    m_IsConnected = false ;
    DisplayMsg("Socket Closed");
    
    return retval ;
}

//////////////////////////////////////////////////////////////////////
// Shutdown Server function, see .h file for description
//////////////////////////////////////////////////////////////////////
void CSocketPort::ShutdownServer()
{
    char buf[512];//Temporary buffer used for messages

    if (m_hLocalSrvr != INVALID_SOCKET)
    { 
      if( m_ShutDownFlag )
      {
        if( shutdown( m_hLocalSrvr, SD_BOTH ) == SOCKET_ERROR ) 
        {
            sprintf(buf,
                "Shutting Down LocalSrvr socket I got: (%d: %s)",
                this->GetLastError(),
                strerror(this->GetLastError())
                );

            DisplayMsg(buf) ;
        }
        else
        {
           sprintf(buf,
                    "Local Server on: %s %d -- Shutdown Complete",
                    m_SrvIPAddr,
                    m_SrvPort
                    );

            DisplayMsg(buf) ;
        }
      }        
 
      if( closesocket(m_hLocalSrvr) == SOCKET_ERROR)
      {
        sprintf(buf,
            "warning Closing LocalSrvr socket: (%d: %s)",
            this->GetLastError(),
            strerror(this->GetLastError())
            );

        DisplayMsg(buf) ;

      }
      
    m_hLocalSrvr = INVALID_SOCKET ;
    
  }
}


//////////////////////////////////////////////////////////////////////
// Read function, see .h file for description
//////////////////////////////////////////////////////////////////////
unsigned long CSocketPort::Read(char *Block, int MaxReadBlockLength)
{
    char Buffer[128];
    struct timeval Timeout;
    Timeout.tv_sec   = 0;	
    Timeout.tv_usec  = SELECT_TIMEOUT_MICROSECS;
    fd_set readfds;
    FD_ZERO(&readfds);
    int ret = 0;
    int nCharsRead = 0;

    if( m_SockType == SERVER_SOCKET && m_IsConnected == false )
    {
      CheckForRequest() ;
    }

    if( m_IsConnected == false ) return 0L;
    
    if (m_hDriver == INVALID_SOCKET)
    { 
      sprintf(Buffer,"socket invalid in Read()\n" ) ;
      DisplayMsg(Buffer);
      m_IsConnected = false;
      return 0L;
    } 

    //Check if it is a dummy read
    if (Block == NULL) return 0L;

    if (HasConnectionDropped())
    {
        Disconnect();
        return 0;
    }

    //Check for incoming messages
    FD_SET(m_hDriver, &readfds);
    
    ret = select(m_hDriver+1, &readfds, NULL, NULL, &Timeout);
    
    if( ret == SOCKET_ERROR )
    {
        sprintf(Buffer,
            "select error in read: (%d: %s)",
            this->GetLastError(),
            strerror(this->GetLastError())  );
        DisplayMsg(Buffer);
        return 0L;
    }
    
    
    if ( FD_ISSET(m_hDriver, &readfds))
    {   
        //Get incoming messages
        GetCharsToRead( &nCharsRead );
        
        if (nCharsRead != 0)
        { 
            //Ok, it's a valid message
            nCharsRead = recv(m_hDriver, Block, MaxReadBlockLength, 0 ) ;
        }
        else
        {   //The socket has been closed from the remote 
            FD_CLR(m_hDriver, &readfds);
            Disconnect();
            DisplayMsg( "connection closed by the remote" );
            nCharsRead = 0 ;
        }
    }
  
    return (unsigned long)nCharsRead;
}

///////////////////////////////////////////////////////////////////
//              flushInFlow
//          See SocketPort.h for details
///////////////////////////////////////////////////////////////////
bool CSocketPort::flushInFlow()
{
   bool retval  = true ;
   struct timeval Timeout;
   Timeout.tv_sec   = 0;	
   Timeout.tv_usec  = SELECT_TIMEOUT_MICROSECS;

   fd_set readfds;
   FD_ZERO(&readfds);

   int nCharsRead = 1;
//   DisplayMsg( "flushing input flow" );
   while( nCharsRead )
   {
      FD_ZERO(&readfds);
      FD_SET(m_hDriver, &readfds);
      int ret = select(m_hDriver+1, &readfds, NULL, NULL, &Timeout);

      if( ret == SOCKET_ERROR )
      {
           char buf[64] ;
           sprintf(buf,
                  "select error in flushInFlow: (%d: %s)",
                  this->GetLastError(),
                  strerror(this->GetLastError()) );
           DisplayMsg(buf);
           return false;
      }
      if ( FD_ISSET(m_hDriver, &readfds))
      {
         char Buffer[128];
         nCharsRead = recv(m_hDriver, Buffer, 128, 0 ) ;
         if( nCharsRead  == SOCKET_ERROR )break;
      }else break ;
   }
   return retval ;
}

///////////////////////////////////////////////////////////////////
//              CheckForRequest
//          See SocketPort.h for details
///////////////////////////////////////////////////////////////////
int CSocketPort::CheckForRequest(void)
{
  char Buffer[128];
  struct timeval Timeout;
  Timeout.tv_sec   = 0;	
  Timeout.tv_usec  = SELECT_TIMEOUT_MICROSECS;
  fd_set readfds;
  FD_ZERO(&readfds);
  
  FD_SET(m_hLocalSrvr, &readfds);
  int ret = select(m_hLocalSrvr+1, &readfds, NULL, NULL, &Timeout);
  if( ret == SOCKET_ERROR )
  {
    sprintf(Buffer,
       "select error in check for request: (%d: %s)",
       this->GetLastError(),
       strerror(this->GetLastError())
       );
    DisplayMsg(Buffer);
    return 1;
  }
  if ( !FD_ISSET(m_hLocalSrvr, &readfds)) return 2;

  m_hDriver = accept(m_hLocalSrvr,(sockaddr*)&m_RemoteAddr, &m_SockAddrLen); 

  if (m_hDriver == INVALID_SOCKET)
  {
    sprintf(Buffer,
        "failure accepting connection on \"%s:%d\" (%d)",
        m_SrvIPAddr,
        m_SrvPort,
       this->GetLastError());
     DisplayMsg( Buffer ) ;		
    return 3;
  }
  else
  { 
//      //Provo ad attivare il keep alive con invio ad un minuto
//      int on = 1;
//      setsockopt ( m_hDriver, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof ( on ) );
//      int firstKeepAlive = 60, nextKeepAlive=5 ;
//            //Il primo keepalive viene mandato dopo un minuto di inattivita'
//      setsockopt ( m_hDriver, IPPROTO_TCP, TCP_KEEPIDLE, &firstKeepAlive, sizeof ( firstKeepAlive ) );
//            //Se non c'è risposta invio un messaggio ogni 5 secondi
//      setsockopt ( m_hDriver, IPPROTO_TCP, TCP_KEEPINTVL, &nextKeepAlive, sizeof ( nextKeepAlive ) );
      
    m_IsConnected = 1 ;	
    sprintf(Buffer,
            "Accepted connection on \"%s:%d\"",
            m_SrvIPAddr,
            m_SrvPort);
    DisplayMsg( Buffer );
    return 0;
  }
}

//////////////////////////////////////////////////////////////////////
// Write function, see .h file for description
//////////////////////////////////////////////////////////////////////
unsigned long CSocketPort::Write(const char *Buffer, int BlockSize = -1)
{
    char ErrorMsg[64];
    long nWrittenChars = 0;
    unsigned long nTotalWrittenChars = 0L;
    struct timeval Timeout;
     fd_set writefds;
     
    if (m_IsConnected == false) return 0 ;
    if (m_hDriver == INVALID_SOCKET)
    { 
        sprintf(ErrorMsg,"socket invalid (Write)\n" ) ;
        DisplayMsg(ErrorMsg);
        m_IsConnected = false;
        return 0;
    } 
  
    if(BlockSize < 0 )
    {
        BlockSize = strlen(Buffer) ;
    }

   Timeout.tv_sec   = 0;	
   Timeout.tv_usec  = SELECT_TIMEOUT_MICROSECS;
  
   try 
   {
    while(nTotalWrittenChars < (unsigned long) BlockSize)
    {
        if (HasConnectionDropped())
        {
            Disconnect();
            break;
        }
        
        FD_ZERO(&writefds);
        FD_SET(m_hDriver, &writefds);
        int ret = select( m_hDriver+1, NULL, &writefds, NULL, &Timeout);
        if( ( ret == SOCKET_ERROR )||( ret > (m_hDriver+1) )||( ret < 0 ) )
        {
            sprintf(ErrorMsg,
                "select error in write: (%d: %s)",
                this->GetLastError(),
                strerror(this->GetLastError())  );
            DisplayMsg(ErrorMsg);
            Disconnect();
            return 0L;
        }
        if (ret == 0)
        {
            nTotalWrittenChars = 0;
            sprintf(ErrorMsg,
                "select returned 0 in write: (%d)",
                this->GetLastError()  );
            DisplayMsg(ErrorMsg);
            Disconnect();
            break;
        }
    
        if ( FD_ISSET(m_hDriver, &writefds))
        {
            nWrittenChars = send(  m_hDriver, 
                                    Buffer+nTotalWrittenChars, 
                                    BlockSize-nTotalWrittenChars, 0); 
            if(nWrittenChars <= 0)
            {
                sprintf(ErrorMsg, "WARNING: %ld bytes sent to socket", nWrittenChars );
                DisplayMsg(ErrorMsg);
                Disconnect() ;
                break;
            }
        }else{
            DisplayMsg(" socket not writeable ");
            Disconnect() ;
            break;
        }
        nTotalWrittenChars += nWrittenChars ;
    }
   }
   catch (exception &e)
   {
       cout << e.what() << endl;
   }
   return nTotalWrittenChars ;
}

///////////////////////////////////////////////////////////////////////////////
//CSocketPort GetLastError, see .h file for description
///////////////////////////////////////////////////////////////////////////////
int CSocketPort::GetLastError(void) 
{ 

#ifdef _WIN32
    int errnum = WSAGetLastError();

    WSASetLastError(0);

    return errnum;

#endif


// #ifdef __QNX__

    return errno;

// #endif

}

///////////////////////////////////////////////////////////////////////////////
//CSocketPort GetCharstoRead, see .h file for description
///////////////////////////////////////////////////////////////////////////////
int CSocketPort::GetCharsToRead (int *charsToRead)
{
#ifdef _WIN32

    return ioctlsocket(m_hDriver, FIONREAD, (unsigned long*)charsToRead);

#endif
   
#ifdef __QNX__

    return ioctl(m_hDriver, FIONREAD, charsToRead);

#endif

#ifdef __linux__

    return ioctl(m_hDriver, FIONREAD, charsToRead);

#endif
}


///////////////////////////////////////////////////////////////////////////////
//CSocketPort SetNonBlocking, see .h file for description
///////////////////////////////////////////////////////////////////////////////
void CSocketPort::SetNonBlocking(void)
{    

#ifdef _WIN32
    unsigned long non_block = 1; 
     ioctlsocket(m_hLocalSrvr, FIONBIO, &non_block);

#endif

#ifdef __QNX__
     unsigned long non_block = 1; 
     ioctl(m_hLocalSrvr, FIONBIO, &non_block);

#endif

#ifdef __linux__
  

     //Cambiata prendendo il codice da una libreria su soruceforge
     //int oldflags=fcntl(m_hLocalSrvr, F_GETFL,0);
     //TBC
     int oldflags=fcntl(m_hLocalSrvr, F_GETFL);
     if (oldflags == -1)
         return;

     oldflags |= O_NONBLOCK;
     fcntl(m_hLocalSrvr, F_SETFL, oldflags);

     return ;
     
// #ifdef CRIS
//   int opts;
//   opts = fcntl ( m_hLocalSrvr,
//                  F_GETFL );
// 
//   if ( opts < 0 )
//   {
//     return;
//   }
// 
//   opts = ( opts | O_NONBLOCK );
// 
//   ioctl(m_hLocalSrvr, FIONBIO, O_NONBLOCK);
// #else
//   //24/09/2008
//   //Cambiato il metodo per definire il socket non bloccante perche' valgrind segnalava operazione non valida
//   //anche se sembrava funzioanre il metodo vecchio
//   if (fcntl(m_hLocalSrvr, F_SETFL, O_NDELAY) < 0)
//   {
//       cout << "ATTENZIONE IL SOCKET NON PUO' ESSERE DEFINITO NON BLOCCANTE!!"<<endl;
//       sleep(10);
//   }
// #endif
  return;

  
#endif

}

unsigned long CSocketPort::Read( std::string & dest)
{
  char buffer[1024];
  int nCharsRead;
  
  memset (buffer, 0x0, 1024);
  nCharsRead = Read(buffer, 1024);
  dest = "";
  
  if (nCharsRead > 0)
  {
    dest = buffer;
  }
  
//   //TBR
//   cout << "SocketPort ha ricevuto: "<<buffer<<endl;
  return nCharsRead;
}


//////////////////////////////////////////////////////////////////////
// Write function, see .h file for description
//////////////////////////////////////////////////////////////////////
unsigned long CSocketPort::WriteClient(const char *Buffer, int BlockSize = -1)
{
    char ErrorMsg[64];

    if (m_IsConnected == false) return 0 ;
    if (m_hLocalSrvr == INVALID_SOCKET)
    { 
        sprintf(ErrorMsg,"socket invalid (Write)\n" ) ;
        DisplayMsg(ErrorMsg);
        m_IsConnected = false;
        return 0;
    } 
  
   
    if(BlockSize < 0 )BlockSize = strlen(Buffer) ;
    long nWrittenChars = 0;
    unsigned long nTotalWrittenChars = 0L;

    struct timeval Timeout;
    Timeout.tv_sec   = 0;    
    Timeout.tv_usec  = SELECT_TIMEOUT_MICROSECS;
    fd_set writefds;
    while(nTotalWrittenChars < (unsigned long) BlockSize)
    {
        FD_ZERO(&writefds);
        FD_SET(m_hLocalSrvr, &writefds);
        int ret = select( m_hLocalSrvr+1, NULL, &writefds, NULL, &Timeout);
        if( ( ret == SOCKET_ERROR )||( ret > (m_hLocalSrvr+1) )||( ret < 0 ) )
        {
            sprintf(ErrorMsg,
                    "select error in write: (%d: %s)",
                    this->GetLastError(),
                    strerror(this->GetLastError())  );
            DisplayMsg(ErrorMsg);
            return 0L;
        }
        if (ret == 0)
        {
            nTotalWrittenChars = 0;
            sprintf(ErrorMsg,
                    "select returned 0 in write: (%d)",
                    this->GetLastError()  );
            DisplayMsg(ErrorMsg);
            break;
        }

        if ( FD_ISSET(m_hLocalSrvr, &writefds))
        {
            nWrittenChars = send(  m_hLocalSrvr, 
                                   Buffer+nTotalWrittenChars, 
                                   BlockSize-nTotalWrittenChars, 0); 
            if(nWrittenChars <= 0)
            {
                sprintf(ErrorMsg, "WARNING: %ld bytes sent to socket", nWrittenChars );
                DisplayMsg(ErrorMsg);
                Disconnect() ;
                break;
            }
        }else{
            DisplayMsg(" socket not writeable ");
            Disconnect() ;
            break;
        }
        nTotalWrittenChars += nWrittenChars ;
    }
    return nTotalWrittenChars ;
}
////////////////////////////////////////////////////////////////////////
bool CSocketPort::HasConnectionDropped()
{
    bool bConnDropped = false;
    int iRet = 0;
    bool bOK = true;

    //22/07/2009
    return false;
    
//     struct timeval timeout = { 0, 0 };
//     fd_set readSocketSet;
// 
//     FD_ZERO( &readSocketSet );
//     FD_SET( m_hDriver, &readSocketSet );
// 
//     iRet = ::select( 0, &readSocketSet, NULL, NULL, &timeout );
//     bOK = ( iRet > 0 );
// 
//     if( bOK )
//     {
//         bOK = FD_ISSET( m_hDriver, &readSocketSet );
//     }
// 
//     if( bOK )
//     {
//         char szBuffer[1] = "";
//         iRet = ::recv( m_hDriver, szBuffer, 1, MSG_PEEK );
//         bOK = ( iRet > 0 );
//         if( !bOK )
//         {
//             int iError = errno;
//             bConnDropped = ( ( iError == ECONNREFUSED ) ||
//                     ( iError == ENOTCONN ) ||
//                     ( iRet == 0 ) ); //Graceful disconnect from other side.
//         }
//     }
// 
//     return( bConnDropped );

}

