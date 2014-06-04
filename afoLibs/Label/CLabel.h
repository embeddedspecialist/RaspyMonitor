////////////////////////////////////////////////////////////////////////////////
//
// CLabel class
//
// OS independent
//
// Class that modelize the incoming data message from the FOX Core Computer 
// called Label. Used by routines 3.2 and 3.3
//
////////////////////////////////////////////////////////////////////////////////

#ifndef CLABEL_H
#define CLABEL_H

#include <iostream> 

#include <vector>
using namespace std;


// Header definition
enum HeaderFieldsEnum {
   LABELID,   // 0 Label ID
   CMDORDATA, // 1 Cmd (1) or Data (0) flag
   NBOFARGS,  // 2 The number of arguments
   HEADERSIZE // 3 The first argument (keep last in this enum)
};


class CLabel {

public:
   /////////////////////////////////////////////////////////////////////////////
   //
   // CLabel contructor
   //
   // Parameter: const char *    Raw Label Msg
   //            or
   //            void
   //
   // Return:    N/A
   //
   // Action:
   //    Instanciate a new CLabel object using the provided Raw Label Msg by 
   //    calling Parse member function. See Parse member function.
   //
   //    See UC.2.1.3 requirement for Raw Label Msg format.
   //
   /////////////////////////////////////////////////////////////////////////////
   CLabel(const char *RawLabelMsg = NULL);

   /////////////////////////////////////////////////////////////////////////////
   //
   // CLabel copy contructor
   //
   // Parameter: const CLabel &  Label (by reference)
   //
   // Return:    N/A
   //
   // Action:
   //    Instanciate a new CLabel object equal to Label. Used by CTHashtable
   //    class. Syntax:
   //       CLabel LabB = LabA;
   //
   //    This constructor is also called when a CLabel object is passed to a 
   //    function by value (not by reference).
   //
   /////////////////////////////////////////////////////////////////////////////
   CLabel(const CLabel &);

   /////////////////////////////////////////////////////////////////////////////
   //
   // ~CLabel destructor
   //
   // Parameter: N/A
   //
   // Return:    N/A
   //
   // Action:
   //    Destruct the current instance of the CLabel object and free all its 
   //    used memory.
   //
   /////////////////////////////////////////////////////////////////////////////
   virtual ~CLabel(void);


   /////////////////////////////////////////////////////////////////////////////
   //
   // Parse function
   //
   // Parameter: const char *   Raw Label Msg
   //
   // Return:    void
   //
   // Action:
   //    Parses the provided Raw Label Msg and sets the current instance of the
   //    CLabel object. The object is validated only the parsing was performed 
   //    without errors. The provided Raw Label Msg can be safely deleted after
   //    being passed to that function, no pointer is kept at the provided
   //    Raw Label Msg. The function handles every case of memory leak.
   //    If the provided Raw Label Msg does not have the right format or is 
   //    null, the used memory of the current instance (if any) is freed and 
   //    the current instance is invalidated, but a copy of the provided Raw
   //    Label Msg is kept.
   //
   //    See UC.2.1.3 requirement for Raw Label Msg format.
   //
   /////////////////////////////////////////////////////////////////////////////
   void Parse(const char *);

   /////////////////////////////////////////////////////////////////////////////
   //
   // IsValid inspector
   //
   // Parameter: void
   //
   // Return:    bool    Validity state
   //
   // Action:
   //    Returns the validity state, i.e: if the message was parsed without 
   //    errors.
   //
   /////////////////////////////////////////////////////////////////////////////
   bool IsValid(void) const;

   /////////////////////////////////////////////////////////////////////////////
   //
   // GetRawMsg inspector
   //
   // Parameter: void
   //
   // Return:    const char *  Raw Label Msg 
   //
   // Action:
   //    Returns the Raw Label Msg of the current instance. The Raw Label Msg
   //    is kept even if the instance is not valid.
   //
   /////////////////////////////////////////////////////////////////////////////
   const char * GetRawMsg(void) const;

   /////////////////////////////////////////////////////////////////////////////
   //
   // GetId inspector
   //
   // Parameter: void
   //
   // Return:    const char *  Label ID
   //
   // Action:
   //    Returns the Label ID of the current instance, NULL if the current 
   //    instance is not valid.
   //
   /////////////////////////////////////////////////////////////////////////////
   const char * GetId(void) const;

