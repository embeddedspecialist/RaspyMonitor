// ioutil.c functions prototypes

#ifndef _IOUTIL_H_
#define _IOUTIL_H_

#include "ownet.h"

#ifdef __cplusplus
extern "C" {
#endif

int  EnterString(char *msg, char *buf, int min, int max);
int  EnterNum(char *msg, int numchars, long *value, long min, long max);
int  EnterHex(char *msg, int numchars, ulong *value);
int  ToHex(char ch);
int  getkeystroke(void);
int  key_abort(void);
void ExitProg(char *msg, int exit_code);
int  getData(uchar *write_buff, int max_len, SMALLINT gethex);
void PrintHex(uchar* buffer, int cnt);
void PrintChars(uchar* buffer, int cnt);
void PrintSerialNum(uchar* buffer);
int ConvertSN2Hex(const char* SNString, uchar* HexOut);
void ConvertSN2Str(char *SNString, uchar *hexROM);

#ifdef __cplusplus
}
#endif

#endif

