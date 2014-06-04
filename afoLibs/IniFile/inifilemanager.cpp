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

#include "inifilemanager.h"

CIniFileManager::CIniFileManager() {
    Clear();
    m_Flags = (AUTOCREATE_SECTIONS | AUTOCREATE_KEYS);
    t_Section newSection;
    m_Sections.push_back(newSection);
}

CIniFileManager::CIniFileManager(t_Str szFileName) {
    t_Section newSection;
    m_bDirty = false;
    m_szFileName = szFileName;
    m_Flags = (AUTOCREATE_SECTIONS | AUTOCREATE_KEYS);
    m_Sections.push_back(newSection);

    Load(m_szFileName);
}

CIniFileManager::~CIniFileManager() {

}

// Clear
// Resets the member variables to their defaults

void CIniFileManager::Clear() {
    m_bDirty = false;
    m_szFileName = t_Str("");
    m_Sections.clear();
}

// SetFileName
// Set's the m_szFileName member variable. For use when creating the CIniFileHandler
// object by hand (-vs- loading it from a file

void CIniFileManager::SetFileName(t_Str szFileName) {
    if (m_szFileName.size() != 0 && CompareNoCase(szFileName, m_szFileName) != 0) {
        m_bDirty = true;

        Report(E_WARN, "[CIniFileManager::SetFileName] The filename has changed from <%s> to <%s>.",
                m_szFileName.c_str(), szFileName.c_str());
    }

    m_szFileName = szFileName;
}

// Load
// Attempts to load in the text file. If successful it will populate the 
// Section list with the key/value pairs found in the file. Note that comments
// are saved so that they can be rewritten to the file later.

bool CIniFileManager::Load(t_Str szFileName) {
    // We dont want to create a new file here.  If it doesn't exist, just
    // return false and report the failure.
    fstream File(szFileName.c_str(), ios::in);

    if (File.is_open()) {
        bool bDone = false;
        bool bAutoKey = (m_Flags & AUTOCREATE_KEYS) == AUTOCREATE_KEYS;
        bool bAutoSec = (m_Flags & AUTOCREATE_SECTIONS) == AUTOCREATE_SECTIONS;

        t_Str szLine;
        t_Str szComment;
        char buffer[MAX_BUFFER_LEN];
        t_Section* pSection = GetSection("");

        //Save file name for later use
        m_szFileName = szFileName;

        // These need to be set, we'll restore the original values later.
        m_Flags |= AUTOCREATE_KEYS;
        m_Flags |= AUTOCREATE_SECTIONS;

        while (!bDone) {
            memset(buffer, 0, MAX_BUFFER_LEN);
            File.getline(buffer, MAX_BUFFER_LEN);

            szLine = buffer;
            Trim(szLine);

            bDone = (File.eof() || File.bad() || File.fail());

            if (szLine.find_first_of(CommentIndicators) == 0) {
                szComment += "\n";
                szComment += szLine;
            } else
                if (szLine.find_first_of('[') == 0) // new section
            {
                szLine.erase(0, 1);
                szLine.erase(szLine.find_last_of(']'), 1);

                CreateSection(szLine, szComment);
                pSection = GetSection(szLine);
                szComment = t_Str("");
            } else
                if (szLine.size() > 0) // we have a key, add this key/value pair
            {
                t_Str szKey = GetNextWord(szLine);
                t_Str szValue = szLine;

                if (szKey.size() > 0 && szValue.size() > 0) {
                    SetValue(szKey, szValue, szComment, pSection->szName);
                    szComment = t_Str("");
                }
            }
        }

        // Restore the original flag values.
        if (!bAutoKey)
            m_Flags &= ~AUTOCREATE_KEYS;

        if (!bAutoSec)
            m_Flags &= ~AUTOCREATE_SECTIONS;
    } else {
        Report(E_INFO, "[CIniFileManager::Load] Unable to open file. Does it exist?");
        return false;
    }

    File.close();

    return true;
}


// Save
// Attempts to save the Section list and keys to the file. Note that if Load
// was never called (the CIniFileHandler object was created manually), then you
// must set the m_szFileName variable before calling save.

