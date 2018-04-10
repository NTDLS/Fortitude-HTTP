///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _Config_ExtensionFilterss_CPP
#define _Config_ExtensionFilters_CPP
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
#include "../../@Common/IPFilters.H"

#include "MainDlg.H"
#include "NetLogoDlg.H"

#include "Config_ExtensionFilters.H"
#include "Config_ExtensionFilters_Item.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK Config_ExtensionFilters(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

    static HWND hEnabled = NULL;

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
        hEnabled = GetDlgItem(hWnd, IDC_ENABLED);

		CXMLReader xml;
		if(gManage->QueryXMLReader(&xml, "::Configuration:ExtensionFilters|%d", gCI.iSite))
		{
			if(xml.ToBoolean("Enable"))
			{
				CheckDlgButton(hWnd, IDC_ENABLED, BST_CHECKED);
			}

			PopExtensionFiltersList(gCI.hList, false, &xml);

			char sText[255];
			sprintf_s(sText, sizeof(sText), "%s%s", gCI.iSite == INVALID_SITE_INDEX ? "Global " : "", "Extension Filters");
			Set_Text(GetDlgItem(hWnd, IDC_CONFIGTITLE), sText);
			SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)sText);

			if(gCI.iSite == INVALID_SITE_INDEX)
			{
				ShowWindow(GetDlgItem(hWnd, IDC_USEGLOBAL), SW_HIDE);
				CheckDlgButton(hWnd, IDC_USEGLOBAL, BST_UNCHECKED);
			}
			else{
				if(gManage->QueryBoolean(&gCI.bGlobal, "::IsGlobal|%d:ExtensionFilters", gCI.iSite))
				{
					if(gCI.bGlobal)
					{
						CheckDlgButton(hWnd, IDC_USEGLOBAL, BST_CHECKED);
					}
					else{
						CheckDlgButton(hWnd, IDC_USEGLOBAL, BST_UNCHECKED);
					}
					SendMessage(hWnd, WM_COMMAND, IDC_USEGLOBAL, 0);
				}
				else{
					MessageBox(NULL, "QueryBoolean failure.", gsTitleCaption, MB_ICONERROR);
				}
			}
		}
		
        return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }
    //--------------------------------------------------------------------------
    else if(uMsg == WM_COMMAND)
    {
		if(wParam == IDC_USEGLOBAL)
		{
			bool bChecked = IsDlgButtonChecked(hWnd, IDC_USEGLOBAL) > 0;
			EnableWindow(gCI.hList, !bChecked);
			EnableWindow(GetDlgItem(hWnd, IDC_ENABLED), !bChecked);
			EnableWindow(GetDlgItem(hWnd, IDC_ADD), !bChecked);
			EnableWindow(GetDlgItem(hWnd, IDC_EDIT), !bChecked);
			EnableWindow(GetDlgItem(hWnd, IDC_DELETE), !bChecked);
			EnableWindow(GetDlgItem(hWnd, IDC_MOVEUP), !bChecked);
			EnableWindow(GetDlgItem(hWnd, IDC_MOVEDOWN), !bChecked);
		}
        else if(wParam == IDC_ADD)
        {
			CLocalConfigItemStorage localStorage(&gCI);

			gCI.iListIndex = INVALID_LIST_INDEX;

			DialogBox(ghAppInstance,
				MAKEINTRESOURCE(IDD_CONFIG_EXTENSIONFILTERS_ITEM),
				hWnd, Config_ExtensionFilters_Item);
		}
        else if(wParam == IDC_EDIT)
        {
			CLocalConfigItemStorage localStorage(&gCI);

			if((gCI.iListIndex = LVGetFirstSelectedItemIndex(gCI.hList)) >= 0)
			{
				DialogBox(ghAppInstance,
					MAKEINTRESOURCE(IDD_CONFIG_EXTENSIONFILTERS_ITEM),
					hWnd, Config_ExtensionFilters_Item);
			}
        }
        else if(wParam == IDC_DELETE)
        {
			LVDeleteItemFriendly(gCI.hList);
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
			//---(Begin Server Update)------------------------------------------------------------------
			if(IsDlgButtonChecked(hWnd, IDC_USEGLOBAL) == FALSE)
			{
				CXMLWriter xml;
				if(LoadNewExtensionFiltersConfiguration(hWnd, gCI.hList, &xml))
				{
					char *xmlText = xml.Text();

					CXMLReader xml;
					if(gManage->QueryXMLReader(&xml, "::Update|%d:%s:%s", gCI.iSite, "ExtensionFilters", xmlText))
					{
						ProcessXMLResult(hWnd, &xml, NULL, "Failed to apply configuration!");
					}
					else {
						MessageBox(hWnd, "Failed to post configuration to server.", gsTitleCaption, MB_ICONERROR);
					}

					free(xmlText);
				}
			}
			else {
				if(!gCI.bGlobal)
				{
					CXMLReader xml;
					if(gManage->QueryXMLReader(&xml, "::Update|%d:%s:%s", gCI.iSite, "ExtensionFilters", "~Global"))
					{
						ProcessXMLResult(hWnd, &xml, NULL, "Failed to apply configuration!");
					}
					else {
						MessageBox(hWnd, "Failed to post configuration to server.", gsTitleCaption, MB_ICONERROR);
					}
				}
				else {
					//This site uses global settings and the global check box is checked - Nothing to do.
				}

			}
			//---(End Server Update)------------------------------------------------------------------

			EndDialog(hWnd, TRUE);
        }
        else if(wParam == IDC_CANCEL || wParam == IDCANCEL)
        {
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
						DialogBox(ghAppInstance,
							MAKEINTRESOURCE(IDD_CONFIG_EXTENSIONFILTERS_ITEM),
							hWnd, Config_ExtensionFilters_Item);
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
        EndDialog(hWnd, FALSE);

        return TRUE;
    }

    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
