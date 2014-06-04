//////////////////////////////////////////////////////////////////////////
//                          CIC class implementation
//                          See IC.h for details
//////////////////////////////////////////////////////////////////////////

#include "ic.h"
#include "conewireengine.h"

#include <iostream>

//mi serve per ritornare un valore nullo nella GetCommand
static const char nullString = 0x0;

//////////////////////////////////////////////////////////////////////////
//                          Constructor
//                          See IC.h for details
//////////////////////////////////////////////////////////////////////////
CIC::CIC(const char* configString)
{
  m_ConfigString = new char[sizeof(configString)+1]; //(char*)malloc(sizeof(configString)+1);
  
  if (m_ConfigString != NULL)
  {
    strcpy(m_ConfigString, configString);
  }
  
  m_ICPort = NULL;
  
  m_DoDebug = 0;
  
  Initialize(m_ConfigString);

  m_EngPtr = 0x0;

}

CIC::CIC( )
{
  m_ICPort = NULL;

}

//////////////////////////////////////////////////////////////////////////
//                           Destructor
//                          See IC.h for details
//////////////////////////////////////////////////////////////////////////
CIC::~CIC()
{
  
  //Freeing the buffers
  if (m_CommandBuffer != NULL)
  {
    free(m_CommandBuffer);
    m_CommandBuffer = NULL;
  }

  if (m_DataBuffer != NULL)
  {
    free(m_DataBuffer);
    m_DataBuffer = NULL;
  }

    //Closing the ports
  if (m_ICPort != NULL)
  {
    m_ICPort->Disconnect();
    delete m_ICPort;
    m_ICPort = NULL;
  }

    //Deleting the CMsgRecv object
  if (m_msgParser != NULL)
  {
    delete m_msgParser;
    m_msgParser = NULL;
  }
  
  //Deleting the configuration string
  if (m_ConfigString != NULL)
  {
    delete []m_ConfigString;
  }


}

//////////////////////////////////////////////////////////////////////////
//                          Initialize
//                          See IC.h for details
//////////////////////////////////////////////////////////////////////////
int CIC::Initialize(const char* configString) 
{
    bool error = false ;

    //Creating the ports > PORT00 = input port
    m_ICPort = new CSocketPort( );
    
    if (configString != NULL)
    {
        m_ConfigString = new char[strlen(configString)+1];
    }

    
    //Initializing the port
    if ( !m_ICPort->Init(configString) )
    {
        if (m_DoDebug)
            cout << "Interface : Could not initialize ports\nStopping" << endl;cout.flush();
       error = true;
    }

    //Allocate the buffers
    m_DataBuffer = (char*)malloc(IC_BUFF_SIZE*sizeof(char));
    m_CommandBuffer = (char*)malloc(IC_BUFF_SIZE*sizeof(char));

    //Checking for errors during allocation
    if ( (m_CommandBuffer == NULL) || (m_DataBuffer == NULL))
    {
        if (m_DoDebug)
            cout << "Interface : Could not allocate memory for IC buffers\nStopping" << endl;cout.flush();
      error = true;
    }

    //Resetting the buffers
    memset(m_CommandBuffer, '\0', IC_BUFF_SIZE);
    memset(m_DataBuffer, '\0', IC_BUFF_SIZE);

    //Initializing pointers
    m_pLastCommand = m_CommandBuffer;
    m_pLastData = m_DataBuffer;

    //Initializing number of commands
    m_NOfCommands = -1;
    m_ActualCommandExtracted = -1;

    //Instantiating the CMsgRecv object
    m_msgParser = new CMsgRecv();

    
    //Save configuration
    if (m_ConfigString != NULL)
    {
      strcpy(m_ConfigString, configString);
    }

    if (error)
    {
        //There were errors stopping....
        m_RunFlag = 0;
        return false;
    }
    else
    {
        //No errors - going to the next state
        m_RunFlag   = 2;  
        return true;
    }
    
}

//////////////////////////////////////////////////////////////////////////
//                          Startup
//                          See IC.h for details
//////////////////////////////////////////////////////////////////////////
int CIC::Startup()
{
   bool error = false;

   if (!m_ICPort->Connect()) 
   {
     error = true;
     if (m_DoDebug)
        cout<<"Interface: could not connect to port."<<endl;cout.flush();cout.flush();      
   }

   if (error)
   {
      m_RunFlag=0;
      return false;
   }
   else
   {
      m_RunFlag=1;
      return true;
   }
}