bool CIniFileManager::Save() {
    if (KeyCount() == 0 && SectionCount() == 0) {
        // no point in saving
        Report(E_INFO, "[CIniFileManager::Save] Nothing to save.");
        return false;
    }

    if (m_szFileName.size() == 0) {
        Report(E_ERROR, "[CIniFileManager::Save] No filename has been set.");
        return false;
    }

    fstream File(m_szFileName.c_str(), ios::out | ios::trunc);

    if (File.is_open()) {
        SectionItor s_pos;
        KeyItor k_pos;
        t_Section Section;
        t_Key Key;

        for (s_pos = m_Sections.begin(); s_pos != m_Sections.end(); s_pos++) {
            Section = (*s_pos);
            bool bWroteComment = false;

            if (Section.szComment.size() > 0) {
                bWroteComment = true;
                WriteLn(File, "\n%s", CommentStr(Section.szComment).c_str());
            }

            if (Section.szName.size() > 0) {
                WriteLn(File, "%s[%s]",
                        bWroteComment ? "" : "\n",
                        Section.szName.c_str());
            }

            for (k_pos = Section.Keys.begin(); k_pos != Section.Keys.end(); k_pos++) {
                Key = (*k_pos);

                if (Key.szKey.size() > 0 && Key.szValue.size() > 0) {
                    if (Key.szComment.size() > 0)
                    {
                        WriteLn(File, "\n");
                        WriteLn(File,"%s",CommentStr(Key.szComment).c_str());
                        WriteLn(File, "\n");
                    }

                    WriteLn(File, "%s%c%s", Key.szKey.c_str(),EqualIndicators[0],
                    Key.szValue.c_str());
                }
            }
        }

    } else {
        Report(E_ERROR, "[CIniFileManager::Save] Unable to save file.");
        return false;
    }

    m_bDirty = false;

    File.flush();
    File.close();

    return true;
}

// SetKeyComment
// Set the comment of a given key. Returns true if the key is not found.

bool CIniFileManager::SetKeyComment(t_Str szKey, t_Str szComment, t_Str szSection) {
    KeyItor k_pos;
    t_Section* pSection;

    if ((pSection = GetSection(szSection)) == NULL)
        return false;

    for (k_pos = pSection->Keys.begin(); k_pos != pSection->Keys.end(); k_pos++) {
        if (CompareNoCase((*k_pos).szKey, szKey) == 0) {
            (*k_pos).szComment = szComment;
            m_bDirty = true;
            return true;
        }
    }

    return false;

}

// SetSectionComment
// Set the comment for a given section. Returns false if the section
// was not found.

bool CIniFileManager::SetSectionComment(t_Str szSection, t_Str szComment) {
    SectionItor s_pos;

    for (s_pos = m_Sections.begin(); s_pos != m_Sections.end(); s_pos++) {
        if (CompareNoCase((*s_pos).szName, szSection) == 0) {
            (*s_pos).szComment = szComment;
            m_bDirty = true;
            return true;
        }
    }

    return false;
}


// SetValue
// Given a key, a value and a section, this function will attempt to locate the
// Key within the given section, and if it finds it, change the keys value to
// the new value. If it does not locate the key, it will create a new key with
// the proper value and place it in the section requested.

bool CIniFileManager::SetValue(t_Str szKey, t_Str szValue, t_Str szComment, t_Str szSection) {
    t_Key* pKey = GetKey(szKey, szSection);
    t_Section* pSection = GetSection(szSection);

    if (pSection == NULL) {
        if (!(m_Flags & AUTOCREATE_SECTIONS) || !CreateSection(szSection, ""))
            return false;

        pSection = GetSection(szSection);
    }

    // Sanity check...
    if (pSection == NULL)
        return false;

    // if the key does not exist in that section, and the value passed 
    // is not t_Str("") then add the new key.
    if (pKey == NULL && szValue.size() > 0 && (m_Flags & AUTOCREATE_KEYS)) {
        pKey = new t_Key;

        pKey->szKey = szKey;
        pKey->szValue = szValue;
        pKey->szComment = szComment;

        m_bDirty = true;

        pSection->Keys.push_back(*pKey);

        delete pKey;

        return true;
    }

    if (pKey != NULL) {
        pKey->szValue = szValue;
        pKey->szComment = szComment;

        m_bDirty = true;

        return true;
    }

    return false;
}

// SetFloat
// Passes the given float to SetValue as a t_Str

bool CIniFileManager::SetFloat(t_Str szKey, float fValue, t_Str szComment, t_Str szSection) {
    char szStr[64];

    snprintf(szStr, 64, "%f", fValue);

    return SetValue(szKey, szStr, szComment, szSection);
}

// SetInt
// Passes the given int to SetValue as a t_Str

bool CIniFileManager::SetInt(t_Str szKey, int nValue, t_Str szComment, t_Str szSection) {
    char szStr[64];

    snprintf(szStr, 64, "%d", nValue);

    return SetValue(szKey, szStr, szComment, szSection);

}

// SetBool
// Passes the given bool to SetValue as a t_Str

bool CIniFileManager::SetBool(t_Str szKey, bool bValue, t_Str szComment, t_Str szSection) {
    t_Str szValue = bValue ? "True" : "False";

    return SetValue(szKey, szValue, szComment, szSection);
}

// GetValue
// Returns the key value as a t_Str object. A return value of
// t_Str("") indicates that the key could not be found.

t_Str CIniFileManager::GetValue(t_Str szKey, t_Str szSection) {
    t_Key* pKey = GetKey(szKey, szSection);

    return (pKey == NULL) ? t_Str("") : pKey->szValue;
}

