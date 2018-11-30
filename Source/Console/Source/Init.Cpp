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

	if(!LoadRegistry())
	{
		//MessageBox(NULL, "Failed to load registry values.", gsTitleCaption, MB_ICONERROR);
		//return false;
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

	if(!SaveRegistry())
	{
		MessageBox(NULL, "Failed to save registry values.", gsTitleCaption, MB_ICONERROR);
		return false;
	}

	delete pMem;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LoadRegistry(void)
{
	DWORD dwSize = 0;
	DWORD dwBuffer = 0;

	dwSize = sizeof(gsPath);
	memset(gsPath, 0, dwSize);

	//Get the path and data path from the registry, if they cant be found - use the application path as a hint.
	if (!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "Path", gsPath, dwSize))
	{
		GetModuleFilePath(gsPath, sizeof(gsPath));
		sprintf_s(gsDataPath, sizeof(gsDataPath), "%s\\..\\Data", gsPath);
		CorrectReversePath(gsPath, sizeof(gsPath));
		CorrectReversePath(gsDataPath, sizeof(gsDataPath));
	}
	else
	{
		sprintf_s(gsDataPath, sizeof(gsDataPath), "%s\\Data", gsPath);
	}

	if (!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ManagementPort", gdwManagementPort))
	{
		gdwManagementPort = DFLT_MANAGEMENT_PORT;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SaveRegistry(void)
{
	bool bResult = true;

	//None of this needs to be saved:
	//bResult = bResult && Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "Path", gsPath);
	//bResult = bResult && Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "DataPath", gsDataPath);
	//bResult = bResult && Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ManagementPort", gdwManagementPort);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

