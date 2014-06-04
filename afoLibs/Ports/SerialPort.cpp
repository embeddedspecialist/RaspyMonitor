//////////////////////////////////////////////////////////////////////
//                   SerialPort.cpp
//              See SerialPort.h for details
//////////////////////////////////////////////////////////////////////
//WINDOWS Required, under QNX is a dummy file
#include "stdafx.h"

#include "SerialPort.h"
#include "LibIniFile.h"

#ifdef _WIN32

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////
//                  WIN32 - Constructor
//                  See SerialPor.h for details
//////////////////////////////////////////////////////////////
CSerialPort::CSerialPort()
   : CVPort( NULL )// base class constructor
{
   m_Serial   = INVALID_HANDLE_VALUE ;
}

//////////////////////////////////////////////////////////////
//                  WIN32 - Destructor
//                  See SerialPor.h for details
//////////////////////////////////////////////////////////////
CSerialPort::~CSerialPort()
{
   Disconnect() ;
}

//////////////////////////////////////////////////////////////
//                              WIN32 - Init
//                  See SerialPort.h for details
//////////////////////////////////////////////////////////////
bool CSerialPort::Init(char * ConfigString)
{
   GetConfigParamString(ConfigString, "PortName", m_Port, 16, "COM1" ) ;

   GetConfigParamInt(ConfigString, "BaudRate"   , &m_BaudRate , 38400 ) ;
   GetConfigParamInt(ConfigString, "Parity"      , &m_Parity , NO_PARITY ) ;
   GetConfigParamInt(ConfigString, "ByteSize"   , &m_ByteSize , 8 ) ;
   GetConfigParamInt(ConfigString, "StopBits"   , &m_StopBits , 2 ) ;
   GetConfigParamBool(ConfigString, "Xonoff_in"   , &m_Xonoff_in , 0 ) ;
   GetConfigParamBool(ConfigString, "Xonoff_out"   , &m_Xonoff_out , 0 ) ;

   return true;
}
//////////////////////////////////////////////////////////////
//                          WIN32 - Connect
//                  See SerialPort.h for details
//////////////////////////////////////////////////////////////
bool CSerialPort::Connect()
{
   Disconnect() ;
   m_Serial = CreateFile(m_Port, GENERIC_READ | GENERIC_WRITE, 
                        0, NULL, OPEN_EXISTING, 0, NULL) ;
   DCB dcb ;
   COMMTIMEOUTS cto ;

   if(m_Serial != INVALID_HANDLE_VALUE) {
      GetCommState( m_Serial, &dcb );
      GetCommTimeouts( m_Serial, &cto ); 

      dcb.DCBlength = sizeof(DCB) ;
      dcb.BaudRate = m_BaudRate ;
      dcb.Parity =   (unsigned char)  m_Parity ;
      dcb.ByteSize = (unsigned char)  m_ByteSize ;
      dcb.StopBits = (unsigned char)  m_StopBits ;
      dcb.fInX = m_Xonoff_in;            // XON/XOFF in flow control 
      dcb.fOutX = m_Xonoff_out;             // XON/XOFF out flow control 

      dcb.XonChar = XON;              // Tx and Rx XON character 
      dcb.XoffChar = XOFF;             // Tx and Rx XOFF character 

      dcb.fTXContinueOnXoff = false ; // XOFF continues Tx 

      dcb.XonLim = 1024;               // transmit XON threshold 
      dcb.XoffLim = 1024;              // transmit XOFF threshold 
    
      dcb.fBinary = true ;          // binary mode, no EOF check 
      dcb.fParity = false;          // enable parity checking 
      dcb.fOutxCtsFlow = false;      // CTS output flow control 
      dcb.fOutxDsrFlow = false;      // DSR output flow control 
      dcb.fDtrControl = DTR_CONTROL_DISABLE ; // 1

      dcb.fDsrSensitivity = false;   // DSR sensitivity 

      dcb.fErrorChar = false;       // enable error replacement 
      dcb.fNull = false;            // enable null stripping 
      dcb.fRtsControl = RTS_CONTROL_DISABLE ; // 3      // RTS flow control 
      dcb.fAbortOnError = true;     // 1  //abort reads/writes on error 
 
      cto.ReadIntervalTimeout = MAXDWORD ;
      cto.ReadTotalTimeoutMultiplier = 0 ;
      cto.ReadTotalTimeoutConstant = 0 ;
      cto.WriteTotalTimeoutMultiplier = 0 ;
      cto.WriteTotalTimeoutConstant = 0 ;

      SetCommTimeouts(m_Serial,&cto) ;
      SetCommState(m_Serial,&dcb) ;

      char Buffer[150] ;
      sprintf(Buffer,"Connected to: %s (bd %d)",m_Port,m_BaudRate) ;
      DisplayMsg(Buffer) ;

      return (m_IsConnected = true) ;
   } else {
      char Buffer[150] ;
      sprintf(Buffer,"Unable to connect: %s.",m_Port) ;
      DisplayMsg(Buffer) ;
      return false;
   }
   return true;
}

