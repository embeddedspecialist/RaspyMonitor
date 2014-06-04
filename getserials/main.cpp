/* 
 * File:   main.cpp
 * Author: amirrix
 *
 * Created on 26 novembre 2009, 10.24
 */

#include <iostream>
#include <cstdlib>
#include <string>
#include <stdio.h>
#include "cownet.h"
#include "ownet.h"
#include <sys/time.h>
#include "IniFileHandler.h"
#include "commonDefinitions.h"
#include "afoerror.h"
#include "ioutil.h"
#include "crcutil.h"

#define MAXDEVICES 300
#define MAX_VAL_COUNTER 65000

using namespace std;
int port;
uchar SN[MAXDEVICES][8];
int totTime;
COWNET *m_Master;
CAfoErrorHandler *errHndlr = 0x0;
CRCUtil m_CRCUtil;
    
bool SalvaDispositivi(char *deviceName, uchar family);

int CercaDispositivi(uchar famNum)
{
    int numDevFound = 0;
    int maxNofDev = MAXDEVICES-1;
    
    int i = 0;
    struct timeval tpStart, tpStop;
                
    cout << "Scanning, please wait..." << endl;
    memset (SN, 0, sizeof(SN));
                
                //Get actual time
    gettimeofday( &tpStart, 0x0);
                        
    numDevFound = m_Master->FindDevices(port, &SN[0], (SMALLINT)famNum, maxNofDev);
                
                //Get Finish Time
    gettimeofday( &tpStop, 0x0);
    
    totTime += (tpStop.tv_sec - tpStart.tv_sec)*1000+(tpStop.tv_usec - tpStart.tv_usec)/1000;
    cout<<"Search Time (ms): "<< (tpStop.tv_sec - tpStart.tv_sec)*1000+(tpStop.tv_usec - tpStart.tv_usec)/1000<<endl;
    cout << "Number of devices found : "<< numDevFound << endl;
    cout << "ROM codes found:" << endl;
                
    for (i = 0; i < numDevFound; i++)
    {
        PrintSerialNum(&SN[i][0]);
        cout << endl;
    }
    
    return numDevFound;
}

bool AcquisisciMaster()
{
    t_Str portName;
    int netDelay = 0;
    CIniFileHandler iniFile;
    CString inCommand;
    
    if (iniFile.Load( "config.ini" ))
    {
        //Leggi l'inizializzazione dal file
        portName = iniFile.GetString( "Port", "COMMON" );
        netDelay = iniFile.GetInt( "NetDelay", "COMMON" );
        if (netDelay < 0)
        {
            netDelay = 0;
        }

        inCommand=portName.c_str();
        m_Master = new COWNET(&iniFile, "COMMON");
    }
    else
    {

        m_Master = new COWNET();
        cout << "Please master specify port (i.e. /dev/ttyUSB0) :";
        cin>>inCommand;
    }

    errHndlr = new CAfoErrorHandler();
    m_Master->SetWireless( 0 );
    m_Master->SetNetDelay( netDelay );
    m_Master->SetErrorHandler( errHndlr );
    m_Master->SetCRCUtil(&m_CRCUtil);
    //errHndlr.SetDebugMode( true );
    
    cout << "Searching for MASTER device..." << endl;
    cout.flush();

    port = m_Master->owAcquireEx((char*)inCommand.c_str());
  
    if (port < 0)
    {
        cout << "An error occurred while connecting to the NET !!" << endl;
//         m_Master->owRelease(port);
        exit(1);
    }

    return true;
}
    

