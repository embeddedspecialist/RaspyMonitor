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
#ifndef STDALARMCOORDINATOR_H
#define STDALARMCOORDINATOR_H

#include <vcoordinator.h>
#include "cstring.h"
#include "LibIniFile.h"
#include "IniFileHandler.h"
#include "digitalio.h"
#include "commonDefinitions.h"
#include "nthacc.h" //<---------- classe di gestione della scheda controllo accessi/allarme (da vedere se modificare quella esistente
#include "ibuttonreader.h"
#include "Cmd.h"

#define MAX_NOF_ZONES 10
#define MAX_NOF_ERRORS 50

//TODO
//ATTENZIONE::C'E' QUALCOSA CHE NON VA NELLA LOGICA DI CONTROLLO DEI CONTROLLI ACCESSI: SE QUESTI HANNO UN FILE DI CHIAVI VALIDO QUESTO HA LA PRECEDENZA RISPETTO A QUANTO DICE IL COORDINATORE
using namespace std;

typedef enum InputType {
    OP_OR,
    OP_AND,
    OP_NONE
}t_InputType;

typedef struct AlarmsInputManager {
    AlarmsInputManager(){digitalInput = 0x0; zoneOperator = OP_NONE;};
    CDigitalIO* digitalInput;
    vector<int> zones;
    t_InputType zoneOperator;
}t_AlarmInputs;

typedef struct {
    int channel; //1, 2, o 3
    CNTH_ACC* accController;
}t_AccController;

typedef struct {
    int when;       //Quando parte il comando 0 - sempre al cambiare dello stato, 1 - solo inserimento, 2 - solo disinserimento
    CString type;
    CString field;
    CString value;
    CString field2;
    CString value2;
    int address;
}t_Command;

typedef struct {
    int zoneNumber;
    int remoteAddr;
    vector<t_Command> commandList;
    vector<t_AccessData> keysList;
    vector<t_AccController> accCtrlList;
    vector<CIButtonReader*> ibtnrdrList; //Questi sono i lettori diretti...
    vector<CDigitalIO*> digINList;
    vector<CDigitalIO*> digOUTList;
    //Questo serve per comunicare con una centralina ext se l'allarme è attivato o meno
    CDigitalIO* outDido;
    CDigitalIO* inDido;
    int inDidoLogic; //Determina la logica dell'ingresso di attivazione
    int lastInputState; //Memorizza l'ultimo stato dell'ingresso per poter decidere se devo attivare o dosattivare l'allarme
    bool isAlarmActive;
    bool isInAlarm;
}t_Zone;