//////////////////////////////////////////////////////////////
//                          WIN32 - Disconnect
//                  See SerialPort.h for details
//////////////////////////////////////////////////////////////
bool CSerialPort::Disconnect()
{
   if( m_Serial == INVALID_HANDLE_VALUE )
   {
     return false;
   }

   m_IsConnected = false ;

   char buf[150] ;
   if( CloseHandle(m_Serial) ){ 
      m_Serial = INVALID_HANDLE_VALUE ;
      sprintf(buf,"%s Disconnected", m_Port ) ;
      DisplayMsg(buf) ;
      return true;
   }else{
      sprintf(buf,"Error in Disconnecting: %s - ERR_CODE:%d",m_Port,GetLastError()) ;
      DisplayMsg(buf) ;
      return false;
   }
}

//////////////////////////////////////////////////////////////
//                          WIN32 - Read
//                  See SerialPort.h for details
//////////////////////////////////////////////////////////////
unsigned long CSerialPort::Read(char *Block,int MaxReadBlockLength)
{
   unsigned long nCharsRead = 0;

   if(m_Serial != INVALID_HANDLE_VALUE) {
      int bResult = ReadFile(m_Serial,Block
                     ,MaxReadBlockLength,&nCharsRead,NULL) ;
      if(!bResult) {
         nCharsRead = 0 ;
         char buf[150] ;
         int err = GetLastError() ;
         sprintf(buf,"Error in Reading from: %s - ERR_CODE:%d"
                                 ,m_Port,err) ;

         
         Disconnect() ;

         DisplayMsg(buf) ;
      }
   }
   return nCharsRead ;
}

//////////////////////////////////////////////////////////////
//                          WIN32 - Write
//                  See SerialPort.h for details
//////////////////////////////////////////////////////////////
unsigned long CSerialPort::Write(const char *Buffer, int BlockSize = -1)
{
   if (m_IsConnected == false) return 0 ;

   if (m_Serial == INVALID_HANDLE_VALUE){ 
      DisplayMsg("Serial invalid (Write)\n") ;
      m_IsConnected = false ;
      return 0;
   } 

   unsigned long nWrittenChars = 0 ;
   unsigned long nTotalWrittenChars = 0L ;
   char *p = Buffer ;

   if(BlockSize < 0 ) BlockSize = strlen(Buffer) ;

   while( nTotalWrittenChars < (unsigned int) BlockSize ) {//flush ALL of the data
      int bResult = WriteFile(m_Serial, p+nTotalWrittenChars, BlockSize, &nWrittenChars, NULL) ;
      nTotalWrittenChars += nWrittenChars ;

      if(!bResult) {
         char buf[150] ;
         sprintf(buf,"Error in Writing on: %s - ERR_CODE:%d",m_Port,GetLastError()) ;
         DisplayMsg(buf) ;
         return nTotalWrittenChars ;
      }
   }
   return nTotalWrittenChars ;
}

#endif // _WIN32



#ifdef __QNX__

/////////////////////////////////////////////////////////////////////
// SerialPort.cpp: QNX implementation of the CSerialPort class.
//
//////////////////////////////////////////////////////////////////////
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <iostream.h>

//////////////////////////////////////////////////////////////
//                          QNX - Constructor
//                  See SerialPor.h for details
//////////////////////////////////////////////////////////////
CSerialPort::CSerialPort()
{
   m_IsConnected = false ;
   m_Serial = NULL;
}

//////////////////////////////////////////////////////////////
//                        QNX - Destructor
//                  See SerialPor.h for details
//////////////////////////////////////////////////////////////
CSerialPort::~CSerialPort()
{
   Disconnect() ;
}

