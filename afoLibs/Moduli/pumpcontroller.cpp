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
#include "pumpcontroller.h"

#ifndef AFO_NO_PUMP
///////////////////////////////////////////////////
//              STANDARD CONSTRUCTOR
///////////////////////////////////////////////////
CPumpController::CPumpController(const char* configString, CTimer *timer)
    : CVController( configString)
{
    int tempval;

    //Reset pointers
    m_Pump1.stateDevice = 0x0;
    m_Pump1.commandDevice = 0x0;
    m_Pump1.status = PUMPOFF;

    m_Pump2.stateDevice = 0x0;
    m_Pump2.commandDevice = 0x0;
    m_Pump2.status = PUMPOFF;

    //Get Swaptime and transform it from days to seconds
    m_IniLib.GetConfigParamInt(configString, "SWAPTIME",  &tempval, 7);
    //TODO da rimettere a posto con conteggio in giorni
    //m_SwapInterval = tempval;
    m_SwapInterval = 86400 * tempval;

    m_IniLib.GetConfigParamInt( configString, "TIMERID", &m_TimerID, -1);

    if ( (m_TimerID > 0) && (timer != 0x0) )
    {
        m_Timer = timer;
        m_UseTimer = true;
    }
    else
    {
        m_UseTimer = false;
    }

    m_ControllerType = DEV_PUMPCTRL;
    m_TypeOfTimerVal = TIMERVAL_DIGITAL;

    m_LastSwapTime = 0;
    m_LastPumpTried = 2;
}

///////////////////////////////////////////////////
//              STANDARD DESTRUCTOR
///////////////////////////////////////////////////
CPumpController::~CPumpController()
{
}

///////////////////////////////////////////////////
//              SetIOs
///////////////////////////////////////////////////
bool CPumpController::SetIOs( CDigitalIO *input1, CDigitalIO *input2, CDigitalIO *output1, CDigitalIO *output2 )
{
    bool retVal = true;

    m_Pump1.stateDevice = input1;
    m_Pump1.commandDevice = output1;

    m_Pump2.stateDevice = input2;
    m_Pump2.commandDevice = output2;

    return retVal;
}

