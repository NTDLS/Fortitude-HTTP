///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _ENTRY_CPP
#define _ENTRY_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"
#include "DebugHandler.H"
#include "Init.H"
#include "CManagement.H"
#include "CWebSites.H"

#include "../../@Common/Constants.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Registry;
using namespace NSWFL::Memory;
using namespace NSWFL::CommandLine;
using namespace NSWFL::Debug;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HINSTANCE ghAppInstance = NULL;

char gsAppCopyRight[64];
char gsFileVersion[10];
char gsTitleCaption[64];
char gsServiceName[255];
char gsRegistryKey[255];

char gsPath[MAX_PATH];
char gsDataPath[MAX_PATH];
DWORD gdwManagementPort = DFLT_MANAGEMENT_PORT;
bool gbLoadManagementInterface = true;

MemoryPool *pMem; //The global memory object.
CService gService;	//The global service.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsApplicationRunning(void)
{
	bool bResult = false;
	HANDLE hMutex = NULL;
	SECURITY_DESCRIPTOR SD;
	SECURITY_ATTRIBUTES SA;

	if (!InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION))
	{
		return false;
	}

	if (!SetSecurityDescriptorDacl(&SD, TRUE, 0, FALSE))
	{
		return false;
	}

	SA.nLength = sizeof(SA);
	SA.lpSecurityDescriptor = &SD;
	SA.bInheritHandle = FALSE;

	char sMutex[255];
	sprintf_s(sMutex, "Global\\%s", gsServiceName);
	if ((hMutex = CreateMutex(&SA, FALSE, sMutex)) != NULL)
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			bResult = true;
		}

		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
	}

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ServiceProcedure(LPVOID lpVoid)
{
	HANDLE MutexHandle = NULL;
	SECURITY_DESCRIPTOR SD;
	SECURITY_ATTRIBUTES SA;

	if (InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION) == FALSE)
		return false;

	if (SetSecurityDescriptorDacl(&SD, TRUE, 0, FALSE) == FALSE)
		return false;

	SA.nLength = sizeof(SA);
	SA.lpSecurityDescriptor = &SD;
	SA.bInheritHandle = FALSE;

	/*
	if((MutexHandle = CreateMutex(&SA, FALSE, gsTitleCaption)) != NULL)
	{
		if(GetLastError() == ERROR_ALREADY_EXISTS)
		{
			MessageBox(NULL, "Application already running.",
				gsTitleCaption, MB_SYSTEMMODAL + MB_TASKMODAL);
			return false;
		}
	}
	*/

	auto pWebsites = new CWebSites; //Load the websites.

	if (pMem->TrackMemory(pWebsites))
	{
		gWebSites->StopTrace();
		gService.KeepServiceRunning();
		delete (CWebSites *)pMem->UnTrackMemory(gWebSites);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int ProcessCommandLine(CommandLineParser *cmdLine)
{
	bool bOpenConsole = false;
	bool bRunService = false;
	bool bRunStandalone = false;

#ifdef _PORTABLE_RELEASE
	bRunStandalone = true;
	bOpenConsole = true;
#endif

	for (int iCommand = 1; iCommand < cmdLine->Count(); iCommand++)
	{
		const char *sCmdLine = cmdLine->Item(iCommand);

		char sCmdBuf[MAX_PATH];

		if (cmdLine->IsCommand(iCommand, "/NoManage"))
		{
			gbLoadManagementInterface = false;
		}
#ifndef _PORTABLE_RELEASE
		else if (cmdLine->IsCommand(iCommand, "/ServiceName:", gsServiceName, sizeof(gsServiceName)))
		{
			gService.ServiceName(gsServiceName);
		}
		else if (cmdLine->IsCommand(iCommand, "/Console"))
		{
			bOpenConsole = true;
		}
		else if (_strcmpi(sCmdLine, "/RunService") == 0)
		{
			bRunService = true;
			bRunStandalone = false;
		}
		else if (_strcmpi(sCmdLine, "/Standalone") == 0)
		{
			bRunService = false;
			bRunStandalone = true;
		}
#endif
		else if (cmdLine->IsCommand(iCommand, "/ManagePort:", sCmdBuf, sizeof(sCmdBuf)))
		{
			gdwManagementPort = atol(sCmdBuf);
		}
#ifndef _PORTABLE_RELEASE
		//------------------------------------------------------------------------------------------
		else if (_strcmpi(sCmdLine, "/Install") == 0)
		{
			if (gService.IsInstalled() == _SVC_YES_)
			{
				//MessageBox(NULL, "The service is already installed.", gsTitleCaption, MB_ICONINFORMATION);
				return 0;
			}

			char sApplication[257];
			sprintf_s(sApplication, sizeof(sApplication), "NetworkDLS %s", gsTitleCaption);
			if (gService.Install(sApplication) == _SVC_SUCCESS_)
			{
				gService.SetDescription("Provides HTTP web services on NT based Operating Systems.");
				return 0;
			}
			else {
				MessageBox(NULL, "Failed to install the service.", gsTitleCaption, MB_ICONWARNING);
				return 0;
			}

			return 0;
		}
		//------------------------------------------------------------------------------------------
		else if (_strcmpi(sCmdLine, "/Delete") == 0)
		{
			if (gService.IsInstalled() == _SVC_NO_)
			{
				return 0;
			}
			else if (gService.IsActive() == _SVC_YES_)
			{
				gService.Stop(); // Failure would not be an error.
				return 0;
			}
			else if (gService.Uninstall() != _SVC_SUCCESS_)
			{
				MessageBox(NULL, "Failed to mark the service for deletion.", gsTitleCaption, MB_ICONWARNING);
				return 0;
			}

			return 0;
		}
		//------------------------------------------------------------------------------------------
		else if (_strcmpi(sCmdLine, "/Start") == 0)
		{
			if (IsApplicationRunning())
			{
				return 0;
			}
			else if (gService.IsInstalled() == _SVC_NO_)
			{
				MessageBox(NULL, "The service is not installed.", gsTitleCaption, MB_ICONWARNING);
				return 0;
			}
			else if (gService.IsActive() == _SVC_YES_)
			{
				return 0;
			}
			else if (gService.Start() != _SVC_SUCCESS_)
			{
				MessageBox(NULL, "Failed to start the service.", gsTitleCaption, MB_ICONWARNING);
				return 0;
			}

			return 0;
		}
		//------------------------------------------------------------------------------------------
		else if (_strcmpi(sCmdLine, "/Stop") == 0)
		{
			if (gService.IsInstalled() == _SVC_NO_)
			{
				MessageBox(NULL, "The service is not installed.", gsTitleCaption, MB_ICONWARNING);
				return 0;
			}
			else if (gService.IsActive() == _SVC_NO_)
			{
				return 0; //Already stopped.
			}
			else if (gService.Stop() != _SVC_SUCCESS_)
			{
				MessageBox(NULL, "Failed to stop the service.", gsTitleCaption, MB_ICONWARNING);
				return 0;
			}
			return 0;
		}
		//------------------------------------------------------------------------------------------
		else if (_strcmpi(sCmdLine, "/SilentStart") == 0)
		{
			if (IsApplicationRunning())
			{
				return 0;
			}
			else if (gService.IsInstalled() == _SVC_NO_)
			{
				return 0;
			}
			else if (gService.IsActive() == _SVC_YES_)
			{
				return 0;
			}
			else if (gService.Start() != _SVC_SUCCESS_)
			{
				return 0;
			}

			return 0;
		}
		//------------------------------------------------------------------------------------------
		else if (_strcmpi(sCmdLine, "/SilentStop") == 0)
		{
			if (gService.IsInstalled() == _SVC_NO_)
			{
				return 0;
			}
			else if (gService.IsActive() == _SVC_NO_)
			{
				return 0; //Already stopped.
			}
			else if (gService.Stop() != _SVC_SUCCESS_)
			{
				return 0;
			}
			return 0;
		}
		//------------------------------------------------------------------------------------------
		else if (_strcmpi(sCmdLine, "/VerboseStart") == 0)
		{
			if (gService.IsInstalled() == _SVC_NO_)
			{
				MessageBox(NULL, "The service is not installed.", gsTitleCaption, MB_ICONWARNING);
				return 0;
			}
			else if (gService.IsActive() == _SVC_YES_)
			{
				MessageBox(NULL, "The service is already running.", gsTitleCaption, MB_ICONINFORMATION);
				return 0;
			}
			else if (IsApplicationRunning())
			{
				MessageBox(NULL, "The application is already running.", gsTitleCaption, MB_ICONINFORMATION);
				return 0;
			}
			else if (gService.Start() != _SVC_SUCCESS_)
			{
				MessageBox(NULL, "Failed to start the service.", gsTitleCaption, MB_ICONWARNING);
				return 0;
			}
			else {
				MessageBox(NULL, "The service was successfully started.", gsTitleCaption, MB_ICONINFORMATION);
			}

			return 0;
		}
		//------------------------------------------------------------------------------------------
		else if (_strcmpi(sCmdLine, "/VerboseStop") == 0)
		{
			if (gService.IsInstalled() == _SVC_NO_)
			{
				MessageBox(NULL, "The service is not installed.", gsTitleCaption, MB_ICONWARNING);
				return 0;
			}
			else if (gService.IsActive() == _SVC_NO_)
			{
				MessageBox(NULL, "The service is not running.", gsTitleCaption, MB_ICONINFORMATION);
				return 0;
			}
			else if (gService.Stop() != _SVC_SUCCESS_)
			{
				MessageBox(NULL, "Failed to stop the service.", gsTitleCaption, MB_ICONWARNING);
				return 0;
			}
			else {
				MessageBox(NULL, "The service was successfully stopped.", gsTitleCaption, MB_ICONINFORMATION);
			}
			return 0;
		}
#endif
	}

	if (bRunStandalone || bRunService)
	{
		int iResult = 0;

		CManagement *manage = NULL;

		if (gbLoadManagementInterface)
		{
			manage = (CManagement *)pMem->TrackMemory(new CManagement());
			if (!manage->IsInitialized())
			{
				delete (CManagement *)pMem->UnTrackMemory(manage);
				MessageBox(NULL, "Failed to initialize the management interface, is the service already running?", gsTitleCaption, MB_ICONERROR);
				return 1;
			}
		}

		if (!IsApplicationRunning() && gService.IsActive() == _SVC_NO_)
		{
			if (bRunService)
			{
				if (gService.RunAsService() == _SVC_SUCCESS_)
				{
					iResult = 0;
				}
				else {
					iResult = 1;
				}
			}
			else {
#ifdef _DEBUG
				bOpenConsole = true;
#endif

				if (bOpenConsole)
				{
					char sCaption[sizeof(gsTitleCaption) + sizeof(gsFileVersion)];
					sprintf_s(sCaption, "%s - %s", gsTitleCaption, gsFileVersion);

					OpenDebugConsole(sCaption);

#ifdef _DEBUG
					printf("%s (Standalone Debug Mode)\n", sCaption);
#else
					printf("%s (Portable Mode)\n", sCaption);
#endif
					if (gbLoadManagementInterface)
					{
						printf("Management interface on port %d.\n", gdwManagementPort);
					}

					printf("Type \"?\" or \"Help\" for a list of available commands.\n\n");
					StartDebugConsoleHandler(&Fortitude_DebugConsoleHandler);
				}

				gService.RunAsStandAlone();
			}
		}

		if (manage)
		{
			delete (CManagement *)pMem->UnTrackMemory(manage);
		}

		return iResult;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _PORTABLE_RELEASE

int main(int argc, char *argv[])
{
	ghAppInstance = GetModuleHandle(NULL);

	int iReturnVal = 0;

	if (!InitializeApp())
	{
		return 1;
	}

	if (true)
	{
		CommandLineParser cmdLine;

		gService.Initialize(gsServiceName, &ServiceProcedure, NULL);

		cmdLine.AddCommand("/Standalone");

		if (cmdLine.Count() > 1)
		{
			iReturnVal = ProcessCommandLine(&cmdLine);
		}
		else
		{
			MessageBox(NULL, "This service was not intended to run standalone. To force this mode, pass the /Standalone parameter.", gsTitleCaption, MB_ICONINFORMATION);
		}
	}
}

#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevious, LPSTR sCmdLine, int iCmdShow)
{
	ghAppInstance = GetModuleHandle(NULL);

	int iReturnVal = 0;

	if (!InitializeApp())
	{
		return 1;
	}

	if (true)
	{
		CommandLineParser cmdLine;

		gService.Initialize(gsServiceName, &ServiceProcedure, NULL);

		if (cmdLine.Count() > 1)
		{
			iReturnVal = ProcessCommandLine(&cmdLine);
		}
		else
		{
			MessageBox(NULL, "This service was not intended to run standalone. To force this mode, pass the /Standalone parameter.", gsTitleCaption, MB_ICONINFORMATION);
		}
	}

#ifdef _DEBUG
	CloseDebugConsole();
#endif

	UninitializeApp();

	return iReturnVal;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

