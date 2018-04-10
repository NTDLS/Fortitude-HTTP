///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _Config_SocketPools_Item_CPP
#define _Config_SocketPools_Item_CPP
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
#include "Config_ListenIPs_Item.H"

#include "../../../../@Libraries/CTabControl/CTabControl.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListView;
using namespace NSWFL::File;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK Config_SocketPools_Item(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

	static CTabControl *TabControl;
	static HWND hTabBasic = NULL;
	static HWND hTabIPs = NULL;
	static HWND hTabAdvanced = NULL;

	static HWND hMaxConnections = NULL;
    static HWND hInitBufferSize = NULL;
    static HWND hMaxBufferSize = NULL;
    static HWND hFileBufferSize = NULL;
    static HWND hPort = NULL;
    static HWND hDescr = NULL;
	static HWND hIPList = NULL;

	static char sUID[SOCKETPOOLS_MAX_UID_LENGTH];

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
		char sText[MAX_TEXT_SIZE];
		CenterOverOwner(hWnd);

        HMENU hMenu = GetSystemMenu(hWnd, FALSE);
        AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
        AppendMenu(hMenu, MF_STRING, GENERIC_MENU_ABOUT, "About");

        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)));

		static LPSTR tabNames[]= {
			"Basic",
			"IP Addresses",
			"Advanced",
			NULL
		};

		static LPSTR dlgNames[] = {
			MAKEINTRESOURCE(IDD_TAB_CONFIG_SOCKETPOOL_BASIC),
			MAKEINTRESOURCE(IDD_TAB_CONFIG_SOCKETPOOL_IPS),
			MAKEINTRESOURCE(IDD_TAB_CONFIG_SOCKETPOOL_ADVANCED),
			NULL
		};

		TabControl = new CTabControl(GetDlgItem(hWnd, IDC_TABCONTROL), tabNames, dlgNames, &Config_SocketPools_Item, TRUE);

		hTabBasic		= TabControl->Handle("Basic");
        hPort			= GetDlgItem(hTabBasic, IDC_PORT);
        hMaxConnections	= GetDlgItem(hTabBasic, IDC_MAXCONNECTIONS);
        hDescr			= GetDlgItem(hTabBasic, IDC_DESCR);

		hTabIPs			= TabControl->Handle("IP Addresses");
		hIPList			= GetDlgItem(hTabIPs, IDC_IPLIST);

		hTabAdvanced	= TabControl->Handle("Advanced");
		hInitBufferSize	= GetDlgItem(hTabAdvanced, IDC_INITBUFFERSIZE);
		hMaxBufferSize	= GetDlgItem(hTabAdvanced, IDC_MAXBUFFERSIZE);
		hFileBufferSize	= GetDlgItem(hTabAdvanced, IDC_FILEBUFFERSIZE);

		if(gCI.iListIndex != INVALID_LIST_INDEX) //Editing an item.
		{
			//We have a list index, so we must be editing.

			LVGetItemText(gCI.hList, gCI.iListIndex, SOCKETPOOLS_LIST_POS_PORT, sText, sizeof(sText));
			Set_Text(hPort, sText);

			LVGetItemText(gCI.hList, gCI.iListIndex, SOCKETPOOLS_LIST_POS_MAXCONNECTIONS, sText, sizeof(sText));
			Set_Text(hMaxConnections, sText);

			LVGetItemText(gCI.hList, gCI.iListIndex, SOCKETPOOLS_LIST_POS_INITBUFFERSIZE, sText, sizeof(sText));
			Set_Text(hInitBufferSize, sText);

			LVGetItemText(gCI.hList, gCI.iListIndex, SOCKETPOOLS_LIST_POS_MAXBUFFERSIZE, sText, sizeof(sText));
			Set_Text(hMaxBufferSize, sText);

			LVGetItemText(gCI.hList, gCI.iListIndex, SOCKETPOOLS_LIST_POS_FILEBUFFERSIZE, sText, sizeof(sText));
			Set_Text(hFileBufferSize, sText);

			LVGetItemText(gCI.hList, gCI.iListIndex, SOCKETPOOLS_LIST_POS_DESCR, sText, sizeof(sText));
			Set_Text(hDescr, sText);

			LVGetItemText(gCI.hList, gCI.iListIndex, SOCKETPOOLS_LIST_POS_UID, sUID, sizeof(sUID));

			LPARAM xmlParam = LVGetItemParam(gCI.hList, gCI.iListIndex, 0);
			const char *sXMLText = (char *)xmlParam;
			if(sXMLText)
			{
				CXMLReader xml(sXMLText);

				PopListeningIPsList(hIPList, false, &xml);

				if(xml.ToBoolean("ListenOnAll"))
				{
					CheckDlgButton(hTabIPs, IDC_LISTENALLIPS, BST_CHECKED);
					SendMessage(hWnd, WM_COMMAND, IDC_LISTENALLIPS, 0);
				}
			}
			else {
				PopListeningIPsList(hIPList, false, NULL); //Pop an empty list (add columns).
				CheckDlgButton(hTabIPs, IDC_LISTENALLIPS, BST_CHECKED);
				SendMessage(hWnd, WM_COMMAND, IDC_LISTENALLIPS, 0);
			}
		}
		else{
			//Adding an item.

			NewGUID(sUID, sizeof(sUID));
			Set_Text(hPort, "");
			Set_LongFormatted(hMaxConnections, FD_SETSIZE / 5);
			Set_LongFormatted(hInitBufferSize, 8 * 1024);
			Set_LongFormatted(hMaxBufferSize, 64 * 1024);
			Set_LongFormatted(hFileBufferSize, 16 * 1024);
			Set_Text(hDescr, "");

			PopListeningIPsList(hIPList, false, NULL);

			CheckDlgButton(hTabIPs, IDC_LISTENALLIPS, BST_CHECKED);
			SendMessage(hWnd, WM_COMMAND, IDC_LISTENALLIPS, 0);
		}

		sprintf_s(sText, sizeof(sText), "Socket Pool");
		Set_Text(GetDlgItem(hWnd, IDC_CONFIGTITLE), sText);
        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)sText);
		
		return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }
    //--------------------------------------------------------------------------
    else if(uMsg == WM_COMMAND)
    {
		if(wParam == IDC_OK)
        {
			SOCKETPOOL Item;

			int iPort = 0;
			int iMaxConnections = 0;
			int iInitialBufferSize = 0;
			int iMaxBufferSize = 0;
			int iFileBufferSize = 0;
			char sDescr[SOCKETPOOLS_MAX_DESCR_LENGTH];
			char sText[MAX_TEXT_SIZE];

			iPort = Get_Long(hPort);
			iMaxConnections = Get_Long(hMaxConnections);
			iInitialBufferSize = Get_Long(hInitBufferSize);
			iMaxBufferSize = Get_Long(hMaxBufferSize);
			iFileBufferSize = Get_Long(hFileBufferSize);

			Get_Text(hDescr, sDescr, sizeof(sDescr));

			if(iPort <= 0 || iPort > 65535)
			{
				MessageBox(hWnd, "You must specify a valid port between 1 and 65,535.", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}

			if(iMaxConnections <= 0 || iMaxConnections > FD_SETSIZE)
			{
				sprintf_s(sText, sizeof(sText), "Max connections must be between 1 and %d.", FD_SETSIZE);
				MessageBox(hWnd, sText, gsTitleCaption, MB_ICONINFORMATION);
				return TRUE;
			}

			if(iInitialBufferSize < 1024 || iInitialBufferSize > 2147483648)
			{
				MessageBox(hWnd, "Buffer size must be between 1,024 and 2,147,483,648.", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}

			if(iMaxBufferSize < 0 || iMaxBufferSize > 2147483648)
			{
				MessageBox(hWnd, "Max buffer size must be between 0 (infinite) and 2,147,483,648.", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}

			if(iFileBufferSize < 1024 || iFileBufferSize > 2147483648)
			{
				MessageBox(hWnd, "File buffer size must be between 1024 and 2,147,483,648.", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}

			Item.UID = sUID;
			Item.Port = iPort;
			Item.Description = sDescr;
			Item.InitialBufferSize = iInitialBufferSize;
			Item.MaxBufferSize = iMaxBufferSize;
			Item.FileBufferSize = iFileBufferSize;
			Item.MaxConnections = iMaxConnections;

			if(gCI.iListIndex != INVALID_LIST_INDEX) //Editing / Adding
			{
				CXMLWriter xmlIPAddresses;
				LoadNewListeningIPsConfiguration(hWnd, hIPList,
					IsDlgButtonChecked(hTabIPs, IDC_LISTENALLIPS) == BST_CHECKED, &xmlIPAddresses);

				char *sXMLText = xmlIPAddresses.Text();

				if(sXMLText)
				{
					//We have a list index, so we must be editing.
					FreeLParamMemory(gCI.hList, gCI.iListIndex);
					LVDeleteItem(gCI.hList, gCI.iListIndex);
					PopSocketPoolsList_AddItem(gCI.hList, &Item, gCI.iListIndex, sXMLText);
					free(sXMLText);
				}
			}
			else {
					CXMLWriter xmlIPAddresses;
					LoadNewListeningIPsConfiguration(hWnd, hIPList,
						IsDlgButtonChecked(hTabIPs, IDC_LISTENALLIPS) == BST_CHECKED, &xmlIPAddresses);

					char *sXMLText = xmlIPAddresses.Text();

					if(sXMLText)
					{
						//We have a list index, so we must be editing.
						FreeLParamMemory(gCI.hList, gCI.iListIndex);
						LVDeleteItem(gCI.hList, gCI.iListIndex);
						PopSocketPoolsList_AddItem(gCI.hList, &Item, sXMLText);
						free(sXMLText);
					}
			}

			LVAutoSizeColumns(gCI.hList, 500);
            EndDialog(hWnd, TRUE);
            DestroyWindow(hWnd);
			delete TabControl;
        }
		else if(wParam == IDC_LISTENALLIPS)
		{
			bool bChecked = IsDlgButtonChecked(hTabIPs, IDC_LISTENALLIPS) > 0;
			EnableWindow(GetDlgItem(hTabIPs, IDC_ADD_IP), !bChecked);
			EnableWindow(GetDlgItem(hTabIPs, IDC_EDIT_IP), !bChecked);
			EnableWindow(GetDlgItem(hTabIPs, IDC_DELETE_IP), !bChecked);
			EnableWindow(GetDlgItem(hTabIPs, IDC_IPLIST), !bChecked);
		}
        else if(wParam == IDC_ADD_IP)
        {
			CLocalConfigItemStorage localStorage(&gCI);

			gCI.hList = hIPList;
			gCI.iListIndex = INVALID_LIST_INDEX;

			DialogBox(ghAppInstance,
				MAKEINTRESOURCE(IDD_CONFIG_LISTENIPS_ITEM),
				hWnd, Config_ListenIPs_Item);
		}
        else if(wParam == IDC_EDIT_IP)
        {
			CLocalConfigItemStorage localStorage(&gCI);

			gCI.hList = hIPList;

			if((gCI.iListIndex = LVGetFirstSelectedItemIndex(gCI.hList)) >= 0)
			{
				DialogBox(ghAppInstance,
					MAKEINTRESOURCE(IDD_CONFIG_LISTENIPS_ITEM),
					hWnd, Config_ListenIPs_Item);
			}
		}
        else if(wParam == IDC_DELETE_IP)
        {
			LVDeleteItemFriendly(hIPList);
        }
		else if(wParam == IDC_CANCEL || wParam == IDCANCEL)
        {
            EndDialog(hWnd, FALSE);
            DestroyWindow(hWnd);
			delete TabControl;
        }
        return FALSE;
    }
	//--------------------------------------------------------------------------
	else if(uMsg == WM_NOTIFY)
    {
        LPNMHDR pHdr = (LPNMHDR)lParam;

		if(pHdr->hwndFrom == hIPList)
        {
			if(pHdr->code == NM_DBLCLK)
			{
				if(ListView_GetItemCount(hIPList) > 0 && ListView_GetSelectedCount(hIPList) == 1)
				{
					CLocalConfigItemStorage localStorage(&gCI);

					gCI.hList = hIPList;

					if((gCI.iListIndex = LVGetFirstSelectedItemIndex(gCI.hList)) >= 0)
					{
						if((gCI.iListIndex = LVGetFirstSelectedItemIndex(gCI.hList)) >= 0)
						{
							DialogBox(ghAppInstance,
								MAKEINTRESOURCE(IDD_CONFIG_LISTENIPS_ITEM),
								hWnd, Config_ListenIPs_Item);
						}
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
        EndDialog(hWnd, 0);
        DestroyWindow(hWnd);
		delete TabControl;
        return TRUE;
    }
    //--------------------------------------------------------------------------

	if(TabControl)
	{
		TabControl->ProcessParentMessages(hWnd, uMsg, wParam, lParam);
	}

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
