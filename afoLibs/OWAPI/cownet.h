/***************************************************************************
 *   Copyright (C) 2005 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it                                           *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 ***************************************************************************/
#ifndef STDCOWNET_H
#define STDCOWNET_H

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <termios.h>
#include <errno.h>
#include <sys/time.h>
#include <iostream>

#include "IniFileHandler.h"
#include "ownet.h"
#include "timeUtil.h"
#include "commonDefinitions.h"
#include "afoerror.h"
#include "crcutil.h"

#define NETDELAY 10       //Put to some time the defualt net delay due to wireless timings

using namespace std ;




/**
This class is used to acquire the one wire net passing through the ds2480 master functions

@author Alessandro Mirri

21/12/2005 - Cambiato il valore di ritorno delle funzioni owRelease e CloseCOM per intercettare eventuali chiusure sbagliate delle porte
*/
class COWNET{
public:
    COWNET();

    COWNET(CIniFileHandler* configFile, const char* section);

    ~COWNET();

    void SetNetDelay(int netDelay) { m_NetDelay = netDelay; };
    int GetNetDelay() {return m_NetDelay;};
    void SetWireless(int wl) {m_Wireless = wl;};
    int GetWireless() {return m_Wireless;};

    int GetNofPortsAcquired();

    SMALLINT DS2480Detect(int portnum);

    SMALLINT DS2480ChangeBaud(int portnum, uchar newbaud);

    // external One Wire functions defined in owsesu.c
    int      owAcquire(int portnum, char *port_zstr);
    int      owAcquireEx(char *port_zstr);
    int      owAcquireEx();
    int      owRelease(int portnum);

    //ownetu
    SMALLINT  owFirst(int portnum, SMALLINT do_reset, SMALLINT alarm_only);
    SMALLINT  owNext(int portnum, SMALLINT do_reset, SMALLINT alarm_only);
    void      owSerialNum(int portnum, uchar *serialnum_buf, SMALLINT do_read);
    void      owFamilySearchSetup(int portnum, SMALLINT search_family);
    void      owSkipFamily(int portnum);
    SMALLINT  owAccess(int portnum);
    SMALLINT  owVerify(int portnum, SMALLINT alarm_only);
    SMALLINT  owOverdriveAccess(int portnum);

    // external One Wire functions from link layer owllu.c
    SMALLINT owTouchReset(int portnum);
    SMALLINT owTouchBit(int portnum, SMALLINT sendbit);
    SMALLINT owTouchByte(int portnum, SMALLINT sendbyte);
    SMALLINT owWriteByte(int portnum, SMALLINT sendbyte);
    SMALLINT owReadByte(int portnum);
    SMALLINT owSpeed(int portnum, SMALLINT new_speed);
    SMALLINT owLevel(int portnum, SMALLINT new_level);
    SMALLINT owProgramPulse(int portnum);
    SMALLINT owWriteBytePower(int portnum, SMALLINT sendbyte);
    SMALLINT owReadBytePower(int portnum);
    SMALLINT owHasPowerDelivery(int portnum);
    SMALLINT owHasProgramPulse(int portnum);
    SMALLINT owHasOverDrive(int portnum);
    SMALLINT owReadBitPower(int portnum, SMALLINT applyPowerResponse);

    // external One Wire functions from transaction layer in owtrnu.c
    SMALLINT owBlock(int portnum, SMALLINT do_reset, uchar *tran_buf, SMALLINT tran_len);
    SMALLINT owReadPacketStd(int portnum, SMALLINT do_access, int start_page, uchar *read_buf);
    SMALLINT owWritePacketStd(int portnum, int start_page, uchar *write_buf,
                              SMALLINT write_len, SMALLINT is_eprom, SMALLINT crc_type);
    SMALLINT owProgramByte(int portnum, SMALLINT write_byte, int addr, SMALLINT write_cmd,
                           SMALLINT crc_type, SMALLINT do_access);


    /**
     * Find devices in the One Wire NET
     * @param portnum number 0 to MAX_PORTNUM-1.  This number is provided to indicate the symbolic port number.
     * @param FamilySN[][] an array of all the serial numbers with the matching family code
     * @param family_code the family code of the devices to search for on the 1-Wire Net
     * @param MAXDEVICES the maximum number of devices to look for with the family code passed.
     * @return TRUE(1)  success, device type found, FALSE(0) device not found
     */
    SMALLINT FindDevices(int portnum, uchar FamilySN[][8], SMALLINT family_code, int MAXDEVICES);

    //FIXME da cambiare il nome perchè non si capisce cosa faccia
    uchar* GetMasterSN(int portNum) {return &SerialNum[portNum][0];}

