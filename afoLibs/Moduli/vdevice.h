/***************************************************************************
 *   Copyright (C) 2005 by Alessandro Mirri   				                     *
 *   alessandro.mirri@newtohm.it                                           *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author	         *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY; 			               *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 ***************************************************************************/
#ifndef VDEVICE_H
#define VDEVICE_H

#include <string.h>
#include "cownet.h"
#include "LibIniFile.h"
// #include "timer.h"
#include "vafoobject.h"
#include "commonDefinitions.h"
#include "crcutil.h"
#include "ioutil.h"

/**
this class is an abstract common class for all the 1wire devices
in the constructor it reads from a configuraztion string the common parameters of every device

The parameter string has the following format:
Devicenn=NAME:name_of_device,FN:family_number,SN:serial_number,UpTime:Update_time (in seconds), HasPower:1[0], Address:


Rev 0.5 - 07/01/2006 -- Added ther CError object to handle errors
@author Alessandro Mirri
*/
class CVDevice : public CVAFOObject{
public:
    CVDevice(const char* configString, COWNET *master) : CVAFOObject( configString )
    {
        char SNBuffer[32];


        //Do some initialization
        m_HasExtPower = false;

        //Read from INI file various common parameters
        if (configString != NULL)
        {
            m_IniLib.GetConfigParamString(configString, "SN", SNBuffer, 32, "NotFound");
    //         GetConfigParamInt(configString, "TIMERID", &m_TimerID, -1);

            m_IniLib.GetConfigParamBool(configString,"VIRTUAL",&m_IsVirtual,false);


            if ( (strcasecmp(SNBuffer, "NotFound")) || (strcasecmp(SNBuffer, "NA")) )
            {
                SetSN(SNBuffer);
            }
            else
            {
                memset (m_SerialNum,0x0,9);
            }
        }
        else
        {
            memset (m_SerialNum,0x0,9);
        }

        m_LastMeasure = -100.0;
        m_Master = master;
  };

  virtual ~CVDevice(){};

  /**
   * Sets the serial number of the device
   * @param sn the new serial number
   */
  void SetSN(uchar *sn) {if (sn!=NULL) memcpy(m_SerialNum, sn, 8);memcpy(&m_FamilyNumber, &m_SerialNum[6],2);};
  void SetSN(const char *sn)  {if (sn!=NULL) ConvertSN2Hex(sn, m_SerialNum);memcpy(&m_FamilyNumber, &m_SerialNum[6],2);}

  /**
   * retrieves the serial number
   * @param sn variable to store the serial number : 8 char array
   */
  void GetSN(uchar *sn) {if ((sn!=NULL)&&(m_SerialNum != NULL)) memcpy(sn, m_SerialNum, 8);};
  uchar* GetSN() {if (m_SerialNum != NULL) return m_SerialNum; else return NULL;};

  /**
   * Sets the family number of the device
   * @param fn the family number
   */
  void SetFamNum(SMALLINT fn) {m_FamilyNumber = fn;};

  /**
   * Gets the family number
   * @param fn variable to store the family number
   */
  void GetFamNum(SMALLINT *fn) {*fn = m_FamilyNumber;};
  SMALLINT GetFamNum() {return m_FamilyNumber;};

  /**
   * Sets the external power state
   * @param newPower TRUE if the device has exyernal power
   */
  void SetExtPowerState(bool newPower) {m_HasExtPower = newPower;};

  /**
   * Gets the device external power
   * @return TRUE if the device has external power
   */
  bool GetExtPowerState(){ return m_HasExtPower;};

  /**
   * Sets the device port number
   * @param portNum the number of the port
   */
  void SetPortNum(int portNum) {m_PortNum = portNum;};

  /**
   * Gets the device's port number
   * @return the device's port number
   */
  int GetPortNum(){return m_PortNum;};


  /**
   * Function used to check if the device is connected to the NET or not
   * @return TRUE if device is connected, false otherwise
   */
  bool VerifyPresence() { m_Master->owSerialNum(m_PortNum,m_SerialNum, false); return m_Master->owVerify( m_PortNum, false);};

  void SendPresenceError() {(VerifyPresence())?PushError(OWERROR_CRC_FAILED, m_NetNumber, m_DeviceNumber):PushError(OWERROR_DEVICE_SELECT_FAIL, m_NetNumber, m_DeviceNumber);
  };

    void SetCRCUtil(CRCUtil* theValue)
    {
        m_CRCUtil = theValue;
    }

    void InvalidateData(){m_DriverData.isValid = false;};

    t_DataVal GetDriverData() const
    {
        return m_DriverData;
    }
    

  COWNET *m_Master;           //!Object used to access the one wire net
  int m_PortNum;

  protected:

    uchar m_SerialNum[9];       //!Serial Number of the device
    SMALLINT m_FamilyNumber;    //!Family number of the device
    
    bool m_HasExtPower;
    
    t_DataVal m_DriverData;

    //Holder for the measurement from the driver
    float m_LastMeasure;

    CRCUtil *m_CRCUtil;

    bool m_IsVirtual;

};

#endif
