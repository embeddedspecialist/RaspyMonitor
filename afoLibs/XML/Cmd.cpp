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
#include "Cmd.h"

 /**
 * Costruttore
 * @param cmd Nome del comando
 */
Cmd::Cmd(CString cmd)
{
    this->cmd=cmd;
    add = "";
}


Cmd::~Cmd()
{
}


/**
    * Ritorna il valore di un attributo
    * @param key Attributo
    * @return Valore
    */
CString Cmd::getValue(CString key)
{
    map<string, string>::iterator it = hm.find(key);
    CString retVal = "";

    if (it != hm.end())
    {
        retVal = it->second;
    }

    return retVal;
}

/**
    * Imposta il valore di un attributo
    * @param key Attributo
    * @param value Valore
    */
void Cmd::putValue(CString key, CString value){
    map<CString, CString>::value_type tempVal(key, value);
    hm.insert(tempVal);
}

void Cmd::putValue(CString key, float value)
{
    char valToAdd[32];

    sprintf (valToAdd,"%.1f",value);

    putValue(key, (const char*)valToAdd);
}

void Cmd::putValue(CString key, int value)
{
    char valToAdd[32];

    sprintf (valToAdd,"%d",value);
    putValue(key,(const char*)valToAdd);
}

void Cmd::putValue(CString key, unsigned int value)
{
    char valToAdd[32];

    sprintf (valToAdd,"%u",value);
    putValue(key,(const char*)valToAdd);
}

//void Cmd::putValue(CString key, bool value)
//{
//    CString valToAdd;
//
//    if(value){
//        valToAdd = "1";
//    }
//    else
//    {
//        valToAdd="0";
//    }
//    putValue(key, valToAdd);
//}

void Cmd::putValue(const char *key, const char *value)
{
    CString keyStr = key;
    CString valueStr = value;

    putValue(keyStr, valueStr);
}
    

/**
    * Ritorna true se un attributo e' gia' incluso
    * @param key Attributo
    * @return true se presente
    */
bool Cmd::containsAttribute(CString key){
    map<string, string>::iterator it = hm.find(key);

    if (it == hm.end())
    {
        return false;
    }
    else
    {
        return true;
    }
}

/**
    * Ritorna la stringa xml corrispondente a cmd
    * @return CString xml
    */
CString Cmd::getXMLValue(){
    CString s = "<"+cmd+" ";
    map<string, string>::iterator it;

    for (it = hm.begin(); it != hm.end(); it++)
    {
        CString key = it->first;
        s += key+"=\"";
        s += it->second+"\" ";
    }
    s += add+" ";
    s += "/>";
    return s;
}
