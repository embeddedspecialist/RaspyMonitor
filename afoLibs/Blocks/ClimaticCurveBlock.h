/* 
 * File:   ClimaticCurveBlock.h
 * Author: amirrix
 *
 * Created on 4 febbraio 2010, 17.23
 *
 * Questo blocco ha le stesse funzionalità del coordinatore curva climatica.
 * Carica dal file ./climatic.ini i punti della curva e fornisce in uscita
 * il valore del setpoint associato al punto dato.
 *
 * Stringa:
 * NAME:CLIMATICA,ADDRESS,INPUT1,OUTPUT1,CONFIGID,COMMENT
 * INPUT1 -- Ingresso della variabile a cui si associa la climatica
 * OUTPUT1 -- Uscita valore calcolato in basse alla curva
 * CONFIGID -- Identificativo della climatica nel file climatic.ini
 *
 * Comandi:
 * GetClimaticCurve -- Risposta ClimasticCurve
 * SetClimaticCurve -- Sintassi ... NOFPOINTS="1" POINT1="20.0:33.0"
 */

#include "block.h"
#include "BlocksCommonData.h"

#ifndef _CLIMATICCURVEBLOCK_H
#define	_CLIMATICCURVEBLOCK_H

#define CLIMATIC_FILE "./climatic.ini"
typedef struct
{
    float tExt;
    float tMnd;
}t_ClimaticBlockPoint;

class ClimaticCurveBlock :public CBlock {
public:
    ClimaticCurveBlock(const char* configString);
    
    virtual ~ClimaticCurveBlock();

    bool Update();

    bool ExecCommand(CXMLUtil *xmlUtil, bool *commandRetCode);
    
private:

    int m_ConfigID;
    vector<t_ClimaticBlockPoint> m_ClimaticPoints;
    
    void SaveClimaticCurve();
    void LoadPoints();

};

#endif	/* _CLIMATICCURVEBLOCK_H */