//////////////////////////////////////////////////////////////
//                        QNX - Init
//                  See SerialPor.h for details
//////////////////////////////////////////////////////////////
bool CSerialPort::Init( char* configString )
{
   int load = 0;

    DisplayMsg("Initialisation of a Serial Port");

    load =GetConfigParamString(configString, "PortName", m_Port, 16, "/dev/ser1" ) ;
    
   if (load == -1) 
    {
      DisplayMsg("Could not find PortName default used");
    }
      
   load =GetConfigParamInt(configString, "BaudRate"   , &m_BaudRate , 38400 ) ;
   if (load == -1) 
    {
      DisplayMsg("Could not find BaudRate default used");
    }
      
   load =GetConfigParamInt(configString, "Parity"   , &m_Parity , NO_PARITY ) ;
    if (load == -1) 
    {
      DisplayMsg("Could not find Parity default used");
    }
      
   load =GetConfigParamInt(configString, "ByteSize"   , &m_ByteSize , 8 ) ;
   if (load == -1) 
    {
      DisplayMsg("Could not find ByteSize default used");
    }
         
   load =GetConfigParamInt(configString, "StopBits"   , &m_StopBits , 2 ) ;
   if (load == -1) 
    {
      DisplayMsg("Could not find StopBits default used");
    }
      
   load =GetConfigParamBool(configString, "Xonoff_in"   , &m_Xonoff_in , 0 ) ;
   if (load == -1) 
    {
      DisplayMsg("Could not find Xonoff_in default used");
    }
      
   load =GetConfigParamBool(configString, "Xonoff_out"   , &m_Xonoff_out , 0 ) ;
   if (load == -1) 
    {
      DisplayMsg("Could not find Xonoff_out default used");
    }

   return true;
}
//////////////////////////////////////////////////////////////
//                        QNX - Connect
//                  See SerialPor.h for details
//////////////////////////////////////////////////////////////
bool CSerialPort::Connect()
{
   struct termios options;
   char Buffer[150] ;

   Disconnect() ;
   
   // open the serial com port
   char buffer[128];
   
   m_Serial = NULL;
   m_Serial = fopen(m_Port,"r+");

   if (m_Serial == NULL)   
   {
      //  Could not open the port
      sprintf(Buffer,"Impossible to connect: %s.",m_Port) ;
      DisplayMsg(Buffer) ;  
      sprintf(buffer,"Errno %d %s\n",errno,strerror(errno));
      DisplayMsg(buffer);

      return false;
   }
   else
   {
      // Port is opened successfully
      // Get the current option of the port
      tcgetattr( this->GethDriver() , &options);

      // Set the baud rate
      switch (m_BaudRate)
      {
      case 300:
         cfsetispeed(&options,B300);
         cfsetospeed(&options,B300);
         break;
      case 1200:
         cfsetispeed(&options,B1200);
         cfsetospeed(&options,B1200);
         break;
      case 2400:
         cfsetispeed(&options,B2400);
         cfsetospeed(&options,B2400);
         break;
      case 4800:
         cfsetispeed(&options,B4800);
         cfsetospeed(&options,B4800);
         break;
      case 9600:
         cfsetispeed(&options,B9600);
         cfsetospeed(&options,B9600);
         break;
      case 19200:
         cfsetispeed(&options,B19200);
         cfsetospeed(&options,B19200);
         break;
      case 38400:
         cfsetispeed(&options,B38400);
         cfsetospeed(&options,B38400);
         break;
      case 56700:
         cfsetispeed(&options,B57600);
         cfsetospeed(&options,B57600);
         break;
      case 115200:
         cfsetispeed(&options,B115200);
         cfsetospeed(&options,B115200);
         break;
      }

   
      switch (m_Parity) 
      {
         case  NO_PARITY:
            options.c_cflag &=~PARENB;
            break;
         case ODD_PARITY:
            options.c_cflag |=PARENB;
            options.c_cflag |=PARODD;
            break;
         case EVEN_PARITY:
            options.c_cflag |=PARENB;
            options.c_cflag &=~PARODD;
            break;
      }
       
      switch (m_ByteSize) 
      {
         case  8:
            options.c_cflag &=~CSIZE;
            options.c_cflag |=CS8;
            break;
         case  7:
            options.c_cflag &=~CSIZE;
            options.c_cflag |=CS7;
            break;
         case  6:
            options.c_cflag &=~CSIZE;
            options.c_cflag |=CS6;
            break;
         case  5:
            options.c_cflag &=~CSIZE;
            options.c_cflag |=CS7;
            break;
      }
    
      switch (m_StopBits) 
      {
         case  ONE_STOPBIT:
            options.c_cflag &=~CSTOPB; 
            break;
         case TWO_STOPBITS:
            options.c_cflag |=CSTOPB;         
            break;
      }
 
      options.c_cflag |= CREAD;
      options.c_oflag &= ~OPOST;
      
 
      // Set the new option for the port
      tcsetattr( this->GethDriver() ,TCSANOW,&options);

      sprintf(Buffer,"Connected to: %s (bd %d)",m_Port,m_BaudRate) ;
      DisplayMsg(Buffer) ;

  } // end else

  return (m_IsConnected = true) ;
}

