/*
 *
 * File:   triggerBlock.h
 * Author: amirrix
 *
 * Created on 18 gennaio 2010, 17.28
 *
 * Implementa un trigger in grado di rilevare fronti di salita, discesa, o livelli
 * Stringa di configurazione:
 * NAME:TRIGGER,TYPE,INPUT1,OUTPUT1,ADDR,COMMENT
 * Dove:
 * TYPE indica il tipo e puo' essere: RISE, FALL, LEV_HI, LEV_LOW
 *      - 
 */

#ifndef _TRIGGERBLOCK_H
#define	_TRIGGERBLOCK_H

#include "block.h"

class CTriggerBlock : public CBlock {
public:
    CTriggerBlock(const char* configString);
    bool Update( );
    virtual ~CTriggerBlock();
private:

    float m_LastInput;
    e_TRIGGERType m_Type;

};

#endif	/* _TRIGGERBLOCK_H */

