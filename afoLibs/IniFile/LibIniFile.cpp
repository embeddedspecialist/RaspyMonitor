////////////////////////////////////////////////////////////////////////////////
//
// IniFile library implementation
//
////////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "LibIniFile.h"

#ifdef __linux__

#define strnicmp strncasecmp

#endif // __linux__


///////////////////////////////////////////////////////////////////////////////
//
//titlePos
//
//Parameters : char *     string line to analyze in which to search the title
//             int *      pointer to the length of the title found (output)
//
//Return type : char *    pointer to the value of the found title
//
//Action :
//   Get a pointer to a section's title from the string to analyze and output 
//   the length of the section's title in the pointer to the length of the 
//   title found. The title must be between [ and ] characters. 
//   Return 0 if the first character encountered that is not a space is not [.
//   Return 0 if the title is not ended by ].
//   If 0 is returned, the length of title is not valid and stays unchanged.
//
///////////////////////////////////////////////////////////////////////////////
char* CLibIniFile::titlePos( char *buf, int *len )
{
   char *p = buf, *q;

   while( *p && isspace(*p) )
   {
      p++;
   }
   
   if( *p != '[' )
   {
      return 0;
   }

   q = p+1;
   while( *q && *q != ']' )
   {
      q++;
   }
   
   if( *q != ']' )
   {
      return 0;
   }
   
   if( len )
   {
      *len = (q - p - 1);
   }
   
   return p+1;
   
}