//////////////////////////////////////////////////////////////
//                        QNX - Disconnect
//                  See SerialPor.h for details
//////////////////////////////////////////////////////////////
bool CSerialPort::Disconnect()
{
  if (m_Serial == NULL)
  {
    return true;
  }
   
   fclose(m_Serial);
   m_IsConnected = false;
   return true;
}

//////////////////////////////////////////////////////////////
//                        QNX - Read
//                  See SerialPor.h for details
//////////////////////////////////////////////////////////////
unsigned long CSerialPort::Read(char *Block,int MaxReadBlockLength)
{
   int nCharsRead = 0;      //Numebr of bytes read

    //!File descriptor set used by the select function
    fd_set fdset;
    //Timeout for the select operation
    struct timeval timeout;

   //Setting timeout to zero for poll mode
   timeout.tv_sec = 0;
   timeout.tv_usec = 0;

   //Initializing file descriptors set
   FD_ZERO( &fdset );

   //Adding current PORT to the fd set
   FD_SET( this->GethDriver() , &fdset );

   //TBR
   //cout<<"Polling PORT :"<<m_Port<<endl;cout.flush();
   //Polling PORT for incoming data
   select( ( this->GethDriver() )+1, &fdset, NULL, NULL, &timeout );


   //IF data awaits ... READ!!
   if (FD_ISSET( this->GethDriver() , &fdset ))
   {
     //nCharsRead=fread(Block, 1, MaxReadBlockLength, m_Serial);
     nCharsRead = read( this->GethDriver() , Block, MaxReadBlockLength);

   }

   //cout<<"Data Read :"<<nCharsRead<<endl;cout.flush();
   return nCharsRead ;
}

//////////////////////////////////////////////////////////////
//                        QNX - Write
//                  See SerialPor.h for details
//////////////////////////////////////////////////////////////
unsigned long CSerialPort::Write(char *Buffer, int BlockSize = -1)
{

   int nTotalWrittenChars = 0 ;
   int nWrittenChars = 0;
   char *p = Buffer ;

   if(BlockSize < 0 ) BlockSize = strlen(Buffer) ;

   //Checking if PORT is valid
   if(m_Serial != NULL)
   {
      
      while( nTotalWrittenChars < BlockSize ) 
      {
         //TBR
         //cout<<"Writing...:"<<p+nTotalWrittenChars<<endl;cout.flush();
         //flush ALL the data
         //nWrittenChars = fwrite((p+nTotalWrittenChars), 1, BlockSize, m_Serial);  
         nWrittenChars = write( this->GethDriver() , (p+nTotalWrittenChars), BlockSize);  

         if (nWrittenChars == -1)
         {
            DisplayMsg("Error writing on serial port: ");
            DisplayMsg(strerror( errno ));
            return (unsigned long)nTotalWrittenChars;
         }

         nTotalWrittenChars += nWrittenChars ;
      }

      return (unsigned long)nTotalWrittenChars ;
   }

   return 0 ;
}

#endif // __QNX__


#ifdef __linux__
/////////////////////////////////////////////////////////////////////
// SerialPort.cpp: linux implementation of the CSerialPort class.
//
//////////////////////////////////////////////////////////////////////
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <iostream.h>
#include <unistd.h>

