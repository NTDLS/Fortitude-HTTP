///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _Config_SSIFiles_Item_CPP
#define _Config_SSIFiles_Item_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>

#include "../Source/Entry.H"
#include "../Source/ConfigTree.H"
#include "../Source/DetailList.H"
#include "../Source/CManagement.H"

#include "../../@Common/WebSite.H"
#include "../../@Common/SSIFiles.h"

#include "MainDlg.H"
#include "NetLogoDlg.H"

#include "Config_SSIFiles.H"
#include "Config_SSIFiles_Item.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK Config_SSIFiles_Item(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

    static HWND hExtension = NULL;
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

        hExtension	= GetDlgItem(hWnd, IDC_EXTENSION);
        hDescr	= GetDlgItem(hWnd, IDC_DESCR);

		if(gCI.iListIndex != INVALID_LIST_INDEX) //Editing an item.
		{
			//We have a list index, so we must be editing.

			LVGetItemText(gCI.hList, gCI.iListIndex, SSIFILES_LIST_POS_EXT, sText, sizeof(sText));
			Set_Text(hExtension, sText);

			LVGetItemText(gCI.hList, gCI.iListIndex, SSIFILES_LIST_POS_DESCR, sText, sizeof(sText));
			Set_Text(hDescr, sText);

			CheckDlgButton(hWnd, IDC_ENABLED, LVChecked(gCI.hList, gCI.iListIndex));
		}
		else{
			//Adding an item.
			CheckDlgButton(hWnd, IDC_ENABLED, BST_CHECKED);
		}

		sprintf_s(sText, sizeof(sText), "%s%s", gCI.iSite == INVALID_SITE_INDEX ? "Global " : "", "SSI File");
		Set_Text(GetDlgItem(hWnd, IDC_CONFIGTITLE), sText);
        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)sText);
		
		return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }
    //--------------------------------------------------------------------------
    else if(uMsg == WM_COMMAND)
    {
		if(wParam == IDC_OK)
        {
			SSIFILE Item;

			char sExt[SSIFILES_MAX_EXT_LENGTH];
			char sDescr[SSIFILES_MAX_DESCR_LENGTH];

			Get_Text(hExtension, sExt, sizeof(sExt));
			Get_Text(hDescr, sDescr, sizeof(sDescr));

			if(strlen(sExt) == 0)
			{
				MessageBox(hWnd, "You must specify a file extension.", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}

			if(sExt[0] != '.')
			{
				char sTmp[sizeof(sExt)];
				sprintf_s(sTmp, sizeof(sTmp), ".%s", sExt);
				strcpy_s(sExt, sizeof(sExt), sTmp);
			}

			Item.Extension = sExt;
			Item.Description = sDescr;
			Item.Enabled = (IsDlgButtonChecked(hWnd, IDC_ENABLED) > 0);

			if(gCI.iListIndex != INVALID_LIST_INDEX) //Editing / Adding
			{
				//We have a list index, so we must be editing.
				LVDeleteItem(gCI.hList, gCI.iListIndex);
				PopSSIFilesList_AddItem(gCI.hList, &Item, gCI.iListIndex);
			}
			else {
				//We DO NOT have a list index, so we must be adding.
				PopSSIFilesList_AddItem(gCI.hList, &Item);
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
