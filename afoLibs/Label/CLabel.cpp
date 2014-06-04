////////////////////////////////////////////////////////////////////////////////
//
// CLabel class implementation
//
// OS independent
//
// See class interface for infos.
//
////////////////////////////////////////////////////////////////////////////////

#include "CLabel.h"
#include <string.h>
#include <stdio.h>
#include <vector>
#include "cstring.h"
#include <stdlib.h>
#include <limits.h>
   
////////////////////////////////////////////////////////////////////////////////
// CLabel constructor, see .h file for infos
////////////////////////////////////////////////////////////////////////////////
CLabel::CLabel(const char *RawLabelMsg)
{
   m_Valid  = false;
   m_RawMsg = NULL;
   Parse(RawLabelMsg);
}

////////////////////////////////////////////////////////////////////////////////
// CLabel copy constructor, see .h file for infos
////////////////////////////////////////////////////////////////////////////////
CLabel::CLabel(const CLabel & ToEqual)
{
   m_Valid  = false;
   m_RawMsg = NULL;
   operator=(ToEqual);
}

////////////////////////////////////////////////////////////////////////////////
// ~CLabel destructor, see .h file for infos
////////////////////////////////////////////////////////////////////////////////
CLabel::~CLabel()
{
   FreeVector();
   FreeRawMsg();
}

////////////////////////////////////////////////////////////////////////////////
// Parse function, see .h file for description
////////////////////////////////////////////////////////////////////////////////
void CLabel::Parse(const char *RawMsg)
{
   // Invalid by default
   m_Valid = false;

   // Free memory used by vector
   FreeVector();

   if( RawMsg != NULL )
   {
      // Work with a copy of the provided raw msg
      FreeRawMsg();
      m_RawMsg = new char[strlen(RawMsg)+1 ];
      strcpy(m_RawMsg, RawMsg);

      // Necessary variables
      char *BeginPtr, *EndPtr, *StarPtr, Delimiter;
      long  FieldLength = 0, ArgNb = 1;

      // Pointer to the # is the beginning, pointer to the * is the end
      BeginPtr = strchr(m_RawMsg, '#');

      // Search for * character only after the # character because junk
      // characters may exist before #
      if( BeginPtr != NULL )
      {
         StarPtr = strchr(BeginPtr, '*');
      }

      if( BeginPtr == NULL || StarPtr == NULL )
      {
         // There is no # char or no * char after # char
         FreeVector();
         return;
      }

      // We want the beginning pointer at the next char of #
      BeginPtr++;

      // Find and set the Label ID
      EndPtr = strchr(BeginPtr, ',');
      FieldLength = EndPtr - BeginPtr;
      if( FieldLength <= 0 )
      {
         // There is no Label ID
         m_Valid = false;
         FreeVector();
         return;
      }
      else
      {
         // Set a end of string at the comma (,)
         *EndPtr = '\0';

         // Store the Label ID
         SetId(BeginPtr);

         // Set back the comma (,)
         *EndPtr = ',';
      }

      // Find and set CmdOrData field
      BeginPtr = ++EndPtr;
      EndPtr   = strchr(BeginPtr, ',');
      FieldLength = EndPtr - BeginPtr;
      if( FieldLength <= 0 )
      {
         // There is no Label ID
         FreeVector();
         return;
      }
      else
      {
         // Set a end of string at the comma (,)
         *EndPtr = '\0';

         // Store the Label ID
         SetCmdOrData(BeginPtr);

         // Set back the comma (,)
         *EndPtr = ',';
      }

      // Find and set NbOfArgs field
      BeginPtr = ++EndPtr;
      EndPtr   = strchr(BeginPtr, ';');
      FieldLength = EndPtr - BeginPtr;
      if( FieldLength <= 0 )
      {
         // There is no Label ID
         FreeVector();
         return;
      }
      else
      {
         // Set a end of string at the semi-colon (;)
         *EndPtr = '\0';

         // Store the Label ID
         SetNbOfArgs(BeginPtr);

         // Set back the semi-colon (;)
         *EndPtr = ';';
      }

      // Find and set all arguments, 
      // EndPtr is now pointing at the end of the header
      if( EndPtr <= StarPtr )
      {
         // Pointer right after the header
         BeginPtr = ++EndPtr;

         // Loop until there is no more comma (,) before the star (*)
         while(EndPtr != NULL && BeginPtr < StarPtr)
         {
            Delimiter = ',';
            EndPtr    = strchr(BeginPtr, Delimiter);

            if( EndPtr == NULL || EndPtr >= StarPtr )
            {
               // We are still into the message, at the last argument
               // exactly at  ...,argN* so no comma (,) was found before
               // the * character
               Delimiter = '*';
               EndPtr    = strchr(BeginPtr, Delimiter);
            }

            if( EndPtr != NULL )
            {
               // Temporary set a end of string at EndPtr
               *EndPtr = '\0';

               // Set the argument
               SetArg(ArgNb++, BeginPtr);

               // Set back the delimiter
               *EndPtr = Delimiter;
            }

            BeginPtr = ++EndPtr;
         }
         m_Valid = true;
      }
      else
      {
         // Match: if( EndPtr <= StarPtr )
         // There was a * character in the header
         FreeVector();
         return;
      }
   }
}