// GetString
// Returns the key value as a t_Str object. A return value of
// t_Str("") indicates that the key could not be found.

t_Str CIniFileManager::GetString(t_Str szKey, t_Str szSection, t_Str defVal) {
    t_Str retString = GetValue(szKey, szSection);

    if (retString.size() == 0) {
        return defVal;
    } else {
        return retString;
    }

}

// GetFloat
// Returns the key value as a float type. Returns FLT_MIN if the key is
// not found.

float CIniFileManager::GetFloat(t_Str szKey, t_Str szSection, float defVal) {
    t_Str szValue = GetValue(szKey, szSection);

    if (szValue.size() == 0) {
        return defVal;
    } else {
        return (float) atof(szValue.c_str());
    }
}

// GetInt
// Returns the key value as an integer type. Returns INT_MIN if the key is
// not found.

int CIniFileManager::GetInt(t_Str szKey, t_Str szSection, int defVal) {
    t_Str szValue = GetValue(szKey, szSection);

    if (szValue.size() == 0) {
        return defVal;
    } else {
        return atoi(szValue.c_str());
    }
}

// GetBool
// Returns the key value as a bool type. Returns false if the key is
// not found.

bool CIniFileManager::GetBool(t_Str szKey, t_Str szSection, bool defVal) {
    bool bValue = false;
    t_Str szValue = GetValue(szKey, szSection);

    if (szValue.size() == 0) {
        return defVal;
    }

    if (szValue.find("1") == 0
            || CompareNoCase(szValue, "true") == 0
            || CompareNoCase(szValue, "yes") == 0) {
        bValue = true;
    }

    return bValue;
}

// DeleteSection
// Delete a specific section. Returns false if the section cannot be 
// found or true when sucessfully deleted.

bool CIniFileManager::DeleteSection(t_Str szSection) {
    SectionItor s_pos;

    for (s_pos = m_Sections.begin(); s_pos != m_Sections.end(); s_pos++) {
        if (CompareNoCase((*s_pos).szName, szSection) == 0) {
            m_Sections.erase(s_pos);
            return true;
        }
    }

    return false;
}

// DeleteKey
// Delete a specific key in a specific section. Returns false if the key
// cannot be found or true when sucessfully deleted.

bool CIniFileManager::DeleteKey(t_Str szKey, t_Str szFromSection) {
    KeyItor k_pos;
    t_Section* pSection;

    if ((pSection = GetSection(szFromSection)) == NULL)
        return false;

    for (k_pos = pSection->Keys.begin(); k_pos != pSection->Keys.end(); k_pos++) {
        if (CompareNoCase((*k_pos).szKey, szKey) == 0) {
            pSection->Keys.erase(k_pos);
            return true;
        }
    }

    return false;
}

// CreateKey
// Given a key, a value and a section, this function will attempt to locate the
// Key within the given section, and if it finds it, change the keys value to
// the new value. If it does not locate the key, it will create a new key with
// the proper value and place it in the section requested.

bool CIniFileManager::CreateKey(t_Str szKey, t_Str szValue, t_Str szComment, t_Str szSection) {
    bool bAutoKey = (m_Flags & AUTOCREATE_KEYS) == AUTOCREATE_KEYS;
    bool bReturn = false;

    m_Flags |= AUTOCREATE_KEYS;

    bReturn = SetValue(szKey, szValue, szComment, szSection);

    if (!bAutoKey)
        m_Flags &= ~AUTOCREATE_KEYS;

    return bReturn;
}


// CreateSection
// Given a section name, this function first checks to see if the given section
// allready exists in the list or not, if not, it creates the new section and
// assigns it the comment given in szComment.  The function returns true if
// sucessfully created, or false otherwise.

bool CIniFileManager::CreateSection(t_Str szSection, t_Str szComment) {
    t_Section* pSection = GetSection(szSection);

    if (pSection) {
        Report(E_INFO, "[CIniFileManager::CreateSection] Section <%s> allready exists. Aborting.", szSection.c_str());
        return false;
    }

    pSection = new t_Section;

    pSection->szName = szSection;
    pSection->szComment = szComment;
    m_Sections.push_back(*pSection);
    m_bDirty = true;

    delete pSection;

    return true;
}

// CreateSection
// Given a section name, this function first checks to see if the given section
// allready exists in the list or not, if not, it creates the new section and
// assigns it the comment given in szComment.  The function returns true if
// sucessfully created, or false otherwise. This version accpets a KeyList 
// and sets up the newly created Section with the keys in the list.

