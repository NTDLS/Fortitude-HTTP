///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _INIT_CPP
#define _INIT_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <WindowsX.H>
#include <ShellAPI.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <CommCtrl.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Init.H"
#include "Entry.H"

#include "../../@Common/Constants.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Registry;
using namespace NSWFL::Memory;
using namespace NSWFL::File;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool InitializeApp(void)
{
	INITCOMMONCONTROLSEX CCEX;
	CCEX.dwSize = sizeof(CCEX);
	CCEX.dwICC = ICC_TREEVIEW_CLASSES | ICC_PROGRESS_CLASS | ICC_BAR_CLASSES | ICC_ANIMATE_CLASS | ICC_LISTVIEW_CLASSES;
	if(!InitCommonControlsEx(&CCEX))
	{
		MessageBox(NULL, "Failed to initialize common controls.", gsTitleCaption, MB_ICONERROR);
		return false;
	}

	pMem = new MemoryPool;

	LoadString(ghAppInstance, IDS_APPCOPYRIGHT, gsAppCopyRight, sizeof(gsAppCopyRight));
    LoadString(ghAppInstance, IDS_FILEVERSION, gsFileVersion, sizeof(gsFileVersion));
    LoadString(ghAppInstance, IDS_TITLECAPTION, gsTitleCaption, sizeof(gsTitleCaption));
    LoadString(ghAppInstance, IDS_REGISTRYKEY, gsRegistryKey, sizeof(gsRegistryKey));

	if(!LoadSettings())
	{
		MessageBox(NULL, "Failed to load settings.", gsTitleCaption, MB_ICONERROR);
	}
	
	gManage = new CManagement();
	if(!gManage->ManagementInitialize())
	{
		MessageBox(NULL, "Failed to connect to the server, is the service running?", gsTitleCaption, MB_ICONERROR);
		return false;
	}

	gService.Initialize("Fortitude HTTP", NULL, NULL);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UninitializeApp(void)
{
	gManage->ManagementDestroy();
	delete gManage;

	if(!SaveSettings())
	{
		MessageBox(NULL, "Failed to save settings.", gsTitleCaption, MB_ICONERROR);
		return false;
	}

	delete pMem;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LoadSettings(void)
{
	DWORD dwSize = 0;
	DWORD dwBuffer = 0;

	dwSize = sizeof(gsPath);
	memset(gsPath, 0, dwSize);

	GetModuleFilePath(gsPath, sizeof(gsPath));
	sprintf_s(gsDataPath, sizeof(gsDataPath), "%s\\Data", gsPath);
	CorrectReversePath(gsPath, sizeof(gsPath));
	CorrectReversePath(gsDataPath, sizeof(gsDataPath));

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SaveSettings(void)
{
	bool bResult = true;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