////////////////////////////////////////////////////////////////////////////////
// IsValid inspector, see .h file for description
////////////////////////////////////////////////////////////////////////////////
bool CLabel::IsValid(void) const
{
   return m_Valid;
}

////////////////////////////////////////////////////////////////////////////////
// GetRawMsg inspector, see .h file for description
////////////////////////////////////////////////////////////////////////////////
const char * CLabel::GetRawMsg(void) const
{
   return m_RawMsg;
}

////////////////////////////////////////////////////////////////////////////////
// GetId inspector, see .h file for description
////////////////////////////////////////////////////////////////////////////////
const char * CLabel::GetId(void) const
{
   if( IsValid() )
   {
      return m_Fields[LABELID];
   }
   return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// GetCmdOrData inspector, see .h file for description
////////////////////////////////////////////////////////////////////////////////
bool CLabel::GetCmdOrData(void) const
{
   if( strcmp(m_Fields[CMDORDATA], "1") == 0 )
   {
      return true;
   }
   return false;
}

////////////////////////////////////////////////////////////////////////////////
// GetNbOfArgs inspector, see .h file for description
////////////////////////////////////////////////////////////////////////////////
long CLabel::GetNbOfArgs(void) const
{
   if( IsValid() )
   {
      return atol( m_Fields[NBOFARGS] );
   }
   return -1;
}

////////////////////////////////////////////////////////////////////////////////
// GetStoredArgsNb inspector, see .h file for description
////////////////////////////////////////////////////////////////////////////////
long CLabel::GetStoredArgsNb(void) const
{
   if( IsValid() ) 
   {
      return m_Fields.size() - HEADERSIZE;
   }
   return -1;
}

////////////////////////////////////////////////////////////////////////////////
// GetArg inspector, see .h file for description
////////////////////////////////////////////////////////////////////////////////
const char * CLabel::GetArg(const long ArgNum) const
{
   if( ArgNum > 0 && ArgNum <= GetStoredArgsNb() )
   {
      // GetStoredArgsNb will return a neg. number if invalid
      // It is impossible to get here if invalid
      return m_Fields[HEADERSIZE + ArgNum - 1];
   }
   return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// SetArg function, see .h file for description
////////////////////////////////////////////////////////////////////////////////
void CLabel::SetArg(const long ArgNum, const char *ArgValue)
{
   SetField(HEADERSIZE + ArgNum - 1, ArgValue);
}

////////////////////////////////////////////////////////////////////////////////
// SetArg function, see .h file for description
////////////////////////////////////////////////////////////////////////////////
void CLabel::SetArg(const long ArgNum, unsigned long ArgValue)
{
   if( ArgNum > 0 )
   {
      // Store value as a string
      char *TempStr = new char[20];
      sprintf(TempStr, "%ld", ArgValue);

      // Save
      SetField(HEADERSIZE + ArgNum - 1, TempStr);

      // Free
      delete [] TempStr;
   }
}

////////////////////////////////////////////////////////////////////////////////
// = operator, see .h file for infos
////////////////////////////////////////////////////////////////////////////////
CLabel & CLabel::operator=(const CLabel & ToEqual)
{
   // Copy Raw Msg
   FreeRawMsg(); // Free first, and set to null
   if( ToEqual.GetRawMsg() != NULL )
   {
      m_RawMsg = new char[ strlen(ToEqual.GetRawMsg()) + 1 ];
      strcpy(m_RawMsg, ToEqual.GetRawMsg());
   }
   
   // Ensure same vector length
   if( m_Fields.size() < ToEqual.m_Fields.size() )
   {
      Grow(ToEqual.m_Fields.size());
   }
   else
   {
      Shrink(ToEqual.m_Fields.size());
   }

   // Copy all fields of vector
   unsigned long Index = 0;
   vector<char*>::const_iterator CopyIter;
   for( CopyIter = ToEqual.GetFieldsBegin();
        CopyIter != ToEqual.GetFieldsEnd();
        CopyIter++ )
   {
      SetField(Index++, *CopyIter);
   }

   m_Valid = ToEqual.IsValid();

   return *this;
}

////////////////////////////////////////////////////////////////////////////////
// == operator, see .h file for infos
////////////////////////////////////////////////////////////////////////////////
bool CLabel::operator==(const CLabel & Equal) const
{

   if( GetStoredArgsNb() != Equal.GetStoredArgsNb() )
   {
      return false;
   }

   // Compare all fields of vectors
   vector<char*>::const_iterator EqualIter;
   vector<char*>::const_iterator InstIter;
   for( InstIter = GetFieldsBegin(), EqualIter = Equal.GetFieldsBegin();
        InstIter != GetFieldsBegin() && EqualIter != Equal.GetFieldsEnd();
        InstIter++, EqualIter++ )
   {
      if( strlen(*InstIter) != strlen(*EqualIter) ||
          strcmp(*InstIter, *EqualIter) != 0 )
      {
         return false;
      }
   }

   return true;
}

////////////////////////////////////////////////////////////////////////////////
// GetFieldsBegin function
////////////////////////////////////////////////////////////////////////////////
vector<char*>::const_iterator CLabel::GetFieldsBegin(void) const
{
   vector<char*>::const_iterator Iter = m_Fields.begin();
   return Iter;
}

////////////////////////////////////////////////////////////////////////////////
// GetFieldsBegin function
////////////////////////////////////////////////////////////////////////////////
vector<char*>::const_iterator CLabel::GetFieldsEnd(void) const
{
   vector<char*>::const_iterator Iter = m_Fields.end();
   return Iter;
}

////////////////////////////////////////////////////////////////////////////////
// SetId function
////////////////////////////////////////////////////////////////////////////////
void CLabel::SetId(const char * Id)
{
   // Use a temporary buffer
   char * TempBuf = new char[ strlen(Id) + 1 ];
   strcpy(TempBuf, Id);
   
   // Store in a new memory space
   SetField(LABELID, TempBuf);

   // Free temp buffer
   delete [] TempBuf;
}

////////////////////////////////////////////////////////////////////////////////
// SetCmdOrData function
////////////////////////////////////////////////////////////////////////////////
void CLabel::SetCmdOrData(const bool CmdOrData)
{
   // Use a temporary buffer
   char * TempBuf = new char[ 2 ];
   sprintf(TempBuf, "%d", CmdOrData);

   // Store in a new memory space
   SetCmdOrData(TempBuf);

   // Free temp buffer
   delete [] TempBuf;
}

////////////////////////////////////////////////////////////////////////////////
// SetCmdOrData function
////////////////////////////////////////////////////////////////////////////////
void CLabel::SetCmdOrData(const char *CmdOrData)
{
   SetField(CMDORDATA, CmdOrData);
}

////////////////////////////////////////////////////////////////////////////////
// SetNbOfArgs function, see .h file for description
////////////////////////////////////////////////////////////////////////////////
void CLabel::SetNbOfArgs(const long Nb)
{
   // Use a temporary buffer
   char *TempBuf = new char[12];
   sprintf(TempBuf, "%ld", Nb);

   // Store in a new memory space
   SetNbOfArgs(TempBuf);

   // Free temp buffer
   delete [] TempBuf;
}

////////////////////////////////////////////////////////////////////////////////
// SetNbOfArgs function, see .h file for description
////////////////////////////////////////////////////////////////////////////////
void CLabel::SetNbOfArgs(const char * Nb)
{
   SetField(NBOFARGS, Nb);
}

////////////////////////////////////////////////////////////////////////////////
// SetField function, see .h file for description
////////////////////////////////////////////////////////////////////////////////
void CLabel::SetField(const unsigned long FieldNb, const char * Value)
{
   // Ensure capacity
   Grow(FieldNb+1);

   if( m_Fields[FieldNb] != NULL )
   {
      // Delete old Value
      delete [] m_Fields[FieldNb];
      m_Fields[FieldNb] = NULL;
   }

   if( Value != NULL )
   {
      // Set a new Value in the vector
      char * NewValue = new char[ strlen(Value) + 1 ];
      strcpy(NewValue, Value);
      m_Fields[FieldNb] = NewValue;
   }
}

////////////////////////////////////////////////////////////////////////////////
// FreeVector function, see .h file for infos
////////////////////////////////////////////////////////////////////////////////
void CLabel::FreeVector(void)
{
   vector<char*>::iterator Iter;

   // Free vector
   if( !m_Fields.empty() )
   {
      for(Iter = m_Fields.begin(); Iter != m_Fields.end(); Iter++)
      {
         if( *Iter != NULL )
         {
            // Free memory at this index
            delete [] *Iter;
            *Iter = NULL;
         }
      }

      m_Fields.clear();
   }

}

////////////////////////////////////////////////////////////////////////////////
// FreeRawMsg function, see .h file for infos
////////////////////////////////////////////////////////////////////////////////
void CLabel::FreeRawMsg(void)
{
   if( m_RawMsg != NULL )
   {
      delete [] m_RawMsg;
      m_RawMsg = NULL;
   }
}

////////////////////////////////////////////////////////////////////////////////
// Grow function, see .h file for infos
////////////////////////////////////////////////////////////////////////////////
void CLabel::Grow(const unsigned long FinalSize)
{
   while( m_Fields.size() < FinalSize )
   {
      m_Fields.push_back(NULL);
   }
}

////////////////////////////////////////////////////////////////////////////////
// Shrink function, see .h file for infos
////////////////////////////////////////////////////////////////////////////////
void CLabel::Shrink(const unsigned long FinalSize)
{
   char *Last;
   while( m_Fields.size() > FinalSize )
   {
      Last = m_Fields.back();
      if( Last != NULL )
      {
         // Avoid memory leaks
         delete [] Last;
         Last = NULL;
      }
      m_Fields.pop_back();
   }
}
