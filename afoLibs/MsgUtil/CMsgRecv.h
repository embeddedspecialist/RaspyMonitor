////////////////////////////////////////////////////////////////////////////////
//
// CLabel class
//
// OS independent
//
// Class that modelize the incoming data message from a READ socket
//
////////////////////////////////////////////////////////////////////////////////

#ifndef CMSGRECV_H
#define CMSGRECV_H

#include <string.h>
#include <stdio.h>

#define STARTCHAR (char)'<'
#define ENDCHAR   (char)'>'


class CMsgRecv {

public:
   /////////////////////////////////////////////////////////////////////////////
   //
   // CMsgRecv contructor
   //
   // Parameter: char *        Buffer (string)
   //            unsigned long BufferSize (Size of the given buffer)
   //
   // Return:    N/A
   //
   // Action:
   //    The constructor take the given Buffer, allocate X strings in the memory
   //    of the size BufferSize (to accomodate the biggest possible msg) to copy
   //    the contained data and call the parser.  Where X is the number of # in
   //    the Buffer.
   //    If no arguments are passed to the constructor it will do nothing
   //
   // See the Allocate function
   // See the Parse function
   /////////////////////////////////////////////////////////////////////////////
   CMsgRecv(char *Buffer= NULL, unsigned long BufferSize=0);

   /////////////////////////////////////////////////////////////////////////////
   //
   // ~CMsgRecv destructor
   //
   // Parameter: N/A
   //
   // Return:    N/A
   //
   // Action:
   //    Destruct the current instance of the CMsgRecv object and free its used 
   //    memory.  Note that the LabelTable is also freed.
   //
   /////////////////////////////////////////////////////////////////////////////
   ~CMsgRecv(void);

   /////////////////////////////////////////////////////////////////////////////
   //
   // CMsgRecv GetLabelTable inspector
   //
   // Parameter: void
   //
   // Return:    char ** Pointer to the String_Table
   //
   // Action:
   //    Return the pointer to the string table containing the labels if there
   //    is at least one # in the buffer given at construction.  In the other 
   //    case, it returns NULL.
   //
   /////////////////////////////////////////////////////////////////////////////
   char ** GetLabelTable(void) const ;

   /////////////////////////////////////////////////////////////////////////////
   //
   // CMsgRecv GetValidLabels inspector
   //
   // Parameter: void
   //
   // Return:    long   Number of valid labels
   //
   // Action:
   //    Return the number of strings begining by a # and finishing by a * in 
   //    the Buffer given at construction.
   //
   /////////////////////////////////////////////////////////////////////////////
   long GetValidLabels(void) const ;

   /////////////////////////////////////////////////////////////////////////////
   //
   // CMsgRecv GetBufferNextPtr inspector
   //
   // Parameter: void
   //
   // Return:    char *  Terminating char in the buffer
   //
   // Action:
   //    Return the pointer to the terminating character in the buffer where to
   //    continue to write data.
   //
   /////////////////////////////////////////////////////////////////////////////
   char * GetBufferNextPtr(void) const ;

   /////////////////////////////////////////////////////////////////////////////
   //
   // CMsgRecv Parse function
   //
   // Parameter: char *Buffer             - The Buffer to parse
   //            unsigned long BufferSize - Length of the buffer to consider
   //
   // Return:    void
   //
   // Action:
   //    Copy all the strings begining by # and ending by * into the LabelTable.
   //    At the end, if there is a string begining by # but without the ending
   //    char, it copies that string into the Buffer and set all the other char
   //    to \0 char, else it set every char in the buffer to \0.  Set the 
   //    NextPtr (see GetBufferNextPtr) to the first \0 in the buffer.
   //
   /////////////////////////////////////////////////////////////////////////////
   void Parse(char *Buffer, unsigned long BufferSize) ;

protected:
   /////////////////////////////////////////////////////////////////////////////
   //
   // CMsgRecv AllocateMemory function
   //
   // Parameter: void
   //
   // Return:    void
   //
   // Action:
   //    Allocate X strings of the size BufferSize (where X is the number of # 
   //    in the Buffer given at the construction).  The pointer to that string 
   //    table can be obtained with the function GetLabelTable.
   //    
   /////////////////////////////////////////////////////////////////////////////
   void AllocateMemory(void) ;

   /////////////////////////////////////////////////////////////////////////////
   //
   // CMsgRecv DeallocateMemory function
   //
   // Parameter: void
   //
   // Return:    void
   //
   // Action:
   //    Free X strings created by the AllocateMemory function.
   //
   /////////////////////////////////////////////////////////////////////////////
   void DeallocateMemory(void) ;

   /////////////////////////////////////////////////////////////////////////////
   //
   // CMsgRecv SetMaxNumberOfLabels inspector
   //
   // Parameter: void
   //
   // Return:    const char * Buffer
   //
   // Action:
   //    Set the Maximum number of possible labels in the given buffer, ie, how
   //    many # there is in the buffer string.
   //
   /////////////////////////////////////////////////////////////////////////////
   void SetMaxNumberOfLabels(const char *) ;

private:
   char ** m_LabelTable;
   char *  m_ReceivingBuffer;
   long    m_BufferSize;
   long    m_MaxNumberOfLabel;
   long    m_ValidLabels;
};
#endif
