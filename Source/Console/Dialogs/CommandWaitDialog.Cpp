///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CommandWaitDialog_CPP
#define _CommandWaitDialog_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <Process.H>
#include <shlobj.h>

#include "../Source/Entry.H"
#include "../Source/ConfigTree.H"
#include "../Source/DetailList.H"
#include "../Source/CManagement.H"

#include "../../@Common/WebSite.H"

#include "MainDlg.H"
#include "NetLogoDlg.H"

#include "CommandWaitDialog.H"

using namespace NSWFL::String;
using namespace NSWFL::XML;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HWND hCommandWaitDialog = NULL;
HANDLE hCommandWaitDialogThread = NULL;
XMLReader *pCommandWaitDialogXML = NULL;
char *sCommandWaitDialogCommand = NULL;
const char *sCommandWaitDialogText = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned __stdcall CommandWaitDialogThread(void *pData)
{
	BOOL bResult = gManage->QueryXMLReader(pCommandWaitDialogXML, sCommandWaitDialogCommand);
	Sleep(500);
	EndDialog(hCommandWaitDialog, bResult);
	_endthreadex(0); return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ProcessLengthyCommand(XMLReader *pXML, HWND hOwner, const char *sText, const char *sCommandText, ...)
{
	bool bResult = false;

	va_list ArgList;
	va_start(ArgList, sCommandText);

	int iMemoryRequired = _vscprintf(sCommandText, ArgList);
	sCommandWaitDialogCommand = (char *) pMem->Allocate(sizeof(char), iMemoryRequired + 1);
	int iSz = _vsprintf_s_l(sCommandWaitDialogCommand, iMemoryRequired + 1, sCommandText, NULL, ArgList);
	va_end(ArgList);
	if(iSz > 0)
	{
		pCommandWaitDialogXML = pXML;
		sCommandWaitDialogText = sText;

		hCommandWaitDialogThread = (HANDLE)	_beginthreadex(NULL, 0, CommandWaitDialogThread, (LPVOID) NULL, CREATE_SUSPENDED, NULL);

		bResult = (DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_COMMAND_WAIT), hOwner, CommandWaitDialog) != 0);

		Sleep(100);

		CloseHandle(hCommandWaitDialogThread);
	}
	pMem->Free(sCommandWaitDialogCommand);

	hCommandWaitDialog = NULL;
	hCommandWaitDialogThread = NULL;
	sCommandWaitDialogCommand = NULL;
	sCommandWaitDialogText = NULL;
	pCommandWaitDialogXML = NULL;

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK CommandWaitDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND hAnimation = NULL;
    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
		hCommandWaitDialog = hWnd;

        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)));

		hAnimation = GetDlgItem(hWnd, IDC_WORKING);
		Animate_Open(hAnimation, MAKEINTRESOURCE(IDR_WORKING));
			
		Set_Text(GetDlgItem(hWnd, IDC_BODYTEXT), sCommandWaitDialogText);

		EnableDialogCloseButton(hWnd, false);
		CenterOverOwner(hWnd);

		ResumeThread(hCommandWaitDialogThread);

		ShowWindow(hAnimation, SW_SHOW);

        return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }

    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
