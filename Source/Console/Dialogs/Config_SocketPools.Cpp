///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _Config_SocketPools_CPP
#define _Config_SocketPools_CPP
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
#include "../../@Common/SocketPools.H"

#include "MainDlg.H"
#include "NetLogoDlg.H"

#include "Config_SocketPools.H"

#include "Config_SocketPools.H"
#include "Config_SocketPools_Item.H"
#include "CommandWaitDialog.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK Config_SocketPools(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

		XMLReader xml;
		if(gManage->QueryXMLReader(&xml, "::Configuration:SocketPools|"))
		{
			PopSocketPoolsList(gCI.hList, false, &xml, true);

			char sText[255];
			sprintf_s(sText, sizeof(sText), "Socket Pools");
			Set_Text(GetDlgItem(hWnd, IDC_CONFIGTITLE), sText);
			SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)sText);

			ShowWindow(GetDlgItem(hWnd, IDC_ENABLED), SW_HIDE);
			ShowWindow(GetDlgItem(hWnd, IDC_USEGLOBAL), SW_HIDE);
			CheckDlgButton(hWnd, IDC_ENABLED, BST_UNCHECKED);
			CheckDlgButton(hWnd, IDC_USEGLOBAL, BST_UNCHECKED);
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
				MAKEINTRESOURCE(IDD_TAB_CONFIG_SOCKETPOOL),
				hWnd, Config_SocketPools_Item);
		}
        else if(wParam == IDC_EDIT)
        {
			CLocalConfigItemStorage localStorage(&gCI);

			if((gCI.iListIndex = LVGetFirstSelectedItemIndex(gCI.hList)) >= 0)
			{
				DialogBox(ghAppInstance,
					MAKEINTRESOURCE(IDD_TAB_CONFIG_SOCKETPOOL),
					hWnd, Config_SocketPools_Item);
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
			//---(Begin Server Update)------------------------------------------------------------------
			XMLWriter xml;
			if(LoadNewSocketPoolsConfiguration(hWnd, gCI.hList, &xml))
			{
				char *xmlText = xml.Text();

				XMLReader xml;
				if(ProcessLengthyCommand(&xml, hWnd,
					"Please wait while the new socket pool configuration is applied. This process can"
					" take a long time depending on the number of connections to dependent websites.",
					"::Update|%d:%s:%s", INVALID_SITE_INDEX, "SocketPools", xmlText))
				{
					if(ProcessXMLResult(hWnd, &xml, NULL, "Server failed to apply configuration!"))
					{
						//Nothing we can do here...
					}
				}
				else {
					MessageBox(hWnd, "The server did not respond to the requested action!", gsTitleCaption, MB_ICONERROR);
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
						DialogBox(ghAppInstance,
							MAKEINTRESOURCE(IDD_TAB_CONFIG_SOCKETPOOL),
							hWnd, Config_SocketPools_Item);
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
        EndDialog(hWnd, FALSE);
        return TRUE;
    }

    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
