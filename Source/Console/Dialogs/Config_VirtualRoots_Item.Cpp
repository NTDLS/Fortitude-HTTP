///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _Config_VirtualRoots_Item_CPP
#define _Config_VirtualRoots_Item_CPP
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
#include "../../@Common/VirtualRoots.H"
#include "../../@Common/Cryptography.h"
#include "../../../NSWFL/NSWFL.h"

#include "MainDlg.H"
#include "NetLogoDlg.H"

#include "Config_VirtualRoots.H"
#include "Config_VirtualRoots_Item.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListView;
using namespace NSWFL::File;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK Config_VirtualRoots_Item_Username_Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	if(uMsg == WM_KEYDOWN)
	{
		Set_Text(GetDlgItem(GetParent(hWnd), IDC_NTPASSWORD), "");
	}
	else if(uMsg == WM_DESTROY)
	{
		RemoveWindowSubclass(hWnd, Config_VirtualRoots_Item_Username_Proc, uIdSubclass);
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK Config_VirtualRoots_Item_Domain_Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	if(uMsg == WM_KEYDOWN)
	{
		Set_Text(GetDlgItem(GetParent(hWnd), IDC_NTPASSWORD), "");
	}
	else if(uMsg == WM_DESTROY)
	{
		RemoveWindowSubclass(hWnd, Config_VirtualRoots_Item_Domain_Proc, uIdSubclass);
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK Config_VirtualRoots_Item(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

    static HWND hRoot = NULL;
    static HWND hPath = NULL;
    static HWND hDescr = NULL;
    static HWND hNTUser = NULL;
    static HWND hNTDomain = NULL;
    static HWND hNTPass = NULL;
	static char sOriginalPassword[VIRTUALROOTS_MAX_PASS_LENGTH];

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
		char sText[MAX_TEXT_SIZE];
		CenterOverOwner(hWnd);

        HMENU hMenu = GetSystemMenu(hWnd, FALSE);
        AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
        AppendMenu(hMenu, MF_STRING, GENERIC_MENU_ABOUT, "About");

        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)));

        hRoot		= GetDlgItem(hWnd, IDC_ROOT);
        hPath		= GetDlgItem(hWnd, IDC_PATH);
        hDescr		= GetDlgItem(hWnd, IDC_DESCR);
        hNTUser		= GetDlgItem(hWnd, IDC_NTUSERNAME);
        hNTDomain	= GetDlgItem(hWnd, IDC_NTDOMAIN);
        hNTPass 	= GetDlgItem(hWnd, IDC_NTPASSWORD);

		if(gCI.iListIndex != INVALID_LIST_INDEX) //Editing an item.
		{
			//We have a list index, so we must be editing.

			LVGetItemText(gCI.hList, gCI.iListIndex, VIRTUALROOTS_LIST_POS_ROOT, sText, sizeof(sText));
			Set_Text(hRoot, sText);

			LVGetItemText(gCI.hList, gCI.iListIndex, VIRTUALROOTS_LIST_POS_PATH, sText, sizeof(sText));
			Set_Text(hPath, sText);

			LVGetItemText(gCI.hList, gCI.iListIndex, VIRTUALROOTS_LIST_POS_DESCR, sText, sizeof(sText));
			Set_Text(hDescr, sText);

			LVGetItemText(gCI.hList, gCI.iListIndex, VIRTUALROOTS_LIST_POS_USER, sText, sizeof(sText));
			Set_Text(hNTUser, sText);
			bool bEnableAuth = (strlen(sText) > 0);

			LVGetItemText(gCI.hList, gCI.iListIndex, VIRTUALROOTS_LIST_POS_DOMAIN, sText, sizeof(sText));
			Set_Text(hNTDomain, sText);

			//LVGetItemText(gCI.hList, gCI.iListIndex, VIRTUALROOTS_LIST_POS_PASS, sText, sizeof(sText));
			LPARAM lAddress = LVGetItemParam(gCI.hList, gCI.iListIndex, 0);
			Set_Text(hNTPass, (char *)lAddress);
			strcpy_s(sOriginalPassword, sizeof(sOriginalPassword), (char *)lAddress);

			if(bEnableAuth)
			{
				CheckDlgButton(hWnd, IDC_NTAUTHENABLED, TRUE);
				SendMessage(hWnd, WM_COMMAND, IDC_NTAUTHENABLED, 0);
			}

			CheckDlgButton(hWnd, IDC_ENABLED, LVChecked(gCI.hList, gCI.iListIndex));
		}
		else{
			//Adding an item.
			CheckDlgButton(hWnd, IDC_ENABLED, BST_CHECKED);
		}

		/*
		if(gCI.iSite != INVALID_SITE_INDEX)
		{
			EnableWindow(GetDlgItem(hWnd, IDC_BROWSE), FALSE);
		}
		*/

		sprintf_s(sText, sizeof(sText), "%s%s", gCI.iSite == INVALID_SITE_INDEX ? "Global " : "", "Virtual Root");
		Set_Text(GetDlgItem(hWnd, IDC_CONFIGTITLE), sText);
        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)sText);

		SetWindowSubclass(hNTUser, Config_VirtualRoots_Item_Username_Proc, 0, 0);
		SetWindowSubclass(hNTDomain, Config_VirtualRoots_Item_Domain_Proc, 0, 0);
		
		return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }
    //--------------------------------------------------------------------------
	else if(uMsg == WM_COMMAND)
    {
		if(wParam == IDC_BROWSE)
		{
			char sFolder[MAX_PATH];
			Get_Text(hPath, sFolder, sizeof(sFolder));
			if(BrowseDirectory(hWnd, "Select a  directory literal path for the virtual root.",
				BIF_RETURNONLYFSDIRS | BIF_NONEWFOLDERBUTTON, sFolder, sizeof(sFolder), sFolder))
			{
				Set_Text(hPath, sFolder);
			}
		}
		else if(wParam == IDC_NTAUTHENABLED)
        {
			BOOL bEnable = IsDlgButtonChecked(hWnd, IDC_NTAUTHENABLED);
			EnableWindow(hNTUser, bEnable);
			EnableWindow(hNTDomain, bEnable);
			EnableWindow(hNTPass, bEnable);
		}
		else if(wParam == IDC_OK)
        {
			ROOT Item;

			char sURI[VIRTUALROOTS_MAX_ROOT_LENGTH];
			char sPath[VIRTUALROOTS_MAX_PATH_LENGTH];
			char sDescr[VIRTUALROOTS_MAX_DESCR_LENGTH];

			Get_Text(hRoot, sURI, sizeof(sURI));
			Get_Text(hPath, sPath, sizeof(sPath));
			Get_Text(hDescr, sDescr, sizeof(sDescr));

			if(strlen(sURI) == 0)
			{
				MessageBox(hWnd, "You must specify a web path.", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}
			if(strlen(sPath) == 0)
			{
				MessageBox(hWnd, "You must specify a file path.", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}

			CorrectForwardPath(sURI, sizeof(sURI));
			if(sURI[strlen(sURI) - 1] != '/')
			{
				strcat_s(sURI, sizeof(sURI), "/");
			}
			if(sURI[0] != '/')
			{
				char sTmp[sizeof(sURI)];
				sprintf_s(sTmp, sizeof(sTmp), "/%s", sURI);
				strcpy_s(sURI, sizeof(sURI), sTmp);
			}

			if (IsDlgButtonChecked(hWnd, IDC_NTAUTHENABLED))
			{
				char sUsername[VIRTUALROOTS_MAX_USER_LENGTH];
				char sDomain[VIRTUALROOTS_MAX_DOMAIN_LENGTH];
				char sPassword[VIRTUALROOTS_MAX_PASS_LENGTH];

				Get_Text(hNTUser, sUsername, sizeof(sUsername));
				Get_Text(hNTDomain, sDomain, sizeof(sDomain));
				Get_Text(hNTPass, sPassword, sizeof(sPassword));

				Item.Username = sUsername;
				Item.Domain = sDomain;
				Item.Password = sPassword;
			}
			else{
				Item.Username = "";
				Item.Domain = "";
				Item.Password = "";
			}

			Item.Root = sURI;
			Item.Path = sPath;
			Item.Description = sDescr;
			Item.Enabled = (IsDlgButtonChecked(hWnd, IDC_ENABLED) > 0);

			if(gCI.iListIndex != INVALID_LIST_INDEX) //Editing / Adding
			{
				//We have a list index, so we must be editing.
				FreeLParamMemory(gCI.hList, gCI.iListIndex);
				LVDeleteItem(gCI.hList, gCI.iListIndex);
				PopVirtualRootsList_AddItem(gCI.hList, &Item, true, gCI.iListIndex);
			}
			else {
				//We DO NOT have a list index, so we must be adding.
				PopVirtualRootsList_AddItem(gCI.hList, &Item, true);
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
