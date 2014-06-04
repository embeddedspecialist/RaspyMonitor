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

/**
 *TODO : da implementare i nuovi modi di funzionamento con gli ingressi
 *TODO : da sistemare con la correzione del limite di mandata inferiore e superiore
 *       come in afomonitor1
 *aggiuntivi fissi
 *
 * Implementa un controllo di temperatura che puo' essere PID o isteresi o con 
 * compensazione esterna. Il setpoint viene letto da file di configurazione ma 
 *può essere passato anche via ad uno degli ingressi (attraverso una costante o un timer). 
 * Stringa di configurazione:
 * NAME:PID, TYPE, INPUTX, OUTPUTX, KP, KI, KD, SP, ADDR, SUMMER, COMMENT
 * Dove:
 * INPUTX   -- Ingresso 
 *             1 -- La variabile controllata (obbligatorio)
 *             2 -- Se LMD è la tamepratura di mandata (fac.)
 *             3 -- Ingresso SetPoint preso dall'esterno (fac.)
 *             4 -- Ingresso Summer (fac.)
 *             5 -- Ingresso delta temperatura dall'esterno per COMPEXT o fascia
 *                  di isteresi se IST (fac.)
 * OUTPUTX  -- Uscita analogica del PID
 * ADDR     -- Indirizzo in memoria
 * COMMENT  -- commento
 * TYPE     -- Tipo PID, puo' essere : 
 *              "PF" -- Punto fisso, 
 *              "LMD" -- PID Limite di mandata, 
 *              "IST" -- Implementa un controll con isteresi (TODO), 
 *              "COMPEXT" -- Compensazione temperatura esterna (TODO) 
 * KP,KI,KD -- Costanti PID
 * SP -- Setpoint principale
 * SUMMER -- funzionamento estivo (1) o invernale (0)
 
	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CPID : public CBlock
{
public:
    CPID(const char* configString);

    ~CPID();
    
    bool Update();

    void setSummer(bool theValue)
    {
      m_IsSummer = theValue;
    }
    

    bool IsSummer() const
    {
      return m_IsSummer;
    }

    e_PIDType GetPIDType() const
    {
      return m_PIDType;
    }
    
    
    
    private:
        
        //!Boolean flag indicating wether we have to invert the error given to the PID to compensate for conditioning
        bool m_IsSummer;
        
        float Integrate(int pidIndex, float value);
        float Derivate(int pidIndex, float value);
 
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
        
        e_PIDType m_PIDType;
        
        float m_normalizeM, m_normalizeQ;
        
        float m_Hysteresis;

};

#endif