int main(int argc, char *argv[])
{   
    CString  inCommand;
    
    int family;
    
    totTime = 0;
    cout << "Hello, world!" << endl;

    AcquisisciMaster();
    
    while(1)
    {
        cout << "Command ?\n1 - Scan devices from family code\n2 - Scan and save devices from family code\n3 - Scan every device on the NET\n99 - Reconnect to Master\nq - to quit\nYour choice: ";
        cin >> inCommand;
        fflush(stdin);
    
        if (inCommand[0] == '1')
        {
            while (1)
            {
                int cycle = 0;
                
                cout << "Please insert family code or 'q' to quit : "; 
                cin>>inCommand;
                
                if (inCommand[0] == 'q')
                    break;

                char tmpBuff[16];
                memset (tmpBuff, 0x0, 16);
                memcpy (tmpBuff, inCommand.c_str(), inCommand.length());
                sscanf (tmpBuff, "%02X", &family);
                             
                while (cycle < 5)
                {
                    cout << "Search number: "<<cycle+1<<endl;
                    CercaDispositivi(family);
                    cycle++;
                    msDelay(1000);
                }
                
            }
        }
        else if (inCommand[0] == '2')
        {
            bool deviceExists = true;
            unsigned int family;
            CString famString;
            CString devName;
            
            while (1)
            {
                deviceExists = true;
                cout << "Please insert family code or 'q' to quit :";
                cin>>inCommand;
                char tmpBuff[16];
                memset (tmpBuff, 0x0, 16);
                memcpy (tmpBuff, inCommand.c_str(), inCommand.length());
                sscanf (tmpBuff, "%X", &family);
                    
                if (inCommand[0] == 'q')
                {
                    break;
                }
        
                famString = inCommand;
        
                if (famString == "10")
                {
                    devName = Device_strings[DEV_DS18S20];
                    family = 0x10;
                }
                else if (famString == "28")
                {
                    devName = Device_strings[DEV_DS18B20];
                    family = 0x28;
                }
                else if (famString == "29")
                {
                    devName = Device_strings[DEV_DS2408];
                    family = 0x29;
                }
                else if ((famString == "2c") || (famString == "2C"))
                {
                    devName = Device_strings[DEV_DS2890];
                    family = 0x2C;
                }
                else if (famString == "26")
                {
                    devName = Device_strings[DEV_DS2438];
                    family = 0x26;
                }
                else if ((famString == "05") || (famString == "5"))
                {
                    devName = Device_strings[DEV_DS2405];
                    family = 0x05;
                }
                else if (famString == "51")
                {
                    devName = Device_strings[DEV_DS2751];
                    family = 0x51;
                }
                else
                {
                    deviceExists = false;
                    cout << "Wrong family number!!" << endl; cout.flush();
                }
                
                if (deviceExists)
                {
                    SalvaDispositivi((char*)devName.c_str(), family);
                }
            }
            
        }
        else if (inCommand[0] == '3')
        {
            int numTotDisp = 0;
            totTime = 0;
            cout << "Searching for DS18S20 (Temp)" << endl;
            CercaDispositivi(DS18S20_FN);
            cout<<"\r\n"<<endl;
            
            cout << "Searching for DS18B20 (Temp)" << endl;
            CercaDispositivi(DS18B20_FN);
            cout<<"\r\n"<<endl;
            
            cout << "Searching for DS2405 (DIDO 2Channel)" << endl;
            CercaDispositivi(DS2405_FN);
            cout<<"\r\n"<<endl;
            
            cout << "Searching for DS2438 (AI, Umid, Lux)" << endl;
            CercaDispositivi(DS2438_FN);
            cout<<"\r\n"<<endl;
            
            cout << "Searching for DS2890 (AO)" << endl;
            CercaDispositivi(DS2890_FN);
            cout<<"\r\n"<<endl;
            
            cout << "Searching for DS2408 (DIDO)" << endl;
            CercaDispositivi(DS2408_FN);
            cout<<"\r\n"<<endl;
            
            cout << "Searching for DS2751 (Virtual OW)" << endl;
            CercaDispositivi(DS2751_FN);
            cout<<"\r\n"<<endl;
            
            cout << "Totoal number of devices : "<< numTotDisp << endl;
            
            cout<<"Total time for search (ms): "<< totTime<<endl;
            
        }
        else if (inCommand[0] == '9')
        {
            m_Master->owRelease(port);
            delete (m_Master);
            delete (errHndlr);
            AcquisisciMaster();

        }
        else if (inCommand[0] == 'q')
        {
            break;
        
        }
        else
        {
            cout << "Command Not valid." << endl;
        }
    } //While
   
 
    cout << "Release Port" << endl;
  
    m_Master->owRelease(port);
    delete (m_Master);
    delete (errHndlr);
  
    return EXIT_SUCCESS;
}

