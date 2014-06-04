//////////////////////////////////////////////////////////////////////////////
//                                CFilePort Class
//                            See FilePort.h for details
//////////////////////////////////////////////////////////////////////////////
#ifdef __QNX__
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>


#include "LibIniFile.h"
#include "FilePort.h"


//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
//////////////////////////////////////////////////////////////////////////////
//                                Constructor
//                            See FilePort.h for details
//////////////////////////////////////////////////////////////////////////////
CFilePort::CFilePort()
: CVPort( NULL )// base class constructor
{
    m_hDriver                           = NULL ;
    memset( m_FileNamePath, 0, 128*sizeof(char) );
}

//////////////////////////////////////////////////////////////////////////////
//                                Destructor
//                            See FilePort.h for details
//////////////////////////////////////////////////////////////////////////////
CFilePort::~CFilePort()
{
    Disconnect() ;
}

//////////////////////////////////////////////////////////////////////////////
//                                Init
//                            See FilePort.h for details
//////////////////////////////////////////////////////////////////////////////
bool CFilePort::Init(const char * ConfigString)
{
    int readMode = 0;

    m_IniLib.GetConfigParamString(ConfigString, "FileNamePath", m_FileNamePath, 128, "" ) ;

    //Check if self generated
    if( strstr( m_FileNamePath, "SelfGenerated" ) != NULL )
    {
        char ext[32] ;
        char *p = NULL , *p1 = NULL ;
        if ( (p = strchr( m_FileNamePath, '.' )) != NULL )
        {
            strcpy( ext, p ) ;
        }
        p = m_FileNamePath ;
        while( (p1 = strchr( p, '\\' )) != NULL )
        {
            p = p1+1;
        }

        p++;
        timeToString( p, 128 - (p - m_FileNamePath) ) ;

        while( *p != '\0' ){
            if(   ((*p < '0')||(*p > '9'))
            &&((*p < 'A')||(*p > 'Z'))
            &&((*p < 'a')||(*p > 'z')) )
            {
                *p = '_' ;
            }
            p++ ;
        }
        sprintf( p, "%s", ext );
    }

    m_IniLib.GetConfigParamString(ConfigString, "Mode", m_Mode, 128, "w+" ) ;

    m_IniLib.GetConfigParamInt(ConfigString, "ReadMode", &readMode , 1 ) ;
    m_ReadMode = (Read_Mode)readMode;

    return true;
}
//////////////////////////////////////////////////////////////////////////////
//                                Connect
//                            See FilePort.h for details
//////////////////////////////////////////////////////////////////////////////
bool CFilePort::Connect()
{
    //   if( ( strlen( m_FileNamePath ) <= 0 )||( Access( m_FileNamePath, 0 ) == -1 ) )
    if( strlen( m_FileNamePath ) == 0  )
    {
        char buf[150] ;
        sprintf(buf,"Invalid Path: %s",m_FileNamePath) ;
        m_IsConnected = false ;
        DisplayMsg(buf) ;
        return false;
    }

    m_hDriver = fopen( m_FileNamePath, m_Mode ) ;

    char buf[150] ;
    if( m_hDriver != NULL )
    {
//         sprintf(buf,"Opened: %s",m_FileNamePath) ;
        m_IsConnected = true ;
    }
    else
    {
        sprintf(buf,"Unable to open: %s",m_FileNamePath) ;
        m_IsConnected = false ;
    }

//    DisplayMsg(buf) ;

    return m_IsConnected ;
}

//////////////////////////////////////////////////////////////////////////////
//                                Destructor
//                            See FilePort.h for details
//////////////////////////////////////////////////////////////////////////////
bool CFilePort::Disconnect()
{
    char buf[150] ;
    int retVal = 0;
    
    if( m_hDriver != NULL )
    {
//         sprintf(buf,"Closed: %s",m_FileNamePath) ;
//         DisplayMsg(buf) ;
    }
    else
    {
        sprintf(buf,"Null handle in closing: %s",m_FileNamePath) ;
        DisplayMsg(buf) ;
        return false ;
    }

    //Flush & close
    fflush(m_hDriver);
    retVal = fclose( m_hDriver ) ;

    if (!retVal)
    {
        //Port closed
        m_hDriver = NULL ;
        m_IsConnected = false ;
    }

    return retVal ;
}

