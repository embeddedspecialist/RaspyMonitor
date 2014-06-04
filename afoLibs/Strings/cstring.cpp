#include "cstring.h"

CString::CString()
 : string()
{
}

CString::CString( const char * initString )
    : string(initString)
{
}

CString::CString( string initString )
    : string(initString)
{
}

// CString::~CString()
// {
//     delete this;
// }

void CString::append(const char *format, ...)
{
    char    strtmp[255];

    memset (strtmp,0,255);

    va_list args;
    va_start(args, format);

#ifdef _WINDOWS
        _vsnprintf( strtmp, 254, format, args );
#else
        vsnprintf( strtmp, 254, format, args );
#endif

        va_end(args);

        *this += strtmp;
}

void CString::add( char c )
{
    std::string tStr;
    
    tStr = c;
    
    *this += tStr;
}

CString CString::ToUpper()
{
    unsigned int i = 0;
    char *tempBuffer;
    
    if (this->size() == 0)
    {
        return *this;
    }
    
    tempBuffer = (char*)malloc(this->size()*sizeof(char)+1);
    if (tempBuffer != 0x0)
    {
        memset (tempBuffer, 0x0, this->size()*sizeof(char)+1);
        
        memcpy (tempBuffer, this->c_str(), this->size()*sizeof(char));
        
        
        for (i = 0; i < this->size(); i++)
        {
            tempBuffer[i] = toupper (tempBuffer[i]);
        }
        
        *this = tempBuffer;
        
        free (tempBuffer);
    }
    
    return *this;
}

CString CString::ToLower()
{
    unsigned int i = 0;
    char *tempBuffer;
    
    if (this->size() == 0)
    {
        return *this;
    }
    
    tempBuffer = (char*)malloc(this->size()*sizeof(char)+1);
    
    if (tempBuffer != 0x0)
    {
        memset (tempBuffer, 0x0, this->size()*sizeof(char)+1);
        
        memcpy (tempBuffer, this->c_str(), this->size()*sizeof(char));
        
        
        for (i = 0; i < this->size(); i++)
        {
            tempBuffer[i] = tolower (tempBuffer[i]);
        }

        *this = tempBuffer;
        
        free (tempBuffer);
    }
    
    return *this;
}

void CString::Split (vector<string>& tokens,const string& delimiters)
{
    // Skip delimiters at beginning.
    string::size_type lastPos =this->find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = this->find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(this->substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = this->find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = this->find_first_of(delimiters, lastPos);
    }
}
