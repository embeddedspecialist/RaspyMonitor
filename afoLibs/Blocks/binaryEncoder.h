/* 
 * File:   binaryEncoder.h
 * Author: amirrix
 *
 * Created on 18 gennaio 2010, 18.10
 *
 * Encoder/decoder binario: la configurazione degli ingressi e' trasformata in un numero in uscita
 * e viceversa. Gestisce fino a 4 in(out)
 *
 * NAME:BINARYENCDEC,TYPE,INPUTx,OUTPUTX,ADDR,COMMENT
 * Dove:
 * INPUT - da 1 a 4 se encoder, 1 se decoder
 * OUTPUT - 1 se encoder, da 1 a 4 se decoder
 * TYPE: ENC o DEC
 */

#ifndef _BINARYENCODER_H
#define	_BINARYENCODER_H

#include "BlocksCommonData.h"
#include "block.h"


class CBinaryEncDec : public CBlock {
public:
    CBinaryEncDec(const char* configString);

    bool Update( );
    
    virtual ~CBinaryEncDec();
private:

    e_BINARYENCDEC_Type m_Type;

};

#endif	/* _BINARYENCODER_H */

