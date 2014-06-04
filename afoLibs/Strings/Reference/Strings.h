#ifndef __STRINGS_H_
#define __STRINGS_H_

#include <string>
#include <list>

#include <stdarg.h>

using namespace std;

class CString:public string
{
	typedef		string::allocator_type		atype;

public:
	CString::CString(const char *s, const atype& al = atype()):
		string(s, al){};
	explicit CString::CString(const atype& al = atype()):
		string(al){};
	CString::CString(const_iterator first, const_iterator last,
		const atype& al = atype()):
		string(first, last, al){};
	CString::CString(size_type n, char c, const atype& al = atype()):
		string(n, c, al){};
	CString::CString(const string& rhs):
		string(rhs){};
	CString::CString(const char *s, size_type n, const atype& al = atype()):
		string(s, n, al){};

	int countItems(char sep)
	{
		size_type pos = 0;
		int		  count = 0;

		while((pos = find(sep,pos)) != string::npos)
		{
			count++;
			pos++;
		}

		return count;
	};

// 	CString& lcase()
// 	{
// 		terminate();
// 		if (empty()) return *this;
// 		
// 		_strlwr(&at(0));
// 		return  *this;
// 	};
// 
// 	CString& ucase()
// 	{
// 		terminate();
// 		if (empty()) return *this;
// 		
// 		_strupr(&at(0));
// 		return *this;
// 	};

	void terminate()
	{
		if (size() == 0) return;
		if (empty()) return;

		if (at(size()-1) != 0)
			append(1, 0);
	};

	void str(double d, char* format = "%5.3f")
	{
		char	strtmp[64];

#ifdef _WINDOWS
		_snprintf(strtmp, 63, format, d);
#else
        snprintf(strtmp, 63, format, d);
#endif

		*this = strtmp;
	}

	void str(int i, char* format = "%d")
	{
		char	strtmp[64];

#ifdef _WINDOWS
		_snprintf(strtmp, 63, format, i);
#else
        snprintf(strtmp, 63, format, i);
#endif

		*this = strtmp;
	}

	void addfmt(char* format, ...)
	{
		char	strtmp[128];

		va_list args;
		va_start(args, format);

#ifdef _WINDOWS
		_vsnprintf( strtmp, 127, format, args );
#else
        vsnprintf( strtmp, 127, format, args );
#endif

		va_end(args);

		*this += strtmp;
	}

	void nfmt(int maxChar, char* format, ...)
	{
		char*	strtmp = (char*)malloc(maxChar+1);

		va_list args;
		va_start(args, format);

#ifdef _WINDOWS
		_vsnprintf( strtmp, maxChar, format, args );
#else
        vsnprintf( strtmp, maxChar, format, args );
#endif

		va_end(args);

		*this = strtmp;

		free (strtmp);
	}

	void fmt(char* format, ...)
	{
		char	strtmp[256];

		va_list args;
		va_start(args, format);

#ifdef _WINDOWS
		_vsnprintf( strtmp, 256, format, args );
#else
        vsnprintf( strtmp, 256, format, args );
#endif
 
		va_end(args);

		*this = strtmp;
	}

	CString tok(int nTok, char* sep)
	{
		int pos = 0;
		int end = 0;

		if(empty()) return substr(0,0);

		terminate();
		char* c_string = &at(0);
		
		for (int i=0; i<(nTok+1); i++)
		{
			// Look for the next separator
			pos = strspn(&c_string[end], sep) + end;
			end = strcspn(&c_string[pos], sep) + pos;
		}

		if (end - pos <= 0) return substr(0,0);

		return substr(pos, end-pos);
	}

	CString tokaft(int nTok, char* sep)
	{
		int pos = 0;
		int end = 0;

		if(empty()) return substr(0,0);

		terminate();
		char* c_string = &at(0);
		
		for (int i=0; i<(nTok+1); i++)
		{
			// Look for the next separator
			pos = strspn(&c_string[end], sep) + end;
			end = strcspn(&c_string[pos], sep) + pos;
		}

		if (end - pos <= 0) return substr(0,0);

		return substr(pos);
	}

	int ntok(char* sep)
	{
		int pos = -1;
		int end = 0;
		int i = 0;

		if(empty()) return 0;

		terminate();
		char* c_string = &at(0);
		
		while(end != pos)
		{
			// Look for the next separator
			pos = strspn(&c_string[end], sep) + end;
			end = strcspn(&c_string[pos], sep) + pos;
			i++;
		}

		i--;
		return i;
	}

	bool operator== (CString& str)
	{
		int sz = size();
		if (sz != str.size()) return false;
		if (sz == 0) return true;
		if (memcmp(c_str(), str.c_str(), sz) == 0)
			return true;
		return false;
	};

	bool operator!= (CString& str)
	{
		if (size() == 0) return true;
		if (str.size() == 0) return true;
		if (strcmp(c_str(), str.c_str()) != 0)
			return true;
		return false;
	};

	bool operator== (const char* str)
	{
		if (str == NULL) return false;
		int sz = strlen(c_str());
		int sz2 = strlen(str);
		if (sz != sz2) return false;
		if (sz == 0) return true;
		if (memcmp(c_str(), str, sz) == 0)
			return true;
		return false;
	};

	bool operator!= (const char* str)
	{
		if (str == NULL) return false;
		int sz = strlen(c_str());
		int sz2 = strlen(str);
		if (sz != sz2) return false;
		if (sz == 0) return true;
		if (memcmp(c_str(), str, sz) != 0)
			return true;
		return false;
	};


// 	_Myt& operator+=(const _Myt& _X)
// 	{
// 		int pos = find_last_not_of((char)0);
// 		if (pos != CString::npos) resize(pos+1);
// 		return (append(_X)); 
// 	}
// 	
// 	_Myt& operator+=(const char *_S)
// 	{
// 		int pos = find_last_not_of((char)0);
// 		if (pos != CString::npos) resize(pos+1);
// 		return (append(_S)); 
// 	}
// 	
// 	_Myt& operator+=(char _C)
// 	{
// 		int pos = find_last_not_of((char)0);
// 		if (pos != CString::npos) resize(pos+1);
// 		return (append(1, _C)); 
// 	}

	void	ltrim(const char* str = " ")
	{
		if (str == NULL) return;
		int postrim= find_first_not_of(str);
		if (postrim > 0)
			*this = substr(postrim);
		if (postrim == -1)
			*this = "";
	};

	void	rtrim(const char* str = " ")
	{
		if (str == NULL) return;
		int postrim= find_last_not_of(str);
		if (postrim != 0)
			*this = substr(0, postrim+1);
	};

	// after trim - eliminate all character after the one in the string
	void	atrim(const char* str = " ")
	{
		if (str == NULL) return;
		int postrim= find_first_of(str);
		*this = substr(0, postrim-1);
	};

	void trim (const char* str = " ")
	{
		ltrim(str);
		rtrim(str);
	};
};


typedef string::size_type		t_strpos;

#endif
