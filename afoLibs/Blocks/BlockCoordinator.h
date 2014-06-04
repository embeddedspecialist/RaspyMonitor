/***************************************************************************
 *   Copyright (C) 2007 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 ***************************************************************************/

#ifndef __BLOCKCOORD_H__
#define __BLOCKCOORD_H__

#include <iostream>
#include <pthread.h>
#include <vector>
#include <sys/types.h> 
#include <unistd.h> 

#include "cstring.h"
#include "commonDefinitions.h"
#include "ownet.h"
#include "vcoordinator.h"
#include "block.h"
#include "pidBlock.h"
#include "ifblock.h"
#include "opblock.h"
#include "logblock.h"
#include "satblock.h"
#include "hystblock.h"
#include "contblock.h"
#include "costantblock.h"
#include "gateblock.h"
#include "timerblock.h"
#include "muxBlock.h"
#include "delayBlock.h"
#include "C3PointCtrlBlock.h"
#include "ClimaticCurveBlock.h"


#include "BlocksCommonData.h"

#define CHANNEL_SEP '-'

//TODO implementare il modo da generare drettamente delle strutture t_Error nei throw



using namespace std;

typedef struct
{
    vector<CBlock*> blockList;
    unsigned int updateTime;
    unsigned long int lastUpdateTime;
    bool hasHSInput;
    bool hasHSOutput;
} t_SubSystemData;

/**
 Classe di gestione della logica liberamente programmabile di afomonitor
 *Parametro inifile: NAME:BlockCoord,INIFILE:nomefile,COMMENT:
    @author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CBlockCoordinator : public CVCoordinator {
public:
    CBlockCoordinator(const char *configString, CTimer *timer);

    ~CBlockCoordinator();
    
    //TODO manca funzione per decidere l'ordine di esecuzione dei blocchi!!!
    //Si potrebbe riordinare il vettore dei blocchi in modo da averlo naturalmente ordinato con la progressione
    //TODO manca la funzione che collega gli ingressi ai blocchi
    
    void Run();
    
    bool InitSystem();
    
    bool ControlSystem();

    bool Update(bool updateData){Run();return true;};

    bool Update2(bool updateData){return false;};

    bool SetVal(float newVal){return false;};

    CString GetSpontaneousData(int) {return "<ERROR TYPE=\"MESSAGE NOT IMPLEMENTED\" />";};
    bool ConnectControllers(){return InitSystem();};

    bool ExecCommand(CXMLUtil* xmlUtil);
    
private:
    int m_RunLevel;
    CString m_IniFileName;
    
    vector<t_SubSystemData> m_SubSystemList;
    
    /**
     * Loads all the informations about the subsystems defined in the ini file and creates them
     * @return 1 on success, 0 if there are no subsystems, -1 on error
     */
    int LoadSubSystemData();

    bool SetupSubsystems();
    
    bool CreateBlock(const char *configString, t_SubSystemData *subSystemData);
    /**
     * Funzione che connette gli ingressi dei blocchi e ordina nel vettore dei blocchi gli stessi in funzione dell'ordine di esecuzione
     * Man mano che i blocchi sono connessi viene controllato se il blocco di ingresso di un dato blocco viene prima nel vettore o meno.
     * In caso venga prima i due vengono swappati. Al termine della connessione viene controllato se si è verificato almeno uno swap e 
     * nel caso viene ripetuta la connessione partendo dal nuovo ordine e swappando quando necessario. L'algoritmo termina quando non ci sono più
     * elementi da swappare
     * @param subSystemIndex Indice del sottosistema a cui appartengono i blocchi da ordinare e connettere
     * @return true se l'operazione e' andata a buon fine.
     */
    bool ConnectBlocksInputs( int subSystemIndex );
    
    bool RunSubSystem(vector<t_SubSystemData>::iterator subSysIt);
    
};


#endif