bool CIniFileManager::CreateSection(t_Str szSection, t_Str szComment, KeyList Keys) {
    if (!CreateSection(szSection, szComment))
        return false;

    t_Section* pSection = GetSection(szSection);

    if (!pSection)
        return false;

    KeyItor k_pos;

    pSection->szName = szSection;
    for (k_pos = Keys.begin(); k_pos != Keys.end(); k_pos++) {
        t_Key* pKey = new t_Key;
        pKey->szComment = (*k_pos).szComment;
        pKey->szKey = (*k_pos).szKey;
        pKey->szValue = (*k_pos).szValue;

        pSection->Keys.push_back(*pKey);
    }

    m_Sections.push_back(*pSection);
    m_bDirty = true;

    return true;
}

// SectionCount
// Simply returns the number of sections in the list.

int CIniFileManager::SectionCount() {
    return m_Sections.size();
}

// KeyCount
// Returns the total number of keys contained within all the sections.

int CIniFileManager::KeyCount() {
    int nCounter = 0;
    SectionItor s_pos;

    for (s_pos = m_Sections.begin(); s_pos != m_Sections.end(); s_pos++)
        nCounter += (*s_pos).Keys.size();

    return nCounter;
}


// Protected Member Functions ///////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

// GetKey
// Given a key and section name, looks up the key and if found, returns a
// pointer to that key, otherwise returns NULL.

t_Key* CIniFileManager::GetKey(t_Str szKey, t_Str szSection) {
    KeyItor k_pos;
    t_Section* pSection;

    // Since our default section has a name value of t_Str("") this should
    // always return a valid section, wether or not it has any keys in it is
    // another matter.
    if ((pSection = GetSection(szSection)) == NULL)
        return NULL;

    for (k_pos = pSection->Keys.begin(); k_pos != pSection->Keys.end(); k_pos++) {
        if (CompareNoCase((*k_pos).szKey, szKey) == 0)
            return (t_Key*)&(*k_pos);
    }

    return NULL;
}

// GetSection
// Given a section name, locates that section in the list and returns a pointer
// to it. If the section was not found, returns NULL

t_Section* CIniFileManager::GetSection(t_Str szSection) {
    SectionItor s_pos;

    for (s_pos = m_Sections.begin(); s_pos != m_Sections.end(); s_pos++) {
        if (CompareNoCase((*s_pos).szName, szSection) == 0)
            return (t_Section*)&(*s_pos);
    }

    return NULL;
}

t_Str CIniFileManager::CommentStr(t_Str szComment) {
    t_Str szNewStr = t_Str("");

    Trim(szComment);

    if (szComment.size() == 0)
        return szComment;

    if (szComment.find_first_of(CommentIndicators) != 0) {
        szNewStr = CommentIndicators[0];
        szNewStr += " ";
    }

    szNewStr += szComment;

    return szNewStr;
}



// Utility Functions ////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

//// GetNextWord
//// Given a key +delimiter+ value t_Str, pulls the key name from the t_Str,
//// deletes the delimiter and alters the original t_Str to contain the
//// remainder.  Returns the key
//t_Str GetNextWord(t_Str& CommandLine)
//{
//    int nPos = CommandLine.find_first_of(EqualIndicators);
//    t_Str sWord = t_Str("");
//
//    if ( nPos > -1 )
//    {
//        sWord = CommandLine.substr(0, nPos);
//        CommandLine.erase(0, nPos+1);
//    }
//    else
//    {
//        sWord = CommandLine;
//        CommandLine = t_Str("");
//    }
//
//    Trim(sWord);
//    return sWord;
//}
//
//
//// CompareNoCase
//// it's amazing what features std::t_Str lacks.  This function simply
//// does a lowercase compare against the two t_Strs, returning 0 if they
//// match.
//int CompareNoCase(t_Str str1, t_Str str2)
//{
//#ifdef WIN32
//    return stricmp(str1.c_str(), str2.c_str());
//#else
//    return strcasecmp(str1.c_str(), str2.c_str());
//#endif
//}
//
//// Trim
//// Trims whitespace from both sides of a t_Str.
//void Trim(t_Str& szStr)
//{
//    t_Str szTrimChars = WhiteSpace;
//
//    szTrimChars += EqualIndicators;
//    int nPos, rPos;
//
//    // trim left
//    nPos = szStr.find_first_not_of(szTrimChars);
//
//    if ( nPos > 0 )
//        szStr.erase(0, nPos);
//
//    // trim right and return
//    nPos = szStr.find_last_not_of(szTrimChars);
//    rPos = szStr.find_last_of(szTrimChars);
//
//    if ( rPos > nPos && rPos > -1)
//        szStr.erase(rPos, szStr.size()-rPos);
//}
//
//// WriteLn
//// Writes the formatted output to the file stream, returning the number of
//// bytes written.
//int WriteLn(fstream& stream, char* fmt, ...)
//{
//    char buf[MAX_BUFFER_LEN];
//    int nLength;
//    t_Str szMsg;
//
//    memset(buf, 0, MAX_BUFFER_LEN);
//    va_list args;
//
//    va_start (args, fmt);
//    nLength = vsnprintf(buf, MAX_BUFFER_LEN, fmt, args);
//    va_end (args);
//
//
//    if ( buf[nLength] != '\n' && buf[nLength] != '\r' )
//        buf[nLength++] = '\n';
//
//
//    stream.write(buf, nLength);
//
//    return nLength;
//}
//
//// Report
//// A simple reporting function. Outputs the report messages to stdout
//// This is a dumb'd down version of a simmilar function of mine, so if
//// it looks like it should do more than it does, that's why...
//void Report(e_DebugLevel DebugLevel, char *fmt, ...)
//{
//    char buf[MAX_BUFFER_LEN];
//    int nLength;
//    t_Str szMsg;
//
//    va_list args;
//
//    memset(buf, 0, MAX_BUFFER_LEN);
//
//    va_start (args, fmt);
//    nLength = vsnprintf(buf, MAX_BUFFER_LEN, fmt, args);
//    va_end (args);
//
//
//    if ( buf[nLength] != '\n' && buf[nLength] != '\r' )
//        buf[nLength++] = '\n';
//
//
//    switch ( DebugLevel )
//    {
//        case E_DEBUG:
//            szMsg = "<debug> ";
//            break;
//        case E_INFO:
//            szMsg = "<info> ";
//            break;
//        case E_WARN:
//            szMsg = "<warn> ";
//            break;
//        case E_ERROR:
//            szMsg = "<error> ";
//            break;
//        case E_FATAL:
//            szMsg = "<fatal> ";
//            break;
//        case E_CRITICAL:
//            szMsg = "<critical> ";
//            break;
//    }
//
//
//    szMsg += buf;
//
//
//#ifdef WIN32
//    OutputDebugString(szMsg.c_str());
//#endif
//
//    printf(szMsg.c_str());
//
//}