    //Functions used to give direct access to the port
    SMALLINT WriteDirect2Port(int portnum, int outlen, uchar *outbuf);
    int ReadPortDirect(int portnum, int inlen, uchar *inbuf);

    //FunctionsUsed to openCOM
    int OpenPortDirect(char *port) {return OpenCOMEx(port);};
// //     int OpenPortDirect() {return OpenCOMEx((char*)(m_PortName.c_str()));};
    int ClosePortDirect(int portnum) {return CloseCOM(portnum);};
    void SetBaudCOMDirect(int portnum, uchar new_baud) {SetBaudCOM(portnum, new_baud);};


    void SetErrorHandler(CAfoErrorHandler *errHandler) {m_AfoErrorHandler = errHandler; return;};
    void SetNetNumber(int netNumber){m_NetNumber = netNumber;};


    void SetCRCUtil(CRCUtil* theValue)
    {
        m_CRCUtil = theValue;
    }

#ifndef SMALL_MEMORY_TARGET
    bool PushError(e_AFOErrors error, int net, int devindex=-1){(m_AfoErrorHandler!=0x0)?m_AfoErrorHandler->PushError(error, net, devindex):cout<<"Errore!! "<< "NET:" << net << " device:"<< devindex << " :" << afoErrorsStrings[error] << endl;return true;};


#else
    bool PushError(e_AFOErrors error, int net, int devindex=-1){(m_AfoErrorHandler!=0x0)?m_AfoErrorHandler->PushError(error, net, devindex):cout<<"Errore!! "<< "NET:" << net << " device:"<< devindex << " Codice:" << error << endl;return true;};

#endif

    bool IsPortOpen(int portnum){ return fd[portnum];};

  private:

      /**
       * Function used to initialize all the strcutures and variables in the master
       */
      void InitMaster();

    //da ownetu
    SMALLINT bitacc(SMALLINT,SMALLINT,SMALLINT,uchar *);
    int LastDiscrepancy[MAX_PORTNUM];
    int LastFamilyDiscrepancy[MAX_PORTNUM];
    uchar LastDevice[MAX_PORTNUM];
    uchar SerialNum[MAX_PORTNUM][8];

    //owllu
    SMALLINT FAMILY_CODE_04_ALARM_TOUCHRESET_COMPLIANCE;
    int dodebug;

    // local varable flag, true if program voltage available
    SMALLINT ProgramAvailable[MAX_PORTNUM];

    //owtrnu
    SMALLINT Write_Scratchpad(int,uchar *,int,SMALLINT);
    SMALLINT Copy_Scratchpad(int,int,SMALLINT);

    //linuxlnk
    SMALLINT  OpenCOM(int portnum, char *port_zstr);
    int       OpenCOMEx(char *port_zstr);
    int       CloseCOM(int portnum);
    void      FlushCOM(int portnum);
    SMALLINT  WriteCOM(int portnum, int outlen, uchar *outbuf);
    int       ReadCOM(int portnum, int inlen, uchar *inbuf);
    void      BreakCOM(int portnum);
    void      SetBaudCOM(int portnum, uchar new_baud);
    void      SendEndPck(int portnum);

    // LinuxLNK global
    int fd[MAX_PORTNUM];
    SMALLINT fd_init;
    struct termios origterm;
    //03/06/2008 -- Array che contiene i nomi delle porte per trattare in maniera diversa quelle USB
    CString m_PortNames[MAX_PORTNUM];



    // global DS2480B state
    SMALLINT ULevel[MAX_PORTNUM]; // current DS2480B 1-Wire Net level
    SMALLINT UBaud[MAX_PORTNUM];  // current DS2480B baud rate
    SMALLINT UMode[MAX_PORTNUM];  // current DS2480B command or data mode state
    SMALLINT USpeed[MAX_PORTNUM]; // current DS2480B 1-Wire Net communication speed
    SMALLINT UVersion[MAX_PORTNUM]; // current DS2480B version

    //!Flag indicating wether the net is wireless
    bool m_Wireless;

    //!Delay to be used when the NET is wireless
    int m_NetDelay;

    //!Error stack
    CAfoErrorHandler *m_AfoErrorHandler;

    //!Net to which it belongs
    int m_NetNumber;

    CRCUtil *m_CRCUtil;

    uchar DecodeStringParameter(CString param, const char** paramArray);
//     uchar DecodeIntParameter(int param, int *paramArray);

    uchar m_SlewRate;
    uchar m_Write1LowTime;
    uchar m_SampleOffset;




};


#endif