//////////////////////////////////////////////////////////////////////////
//                          Update
//                          See IC.h for details
//////////////////////////////////////////////////////////////////////////
int CIC::Update()  
{
    
   switch(m_RunFlag)
   {
   case -1:
      //System not Initialized

     Initialize(m_ConfigString);
      break;
   case 0:
      //System stopped

      break;
   case 1:
      //TBM
      ReadCommands();
      break;
   case 2:
      //System starting up

      Startup();
      break;
   case 3:
      //Waiting for setup to complete

      break;
   }

   return 1;

}

// //////////////////////////////////////////////////////////////////////////
// //                          Terminate
// //                          See IC.h for details
// //////////////////////////////////////////////////////////////////////////
int CIC::Terminate() 
{
    if (m_ICPort->GetSocketType() == SERVER_SOCKET)
    {
        m_ICPort->ShutdownServer( );
    }
    else
    {
        m_ICPort->Disconnect();
    }
    
    return 0;
}



//////////////////////////////////////////////////////////////////////////
//                          ReadCommands
//                          See IC.h for details
//////////////////////////////////////////////////////////////////////////
int CIC::ReadCommands()
{
   int charsToRead = 0 ;
   int charsRead = 0 ;
   int i = 0;
   char *p1 = NULL;
   
    //Computing free space in the buffer
    charsToRead = (m_CommandBuffer + IC_BUFF_SIZE) - m_pLastCommand - 1 ;

    //If buffer full...Houston we have a problem... stopping
    if (charsToRead <= 0)
    {
        if (m_DoDebug)
            cout <<"Class CIC: Command buffer full!!!\nFlushing Input Buffer" << endl;
        
        m_ICPort->flushInFlow();
        
//         m_RunFlag = 0;
//         return -1;
    }

    //TBR
//     char tbuff[128];
//     sprintf(tbuff,"Chars to read:%d",charsToRead);
//     m_msgLog->Write(tbuff);

    //Reading data from the socket
    if (m_pLastCommand == NULL)
    {
        //m_msgLog->Write("Argh!! the command buffer is invalid");
        return -1;
    }

    charsRead = m_ICPort->Read(m_pLastCommand, charsToRead) ;

    //No Data Read exiting
    if (charsRead == 0)
    {
       //m_msgLog->Write("No Data found");
//        printf("\nNo data found\n");
       return 0;
    }

    //Eliminate all the \0 chars in the received string
    p1 = m_pLastCommand;
    for (i = 0; i < charsRead; i++)
    {
        if ((*p1 == '\0') || (*p1 == '\n'))
        {
//             //TBR
//             cout << "Carattere speciale trovato" << endl; cout.flush();
            *p1 = '$';
        }
        p1++;
    }
        

//TBR
//    printf("\nData Read");
//    printf("\nCharsRead == %d\n",charsRead);
    
    //Updating the pointer
    m_pLastCommand = m_pLastCommand + charsRead ;
//    printf("\n\nPointer updated\n\n\n");

    //NULL terminating the buffer
    *m_pLastCommand = '\0' ;
          
//TBR
//     cout << "Command Buffer: " << m_CommandBuffer << endl;


    //Updating the CommandTable
//     ParseData();
 
    return charsRead;   
}

//////////////////////////////////////////////////////////////////////////
//                          ParseData
//                          See IC.h for details
//////////////////////////////////////////////////////////////////////////
void CIC::ParseData()
{
    //Parsing the new buffer
    m_msgParser->Parse(m_CommandBuffer, IC_BUFF_SIZE - 1) ;

    m_NOfCommands = m_msgParser->GetValidLabels( );
    //TBR
//     cout << "Numero comandi registrato: " << m_NOfCommands << endl; cout.flush();

    //If nOfCommands = 0 there are no messages in the queue
    if (m_NOfCommands == 0)
    {
       //No Valid commands in the Buffer
        m_NOfCommands = -1;
        return ;
    }
    
    m_ActualCommandExtracted = -1;

    //Updating the pointer in the buffer
    m_pLastCommand = m_msgParser->GetBufferNextPtr();

}

