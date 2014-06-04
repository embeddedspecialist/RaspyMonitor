////////////////////////////////////////////////////////////////////////////////
//!\class  CFilePort
//!                      FilePort
//!The FilePort is used to perform I/O operations on files. It works
//!under QNX and under Windows. <br>
//!The config string format is the following:<br>
//! - <B>PortType:File,</B>    - Type of port <br>
//! - <B>FileNamePath:XXX,</B> - File name  -- putting the key expression <I>SelfGenerated</I>
//!                    instead of the file name the system generates a file name 
//!                    using the clock time keeping the path and the extension<br>
//! - <B>Mode:w+,</B>          - Opening mode for the file: read or write, new or append
//! - <B>ReadMode:X,</B>       - Read Mode: 
//!     - 0 = All the file will be readen in one single operation
//!     - 1 = Each Read() will read on line
//////////////////////////////////////////////////////////////////////////////////
#if !defined FILEPORT_H_INCLUDED
#define FILEPORT_H_INCLUDED
//////////////////////////////////////////////////////////////////////
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//////////////////////////////////////////////////////////////////////
#include <fcntl.h>
#ifdef _WIN32
#include <io.h>
#endif

#ifdef __QNX__
#include <unistd.h>
#endif

//!Enumerates different Read Modes
typedef enum
{
    LINE_BY_LINE,
    ALL_IN_A_ROW
}Read_Mode;


#include "VPort.h"

class CFilePort : public CVPort
{
    public:

///////////////////////////////////////////////////////////////////////////////
//
//! Constructor
//
//!\param CMsgStream* msg = NULL    -   The messagestream to use for messages
//!
//!\return nothing
//!
//! <b>Actions :</b><br>
//!   Prepares the input buffer
//!
///////////////////////////////////////////////////////////////////////////////   
    CFilePort();
    
///////////////////////////////////////////////////////////////////////////////
//
//! Destructor
//
//!\param void
//!
//!\return nothing
//!
//! <b>Actions :</b><br>
//!   Calls Disconnect() to close the file
//!
///////////////////////////////////////////////////////////////////////////////
    ~CFilePort();

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
//
///////////////////////////////////////////////////////////////////////////////
    bool Init( const char* ) ;

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
//!\return returns the value returned by fclose
//!
//! <b>Actions :</b><br>
//!   Disconnects the Port closing the I/O.
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
//!   If m_ReadMode is ALL_IN_A_ROW it calls ReadAll() to read from the port up 
//!   to int bytes , otherwise it calls ReadLn() to read a single line,
//!
///////////////////////////////////////////////////////////////////////////////
    unsigned long Read(char *,int)	;


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
//!
///////////////////////////////////////////////////////////////////////////////
    unsigned long Write(const char *,int )	;

    //!Return the handle to the port
    FILE* GethDriver(){return m_hDriver	;} ;

///////////////////////////////////////////////////////////////////////////////
//
// GetFileNamePath
//!Returns the complete file name and path
//
//!\return the file name and path in use
//!
//! <b>Actions :</b><br>
//!   Retrieves the actually used fil ename and path
//!
///////////////////////////////////////////////////////////////////////////////
    char* GetFileNamePath() const
    {
      return (char*)m_FileNamePath;
    }
    

    private:
    //!Read mode, see Read()
    Read_Mode m_ReadMode;

///////////////////////////////////////////////////////////////////////////////
//
// Access
//!Check existence and access of a file
//
//!\param m_FileNamePath     -   Path to the file
//!\param mode               -   Access mode to the file
//!
//!\return 0 if the file exists and has correct access, otherwise -1, errno is set
//!
//! <b>Actions :</b><br>
//!   Common interface under QNX and WIN32 to system functions used to check the 
//!   existence and access mode to a file.
//!
///////////////////////////////////////////////////////////////////////////////
    int Access( char* m_FileNamePath, int mode );

///////////////////////////////////////////////////////////////////////////////
//
// ReadAll
//!Read all file in a single operation
//
//!\param buff     -   Buffer to store data
//!\param buffLen    -   Buffer size
//!
//!\return The number of bytes read
//!
//! <b>Actions :</b><br>
//!   Function used to read all the file in a single operation. It stops only
//!   when EOF has been reached or buffLen bytes have been read
//!
/////////////////////////////////////////////////////////////////////////////// 
    unsigned long ReadAll(char* buff, int buffLen);
    
///////////////////////////////////////////////////////////////////////////////
//
// ReadLn
//!Read a single line of data
//
//!\param buff     -   Buffer to store data
//!\param buffLen    -   Buffer size
//!
//!\return The number of bytes read
//!
//! <b>Actions :</b><br>
//!   Function used to read a single line from the file. It stops if it 
//!   encounters a \n char or when buffLen bytes have been read.
//!
/////////////////////////////////////////////////////////////////////////////// 
    unsigned long ReadLn(char* buff, int buffLen );
    
///////////////////////////////////////////////////////////////////////////////
//
// timeToString
//!Transform time to a string
//
//!\param p       -   Buffer to store string
//!\param maxLen    -   Buffer size
//!
//!\return void
//!
//! <b>Actions :</b><br>
//!   Read the actual time and writes it to a string. It is used to generate file 
//!   names.
//!
///////////////////////////////////////////////////////////////////////////////
    void timeToString( char *p, unsigned int maxLen );

    //!Handle to file
    FILE* m_hDriver ;

    //!File name
    char   m_FileNamePath[128] ;
    
    //!Access Mode
    char   m_Mode[4] ;
};

#endif // #ifndef FILEPORT_H_INCLUDED