/**
Classe che gestisce l'interfacciamento verso un sistema di allarme a monte e i moduli controllo accessi a valle.
Mantiene traccia dei passaggi con data e ora e interroga continuamente i moduli di accesso per dare il consenso allo stacco dell'allarme. Si appoggia sul file alarmControl.ini
 * Nella lisrta dei controller digitali ci sono prima gli ingressi e poi le uscite. Il numero di ingressi è memorizzato in m_NofInputs.
 * Stringa di configurazione:
 * NAME:FullUTACtrl,ADDR:YY, NOFDIGIN:,IN1:,IN2:,NOFDIGOUT:,OUT1:,OUT2:,COMMENT:
 *
 * ATTENZIONE: Il coordinatore si connette agli ingressi/uscite attraverso gli indirizzi in memoria e funziona in modo "transnet". NON funziona nel caso in cui i dispositivi si riferiscono a net diverse da quelle del coordinatore e queste net sono collegate via wireless

	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/

//TODO List
//1 - Salvataggio su log degli eventi
//2 - Errori codificati per consentire il tracciamento degli eventi
//3 - comandi per Aggiungere/cancellare una chiave -- OK
//4 - Salvataggio su file ini della chiave -- OK
class AlarmCoordinator : public CVCoordinator
{
public:
    AlarmCoordinator(const char *configString, CTimer *timer);

    ~AlarmCoordinator();

    CIniFileHandler m_AlarmIniHandler;

    int m_NofInputs,m_NofOutputs;

    //Questo vettore coordina gli ingressi tra varie zone permettendo di correlare le informazioni
    //provenienti dagli ingressi con i due operatori logici AND e OR
    vector<t_AlarmInputs> m_AlarmInputsVector;

    bool SetVal(float val){return false;};
    
    bool Update(bool updateData);
    bool Update2(bool updateData){return false;};
    
    CString GetSpontaneousData(int lParam = 0) { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};

    bool InitAlarmSystem();

    bool ConnectControllers();

    bool ConnectDIDO(CString sectionName, CIniFileHandler* iniFileReader, bool isInput);

    //Vettore contenente la lista delle zone
    vector<t_Zone> m_ZoneVector;

    int DecodeZones(CString zoneListString, int* zoneNumbers, int maxNofZones, t_AlarmInputs *alarmInput);

    bool LoadKeys(int zoneIndex, CString zoneName, CIniFileHandler *iniFileReader);

    int m_ErrorNumber;

    bool SearchForActivity();

    bool SetZoneAlarm(int zone){return SetZoneDIGOUT(1,zone);};
    bool ClearZoneAlarm (int zone){return SetZoneDIGOUT(0, zone);};

    /**
     * Search the given address in every zone and sets the alarm flag according
     * @param address the address of a dido
     * @return the zone index or -1 if dido not found
     */
    int FindZoneInAlarm(int address);

    bool SendAlarms();

    bool ResetAlarm(int zoneIndex = -1);

    bool ChangeZoneAlarmState(int zoneIndex);

    bool SetZoneAlarmState(unsigned int zoneIndex, bool newState);

    bool AddKey(CString name, CString keySN, bool enablesAlarms,int zoneIndex=-1 );
    bool RemoveKey (CString keySN, int zoneIndex = -1);


    private:
        /**
         * Cambia lo stato dei digitali di uscita di una certa zona
         * @param zone indice della zona
         * @param state stato delle uscite (1 uscite chiuse, 0 uscite aperte)
         * @return true se tutto ok
         */
        bool SetZoneDIGOUT(int state,int zone);
        /**
         * Cerca la chiave data in tutte le zone
         * @param keySN numero di chiave da cercare
         * @return vettore di zone a cui la chiave appartiene
         */
        int FindKeyInZones(CString keySN, int* zoneList, int maxZones);
        
        /**
         * Controlla se la chiave data esiste nella zone indicata
         * @param keySN chiave da cercare
         * @param zoneIndex zona in cui cercare
         * @return true se la chiave è trovata
         */
        bool CheckKeyInZone(CString keySN, int zoneIndex);

        /**
         * Cancella tutte le chiavi dalla zona data
         * @param zoneIndex indice della zona, se -1 le chiavi sono cancellate da tutte le zone
         * @return true se tutto ok
         */
        bool EraseZoneKeysFromFile(int zoneNumber = -1);

        /**
         * Cancella la chiave data da una specifica zona o da tutte le zone
         * @param keySN numero chiave
         * @param zoneIndex numero della zona o -1 se da tutte le zone a cui la chiave appartiene
         * @return true se tutto ok
         */
        bool EraseKeyFromFile(CString keySN, int zoneIndex = -1);

        bool WriteZoneKeysOnFile(int zoneIndex);

        /**
         * Data la zona estrae i limiti di partenza e arrivo per eseguire i cicli
         * (Visot che lo devo fare un po' di volte conviene)
         * @param zone indice della zone
         * @return true
         */
        bool ExtractCycleLimits(int zone, int* startIndex, int* stopIndex);

        bool ProgramAccessDevice(vector< t_Zone >::iterator zoneIt, vector< t_AccController >::iterator accIt);

        bool RemoveKeyFromDevice(vector< t_AccessData >::iterator keyIt, vector< t_AccController >::iterator accIt);

        void ResetZone(t_Zone *zone);
        void ResetAccessData(t_AccessData *accessData);
        
        void CheckInputActivation(int zoneIndex);
        
        /**
         * Invia i comandi che appartengono alla zona di allarme
         * @param zoneIndex indice della zona
         */
        void SendCommands(int zoneIndex, bool alarmState);

};


#endif
