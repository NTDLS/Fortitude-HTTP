///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _Config_ErrorPages_Item_CPP
#define _Config_ErrorPages_Item_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <shlobj.h>

#include "../Source/Entry.H"
#include "../Source/ConfigTree.H"
#include "../Source/DetailList.H"
#include "../Source/CManagement.H"

#include "../../@Common/WebSite.H"
#include "../../@Common/ErrorPages.H"

#include "MainDlg.H"
#include "NetLogoDlg.H"

#include "Config_ErrorPages.H"
#include "Config_ErrorPages_Item.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListView;
using namespace NSWFL::File;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK ReturnCodeProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	if(uMsg == WM_KILLFOCUS)
	{
		SendMessage(GetParent(hWnd), WM_COMMAND, IDC_RETURNCODE, 0);
	}
	else if(uMsg == WM_DESTROY)
	{
		RemoveWindowSubclass(hWnd, ReturnCodeProc, uIdSubclass);
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK Config_ErrorPages_Item(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

    static HWND hCode = NULL;
    static HWND hReturnCode = NULL;
	static HWND hReturnDesc = NULL;
    static HWND hFile = NULL;
    static HWND hDescr = NULL;

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
		char sText[MAX_TEXT_SIZE];
		CenterOverOwner(hWnd);

        HMENU hMenu = GetSystemMenu(hWnd, FALSE);
        AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
        AppendMenu(hMenu, MF_STRING, GENERIC_MENU_ABOUT, "About");

        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)));

        hCode		= GetDlgItem(hWnd, IDC_CODE);
        hReturnCode	= GetDlgItem(hWnd, IDC_RETURNCODE);
        hReturnDesc	= GetDlgItem(hWnd, IDC_RETURNDESC);
        hFile		= GetDlgItem(hWnd, IDC_FILE);
        hDescr		= GetDlgItem(hWnd, IDC_DESCR);

		if(gCI.iListIndex != INVALID_LIST_INDEX) //Editing an item.
		{
			//We have a list index, so we must be editing.
			LVGetItemText(gCI.hList, gCI.iListIndex, ERRORPAGES_LIST_POS_CODE, sText, sizeof(sText));
			Set_Text(hCode, sText);

			LVGetItemText(gCI.hList, gCI.iListIndex, ERRORPAGES_LIST_POS_RETURNCODE, sText, sizeof(sText));
			Set_Text(hReturnCode, sText);

			LVGetItemText(gCI.hList, gCI.iListIndex, ERRORPAGES_LIST_POS_FILE, sText, sizeof(sText));
			Set_Text(hFile, sText);

			LVGetItemText(gCI.hList, gCI.iListIndex, ERRORPAGES_LIST_POS_DESCR, sText, sizeof(sText));
			Set_Text(hDescr, sText);

			CheckDlgButton(hWnd, IDC_ENABLED, LVChecked(gCI.hList, gCI.iListIndex));
		}
		else{
			//Adding an item.
			CheckDlgButton(hWnd, IDC_ENABLED, BST_CHECKED);
		}

		sprintf_s(sText, sizeof(sText), "%s%s", gCI.iSite == INVALID_SITE_INDEX ? "Global " : "", "Error Page");
		Set_Text(GetDlgItem(hWnd, IDC_CONFIGTITLE), sText);
        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)sText);

		SetWindowSubclass(hReturnCode, ReturnCodeProc, 0, 0);
		SendMessage(hWnd, WM_COMMAND, IDC_RETURNCODE, 0);
	
		return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }
    //--------------------------------------------------------------------------
    else if(uMsg == WM_COMMAND)
    {
		if(wParam == IDC_RETURNCODE)
        {
			//Get the error code text for the override return code.
			int iGridItems = LVItemCount(gCI.hList);
			bool bFound = false;
			char sCode[ERRORPAGES_MAX_CODE_LENGTH];
			char sReturnCode[ERRORPAGES_MAX_CODE_LENGTH];
			Get_Text(hReturnCode, sReturnCode, sizeof(sReturnCode));

			for(int iItem = 0; iItem < iGridItems; iItem++)
			{
				XMLWriter Item("Page");

				LVGetText(gCI.hList, iItem, ERRORPAGES_LIST_POS_CODE, sCode, sizeof(sCode));
				if(_strcmpi(sCode, sReturnCode) == 0)
				{
					char sDescr[ERRORPAGES_MAX_DESCR_LENGTH];
					LVGetText(gCI.hList, iItem, ERRORPAGES_LIST_POS_DESCR, sDescr, sizeof(sDescr));
					Set_Text(hReturnDesc, sDescr);
					bFound = true;
					break;
				}
			}

			if(!bFound)
			{
				Set_Text(hReturnDesc, "<undefined>");
			}
		}
		else if(wParam == IDC_OK)
        {
			ERRORPAGE Item;

			char sCode[ERRORPAGES_MAX_CODE_LENGTH];
			char sReturnCode[ERRORPAGES_MAX_CODE_LENGTH];

			char sFile[ERRORPAGES_MAX_FILE_LENGTH];
			char sDescr[ERRORPAGES_MAX_DESCR_LENGTH];

			Get_Text(hCode, sCode, sizeof(sCode));
			Get_Text(hReturnCode, sReturnCode, sizeof(sReturnCode));
			Get_Text(hFile, sFile, sizeof(sFile));
			Get_Text(hDescr, sDescr, sizeof(sDescr));

			if(strlen(sCode) == 0 || iFormattedValue(sCode) <= 0)
			{
				MessageBox(hWnd, "You must specify an error code.", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}
			if(strlen(sFile) == 0)
			{
				MessageBox(hWnd, "You must specify a file name.", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}

			if(strlen(sReturnCode) == 0)
			{
				//Default to the "error code" when no "return code" is specified.
				strcpy_s(sReturnCode, sizeof(sReturnCode), sCode);
			}

			CorrectForwardPath(sFile, sizeof(sFile));
			if(sFile[0] != '/')
			{
				char sTmp[sizeof(sFile)];
				sprintf_s(sTmp, sizeof(sTmp), "/%s", sFile);
				strcpy_s(sFile, sizeof(sFile), sTmp);
			}

			Item.Description = sDescr;
			Item.Code = sCode;
			Item.ReturnCode = sReturnCode;
			Item.File = sFile;
			Item.Enabled = (IsDlgButtonChecked(hWnd, IDC_ENABLED) > 0);

			if(gCI.iListIndex != INVALID_LIST_INDEX) //Editing / Adding
			{
				//We have a list index, so we must be editing.
				LVDeleteItem(gCI.hList, gCI.iListIndex);
				PopErrorPagesList_AddItem(gCI.hList, &Item, gCI.iListIndex);
			}
			else {
				//We DO NOT have a list index, so we must be adding.
				PopErrorPagesList_AddItem(gCI.hList, &Item);
			}

			LVAutoSizeColumns(gCI.hList, 500);
            EndDialog(hWnd, TRUE);
            DestroyWindow(hWnd);
        }
        else if(wParam == IDC_CANCEL || wParam == IDCANCEL)
        {
            EndDialog(hWnd, FALSE);
            DestroyWindow(hWnd);
        }
        return FALSE;
    }
    //--------------------------------------------------------------------------
	else if(uMsg == WM_PAINT)
    {
        HDC ThisHDC;
        PAINTSTRUCT ThisPS;

        ThisHDC = BeginPaint(hWnd, &ThisPS);

        //- Any painting should be done here.

        EndPaint(hWnd, &ThisPS);
        return TRUE;
    }
    //--------------------------------------------------------------------------
    else if(uMsg == WM_SYSCOMMAND) //- Received a system menu message.
    {
        if(LOWORD(wParam) == GENERIC_MENU_ABOUT) //- About.
        {
            _AboutDialogInfo ADI;
    
            ADI.DisplayIcon  = LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN));
            ADI.TitleCaption = gsTitleCaption;
            ADI.FileVersion  = gsFileVersion;
            ADI.BuildDate    = __DATE__;
            ADI.BuildTime    = __TIME__;
            ADI.CopyRight    = gsAppCopyRight;
            ADI.OwnerHandle  = hWnd;

            NetLogo(&ADI);

            return TRUE;
        }

        return FALSE;
    }
    //--------------------------------------------------------------------------
    else if(uMsg == WM_CLOSE) //- Received close message.
    {
        EndDialog(hWnd, 0);
        DestroyWindow(hWnd);
        return TRUE;
    }
    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
