/***************************************************************************
 *   Copyright (C) 2008 by Alessandro Mirri                                *
 *   alessandro.mirri@newtohm.it                                           *
 *                                                                         *
 *   This program is NOT free software; you can NOT redistribute it and/or *
 *   modify it in any way without the authorization of the author          *
 *                                                                         *
 *   This program is distributed WITHOUT ANY WARRANTY;                     *
 *   without even the implied warranty of FITNESS FOR A PARTICULAR PURPOSE *
 *                                                                         *
 *************************************************************************/
#include <map>
#include "cstring.h"

#ifndef __CMD_H__
#define __CMD_H__

using namespace std;

class Cmd {

    public:
        
    CString cmd;
    map<string,string> hm;
    CString add;
    
    /**
     * Costruttore
     * @param cmd Nome del comando
     */
    Cmd(CString cmd);

    ~Cmd();
    
    /**
     * Ritorna il nome del comando
     * @return Comando
     */
    CString getCmd(){
        return cmd;
    };
    
    /**
     * Ritorna il valore di un attributo
     * @param key Attributo
     * @return Valore
     */
    CString getValue(CString key);
    
    /**
     * Imposta il valore di un attributo
     * @param key Attributo
     * @param value Valore
     */
    void putValue(CString key, CString value);
    void putValue(CString key, float value);
    void putValue(CString key, int value);
    void putValue(CString key, unsigned int value);
//    void putValue(CString key, bool value);
    void putValue(const char *key, const char *value);
    
    /**
     * Ritorna true se un attributo � gi� incluso
     * @param key Attributo
     * @return true se presente
     */
    bool containsAttribute(CString key);
    
    /**
     * Ritorna la stringa xml corrispondente a cmd
     * @return CString xml
     */
    CString getXMLValue();
    
    /**
     * Aggiungo una stringa al comando xml
     * @param s Stringa da aggiungere
     */
    void addPart(CString s);
    
   CString toString(){
        return getXMLValue();
    }

};
#endif

