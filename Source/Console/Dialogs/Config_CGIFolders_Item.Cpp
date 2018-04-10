///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _Config_CGIFolders_Item_CPP
#define _Config_CGIFolders_Item_CPP
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
#include "../../@Common/CGIFolders.H"

#include "MainDlg.H"
#include "NetLogoDlg.H"

#include "Config_CGIFolders.H"

#include "Config_CGIFolders.H"
#include "Config_CGIFolders_Item.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListView;
using namespace NSWFL::File;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK Config_CGIFolders_Item(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

    static HWND hName = NULL;
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

        hName	= GetDlgItem(hWnd, IDC_NAME);
        hDescr	= GetDlgItem(hWnd, IDC_DESCR);

		if(gCI.iListIndex != INVALID_LIST_INDEX) //Editing an item.
		{
			//We have a list index, so we must be editing.

			LVGetItemText(gCI.hList, gCI.iListIndex, CGIFOLDERS_LIST_POS_PATH, sText, sizeof(sText));
			Set_Text(hName, sText);

			LVGetItemText(gCI.hList, gCI.iListIndex, CGIFOLDER_LIST_POS_DESCR, sText, sizeof(sText));
			Set_Text(hDescr, sText);

			CheckDlgButton(hWnd, IDC_ENABLED, LVChecked(gCI.hList, gCI.iListIndex));
		}
		else{
			//Adding an item.
			CheckDlgButton(hWnd, IDC_ENABLED, BST_CHECKED);
		}

		sprintf_s(sText, sizeof(sText), "%s%s", gCI.iSite == INVALID_SITE_INDEX ? "Global " : "", "CGI Folder");
		Set_Text(GetDlgItem(hWnd, IDC_CONFIGTITLE), sText);
        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)sText);
		
		return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }
    //--------------------------------------------------------------------------
    else if(uMsg == WM_COMMAND)
    {
		if(wParam == IDC_OK)
        {
			CGIFOLDER Item;

			char sPath[CGIFOLDER_MAX_PATH_LENGTH];
			char sDescr[CGIFOLDER_MAX_DESCR_LENGTH];

			Get_Text(hName, sPath, sizeof(sPath));
			Get_Text(hDescr, sDescr, sizeof(sDescr));

			if(strlen(sPath) == 0)
			{
				MessageBox(hWnd, "You must specify a web path.", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}

			CorrectForwardPath(sPath, sizeof(sPath));
			if(sPath[0] != '/')
			{
				char sTmp[sizeof(sPath)];
				sprintf_s(sTmp, sizeof(sTmp), "/%s", sPath);
				strcpy_s(sPath, sizeof(sPath), sTmp);
			}

			Item.Description = sDescr;
			Item.Path = sPath;
			Item.Enabled = (IsDlgButtonChecked(hWnd, IDC_ENABLED) > 0);

			if(gCI.iListIndex != INVALID_LIST_INDEX) //Editing / Adding
			{
				//We have a list index, so we must be editing.
				LVDeleteItem(gCI.hList, gCI.iListIndex);
				PopCGIFoldersList_AddItem(gCI.hList, &Item, gCI.iListIndex);
			}
			else {
				//We DO NOT have a list index, so we must be adding.
				PopCGIFoldersList_AddItem(gCI.hList, &Item);
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
