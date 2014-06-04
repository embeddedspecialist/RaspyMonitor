/* 
 * File:   C3PointCtrlBlock.h
 * Author: amirrix
 *
 * Created on 19 gennaio 2010, 17.44
 *
 * Questo blocco incapsula un controllo 3 punti.
 * Il controllo puo' essere LMD o normale a seconda degli ingre4ssi collegati.
 *
 *
 * Sintassi:
 * NAME:C3POINT, ADDR:, INPUTX:, OUTPUTX,SUMMER:,SETPOINT:,HYST:,MOVETIME:,LIMH,LIML,,MFACTOR1,QFACTOR1,MFACTOR2,QFACTOR2,COMMENT:
 * Dove:
 * INPUTx -- Ingresso:
 *           1 - Variabile di controllo
 *           2 - Variabile limite (fac.)
 *           3 - Setpoint (fac.)
 *           4 - Summer (fac.)
 * OUTPUTX -- Uscite
 *           1 -- Comando apertura
 *           2 -- Comando chiusura
 *           3 -- FullOpen
 *           4 -- FullClosed
 * 
 * COMANDI:
 * SetParameters -- Imposta i parametri a seconda dei sottocampi Come da file ini
 * GetParameters -- ritorna i parametri <BLOCK BLOCKEXEC="C3PointParameters" ADDRESS="" parametri come da ini file/>
 */

#ifndef _C3POINTCTRLBLOCK_H
#define	_C3POINTCTRLBLOCK_H

#include "block.h"
#include "c3pointctrl.h"

#define C3P_DIRECT_IN   0
#define C3P_LMD_IN      1
#define C3P_SETPOINT_IN 2
#define C3P_SUMMER_IN   3

#define C3P_OPEN 0
#define C3P_CLOSE 1

extern ClockManager mainClockManager;

class C3PointCtrlBlock : public CBlock {
public:
    C3PointCtrlBlock(const char* configString);
    
    virtual ~C3PointCtrlBlock();

    bool Update();

    bool ExecCommand(CXMLUtil *xmlUtil, bool *commandRetCode);
    
private:
    float m_NullZone;
    float m_MoveTimeOut;
    float m_Setpoint, m_SetpointH, m_SetpointL;
    bool m_IsSummer;
    clock_t m_TimeOfOpening, m_TimeOfClosing;

    float m_NormalizeM1, m_NormalizeQ1, m_NormalizeM2, m_NormalizeQ2;

    int m_ClockIdx;

    bool UpdateSimpleControl();
    bool UpdateLMDControl();

    void GetSummer();
    void GetSetpoint();

    bool Open();
    bool Close();
    bool Stop();

    float GetError(float temp, float setpoint, bool isLmd);

};

#endif	/* _C3POINTCTRLBLOCK_H */

