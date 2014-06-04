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
 
#ifndef TEMPERATURECONTROLLER_H
#define TEMPERATURECONTROLLER_H

#include <time.h>
#include "vcontroller.h"
#include "vdevice.h"
#include "timer.h"
#include "ds18X20.h"
#include "ds2438.h"
#include "digitalio.h"
#include "etraxgpio.h"
#include "gio.h"

/**

 * Classe che consente due tipi di funzionamento: puo' fungere da semplice lettore di temperatura e segnalazione verso l'esterno di allarmi
 * oppure gestire in automatico gli eventi di allarme in caso di superamento delle soglie. In quest'ultimo caso è necessario specificare:
 * - Un eventuale ingresso per un segnale di sbrinamento: se lo sbrinamento è attivo l'alarme è temporaneamnete disabilitato
 * - Eventuali segnali di allarme (intesi come ingressi digitali generici) che generano immediatamente un allarme
 * - Un'uscita da azionare per il segnale di allarme. Viene anche sempre utilizzato il pin 6 della porta B della FOX
 * La logica e' la seguente: nel caso in cui si sia superata la soglia per un tempo maggiore del parametro TOA vengono attivate le uscite e rimangono 
 * tali finchè l'allarme non è spento manualmente. Nel caso di allarmi diretti, invece, l'allarme è generato immediatamente.
 * I dispositivi di temperatura supportati sono: DS18S20, DS18B20 e DS2438, i dispositivi I/O sono della serie DS2408 e DS2405
 * 
 * Rga di Configurazione:
 * NAME:TempController, INPUT:20, ADDR:1027, SWALARMS:1, INPUTDF:, DFCH:, INVDF:, NIN:, IN1:, CH1, IN2; CH2: NOUT:, OUT1:, OUTCH1:, OUT2,..,INVERTIN:, INVERTOUT:, INVPIO:,COMMENT:T Mnd Vendita

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CTempCtrl : public CVController {
public:
    CTempCtrl(const char* configString, CVDevice *inDevice, CTimer *timer = 0x0);
    CTempCtrl(const char* configString, CTimer *timer = 0x0);

    ~CTempCtrl();
    
    bool SetInputDevice (CVDevice* inDevice);
    CVDevice* GetInputDevice(){return m_Device;};
    
        /**
     * Reads the temperature of the device and store it, if newTemp != NULL copies the new temp in the variable.
     * As a result also the local copy of the device memory is updated
     * @param updateFirst Flag that indicates if we have to perform a temperature conversion before reading the value
     * @param newTemp the variable to return
     * @return TRUE if operation successful
         */
    bool ReadTemperature(bool updateFirst, float *newTemp );
    
    float ReadTemperature(bool updateFirst);
    
    /**
     * Returns the last measured temperature
     * @return the last measured temperature, -100 if no temp available
     */
    float GetLastTemp();
    
    /**
     * Forces the temperature measurement of the device
     * @return TRUE if operation successful
     */
    bool UpdateTemp();
    
    bool Update(bool updateData);
    bool Update2(bool updateData);
    bool SetVal(float val){return false;};
    
    bool VerifyIOPresence();
    
    CString GetSpontaneousData(int lParam = 0);

    /**
     * Sets the internal alarm level of the device, note that the internal m_Hysteresis variable
     * is used to indicate the upper and lower levels for the temperature:
     * alarm = alarmLevel +- m_Hysteresis
     * @param alarmLevel the value around which to set the alarm
     * @return TRUE if operation successful
     */
    bool SetAlarmLevel(int MaxAlarmLevel, int MinAlarmLevel = -100);
    
    bool GetAlarmLevel(int *maxAlarmLevel, int *minAlarmLevel);

    /**
     * Sets the internal Alarms by applying the values read at the creation
     * @return TRUE if operation successfull
     */
    bool AutoSetAlarmLevel();

    /**
     * Returns the Minimum Alarm Trigger
     * @return the minumum alarm trigger 
     */
    int GetMinAlarmLevel(){return m_AlarmMin;};

    /**
     * Returns the maximum alarm trigger
     * @return the maximum alarm trigger
     */
    int GetMaxAlarmLevel(){return m_AlarmMax;};
    
    /**
     * Sets the alarm state of the sensor
     */
    void SetAlarmState();
    
    void ClearAlarmState() { m_IsInAlarm = false;};
    
    /**
     * Enables/disables the state of the alarm reporting via digitalOutput.
     * When invoked the status of the alarms is reset
     * @param  newState true -- enables the alarms, false disables them
     */
    void EnablePhoneAlarm(bool newState) ;
    
    /**
     * Returns the alarm state
     * @return the alarm state
     */
    bool GetAlarmState() {return m_IsInAlarm;};
    
    /**
     * Function used to check if it is time to raise a stronger alarm: if m_TimeOutOnError has passed from the last error
     * the function returns TRUE
     * @return TRUE if there is a strong alarm condition
     */
    bool CheckAlarmTime();
    
    /**
     * Updates the alarm status IF the device is configured for software alarms, otherwise do nothing
     * @return TRUE if the sensor is in alarm
     */
    bool UpdateAlarmStatus(t_DataVal data );
    
    void SetSoftwareAlarms(bool enableSWAlarms);
    
    
    /**
     * Fills the vector of the inputs. Note that the actual devices are not connected but it must be done
     * from the outside
     * @param configString string containing the list of the digital inputs
     * @param isOutput flag that indicates if the function has to create inputs or outputs
     * @return true on success
     */
    bool CreateIO(const char* configString, bool isOutput);
    
    void InitTempController();
    
    //Flag che indica se il controllo deve gestire gli allarmi HW o sono disabilitati manualmente
    bool m_AlarmEnabled;
    
    vector<CDigitalIO*> m_OutVector;
    vector<CDigitalIO*> m_InVector;
    CDigitalIO* m_DefreezeDevice;

    bool GetPhoneAlarmState() const
    {
        return m_PhoneAlarmActive;
    }

    
    private:
        
        void ActivatePhoneCaller(bool activate);
   
        CVDevice *m_Device;
        uchar m_DeviceFamilyNumber;
        
        float m_LastTemp;           //!last measured temp, -100 if no previous record available
        int m_AlarmMin;             //!Alarm level set, -100 if no alarm is set
        int m_AlarmMax;             //!Max Alarm value
        bool m_IsInAlarm;           //!Flag indicating wether the sensor is in alarm mode
        time_t m_TimeOfLastAlarm;   //!Time at which the sensor entered the alarm situation
        time_t m_TimeOutOnAlarm;    //!Maximum time to wait before sending a stronger alarm signalation
        bool m_SoftwareAlarms;   //!Flag indicating wether the sensor has to manage the alarms by itself
        
        int m_NofInputs;
        int m_NofOutputs;

        //Flag che indica se e' presente un ingresso di defreeze
        bool m_HasDefreeze;
        
        bool m_PhoneAlarmActive;
        
        //Indica se i piedino 6 di JP8 funziona invertito o meno (NA o NC)
        bool m_IsPIOInverted;
        

};

#endif