///////////////////////////////////////////////////////////////////////////////
//
//isTitleLine
//
//Parameters : char *   line to process
//
//Return types : int    logic value (0 or 1)
//
//Actions :
//   Check if a string is a section title line, return 0 if false, return 1
//   if true.
//
///////////////////////////////////////////////////////////////////////////////
int CLibIniFile::isTitleLine( char *bufPtr )
{
   return titlePos( bufPtr, 0 ) != 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//containTitle
//
//Parameters :       char *  string in which to search the title of a section
//             const char *  searched title
//
//Return type : int          logic boolean value
//
//Actions :
//   Check if a string contain the searched title. Return 1 if it contains it,
//   otherwise, 0.
//
///////////////////////////////////////////////////////////////////////////////
int CLibIniFile::containTitle( char *buf, const char *section )
{
   char *p;
   int len;

   // Obtain the title from the string to search
   p = titlePos( buf, &len );
   
   if( p )
   {
      // The searched string contained a valid title
      if( (signed)strlen( section )   == len && 
          strnicmp( section, p, len ) == 0 )
      {
         // It is the one we were searching for
         return true;
      }
   }
   return false;
}

///////////////////////////////////////////////////////////////////////////////
//
//gotoSection
//
//Parameters :  FILE *        input file
//              const char *  name of the section to go to
//
//Return type : int           logic boolean value
//
//Actions :
//   Move file position to start line of a section. Return true if the section
//   has been found, otherwise, false.
//
//Limitation :
//   The maximum read length of the lines in the input file is MAX_CONFIG_BUF
//
///////////////////////////////////////////////////////////////////////////////

int CLibIniFile::gotoSection( FILE *is, const char *section )
{
   char line[MAX_CONFIG_BUF];

   while( fgets(line, MAX_CONFIG_BUF, is ))
   {
      if( containTitle( line, section ) )
      {
         return true;
      }
   }
   
   return false;
   
}

///////////////////////////////////////////////////////////////////////////////
//
//textPos
//
//Parameters : char *        line to analyze
//             const char *  searched entry
//
//Return type : char *       pointer to the entry value
//
//Action :
//   Get a pointer to the value of the searched entry if it is present into
//   the line to analyze, otherwise, return 0. Return 0 also if the line to
//   analyze is a comment line.
//
///////////////////////////////////////////////////////////////////////////////
char* CLibIniFile::textPos( char *buf, const char *entry )
{
   char *p;
   unsigned int  len;
    
   // lines beginning with ; are comment lines in the .ini file
   if( buf[0] == ';' ) 
   {
      return 0;
   }

   // Put the pointer to the = character of the line
   p = strchr( buf, '=' );
   if( !p )
   {
      // Return 0 if there is no = character
      return 0;
   }

   // Compute the length of the entry name
   len = (p - buf);
   
   // Return a pointer to the value if the entry is the same
   if( strlen(entry) == len && strnicmp( buf, entry, len ) == 0 )
   {
      return p+1;
   }

   return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//stripQuotationChar
//
//Parameter : char *  String from which to strip the spaces characters
//
//Return type : void
//
//Action:
//   Strip all the spaces placed in front and at the end of the value in the 
//   input string.
//
///////////////////////////////////////////////////////////////////////////////
void CLibIniFile::stripQuotationChar( char *buf )
{
   char *p;
   char *q;
   int len;
   p = buf;

   while( *p && isspace(*p) )
   {
      p++;
   }
   q = p;

   while( *q && !isspace(*q) )
   {
      q++;
   }
   
   if( q == p )
   {
      return;
   }

   len =  q - p;
   memmove( buf, p, len );
   buf[len] = 0;
   
}

///////////////////////////////////////////////////////////////////////////////
//
//readEntry
//
//Parameters : FILE *        input file
//             const char *  entry from which obtain the value
//             char *        output buffer for the found value of the entry
//             int           maximum length allowed to put the value
//             int           enable(1)/disable(0) padding spaces stripping
//
//Return type : int          length of the found value, or -1 if not found
//
//Actions :
//   Get the value of an entry from the input file. The found value is copied
//   in the output buffer until the maximum length allowed to put the value is
//   reached.
//   Return the length of the found value, or -1 if the entry was not
//   found in the current section.
//
//Limitation :
//   The maximum line length read from the input file is MAX_CONFIG_BUF.
//   To be able to found the value, the actual position of the index in the 
//   input file must be at the end of the title of the scaned section.
//
///////////////////////////////////////////////////////////////////////////////
int CLibIniFile::readEntry(       FILE *is,    // input
                      const char *entry, // input
                            char *buf,   // output
                            int  bufSize,// input
                            int  strip ) // input
{
   char lineBuf[MAX_CONFIG_BUF];
   char *p, *cur;
   int  len;

   cur  = buf;
   *cur = '\0';
   len  = -1;

   while( fgets( lineBuf, MAX_CONFIG_BUF , is) )
   {

      if( isTitleLine( lineBuf ) )
      {
         // this is the next title line, the entry was not found in the
         // scaned section
         break;
      }

      p = textPos( lineBuf, entry );
      if( p == 0 )
      {
         // the entry at this line is not the entry searched
         continue;
      }

      // Strip the padding spaces if the caller enabled this behaviour
      if( strip )
      {
         stripQuotationChar( p );
      }
      
      // Compute the length of the value
      len = (strlen(p))-1;
      
      // Put a end of string character if there is enough space
      if( bufSize-1 < len )
      {
         p[bufSize-1] = '\0';
      }
      
      // Copy the found value in the output buffer for a maximum of
      // bufSize characters
      strncpy( cur, p, bufSize);
      break;
   }

   return len;
}


///////////////////////////////////////////////////////////////////////////////
//
//GetPrivateProfileString
//
//Parameters : const char *  Name of the section containing the entry
//             const char *  Name of the entry
//             const char *  Default value of the entry in case of failure
//                   char *  Output pointer to the found value
//                   int     Maximum allowed size of the found value
//             const char *  Input file name
//
//Return type :      int     Length of the used value (default or found)
//
//Actions :
//   Reads the file specified by the input file name, then step at the name of
//   the section containing the entry. If the section has been found, get the
//   value of the entry. 
//   If the section or the entry were not found, or if the input file cannot 
//   be opened, use the default value.
//
///////////////////////////////////////////////////////////////////////////////
int CLibIniFile::GetPrivateProfileString( const char *section,
                             const char *entry,
                             const char *defaultString,
                                   char *buffer,
                                   int   bufLen,
                             const char *fileName )
{

   FILE* is = NULL;
   int  len = -1;

   // Zero the buffer
   memset(buffer,'\0',bufLen);

   is = fopen(fileName, "r" );
      
   if( is == NULL ) 
   {
      //can not open the file, use default string
      strncpy( buffer, defaultString, bufLen-1 );
      len = strlen(buffer);
      return len;
   }     

   if( gotoSection( is, section ) )
   {
      // The section has been found, get the value of the entry
      len = readEntry(is, entry, buffer, bufLen, true);
   }

   if( len < 0 ) 
   {
      //cannot read entry, use default string
      strncpy( buffer, defaultString, bufLen-1 );
      len = strlen(buffer);
   }
   
   fseek(is, 0L, SEEK_SET);
      
   if (fclose(is) != 0)
   {
     printf ("LibIniFile: error in closing file");
   }
   
   is = NULL;
   
   return len;
}

///////////////////////////////////////////////////////////////////////////////
//
//GetPrivateProfileInt
//
//Parameters : const char *  Name of the section containing the entry
//             const char *  Name of the entry
//                   int     Default integer value to use in case of failure
//             const char *  Input file name
//
//Return type :      int     Value used (found or default)
//
//Action : 
//   Get the value of the entry in the specified section from the input file
//   as an integer.
//   Use the default integer value to use in case of failure.
//
///////////////////////////////////////////////////////////////////////////////
int CLibIniFile::GetPrivateProfileInt( const char *section,
                          const char *entry,
                                int   defaultInt,
                          const char *fileName )
{
   char buf[MAX_CONFIG_BUF];
   char iBuf[34];
      //"34" is max space "itoa" required under 32 bit C++
   
   memset(iBuf,0,34);

   // Convert the default value into a string
   sprintf (iBuf,"%d",defaultInt);
   
   // Get the value as a string, use the previously converted default value
   // in case of failure
   GetPrivateProfileString( section, entry, iBuf, 
                            buf, MAX_CONFIG_BUF, fileName);
   
   // Return the value as an integer
   return atoi( buf );
}


///////////////////////////////////////////////////////////////////////////////
//
//GetConfigParamString
//
//Parameters : const char *  Configuration string             (in)
//             const char *  Parameter Name                   (in)
//                   char *  Destination buffer               (out)
//             const char *  Default value in case of failure (in)
//
//Return type :      short     
//
//Action : 
//   Gets the value of the provided Parameter in the Given Configuration 
//   string and copies it in the provided Destination buffer. Return 0 if 
//   these operation occured fine. Otherwise, it uses the provided Default
//   value and copies it in Destination buffer. In this case, it returns -1.
//   Every paramaters have to be specified, otherwise the function returns
//   -2 without setting the Destination buffer.
//
//   The Configuration string format is:
//      Param1:Value1,Param2:Value2,Param3:Value3,...,ParamN:ValueN
//
///////////////////////////////////////////////////////////////////////////////
short CLibIniFile::GetConfigParamString(const char* ConfigString,
                           const char* ParamName, 
                                 char* Dest, 
                                 int   MaxLen,
                           const char* Default )
{
  char *P1 = NULL , *P2 = NULL, EndChar = ',', *tempStr = NULL;
   int retval = 0 ;
   
   char sep = '='; //Separator by default
      
      //Checking if the separator is present
   if (strchr(ConfigString, sep) == NULL)
   {
        //No default separator found, try with another
     sep = ':';
        
     if (strchr(ConfigString, sep) == NULL)
     {
          //No separator found, abort
       return -2;
     }
   }

  if( MaxLen < 0 ) MaxLen = 0 ;

  if( ConfigString != NULL && ParamName != NULL &&
      Dest         != NULL && Default   != NULL ) 
  {
    //copy the configuration string because it is a const char
    tempStr = new char[strlen(ConfigString)+1];
    
    if (tempStr!=NULL) 
    {
        memset (tempStr, 0x0, (strlen(ConfigString)+1)*sizeof(char));
      strcpy(tempStr, ConfigString);
          
      // Get a pointer to the given ParamName into ConfigString
      char *Param = strstr( tempStr, ParamName );

      while ((Param < tempStr + strlen (tempStr)) && (Param != 0x0))
      {
          if (*(Param+strlen(ParamName)) == sep)
          {
              if ( (Param == tempStr) || (*(Param-1)==' ') || (*(Param-1) == ',') )
              {
                 //We found the correct substring
                  break;
              }
          }

            //The substring found is not the one we searched for
            Param = strstr( Param+1, ParamName );

      }  
      
      if((Param != NULL) && (Param < tempStr + strlen (tempStr)))
      {
        P1 = (char*)strchr(Param, sep );
        if(P1 != NULL)
        {
            P2 = strchr( P1, EndChar );
            if( P2 == NULL )
            {
              // Try if The value is the last of the config string
              P2 = strchr(P1, '\0');
              EndChar = '\0';
            }
            if( P2 != NULL )
            {
              *P2 = '\0';              // Temporary end of string at the comma
              if( strlen( 1 + P1 ) < (unsigned int) MaxLen )
              {
                  strcpy( Dest , 1 + P1 ); // Copy the value in the destination buffer
                  *P2 = EndChar;           // Set back the comma ( or \0 )
                  delete []tempStr;        //free allocated memory
                  return 0;                // No error occured
              }
              else retval = -4 ;// MaxLen exceded
            }
            else retval = -3 ;// EndChar (',' or '\0') not found
        }
        else retval = -2 ;// separator not found
      }
      else retval = -1 ;// Param name string not found 
     }
     else retval = -1;
     
   }
   
   if (tempStr != NULL)
   {
     delete []tempStr;
   }


   strcpy( Dest , Default );
   return retval;             
}

short CLibIniFile::GetConfigParamString( const char * ConfigString, const char * ParamName, string * DestString, const char * DefaultString )
{
    char buffer[MAX_CONFIG_BUF];
    short retVal = 0;
    
    memset (buffer, 0x0, MAX_CONFIG_BUF);
    
    retVal = GetConfigParamString( ConfigString, ParamName, buffer, MAX_CONFIG_BUF-1, DefaultString);
    
    *DestString = buffer;
    
    return retVal;
}

///////////////////////////////////////////////////////////////////////////////
//
//GetConfigParamInt
//
//Parameters : const char *  Configuration string             (in)
//             const char *  Parameter Name                   (in)
//                   int  *  Destination integer              (out)
//             const int     Default value in case of failure (in)
//
//Return type :      short   Error
//
//Action : 
//   Gets the value of the provided Parameter in the Given Configuration 
//   string and copies it in the provided Destination integer. Return 0 if 
//   these operation occured fine. Otherwise, it uses the provided Default
//   value and copies it in Destination integer. In this case, it returns -1.
//   Every paramaters have to be specified, otherwise the function returns
//   -2 without setting the Destination integer.
//
//   The Configuration string format is:
//      Param1:Value1,Param2:Value2,Param3:Value3,...,ParamN:ValueN
//
///////////////////////////////////////////////////////////////////////////////
short CLibIniFile::GetConfigParamInt(const char* ConfigString,
                        const char* ParamName,
                              int*  Dest,
                        const int   Default )
{

   string temp;
   int retVal, retCode;

   if (GetConfigParamString(ConfigString,ParamName,&temp,"")<0)
   {
       retVal = Default;
       retCode = -1;
   }
   else{
       retVal = atoi (temp.c_str());
       retCode = 0;
   }

   if (Dest != NULL){
       *Dest = retVal;
   }

   return retCode;

}

///////////////////////////////////////////////////////////////////////////////
//
//GetConfigParamBool
//
//Parameters : const char *  Configuration string             (in)
//             const char *  Parameter Name                   (in)
//                   bool *  Destination bool                 (out)
//             const bool    Default value in case of failure (in)
//
//Return type :      short   Error
//
//Action : 
//   Gets the value of the provided Parameter in the Given Configuration 
//   string and copies it in the provided Destination bool. Return 0 if 
//   these operation occured fine. Otherwise, it uses the provided Default
//   value and copies it in Destination bool. In this case, it returns -1.
//   Every paramaters have to be specified, otherwise the function returns
//   -2 without setting the Destination bool.
//
//   The Configuration string format is:
//      Param1:Value1,Param2:Value2,Param3:Value3,...,ParamN:ValueN
//
///////////////////////////////////////////////////////////////////////////////
short CLibIniFile::GetConfigParamBool(const char* ConfigString,
                         const char* ParamName, 
                               bool* Dest,
                         const bool  Default )
{
    int Temp = 0;
   int Retval = GetConfigParamInt(ConfigString,
                              ParamName,
                              &Temp,
                              Default);

   if( Temp == 0 )
   {
      *Dest = false;
   }
   else
   {
      *Dest = true;
   }

   return Retval;
}

///////////////////////////////////////////////////////////////////////////////
//
//GetParamStringLength 
//
//Parameters :  const char* configString  - String containing the parameters  (in)
//              const char* paramName     - the parameter we are looking for  (in)
//                    int *stringLength   - Where to store the value          (out)
//              const int defaultLength   - Default value for string length   (in)
//
//Return type : int - 0     Operation succesful
//                  - -1    Problems in computing the length of the string
//
//Actions :
//   Searches for paramName inside configString and stores the length 
//   in stringLength.
//
///////////////////////////////////////////////////////////////////////////////
int CLibIniFile::GetParamStringLength (const char* configString, 
                          const char* paramName, 
                                int * stringLength, 
                          const int   defaultLength )
{
    //Return value
	int retval = 0 ;

    //Pointers used to compute the string length
    const char *p2 = NULL;

    //Searching for parameter
	const char *p1 = strstr( configString, paramName );

	if(p1 != NULL) 
    {

        //Parameter found, searching for the colon separator
        p1 = strchr( p1, ':' );

        if(p1 != NULL) 
        {
            p2 = strchr( p1, ',' );
        }
        else
        {
            //Semicolon not found!! 
            *stringLength = defaultLength;
            retval = -1;
        }

	    if(p2 != NULL) 
        {
		    //Computing stringLength, adding 1 to count for end of line
		    *stringLength = (p2 - p1) + 1;
            retval = 0;
	    }
        else
        { 
            //Comma not found, using default value
            *stringLength = defaultLength;
            retval = -1 ; 
        }
    }

	return retval ;

}

float CLibIniFile::GetConfigParamFloat( const char * ConfigString, const char * ParamName, float * Dest, const float Default )
{
    char tempBuffer[32];
    char defaultBuffer[32];
    float retVal = 0;
    
    sprintf(defaultBuffer, "%f", Default);
    
    GetConfigParamString(ConfigString,
                                   ParamName,
                                   tempBuffer,
                                   31,
                                   defaultBuffer);
    
    if (Dest != 0x0)
    {
        sscanf(tempBuffer, "%f", Dest);
        retVal = *Dest;
    }
    else
    {
        sscanf(tempBuffer, "%f", &retVal);
    }

    

    return retVal;
}

bool CLibIniFile::ExistsConfigParam(string configStr, string param)
{
    string::size_type paramIndex;

    paramIndex = configStr.find(param);

    return paramIndex != string::npos;
}

bool CLibIniFile::SetConfigParamString( string *configString, const char * paramName, const char * newVal )
{
    bool retVal = false;
    string::size_type paramIndex = 0, commaIndex = 0, colonIndex = 0;
    string tempString;
    
    //Search for the parameters
    paramIndex = configString->find(paramName);
    
    if (paramIndex != string::npos)
    {
        //Get the subfields indexs
        colonIndex = configString->find(":", paramIndex);
        commaIndex = configString->find(",", paramIndex);
        
        if (colonIndex != string::npos)
        {
            //Copy everything up to the parameter in temporary string
            tempString = configString->substr(0, colonIndex + 1);
            tempString += newVal;
        
            //Check if there is more to copy
            if (commaIndex != string::npos)
            {
                tempString += configString->substr(commaIndex);
            }
            
            *configString = tempString;
            retVal = true;
        }
    }
    
    return retVal;
        
}

bool CLibIniFile::AddConfigParamString(string *configString, const char * paramName, const char * newVal )
{
    if (configString->size() != 0)
    {
        *configString+=',';
    }
    
    *configString+=paramName;
    *configString+=':';
    *configString+=newVal;
    
    return true;
}




