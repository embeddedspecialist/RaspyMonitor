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
 
 #ifndef CPID_H
#define CPID_H

#include <block.h>

#define PID1_INDEX 0
#define PID2_INDEX 1
#define PID3_INDEX 2

#define PID_SP_INPUT 2
#define PID_SUMMER_INPUT 3

#define MAX_PID_OUTPUT 1000.0

/**
 *
 * Implementa un controllo di tipo PID. Il setpoint viene letto da file di configurazione ma
 *può essere passato anche via ad uno degli ingressi (attraverso una costante o un timer).
 * Il PID ha un'uscita tra 0.0 e 1000.0
 * Stringa di configurazione:
 * NAME:PID, TYPE, INPUTX, OUTPUTX, KP, KI, KD, SP, ADDR, SUMMER, LIMH, LIML, MFACTOR1,QFACTOR1,MFACTOR2,QFACTOR2,COMMENT
 * Dove:
 * INPUTX   -- Ingresso 
 *             1 -- La variabile controllata (obbligatorio)
 *             2 -- Se e' collegato il PID e' LMD e questgo e' l'ingresso limite
 *             3 -- Ingresso SetPoint preso dall'esterno (fac.)
 *             4 -- Ingresso Summer (fac.)
 * OUTPUTX  -- Uscita analogica del PID
 * ADDR     -- Indirizzo in memoria
 * COMMENT  -- commento
 * TYPE     -- Tipo PID, puo' essere : 
 *              "PF" -- Punto fisso, 
 *              "LMD" -- PID Limite di mandata, 
 * KP,KI,KD -- Costanti PID
 * KP2,KI2,KD2 -- Costanti PID Limite
 * SP -- Setpoint principale
 * SUMMER -- funzionamento estivo (1) o invernale (0)

 * COMANDI:
 * SetParameters -- Imposta i parametri a seconda dei sottocampi SETPOINT,DELTA,MIN e MAX
 * GetParameters -- ritorna i parametri <BLOCK COMMAND="PIDParameters" ADDRESS="" parametri come da ini file/>
	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CPIDBlock : public CBlock
{
public:
    CPIDBlock(const char* configString);

    ~CPIDBlock();
    
    bool Update();

    void setSummer(bool theValue)
    {
      m_IsSummer = theValue;
    }
    

    bool IsSummer() const
    {
      return m_IsSummer;
    }


    bool ExecCommand(CXMLUtil *xmlUtil, bool *commandRetCode);

    void SendBlockInfo();
    
    private:
        
        bool UpdateSimpleControl();
        bool UpdateLMDControl();
        
        float Integrate(int pidIndex, float value);
        float Derivate(int pidIndex, float value);

        void GetSummer();
        void GetSetpoint();
 
        //!Array containing all the parameters in the following list: 
        //!PID PF, HYST, COMPEXT :Proportional, Integrative and Derivative
        //!For PID LMD the frist 3 values are for the primary PID and the second 3 are for the LMD pid
        vector<float> m_Parameters;
        
        //!Array containing the set points of the system:
        //!PID PF, HYST, COMPEXT : it is the set point to keep
        //!for PID LMD: the first is the setpoint to keep, the second is the upper setpoint (SPH) and the third is the lower setpoint (SPL)
        vector<float> m_SetPoints;
        
        vector<float> m_IntegralState;
        vector<float> m_MaxIntegralError;
        
        vector<float> m_DerivativeState;
        
        float m_normalizeM1, m_normalizeQ1,m_normalizeM2, m_normalizeQ2;

        //!Boolean flag indicating wether we have to invert the error given to the PID to compensate for conditioning
        bool m_IsSummer;
        
};

#endif