//////////////////////////////////////////////////////////////
//                          linux - Constructor
//////////////////////////////////////////////////////////////
CSerialPort::CSerialPort()
{
  m_BaudRate = 0;
  m_Parity = 0 ;
  m_ByteSize = 0 ;
  m_StopBits = 0;
  m_Xonoff_in = false;
  m_Xonoff_out = false;
  m_IsConnected = false;
  m_Serial = -1;
    m_Port[0] = '\0' ;
}
//////////////////////////////////////////////////////////////
//                        linux - Destructor
//////////////////////////////////////////////////////////////
CSerialPort::~CSerialPort()
{
   Disconnect() ;
}
//////////////////////////////////////////////////////////////
//                        linux - Init
//////////////////////////////////////////////////////////////

bool CSerialPort::Init( char* configString )
{
    int load = 0;
    DisplayMsg("Initialisation of a Serial Port");

    load = GetConfigParamString(configString, "PortName", m_Port, 16, "/dev/ttyS1" ) ;
    if (load == -1) DisplayMsg("Could not find PortName default used");
    load = GetConfigParamInt(configString, "BaudRate"   , &m_BaudRate , 38400 ) ;
    if (load == -1) DisplayMsg("Could not find BaudRate default used");
    load = GetConfigParamInt(configString, "Parity"   , &m_Parity , NO_PARITY ) ;
    if (load == -1) DisplayMsg("Could not find Parity default used");
    load = GetConfigParamInt(configString, "ByteSize"   , &m_ByteSize , 8 ) ;
    if (load == -1) DisplayMsg("Could not find ByteSize default used");
    load = GetConfigParamInt(configString, "StopBits"   , &m_StopBits , 2 ) ;
    if (load == -1) DisplayMsg("Could not find StopBits default used");
    load = GetConfigParamBool(configString, "Xonoff_in"   , &m_Xonoff_in , 0 ) ;
    if (load == -1) DisplayMsg("Could not find Xonoff_in default used");
    load = GetConfigParamBool(configString, "Xonoff_out"   , &m_Xonoff_out , 0 ) ;
    if (load == -1) DisplayMsg("Could not find Xonoff_out default used");

    return true;
}
//////////////////////////////////////////////////////////////
//                        linux - Connect
//////////////////////////////////////////////////////////////
bool CSerialPort::Connect()
{
    struct termios options;
    char msgBuff[150] ;

    Disconnect() ;

    // open the serial com port
    m_Serial = -1 ;
    m_Serial = open(m_Port, O_RDWR | O_NOCTTY );

    if (m_Serial < 0 )
    {  //  Could not open the port
        sprintf( msgBuff, "Impossible to connect: %s.", m_Port) ;
        DisplayMsg( msgBuff ) ;
        return false;
    }
    // Port is opened successfully, get the current setting of the port and clear it
    tcgetattr( this->GethDriver() , &options);
    bzero(&options , sizeof(options)); 

    int baudRate_cFlag = 0 ;
    // Set the baud rate
    switch (m_BaudRate)
    {
      case 300:    baudRate_cFlag = B300   ; break ;
      case 1200:   baudRate_cFlag = B1200  ; break ;
      case 2400:   baudRate_cFlag = B2400  ; break ;
      case 4800:   baudRate_cFlag = B4800  ; break ;
      case 9600:   baudRate_cFlag = B9600  ; break ;
      case 19200:  baudRate_cFlag = B19200 ; break ;
      case 38400:  baudRate_cFlag = B38400 ; break ;
      case 56700:  baudRate_cFlag = B57600 ; break ;
      case 115200: baudRate_cFlag = B115200; break ;
      default :    baudRate_cFlag = B9600  ; break ;
    }
    options.c_cflag = baudRate_cFlag | CREAD;

    options.c_cflag |= PARENB;
    switch (m_Parity)
    {
        case  NO_PARITY:  options.c_cflag &=~PARENB; break;
        case ODD_PARITY:  options.c_cflag |=PARODD;  break;
        case EVEN_PARITY: options.c_cflag &=~PARODD; break;
    }

    options.c_cflag &= ~ CSIZE ;
    switch (m_ByteSize)
    {
        case  8: options.c_cflag |=CS8 ; break;
        case  7: options.c_cflag |=CS7 ; break;
        case  6: options.c_cflag |=CS6 ; break;
        case  5: options.c_cflag |=CS5 ; break;
        default: options.c_cflag |=CS5 ; break;
    }

    switch (m_StopBits)
    {
         case  ONE_STOPBIT: options.c_cflag &=~CSTOPB; break;
         case TWO_STOPBITS: options.c_cflag |=CSTOPB; break;
    }

    options.c_cflag |= CREAD;
    
    //Ignore modem control lines
    options.c_cflag |= CLOCAL;
    
    options.c_oflag &= ~OPOST;
    
    options.c_lflag = 0;
    
    //????
    options.c_cc[VTIME] = 0;


    tcflush(this->GethDriver(), TCIFLUSH);
    
      // Set the new option for the port
    tcsetattr( this->GethDriver() ,TCSANOW, &options);

    sprintf( msgBuff, "Connected to: %s (bd %d)", m_Port, m_BaudRate) ;
    DisplayMsg( msgBuff ) ;
    
    m_Options = options;

    return (m_IsConnected = true) ;
}

