#ifndef FLOORZONE2_H
#define FLOORZONE2_H

#include "vcoordinator.h"
#include "pidsimple.h"
#include "c3pointctrl.h"
#include "temperaturecontroller.h"
#include "climaticcurve.h"

//defines per l'isteresi//
#define HYS_DO_NONE 0
#define HYS_DO_ON 1
#define HYS_DO_OFF -1
//---------------------//

#define UNSET_TEMPMAINT -9999.9

using namespace std;

//I PUNTATORI VALGONO OxO se non trovato.
#define DHT_SEASON_OPTIONS 4
//uso un enum, più efficente che tenermi dentro la stringa!!!
//dichiaro un enum dentro la classe, accessibile come un name-space.
typedef enum DHTSeason { DHT_NEVER = 0, DHT_ALWAYS, DHT_SUMMER, DHT_WINTER, DHT_INVALID } e_DHTSeason; //attenzione: corrispondenza 1:1 con indice dell'array str_DHTSeason
//usare le static: SStr2Enum e SEnum2Str per le conversioni.
typedef enum WaterController {WATER_NONE, WATER_PID, WATER_3POINT} e_WaterCtrl;
typedef struct _st_onoff
{
    bool isOn;
    CDigitalIO* Ctrl;

    inline void init() { memset(this,0,sizeof(_st_onoff)); }
    inline void set(bool On, CDigitalIO* ctrlPtr) { isOn=On; Ctrl=ctrlPtr; }
    inline bool IsCtrl() const { return (Ctrl!=0x0); }

    _st_onoff() { init(); }
    _st_onoff(bool On, CDigitalIO* ctrlPtr) { set(On,ctrlPtr); }
    _st_onoff(const _st_onoff& stRef) { set(stRef.isOn, stRef.Ctrl); }

} t_FloorOnOff;

//Struttura sonde umidità//
typedef struct _st_dehum
{
    unsigned int Num;
    float Level;
    float DeltaT;
    CDigitalIO* Ctrl;

    inline void init() { memset(this,0,sizeof(_st_dehum)); }
    inline void set(unsigned int num, float level, float delta, CDigitalIO*  ctrlPtr) { Num = num; Level = level; DeltaT = delta, Ctrl = ctrlPtr; }
    inline bool IsCtrl() const { return (Ctrl!=0x0); }

    _st_dehum() { init(); }
    _st_dehum(unsigned int num, float level, float delta, CDigitalIO* ctrlPtr) { set(num, level, delta, ctrlPtr); }
    _st_dehum(const _st_dehum& stRef) { set(stRef.Num, stRef.Level, stRef.DeltaT, stRef.Ctrl); }

} t_FloorDeHum;


//Struttura Ritaratore setpoint
typedef struct _st_setpoint
{
    float Value;
    //Questo valore mi serve per i ritaratori relativi: quando ci sono Value contiene il valore fixedValue+ritaratore
    //Altrimenti sono uguali
    float fixedValue;
    float Hysteresis; //isteresi
    CAnalogIO* Ctrl;
    //Famiglia ritaratore di temperatura: 1 = NTH, 0 = ITK
    int tempRegFamily;
    //Tipo ritaratore: assoluto (0) o relativo (1)
    int tempRegType;

    float summerSetpoint;
    float winterSetpoint;

    inline void init() { memset(this,0,sizeof(_st_setpoint)); }
    inline void set(float value, float hys, CAnalogIO* ctrlPtr, float sumSP, float winSP) { Value = value; fixedValue = Value; Hysteresis = hys; Ctrl = ctrlPtr; summerSetpoint= sumSP; winterSetpoint = winSP; tempRegType = 1;tempRegFamily = 1; }
    inline bool IsCtrl() const { return (Ctrl!=0x0); }

    _st_setpoint() { init(); }
    _st_setpoint(float value, float hys, CAnalogIO* ctrlPtr, float sumSP, float winSP) { set( value, hys, ctrlPtr, sumSP, winSP ); }
    _st_setpoint(const _st_setpoint& stRef) { set( stRef.Value, stRef.Hysteresis, stRef.Ctrl, stRef.summerSetpoint, stRef.winterSetpoint ); }

} t_FloorSetPoint;

//struttura maintenimento
typedef struct _st_maintenance
{
    bool isMaintenance;
    float SummerTempValue;
    float WinterTempValue;
    CDigitalIO* Ctrl;

    inline void init() { memset(this,0,sizeof(_st_maintenance)); }
    inline void set(bool isMaint, float sumValue, float winValue, CDigitalIO* ctrlPtr) { isMaintenance = isMaint, SummerTempValue = sumValue; WinterTempValue = winValue; Ctrl = ctrlPtr; }
    inline bool IsCtrl() const { return (Ctrl!=0x0); }
    inline bool IsValue(bool isSummer) const { if (isSummer) return (SummerTempValue!=UNSET_TEMPMAINT); else return (WinterTempValue!=UNSET_TEMPMAINT); }

    _st_maintenance() { init(); }
    _st_maintenance(bool isMaint, float sumValue, float winValue, CDigitalIO* ctrlPtr) { set( isMaint, sumValue, winValue, ctrlPtr ); }
    _st_maintenance(const _st_maintenance& stRef) { set( stRef.isMaintenance, stRef.SummerTempValue, stRef.WinterTempValue, stRef.Ctrl ); }

} t_FloorMaintenance;