   /////////////////////////////////////////////////////////////////////////////
   //
   // GetCmdOrData inspector
   //
   // Parameter: void
   //
   // Return:    bool    Cmd (1) or Data (0) flag
   //
   // Action:
   //    Returns the Label Cmd Or Data identifier flag, 1 for commands, 0 for
   //    data. Returns a not reliable value if the current instance is not
   //    valid.
   //
   /////////////////////////////////////////////////////////////////////////////
   bool GetCmdOrData(void) const;

   /////////////////////////////////////////////////////////////////////////////
   //
   // GetNbOfArgs inspector
   //
   // Parameter: void
   //
   // Return:    long    Number of args
   //
   // Action:
   //    Returns the Label number of arguments as specified by the parsed label
   //    message. Note that it could differ from the actual number of stored
   //    arguments. See GetStoredArgsNb inspector function.
   //    Returns a negative number if the current instance is not valid.
   //    Returns 0 if NbOfArgs is 0 or not a valid number to convert from a 
   //    character string, refer to atol documentation.
   //
   /////////////////////////////////////////////////////////////////////////////
   long GetNbOfArgs(void) const;

   /////////////////////////////////////////////////////////////////////////////
   //
   // GetStoredArgsNb inspector
   //
   // Parameter: void
   //
   // Return:    long    Number of args
   //
   // Action:
   //    Returns the Label number of stored arguments. Independent from the
   //    label message NbOfArgs field. It is the actual number of arguments
   //    that the parsed Label Message contained. Note that it could differ
   //    from the NbOfArgs number. See GetNbOfArgs inspector function.
   //    Returns a negative number if the current instance is not valid.
   //
   /////////////////////////////////////////////////////////////////////////////
   long GetStoredArgsNb(void) const;

   /////////////////////////////////////////////////////////////////////////////
   //
   // GetArg inspector
   //
   // Parameter: const long     Argument Number
   //
   // Return:    const char *   Pointer the specified Argument Number
   //
   // Action:
   //    Gets the argument specified by Argument Number. Returns NULL if 
   //    the provided Argument Number is out of index or if the current
   //    instance is not valid.
   //
   /////////////////////////////////////////////////////////////////////////////
   const char * GetArg(const long) const;

   /////////////////////////////////////////////////////////////////////////////
   //
   // SetArg function
   //
   // Parameter: const long     Argument Number
   //            const char *   Argument Value
   //
   // Return:    void
   //
   // Action:
   //    Sets the argument specified by Argument Number with the provided 
   //    Argument Value. The function has no effects if at least one of the 
   //    given parameters is not valid. NULL is not valid, Argument Number
   //    below 1 is not valid. See SetField().
   //
   /////////////////////////////////////////////////////////////////////////////
   void SetArg(const long, const char *);

   /////////////////////////////////////////////////////////////////////////////
   //
   // SetArg function
   //
   // Parameter: const long        Argument Number
   //            unsigned long *   Argument Value
   //
   // Return:    void
   //
   // Action:
   //    See previous SetArg.
   //
   /////////////////////////////////////////////////////////////////////////////
   void SetArg(const long, unsigned long);

   /////////////////////////////////////////////////////////////////////////////
   //
   // = operator
   //
   // Parameter: const CLabel &   Label
   //
   // Return:    CLabel           Equal Label
   //
   // Action:
   //    Equals the current instance with the content of Label and returns 
   //    the current instance. If the provided Label is invalid, the current
   //    instance is invalidated and its used memory is freed.
   //
   /////////////////////////////////////////////////////////////////////////////
   CLabel & operator=(const CLabel &);

   /////////////////////////////////////////////////////////////////////////////
   //
   // == operator
   //
   // Parameter: const CLabel &   Label
   //
   // Return:    bool             == or not
   //
   // Action:
   //    Verifies equality between current instance and the provided Label
   //    by comparing every fields of the vector container, note that the
   //    raw message is not considered by this operator.
   //
   /////////////////////////////////////////////////////////////////////////////
   bool operator==(const CLabel &) const;


   /////////////////////////////////////////////////////////////////////////////
   //
   // GetFieldsBegin
   //
   // Parameter: void
   //
   // Return:    vector<const char*>const_iterator Iterator
   //
   // Action:
   //    Returns an iterator currently at the beginning of the vector
   //    used to store the instance's values.
   //
   /////////////////////////////////////////////////////////////////////////////
   vector<char*>::const_iterator GetFieldsBegin(void) const;

   /////////////////////////////////////////////////////////////////////////////
   //
   // GetFieldsEnd
   //
   // Parameter: void
   //
   // Return:    vector<const char*>const_iterator Iterator
   //
   // Action:
   //    Returns an iterator currently at the end of the vector
   //    used to store the instance's values.
   //
   /////////////////////////////////////////////////////////////////////////////
   vector<char*>::const_iterator GetFieldsEnd(void) const;


protected:

