/***************************************************************************
 *   Copyright (C) 2006 by Alessandro Mirri   *
 *   alessandro.mirri@newtohm.it   *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 *************************************************************************/
#ifndef STDAFOERROR_H
#define STDAFOERROR_H

#include <vector>
#include <iostream>
#include "cstring.h"
#include "commonDefinitions.h"


#define MAX_NOF_AFOERRORS 10

typedef enum AFOErrors
{
    OWERROR_NO_ERROR_SET                    ,
    OWERROR_NO_DEVICES_ON_NET               ,
    OWERROR_RESET_FAILED                    ,
    OWERROR_SEARCH_ERROR                    ,
    OWERROR_ACCESS_FAILED                   ,
    OWERROR_DS2480_NOT_DETECTED             ,
    OWERROR_DS2480_WRONG_BAUD               ,
    OWERROR_DS2480_BAD_RESPONSE             ,
    OWERROR_OPENCOM_FAILED                  ,
    OWERROR_WRITECOM_FAILED                 ,
    OWERROR_READCOM_FAILED                  ,
    OWERROR_BLOCK_TOO_BIG                   ,
    OWERROR_BLOCK_FAILED                    ,
    OWERROR_PROGRAM_PULSE_FAILED            ,
    OWERROR_PROGRAM_BYTE_FAILED             ,
    OWERROR_WRITE_BYTE_FAILED               ,
    OWERROR_READ_BYTE_FAILED                ,
    OWERROR_WRITE_VERIFY_FAILED             ,
    OWERROR_READ_VERIFY_FAILED              ,
    OWERROR_WRITE_SCRATCHPAD_FAILED         ,
    OWERROR_COPY_SCRATCHPAD_FAILED          ,
    OWERROR_INCORRECT_CRC_LENGTH            ,
    OWERROR_CRC_FAILED                      ,
    OWERROR_GET_SYSTEM_RESOURCE_FAILED      ,
    OWERROR_SYSTEM_RESOURCE_INIT_FAILED     ,
    OWERROR_DATA_TOO_LONG                   ,
    OWERROR_READ_OUT_OF_RANGE               ,
    OWERROR_WRITE_OUT_OF_RANGE              ,
    OWERROR_DEVICE_SELECT_FAIL              ,
    OWERROR_READ_SCRATCHPAD_VERIFY          ,
    OWERROR_COPY_SCRATCHPAD_NOT_FOUND       ,
    OWERROR_ERASE_SCRATCHPAD_NOT_FOUND      ,
    OWERROR_ADDRESS_READ_BACK_FAILED        ,
    OWERROR_EXTRA_INFO_NOT_SUPPORTED        ,
    OWERROR_PG_PACKET_WITHOUT_EXTRA         ,
    OWERROR_PACKET_LENGTH_EXCEEDS_PAGE      ,
    OWERROR_INVALID_PACKET_LENGTH           ,
    OWERROR_NO_PROGRAM_PULSE                ,
    OWERROR_READ_ONLY                       ,
    OWERROR_NOT_GENERAL_PURPOSE             ,
    OWERROR_READ_BACK_INCORRECT             ,
    OWERROR_INVALID_PAGE_NUMBER             ,
    OWERROR_CRC_NOT_SUPPORTED               ,
    OWERROR_CRC_EXTRA_INFO_NOT_SUPPORTED    ,
    OWERROR_READ_BACK_NOT_VALID             ,
    OWERROR_COULD_NOT_LOCK_REDIRECT         ,
    OWERROR_READ_STATUS_NOT_COMPLETE        ,
    OWERROR_PAGE_REDIRECTION_NOT_SUPPORTED  ,
    OWERROR_LOCK_REDIRECTION_NOT_SUPPORTED  ,
    OWERROR_READBACK_EPROM_FAILED           ,
    OWERROR_PAGE_LOCKED                     ,
    OWERROR_LOCKING_REDIRECTED_PAGE_AGAIN   ,
    OWERROR_REDIRECTED_PAGE                 ,
    OWERROR_PAGE_ALREADY_LOCKED             ,
    OWERROR_WRITE_PROTECTED                 ,
    OWERROR_NONMATCHING_MAC                 ,
    OWERROR_WRITE_PROTECT                   ,
    OWERROR_WRITE_PROTECT_SECRET            ,
    OWERROR_COMPUTE_NEXT_SECRET             ,
    OWERROR_LOAD_FIRST_SECRET               ,
    OWERROR_POWER_NOT_AVAILABLE             ,
    OWERROR_XBAD_FILENAME                   ,
    OWERROR_XUNABLE_TO_CREATE_DIR           ,
    OWERROR_REPEAT_FILE                     ,
    OWERROR_DIRECTORY_NOT_EMPTY             ,
    OWERROR_WRONG_TYPE                      ,
    OWERROR_BUFFER_TOO_SMALL                ,
    OWERROR_NOT_WRITE_ONCE                  ,
    OWERROR_FILE_NOT_FOUND                  ,
    OWERROR_OUT_OF_SPACE                    ,
    OWERROR_TOO_LARGE_BITNUM                ,
    OWERROR_NO_PROGRAM_JOB                  ,
    OWERROR_FUNC_NOT_SUP                    ,
    OWERROR_HANDLE_NOT_USED                 ,
    OWERROR_FILE_WRITE_ONLY                 ,
    OWERROR_HANDLE_NOT_AVAIL                ,
    OWERROR_INVALID_DIRECTORY               ,
    OWERROR_HANDLE_NOT_EXIST                ,
    OWERROR_NONMATCHING_SNUM                ,
    OWERROR_NON_PROGRAM_PARTS               ,
    OWERROR_PROGRAM_WRITE_PROTECT           ,
    OWERROR_FILE_READ_ERR                   ,
    OWERROR_ADDFILE_TERMINATED              ,
    OWERROR_READ_MEMORY_PAGE_FAILED         ,
    OWERROR_MATCH_SCRATCHPAD_FAILED         ,
    OWERROR_ERASE_SCRATCHPAD_FAILED         ,
    OWERROR_READ_SCRATCHPAD_FAILED          ,
    OWERROR_SHA_FUNCTION_FAILED             ,
    OWERROR_NO_COMPLETION_BYTE              ,
    OWERROR_WRITE_DATA_PAGE_FAILED          ,
    OWERROR_COPY_SECRET_FAILED              ,
    OWERROR_BIND_SECRET_FAILED              ,
    OWERROR_INSTALL_SECRET_FAILED           ,
    OWERROR_VERIFY_SIG_FAILED               ,
    OWERROR_SIGN_SERVICE_DATA_FAILED        ,
    OWERROR_VERIFY_AUTH_RESPONSE_FAILED     ,
    OWERROR_ANSWER_CHALLENGE_FAILED         ,
    OWERROR_CREATE_CHALLENGE_FAILED         ,
    OWERROR_BAD_SERVICE_DATA                ,
    OWERROR_SERVICE_DATA_NOT_UPDATED        ,
    OWERROR_CATASTROPHIC_SERVICE_FAILURE    ,
    OWERROR_LOAD_FIRST_SECRET_FAILED        ,
    OWERROR_MATCH_SERVICE_SIGNATURE_FAILED  ,
    OWERROR_KEY_OUT_OF_RANGE                ,
    OWERROR_BLOCK_ID_OUT_OF_RANGE           ,
    OWERROR_PASSWORDS_ENABLED               ,
    OWERROR_PASSWORD_INVALID                ,
    OWERROR_NO_READ_ONLY_PASSWORD           ,
    OWERROR_NO_READ_WRITE_PASSWORD          ,
    OWERROR_OW_SHORTED                      ,
    OWERROR_ADAPTER_ERROR                   ,
    OWERROR_EOP_COPY_SCRATCHPAD_FAILED      ,
    OWERROR_EOP_WRITE_SCRATCHPAD_FAILED     ,
    OWERROR_HYGRO_STOP_MISSION_UNNECESSARY  ,
    OWERROR_HYGRO_STOP_MISSION_ERROR        ,
    OWERROR_PORTNUM_ERROR                   ,
    OWERROR_LEVEL_FAILED            ,       //116

    AFOERROR_UNABLE_TO_ACQUIRE_NET,
    AFOERROR_UNABLE_TO_SETUP_NET,
    AFOERROR_UNABLE_TO_OPEN_LOGFILE,
    AFOERROR_DS2890_UNABLE_TO_SETPOS,
    AFOERROR_DS2890_UNABLE_TO_SEND_RELCODE,
    AFOERROR_DS2890_UNABLE_TO_WRITE_CTRLREG,
    AFOERROR_DS2890_UNABLE_TO_READ_CTRLREG,
    AFOERROR_DS2890_UNABLE_TO_READPOS,
    AFOERROR_DS2890_UNABLE_TO_INCREMENT_POS,
    AFOERROR_DS2890_UNABLE_TO_DECREMENT_POS,
    AFOERROR_DS2438_UNABLE_TO_READ_TEMP,
    AFOERROR_DS2438_UNABLE_TO_READ_VOLTAGE,
    AFOERROR_DS2438_UNABLE_TO_READ_VDD,
    AFOERROR_DS2438_UNABLE_TO_SET_VOLTAGE_MEASUREMENT,
    AFOERROR_DS2438_UNABLE_TO_READ_TIMER,
    AFOERROR_DS2438_UNABLE_TO_UPDATE_TEMP,
    AFOERROR_DS2438_UNABLE_TO_READ_CURRENT,
    AFOERROR_DS2438_UNABLE_TO_SET_CURRENT_MEASUREMENT,
    AFOERROR_DS18X20_UNABLE_TO_READ_TEMP,
    AFOERROR_DS18X20_UNABLE_TO_UPDATE_TEMP,
    AFOERROR_DS18X20_UNABLE_TO_SET_ALARMS,
    AFOERROR_DS2408_UNABLE_TO_SETSWITCH,
    AFOERROR_DS2408_UNABLE_TO_READSWITCH,
    AFOERROR_DS2408_UNABLE_TO_SET_REGISTER,
    AFOERROR_DS2408_UNABLE_TO_READ_REGISTER,
    AFOERROR_DS2408_UNABLE_TO_CLEAR_LATCHES,
    AFOERROR_DIDO_STATE_DIFFERS_FROM_INTERNAL_STATE,
    AFOERROR_UNABLE_TO_SET_DIDO,
    AFOERROR_CONTROLLER_BLACKLISTED,
    AFOERROR_UNABLE_TO_UPDATE_ALL_TEMP,
    AFOERROR_UNABLE_TO_GET_TEMPS,
    AFOERROR_UNABLE_TO_GET_TEMPALARMS,
    AFOERROR_DS2751_UNABLE_TO_READ_MEM,
    AFOERROR_PID_UNABLE_TO_GETINPUT,
    AFOERROR_PID_UNABLE_TO_UPDATE,
    AFOERROR_PID_IO_NOT_VALID,
    AFOERROR_UNABLE_TO_UPDATE_DIDO_OUT,
    AFOERROR_TEMP_SENSORS_NOT_FOUND,
    AFOERROR_DIDO_NOT_FOUND,
    AFOERROR_PID_NOT_FOUND,
    AFOERROR_HUMS_NOT_FOUND,
    AFOERROR_AIAO_NOT_FOUND,
    AFOERROR_UNABLE_TO_START_PUMP1,
    AFOERROR_UNABLE_TO_START_PUMP2,
    AFOERROR_UNABLE_TO_START_BOTHPUMPS,
    AFOERROR_PUMP1_NOT_WORKING,
    AFOERROR_PUMP2_NOT_WORKING,
    AFOERROR_UNABLE_TO_STOP_PUMP1,
    AFOERROR_UNABLE_TO_STOP_PUMP2,
    AFOERROR_ALARM_ON_BOTH_PUMPS,
    AFOERROR_CTRLVLV_UNABLE_TO_EXEC_COMMAND,
    AFOERROR_CTRLVLV_UNABLE_TO_READ_SRAM,
    AFOERROR_CTRLVLV_UNABLE_TO_GET_SPECREG,
    AFOERROR_CTRLVLV_UNABLE_TO_SET_SPECREG,
    AFOERROR_CTRLVLV_UNABLE_TO_WRITE_SRAM,

    AFOERROR_UTACTRL_UNABLE_TO_CLOSE_SHUTTER,
    AFOERROR_UTACTRL_UNABLE_TO_OPEN_SHUTTER,

    AFOERROR_UTACTRL_UNABLE_TO_START_CLOSE_SHUTTER_MOTOR,
    AFOERROR_UTACTRL_UNABLE_TO_STOP_CLOSE_SHUTTER_MOTOR,
    AFOERROR_UTACTRL_UNABLE_TO_START_OPEN_SHUTTER_MOTOR,
    AFOERROR_UTACTRL_UNABLE_TO_STOP_OPEN_SHUTTER_MOTOR,

    AFOERROR_UTACTRL_UNABLE_TO_START_IN_FANS,
    AFOERROR_UTACTRL_UNABLE_TO_STOP_IN_FANS,
    AFOERROR_UTACTRL_UNABLE_TO_START_OUT_FANS,
    AFOERROR_UTACTRL_UNABLE_TO_STOP_OUT_FANS,
    AFOERROR_UTACTRL_UNABLE_TO_SET_HEATBATTERY,
    AFOERROR_UTACTRL_UNABLE_TO_SET_COLDBATTERY,

    AFOERROR_UNABLE_TO_REPROGRAM_RADIOMODEM,
    AFOERROR_UNABLE_TO_REPROGRAM_ETHERNET_CONV,

    AFOERROR_CTRLUPID2_DEVICE_NOT_PRESENT,
    AFOERROR_CTRLUPID2_DEVICE_NOT_RESPONDING,


    AFOERROR_NUM_TOT_ERRORS
} e_AFOErrors;

