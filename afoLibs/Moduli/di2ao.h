/***************************************************************************
*   Copyright (C) 2007 by Alessandro Mirri                                *
*   alessandro.mirri@newtohm.it                                           *
*                                                                         *
*   This program is NOT free software; you can NOT redistribute it and/or *
*   modify it in any way without the authorization of the author          *
*                                                                         *
*   This program is distributed WITHOUT ANY WARRANTY;                     *
*   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
*                                                                         *
***************************************************************************/
#ifndef DI2AO_H
#define DI2AO_H
        
#include "vcontroller.h"
#include "timer.h"
#include "digitalio.h"
#include "analogIO.h"
#include "cstring.h"
        
#define CONF_SEP '*'
#define OUT_SEP '&'     
#define OUTVAL_SEP '-'
           
/**
Classe che correla uno o più ingressi digitali con una o più uscite analogiche (Implementata per S.Arcangelo Coop)
Configurazione:
 * NAME,ADDR,NIN,NOUT,IN1,IN1CH,IN2,IN2CH,IN3,IN3CH,...,INVERT,OUT1,OUT2,OUT3...,TIMERID:,NFUN,FUN1,FUN2,...
 * Dove:
 * INVERT = rappresenta la maschera per l'inversione degli input digitali: il bit più significativo corrisponde all'ingresso di indice maggiore
 * NIN = Numero Ingressi
 * NOUT = Numero Uscite
 * NFUN = Numero funzioni di correlazione
 * FUNXX = Funzione di correlazione, il formato è il seguente: configurazione ingressi-uscita da attivare.valore
 * Es. FUN1:8-1;255 Indica che l'ingresso 4 (2^3) deve attivare l'uscita 1 al valore 255
 * Es2. FUN2:12-1.2.3;0 Indica che se sono attivi gli ingressi 4 e 3 (2^3+2^2) le uscite 1, 2 e 3 si portano al valore 0
 * Attenzione: l'uscita di indice maggiore rappresenta il bit più significativo della configurazione indicata nelle funzioni
	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/

typedef struct
{
    int configuration;
    vector<int> outputIDXVector;
    int output;
    
} t_Function;

class CDI2AO : public CVController 
{
    public:
        CDI2AO(const char* configString, CTimer *timer);
    
        ~CDI2AO();

        bool SetVal(float val){return false;};
        //TODO da implementare
        bool VerifyIOPresence(){return true;};
        bool Update(bool updateData);
        bool Update2(bool updateData){return false;};
        CString GetSpontaneousData(int lParam = 0) { return "<ERROR TYPE=\"Function NOT IMPLEMENTED\" />";};
        
        vector<CDigitalIO*> m_InVector;
        vector<CAnalogIO*> m_OutVector;
        
        vector<t_Function> m_FunctionVector;
        
    private:

        bool m_ParseOK;
        
        bool ParseFunctions(const char* configString);
        
        bool CreateDigital(const char* configString);
        
        bool CreateAnalog(const char* configString);

};

#endif
