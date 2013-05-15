#ifndef WINTABAPI_H
#define WINTABAPI_H

#include <windows.h>
// WINTAB headers
#include "wintab.h"
#define PACKETDATA ( PK_X | PK_Y | PK_Z | PK_BUTTONS | PK_NORMAL_PRESSURE | PK_ORIENTATION | PK_TANGENT_PRESSURE )
#define PACKETMODE PK_BUTTONS
// Tablet control extension defines
#define PACKETEXPKEYS		PKEXT_ABSOLUTE
#define PACKETTOUCHSTRIP	PKEXT_ABSOLUTE
#define PACKETTOUCHRING		PKEXT_ABSOLUTE
#include "pktdef.h"

class WinTabAPI
{
public:
    // Function pointers to Wintab functions exported from wintab32.dll.
    typedef UINT ( API * WTINFOA ) ( UINT, UINT, LPVOID );
    typedef HCTX ( API * WTOPENA )( HWND, LPLOGCONTEXTA, BOOL );
    typedef BOOL ( API * WTGETA ) ( HCTX, LPLOGCONTEXT );
    typedef BOOL ( API * WTSETA ) ( HCTX, LPLOGCONTEXT );
    typedef BOOL ( API * WTCLOSE ) ( HCTX );
    typedef BOOL ( API * WTENABLE ) ( HCTX, BOOL );
    typedef BOOL ( API * WTPACKET ) ( HCTX, UINT, LPVOID );
    typedef BOOL ( API * WTOVERLAP ) ( HCTX, BOOL );
    typedef BOOL ( API * WTSAVE ) ( HCTX, LPVOID );
    typedef BOOL ( API * WTCONFIG ) ( HCTX, HWND );
    typedef HCTX ( API * WTRESTORE ) ( HWND, LPVOID, BOOL );
    typedef BOOL ( API * WTEXTSET ) ( HCTX, UINT, LPVOID );
    typedef BOOL ( API * WTEXTGET ) ( HCTX, UINT, LPVOID );
    typedef BOOL ( API * WTQUEUESIZESET ) ( HCTX, int );
    typedef int  ( API * WTDATAPEEK ) ( HCTX, UINT, UINT, int, LPVOID, LPINT);
    typedef int  ( API * WTPACKETSGET ) (HCTX, int, LPVOID);
    typedef HMGR ( API * WTMGROPEN ) ( HWND, UINT );
    typedef BOOL ( API * WTMGRCLOSE ) ( HMGR );
    typedef HCTX ( API * WTMGRDEFCONTEXT ) ( HMGR, BOOL );
    typedef HCTX ( API * WTMGRDEFCONTEXTEX ) ( HMGR, UINT, BOOL );
    typedef BOOL ( API * WTMGRCSRPRESSUREBTNMARKSEX ) ( HMGR, UINT, UINT FAR *, UINT FAR * );

    // TODO - add more wintab32 function defs as needed

    WTINFOA ptrWTInfoA = nullptr;
    WTOPENA ptrWTOpenA = nullptr;
    WTGETA ptrWTGetA = nullptr;
    WTSETA ptrWTSetA = nullptr;
    WTCLOSE ptrWTClose = nullptr;
    WTPACKET ptrWTPacket = nullptr;
    WTENABLE ptrWTEnable = nullptr;
    WTOVERLAP ptrWTOverlap = nullptr;
    WTSAVE ptrWTSave = nullptr;
    WTCONFIG ptrWTConfig = nullptr;
    WTRESTORE ptrWTRestore = nullptr;
    WTEXTSET ptrWTExtSet = nullptr;
    WTEXTGET ptrWTExtGet = nullptr;
    WTQUEUESIZESET ptrWTQueueSizeSet = nullptr;
    WTDATAPEEK ptrWTDataPeek = nullptr;
    WTPACKETSGET ptrWTPacketsGet = nullptr;
    WTMGROPEN ptrWTMgrOpen = nullptr;
    WTMGRCLOSE ptrWTMgrClose = nullptr;
    WTMGRDEFCONTEXT ptrWTMgrDefContext = nullptr;
    WTMGRDEFCONTEXTEX ptrWTMgrDefContextEx = nullptr;
    WTMGRCSRPRESSUREBTNMARKSEX ptrWTMgrCsrPressureBtnMarksEx = nullptr;

    HCTX context_ = nullptr;
};

#endif // WINTABAPI_H
