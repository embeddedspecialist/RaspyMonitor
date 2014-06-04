#include "vcoordinator.h"

//Includo qui i riferimenti alla engine e alla net
#include "conewireengine.h"
#include "conewirenet.h"

#define ONEWIREENG_ptr(ptr) reinterpret_cast<COneWireEngine*>(m_EnginePtr)
#define ONEWIRENET_ptr(ptr) reinterpret_cast<COneWireNet*>(m_NetPtr)

/*static*/ CVController* CVCoordinator::RetrieveCtrl( void* pNet, int Addr, unsigned short* errorCode )
{
    CVController *retCtrltPtr = 0x0;

    //controllo puntatore void//
    if( pNet == 0x0 )
    { 
        if ( errorCode != 0x0 ) { *errorCode = RC_INVALID_VPTR; } 
        return 0x0;
    }
    
    //controllo ADDR//
    if( Addr < 0 )
    {
        if ( errorCode != 0x0 ) { *errorCode = RC_INVALID_ADDR; } 
        return 0x0;
    }
    
    //cast del puntatore//
    COneWireNet *netPtr = reinterpret_cast<COneWireNet*>( pNet );
    
    //Recupero l'handler del dispositivo
    const int netIndex = netPtr->GetNetByMemoryAddress ( Addr );
    if( netIndex < 0 )
    {
        if ( errorCode != 0x0 ) { *errorCode = RC_INVALID_NETINDEX; } 
        return 0x0;
    }
    
    const int ctrlIndex = netPtr->GetDeviceIndexByMemoryAddress ( netIndex, Addr );
    if( netIndex < 0 )
    {
        if ( errorCode != 0x0 ) { *errorCode = RC_INVALID_CTRLINDEX; } 
        return 0x0;
    }

    retCtrltPtr = ( netPtr->GetNetHandler( netIndex ) )->CtrlList[ctrlIndex];
    //nessun controllo previsto//
    
    if ( errorCode != 0x0 ) { *errorCode = RC_OK; } 
    return retCtrltPtr;
}

CVController* CVCoordinator::RetrieveCtrl( int Addr, unsigned short* errorCode )
{
    return ( RetrieveCtrl( m_NetPtr, Addr, errorCode ) );
}

