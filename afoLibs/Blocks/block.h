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

#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "BlocksCommonData.h"
#include "cstring.h"
#include "commonDefinitions.h"
#include "inifilemanager.h"
#include "conewirenet.h"

#ifndef STDBLOCK_H
#define STDBLOCK_H

#define MAX_NUM_OUTPUT 32
#define MAX_NUM_INPUT 32

using namespace std;

typedef struct _blockOutputData
{
    float outValue; //valore usato in uscita dal blocco
    vector<CVController*> outControllerList;   // Controller di uscita se l'output va fuori
    bool isValid;  //flag che indica se il valore è valido o meno
    unsigned long int timeOfData;
    inline void Init() {outValue = 0.0; isValid = false; timeOfData = 0;};
    _blockOutputData(){Init();};
}t_BlockOutputData;


/**
Classe virtuale padre di tutti i blocchetti funzionali. Espone un'interfaccia comune a tutti quanti.
 * Ogni blocco ha definiti uno o più ingressi (parametro INPUTx), una (e in futuro piu') uscita
 * (parametro OUTPUTx) e da 0 a N ingressi di comando dipendenti dal blocco stesso (COMMANDx).
 *
 * INPUT
 * Gli ingressi sono identificati dal prefisso:
 * - A -- corrisponde ad un dato disponibile a sistema da un controllore avente l'indirizzo che segue la A
 * - B -- il dato è fornito dall'uscita di un altro blocco (in futuro si potra' scegliere quale uscita separandola con il segno meno
 * - N -- Non connesso: serve per alcuni blocchi che devono avere un numero di ingressi fisso anche se non li usano tutti (es. MUX)

 * 23/10/2007 -- Aggiunte le funzioni per gestire il tempo di acquisizione dei dati
 * 
    @author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CBlock{
public:
    CBlock(const char *configString);

    virtual ~CBlock() = 0;

    e_BlockType GetBlockType() const
    {
      return m_BlockType;
    }
    
    /**
     * Consente di collegare un "canale di ingresso" a quello di uscita di un altro blocco
     * @param newInput Blocco da cui prendere l'uscita 0 0x0 se è un indirizzo di memoria
     * @param newChannel Canale del blocco
     * @param newAddress Indirizzo di memoria
     * @return true se il nuovo canale è correttamente inserito nel blocco, false su errore
     */
    bool SetInputChannel (CBlock* newInput, int newChannel = 0, CVController *newController = 0x0);

    void ResetInputChannels();

    bool SetCommandChannel( CBlock * newInput, int newChannel, CVController* newController = 0x0 );
    
    /**
     * Legge il valore memorizzato in un canale di uscita. Viene chiamata dai blocchi per aggiornare i propri ingressi
     * @param destination valore di destinazione
     * @param outIndex indice del canale
     * @return true se in *destination è memorizzato il valore corretto
     */
    bool GetOutputData (float *destination, int outIndex = 0);
    
    /**
     * Legge la validita' ed il tempo dei dati memorizzati all'interno di un canale
     * @param timeOfData istante di tempo a cui si riferiscono o dati
     * @param outIndex indice del canale
     * @return true se il dato è valido, false altrimenti
     */
    bool GetOutputValidityAndTime(unsigned long int *timeOfData, int outIndex = 0);

    
    virtual bool Update () = 0;

    /**
     * Imposta i valori di SubSystem e Indice utilizzati nel file di configurazione asll'interno del dispositivo
     * @param subSystemIdx Indice del Subsystem usato nel file di configurazione
     * @param blockIdx Indice del blocco
     */
    void SetConfigIndexes(int subSystemIdx, int blockIdx);
    
    /**
     * @return ritorna il valore del subsystem del file di configurazione a cui questo blocco appartiene
     */
    int GetConfigSubSystemIdx(){ return m_ConfigSubSystemIndex;};
    
    /**
     * @return L'indice del blocco all'interno della sezione subsystem del file di configurazione
     */
    int GetConfigBlockIdx() {return m_ConfigBlockIndex;};

    int GetBlockAddress(){ return m_BlockAddress;};
    
    bool SetupBlock(void *netPtr, void *engPtr);

    void SetDebugLevel(int newDebugLevel){m_DebugLevel = newDebugLevel;};

    void PrintBlockInfo();
    virtual void SendBlockInfo();

    /**
     * Esegue un comando
     * ritorna true se il comando era per lei e mette il codice di esecuzione in commandRetCode
     * */
    virtual bool ExecCommand(CXMLUtil *xmlUtil, bool *commandRetCode){*commandRetCode = false;return false;};
    
