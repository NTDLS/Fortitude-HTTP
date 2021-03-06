///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright � NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _Config_ServerSettings_Item_CPP
#define _Config_ServerSettings_Item_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>

#include "../Source/Entry.H"
#include "../Source/ConfigTree.H"
#include "../Source/DetailList.H"
#include "../Source/CManagement.H"

#include "../../@Common/WebSite.H"
#include "../../@Common/ServerSettings.H"
#include "../../../NSWFL/NSWFL.h"

#include "MainDlg.H"
#include "NetLogoDlg.H"

#include "Config_ServerSettings.H"
#include "Config_ServerSettings_User_Item.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListView;
using namespace NSWFL::Hashing;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK Config_ServerSettings_User_Item(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

	static HWND hUsername = NULL;
	static HWND hPassword = NULL;
	static HWND hDescr = NULL;
	static char sOriginalPassword[SERVERS_MAX_PASSWORD_LENGTH];

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
		char sText[MAX_TEXT_SIZE];

		CenterOverOwner(hWnd);

        HMENU hMenu = GetSystemMenu(hWnd, FALSE);
        AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
        AppendMenu(hMenu, MF_STRING, GENERIC_MENU_ABOUT, "About");

        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)));

        hUsername = GetDlgItem(hWnd, IDC_USERNAME);
        hPassword = GetDlgItem(hWnd, IDC_PASSWORD);
        hDescr = GetDlgItem(hWnd, IDC_DESCR);

		if(gCI.iListIndex != INVALID_LIST_INDEX) //Editing an item.
		{
			//We have a list index, so we must be editing.

			LVGetItemText(gCI.hList, gCI.iListIndex, SERVERS_LIST_POS_USERNAME, sText, sizeof(sText));
			Set_Text(hUsername, sText);

			//LVGetItemText(gCI.hList, gCI.iListIndex, SERVERS_LIST_POS_PASSWORD, sText, sizeof(sText));
			LPARAM lAddress = LVGetItemParam(gCI.hList, gCI.iListIndex, 0);
			Set_Text(hPassword, (char *)lAddress);

			LVGetItemText(gCI.hList, gCI.iListIndex, SERVERS_LIST_POS_DESCR, sText, sizeof(sText));
			Set_Text(hDescr, sText);

			CheckDlgButton(hWnd, IDC_ENABLED, LVChecked(gCI.hList, gCI.iListIndex));
		}
		else{
			//Adding an item.
			CheckDlgButton(hWnd, IDC_ENABLED, BST_CHECKED);
		}

		Get_Text(hPassword, sOriginalPassword, sizeof(sOriginalPassword));

		Set_Text(GetDlgItem(hWnd, IDC_CONFIGTITLE), "Server Settings");
        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)"Server Settings");
		
		return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }
    //--------------------------------------------------------------------------
    else if(uMsg == WM_COMMAND)
    {
		if(wParam == IDC_OK)
        {
			SERVERUSER Item;

			char sUsername[SERVERS_MAX_USERNAME_LENGTH];
			char sPassword[SERVERS_MAX_PASSWORD_LENGTH];
			char sDescr[SERVERS_MAX_PASSWORD_LENGTH];

			Get_Text(hUsername, sUsername, sizeof(sUsername));
			Get_Text(hPassword, sPassword, sizeof(sPassword));
			Get_Text(hDescr, sDescr, sizeof(sDescr));

			if(strcmp(sPassword, sOriginalPassword) != 0)
			{
				char sSHA1[48];
				SimpleSHA1(sPassword, sSHA1, sizeof(sSHA1));
				strcpy_s(sPassword, sizeof(sPassword), sSHA1);
			}

			Item.Username = sUsername;
			Item.Password = sPassword;
			Item.Description = sDescr;
			Item.Enabled = (IsDlgButtonChecked(hWnd, IDC_ENABLED) > 0);

			if(gCI.iListIndex != INVALID_LIST_INDEX) //Editing / Adding
			{
				//We have a list index, so we must be editing.
				FreeLParamMemory(gCI.hList, gCI.iListIndex);
				LVDeleteItem(gCI.hList, gCI.iListIndex);
				PopServerSettingsList_User_AddItem(gCI.hList, &Item, true, gCI.iListIndex);
			}
			else {
				//We DO NOT have a list index, so we must be adding.
				PopServerSettingsList_User_AddItem(gCI.hList, &Item, true);
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
