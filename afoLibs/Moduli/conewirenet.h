/***************************************************************************
 *   Copyright (C) 2005 by Alessandro Mirri                                *
 *   amirri@deis.unibo.it                                                  *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 ***************************************************************************/
#ifndef STDCONEWIRENET_H
#define STDCONEWIRENET_H

#include <string.h>
#include <iostream>
#include "xmlutil.h"
#include "t_net.h"
#include "ownet.h"
#include "vdevice.h"
#include "cownet.h"
#include "IniFileHandler.h"
#include "ds18X20.h"
#include "ds2405.h"
#include "ds2438.h"
#include "ds2408.h"
#include "ds2890.h"
#include "ds2751.h"
#include "digitalio.h"
#include "analogIO.h"
#include "commonDefinitions.h"
#include "afoerror.h"
#include "vpid.h"
#include "pidsimple.h"
#include "pidlmd.h"
#include "humcontroller.h"
#include "timer.h"
#include "temperaturecontroller.h"
#include "tempctrlhyst.h"
// #include "remotedido.h"
#include "pumpcontroller.h"
#include "nthvlv2-vav.h"
#include "ibuttonreader.h"
#include "fullutactrl2.h"
#include "Thu.h"

#ifndef USE_ADV_VLV
    #include "nthvlv.h"
#else
    #include "nthvlv-adv.h"
#endif
#include "buttoncontroller.h"
#include "stepdigitalout.h"
#include "vmultidido.h"
#include "tagcontrol.h"
#include "utactrl.h"
#include "di2ao.h"
#include "crcutil.h"
#include "nthmgc.h"
#include "nthacc.h"
#include "nthvlv2.h"
#include "digitalctrl.h"
#include "vcoordinator.h"
#include "c3pointctrl.h"

#include "fullutactrl.h"
#include "alarmcoordinator.h"
#include "changeovercoord.h"
#include "timemarkerctrl.h"
#include "climaticcurve.h"
#include "floorcoord2.h"
#include "BlockCoordinator.h"

#include "gio.h"
#include "unistd.h"
#include "sys/ioctl.h"
#include "fcntl.h"
#include "etraxgpio.h"
#include <float.h>
#include <vector>
#include "ClockManager.h"

extern ClockManager mainClockManager;

/**
This class is a wrapper of the original ownet API by Dallas semiconductoros to encapsulate the functions defined at session and network level
The configuration is done by reading an .ini type file, named config.ini.

@author Alessandro Mirri
*/
class COneWireNet{
public:
    COneWireNet(CString iniFileName, CTimer *timer);

    ~COneWireNet();

    /**
     * Attempt to acquire a 1-Wire net using a com port and a DS2480 based
     * adapter.
     * @param port number 0 to MAX_PORTNUM-1.  This number was provided to OpenCOM to indicate the port number.
     * @param portnum zero terminated port name.  For this platform use format COMX where X is the port number.
     * @return TRUE if NET acquired
     */
    bool AcquireNet(int netIndex, const char *port, int portnum=-1);
    bool AcquireNet(int netIndex);


    /**
     * Release the net previously acquired
     * @param netIndex Index of the net to release
     *@return TRUE if successful
     */
    bool ReleaseNet(int netIndex);

    /**
     * Function used to Release the ports of all the nets
     * @return TRUE if all the ports are correctly closed
     */
    bool ReleaseAllNets();

    /**
     * Allows to get the total number of nets acquired
     * @return the number of nets acquired
     */
    int GetTotalNumberOfNets() {return m_NetList.size();};

    /**
     * Returns the handler to the corresponding net
     * @param index index of the net to acquire
     * @return the handler of the net
     */
    int GetPortHandler(int index) {return m_NetList[index].portHandler;};

    /**
     * Returns the pointer to a given net in order to access its submembers
     * @param netIndex index of the net
     * @return the pointer to the T_Net object
     */
    T_Net* GetNetHandler(int netIndex) ;

    /**
     * Sends a Reset to a specified net
     * @param netIndex
     * @return TRUE if successful
     */
    bool SendReset(int netIndex);


