#include <string>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <stdarg.h>
#include <vector>

#ifndef STDCSTRING_H
#define STDCSTRING_H

using namespace std;

/**
	@author Alessandro Mirri <alessandro.mirri@newtohm.it>
*/
class CString : public string
{
public:
    CString();
    CString(const char *initString);
    CString(string initString);

//     virtual ~CString();

    //------------ += operator
        /**
     * Append operator.
         */
    inline CString &operator+=(const CString &str)
    {string::append(str); return *this;};

    /**
     * Append operator.
     */
    inline CString &operator+=(char c)
    {add(c); return *this;};

    /**
     * Append operator.
     */
    inline CString &operator+=(const char *str)
    {string::append(str); return *this;};

    /**
     * Append operator.
     */
    inline CString &operator+=(const std::string &str)
    {string::append(str); return *this;};
    
    inline CString &operator+=(int i)
    {append("%d", i); return *this;};

    inline CString &operator+=(unsigned int i)
    {append("%u", i); return *this;};

    inline CString &operator+=(long l)
    {append("%l", l); return *this;};

    inline CString &operator+=(unsigned long l)
    {append("%ul", l); return *this;};

    inline CString &operator+=(float f)
    {append("%2.1f", f); return *this;};

    inline CString &operator+=(double d)
    {append("%2.1f", d); return *this;};

    inline CString &operator+=(short s)
    {append("%hd", s); return *this;};

    inline CString &operator+=(unsigned short s)
    {append("%hu", s); return *this;};

    //--------- + Operator ---------------
    inline CString &operator+(const CString &str)
    { static CString newStr; newStr = *this; newStr+=str;return newStr;};

    /**
     * Append operator.
     */
    inline CString &operator+(char c)
    { static CString newStr; newStr = *this; newStr+=c;return newStr;};

    /**
     * Append operator.
     */
    inline CString &operator+(const char *str)
    { static CString newStr; newStr = *this; newStr+=str;return newStr;};

    inline CString &operator+(const std::string &str)
    { static CString newStr; newStr = *this; newStr+=str;return newStr;};
    
    inline CString &operator+(int i)
    { static CString newStr; newStr = *this; newStr+=i;return newStr;};

    inline CString &operator+(unsigned int i)
    { static CString newStr; newStr = *this; newStr+=i;return newStr;};

    inline CString &operator+(long l)
    { static CString newStr; newStr = *this; newStr+=l;return newStr;};

    inline CString &operator+(unsigned long l)
    { static CString newStr; newStr = *this; newStr+=l;return newStr;};

    inline CString &operator+(float f)
    { static CString newStr; newStr = *this; newStr+=f;return newStr;};

    inline CString &operator+(double d)
    { static CString newStr; newStr = *this; newStr+=d;return newStr;};

    inline CString &operator+(short s)
    { static CString newStr; newStr = *this; newStr+=s;return newStr;};

    inline CString &operator+(unsigned short s)
    { static CString newStr; newStr = *this; newStr+=s;return newStr;};

    
    CString ToUpper();
    CString ToLower();

//    vector<string>&  Split(const string& str,
//                           vector<string>& tokens,
//                           const string& delimiters = " ");

    void Split(vector<string>& tokens,const string& delimiters = " ");

    protected:
        void append(const char *format, ...);
        void add(char c);
//         void CString::append(const t_Str &str);
//         void CString::append(const char *str, size_t len);
};

#endif