#ifndef SMALL_MEMORY_TARGET
static const char* afoErrorsStrings[] =
{
       /*000*/ "No Error Was Set",
   /*001*/ "No Devices found on 1-Wire Network",
   /*002*/ "1-Wire Net Reset Failed",
   /*003*/ "Search ROM Error: Couldn't locate next device on 1-Wire",
   /*004*/ "Access Failed: Could not select device",
   /*005*/ "DS2480B Adapter Not Detected",
   /*006*/ "DS2480B: Wrong Baud",
   /*007*/ "DS2480B: Bad Response",
   /*008*/ "Open COM Failed",
   /*009*/ "Write COM Failed",
   /*010*/ "Read COM Failed",
   /*011*/ "Data Block Too Large",
   /*012*/ "Block Transfer failed",
   /*013*/ "Program Pulse Failed",
   /*014*/ "Program Byte Failed",
   /*015*/ "Write Byte Failed",
   /*016*/ "Read Byte Failed",
   /*017*/ "Write Verify Failed",
   /*018*/ "Read Verify Failed",
   /*019*/ "Write Scratchpad Failed",
   /*020*/ "Copy Scratchpad Failed",
   /*021*/ "Incorrect CRC Length",
   /*022*/ "CRC Failed",
   /*023*/ "Failed to acquire a necessary system resource",
   /*024*/ "Failed to initialize system resource",
   /*025*/ "Data too long to fit on specified device.",
   /*026*/ "Read exceeds memory bank end.",
   /*027*/ "Write exceeds memory bank end.",
   /*028*/ "Device select failed",
   /*029*/ "Read Scratch Pad verify failed.",
   /*030*/ "Copy scratchpad complete not found",
   /*031*/ "Erase scratchpad complete not found",
   /*032*/ "Address read back from scrachpad was incorrect",
   /*033*/ "Read page with extra-info not supported by this memory bank",
   /*034*/ "Read page packet with extra-info not supported by this memory bank",
   /*035*/ "Length of packet requested exceeds page size",
   /*036*/ "Invalid length in packet",
   /*037*/ "Program pulse required but not available",
   /*038*/ "Trying to access a read-only memory bank",
   /*039*/ "Current bank is not general purpose memory",
   /*040*/ "Read back from write compare is incorrect, page may be locked",
   /*041*/ "Invalid page number for this memory bank",
   /*042*/ "Read page with CRC not supported by this memory bank",
   /*043*/ "Read page with CRC and extra-info not supported by this memory bank",
   /*044*/ "Read back from write incorrect, could not lock page",
   /*045*/ "Read back from write incorrect, could not lock redirect byte",
   /*046*/ "The read of the status was not completed.",
   /*047*/ "Page redirection not supported by this memory bank",
   /*048*/ "Lock Page redirection not supported by this memory bank",
   /*049*/ "Read back byte on EPROM programming did not match.",
   /*050*/ "Can not write to a page that is locked.",
   /*051*/ "Can not lock a redirected page that has already been locked.",
   /*052*/ "Trying to redirect a locked redirected page.",
   /*053*/ "Trying to lock a page that is already locked.",
   /*054*/ "Trying to write to a memory bank that is write protected.",
   /*055*/ "Error due to not matching MAC.",
   /*056*/ "Memory Bank is write protected.",
   /*057*/ "Secret is write protected, can not Load First Secret.",
   /*058*/ "Error in Reading Scratchpad after Computing Next Secret.",
   /*059*/ "Load Error from Loading First Secret.",
   /*060*/ "Power delivery required but not available",
   /*061*/ "Not a valid file name.",
   /*062*/ "Unable to Create a Directory in this part.",
   /*063*/ "That file already exists.",
   /*064*/ "The directory is not empty.",
   /*065*/ "The wrong type of part for this operation.",
   /*066*/ "The max len for this file is too small.",
   /*067*/ "This is not a write once bank.",
   /*068*/ "The file can not be found.",
   /*069*/ "There is not enough space availabe.",
   /*070*/ "There is not a page to match that bit in the bitmap.",
   /*071*/ "There are no jobs for EPROM parts.",
   /*072*/ "Function not supported to modify attributes.",
   /*073*/ "Handle is not in use.",
   /*074*/ "Tring to read a write only file.",
   /*075*/ "There is no handle available for use.",
   /*076*/ "The directory provided is an invalid directory.",
   /*077*/ "Handle does not exist.",
   /*078*/ "Serial Number did not match with current job.",
   /*079*/ "Can not program EPROM because a non-EPROM part on the network.",
   /*080*/ "Write protect redirection byte is set.",
   /*081*/ "There is an inappropriate directory length.",
   /*082*/ "The file has already been terminated.",
   /*083*/ "Failed to read memory page of iButton part.",
   /*084*/ "Failed to match scratchpad of iButton part.",
   /*085*/ "Failed to erase scratchpad of iButton part.",
   /*086*/ "Failed to read scratchpad of iButton part.",
   /*087*/ "Failed to execute SHA function on SHA iButton.",
   /*088*/ "SHA iButton did not return a status completion byte.",
   /*089*/ "Write data page failed.",
   /*090*/ "Copy secret into secret memory pages failed.",
   /*091*/ "Bind unique secret to iButton failed.",
   /*092*/ "Could not install secret into user token.",
   /*093*/ "Transaction Incomplete: signature did not match.",
   /*094*/ "Transaction Incomplete: could not sign service data.",
   /*095*/ "User token did not provide a valid authentication response.",
   /*096*/ "Failed to answer a challenge on the user token.",
   /*097*/ "Failed to create a challenge on the coprocessor.",
   /*098*/ "Transaction Incomplete: service data was not valid.",
   /*099*/ "Transaction Incomplete: service data was not updated.",
   /*100*/ "Unrecoverable, catastrophic service failure occured.",
   /*101*/ "Load First Secret from scratchpad data failed.",
   /*102*/ "Failed to match signature of user's service data.",
   /*103*/ "Subkey out of range for the DS1991.",
   /*104*/ "Block ID out of range for the DS1991",
   /*105*/ "Password is enabled",
   /*106*/ "Password is invalid",
   /*107*/ "This memory bank has no read only password",
   /*108*/ "This memory bank has no read/write password",
   /*109*/ "1-Wire is shorted",
   /*110*/ "Error communicating with 1-Wire adapter",
   /*111*/ "CopyScratchpad failed: Ending Offset must go to end of page",
   /*112*/ "WriteScratchpad failed: Ending Offset must go to end of page",
   /*113*/ "Mission can not be stopped while one is not in progress",
   /*114*/ "Error stopping the mission",
   /*115*/ "Port number is outside (0,MAX_PORTNUM) interval",
   /*116*/ "Level of the 1-Wire was not changed",

   /*117*/ "Unable to acquire NET",
   /*118*/ "Error during NET setup",
   /*119*/ "Unable to open LOG file",
           "DS2890: Error while setting potentiometer",
           "DS2890: Error sending release code",
           "DS2890: Unable to write control register",
           "DS2890: Unable to read control register",
           "DS2890: Unable to read potentiometer position",
           "DS2890: Unable to increase potentiometer",
           "DS2890: Unable to decrease potentiometere",
           "DS2438: Unable to read temperature",
           "DS2438: Unable to read voltage",
           "DS2438: Unable to read VDD",
           "DS2438: Unable to set voltage conversion",
           "DS2438: Unable to read internal timer",
           "DS2438: Unable to update temperature",
           "DS2438: Unable to read current",
           "DS2438: Unable to set current reading",
           "DS18X20: Unable to read temperature",
           "DS18X20: Unable to update temperature",
           "DS28X20: Unable to set alarms levels",
           "DS2408: Unable to set single channel",
           "DS2408: Unable to read single channel",
           "DS2408: Unable to set internal register",
           "DS2408: Unable to read internal register",
           "DS2408: Unable to clear activity register",
           "Warning: DIDO state differs from internal one",
           "Error while setting DIDO",
           "Warning!! Device removed from system",
           "Errir while updating NET temperatures",
           "Unable to read NET temperatures",
           "Error while searching for temperature alarms",
           "DS2751: Unable to read device memory",
           "PID: Unable to read input",
           "PID: Unable to update output",
           "PID: I/O devices not valid",
           "Error while updating DIDO output",
           "Warning: temperature sensors not found",
           "Warning: DIDOs not found",
           "Warning: PIDs not updated",
           "Warning: Humidity sensors not found",
           "Warning: Analog I/O not found",
           "PumpController: unable to start pump number 1",
           "PumpComtroller: unable to start pump number 2",
           "PumpController: unable to start both pumps",
           "PumpController: pump 1 off or alarmed",
           "PumpController: pump 2 off or alarmed",
           "PumpController: unable to stop pump 1",
           "PumpController: unable to stop pump 2",
           "PumpController: both pumps are in emergency",
           "ValveController: Impossibile eseguire il comando",
           "ValveController: Impossibile leggere la SRAM",
           "ValveController: Impossibile leggere registro speciale",
           "ValveController: Impossibile impostare registro speciale",
           "ValveController: Impossibile scrivere la SRAM",
           "UTAController: Impossibile chiudere le serrande",
           "UTAController: Impossibile aprire le serrande",
           "UTAController: Impossibile avviare motore di chiusura serrande",
           "UTAController: Impossibile fermare motore di chiusura serrande",
           "UTAController: Impossibile avviare motore di apertura serrande",
           "UTAController: Impossibile fermare motore di apertura serrande",
           "UTAController: Impossibile avviare le ventilanti di ripresa",
           "UTAController: Impossibile fermare le ventilanti di ripresa",
           "UTAController: Impossibile avviare le ventilanti di mandata",
           "UTAController: Impossibile fermare le ventilanti di mandata",
           "UTAController: Impossibile impostare valore su batteria caldo",
           "UTAController: Impossibile impostare valore su batteria freddo",
           "Impossibile riprogrammare il radiomodem",
           "Impossibile riprogrammare il convertitore seriale-ethernet",
           "UPID2: Dispositivo non presente",
           "UPID2: Il dispositivo non risponde ai comandi"

};
#endif