//////////////////////////////////////////////////////////////
//                        linux - Disconnect
//////////////////////////////////////////////////////////////
bool CSerialPort::Disconnect()
{
    if (m_Serial < 0 ) return false;

    tcsendbreak(m_Serial, 0);
    
    tcflush(m_Serial, TCIOFLUSH);

    
    if (!close(m_Serial))
    {
      
      m_Serial = -1;
      
      m_IsConnected = false;
      return true;
    }
    else
    {
      switch( errno )
      {
        case EBADF : DisplayMsg("Serial Port close error: An invalid file descriptor was given in one of the sets"); break;
        case EINTR : DisplayMsg("Serial Port close error: A non blocked signal was caught"); break;
        case EIO : DisplayMsg("Serial Port close error: n. of file descriptors is negative"); break;
        default : DisplayMsg("Serial Port close error"); 
      }
      
      return false;
    }
}

//////////////////////////////////////////////////////////////
//                        linux - Read
//////////////////////////////////////////////////////////////
unsigned long CSerialPort::Read(char *Block,int MaxReadBlockLength)
{
    int nCharsRead = 0;      //Numebr of bytes read

    fd_set fdset;
    struct timeval timeout;

   //Setting timeout to zero for poll mode
   timeout.tv_sec = 0;
   timeout.tv_usec = 1;

   //Initializing file descriptors set
   FD_ZERO( &fdset );
   //Adding current PORT to the fd set
   FD_SET( this->GethDriver() , &fdset );
   int ret = select( ( this->GethDriver() )+1, &fdset, NULL, NULL, &timeout );

   if( ret < 0 )
   {
       switch( errno )
       {
           case EBADF : DisplayMsg("Serial Port select error: An invalid file descriptor was given in one of the sets"); break;
           case EINTR : DisplayMsg("Serial Port select error: A non blocked signal was caught"); break;
           case EINVAL : DisplayMsg("Serial Port select error: n. of file descriptors is negative"); break;
           case ENOMEM : DisplayMsg("Serial Port select error: unable to allocate memory for internal tables"); break;
           default : DisplayMsg("Serial Port select error"); 
       }
   }

   //IF data awaits ... READ!!
   if (FD_ISSET( this->GethDriver() , &fdset ))
   {
     //nCharsRead=fread(Block, 1, MaxReadBlockLength, m_Serial);
     nCharsRead = read( this->GethDriver() , Block, MaxReadBlockLength);
   }

   //cout<<"Data Read :"<<nCharsRead<<endl;cout.flush();
   return nCharsRead ;
}

//////////////////////////////////////////////////////////////
//                        linux - Write
//////////////////////////////////////////////////////////////
unsigned long CSerialPort::Write(const char *Buffer, int BlockSize = -1)
{

   int nTotalWrittenChars = 0 ;
   int nWrittenChars = 0;
   char *p = (char*)Buffer ;

   if(BlockSize < 0 ) BlockSize = strlen(Buffer) ;

   //Checking if PORT is valid
   if(m_Serial > 1 )
   {
      while( nTotalWrittenChars < BlockSize )
      {
         nWrittenChars = write( this->GethDriver() , (p+nTotalWrittenChars), BlockSize);

         if (nWrittenChars == -1)
         {
            DisplayMsg("Error writing on serial port: ");
            DisplayMsg(strerror( errno ));
            return (unsigned long)nTotalWrittenChars;
         }
         nTotalWrittenChars += nWrittenChars ;
      }
      return (unsigned long)nTotalWrittenChars ;
   }
   return 0 ;
}

#endif // __linux__