   /////////////////////////////////////////////////////////////////////////////
   //
   // SetId function
   //
   // Parameter: const char *   Label ID
   //
   // Return:    void
   //
   // Action:
   //    Sets the ID of the current instance of CLabel. The function has no 
   //    effects if the provided Label ID is NULL.
   //
   /////////////////////////////////////////////////////////////////////////////
   void SetId(const char *);

   /////////////////////////////////////////////////////////////////////////////
   //
   // SetCmdOrData function
   //
   // Parameter: const bool  Cmd (true) or Data (false)
   //
   // Return:    void
   //
   // Action:
   //    Sets the Cmd Or Data flag of the current instance of CLabel.
   //
   /////////////////////////////////////////////////////////////////////////////
   void SetCmdOrData(const bool);

   /////////////////////////////////////////////////////////////////////////////
   //
   // SetCmdOrData function
   //
   // Parameter: const char *   Cmd ("1") or Data ("0")
   //
   // Return:    void
   //
   // Action:
   //    Sets the Cmd Or Data flag of the current instance of CLabel.
   //
   /////////////////////////////////////////////////////////////////////////////
   void SetCmdOrData(const char *);

   /////////////////////////////////////////////////////////////////////////////
   //
   // SetNbOfArgs function
   //
   // Parameter: const long   Number of Arguments
   //
   // Return:    void
   //
   // Action:
   //    Sets the Number of Arguments of the current instance of CLabel object
   //    with the provided Number of Arguments.
   //
   /////////////////////////////////////////////////////////////////////////////
   void SetNbOfArgs(const long);

   /////////////////////////////////////////////////////////////////////////////
   //
   // SetNbOfArgs function
   //
   // Parameter: const char *  Number of Arguments (as a string)
   //
   // Return:    void
   //
   // Action:
   //    Sets the Number of Arguments of the current instance of CLabel object
   //    with the provided Number of Arguments. The function has no effects if
   //    the provided Number of Arguments is NULL.
   //
   /////////////////////////////////////////////////////////////////////////////
   void SetNbOfArgs(const char *);


   // The Label fields are kept in a list
   //vector<const char*> m_Fields;
   vector<char*> m_Fields;

private:

   /////////////////////////////////////////////////////////////////////////////
   //
   // SetField function
   //
   // Parameter: const unsigned long     Field Number (vector index)
   //            const char *            Field Value
   //
   // Return:    void
   //
   // Action:
   //    Sets the field specified by Field Number with the provided Field Value.
   //    A copy of Field Value is used in a new memory area so the given Field 
   //    Value can be safely deleted or written after call to this function.
   //    If the provided Field Value is null, the field at Field Number is 
   //    cleared. Field Number begins at zero (0).
   //
   /////////////////////////////////////////////////////////////////////////////
   void SetField(const unsigned long, const char *);

   /////////////////////////////////////////////////////////////////////////////
   //
   // FreeVector function
   //
   // Parameter: void
   //
   // Return:    void
   //
   // Action:
   //    Frees the memory used by the vector containing fields of the current 
   //    instance.
   //
   /////////////////////////////////////////////////////////////////////////////
   void FreeVector(void);

   /////////////////////////////////////////////////////////////////////////////
   //
   // FreeRawMsg function
   //
   // Parameter: void
   //
   // Return:    void
   //
   // Action:
   //    Frees the memory used by the raw message copied at parsing time.
   //
   /////////////////////////////////////////////////////////////////////////////
   void FreeRawMsg(void);

   /////////////////////////////////////////////////////////////////////////////
   //
   // Grow function
   //
   // Parameter: const unsigned long  Final Size
   //
   // Return:    void
   //
   // Action:
   //    Grows the vector used to store fields until it reaches Final Size.
   //    This function has no effects if the vector already has the provided 
   //    Final Size (or more).
   //
   /////////////////////////////////////////////////////////////////////////////
   void Grow(const unsigned long);


   /////////////////////////////////////////////////////////////////////////////
   //
   // Shrink function
   //
   // Parameter: const unsigned long  Final Size
   //
   // Return:    void
   //
   // Action:
   //    Shrinks the vector used to store fields until it reaches Final Size.
   //    This function has no effects if the vector already has the provided 
   //    Final Size (or less).
   //
   /////////////////////////////////////////////////////////////////////////////
   void Shrink(const unsigned long);

   // The raw message, before being parsed
   char * m_RawMsg;

   // Validity state
   bool m_Valid;

};

#endif