bool SalvaDispositivi(char *deviceName, uchar family)
{
    std::string configString;
    std::string famString;
    std::string serString;
    FILE *fileHandler = 0x0;
    CIniFileHandler *iniReader;
    bool deviceExists = true;
    int deviceCounter = 0;
    CString fileName;
    CString devConfigString;
    CString outString;
    CString inCommand;
    bool isDevFound = false;
      
    cout << "Please insert filename (q to quit):"; 
    cin >> inCommand;
    if (inCommand[0] == 'q')
    {
        return true;
    }
    else
    {
        fileName = inCommand;
    }

          //Controllo se il file esiste già o meno, se non esiste gli metto l'intestazione
    fileHandler = fopen(fileName.c_str(), "a");
          
    if (ftell(fileHandler) == 0)
    {
              //Il file è appena stato creato, aggiungo l'header standard
        outString = "[LIST]\r\n";
              
        fprintf (fileHandler, "%s", outString.c_str());
    }
          
          //Chiudo il file per permettere di leggerlo come ini
    fclose (fileHandler);

          //Cerco se nel file indicato ci sono gìa' dei dispositivi
    iniReader = new CIniFileHandler(inCommand);
          
    for (deviceCounter = 1; deviceCounter < MAX_VAL_COUNTER; deviceCounter++)
    {
              
        devConfigString = "Device";
              
        if (deviceCounter < 10)
        {
            devConfigString+="0";
        }
              
        devConfigString+=deviceCounter;
              
        devConfigString = iniReader->GetString(devConfigString, "LIST");
              
        if (devConfigString.size() == 0)
        {
                  //Ho finito i dispositivi
            break;
        }
    }
          
          //Cancello il lettore
    delete iniReader;
          
    if (deviceCounter == MAX_VAL_COUNTER)
    {
        cout << "Maximum number of devices per file reached!!\n Please change file and retry"<<endl;
    }
    else
    {
        if (deviceCounter > 1)
        {
            cout << "Found " << deviceCounter - 1 << " devices in the provided file, restarting from number " << deviceCounter << endl;
        }
        else
        {
            cout << "No devices found, starting from number 1" << endl;
            deviceCounter = 1;
        }
              
        cout << "(c) - Continue, (m) - modify start number, (q) - quit"<<endl;
        cin >> inCommand;
              
        if ((inCommand[0] == 'q') || (inCommand[0] == 'Q'))
        {
            return true;
        }
        else if ((inCommand[0] == 'm') || (inCommand[0] == 'M'))
        {
            cout << "Please insert starting number: ";
            cin >> inCommand;
            char tmpBuff[16];
            memset (tmpBuff, 0x0, 16);
            memcpy (tmpBuff, inCommand.c_str(), inCommand.length());
            sscanf(tmpBuff, "%d", &deviceCounter);

        }
              
        fileHandler = fopen(fileName.c_str(), "a+");
          
        while(deviceExists)
        {
            char snBuffer[255], numDevFound;
            outString.erase();
                    
            cout << "Please conncet the device and press (c) to continue or (q) to quit:" << endl;
            cin >> inCommand;
                            
            if ((inCommand[0] == 'q') || (inCommand[0] == 'Q'))
            {
                fclose (fileHandler);
                break;
            }
            
            cout << "Getting device code, please wait..." << endl;
            memset (SN, 0x0, sizeof(SN)*sizeof(uchar));
            
            isDevFound = false;        
            numDevFound = m_Master->FindDevices(port, &SN[0], family, 1);
            
            memset (snBuffer, 0x0, 255*sizeof(char));
            ConvertSN2Str(snBuffer, &SN[0][0]);
                            
            if ((numDevFound <= 0) || (!strncmp(snBuffer+14, "00", 2)))
            {
                cout << "Device NOT found !!" << endl;
            }
            else
            {
                memset (snBuffer, 0x0, 255*sizeof(char));
                ConvertSN2Str(snBuffer, &SN[0][0]);
                
                cout << "Device number : ";
                PrintSerialNum( &SN[0][0]);
                cout << " found" << endl;
                
                cout << "Writing to file" << endl;
                
                outString = "Device";
                if (deviceCounter < 10)
                {
                    outString += "0";
                }
                outString+= deviceCounter;
                outString+= "=NAME:";
                outString+= deviceName;
                outString+=", SN:"; 
                outString+= snBuffer;
                outString+= "\n";
                
                fprintf (fileHandler, "%s", outString.c_str());
                
                deviceCounter++;
            }
        }
    }

    return true;
}

