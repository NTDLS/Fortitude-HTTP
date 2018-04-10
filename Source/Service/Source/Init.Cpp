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
#include "CManagement.H"

#include "CWebSites.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Registry;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool InitializeApp(void)
{
	INITCOMMONCONTROLSEX ccex;
	ccex.dwSize = sizeof(ccex);
	ccex.dwICC = ICC_TREEVIEW_CLASSES | ICC_PROGRESS_CLASS | ICC_BAR_CLASSES;
	InitCommonControlsEx(&ccex);

	pMem = new CMemPool;
	gWebSites = NULL;

	LoadString(ghAppInstance, IDS_APPCOPYRIGHT, gsAppCopyRight, sizeof(gsAppCopyRight));
    LoadString(ghAppInstance, IDS_FILEVERSION, gsFileVersion, sizeof(gsFileVersion));
    LoadString(ghAppInstance, IDS_TITLECAPTION, gsTitleCaption, sizeof(gsTitleCaption));
    LoadString(ghAppInstance, IDS_REGISTRYKEY, gsRegistryKey, sizeof(gsRegistryKey));

	strcpy_s(gsServiceName, sizeof(gsServiceName), gsTitleCaption);

	if(!LoadRegistry())
	{
		MessageBox(NULL, "Failed to load registry values.", gsTitleCaption, MB_ICONERROR);
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UninitializeApp(void)
{
	if(!SaveRegistry())
	{
		MessageBox(NULL, "Failed to load registry values.", gsTitleCaption, MB_ICONERROR);
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
	if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "Path", gsPath, dwSize))
	{
		return false;
	}
	sprintf_s(gsDataPath, sizeof(gsDataPath), "%s\\Data", gsPath);

	if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ManagementPort", dwManagementPort))
	{
		return false;
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
	//bResult = bResult && Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ManagementPort", dwManagementPort);

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