///////////////////////////////////////////////////
//              Update
///////////////////////////////////////////////////
bool CPumpController::Update(bool updateData )
{
    bool retVal = false;
    time_t actTime;

    //TODO da mettere la gestione del timer
    actTime = time(NULL);

    //Check if both pumps are in stop state
    if ((m_Pump1.status == PUMPOFF) && (m_Pump2.status == PUMPOFF))
    {
        //Both pumps in stop state, probably it is the first time we pass from here
        //Synchronize: stop both pumps
        EnablePump(1,false);
        EnablePump(2,false);
        
         //Try to start pump number 1
         if (EnablePump( 1, true))
         {
            //Get time
            m_LastSwapTime = time(NULL);
            m_Pump1.status = PUMPON;

            //TBR
            cout << "Abilitata pompa 1" << endl;
            return true;
         }
         else
         {
             PushError(AFOERROR_UNABLE_TO_START_PUMP1, m_NetNumber, m_DeviceNumber);
             m_Pump1.status = PUMPALARM;

             if (EnablePump( 2, true))
             {
                //Get time
                m_LastSwapTime = time(NULL);
                m_Pump2.status = PUMPON;
                cout << "Abilitata pompa 2" << endl;
                return true;
             }
             else
             {
                PushError(AFOERROR_UNABLE_TO_START_PUMP2, m_NetNumber, m_DeviceNumber);
                PushError(AFOERROR_UNABLE_TO_START_BOTHPUMPS, m_NetNumber, m_DeviceNumber);
                m_Pump2.status = PUMPALARM;
             }

             return retVal;
         }
    }

    if (m_Pump1.status == PUMPON)
    {
        //Check status of pump1
//         if (m_Pump1.stateDevice->GetChannelLevel(false) == 0)
        if (m_Pump1.stateDevice->GetState(false) == 0)
        {
            //An error occurred: pump should be on but it isn't
            EnablePump( 1, false);
            m_Pump1.status = PUMPALARM;

            cout << "Allarme pompa 1: stato inattivo" << endl;

            PushError(AFOERROR_PUMP1_NOT_WORKING, m_NetNumber, m_DeviceNumber);
            return false;
        }

        //Check time
        if (actTime >= m_LastSwapTime + m_SwapInterval)
        {
            //It is time to change
            if (EnablePump( 2, true))
            {
                m_Pump2.status = PUMPON;

                if (EnablePump( 1, false))
                {
                    m_LastSwapTime = time (NULL);
                    m_Pump1.status = PUMPOFF;

                    cout << "Swap time: Abilitata pompa 2" << endl;

                    return true;
                }
                else
                {
                    PushError(AFOERROR_UNABLE_TO_START_PUMP2, m_NetNumber, m_DeviceNumber);
                }
            }
            else
            {
                PushError(AFOERROR_UNABLE_TO_STOP_PUMP1, m_NetNumber, m_DeviceNumber);
            }
        }

        return false;
    }

    if (m_Pump2.status == PUMPON)
    {
        //Check status of pump1
//         if (m_Pump2.stateDevice->GetChannelLevel(false) == 0)
        if (m_Pump2.stateDevice->GetState(false) == 0)
        {
            //An error occurred: pump should be on but it isn't
            EnablePump( 2, false);
            m_Pump2.status = PUMPALARM;

            cout << "Allarme pompa 2: stato inattivo" << endl;

            PushError(AFOERROR_PUMP2_NOT_WORKING, m_NetNumber, m_DeviceNumber);
            return false;
        }

        //Check time
        if (actTime >= m_LastSwapTime + m_SwapInterval)
        {
            //It is time to change
            if (EnablePump( 1, true))
            {
                m_Pump1.status = PUMPON;

                if (EnablePump( 2, false))
                {
                    m_LastSwapTime = time (NULL);
                    m_Pump1.status = PUMPOFF;

                    cout << "Swap time: Abilitata pompa 1" << endl;

                    return true;
                }
                else
                {
                    PushError(AFOERROR_UNABLE_TO_START_PUMP1, m_NetNumber, m_DeviceNumber);
                }
            }
            else
            {
                PushError(AFOERROR_UNABLE_TO_STOP_PUMP2, m_NetNumber, m_DeviceNumber);
            }
        }

        return false;
    }

    if ((m_Pump1.status == PUMPALARM) && (m_Pump2.status == PUMPOFF))
    {
        if (EnablePump( 2, true))
        {
            m_LastSwapTime = time (NULL);
            m_Pump2.status = PUMPON;

            cout << "Pompa 2 accesa su allarme pompa 1" << endl;

            return true;
        }
        else
        {
            PushError(AFOERROR_UNABLE_TO_START_PUMP2, m_NetNumber, m_DeviceNumber);
            return false;
        }
    }

    if ((m_Pump2.status == PUMPALARM) && (m_Pump1.status == PUMPOFF))
    {
        if (EnablePump( 1, true))
        {
            m_LastSwapTime = time (NULL);
            m_Pump1.status = PUMPON;

            cout << "Pompa 1 accesa su allarme pompa 2" << endl;

            return true;
        }
        else
        {
            PushError(AFOERROR_UNABLE_TO_START_PUMP1, m_NetNumber, m_DeviceNumber);
            return false;
        }
    }

    cout << "Entrambe le pompe sono in allarme" << endl;
    PushError(AFOERROR_ALARM_ON_BOTH_PUMPS, m_NetNumber, m_DeviceNumber);
     //Both pumps are in alarm
    if (m_LastPumpTried == 2)
    {
        if (EnablePump( 1, true ) && (m_Pump1.stateDevice->GetState()))
        {
            //Pump restarted
            m_LastSwapTime = time (NULL);
            m_Pump1.status = PUMPON;

            retVal = true;
        }
        else
        {
            PushError(AFOERROR_UNABLE_TO_START_PUMP1, m_NetNumber, m_DeviceNumber);
            cout << "Fallita riaccensione pompa 1" << endl;
            m_LastPumpTried = 1;
            EnablePump( 1, false);
        }
    }
    else
    {
        if (EnablePump( 2, true ) && (m_Pump2.stateDevice->GetState()))
        {
            //Pump restarted
            m_LastSwapTime = time (NULL);
            m_Pump2.status = PUMPON;

            retVal = true;
        }
        else
        {
            PushError(AFOERROR_UNABLE_TO_START_PUMP2, m_NetNumber, m_DeviceNumber);
            cout << "Fallita riaccensione pompa 1" << endl;
            m_LastPumpTried = 2;
            EnablePump( 2, false);
        }
    }



    return retVal;
}


///////////////////////////////////////////////////
//              EnablePump
///////////////////////////////////////////////////
bool CPumpController::EnablePump( int pump, bool enable )
{
    bool retVal = false;

    if (pump == 1)
    {
        retVal = m_Pump1.commandDevice->SetState(enable);
    }
    else if (pump == 2)
    {
        retVal = m_Pump2.commandDevice->SetState(enable);
    }

    return retVal;
}

///////////////////////////////////////////////////
//              VerifyIOPresence
///////////////////////////////////////////////////
bool CPumpController::VerifyIOPresence( )
{
    return (m_Pump1.stateDevice->VerifyIOPresence() && m_Pump1.commandDevice->VerifyIOPresence() && m_Pump2.stateDevice->VerifyIOPresence() && m_Pump2.commandDevice->VerifyIOPresence());
}

///////////////////////////////////////////////////
//              GetSettings
///////////////////////////////////////////////////
void CPumpController::GetSettings( int * pump1State, int * pump2State, int * swapTime )
{
    *pump1State = m_Pump1.status;
    *pump2State = m_Pump2.status;
    *swapTime = m_SwapInterval;
}

///////////////////////////////////////////////////
//              SetSwapTime
///////////////////////////////////////////////////
void CPumpController::SetSwapTime( time_t newInterval )
{

    m_SwapInterval = newInterval * 86400;

}


//////////////////// SMALL TARGET ////////////////////////////
#else
CPumpController::CPumpController( const char* configString, CTimer * timer ) : CVController( configString)
{
}

CPumpController::~ CPumpController( )
{
}

bool CPumpController::SetIOs( CDigitalIO * input1, CDigitalIO * input2, CDigitalIO * output1, CDigitalIO * output2 )
{
}

bool CPumpController::Update( bool updateData )
{
}

bool CPumpController::VerifyIOPresence( )
{
}

void CPumpController::GetSettings( int * pump1State, int * pump2State, int * swapTime )
{
}

void CPumpController::SetSwapTime( time_t newInterval )
{
}

bool CPumpController::EnablePump( int pump, bool enable )
{
}

#endif