///////////////////////////////////////////////////////////////////////////////
//
//titlePos
//
//Parameters : char *     string line to analyze in which to search the title
//             int *      pointer to the length of the title found (output)
//
//Return type : char *    pointer to the value of the found title
//
//Action :
//   Get a pointer to a section's title from the string to analyze and output 
//   the length of the section's title in the pointer to the length of the 
//   title found. The title must be between [ and ] characters. 
//   Return 0 if the first character encountered that is not a space is not [.
//   Return 0 if the title is not ended by ].
//   If 0 is returned, the length of title is not valid and stays unchanged.
//
///////////////////////////////////////////////////////////////////////////////

char* CIniFileManager::titlePos(char *buf, int *len) {
    char *p = buf, *q;

    while (*p && isspace(*p)) {
        p++;
    }

    if (*p != '[') {
        return 0;
    }

    q = p + 1;
    while (*q && *q != ']') {
        q++;
    }

    if (*q != ']') {
        return 0;
    }

    if (len) {
        *len = (q - p - 1);
    }

    return p + 1;

}

///////////////////////////////////////////////////////////////////////////////
//
//isTitleLine
//
//Parameters : char *   line to process
//
//Return types : int    logic value (0 or 1)
//
//Actions :
//   Check if a string is a section title line, return 0 if false, return 1
//   if true.
//
///////////////////////////////////////////////////////////////////////////////

