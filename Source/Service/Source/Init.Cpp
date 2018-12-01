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
using namespace NSWFL::Memory;
using namespace NSWFL::File;
using namespace NSWFL::CommandLine;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool InitializeApp(void)
{
	INITCOMMONCONTROLSEX ccex;
	ccex.dwSize = sizeof(ccex);
	ccex.dwICC = ICC_TREEVIEW_CLASSES | ICC_PROGRESS_CLASS | ICC_BAR_CLASSES;
	InitCommonControlsEx(&ccex);

	pMem = new MemoryPool;
	gWebSites = NULL;

	LoadString(ghAppInstance, IDS_APPCOPYRIGHT, gsAppCopyRight, sizeof(gsAppCopyRight));
    LoadString(ghAppInstance, IDS_FILEVERSION, gsFileVersion, sizeof(gsFileVersion));
    LoadString(ghAppInstance, IDS_TITLECAPTION, gsTitleCaption, sizeof(gsTitleCaption));
    LoadString(ghAppInstance, IDS_REGISTRYKEY, gsRegistryKey, sizeof(gsRegistryKey));

	strcpy_s(gsServiceName, sizeof(gsServiceName), gsTitleCaption);

	CommandLineParser cmdLine;

	memset(gsDataPath, 0, sizeof(gsDataPath));
	for (int iCommand = 1; iCommand < cmdLine.Count(); iCommand++)
	{
		if (cmdLine.IsCommand(iCommand, "/Data:", gsDataPath, sizeof(gsDataPath)))
		{
			break;
		}
	}

	if(!LoadSettings())
	{
		MessageBox(NULL, "Failed to load settings.", gsTitleCaption, MB_ICONERROR);
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UninitializeApp(void)
{
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
	if (strlen(gsDataPath) == 0)
	{
		sprintf_s(gsDataPath, sizeof(gsDataPath), "%s\\Data", gsPath);
		CorrectReversePath(gsDataPath, sizeof(gsDataPath));
	}
	CorrectReversePath(gsPath, sizeof(gsPath));

	char sXMLFile[MAX_PATH];
	memset(gsPath, 0, sizeof(sXMLFile));

	XMLReader xmlService;
	_makepath_s(sXMLFile, sizeof(sXMLFile), NULL, gsDataPath, "ServiceSettings.xml", NULL);
	if (xmlService.FromFile(sXMLFile))
	{
		gdwManagementPort = xmlService.ToInteger("ManagementPort", DFLT_MANAGEMENT_PORT);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SaveSettings(void)
{
	bool bResult = true;

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