    /**
     * Function used to add/create a new net in the netlist database
     * @return TRUE if successfull, FALSE otherwise
     */
    bool AddNet();

    /**
     * Removes the last net stored in m_NetList
     * @return TRUE if successful
     */
    bool RemoveLastNet();

    /**
     * Used to initialize one net from the inifile
     * @param  netIndex the index of the net to initialize
     * @param *timer the timer object
     * @return TRUE if init successfull
     */
    bool InitNet(int netIndex);

    /**
     * Initializes all the nets reading from the ini file
     * @return TRUE if init successfull
     */
    bool InitAllNets();

    /**
     * This function is used to issue an update command to the one wire devices to force the update of the internal values
     * @param netIndex The NET to which to issue the command
     * @param famNum   The Family Number of the devices, -1 means to everybody
     * @param command The type of the command. Could be : CONVERT_T, CONVERT_V
     * @return TRUE if command successful
     */
    bool UpdateAllTemp(int netIndex, unsigned char command);

    /**
     * Function to set the temperature alarm on one Device. It works with DS18B20, DS18S20 and DS2438 devices
     * @param netIndex the net
     *@param devIndex The index of the device
     * @param alarmLevel the temperature to which we would like to set the alarm
     * @return TRUE if successful
     */
    bool SetAlarmT(int netIndex, int devIndex, int maxAlarmLevel, int minAlarmLevel);

    bool GetAlarmT (int netIndex, int devIndex, int *maxAlarmLevel, int *minAlarmLevel);

    /**
     * Function to set all the temperature alarms on one net. It works with DS18B20, DS18S20 and DS2438 devices
     * @param netIndex The Net to write the alarm
     * @param alarmLevel the temperature to which we would like to set the alarm
     * @return TRUE if successful, FALSE otherwise
     */
    bool SetAllAlarmT(int netIndex, int maxAlarmLevel, int minAlarmLevel = -100);

    int GetAllAlarmT(int netIndex, int alarmArray[][3], int maxNumOfAlarms);

    bool InitTempControllers(int netIndex);



    /**
     * Gets the temperature of the given device. If updateFirst is setted the device is updated
     * @param netIndex Index of the net
     * @param devIndex Index of the device
     * @param updateFirst Update flag
     * @return The value retrieved from the device
     */
    float GetTemp(int netIndex, int devIndex, bool updateFirst=false);
    bool GetTemp(float *dest, int netIndex, int devIndex, bool updateFirst=false);

    /**
     * Gets All Temperatures in one net and writes them on a matrix. The first value is the index of the device
     * while the second one is the temperature.
     * @param netIndex the net index
     * @param upFirst Update first flag
     * @param dest the array where the values are stored
     * @param maxNum The maximum number of values to be stored
     * @return the number of devices read, -1 if an error occurred
     */
    int GetAllTemp(int netIndex, bool upFirst, float dest[][2], int maxNum);

    /**
     * Finds all the temperature sensors in alarm on one net and stores the device number in the given array
     * @param netIndex the net to search
     * @param family Yhe family of the temperature sensor, could be DS18B20_FN or DS18S20_FN
     * @param dest The destination array
     * @param maxNum The maximum number of values to be stored
     * @return the total number of devices found or -1 if an error occurred
     */
    int SearchAlarmTemp(int netIndex, SMALLINT family, int* dest, int maxNum);


    /**
     * Finds the position of the device starting from the serialNumber
     * @param netIndex The device net
     * @param serNum The serial Number used to search
     * @return the index value, -1 if  an error occurred or the device was not found
     */
    int GetIndexBySerial(int netIndex, uchar *serNum);
    int GetIndexBySerial( int netIndex, CString serNum );

    /**
     * Get the NET corresponding to the given ROM number
     * @param serial The ROM Code to search for
     * @return the NET index or -1 if an error occurred
     */
    int GetNetBySerial(uchar *serial);
    int GetNetBySerial(CString serial);

    /**
     * Function used to reprogram the XSTREAM XBEE device. Given the netIndex it reprograms the module
     * @param netIndex the NET containing the new address
     * @return TRUE if operation successfull
     */
    bool ChangeWLAddr(int netIndex);

