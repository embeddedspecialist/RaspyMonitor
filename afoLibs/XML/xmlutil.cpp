/***************************************************************************
 *   Copyright (C) 2005 by Alessandro Mirri   *
 *   alessandro.mirri@newtohm.it   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "xmlutil.h"

///////////////////////////////////////////////////
//             StandardConstructor
///////////////////////////////////////////////////
CXMLUtil::CXMLUtil()
{
    //Do some initialization
    m_InMessage.content = "";
    m_OutMessage ="";
}

///////////////////////////////////////////////////
//             StandardDestructor
///////////////////////////////////////////////////
CXMLUtil::~CXMLUtil()
{
}

///////////////////////////////////////////////////
//             CreateMessage
///////////////////////////////////////////////////
CString CXMLUtil::CreateMessage( CString type, int nArg, ... )
{
  e_DeviceType typeIndex;
  CString varArg;             //Variable argument String
  CString deviceType;
  va_list va;
  CString addrString;
  
  //Reset last outputted message
  m_OutMessage = "";
  
  //Start getting all the arguments needed
  if (nArg < 2) //FIXME da mettere a posto: non c'e'un controllo sul numero di argomenti per i diversi casi
  {
       //Wrong number of arguments, exit
      return m_OutMessage;
  }
  
  //Init the variable arguments
  va_start(va, nArg);

  if (type == "DEVICE")
  {
    //Start writing the common header
    //Get type of sensor
    deviceType=va_arg(va, char*);

    //Tryng to get the type of sensor
    typeIndex = GuessFamilyType(deviceType);
    
    varArg = va_arg(va, char*);
    addrString = " ADDRESS=\"" + varArg + "\"";
    
      
    switch (typeIndex)
    {
        case DEV_NONE: //No device found, break
        break;
        
        case DEV_TEMPCTRL:
        {
            CString typeOfMessage;
           
            typeOfMessage = va_arg(va, char*);
            
            if (typeOfMessage == "Temperature")
            {
                m_OutMessage = "<DEVICE TYPE=\"TempController\"";
                            //Insert address
                m_OutMessage += addrString;
        
                //Compose the message
                m_OutMessage+= " TEMP=\""; 
                
                varArg = va_arg(va, char*);
                m_OutMessage+= varArg + "\"";
                
                if (nArg == 5)
                {
                    //Compose the message
                    m_OutMessage+= " ALL=\"";
                    
                    varArg = va_arg(va, char*);
                    m_OutMessage+= varArg + "\"";
                }
            }
            else if (typeOfMessage == "AlarmLevels")
            {
                m_OutMessage = "<DEVICE TYPE=\"TempAlarmsSettings\"";
                            //Insert address
                m_OutMessage += addrString;
                
                m_OutMessage += " ALARMMIN=\"";
                varArg = va_arg(va, char*);
                m_OutMessage += varArg + "\"";
                
                m_OutMessage += " ALARMMAX=\"";
                varArg = va_arg(va, char*);
                m_OutMessage += varArg + "\"";
            }
            
            //Close Message
            m_OutMessage+=" />";
        };
        break;
        case DEV_DIDO:
        {
            m_OutMessage = "<DEVICE TYPE=\"DIDO\"";
            
            //Insert address
            m_OutMessage += addrString;
            
            //Get Status
            m_OutMessage += " STATE=\"";
            varArg = va_arg(va, char*);
            m_OutMessage += varArg + "\"";

            //Close Message
            m_OutMessage+=" />";
        }
        break;
        case DEV_PIDSIMPLE:
        {
            char buffer[8];
            float tempFloat = -100.0;
            bool tempBool = false;
            CString typeOfMessage;
            
            //TODO aggiungere il controllo argomenti!!
            typeOfMessage = va_arg(va, char*);
            
            if (typeOfMessage == "Settings")
            {
                m_OutMessage = "<DEVICE TYPE=\"PIDInfo\" ";
                
                //Insert address
                m_OutMessage += addrString;                
                
                memset (buffer, 0, 8);
                tempFloat = va_arg(va, double);
                sprintf(buffer, "%2.1f", tempFloat);
                m_OutMessage+=" KAPPA=\"";
                m_OutMessage+=buffer;
                m_OutMessage+="\"";
                
                memset (buffer, 0, 8);
                tempFloat = va_arg(va, double);
                sprintf(buffer, "%2.1f", tempFloat);
                m_OutMessage+=" TINT=\"";
                m_OutMessage+=buffer;
                m_OutMessage+="\"";
                
                memset (buffer, 0, 8);
                tempFloat = va_arg(va, double);
                sprintf(buffer, "%2.1f", tempFloat);
                m_OutMessage+=" TDER=\"";
                m_OutMessage+=buffer;
                m_OutMessage+="\"";
                
                memset (buffer, 0, 8);
                tempFloat = va_arg(va, double);
                sprintf(buffer, "%2.1f", tempFloat);
                m_OutMessage+=" SETPOINT=\"";
                m_OutMessage+=buffer;
                m_OutMessage+="\"";
                
                tempFloat = va_arg(va, double);
                sprintf(buffer,"%2.1f", tempFloat);
                m_OutMessage+=" TEMP=\"";
                m_OutMessage+=tempFloat;
                m_OutMessage+="\"";
                
                tempBool = (bool)va_arg(va, int);
                if (tempBool)
                {
                    m_OutMessage+=" SUMMER=\"1\"";
                }
                else
                {
                    m_OutMessage+=" SUMMER=\"0\"";
                }
                
                //Close Message
                m_OutMessage+=" />";
            }
            else if (typeOfMessage == "LastPosition" )
            {
                m_OutMessage = "<DEVICE TYPE=\"PIDOutput\" ";
                
                //Insert address
                m_OutMessage += addrString;
                
                varArg = va_arg(va, char*);
                m_OutMessage+=" VAL=\"";
                m_OutMessage+=varArg;
                m_OutMessage+="\" ";
                
                //Close Message
                m_OutMessage+=" />";
            }
        }
        break;
        case DEV_PIDLMD:
        case DEV_UTACTRL:
        {
            char buffer[8];
            float tempFloat;
            bool tempBool;
            CString typeOfMessage;
            
            //TODO aggiungere il controllo argomenti!!
            typeOfMessage = va_arg(va, char*);
            
            if (typeOfMessage == "Settings")
            {
                m_OutMessage = "<DEVICE TYPE=\"PIDInfo\" ";
                
                //Insert address
                m_OutMessage += addrString;
                                
                memset (buffer, 0, 8);
                tempFloat = va_arg(va, double);
                sprintf(buffer, "%2.1f", tempFloat);
                
                m_OutMessage+=" KAPPA=\"";
                m_OutMessage+=buffer;
//                 m_OutMessage+=tempFloat;
                m_OutMessage+="\"";
                
                memset (buffer, 0, 8);
                tempFloat = va_arg(va, double);
                sprintf(buffer, "%2.1f", tempFloat);
                m_OutMessage+=" TINT=\"";
                m_OutMessage+=buffer;
//                 m_OutMessage+=tempFloat;
                m_OutMessage+="\"";
                
                memset (buffer, 0, 8);
                tempFloat = va_arg(va, double);
                sprintf(buffer, "%2.1f", tempFloat);
                m_OutMessage+=" TDER=\"";
                m_OutMessage+=buffer;
//                 m_OutMessage+=tempFloat;
                m_OutMessage+="\"";
                
                memset (buffer, 0, 8);
                tempFloat = va_arg(va, double);
                sprintf(buffer, "%2.1f", tempFloat);
                m_OutMessage+=" KAPPA2=\"";
                m_OutMessage+=buffer;
//                 m_OutMessage+=tempFloat;
                m_OutMessage+="\"";
                
                memset (buffer, 0, 8);
                tempFloat = va_arg(va, double);
                sprintf(buffer, "%2.1f", tempFloat);
                m_OutMessage+=" TINT2=\"";
                m_OutMessage+=buffer;
//                 m_OutMessage+=tempFloat;
                m_OutMessage+="\"";
                
                memset (buffer, 0, 8);
                tempFloat = va_arg(va, double);
                sprintf(buffer, "%2.1f", tempFloat);
                m_OutMessage+=" TDER2=\"";
                m_OutMessage+=buffer;
//                 m_OutMessage+=tempFloat;
                m_OutMessage+="\"";
                
                memset (buffer, 0, 8);
                tempFloat = va_arg(va, double);
                sprintf(buffer, "%2.1f", tempFloat);
                m_OutMessage+=" SETPOINT=\"";
                m_OutMessage+=buffer;
//                 m_OutMessage+=tempFloat;
                m_OutMessage+="\"";
                
                memset (buffer, 0, 8);
                tempFloat = va_arg(va, double);
                sprintf(buffer, "%2.1f", tempFloat);
                m_OutMessage+=" SETPOINTH=\"";
                m_OutMessage+=buffer;
//                 m_OutMessage+=tempFloat;
                m_OutMessage+="\"";
                
                memset (buffer, 0, 8);
                tempFloat = va_arg(va, double);
                sprintf(buffer, "%2.1f", tempFloat);
                m_OutMessage+=" SETPOINTL=\"";
                m_OutMessage+=buffer;
//                 m_OutMessage+=tempFloat;
                m_OutMessage+="\"";
                
                tempFloat = va_arg(va, double);
                sprintf(buffer,"%2.1f", tempFloat);
                m_OutMessage+=" TEMP=\"";
                m_OutMessage+=buffer;
//                 m_OutMessage+=tempFloat;
                m_OutMessage+="\"";
                
                tempBool = (bool)va_arg(va, int);
                if (tempBool)
                {
                    m_OutMessage+=" SUMMER=\"1\"";
                }
                else
                {
                    m_OutMessage+=" SUMMER=\"0\"";
                }
                
                //Close Message
                m_OutMessage+=" />";
            }
            else if (typeOfMessage == "LastPosition" )
            {
                m_OutMessage = "<DEVICE TYPE=\"PIDOutput\"";
                
                //Insert address
                m_OutMessage += addrString;

                varArg = va_arg(va, char*);
                m_OutMessage+=" VAL=\"";
                m_OutMessage+=varArg;
                m_OutMessage+="\" ";
                
               
                //Close Message
                m_OutMessage+=" />";
            }
            
        }
        break;
        case DEV_HUMIDITY:
        {
            char buffer[8];
            float tempVal; //Temporary value
            CString typeOfMessage;

            //Get type of message
            typeOfMessage = va_arg(va, char*);
            
            if (typeOfMessage == "humidity")
            {
                m_OutMessage = "<DEVICE TYPE=\"Humidity\" ";
                
                //Insert address
                m_OutMessage += addrString;
                
                //Get absolute humidity            
                tempVal = va_arg(va, double);
                sprintf(buffer, "%2.1f", tempVal);
                
                m_OutMessage += " ABSOLUTE=\"";
                m_OutMessage +=buffer;
                m_OutMessage +="\" ";
                
                //Get relative humidity
                tempVal = va_arg(va, double);
                sprintf(buffer, "%2.1f", tempVal);
                m_OutMessage += "RELATIVE=\"";
                m_OutMessage += buffer;
                m_OutMessage += "\" ";
                
                //Close message
                m_OutMessage+=" />";
            }
            else if (typeOfMessage == "settings")
            {
                //Get SetPoint
                tempVal = va_arg(va, double);
                sprintf(buffer, "%2.1f", tempVal);
                
                m_OutMessage += " SETPOINT=\"";
                m_OutMessage +=buffer;
                m_OutMessage +="\"";
                
                //Get if automatic control is active
                tempVal = va_arg(va, int);
                sprintf(buffer, "%d", (int)tempVal);
                m_OutMessage += " AUTO=\"";
                m_OutMessage += buffer;
                m_OutMessage += "\" ";
                
                //Get hysteresis
                tempVal = va_arg(va, double);
                sprintf(buffer, "%2.1f", tempVal);
                m_OutMessage += " HYST=\"";
                m_OutMessage += buffer;
                m_OutMessage += "\"";
                                
                //Close message
                m_OutMessage+=" />";
                  
            }
                
        }
        break;
        case DEV_AIAO:
        {
            m_OutMessage = "<DEVICE TYPE=\"AIAO\" ";
                
            //Insert address
            m_OutMessage += addrString;
            
            varArg = va_arg(va, char*);
            m_OutMessage +=" VAL=\"";
            m_OutMessage +=varArg;
            m_OutMessage +="\"";
            
            varArg = va_arg(va, char*);
            m_OutMessage +=" ISCURRENT=\"";
            m_OutMessage +=varArg;
            m_OutMessage +="\"";
            
            //Close message
            m_OutMessage+=" />";
            
        }
        break;
        case DEV_TEMPCTRLHYST:
        {
            m_OutMessage="<DEVICE TYPE=\"HystTempCtrlSettings\" ";
            
            //Insert address
            m_OutMessage += addrString;
            
            varArg = va_arg(va, char*);
            m_OutMessage+=" AUTO=\"";
            m_OutMessage+=varArg;
            m_OutMessage+="\"";
            
            varArg = va_arg(va, char*);
            m_OutMessage+=" SETPOINT=\"";
            m_OutMessage+=varArg;
            m_OutMessage+="\"";
            
            varArg = va_arg(va, char*);
            m_OutMessage+=" HYSTERESIS=\"";
            m_OutMessage+=varArg;
            m_OutMessage+="\"";
            
            //Close message
            m_OutMessage+=" />";
        };
        break;
        default: //Some error occurred
        {
            m_OutMessage = "";
        }
    }
  }
  else if (type == "STATUS")
  {
      //Get the subtype
      varArg = va_arg(va, char*);
      
      if (varArg == "NETSTATUS")
      {
        //Get net number
        m_OutMessage = "<STATUS NET=\"";
        varArg = va_arg(va, char*);
        m_OutMessage = m_OutMessage + varArg + "\" ";
        
        //Get Alarm State
        m_OutMessage = m_OutMessage + "ALARMSTATE=\"";
        varArg = va_arg(va, char*);
        m_OutMessage = m_OutMessage + varArg + "\" ";
                
        //Get ON time
        m_OutMessage = m_OutMessage + "ON=\"";
        varArg = va_arg(va, char*);
        m_OutMessage = m_OutMessage + varArg + "\" ";
        
        //Get Off time
        m_OutMessage = m_OutMessage + "OFF=\"";
        varArg = va_arg(va, char*);
        m_OutMessage = m_OutMessage + varArg + "\" ";
      }
      else if (varArg == "DATE")
      {
          int tempInt = 0;
          
          m_OutMessage = "<STATUS TYPE=\"DateAndTime\" DATE=\"";
          //Format of the date is MMDDhhmmYYYY
          //Month
          tempInt = va_arg(va, int);
          if (tempInt < 10)
          {
              m_OutMessage+="0";
          }
          m_OutMessage+=tempInt;
          
          //Day
          tempInt = va_arg(va, int);
          if (tempInt < 10)
          {
              m_OutMessage+="0";
          }              
          m_OutMessage+=tempInt;
        
          //Hour
          tempInt = va_arg(va, int);
          if (tempInt < 10)
          {
              m_OutMessage+="0";
          }
          m_OutMessage+=tempInt;
          
          //Minute
          tempInt = va_arg(va, int);
          if (tempInt < 10)
          {
              m_OutMessage+="0";
          }
          m_OutMessage+=tempInt;
          
          //Year
          tempInt = va_arg(va, int);
          m_OutMessage+=tempInt;
          
          m_OutMessage+="\" ";
      }
      
    //Close message
    m_OutMessage+=" />";
        
  }
  else if (type == "ERROR")
  {
      int tempInt;
      std::string errorTime;
      
      m_OutMessage = "<ERROR CODE=\"";
      tempInt = va_arg(va, int);
      m_OutMessage += tempInt;
#if !defined(SMALL_MEMORY_TARGET)      
      if ((tempInt > 0) && (tempInt < AFOERROR_NUM_TOT_ERRORS))
      {
        m_OutMessage +="\" DESCRIPTION=\"";
        m_OutMessage += afoErrorsStrings[tempInt];
      }
#endif     
      m_OutMessage +="\" NETORIGIN=\"";
      tempInt = va_arg(va, int);
      m_OutMessage += tempInt;
      m_OutMessage +="\" DEVICEORIGIN=\"";
      tempInt = va_arg(va, int);
      m_OutMessage += tempInt;
      m_OutMessage += "\" TIME=\"";
      tempInt = va_arg(va,int);
      //Convert time into string
      errorTime = ctime((time_t*)(&tempInt));
      
      //trim the last /n char
      errorTime.erase(errorTime.end()-1);
      
      m_OutMessage+=errorTime;
      m_OutMessage+="\"";
      
      //Close message
      m_OutMessage+=" />";
  }       
        
  //Close variable list
  va_end(va);
  return m_OutMessage;
 
}

///////////////////////////////////////////////////
//             GuessFamilyType
///////////////////////////////////////////////////
e_DeviceType CXMLUtil::GuessFamilyType( CString type )
{
  int index;
  
  for (index = DEV_NONE; index < DEV_NUMTOT; index = index + 1)
  {
    if (!strcasecmp(type.c_str(), Device_strings[index]))
    {
      //Device found
      break;
    }
  }
  
  return (e_DeviceType)index;
}

///////////////////////////////////////////////////
//             ParseXML
///////////////////////////////////////////////////
bool CXMLUtil::ParseXML(const char * buffer )
{
    CString bufferString;
    
    if (buffer != 0x0)
    {
        bufferString = buffer;
        return ParseXML( bufferString );
    }
    else
    {
        return false;
    }
 
  return true;
}

///////////////////////////////////////////////////
//             ParseXML
///////////////////////////////////////////////////
bool CXMLUtil::ParseXML( CString buffer )
{
    string::size_type idx1, idx2;
    
   
    //Empty internal buffers
    m_InMessage.command="";
    m_InMessage.content="";
    
    //Start by checking if it is a valid message
    idx1 = buffer.find("/>");
    idx2 = buffer.find("<");
    
    if ((idx1 == string::npos) || (idx2 == string::npos))
    {
        //Message not complete
        return false;
    }
    
    //start parsing
    idx1 = buffer.find("COMMAND=");  
    if (idx1 != string::npos)
    {
        idx1 += 9;
        idx2 = buffer.find("\"", idx1);
        if (idx2 != string::npos)
        {
            m_InMessage.command = buffer.substr(idx1, idx2-idx1);
        }
        else
        {
            //Bad field
            m_InMessage.command.empty();
            m_InMessage.content.empty();
            return false;
        }
    }
    else
    {
        idx1 = buffer.find("<COMMAND");
        
        if (idx1 == string::npos)
        {
            //Required field missing, aborting
            return false;
        }
        
        idx1 = buffer.find("TYPE");
        
        if (idx1 == string::npos)
        {
            //required field missing, aborting
            return false;
        }
        
        idx1 = idx1 + 6;
        idx2 = buffer.find("\"", idx1);
        
        if (idx2 == string::npos)
        {
            return false;
        }
        
        m_InMessage.command = buffer.substr(idx1, idx2-idx1);
    }
            
    
    //Getting all the rest
    idx1 = buffer.find("<");
    idx1 = idx1+7;
    idx2 = buffer.find("/>");
    if ( (idx2 != string::npos) && (idx1 < idx2) )
    {
        //remove the slash
        idx2 = idx2 - 1;
        
        m_InMessage.content = buffer.substr(idx1, idx2-idx1);
    }
    
    return true;
}

///////////////////////////////////////////////////
//             GetIntParam
///////////////////////////////////////////////////
bool CXMLUtil::GetIntParam( CString param, int * value )
{
    bool retVal = false;
    CString valueString = "";
    char buffer[255];

    memset (buffer,0,255);
    valueString = GetStringParam( param );
    
    if (!valueString.empty())
    {
        strcpy(buffer,valueString.c_str()); 
        sscanf (buffer, "%d", value);
        retVal = true;
    }
    
    return retVal;
}

///////////////////////////////////////////////////
//             GetIntParam
///////////////////////////////////////////////////
int CXMLUtil::GetIntParam( CString param )
{
    int retVal;
    
    if (GetIntParam( param, &retVal))
    {
        return retVal;
    }
    else
    {
        return (int)MINVAL;
    }
    
}

///////////////////////////////////////////////////
//             AcknowledgeCommand
///////////////////////////////////////////////////
CString CXMLUtil::AcknowledgeCommand( CString command, bool value )
{
    CString message="";
    
    //Start creating the message
    message = "<EXEC COMMAND=\""+command+"\" RESULT=\"";
    
    //Add the result value
    if (value)
    {
        message+="1\" ";
    }
    else
    {
        message+="0\" ";
    }
    
    //Close the message
    message+="/>";
    
    return message;
    
}

////////////////////////////////////////////////////////
//              GetStringParam
////////////////////////////////////////////////////////
bool CXMLUtil::GetStringParam( CString param, char * destString, int maxString )
{
    string::size_type index1, index2;
    CString tempBuffer, newParam;
    
    newParam=param;
    newParam+="=";
    memset (destString, 0x0, maxString);
    
    index1 = m_InMessage.content.find(newParam);
    
    if (index1 != string::npos)
    {
        int nOfChars = 0;
                   
        index2 = m_InMessage.content.find("=", index1);
        
        //Advance the pointer to get the value
        index1 = index2 + 2;
            
        //Search the terminating value char
        index2 = m_InMessage.content.find("\"", index1);
            
        nOfChars = index2 - index1;
            
        //Check the number of chars just in case...
        if (nOfChars <= 0)
        {
            return false;
        }
        else if (nOfChars > maxString - 1)
        {
            //Check length of string
            nOfChars = maxString -1;
        }
                    
        //Extract the substring
        tempBuffer = m_InMessage.content.substr(index1, nOfChars);
        
        //Copy it into the given array
        memcpy (destString, tempBuffer.c_str(), nOfChars);

        return true;
    }
    else
    {
        return false;
    }
    
}

////////////////////////////////////////////////////////
//              GetFloatParam
////////////////////////////////////////////////////////
float CXMLUtil::GetFloatParam( CString param )
{
    char valueString[32];
    float floatParam = 0.0;
    
    memset (valueString, 0x0, 16);
    if (GetStringParam( param, valueString, 16))
    {
        sscanf (valueString, "%f", &floatParam);
    }
    else
    {
        floatParam = MINVAL;
    }
    
    return floatParam;
}

CString CXMLUtil::GetStringParam( CString param )
{
    CString retVal="", newParam;
    string::size_type index1, index2;
    int nOfChars = 0;
    
    newParam = param;
    newParam+="=";
    index1 = m_InMessage.content.find(newParam);
    
    if (index1 != string::npos)
    {
                   
        index2 = m_InMessage.content.find("=", index1);
        
        //Advance the pointer to get the value
        index1 = index2 + 2;
            
        //Search the terminating value char
        index2 = m_InMessage.content.find("\"", index1);
            
        nOfChars = index2 - index1;
            
        //Check the number of chars just in case...
        if (nOfChars <= 0)
        {
            return retVal;
        }
                    
        //Extract the substring
        retVal = m_InMessage.content.substr(index1, nOfChars);

    }

    return retVal;
    
}
//////////////////////////////////////////////////////////////////
bool CXMLUtil::GetBoolParam( CString param )
{
    int retVal;
    
    retVal = (bool)GetIntParam( param );
    
    return  retVal;
}
//////////////////////////////////////////////////////////////////
bool CXMLUtil::GetFloatParam( CString param, float * value )
{
    bool retVal = false;
    CString valueString = "";
    char buffer[255];

    memset(buffer, 0, 255);
    valueString = GetStringParam( param );
    
    if (!valueString.empty())
    {
        strcpy(buffer,valueString.c_str());
        sscanf (buffer, "%f", value);
        retVal = true;
    }
    
    return retVal;
    
}
//////////////////////////////////////////////////////////////////
bool CXMLUtil::GetBoolParam( CString param, bool * val )
{
    return GetIntParam( param, (int*)val);
}

//////////////////////////////////////////////////////////////////
bool CXMLUtil::ExistsParam(CString param)
{
    CString value;

    value = GetStringParam(param);

    if (value.size() == 0)
    {
        return false;
    }

    return true;
}
