/* 
 * File:   ClockBlock.h
 * Author: amirrix
 *
 * Created on 26 gennaio 2010, 17.13
 *
 * Questo blocco implementa un contatore in secondi, ha un ingresso di Reset
 *
 * Stringa:
 * NAME:CLOCK, INPUT1, COMMON:
 * INPUT1 e' l'ingresso di reset (fac)
 *
 * COMANDI:
 * RESET -- Resetta l'orologio
 */

#include "block.h"
#include "BlocksCommonData.h"
#include "ClockManager.h"
#include "xmlutil.h"

#ifndef _CLOCKBLOCK_H
#define	_CLOCKBLOCK_H

using namespace std;

extern ClockManager mainClockManager;

class ClockBlock : public CBlock {
public:
    ClockBlock(const char* configString);
    ClockBlock(const ClockBlock& orig);
    virtual ~ClockBlock();

    bool Update();

    bool ExecCommand(CXMLUtil *xmlUtil, bool *commandRetCode);
private:

    int m_ClockIdx;

};

#endif	/* _CLOCKBLOCK_H */