    bool ChangeIPAddr(int netIndex);

    /**
     * Checks if a given net is wireless
     * @param netIndex index of the NET to check
     * @return TRUE if the net is wireless
     */
    bool IsNetWL(int netIndex) {return m_NetList[netIndex].isWl;};

    /**
     * Function used to set the "setup state" of the net: true means that the net has benn correctly initialized and all values stored in the sensors
     * @param netindex index of the net
     * @param setupState the setup state to write
     */
    void SetSetupState(int netindex, bool setupState);

    /**
     * Function used to get the setup state of the given net
     * @param netIndex net to check
     * @return true if the net has been correctly setupped
     */
    bool GetSetupState(int netIndex);

    /**
     * Allows to get the state of the alarms
     * @param netIndex Index of the net
     * @return true if alarms enalbled, false otherwise or if error the netindex is out of bounds
     */
    bool GetNetAlarmState(int netIndex);


    /**
     * Gets the last recorded temperatures from all the temp devices in one net
     * @param netIndex index of the Net
     * @param dest[][] destination matrix: column 0 is the sensor index and column 1 is the temperature
     * @param maxNum Maximum number of devices to get information
     * @return Number of temperatures retrieved
     */
    int GetAllLastTemp(int netIndex, float dest[][2], int maxNum);


    /**
     * Allows to get the index of the NET starting by the address of a device
     * @param memoryAddress address of the device
     * @return the index of the NET or -1
     */
    int GetNetByMemoryAddress( int memoryAddress );

    CVController* GetControllerHndlrByMemoryAddress(int memoryAddress);

    /**
     * This function allows to get the state of the given digital input
     * @param memoryAddress address in switch space of the device
     * @return 1 if Input is high, 0 if it is low, -1 if an error occurred
     */
    int GetDigitalInput(int memoryAddress);

    /**
     * This function gets the status of all digital inputs belonging to the same net. The return matrix has the following fields:
     * index of the device, state of the input
     * @param netIndex index of the net
     * @return Total Number of devices found
     */
    int UpdateDIDOs(int netIndex, int inputMatrix[][2], int remoteDidoMatrix[MAX_NUM_DEV_PER_NET] = 0x0, int *nOfRemoteDIDO = 0x0);

    /**
     * reads the registers from the ds2408 and stores them inside the controllers
     * @param netIndex 
     * @return number of ds2408 found and updated
     */
    int UpdateDIDOsRegisters ( int netIndex );

    /**
     * This function is used to get informations on DIDO activity in a particular NET.The return matrix has the following fields:
     * index of the device that has had activity, state of the input
     * @param portNumber Number of the master port to scan for activity
     * @param inputMatrix[][] matrix used to store the results
     * @return
     */
    int SearchDIDOActivity(int portNumber, int *remoteDIDOList, int maxNum, vector<CVDevice*>& ds2408ToClear);


    /**
     * Function used to clear the activity latches on one NET
     * @param netIndex the NET to which the ds2408 belongs to
     * @return true
     */
    bool ClearAllActivityLatches(int netIndex);

    /**
     * Sets the activity search for all the ds2408 in the given NET
     * @param netIndex net to which the ds2408 belong to
     * @return true
     */
    bool SetActivitySearch(int netIndex);

    /**
     * This function allows to set the given digital output. The argument is (usaully) in "straight logic": 0 means that the real device connected to the output will be turned off, 1 the opposite.
     * @param memoryAddress address in switch space of the device
     * @param turnOn if true the output will be in high state, if false in low state
     * @return TRUE if the switch has been correctly set, FALSE otherwise
     */
    bool SetDigitalOutput(int memoryAddress, bool turnOn);

    /**
     * This function allows to set all the digital outputs in the given NET with the given state
     * @param netIndex Index of the NET
     * @param newState State to set on the Outputs
     * @return TRUE if every switch has been set
     */
    bool SetAllDigitalOutputs(int netIndex, bool newState);

    /**
     * Allows to get if there has been activity on the given switch
     * @param memoryAddress address of the device in the device space
     * @return TRUE if operation successfull
     */
    bool GetActivityOnDigitalInput(int memoryAddress);

