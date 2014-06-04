#include "CMsgRecv.h"
#include <string.h>
 
/////////////////////////////////////////////////////////////////////////////
// CMsgRecv constructor, see .h file for description
/////////////////////////////////////////////////////////////////////////////
CMsgRecv::CMsgRecv(char *Buffer, unsigned long BufferSize)
{
   m_ReceivingBuffer = Buffer;
   m_BufferSize  = BufferSize;
   m_ValidLabels = 0;
   m_MaxNumberOfLabel = 0;

   m_LabelTable  = NULL;

   if ((Buffer != NULL) && (BufferSize > 0))
   {
      Parse(m_ReceivingBuffer, m_BufferSize);
   }
}

/////////////////////////////////////////////////////////////////////////////
// CMsgRecv destructor, see .h file for description
/////////////////////////////////////////////////////////////////////////////
CMsgRecv::~CMsgRecv()
{
   DeallocateMemory();
}

/////////////////////////////////////////////////////////////////////////////
// GetLabelTable inspector, see .h file for description
/////////////////////////////////////////////////////////////////////////////
char ** CMsgRecv::GetLabelTable(void) const 
{
   //IF there is at least one # THEN
   if(m_MaxNumberOfLabel > 0)
   {
     return m_LabelTable;
   }
   //ELSE
   else
   {
      return NULL;
   }
}

/////////////////////////////////////////////////////////////////////////////
// GetValidLabels inspector, see .h file for description
/////////////////////////////////////////////////////////////////////////////
long CMsgRecv::GetValidLabels(void) const
{
   return m_ValidLabels;
}

/////////////////////////////////////////////////////////////////////////////
// GetBufferNextPtr inspector, see .h file for description
/////////////////////////////////////////////////////////////////////////////
char * CMsgRecv::GetBufferNextPtr(void) const
{
   //IF there is at least one STARTCHAR THEN
   if(m_MaxNumberOfLabel > 0)
   {
     return (m_ReceivingBuffer + strlen(m_ReceivingBuffer) );
   }
   //ELSE
   else
   {
      return m_ReceivingBuffer;
   }
}

/////////////////////////////////////////////////////////////////////////////
// AllocateMemory function, see .h file for description
/////////////////////////////////////////////////////////////////////////////
void CMsgRecv::AllocateMemory(void)
{
   long i = 0 ;

   m_LabelTable = new char* [m_MaxNumberOfLabel];

   for(i=0; i< m_MaxNumberOfLabel; i++)
   {
      m_LabelTable[i] = new char[m_BufferSize];
   }
}

/////////////////////////////////////////////////////////////////////////////
// DeallocateMemory function, see .h file for description
/////////////////////////////////////////////////////////////////////////////
void CMsgRecv::DeallocateMemory(void)
{
   long i = 0;

   if (m_LabelTable != NULL)
   {
      for (i = 0; i<m_MaxNumberOfLabel; i++)
      {
         delete []  m_LabelTable[i];
      }
   
      delete [] m_LabelTable;
      m_LabelTable = NULL;
   }
}

/////////////////////////////////////////////////////////////////////////////
// SetMaxNumberOfLabels function, see .h file for description
/////////////////////////////////////////////////////////////////////////////
void CMsgRecv::SetMaxNumberOfLabels(const char * AnalyzerPtr)
{
   m_MaxNumberOfLabel = 0;

   //WHILE not a the last char of the string
   while(*AnalyzerPtr != '\0')
   {
      //IF the current char is a STARTCHAR THEN
      if(*AnalyzerPtr == STARTCHAR)
      {
         //Increment the number of possible labels
         m_MaxNumberOfLabel++;
      }
      //Move the Analyzer Pointer to the next char
      AnalyzerPtr++;
   }
}

/////////////////////////////////////////////////////////////////////////////
// Parse function, see .h file for description
/////////////////////////////////////////////////////////////////////////////
void CMsgRecv::Parse(char *Buffer, unsigned long BufferSize) 
{
   char * StartPtr = NULL;
   char * EndPtr   = NULL;

   long i = 0 ; // Loop Counter

   //Freeing the Table memory
   DeallocateMemory();

   //Assigning memeber variables
   m_ReceivingBuffer = Buffer;
   m_BufferSize  = BufferSize;
   m_ValidLabels = 0;

   SetMaxNumberOfLabels(m_ReceivingBuffer);



   //IF the buffer contains at least one < THEN
   if(m_MaxNumberOfLabel > 0)
   {
      AllocateMemory();
      StartPtr = m_ReceivingBuffer;

      //Parse the buffer as many times as there is #
      for (i = 0; i<m_MaxNumberOfLabel; i++)
      {
         //Get the Start of a Label
         StartPtr = strchr(StartPtr, STARTCHAR);

         EndPtr = strchr(StartPtr, ENDCHAR);


         //IF both char were found THEN
         if((StartPtr != NULL) && (EndPtr != NULL))
         {
            //Copy the string into the LabelTable
            strncpy(m_LabelTable[i], StartPtr, EndPtr-StartPtr+1);
            m_LabelTable[i][EndPtr-StartPtr+1] = '\0' ;

            StartPtr = EndPtr+1;

            //Increment the number of full labels
            m_ValidLabels++;
         }
         //ELSE IF Begining char without the ending (incomplete msg) THEN
         else if (StartPtr != NULL)
         {
            strcpy(m_LabelTable[i], StartPtr);
         }
      }

      //Put only NULL char in the buffer
      memset( m_ReceivingBuffer, '\0', m_BufferSize );

      //IF there is a incomplete msg THEN copy it into the buffer
      if((StartPtr != NULL) && (EndPtr == NULL))
      {
         //Copy the incomplete message in the buffer
         strcpy(m_ReceivingBuffer, m_LabelTable[m_MaxNumberOfLabel-1]);
      }
   }
}


