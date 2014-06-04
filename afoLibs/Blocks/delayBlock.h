/* 
 * File:   delayBlock.h
 * Author: amirrix
 *
 * Created on 18 gennaio 2010, 20.57
 *
 * Questo blocco applica un ritardo programmabile: ad una transizione 0->1 sull'ingresso start corrisponde
 * dopo un certo tempo in secondi una transizione 0->1 dell'uscita. Il pin di reset
 * riporta a 0 l'uscita e ferma il conteggio del tempo
 *
 * TODO si potrebbe fare che il blocco ricopia sull'uscita l'ingresso con un certo ritardo
 *
 * String di configurazione:
 * NAME:DELAY, TIME,ADDR,INPUTX,OUTPUT1,COMMENT
 * Dove:
 * TIME -- e' la quantita' di tempo del ritardo
 * INPUT1 -- E' il comando di start
 * INPUT2 -- E' il comando di reset
 * INPUT3 -- Consente di precaricare un valore di ritardo (fac.)
 *
 * //TODO comandi, li implemento quando cambio il blocco
 * Comandi:
 * -Reset
 * -SetDelay --
 * -GetDelay --
 */

#include <time.h>
#include "block.h"
#include "ClockManager.h"

extern ClockManager mainClockManager;

#ifndef _DELAYBLOCK_H
#define	_DELAYBLOCK_H

#define DELAY_IN_START 0
#define DELAY_IN_RESET 1
#define DELAY_IN_LOAD 2


class CDelayBlock : public CBlock{
public:
    CDelayBlock(const char* configString);
    
    virtual ~CDelayBlock();

    bool Update();

    bool ExecCommand(CXMLUtil *xmlUtil, bool *commandRetCode);
private:
    typedef enum {
        DELAY_RUNNNING,
        DELAY_STOPPED
    }e_DelayStatus;

    unsigned int m_ClockIdx;
    float m_Delay;

    e_DelayStatus m_Status;


};

#endif	/* _DELAYBLOCK_H */