typedef struct Error
{
    e_AFOErrors errorType;
    int netIndex;
    int deviceIndex ;
    time_t errorTime;
} t_Error;

/**
This class is used to hold all the errors generated by the afomonitor program. Essentially it is a stack of errors used to output them


	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CAfoErrorHandler{
public:
    CAfoErrorHandler();

    ~CAfoErrorHandler();

    bool PushError(e_AFOErrors error, int netIndex, int devIndex = -1);

    bool HasErrors();
    int GetNofErrors();

    bool PopError(t_Error *retError);
    t_Error GetError(int errIndex);
    t_Error GetLastError();
    CString GetLastErrorString();

    void SetDebugMode(bool newDebugMode) { m_DoDebug = newDebugMode;};

    int GetLastErrorNum(){return GetLastError().errorType;};

    void EnableLog (bool enableLog){m_EnableLog = enableLog;};
    void SetMaxFileSize(int newMaxFileSize){ m_MaxErrorFileSize = newMaxFileSize;};

    bool GetErrorSuppression() { return m_SuppressErrors;};
    void SetErrorSuppression(bool enable) {m_SuppressErrors = enable;};


private:

    bool TrimFile(CString fileName, int bytesToTrim, int maxFileSize);

    vector<t_Error> m_ErrVector;

    bool m_DoDebug;
    bool IsErrorAlreadyPresent(t_Error error2Check);

    bool m_EnableLog;
    int m_MaxErrorFileSize;

    //Consente di sopprimere dall'esterno il lancio degli errori
    //Puo' servire per gestire il protocollo con le uPID2
    bool m_SuppressErrors;

};


#endif