//////////////////////////////////////////////////////////////////////////
//                          GetCommand
//                          See IC.h for details
//////////////////////////////////////////////////////////////////////////
char* CIC::GetCommand()
{
    //Are there anymore commands ??
    //15/09/2009 -- aggiunto il check su m_NofCommands == -1 perchè mi è crashato in debug qui
    if ((m_ActualCommandExtracted == m_NOfCommands - 1 ) || (m_NOfCommands == -1))
    {
        //NO, so return NULL
        return (char*)(&nullString);
    }
    
    //Extract a command and decrease the counter
    m_ActualCommandExtracted++;

    //Checking if there are errors
    if (m_msgParser->GetLabelTable() == NULL)
    {
        //No valid messages return NULL
        return (char*)(&nullString);
    }
    else
    {
        return m_msgParser->GetLabelTable()[m_ActualCommandExtracted];
    }
    
}

//////////////////////////////////////////////////////////////////////////
//                          ReadData
//                          See IC.h for details
//////////////////////////////////////////////////////////////////////////
void CIC::ReadData()
{
   char tbuf;

   //FIXME:Da controllare se funziona anche senza la finta lettura
   //Polling the Port to see if someone is connected
   m_ICPort->Read(&tbuf,1); 

   if (!m_ICPort->IsConnected())
   {
      return;
   }   
 
 
}

//////////////////////////////////////////////////////////////////////////
//                          OutputLog
//                          See IC.h for details
//////////////////////////////////////////////////////////////////////////
bool CIC::OutputData(const char *data, int datalen)
{
    int len = 0;
    char* outBuffer = 0x0;
    
    //Check if port is connected
    if (!IsConnected())
    {
        //Port is not connected, don't signal error
        return false;
    }
    
    outBuffer = (char*)malloc(datalen+5);

    if (outBuffer == 0x0)
    {
        return false;
    }
    
    memset (outBuffer, 0x0, datalen+1*sizeof(char));
    memmove (outBuffer, data, datalen*sizeof(char));
    
    //Add 1 to write out also the terminating null char for XML
    len = m_ICPort->Write(outBuffer, datalen+1);
    
    if (outBuffer != 0x0)
    {
        free (outBuffer);
        outBuffer = 0x0;
    }
   
    if (len == datalen + 1)
    {
        return true;
    }
    else
    {
        return false;
    }
   

}

//////////////////////////////////////////////////////////////////////////
//                          WriteError
//                          See IC.h for details
//////////////////////////////////////////////////////////////////////////
bool CIC::WriteError( e_AFOErrors errorType, int netIndex, int objNumber )
{
    char netBuffer[8], objBuffer[8], codeBuffer[8];
    CString outMessage;
    
    
    sprintf(netBuffer,"%d", netIndex);
    sprintf (objBuffer, "%d", objNumber);
    sprintf (codeBuffer, "%d", errorType+1);
    
    outMessage = "<ERROR CODE=\"";
    outMessage +=codeBuffer;
#ifndef SMALL_MEMORY_TARGET
    outMessage +="\" DESCRIPTION=\"";
    outMessage += afoErrorsStrings[errorType];
#endif
    outMessage +="\" NETORIGIN=\"";
    outMessage +=netBuffer;
    outMessage +="\" DEVICEORIGIN=\"";
    outMessage +=objBuffer;
    outMessage +="\" />";
    
    if (!OutputData( outMessage.c_str(), outMessage.size()))
    {
        return true;
    }
    else
    {
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////
int CIC::ReadCommands2(){
    COneWireEngine *eng = reinterpret_cast<COneWireEngine*>(m_EngPtr);
    char newChar;
    bool stop = false;


    while (!stop){
        bool endCharRecvd = false;

        if (m_InCommand.length()  == 0){
            while (m_ICPort->Read(&newChar,1) > 0) {
                if (newChar == '<'){
                    m_InCommand="";
                    m_InCommand+=newChar;
                    break;
                }
            }
        }

        while (m_ICPort->Read(&newChar,1) > 0){
            m_InCommand+=newChar;
            if (newChar == '>'){
                endCharRecvd = true;
                break;
            }
            else if (newChar == '<'){
                //Ho ricevuto uno start quando non dovevo
                m_InCommand="<";
                continue;
            }
        }

        if ( endCharRecvd ) {
            eng->m_XMLParser.ParseXML( m_InCommand.c_str() );

            eng->ExecCommand();
            m_InCommand.clear();
        }
        else {
            //Mi manca un messaggio completo
            stop = true;
        }
    }

    return 0;
}