//struttura aerotermo
typedef struct _st_aero
{
    int number;
    vector<CDigitalIO*> speedVector;
    vector<float> humiditySPVector;
    vector<float> temperatureDeltaVector;

    //Questa variabile indica qual'e' lo stato delle velocità: serve per l'isteresi sulle umidita'
    int humSpeedState;
    int tempSpeedState;

    e_DHTSeason seasonUsage;

    inline void init() { number = -1;seasonUsage =  DHT_NEVER; speedVector.clear(); humiditySPVector.clear();temperatureDeltaVector.clear(); tempSpeedState = 0; humSpeedState = 0;}
    inline void addHumSP(float newSP) { humiditySPVector.push_back(newSP);};
    inline void addDeltaT(float newDeltaT) { temperatureDeltaVector.push_back(newDeltaT);};
    inline void clearHumidities(){ humiditySPVector.clear();};
    inline void clearTemps(){ temperatureDeltaVector.clear();};
    _st_aero() { init(); }

} t_Aerotherm;

////Struttura che contiene i parametri del regolatore PID//
//typedef struct _st_PID
//{
//    PID pidDirect;
//    PID pidLMDMIN;
//    PID pidLMDMAX;
//    CAnalogIO* pidOutput;
//    inline void init() { pidOutput = 0x0; };
//    _st_PID(){ init(); };
//}t_PID;
//
////Struttura per controllo 3Punti//
//typedef struct _st_3Point
//{
//    CDigitalIO* openDevice;
//    CDigitalIO* closeDevice;
//    float nullZone;
//    float minTemp;
//    float maxTemp;
//    unsigned int movementTimeOut;
//    bool isSummer;
//    float setpoint;
//
//    unsigned int timeOfOpening;
//    unsigned int timeOfClosing;
//
//    inline void init() { openDevice = 0x0; closeDevice = 0x0; timeOfOpening = 0; timeOfClosing = 0;isSummer=true; setpoint = 20.0;};
//    inline float GetError(float temp, float setpoint) { if (isSummer) { return temp - setpoint; } else { return setpoint - temp; }};
//    inline bool OpenValve(){};
//    inline bool CloseValve(){};
//    inline bool StopValve(){};
//
//    _st_3Point() {init();};
//}t_3Point;

//Struttura ZONA//
typedef struct _st_floor_zone
{
    unsigned int zNumber;//numero della zona da file INI:

    //Informazioni modificate in ogni update//
    bool IsTempOn;//VALVOLE e POMPA ON
    bool IsHumOn;//DEHUM ON

    //da verificare sempre che non sianp TEMP_ERRVAL
    float CurrentTemp;
    float CurrentSetPoint;//o mantenimento se si è in questa modalità.
    float CurrentHum;
    float HumHyst;
    //--------------------------------------//

    t_FloorOnOff OnOff;
    t_FloorMaintenance Maintenance;

    vector<t_Aerotherm> Aerotherms;

    vector <CDigitalIO*> Valves; //valvole
    vector <CAnalogIO*> Hum; //sensori umidità
    vector <CTempCtrl*> Temp; //sonde temperatura

    vector <t_FloorDeHum> DeHum; //aerotermi
    e_DHTSeason DHTSeason; //quando utilizzare gli aerotermi anche per controllare la temperatura.

    t_FloorSetPoint SetPoint; //ritaratore setpoint;

    CDigitalIO* pPump;
    CTempCtrl* waterTempProbe;                 //Temperatura dell'acqua
    e_WaterCtrl waterCtrlType;                 //Tipo di controllore acqua
    C3PointCtrl *digitalValveCtrl;                 //Per controllo valvola principale a 3 punti
    CPIDSimple *pPid;                               //Per controllo valvola principale analogica
    CClimaticCurve *climaticCurve;             //Controllore per curva climatica acqua
    float waterSummerSetpoint;                 //Setpoint estivo dell'acqua da mantenere attraverso la valvola
    float waterWinterSetpoint;                 //Setpoint invernale dell'acqua da mantenere attraverso la valvola
    
    int linkedZone;                            //Questo mi serve per "collegare due zone": a livello di engine se ci sono comandi per una zona vengono ripetuti
                                               //sulla zona linkata: NON E' GESTITO A LIVELLO DI COORDINATORE
    
    int timerId;                                //Se voglio posso personalizzare il timer di questa zona

    inline void init()
    { zNumber=0; IsTempOn = false; IsHumOn = false; OnOff.init(); CurrentTemp = TEMP_ERRVAL; CurrentSetPoint = TEMP_ERRVAL; CurrentHum = ANALOG_ERRVAL;
    HumHyst = 4.0; Maintenance.init(); Valves.clear(); DeHum.clear(); Temp.clear(); Hum.clear(); SetPoint.init(); DHTSeason = DHT_INVALID; pPump = 0x0; 
    linkedZone=-1; timerId = -1; waterTempProbe = 0x0; waterSummerSetpoint=0.0; waterWinterSetpoint = 0.0;waterCtrlType = WATER_NONE;}
    //inutile una set..

    _st_floor_zone() { init(); }

} t_FloorZone;

#endif

