//
// File:   nthCnt.h
// Author: amirrix
//
// Created on 1 aprile 2008, 19.26
//

#ifndef _NTHCNT_H
#define	_NTHCNT_H

#include <upid2.h>
#include <inttypes.h>
#include "vdevice.h"
#include "timer.h"
#include "ds2751.h"
#include "ownet.h"
#include "temperaturecontroller.h"
#include "analogIO.h"

/*File di configurazione:
NAME,INPUT,ADDR,COMMENT
Dove:
NAME = NTH-CNT

Classe di gestione dei contabilizzatori Newtohm
PROTOCOLLO su ds2751:

*/

typedef enum CounterType
{
  CNT_NONE,
  CNT_WATER,    //Acqua
  CNT_WATT,
  CNT_CAL,
  CNT_NTOT
} e_CounterType;

typedef struct
{
  char countOnlyHeat;             //Conteggia solo il caldo
  int countValue;                 //Numero totale impulsi misurati
  e_CounterType counterType;      //tipo contatore
  int scaleIndex;                 //Fattore di scala, indice del vettore scaleArray
  int isLiterPerImpulse;          //Impulsi/litro(false) o litri/impulso(true)
  float tMnd;                     //Temperatura Mandata
  float tRip;                     //Temperatura Ripresa
  float heatEnergy;                 //Calorie
  float coldEnergy;                 //Frigorie
  char getVolumeOnRip;            //Se 1 il volumetrico � sulla ripresa, altrimenti sulla mandata
} t_Counter;

typedef struct
{
    float val1;   //Valore del contatore o delle calorie se il contatore è di tipo CNT_CAL
    float val2;   //Non usato o valore delle frigorie se il contatore è di tipo CNT_CAL
} t_Valori;

class CNTH_CNT : public CUPID2
{
public:
    CNTH_CNT(const char* configString, CTimer *timer = 0x0);

    ~CNTH_CNT();

    bool Update(bool updateData);
    bool Update2(bool updateData){return false;};

    CString GetSpontaneousData(int lParam = 0) { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};

    /**
     * Function used to initialize the device: it writes all the configuration read from the config.ini file
     * and sets the FLAG_START to 0xFF once initialization is done
     * @return true if initialization performed
     */
    bool InitDevice(CIniFileHandler *iniFileHandler = 0x0);

    /**
     *Ritorna i valori di contabilizzazione memorizzati dal contatore
     *in un array di float nel seguente ordine:
     *Valore della grandezza contata (litri o watt)
     *tempMnd
     *tempRip
     *Calorie
     *Frigorie
     */
    void GetData(int cntIdx, float *destArray);

    /**
     *Legge tutti i contatori e le energie della scheda
     */
    bool ReadCounters();

    //bool GetInternalParameters(int counterIndex, uchar *destArray);

    void setNetPtr ( void* theValue )
    {
        m_NetPtr = theValue;
    }

    void SetSummer ( bool theValue )
    {
        //TODO da mettere il salvataggio su file
        m_IsSummer = theValue;
    }
    

    bool GetSummer() const
    {
        return m_IsSummer;
    }

    void SetEnabled ( bool theValue )
    {
        //TODO da mettere il salvataggio su file
        m_IsEnabled = theValue;
    }
    

    bool GetEnabled() const
    {
        return m_IsEnabled;
    }

    bool GetFarnetoMode() const
    {
        return m_FarnetoMode;
    }
    
    t_Valori m_CounterValue[4];

    t_Counter m_Counters[4];

    /*
     * Function: Clear Counter
     * - nCnt = 0x00 --> Cancella il Counter 0;
     * - nCnt = 0x01 --> Cancella il Counter 1;
     * - nCnt = 0x02 --> Cancella il Counter 2;
     * - nCnt = 0x03 --> Cancella il Counter 3;
     * - nCnt = 0x0F --> Cancella tutti i Counter;
    */
    bool ClearCounter ( unsigned char nCnt );

    bool m_ModuleInitOK;

    private:
        //char EncodeCounterConfig(int cntIdx);
        //void DecodeCounterConfig(int cntIdx, char config);
        
        void ManageFarnetoData();
        float AcquireSetpoint(int index);

        bool m_FirstUpdate;

        bool m_Farneto;
        
        //19-05-2009 -- variabili per funzionamento farneto: gestisco due ritaratori e l'abilitazione/disabilitazione generale
        //i ritaratori danno setpoint e se |temp-setpoint|>isteresi accendo. L'abilitazione generale serve per spegnere completamente
        int m_TempControllerAddr[2], m_AnalogIOAddr[2]; //Indirizzi
        CTempCtrl *m_TempController[2]; //sonde di temperatura
        CAnalogIO *m_AnalogIO[2];       //ritaratori
        bool m_IsOn[2];                 //Stati acceso (true)/spento(false)
        bool m_IsEnabled;               //Abilitazione/disabilitazione
        void *m_NetPtr;                    //The only one net object
        bool m_FarnetoMode;
        bool m_IsSummer;
        float m_Hyst;                   //Isteresi



};

#endif	/* _NTHCNT_H */

