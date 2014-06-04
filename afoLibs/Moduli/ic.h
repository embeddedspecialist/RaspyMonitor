///////////////////////////////////////////////////////////////////////////////
//! \class CIC
//!                  IC Module
//! The IC Module is used to connect the Fox Core with the remote GUI via socket
//! Port<br>
///////////////////////////////////////////////////////////////////////////////

/*
21/12/2005 - Modificata la funzione ReadCommands per eliminare eventuali caratteri \0 ricevuti
*/

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <string>
#include <vector>
#include "CMsgRecv.h"
#include "SocketPort.h"
#include "LibIniFile.h"
#include "CLabel.h"
#include "commonDefinitions.h"
#include "afoerror.h"


#if !defined(_IC_H_)
#define _IC_H_

//28/12/2009 -- aumentato a 100K il buffer della connessione
#define IC_BUFF_SIZE 102400
#define MAX_NOF_DATA_MSG 100


class CIC
{
    public:
///////////////////////////////////////////////////////////////////////////////
//
//! Constructor
//
//!\param void
//!
//!\return nothing
//!
//! <b>Actions :</b><br>
//!   Initialize. 
//!
///////////////////////////////////////////////////////////////////////////////
    CIC(const char* configString);
    CIC();
    
///////////////////////////////////////////////////////////////////////////////
//
//! Destructor
//!
//!\param void
//!
//!\return nothing
//!
//! <b>Actions :</b><br>
//!   Destroy the current module and delete dynamically assigned objects 
//!   within the current module.
//
///////////////////////////////////////////////////////////////////////////////
    ~CIC();
 
    //Standard Module functions
    
///////////////////////////////////////////////////////////////////////////////
//
//! Initialize
//
//!\param void
//!
//!\return  0 if an error occurred otherwise 1
//!
//! <b>Actions :</b><br>
//!   Assign m_ICPort member to a new instantiation of a CPort object.
//!   Allocate the I/O buffers.
//!   Set the m_RunFlag to 2 (ready);
//!
///////////////////////////////////////////////////////////////////////////////
    int Initialize(const char* configString = NULL);
    
///////////////////////////////////////////////////////////////////////////////
//
//! Startup
//
//!\param void
//!
//!\return 0 if an error occurred otherwise 1
//!
//! <b>Actions :</b><br>
//!   Connect the ports (CPort object) by calling its Connect function.
//!   Set m_RunFlag to 1 (running) and return true. 
//!   Otherwise, set m_RunFlag to 0 (stopped), and return false.
//!
///////////////////////////////////////////////////////////////////////////////
    int Startup() ;
    
///////////////////////////////////////////////////////////////////////////////
//
//!  Update
//
//!\param void
//!
//!\return 0 if an error occurred otherwise 1
//!
//! <b>Actions :</b><br>
//!  <br>
//!  Call ReadData() if the module is ready.
///////////////////////////////////////////////////////////////////////////////
    int Update()  ;
    
///////////////////////////////////////////////////////////////////////////////
//
//!  Terminate
//
//!\param void
//!
//!\return 0 if an error occurred otherwise 1
//!
//! <b>Actions :</b><br>
//!   Empty the command list and free its needed memory.
//
///////////////////////////////////////////////////////////////////////////////  
    int Terminate() ;

///////////////////////////////////////////////////////////////////////////////
//
//GetCommand
//!Get first waiting command
//
//!\param void
//!
//!\return  Pointer to the first command string or NULL if the table is empty
//!
//! <b>Actions :</b><br>
//!   Extracts from the CommandTable a command, if the table is empty it deallocates
//!   the memory. COneWireEngine should call this function until the table is empty
//!
///////////////////////////////////////////////////////////////////////////////
    char *GetCommand();
   
///////////////////////////////////////////////////////////////////////////////
//
//ReadCommands
//!Read commands from buffer
//
//!\param void
//!
//!\return the number of chars read
//!
//! <b>Actions :</b><br>
//!   Extracts from the buffer the commands and calls the ParseData to load
//!   them in the CommandTable
//!
///////////////////////////////////////////////////////////////////////////////
    int ReadCommands();

///////////////////////////////////////////////////////////////////////////////
//
//ReadCommands
//!Read commands from buffer
//
//!\param void
//!
//!\return the number of chars read
//!
//! <b>Actions :</b><br>
//!   Extracts from the buffer the commands and calls the ParseData to load
//!   them in the CommandTable
//!
///////////////////////////////////////////////////////////////////////////////
    int ReadCommands2();
///////////////////////////////////////////////////////////////////////////////
//
//CIC IsConnected
//!Function used to Get the status of the input port
//
//!
//!\return  true if the port is connected otherwise false
//!
//! <b>Actions :</b><br>
//!   Allows the manager to know if an interface is actually connected 
///////////////////////////////////////////////////////////////////////////////
            bool IsConnected(){if (m_ICPort!=NULL) return m_ICPort->IsConnected(); else return false;};

    /**
     * Writes the messages coming from the COneWireEngine
     * @param data The data to be written
     * @param datalen The length of data
     * @return TRUE if all data has been written, FALSE otherwise
     */
    bool OutputData(const char *data, int datalen);
    
    /**
     * Returns the number of commands in queue
     * @return numbers of commands pending
     */
    int GetNofCommands() { return m_NOfCommands - m_ActualCommandExtracted - 1;};
    
    bool WriteError( e_AFOErrors errorType, int netIndex, int objNumber );
    
    string GetPortAddress() { return m_ICPort->GetSocketAddress();};
    
///////////////////////////////////////////////////////////////////////////////
//
//ParseData
//!Search for commands
//
//!\param void
//!
//!\return    void
//!
//! <b>Actions :</b><br>
//!   Parses the commandBuffer to look for commands aariving from interface.
///////////////////////////////////////////////////////////////////////////////

     void ParseData();
     
     void SetDebugLevel(int newDebugLevel) {m_DoDebug = newDebugLevel;};

     void *m_EngPtr;
     CString m_InCommand;
        
    private:
///////////////////////////////////////////////////////////////////////////////
//
//ReadData
//!Read data from input
//
//!\param void
//!
//\return   Nothing
//!
//! <b>Actions :</b><br>
//!   Reads the data from the input channels and puts them to the databuffer
//!
///////////////////////////////////////////////////////////////////////////////
      void ReadData();

	
     //!Buffers for input and output data
     char *m_DataBuffer, *m_CommandBuffer;
		
     //!Pointers to the buffers
     char *m_pLastCommand, *m_pLastData;


     //!Number of commands
     long m_NOfCommands;
     long m_ActualCommandExtracted;

     //!Creating an object used to parse the buffer
     CMsgRecv *m_msgParser ;

     //!Creating the I/O Ports and the log port
     CSocketPort *m_ICPort;

     //!The logs listing
     char m_LogString[255];
     
     //!ConfigString
     char *m_ConfigString;
     
     //!InternalState of the interface
     int m_RunFlag;
     
     vector<string> m_CommandVector;
     
     int m_DoDebug;

};

#endif //_IC_H_