protected:
    CString m_ConfigString;
     //Fa un po' schifo ma l'ho visto fare in altri esempi di classi.
    typedef struct _blockInputData
    {
        CBlock *sourceBlock;            //Se l'ingresso e' controller sourceBlock e' uguale a 0x0
        CVController* sourceController;        //Se l'ingresso è un blocco questo è pari a 0x0
        int sourceChannel;
        unsigned long int timeOfData; //Secondi dell'ultima lettura dell'ingresso
        inline void Init() { sourceBlock = 0x0; sourceChannel = -1; sourceController = 0x0; timeOfData = 0;};
        _blockInputData(){ Init();};
    }t_BlockInputData;
    
    //Indirizzo del blocco in memoria, serve per identificarlo e potergli inviare dei comandi/variazioni di parametri
    int m_BlockAddress;
    //Indice del sottosistema a cui appartiene come da file di configurazione
    int m_ConfigSubSystemIndex;
    //Indice del blocco come da file di configurazione
    int m_ConfigBlockIndex;

    int m_DebugLevel;
    
    //Gestore file ini
    CIniFileManager m_LibIniReader;

    //Ingressi dai dati al blocco
    vector<t_BlockInputData> m_InputVector;
    //Uscite del blocco
    vector<t_BlockOutputData> m_OutputVector;
    //Ingressi di comando del blocco
    vector<t_BlockInputData> m_CommandInputVector;
    
    e_BlockType m_BlockType;
    
    CString m_Comment;

    void *m_NetPtr, *m_EngPtr;
    
    
    
    /**
     * Legge i dati di ingresso, sia che provengano da un blocco che da una cella di memoria, confrontado sia il flag di validità che quello 
     * temporale.
     * @param inputIndex Indice dell'ingresso da leggere
     * @param dest destinazione in cui memorizzare il valore, nel caso in cui si verifichino errori viene impostato a -100.0
     * @return 1 -- dato Valido e aggiornato, 0 -- Dato non aggiornato, -1 -- dato NON valido, -2 -- Errore
     */
    int GetDataFromInput(int inputIndex, float *dest);

    int GetDataFromCommand( int commandIndex, float *dest );
    
    /**
     * Checks if the value associated to an input channel is valid or not
     * @param inputIndex index of the input channel to test
     * @return true if value valid, false otherwise
     */
    bool IsInputValid(int inputIndex);

    /**
     * Verifica che l'ingresso corrispondente all'indice sia connesso a qualcosa
     * */
    bool IsInputConnected(unsigned int inputIndex);

    bool IsCommandValid( int commandIndex );
    
    /**
     * Imposta il valore di un canale di uscita
     * @param outputIndex indice del canale di uscita
     * @param value valore del canale
     * @param isValid validità del valore
     * @return true se l'aggiornamento è stato eseguito in maniera corretta
     */
    bool SetOutputVal (int outputIndex, float value, bool isValid = true);

    /**
     * Verifica se il comando e' per questo bloc
     * */
    bool IsCommandForMe(CXMLUtil *com);

    bool UpdateIniFile ( int netIndex, int devIndex, CString subKey, CString newVal );

    /*
     * Questa funzione controlla se nel messaggio inviato esiste l'argomento dato e se
     * esiste lo legge e lo copia nella destinazione. Se saveArgument e' true
     * il valore viene salvato nel file ini
     * */
    bool ParseCommandArgument(CXMLUtil* command, CString argument, float* destination, bool saveArgument = true);

    
};



#endif
