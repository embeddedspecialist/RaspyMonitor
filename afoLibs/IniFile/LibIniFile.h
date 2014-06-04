////////////////////////////////////////////////////////////////////////////////
//
// IniFile Library interface
//
////////////////////////////////////////////////////////////////////////////////
#ifndef INIFILE_H
#define INIFILE_H

#include <unistd.h>
#include <stdio.h> 
#include <stdlib.h>
#include <cstdlib>
#include <string>

#define MAX_CONFIG_BUF 1024

using namespace std;

class CLibIniFile
{
    public:
        CLibIniFile(){};
        ~CLibIniFile(){};
        
    // See implementation for description
    int GetPrivateProfileInt( const char *section,
                            const char *entry,
                                    int   defaultInt,
                            const char *fileName );
    
    // See implementation for description                          
    int GetPrivateProfileString( const char *section,
                                const char *entry,
                                const char *defaultString,
                                    char *buffer,
                                    int   bufLen,
                                const char *fileName );
    
    
    // See implementation for description
    short GetConfigParamString(const char* ConfigString,
                            const char* ParamName, 
                                    char* DestString, 
                                    int   MaxLen,
                            const char* DefaultString );
                            
    // See implementation for description
    short GetConfigParamString(const char* ConfigString,
                            const char* ParamName, 
                            string* DestString, 
                            const char* DefaultString );
                            
    // See implementation for description
    short GetConfigParamInt(const char* ConfigString,
                            const char* ParamName,
                                int*  Dest,
                            const int   Default );
    
    // See implementation for description
    short GetConfigParamBool(const char* ConfigString,
                            const char* ParamName, 
                                bool* Dest, 
                            const bool  Default );
                            
                            
    float GetConfigParamFloat (const char* ConfigString,
                                            const char* ParamName,
                                            float *Dest,
                                            const float Default);
    
    //See implementation for description                         
    int GetParamStringLength (const char* configString, 
                            const char* paramName, 
                                    int * stringLength, 
                            const int   defaultLength );
    
    bool SetConfigParamString (string *configString, const char* paramName, const char* newVal);
    
    bool AddConfigParamString (string *configString, const char* paramName, const char* newVal);

    bool ExistsConfigParam(string configStr, string param);
    
    private:
        int readEntry(       FILE *is,    // input
                             const char *entry, // input
                             char *buf,   // output
                             int  bufSize,// input
                             int  strip ); // input
        
        void stripQuotationChar( char *buf );
        
        char *textPos( char *buf, const char *entry );
        
        int gotoSection( FILE *is, const char *section );
        
        int containTitle( char *buf, const char *section );
        
        int isTitleLine( char *bufPtr );
        
        char *titlePos( char *buf, int *len );
};

#endif // ifndef INIFILE_H