int CIniFileManager::isTitleLine(char *bufPtr) {
    return titlePos(bufPtr, 0) != 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//containTitle
//
//Parameters :       char *  string in which to search the title of a section
//             const char *  searched title
//
//Return type : int          logic boolean value
//
//Actions :
//   Check if a string contain the searched title. Return 1 if it contains it,
//   otherwise, 0.
//
///////////////////////////////////////////////////////////////////////////////

int CIniFileManager::containTitle(char *buf, const char *section) {
    char *p;
    int len;

    // Obtain the title from the string to search
    p = titlePos(buf, &len);

    if (p) {
        // The searched string contained a valid title
        if ((signed)strlen(section) == len &&
                strnicmp(section, p, len) == 0) {
            // It is the one we were searching for
            return true;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
//
//gotoSection
//
//Parameters :  FILE *        input file
//              const char *  name of the section to go to
//
//Return type : int           logic boolean value
//
//Actions :
//   Move file position to start line of a section. Return true if the section
//   has been found, otherwise, false.
//
//Limitation :
//   The maximum read length of the lines in the input file is MAX_BUFFER_LEN
//
///////////////////////////////////////////////////////////////////////////////

int CIniFileManager::gotoSection(FILE *is, const char *section) {
    char line[MAX_BUFFER_LEN];

    while (fgets(line, MAX_BUFFER_LEN, is)) {
        if (containTitle(line, section)) {
            return true;
        }
    }

    return false;

}

///////////////////////////////////////////////////////////////////////////////
//
//textPos
//
//Parameters : char *        line to analyze
//             const char *  searched entry
//
//Return type : char *       pointer to the entry value
//
//Action :
//   Get a pointer to the value of the searched entry if it is present into
//   the line to analyze, otherwise, return 0. Return 0 also if the line to
//   analyze is a comment line.
//
///////////////////////////////////////////////////////////////////////////////

char* CIniFileManager::textPos(char *buf, const char *entry) {
    char *p;
    unsigned int len;

    // lines beginning with ; are comment lines in the .ini file
    if (buf[0] == ';') {
        return 0;
    }

    // Put the pointer to the = character of the line
    p = strchr(buf, '=');
    if (!p) {
        // Return 0 if there is no = character
        return 0;
    }

    // Compute the length of the entry name
    len = (p - buf);

    // Return a pointer to the value if the entry is the same
    if (strlen(entry) == len && strnicmp(buf, entry, len) == 0) {
        return p + 1;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//stripQuotationChar
//
//Parameter : char *  String from which to strip the spaces characters
//
//Return type : void
//
//Action:
//   Strip all the spaces placed in front and at the end of the value in the 
//   input string.
//
///////////////////////////////////////////////////////////////////////////////

void CIniFileManager::stripQuotationChar(char *buf) {
    char *p;
    char *q;
    int len;
    p = buf;

    while (*p && isspace(*p)) {
        p++;
    }
    q = p;

    while (*q && !isspace(*q)) {
        q++;
    }

    if (q == p) {
        return;
    }

    len = q - p;
    memmove(buf, p, len);
    buf[len] = 0;

}

///////////////////////////////////////////////////////////////////////////////
//
//readEntry
//
//Parameters : FILE *        input file
//             const char *  entry from which obtain the value
//             char *        output buffer for the found value of the entry
//             int           maximum length allowed to put the value
//             int           enable(1)/disable(0) padding spaces stripping
//
//Return type : int          length of the found value, or -1 if not found
//
//Actions :
//   Get the value of an entry from the input file. The found value is copied
//   in the output buffer until the maximum length allowed to put the value is
//   reached.
//   Return the length of the found value, or -1 if the entry was not
//   found in the current section.
//
//Limitation :
//   The maximum line length read from the input file is MAX_BUFFER_LEN.
//   To be able to found the value, the actual position of the index in the 
//   input file must be at the end of the title of the scaned section.
//
///////////////////////////////////////////////////////////////////////////////

int CIniFileManager::readEntry(FILE *is, // input
        const char *entry, // input
        char *buf, // output
        int bufSize, // input
        int strip) // input
{
    char lineBuf[MAX_BUFFER_LEN];
    char *p, *cur;
    int len;

    cur = buf;
    *cur = '\0';
    len = -1;

    while (fgets(lineBuf, MAX_BUFFER_LEN, is)) {

        if (isTitleLine(lineBuf)) {
            // this is the next title line, the entry was not found in the
            // scaned section
            break;
        }

        p = textPos(lineBuf, entry);
        if (p == 0) {
            // the entry at this line is not the entry searched
            continue;
        }

        // Strip the padding spaces if the caller enabled this behaviour
        if (strip) {
            stripQuotationChar(p);
        }

        // Compute the length of the value
        len = (strlen(p)) - 1;

        // Put a end of string character if there is enough space
        if (bufSize - 1 < len) {
            p[bufSize - 1] = '\0';
        }

        // Copy the found value in the output buffer for a maximum of
        // bufSize characters
        strncpy(cur, p, bufSize);
        break;
    }

    return len;
}

///////////////////////////////////////////////////////////////////////////////
//
//GetConfigParamString
//
//Parameters : const char *  Configuration string             (in)
//             const char *  Parameter Name                   (in)
//                   char *  Destination buffer               (out)
//             const char *  Default value in case of failure (in)
//
//Return type :      short     
//
//Action : 
//   Gets the value of the provided Parameter in the Given Configuration 
//   string and copies it in the provided Destination buffer. Return 0 if 
//   these operation occured fine. Otherwise, it uses the provided Default
//   value and copies it in Destination buffer. In this case, it returns -1.
//   Every paramaters have to be specified, otherwise the function returns
//   -2 without setting the Destination buffer.
//
//   The Configuration string format is:
//      Param1:Value1,Param2:Value2,Param3:Value3,...,ParamN:ValueN
//
///////////////////////////////////////////////////////////////////////////////

short CIniFileManager::GetConfigParamString(const char* ConfigString,
        const char* ParamName,
        char* Dest,
        int MaxLen,
        const char* Default) {
    char *P1 = NULL, *P2 = NULL, EndChar = ',', *tempStr = 0x0;
    int retval = 0;

    char sep = '='; //Separator by default

    //Checking if the separator is present
    if (strchr(ConfigString, sep) == NULL) {
        //No default separator found, try with another
        sep = ':';

        if (strchr(ConfigString, sep) == NULL) {
            //No separator found, abort
            return -2;
        }
    }

    if (MaxLen < 0) MaxLen = 0;

    if (ConfigString != NULL && ParamName != NULL &&
            Dest != NULL && Default != NULL) {
        //copy the configuration string because it is a const char
        tempStr = new char[strlen(ConfigString) + 1];

        if (tempStr != NULL) {
            memset(tempStr, 0x0, (strlen(ConfigString) + 1) * sizeof (char));
            strcpy(tempStr, ConfigString);

            // Get a pointer to the given ParamName into ConfigString
            char *Param = strstr(tempStr, ParamName);

            while ((Param < tempStr + strlen(tempStr)) && (Param != 0x0)) {
                if (*(Param + strlen(ParamName)) == sep) {
                    if ((Param == tempStr) || (*(Param - 1) == ' ') || (*(Param - 1) == ',')) {
                        //We found the correct substring
                        break;
                    }
                }

                //The substring found is not the one we searched for
                Param = strstr(Param + 1, ParamName);

            }

            if ((Param != NULL) && (Param < tempStr + strlen(tempStr))) {
                P1 = (char*) strchr(Param, sep);
                if (P1 != NULL) {
                    P2 = strchr(P1, EndChar);
                    if (P2 == NULL) {
                        // Try if The value is the last of the config string
                        P2 = strchr(P1, '\0');
                        EndChar = '\0';
                    }
                    if (P2 != NULL) {
                        *P2 = '\0'; // Temporary end of string at the comma
                        if (strlen(1 + P1) < (unsigned int) MaxLen) {
                            strcpy(Dest, 1 + P1); // Copy the value in the destination buffer
                            *P2 = EndChar; // Set back the comma ( or \0 )
                        } else retval = -4; // MaxLen exceded
                    } else retval = -3; // EndChar (',' or '\0') not found
                } else retval = -2; // separator not found
            } else retval = -1; // Param name string not found
        } else retval = -1;

    }

    if (tempStr != 0x0) {
        delete []tempStr;
    }


    if (retval != 0) {
        strcpy(Dest, Default);
    }

    return retval;
}

short CIniFileManager::GetConfigParamString(const char * ConfigString, const char * ParamName, string * DestString, const char * DefaultString) {
    char buffer[MAX_BUFFER_LEN];
    short retVal = 0;

    memset(buffer, 0x0, MAX_BUFFER_LEN);

    retVal = GetConfigParamString(ConfigString, ParamName, buffer, MAX_BUFFER_LEN - 1, DefaultString);

    *DestString = buffer;

    return retVal;
}

///////////////////////////////////////////////////////////////////////////////
//
//GetConfigParamInt
//
//Parameters : const char *  Configuration string             (in)
//             const char *  Parameter Name                   (in)
//                   int  *  Destination integer              (out)
//             const int     Default value in case of failure (in)
//
//Return type :      short   Error
//
//Action : 
//   Gets the value of the provided Parameter in the Given Configuration 
//   string and copies it in the provided Destination integer. Return 0 if 
//   these operation occured fine. Otherwise, it uses the provided Default
//   value and copies it in Destination integer. In this case, it returns -1.
//   Every paramaters have to be specified, otherwise the function returns
//   -2 without setting the Destination integer.
//
//   The Configuration string format is:
//      Param1:Value1,Param2:Value2,Param3:Value3,...,ParamN:ValueN
//
///////////////////////////////////////////////////////////////////////////////

short CIniFileManager::GetConfigParamInt(const char* ConfigString,
        const char* ParamName,
        int* Dest,
        const int Default) {

    // Error by default
    short Retval = -2;

    if (ConfigString != NULL && ParamName != NULL && Dest != NULL) {
        // Get a pointer to the given ParamName into ConfigString
        const char *P1 = strstr(ConfigString, ParamName);

        char sep = '='; //Separator by default

        //Checking if the separator is present
        if (strchr(ConfigString, sep) == NULL) {
            //No default separator found, try with another
            sep = ':';

            if (strchr(ConfigString, sep) == NULL) {
                //No separator found, abort
                return Retval;
            }
        }

        while ((P1 < ConfigString + strlen(ConfigString)) && (P1 != 0x0)) {
            if ((*(P1 + strlen(ParamName)) != sep) || ((*(P1 - 1) != ' ') && (*(P1 - 1) != ',') && (P1 != ConfigString))) {
                //The substring found is not the one we searched for
                P1 = strstr(P1 + 1, ParamName);
            } else {
                //We found the correct substring
                break;
            }
        }

        if ((P1 != NULL) && (P1 < ConfigString + strlen(ConfigString))) {

            P1 = strchr(P1, sep);

            // atoi does not have to receive a null terminated string
            *Dest = atoi(P1 + 1);

            // No error occured
            Retval = 0;
        } else {
            // Param was not found, use default
            *Dest = Default;
            Retval = -1;
        }

    }

    return Retval;

}

///////////////////////////////////////////////////////////////////////////////
//
//GetConfigParamBool
//
//Parameters : const char *  Configuration string             (in)
//             const char *  Parameter Name                   (in)
//                   bool *  Destination bool                 (out)
//             const bool    Default value in case of failure (in)
//
//Return type :      short   Error
//
//Action : 
//   Gets the value of the provided Parameter in the Given Configuration 
//   string and copies it in the provided Destination bool. Return 0 if 
//   these operation occured fine. Otherwise, it uses the provided Default
//   value and copies it in Destination bool. In this case, it returns -1.
//   Every paramaters have to be specified, otherwise the function returns
//   -2 without setting the Destination bool.
//
//   The Configuration string format is:
//      Param1:Value1,Param2:Value2,Param3:Value3,...,ParamN:ValueN
//
///////////////////////////////////////////////////////////////////////////////

short CIniFileManager::GetConfigParamBool(const char* ConfigString,
        const char* ParamName,
        bool* Dest,
        const bool Default) {
    int Temp = 0;
    int Retval = GetConfigParamInt(ConfigString,
            ParamName,
            &Temp,
            Default);

    if (Temp == 0) {
        *Dest = false;
    } else {
        *Dest = true;
    }

    return Retval;
}

///////////////////////////////////////////////////////////////////////////////
//
//GetParamStringLength 
//
//Parameters :  const char* configString  - String containing the parameters  (in)
//              const char* paramName     - the parameter we are looking for  (in)
//                    int *stringLength   - Where to store the value          (out)
//              const int defaultLength   - Default value for string length   (in)
//
//Return type : int - 0     Operation succesful
//                  - -1    Problems in computing the length of the string
//
//Actions :
//   Searches for paramName inside configString and stores the length 
//   in stringLength.
//
///////////////////////////////////////////////////////////////////////////////

int CIniFileManager::GetParamStringLength(const char* configString,
        const char* paramName,
        int * stringLength,
        const int defaultLength) {
    //Return value
    int retval = 0;

    //Pointers used to compute the string length
    const char *p2 = NULL;

    //Searching for parameter
    const char *p1 = strstr(configString, paramName);

    if (p1 != NULL) {

        //Parameter found, searching for the colon separator
        p1 = strchr(p1, ':');

        if (p1 != NULL) {
            p2 = strchr(p1, ',');
        } else {
            //Semicolon not found!! 
            *stringLength = defaultLength;
            retval = -1;
        }

        if (p2 != NULL) {
            //Computing stringLength, adding 1 to count for end of line
            *stringLength = (p2 - p1) + 1;
            retval = 0;
        } else {
            //Comma not found, using default value
            *stringLength = defaultLength;
            retval = -1;
        }
    }

    return retval;

}

float CIniFileManager::GetConfigParamFloat(const char * ConfigString, const char * ParamName, float * Dest, const float Default) {
    char tempBuffer[32];
    char defaultBuffer[32];
    float retVal = 0;

    sprintf(defaultBuffer, "%f", Default);

    GetConfigParamString(ConfigString,
            ParamName,
            tempBuffer,
            31,
            defaultBuffer);

    if (Dest != 0x0) {
        sscanf(tempBuffer, "%f", Dest);
        retVal = *Dest;
    } else {
        sscanf(tempBuffer, "%f", &retVal);
    }



    return retVal;
}

bool CIniFileManager::SetConfigParamString(string *configString, const char * paramName, const char * newVal) {
    bool retVal = false;
    string::size_type paramIndex = 0, commaIndex = 0, colonIndex = 0;
    string tempString;

    //Search for the parameters
    paramIndex = configString->find(paramName);

    if (paramIndex != string::npos) {
        //Get the subfields indexs
        colonIndex = configString->find(":", paramIndex);
        commaIndex = configString->find(",", paramIndex);

        if (colonIndex != string::npos) {
            //Copy everything up to the parameter in temporary string
            tempString = configString->substr(0, colonIndex + 1);
            tempString += newVal;

            //Check if there is more to copy
            if (commaIndex != string::npos) {
                tempString += configString->substr(commaIndex, configString->size() - colonIndex);
            }

            *configString = tempString;
            retVal = true;
        }
    }
    else {
        //LA aggiungo visto che non esiste
        *configString+=paramName;
        *configString+=":";
        *configString+=newVal;
        retVal = true;
    }

    return retVal;

}

bool CIniFileManager::AddConfigParamString(string *configString, const char * paramName, const char * newVal) {
    if (configString->size() != 0) {
        *configString += ',';
    }

    *configString += paramName;
    *configString += ':';
    *configString += newVal;

    return true;
}