    /**
     * Allows to clear the activity flag in the given switch
     * @param memoryAddress address of the device in the device space
     * @return TRUE if operation succesfull
     */
    bool ClearActivityOnDigitalInput(int memoryAddress);

    /**
     * Forces the position of the switches defined as outputs to the startup one
     * @param netIndex index of the NET
     * @return TRUE if all switches have been correctly set
     */
    bool InitializeDOs(int netIndex);

    /**
     * Forces the position of all the AO's, it it is an AI just return true
     * @param netIndex index of the NET for which we would like to init the AIAOs
     * @return true if every AO has been correctly initialized
     */
    bool InitializeAOs( int netIndex );

    bool InitializePIDs( int netIndex );

    /**
     * Puts every valve controller in run mode
     * @param netIndex index of the NET to whihc the objects belong to
     * @return true if every valve controller has been initialized
     */
    bool InitializeAdvancedControllers (int netIndex);

    bool InitializeMGC(int netIndex, bool skipInit=false);


    bool InitializeCNT(int netIndex);

    bool InitializeACC(int netIndex);

    bool InitializeCoordinators (int netIndex);

    /**
     * This function allows to get the number of devices in the given NET
     * @param netIndex index of the NET
     * @return number of devices or -1 if an error occurred
     */
    int GetNofDevices(int netIndex);

    int GetNofControllers( int netIndex);

    bool SetPIDParameters (int address, float *parameters, int nOfParameters);

    bool SetSetpoint(int address, float *parameters, int nOfVals, CXMLUtil *xmlParser = 0x0);
    bool SetSetpoint(int netIndex, int devIndex, float *parameters, int nOfVals, CXMLUtil *xmlParser = 0x0);

    //07/11/2009 -- Aggiunto argomento XML per il parsing diretto del comando
    bool SetAllPIDSetpoint(int netIndex, float *setPoints, int nOfSetPoints, CXMLUtil* xmlParser = 0x0);

    bool SetAllPIDParameters( int netIndex, float *parameters, int nOfParameters );

    bool GetPIDSetup (int address, float *parameters, bool *isSummer, CString &type );

         /**
     * Finds the index of the device given its memory address and its family
     * @param netIndex Index of the NET to which the device belongs
     * @param address address of the device
     * @return the index of the device or -1 if an error occurs or the device was NOT found
          */
    int GetDeviceIndexByMemoryAddress(int netIndex, int address);

    /**
     * This function is used to change the state of the timer of the given NET
     * @param netIndex index of the NET
     * @param newState new state of the timer
     * @return TRUE if state changed, FALSE otherwise
     */
    bool ChangeNetTimerState (int netIndex, bool newState);

    bool ChangeNetTimerID (int netIndex, int newID);

    /**
     * Allows to set the summer variable in one PID or in all the PIDs of the system
     * @param isSummer new value to set
     * @param address address of the PID in PID space or -1 for every PID of the system
     * @return true if operation successfull
     */
    bool SetSummer(bool isSummer, int address);
    bool SetAllSummer(int netIndex, bool isSummer);

    /**
     * This function allows to read the relative humidity and the absolute humidity from the humidity controller device
     * @param netIndex index of the net
     * @param devIndex index of the device
     * @param absH return value, absolute humidity
     * @param relH retunr value, relative humidity
     * @return true if the readings were successfull
     */
    bool GetHumidity(int netIndex, int devIndex, float* absH, float *relH);
    /**
     * Get the absolute and relative humidity settings starting only from the romCode
     * @param romCode romCode of the device in string format, NOT HEX
     * @param absH return value, absolute humidity
     * @param relH return value, relative humidity
     * @return true if operation successfull
     */
    bool GetHumidity(int address, float *absH, float *relH);

    int GetAllHumidities (int netIndex, float dest[][3], int maxNum);
    /**
     * Get the settings (setPoint and automatic control) of the given humidity device
     * @param romCode rom code of the device in string format, NOT HEX
     * @param setPoint return value, set point used to control the device
     * @param autoControlState return value, the state of the automatic control (true enabled, false disabled)
     * @param hysteresis return value, the currently amount of hysteresis used
     * @return true if operation successfull
     */
    bool GetHumiditySettings(int address, float* setPoint, bool* autoControlState, float* hysteresis);

