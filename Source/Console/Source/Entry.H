///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _ENTRY_H
#define _ENTRY_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../../@Libraries/CMemPool/CMemPool.H"
#include "../../../../@Libraries/CCmdLine/CCmdLine.H"
#include "../../../../@Libraries/CService/CService.H"

#include "../../../../@Libraries/Debug/Debug.H"
#include "../../../../@Libraries/NSWFL/NSWFL.H"

#include "../Resources/Resource.H"

#include "Routines.H"
#include "CManagement.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GENERIC_MENU_ABOUT				WM_USER + 10
#define WM_REFRESH_SELECTED_CONFIG_ITEM	WM_USER + 11
#define WM_REFRESH_CONFIG_TREE_ITEM		WM_USER + 12
#define WM_INITGRAPH                    WM_USER + 13

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern HINSTANCE ghAppInstance;

extern char gsAppCopyRight[64];
extern char gsFileVersion[10];
extern char gsTitleCaption[64];
extern char gsRegistryKey[255];

extern char gsPath[MAX_PATH];
extern char gsDataPath[MAX_PATH];
extern DWORD gdwManagementPort;

extern CMemPool *pMem;
extern CService gService;	//The global service.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