//////////////////////////////////////////////////////////////////////////////
//                                Read
//                            See FilePort.h for details
//////////////////////////////////////////////////////////////////////////////
unsigned long CFilePort::Read(char* buff, int count)
{
    if (m_ReadMode == LINE_BY_LINE)
    {
        return ReadLn(buff, count);
    }
    else
    {
        return ReadAll(buff, count );
    }
}

//////////////////////////////////////////////////////////////////////////////
//                                ReadAll
//                            See FilePort.h for details
//////////////////////////////////////////////////////////////////////////////
unsigned long CFilePort::ReadAll(char* buff, int count)
{
    unsigned long ul ;
    char *p = NULL ;

    if( (m_hDriver == NULL)||(m_IsConnected == false) )
    {
        return 0 ;
    }

    p = strchr( m_Mode, 'r' );
    if( p == NULL )
    {
        p = strchr( m_Mode, '+' );
    }

    if( p == NULL )
    {
        // not opened in any read mode
        return 0;
    }

    ul = fread( buff, sizeof(char), count, m_hDriver ) ;

    // reset filepointer to the beginning of the file
    fseek( m_hDriver, 0L, SEEK_SET );

    return ( ul );
}

//////////////////////////////////////////////////////////////////////////////
//                                ReadLn
//                            See FilePort.h for details
//////////////////////////////////////////////////////////////////////////////
unsigned long CFilePort::ReadLn(char* buff, int buffLen )
{
    int retval = 1 ;
    int ch = '\0' ;
    char * p = buff;
    int count = 0 ;
    char delim='\n';

    //Free buffer
    memset ( buff, '\0', buffLen);

    while(1)
    {
        ch = fgetc (m_hDriver);
        if( ++count >= buffLen ) break;
        if( ch == EOF )
        {
            retval = 0;
            break;
        }
        *p++ = ch ;
        if( ch == delim) break;
    }
    *p = '\0' ;

    return strlen(buff); ;
};

//////////////////////////////////////////////////////////////////////////////
//                                Write
//                            See FilePort.h for details
//////////////////////////////////////////////////////////////////////////////
unsigned long CFilePort::Write(const char* buff, int count = -1 )
{
    char *p = NULL ;

    if( (m_hDriver == NULL)||(m_IsConnected == false) )
    {
        return 0 ;
    }

    p = strchr( m_Mode, 'w' );
    if( p == NULL )
    {
        p = strchr( m_Mode, '+' );
    }

    if( p == NULL )
    {
        p = strchr( m_Mode, 'a' );
    }

    if( p == NULL )
    {
        // not opened in any write mode
        return 0;
    }

    unsigned long ret = 0;

    if( count == -1 )
    {// it's a null term string
        ret = ( unsigned long ) fprintf(m_hDriver, "%s", buff);
    }
    else
    {
        //  size_t fwrite( const void * buffer, size_t size, size_t count, FILE *stream) ;
        ret = ( unsigned long ) fwrite( buff, 1, count, m_hDriver ) ;
    }

    fflush( m_hDriver ) ;

    return ( ret ) ;
}

///////////////////////////////////////////////////////
//                  Access
//              See FilePort.h
////////////////////////////////////////////////////////
int CFilePort::Access( char* m_FileNamePath, int mode )
{
#ifdef _WIN32
    return _access( m_FileNamePath, mode );
#endif

#ifdef __QNX__
    return access( m_FileNamePath, mode );
#endif
}


///////////////////////////////////////////////////////
//               timeToString
//              See FilePort.h
////////////////////////////////////////////////////////
void CFilePort::timeToString( char *p, unsigned int maxLen )
{ 
#ifdef __QNX__
    if( maxLen > strlen( "randomName"  ) )
    {
        sprintf( p, "randomName"  );
        DisplayMsg("FileName SelfGenerator not implemented");
    }
#endif

#ifdef _WIN32
    time_t ltime;
    time( &ltime );
    if( maxLen > strlen( ctime( &ltime ) ) )
    {
        sprintf( p, "%s", ctime( &ltime ) );
    }
#endif
}