    bool SetHumAutoControl(int address, bool state);
    bool SetAllHumAutoControl(int netIndex, bool state);

    bool SetHumSetPoint(int address, float setPoint);
    bool SetAllHumSetPoint(int netIndex, float setPoint);

    bool SetHumidityHysteresis(int address, float newHysteresis);
    bool SetAllHumidityHysteresis(int netIndex, float newHysteresis);

    int GetAllAIAO(int netIndex, float posArray[][2], int maxNum);
    bool GetAnalogIO(int address, float* inputVal, bool *isCurrent);

    bool SetAnalogInput(int address, bool readCurrent);

    /**
    * Gestisce le chiavi per il controllo accessi.
    * @param addKey if true addkey if false remove key
    * @param snum serial number of the key
    * @param expDateSec Daqte of expiry of the key
    * @param isHotel if true the operation will be performed only ont the devices marked as hotel
    * @param channel indicates the channel for the access control device (0 = ch1, 1 = ch2, 2 = both channels)
    * @return true if operation successfull
    */
// TODO (amirrix#1#): Da modificare per gestire i due canali diversi del controllo accessi
    bool ManageAccKeys(bool addKey,CString name, uchar* snum, unsigned int expDateSec, int channel, bool sendToRemote = false);

        /**
     * Allows to set the value of a single potentiometer in the range 0-255
     * @param netindex index of the NET
     * @param devIndex Index of the device
     * @param newPosition new position to set the device
     * @return TRUE if the value was correctly set
         */
    bool SetAnalogOutput(int netIndex, int devIndex, uchar newPosition);
    bool SetAllAnalogOutput(int netIndex, uchar newPos );

    bool CheckControllerType( int netIndex, int ctrlIndex, e_DeviceType type );
    bool CheckNetIndex (unsigned int netIndex) {return ((netIndex >= 0) && (netIndex<m_NetList.size()) );};

    bool EnableNetTimer(int netIndex, bool timerState);
    bool EnableAllTimers(int netIndex, bool timerState);
    bool UseTimer (int netIndex, int ctrlIndex, bool timerState);

    bool SetNetTimerID(int netIndex, int timerID);
    bool SetAllTimerID(int netIndex, int timerID);
    bool SetTimerID(int netIndex, int ctrlIndex, int timerID);

    int GetControllerMemoryAddress(int netIndex, int ctrlIndex);

    bool IsNetAcquired (int netIndex);

    bool UpdateBlackList(int netIndex);
    bool UpdateAllBlackLists();

    void SetErrorHandler(CAfoErrorHandler *errHndlr) {m_AfoErrorHandler = errHndlr;m_Master->SetErrorHandler( errHndlr);};

    bool NetHasTempDevices(int netIndex);
    bool NetHasDIDOs(int netIndex);
    bool NetHasPIDs (int netIndex);
    bool NetHasHums (int netIndex);
    bool NetHasAIAOs (int netIndex);
    bool NetHasButtonDIDOs(int netIndex);

    /**
     * Function used to update all the controllers with hystersis in the given NET
     * @param controllerType type of controller to update
     * @param netIndex index of the NET to which the controller belongs
     * @param updateData Flag that indicates if the controller has to update its internal data before trying to perfomr the control action
     * @return TRUE if update SUCCESSFULL
     */
    bool UpdateAllHysteresisCtrl( e_DeviceType controllerType, int netIndex, bool updateData );

    bool UpdateIniFile(int netIndex, int devIndex, CString subKey, CString newVal);

    int GetCtrlIndexByConfigNumber(int netIndex, int configNumber);

    int GetDeviceIndexByConfigNumber(int netIndex, int configNumber);

#ifndef SMALL_MEMORY_TARGET
    bool PushError(e_AFOErrors error, int net, int devindex=-1){(m_AfoErrorHandler!=0x0)?m_AfoErrorHandler->PushError(error, net, devindex):cout<<"Errore!! "<< "NET:" << net << " device:"<<devindex << " :" << afoErrorsStrings[error] << endl;return true;};
#else
    bool PushError(e_AFOErrors error, int net, int devindex=-1){(m_AfoErrorHandler!=0x0)?m_AfoErrorHandler->PushError(error, net, devindex):cout<<"Errore!! "<< "NET:" << net << " device:"<< devindex << " Codice:" << error << endl;return true;};
#endif

