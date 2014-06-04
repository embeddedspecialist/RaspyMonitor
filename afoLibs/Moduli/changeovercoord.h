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
 #ifndef STDCHANGEOVERCOORD_H
#define STDCHANGEOVERCOORD_H

#include <vcoordinator.h>
#include <vcontroller.h>

using namespace std;

typedef enum ActiveSeason
{
    WINTER,
    SUMMER,
    EVER,
    TOT_SEASON
} e_ActiveSeason;

typedef enum IOType
{
    OUTPUT,
    CHOVER,
} e_IOType;

typedef struct {
    CString type;
    CString field;
    CString value;
    int address;
}t_CommandChangeOver;

//Contiene tutte le informazioni per i dispositivi controllati:
//outHandler -- E' un dispositivo di uscita (per change over o per fermarlo, riattivarlo
//inHandler -- E' il dispositivo di input associato all'outHandler (solo changeover)
//activeSeason  -- Indica in quale stagione e' attivo il dispositivo (solo per dispositivi di uscita)
//ctrlType      -- Indica il tipo di dispositivo se per changeover o di sucita generico
//summerSet     --  E' lo stato che deve avere il dispositivo in estate (solo changeover)
//remote        -- Indica se il cambio di stato è remotata o meno (SOLO per uscita, NON per changeover)
typedef struct activeControllers
{
    activeControllers(){outHandler = inHandler = 0x0;activeSeason = WINTER; ctrlType = OUTPUT; summerSet = 0;isRemote=false;remoteAddress=-1;};
    CVController* outHandler;
    CVController* inHandler;
    e_ActiveSeason activeSeason;
    e_IOType ctrlType;
    int summerSet;
    bool isRemote;
    int remoteAddress;
    bool isRemoteDido;
}t_ActiveControllers;



/**
Classe che serve per automatizzare il change over di centrale termica tra estate ed inverno. Si appoggia su un file ini esterno (changeover.ini) che contiene 4 sezioni: COMMON, INPUTS, OUTPUTS, CHOVDEVS che contengono:
COMMON - Impostazione estate/inverno e il delay necessario per il change over
INPUTS - Gli input da controllare durante il change over
OUTPUTS - Gli output da fermare durante il change over e quelli da riattivare alla fine
CHOVDEVS - I dispositivi da utilizzare per il change over

*  * Stringa di configurazione:
* NAME:ChangeoverCoord,ADDR:YY,COMMENT:
*
* ATTENZIONE: Il coordinatore si connette agli ingressi/uscite attraverso gli indirizzi in memoria e funziona in modo "transnet". NON funziona nel caso in cui i dispositivi si riferiscono a net diverse da quelle del coordinatore e queste net sono collegate via wireless
*
	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class ChangeOverCoord : public CVCoordinator
{
public:
    ChangeOverCoord(const char* configString, CTimer *timer);

    ~ChangeOverCoord();

    bool Update(bool updateData);
    bool Update2(bool updateData){return false;};
    bool SetVal(float val){return false;};
    
    CString GetSpontaneousData(int lParam = 0) { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};

    bool ConnectControllers();

    bool InitChangeOver();

    void StartChangeOver ( )
    {
        m_StartChangeOver = true;
    }

    bool GetStartChangeOver() const
    {
        return m_StartChangeOver;
    }

    void SendCommands();

    int m_InputForSummer;
    bool m_OldSummer;
    CDigitalIO* m_InSummer;

    t_CommandChangeOver m_Commands[128];
    int m_NumOfCommands;
    
    bool m_IsSummer;
    vector<t_ActiveControllers> m_ActiveControllers;
    bool m_IsChangeOverStarted;

    private:

        int m_ChangeOverTime;
        int m_ChangeOverStartTime;
        bool m_StartChangeOver;

        bool m_AllStopped;

        bool ConnectIO(e_IOType ioType,  CIniFileHandler* iniFileReader);
        bool StopAll();
        bool Changeover();
        bool StartAll();

};


#endif
