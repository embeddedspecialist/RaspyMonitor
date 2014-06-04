/***************************************************************************
 *   Copyright (C) 2005 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it                                           *
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
#ifndef STDXMLUTIL_H
#define STDXMLUTIL_H

#include "commonDefinitions.h"
#include "SocketPort.h"
#include <string>
#include <iostream>
#include "afoerror.h"

//TODO da gestire con i vettori anche questo

 //using namespace std;
 
/**
This class is used to encode/parse XML messages used to control the ONE-Wire Net
Actually the class can store 1 message.

Every device is encapsulated in the following structure:\n
<device TYPE="DeviceFamily" ROM="RomCode" [variable arguments] > <\device>\n
Where:\n
TYPE is the device family name, valid values are:\n
  DS18B20\n
  ......\n

ROM is the unique laser coded ID number of the device\n

Family specific arguments are listed below:\n
-DS18B20\n
  VAL     is the temperature acquired from the sensor\n
  ALARM   is the alarm level to be set or to be read\n

@author Alessandro Mirri
 * Rev. 0.7 - 11/3/2006
 * Aggiunta la creazione del messaggio per dispositivi PIDSimple
 * Aggiunta la funzione GetFloatParam per estrarre parametri di tipo float
 * 
 * Rev. 0.6 - 01/03/2006
 * Cambiate le funzioni per recuperare i parametri: ora la stringa da cui estrarre i valori e'implicitamente
 * il contenuto del messaggio ricevuto

 * Rev 0.5 - 20/02/2006
 * Aggiunta gestione di tipi diversi di messaggi e di dispositivi.
 * 
 * Rev 0.1 - 18/12/2005
 * Aggiunto controllo errori sui puntatori nella funzione ParseXML
*/
class CXMLUtil{
public:
    CXMLUtil();

    ~CXMLUtil();
    
    /**
     * Function used to create and compose the XML message, hopefully it will recognize
     * the device type and will format accordingly the message. Arguments are at least:
     * DEVICE
     *      string type of sensor
     *      string ROM code
     *      int flag indicating if we have to include the alarms
     *      string alarmStatus and/or temp value
     * STATUS
     *      string number of the NET
     *      string status of the NET
     * @param type type of message, DEVICE for the devices, STATUS for status message...
     * @param nArg Number of arguments to follow
     * @return The composed message or an empty string if an error was encountered
     */
    CString CreateMessage(CString type, int nArg, ...);

    bool ExistsParam(CString param);
    

    /**
     * Retrieves the content of the last received message
     * @return content of the struct Msg
     */
    CString GetContent(){return m_InMessage.content;};
    
    
    /**
     * Retrieves the last sent message
     * @return the last sent message
     */
    CString GetOutMessage(){return m_OutMessage;};
    
    /**
     * Retrieves the last command
     * @return the last received command
     */
    CString GetCommand(){return m_InMessage.command;};
    
    /**
     * Parses the input string searching for device dependent messages. Stores internally the values
     * @param buffer The input string
     * @return true if the message has been correctly parsed
     */
    bool ParseXML(const char *buffer);
    bool ParseXML(CString buffer);
    
    /**
     * retrieves the int value corresponding to the given param in the la loaded XML string
     * @param param the parameter
     * @param value the stored value
     * @return TRUE if parameter was found, FALSE otherwise
     */
    bool GetIntParam( CString param, int *value);    
    int GetIntParam( CString param);
    
    float GetFloatParam(CString param);
    bool GetFloatParam( CString param, float *value);

    //bool GetStringParam (CString param, CString& destString);
    bool GetStringParam (CString param, char *destString, int maxString);
    CString GetStringParam(CString param);
    
    bool GetBoolParam (CString param);
    bool GetBoolParam(CString param, bool* val);
    
    /**
     * Function used to compose the standard command execution report
     * @param command The command acknowledged
     * @param value the result of the execution
     * @return the message created
     */
    CString AcknowledgeCommand(CString command, bool value);
    
  
  private:    
    t_Msg m_InMessage;    //LastMessage received
    CString m_OutMessage;   //Message to send
    
    CString m_ConfigString;
    
    /**
     * This function retrieves and index to the family type argument to allow
     * the class to compose the right message
     * @param type 
     * @return 
     */
    e_DeviceType GuessFamilyType(CString type);

};


#endif