    COWNET* GetMasterHandler() { return m_Master;};

    void SetInitialParameters (CIniFileHandler *handlr);

    bool ConnectCoordinators(int netIndex);
    /**
     * Ritorna l'handler al dispositivo o null se il dispositivo non esiste
     * @param netIndex Indice della NET
     * @param devIndex Indice del dispositivo come risulta nel file di configurazione
     * @return handler del dispositivo o 0x0 se non esiste
     */
    CVDevice* GetDeviceHndlrByConfigNumber(int netIndex, int configDevIndex);

    void SetEnginePtr ( void* theValue )
    {
        m_EnginePtr = theValue;
    }
    

    //The one and only master of the nets
    COWNET *m_Master;
    CRCUtil m_CRCUtil;

  private:

    //!Array containing the net ports acquired
    vector<T_Net> m_NetList;

    CString m_ErrorLogFileName;

    //!Last netAddress used by wireless module
    int m_LastAddress;

    CIniFileHandler  *m_IniFile;

    //!Main debug variable
    int m_DoDebug;

    //!The one and only timer
    CTimer *m_Timer;

    CLibIniFile m_IniLib;

    //!The one and only error handler
    CAfoErrorHandler *m_AfoErrorHandler;

    time_t m_LastBLRecoveryTime, m_BLRecoveryInterval;

    //!Specifies the maximum number of errors allowed per controller before entering in the blacklist
    int m_MAXNofErrors;
    
    void* m_EnginePtr;


    /**
     * Removes (deletes) all the devices from the given net
     * @param netIndex index of the net
     * @return the number of devices removed
     */
    int RemoveDevices(int netIndex);

    /**
     * Create and initializes all the devices in one NET
     * @param netIndex index of the NET
     * @return TRUE if all the devices have been created correctly
     */
    bool InitDevices(int netIndex);


    /**
     * Create one device object inside the given NET and puts it in the internal array
     * @param netIndex index of the NET
     * @param deviceNumber number of the device in the list
     * @param configString configuration string of the device
     * @return TRUE if operation successfull, FALSE if a problem occurred
     */
    bool CreateDevice(int netIndex, int objectNumber, const char *configString);

    /**
     * Checks the given indexes against, respectively, the number of nets and the number of devices
     * @param netIndex index to check
     * @param devindex Index of the device in the NET
     * @param isDevice boolean indicating if we are checking for a device or a controller
     * @return TRUE if the given indexes are inside the range of the created NETS and the given device is inside the created devices, FALSE otherwise
     */
    bool CheckIndexes(int netIndex, int devIndex = 0, bool isDevice = true);

    /**
     * Checks the family of a device
     * @param netIndex index of the NET to which the device belongs
     * @param devIndex index of the device in the NET
     * @param FN family number to check for
     * @return TRUE if the given device belongs to the given family
     */
    bool CheckDeviceFamily (int netIndex, int devIndex, SMALLINT FN);

    /**
     * Force the RC1280 radio module to enter the pogramming state
     * @param netIndex handler to an open NET to access the COM port
     * @return TRUE if the command to the RC module have been issued
     */
    bool RCEnterProgramMode(int netIndex);

    /**
     * This functions sets the variable reporting the alarm state of the temperature sensors
     * @param netIndex Index of the NET to which the sensors belongs
     * @param sensorList Array containing the index of the sensors in alarm state
     * @param numSensors the numbr of sensors in alarm state
     */
    void UpdateTempAlarmState(int netIndex, int *sensorList, int numSensors);

    void AddToBlackList(int netIndex, int devIndex);
    void RemoveFromBlackList(int netIndex, int devIndex);
    bool CheckAddressValidity(int addr2Check);
    bool ConnectControllers(int netIndex);

    

};



#endif
