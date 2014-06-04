/***************************************************************************
 *   Copyright (C) 2006 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it                                           *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 *************************************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <cstdlib>
#include <pthread.h>

#include "conewireengine.h"
#include "timer.h"
#include "ic.h"
#include "commonDefinitions.h"
#include "version.h"

#include <time.h>
#include "timeUtil.h"
#include "cstring.h"
#include "signal.h"
#include "ClockManager.h"

#define INTERVAL_SEC 0
#define INTERVAL_USEC 50000

using namespace AutoVersion;

//The one and only clock
ClockManager mainClockManager;

COneWireEngine *eng;

struct itimerval tout_val;

void ex_program(int sig);
void* CheckForCommands(void* ptr);

int main(int argc, char *argv[])
{

    //pthread_t thread1;
    string version = AutoVersion::FULLVERSION_STRING;
    int maxNumberOfCycles = 0;

    //Intercetto ctrl-c
    (void) signal(SIGINT, ex_program);

    ///////////////////////////////////////
//    tout_val.it_interval.tv_sec = 0;
//    tout_val.it_interval.tv_usec = 0;
//    tout_val.it_value.tv_sec = INTERVAL_SEC; /* set timer for "INTERVAL (1) seconds */
//    tout_val.it_value.tv_usec = INTERVAL_USEC;
//    setitimer(ITIMER_REAL, &tout_val,0);
//    pthread_create( &thread1, NULL, CheckForCommands, 0x0);


//    (void)signal(SIGALRM,CheckForCommands);
    ////////////////////////////////////////////////////////////////////

    cout << "Starting the Raspy Monitor(TM)"<<endl;
    cout<<"V. "<<version<<" - "<<AutoVersion::DATE<<"/"<<AutoVersion::MONTH<<"/"<<AutoVersion::YEAR<<"\n Please stand by..." << endl;cout.flush();

    eng = new COneWireEngine(CONFIG_FILE);
    
    if (eng == NULL)
    {
        cout << "UNABLE TO CREATE ENGINE, STOPPING" << endl; cout.flush();
        exit(1);
    }
    
    cout << "Engine created succesfully" << endl;cout.flush();

    //Leggo il n umero massimo di cicli da riga di comando
    if (argc == 1)
    {
        cout << "No cycle limits: the program will run forever..."<<endl;
    }
    else
    {
        maxNumberOfCycles = atoi(argv[1]);

        cout << "WARNING! Execution limited to "<<maxNumberOfCycles<<" cycles"<<endl;
        sleep(5);
        eng->m_MaxNumberOfCycles = maxNumberOfCycles;
    }

    //Start
    if (eng->GetRunLevel())
    {
        std::string command;

        while (1)
        {
//            if (eng->GetRunLevel() == 0)
//            {
//
//                //The engine is in stop mode, wait 10 seconds and restart it
//                delete (eng);
//                eng = 0x0;
//
//                //Wait a bit
//                msDelay(10000);
//
//                //Recreate the main object
//                eng = new COneWireEngine("./config.ini");
//                eng->m_MaxNumberOfCycles = maxNumberOfCycles;
//                eng->Run();
//            }
//            else
            if (eng->GetRunLevel() >= 0)
            {
                eng->Run();
            }
            else if (eng->GetRunLevel() == -1)
            {
                break;
            }
            else if (eng->GetRunLevel() == -2){

                //The engine is in stop mode, wait 10 seconds and restart it
                delete (eng);
                eng = 0x0;

                //Wait a bit
                msDelay(10000);

                //Recreate the main object
                eng = new COneWireEngine("./config.ini");
                eng->m_MaxNumberOfCycles = maxNumberOfCycles;
                eng->Run();
            }
        }
    }

    if (eng != NULL)
    {
        delete eng;
    }


    return EXIT_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
void ex_program(int sig) {
    if (eng != 0x0){
     eng->SetRunLevel(-1);
     delete eng;
    }

    exit(0);

}
////////////////////////////////////////////////////////////////////////////////
void* CheckForCommands(void *ptr){

    while (1) {
        if ((eng != 0x0) && (eng->GetRunLevel() == 3))
        {
            eng->CheckForCommands2();
        }

        sleep(1);
    }

    return 0x0;
}