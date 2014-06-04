#ifndef STDVCONTROLLER_H
#define STDVCONTROLLER_H

#include "timer.h"
#include "commonDefinitions.h"
#include "vafoobject.h"
#include "IniFileHandler.h"
#include "LibIniFile.h"
#include "Cmd.h"
#include "xmlutil.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>

//using namespace std;
#define IS_CTRL(pCtrl) pCtrl!=0x0

/**
This is a generic virtual controller class to derive all the non device classes

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CVController : public CVAFOObject
{
    public:
        CVController(const char* configString) : CVAFOObject( configString )
        {  
            
            //Get Memory Address
            m_IniLib.GetConfigParamInt( configString, "ADDR", &m_Address, -1 );
            
            m_NofErrors = 0;

            m_IsActive = true;
            
            m_IsOn = true;

            //Di default e' vecchia revisione.
            m_CodeRevision = 0;

            //Di default il controllore invia il suo riisultato via rete
            m_SendNETMessages = true;

            m_ControllerType = DEV_NONE;

            m_IsCoordinator = false;
            
        };

        bool IsCoordinator(){return m_IsCoordinator;};

        //09/10/2009 -- nuova interfaccia per tutti i controllori
        bool IsDataValid(){return m_Data.isValid;};
        float GetData(){return m_Data.floatData[0];};
        t_DataVal GetDataStructure (){return m_Data;};

        //Serve per dare un punto unico in cui si può scrivere l'uscita
        virtual bool SetVal(float newVal) = 0;

        virtual ~CVController(){};
    
        virtual bool Update(bool updateData) = 0;
        
        virtual bool Update2(bool updateData) = 0;

        
        /**
         * Function used to connect the devices to the controller
         * @param  void pointer to main conewirenet object
         * @return true if connection successfull
         */
        virtual bool ConnectDevices(void* netPtr){return false;};

        virtual bool ExecCommand(CXMLUtil* xmlUtil){ return false;};
        
        /**
         * Function used to check for the presence of the input and output devices
         * @return 
         */
        virtual bool VerifyIOPresence() = 0;
        
        /**
         * Function that returns a command contaning the data normally sent by the controller
         * in order to force the sending of data from outside
         * @return A CString containing the message to send
         */
        virtual CString GetSpontaneousData(int lParam = 0) = 0;

         /**
         * Enable/disable the timer
         * @param newState if TRUE the timer will be enabled
         */
        void UseTimer(bool newState) {(m_TimerID > -1) ? m_UseTimer = newState:m_UseTimer=false;};
        /**
         * Allows to associate the object with a different timer
         * @param newID The new timer ID
         */
        void ChangeTimerID(int newID)
        {
            if (m_Timer != 0x0)
            {
                m_TimerID = newID;
            }
            else
            {
                m_TimerID = 0;
            };

            if (m_TimerID > 0)
            {
                m_UseTimer = true;
            }
            else {m_UseTimer = false;}
            SaveConfigParam("TIMERID", CString("")+m_TimerID);
        }

        /**
         * Allows to set the timer object if timers are to be used
         * @param timer pointer to a timer object
         */
        void SetTimer(CTimer *timer) { m_Timer = timer; };

        /**
         * Allows to get the ID of the timer connected to this object
         * @return ID of the timer connected, 0 if no timer are connected
         */
        unsigned int GetTimerID() { return m_TimerID;};
    
        int GetMemoryAddress() { return m_Address;};
        
        int GetValFromTimer() { return m_Timer->GetValue( m_TimerID, m_TypeOfTimerVal );};
        int GetValFromTimer(int timerId) { return m_Timer->GetValue( timerId, m_TypeOfTimerVal );};
        
        bool IsTimerEnabled() { return m_Timer->IsTimerEnabled(m_TimerID);};
        bool IsTimerEnabled(int timerId) { return m_Timer->IsTimerEnabled(timerId);};
        
        e_DeviceType GetControllerType(){ return m_ControllerType;};
        
        void AddError() { m_NofErrors++;};
        int GetNofErrors() { return m_NofErrors;};
        void ClearError() { m_NofErrors = 0;};

        void ActivateController(bool activate){m_IsActive = activate;};

        bool SaveConfigParam(CString name, CString value) {
            CIniFileHandler hIni;
            CString configStr, key, section;
            bool retVal = false;

            if (!hIni.Load(CONFIG_FILE))
            {
                return false;
            }

            key = "Device";
    
            if (m_DeviceNumber < 10)
            {
                key += "0";
            }

            key += m_DeviceNumber;
            section = CString("NET")+m_NetNumber;
    
            configStr = hIni.GetString(key,section,"");

            if (configStr.size() == 0)
            {
                //TODO generare errore
                return false;
            }

            if (m_IniLib.ExistsConfigParam(configStr, name))
            {
                retVal = m_IniLib.SetConfigParamString(&configStr, name.c_str(), value.c_str());

            }
            else
            {
                retVal = m_IniLib.AddConfigParamString(&configStr, name.c_str(), value.c_str());
            }

            if ((retVal) && hIni.SetValue(key, configStr,"",section))
            {
                return hIni.Save();
            }
            else
            {
                return false;
            }
        };


    

        void TurnOn(bool turnOn){m_IsOn = turnOn;}
        unsigned int GetCodeRevision() const {
            return m_CodeRevision;
        };

        unsigned int ParseCommand (CXMLUtil *xmlUtil)
        {
            unsigned int index;
            CString inCommand;

            //Get the command
            inCommand = xmlUtil->GetCommand();

            if (m_DebugLevel)
            {
                cout << "CONTROLLER NET:"<<m_NetNumber<<" INDICE:"<<m_DeviceNumber<<" Comando ricevuto: " << inCommand << " -- " << xmlUtil->GetContent() << endl;cout.flush();
            }

            for (index = COMM_NONE; index < COMM_NUMTOT; index = index +1)
            {
                if (!strcasecmp(commandStrings[index], inCommand.c_str()))
                {
                    break;
                }
            }

            return index;
        }

    bool sendNETMessages() const {
        return m_SendNETMessages;
    }

    void SetSendNETMessages(bool SendNETMessages) {
        this->m_SendNETMessages = SendNETMessages;
    }

        
       
    protected:
        //Questo rappresenta il dato principale gestito da questo controller
        t_DataVal m_Data;
        int m_NofErrors;

        //Quando il controllo e' disattivato NON influisce sulle sue uscite
        //!Flag che consente di disattivare il controller dall'esterno,
        bool m_IsActive;
        
        //Quando il controllo è spento le uscite sono forzate a OFF o 0 Volt se analogico
        //!Flag che consente di spegnere il controllo
        bool m_IsOn; 
        
        e_DeviceType m_ControllerType;
        
        //!Address of the device in the "Controller space"
        int m_Address;
        
        e_TimerValues m_TypeOfTimerVal;       //!Variable indicating which value from the preset ones has to be chosen from the timer
        
        //Indica se il controllore e' in grado autonomamente di eseguire i comandi (da revisione 1) o meno
        //andra' rimosso quando e se tutti i controllori risponderanno ai comandi
        unsigned int m_CodeRevision;

        //Indica se il risultato dell'elaborazione del controllo attraverso la Update2 deve essere inviato via rete.
        //Serve nel caso si vogliano riutilizzare alcuni controllori all'interno di altri  e non si vogliono generare i messsaggi
        //relativi
        //DEFAULT TRUE
        bool m_SendNETMessages;
        
        //Indica se il controllo è un coordinatore o meno
        bool m_IsCoordinator;
        
};



#endif
