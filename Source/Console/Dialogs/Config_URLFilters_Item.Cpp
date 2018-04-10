///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _Config_URLFilters_Item_CPP
#define _Config_URLFilters_Item_CPP
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
#include "../../@Common/URLFilters.H"

#include "MainDlg.H"
#include "NetLogoDlg.H"

#include "Config_URLFilters.H"
#include "Config_URLFilters_Item.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK Config_URLFilters_Item(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

    static HWND hSequence = NULL;
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

        hSequence	= GetDlgItem(hWnd, IDC_SEQUENCE);
        hDescr	= GetDlgItem(hWnd, IDC_DESCR);

		if(gCI.iListIndex != INVALID_LIST_INDEX) //Editing an item.
		{
			//We have a list index, so we must be editing.

			LVGetItemText(gCI.hList, gCI.iListIndex, URLFILTERS_LIST_POS_SEQ, sText, sizeof(sText));
			Set_Text(hSequence, sText);

			LVGetItemText(gCI.hList, gCI.iListIndex, URLFILTERS_LIST_POS_DESCR, sText, sizeof(sText));
			Set_Text(hDescr, sText);

			LVGetItemText(gCI.hList, gCI.iListIndex, URLFILTERS_LIST_POS_SCANREQUEST, sText, sizeof(sText));
			CheckDlgButton(hWnd, IDC_SCANREQUEST, ParseBoolean(sText));

			LVGetItemText(gCI.hList, gCI.iListIndex, URLFILTERS_LIST_POS_SCANQUERY, sText, sizeof(sText));
			CheckDlgButton(hWnd, IDC_SCANQUERY, ParseBoolean(sText));

			CheckDlgButton(hWnd, IDC_ENABLED, LVChecked(gCI.hList, gCI.iListIndex));
		}
		else{
			//Adding an item.
			CheckDlgButton(hWnd, IDC_ENABLED, BST_CHECKED);
			CheckDlgButton(hWnd, IDC_SCANQUERY, BST_CHECKED);
			CheckDlgButton(hWnd, IDC_SCANREQUEST, BST_CHECKED);
		}

		sprintf_s(sText, sizeof(sText), "%s%s", gCI.iSite == INVALID_SITE_INDEX ? "Global " : "", "URL Filter");
		Set_Text(GetDlgItem(hWnd, IDC_CONFIGTITLE), sText);
        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)sText);
		
		return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }
    //--------------------------------------------------------------------------
    else if(uMsg == WM_COMMAND)
    {
		if(wParam == IDC_OK)
        {
			URLFILTER Item;

			char sSequence[URLFILTERS_MAX_SEQ_LENGTH];
			char sDescr[URLFILTERS_MAX_DESCR_LENGTH];

			Get_Text(hSequence, sSequence, sizeof(sSequence));
			Get_Text(hDescr, sDescr, sizeof(sDescr));

			if(strlen(sSequence) == 0)
			{
				MessageBox(hWnd, "You must specify a filter sequence.", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}

			Item.Sequence = sSequence;
			Item.Description = sDescr;
			Item.ScanQuery = (IsDlgButtonChecked(hWnd, IDC_SCANQUERY) == BST_CHECKED);
			Item.ScanRequest = (IsDlgButtonChecked(hWnd, IDC_SCANREQUEST) == BST_CHECKED);
			Item.Enabled = (IsDlgButtonChecked(hWnd, IDC_ENABLED) > 0);

			if(gCI.iListIndex != INVALID_LIST_INDEX) //Editing / Adding
			{
				//We have a list index, so we must be editing.
				LVDeleteItem(gCI.hList, gCI.iListIndex);
				PopURLFiltersList_AddItem(gCI.hList, &Item, gCI.iListIndex);
			}
			else {
				//We DO NOT have a list index, so we must be adding.
				PopURLFiltersList_AddItem(gCI.hList, &Item);
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
