/***************************************************************************
 *   Copyright (C) 2006 by Alessandro Mirri                                      *
 *   alessandro.mirri@newtohm.it                                           *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author            *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                         *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 ***************************************************************************/
#ifndef STDVAFOOBJECT_H
#define STDVAFOOBJECT_H

#include "commonDefinitions.h"
#include "timer.h"
#include "LibIniFile.h"
#include "afoerror.h"
#include "IniFileHandler.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>

//using namespace std;

/**
This class is a holder for the generic AFOObject which is undiffertiated between device or controller

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>objectNumber
*/
class CVAFOObject {
public:
    CVAFOObject(const char *configString)
    {
        CIniFileHandler iniFile;
        
        
        if (configString != 0x0)
        {
            m_IniLib.GetConfigParamString( configString, "COMMENT", &m_Comment, "NA");
            
            m_IniLib.GetConfigParamString(configString, "NAME",  &m_Name, "NA");

            m_ConfigString = configString;
        }
        
        if (iniFile.Load(CONFIG_FILE))
        {
            m_DebugLevel = iniFile.GetInt(Config_Strings[CONF_DODEBUG],"COMMON",0);
        }
        
        m_TimerID = -1;
        m_AfoErrorHandler = 0x0;
        
        m_DeviceNumber = -1;
        m_NetNumber = -1;
        
        m_DeviceType = DEV_NONE;
        
    };

    ~CVAFOObject(){};

    /**
     * Sets the number used in the configuration file to store them
     * @param devNumber device number in the configuration file
     * @param netNumber NET number in the configuration file
     */
    void SetReferenceNumbers (int devNumber, int netNumber) { m_DeviceNumber = devNumber; m_NetNumber = netNumber;};
    
    /**
     * Returns the device number in the ocnfiguration file
     * @return device number in configuration file
     */
    int GetConfigFileDevIndex() { return m_DeviceNumber;};
    
    /**
     * Allows to retrieve the number of the in the configuration file NET to which the object belongs
     * @return number of NET to which the object belongs
     */
    int GetConfigFileNetIndex() { return m_NetNumber;};
    /**
     * Allows to retrieve the comment associated with the object
     * @return the comment
     */
    CString GetComment () {return m_Comment;};
    void SetComment (CString newComment) {m_Comment = newComment;};
    /**
     * Returns the name used in the configuration file for he object
     * @return name of the object
     */
    CString GetName() { return m_Name;};
      /**
     * Sets the device name
     * @param devName new device name (max length 15bytes)
       */
    void SetName(char *objName) {m_Name = objName;};
    void SetName(CString objName) {m_Name = objName;};
  

    /**
     * Allows to check if the timer is to be used or not
     * @return TRUE if timer enabled
     */
    bool IsTimerUsed() {return m_UseTimer;};
   
    /**
     * Allows to associate the error handler object
     * @param err pointer to a CAfoErrorHandler object
     */
    void SetErrorHandler(CAfoErrorHandler *err) { m_AfoErrorHandler = err;};

    e_DeviceType GetDeviceType() const
    {
        return m_DeviceType;
    }

#ifndef SMALL_MEMORY_TARGET
    bool PushError(e_AFOErrors error, int net=0, int devindex=0){(m_AfoErrorHandler!=0x0)?m_AfoErrorHandler->PushError(error, m_NetNumber, m_DeviceNumber):cout<<"Errore!! "<< "NET:" << m_NetNumber << " device:"<<m_DeviceNumber << " :" << afoErrorsStrings[error] << endl;return true;};


    
#else
    bool PushError(e_AFOErrors error, int net = 0, int devindex = 0){(m_AfoErrorHandler!=0x0)?m_AfoErrorHandler->PushError(error, m_NetNumber, m_DeviceNumber):cout<<"Errore!! "<< "NET:" << m_NetNumber << " device:"<< m_DeviceNumber << " Codice:" << error << endl;return true;};
#endif
    
    
        void SetEngPtr(void *engPtr) {m_EnginePtr = engPtr;};
        void SetNetPtr(void *netPtr) {m_NetPtr = netPtr;};

    protected:
        CAfoErrorHandler *m_AfoErrorHandler;
        
        bool m_UseTimer;
        CTimer *m_Timer;
        int m_TimerID;
        
        //!Numbers used in the configuration file
        int m_DeviceNumber, m_NetNumber;

        //Puntatore all'oggetto engine principale per l'invio diretto da parte dei moduli dei messaggi
        void *m_EnginePtr;
        void *m_NetPtr;

        //!Comment associated with the object
        CString m_Comment;            
        
        //!Name of the object
        CString m_Name;
        
        CLibIniFile m_IniLib;

        unsigned int m_DebugLevel;

        CString m_ConfigString;
        
        e_DeviceType m_DeviceType;
        
};


#endif
