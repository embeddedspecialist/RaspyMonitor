////////////////////////////////////////////////////////////////////////////////
//!\class  CMsgStream
//!                      MsgStream
//!The CMsgStream class is a common interface for the ports to access output 
//!devices. Must be derived to give more functionalities because in is basic 
//!implementation it just writes on stdout.
//!
//////////////////////////////////////////////////////////////////////////////// 
#if !defined(AFX_MSGSTREAM_H__793C3081_882A_11D6_8EE6_0050BA4BF4D2__INCLUDED_)
#define AFX_MSGSTREAM_H__793C3081_882A_11D6_8EE6_0050BA4BF4D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>


class CMsgStream  
{
public:

///////////////////////////////////////////////////////////////////////////////
//
//! Destructor
//
//!\param void
//!
//!\return nothing
//!
//! <b>Actions :</b><br>
//!   Empty destructor
//!
///////////////////////////////////////////////////////////////////////////////
   virtual ~CMsgStream() {} ;   
   
///////////////////////////////////////////////////////////////////////////////
//
// Write
//!Write to message stream
//
//!\param buf Pointer to the buffer conatining data
//!
//!\return nothing
//!
//! <b>Actions :</b><br>
//!   If not derived it just writes on stdout.
//!
///////////////////////////////////////////////////////////////////////////////
   virtual void Write(const char* buf ) { printf( "%s\r\n",buf ) ; fflush(stdout);};
};

#endif // !defined(AFX_MSGSTREAM_H__793C3081_882A_11D6_8EE6_0050BA4BF4D2__INCLUDED_)






