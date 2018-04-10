///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _Config_ServerSettings_CPP
#define _Config_ServerSettings_CPP
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
#include "../../@Common/ServerSettings.H"

#include "MainDlg.H"
#include "NetLogoDlg.H"

#include "Config_ServerSettings.H"
#include "Config_ServerSettings_User_Item.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK Config_ServerSettings_Users(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
		gCI.hDialog = hWnd;

		CenterOverOwner(hWnd);

        HMENU hMenu = GetSystemMenu(hWnd, FALSE);
        AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
        AppendMenu(hMenu, MF_STRING, GENERIC_MENU_ABOUT, "About");

        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)));

        gCI.hList = GetDlgItem(hWnd, IDC_LIST);

		Set_Text(GetDlgItem(hWnd, IDC_CONFIGTITLE), "Server Settings");
		SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)"Server Settings");

		CXMLReader xml;
		if(gManage->QueryXMLReader(&xml, "::Configuration:ServerSettings|"))
		{
			if(xml.ToBoolean("AllowRemoteManagement"))
			{
				CheckDlgButton(hWnd, IDC_ALLOWREMOTEMGMT, BST_CHECKED);
			}

			PopServerSettingsList_User(gCI.hList, false, &xml);
		}
        return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }
    //--------------------------------------------------------------------------
    else if(uMsg == WM_COMMAND)
    {
        if(wParam == IDC_ADD)
        {
			CLocalConfigItemStorage localStorage(&gCI);

			gCI.iListIndex = INVALID_LIST_INDEX;

			DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_TAB_SERVER_SETTINGS_USER), hWnd, Config_ServerSettings_User_Item);
		}
        else if(wParam == IDC_EDIT)
        {
			CLocalConfigItemStorage localStorage(&gCI);

			if((gCI.iListIndex = LVGetFirstSelectedItemIndex(gCI.hList)) >= 0)
			{
				DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_TAB_SERVER_SETTINGS_USER), hWnd, Config_ServerSettings_User_Item);
			}
        }
        else if(wParam == IDC_DELETE)
        {
			int iItem = LVGetFirstSelectedItemIndex(gCI.hList);
			FreeLParamMemory(gCI.hList, iItem);
			LVDeleteItemFriendly(gCI.hList, iItem);
        }
        else if(wParam == IDC_MOVEUP)
        {
			LVMoveItemUp(gCI.hList);
        }
        else if(wParam == IDC_MOVEDOWN)
        {
			LVMoveItemDown(gCI.hList);
        }
		else if(wParam == IDC_OK)
        {
			if(LVItemCount(gCI.hList) == 0)
			{
				MessageBox(hWnd, "You must define at least one user or you will not be able to manage the server.", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}

			//---(Begin Server Update)------------------------------------------------------------------
			CXMLWriter xml;
			if(LoadNewServerSettings_User_Configuration(hWnd, gCI.hList, &xml))
			{
				char *xmlText = xml.Text();

				CXMLReader xml;
				if(gManage->QueryXMLReader(&xml, "::Update|%d:%s:%s", gCI.iSite, "ServerSettings", xmlText))
				{
					ProcessXMLResult(hWnd, &xml, NULL, "Failed to apply configuration!");
				}
				else {
					MessageBox(hWnd, "Failed to post configuration to server.", gsTitleCaption, MB_ICONERROR);
				}

				free(xmlText);
			}
			//---(End Server Update)------------------------------------------------------------------

			FreeLParamMemory(gCI.hList);
			EndDialog(hWnd, TRUE);
        }
        else if(wParam == IDC_CANCEL || wParam == IDCANCEL)
        {
			FreeLParamMemory(gCI.hList);
            EndDialog(hWnd, FALSE);
        }

		return FALSE;
    }
	//--------------------------------------------------------------------------
	else if(uMsg == WM_NOTIFY)
    {
        LPNMHDR pHdr = (LPNMHDR)lParam;

		if(pHdr->hwndFrom == gCI.hList)
        {
			if(pHdr->code == NM_DBLCLK)
			{
				CLocalConfigItemStorage localStorage(&gCI);

				if(ListView_GetItemCount(gCI.hList) > 0 && ListView_GetSelectedCount(gCI.hList) == 1)
				{
					if((gCI.iListIndex = LVGetFirstSelectedItemIndex(gCI.hList)) >= 0)
					{
						DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_TAB_SERVER_SETTINGS_USER), hWnd, Config_ServerSettings_User_Item);
					}
				}
			}
		}
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
		FreeLParamMemory(gCI.hList);
		EndDialog(hWnd, 0);
        return TRUE;
    }

    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
