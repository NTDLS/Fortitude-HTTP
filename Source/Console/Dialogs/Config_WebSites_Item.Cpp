///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _Config_WebSites_Item_CPP
#define _Config_WebSites_Item_CPP
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
#include "../../@Common/HostHeaders.H"

#include "MainDlg.H"
#include "NetLogoDlg.H"

#include "Config_WebSites.H"
#include "Config_WebSites_Item.H"
#include "Config_SocketPools.H"
#include "Config_HostHeaders_Item.H"
#include "CommandWaitDialog.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopListenSocketsComboBox(HWND hCombo)
{
	FreeComboBoxMemory(hCombo);
	SendMessage(hCombo, (UINT)CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);

	char sText[MAX_TEXT_SIZE];
	XMLReader xml;
	if(gManage->QueryXMLReader(&xml, "::Configuration:SocketPools|"))
	{
		int iPort = 0;
		int iMaxConnections = 0;
		char sUID[SOCKETPOOLS_MAX_UID_LENGTH];
		char sDescription[SOCKETPOOLS_MAX_DESCR_LENGTH];

		xml.ScanPosition(0);
		xml.ProgressiveScan(true);

		//Pop the list-view.
		for(XMLReader item; xml.ToReader("SocketPool", &item);)
		{
			//Read XML.
			iPort = item.ToInteger("Port");
			iMaxConnections = item.ToInteger("MaxConnections");
			item.ToString("Description", sDescription, sizeof(sDescription));
			item.ToString("UID", sUID, sizeof(sUID));

			if(CleanWhiteSpaces(sDescription) == 0)
			{
				sprintf_s(sText, sizeof(sText), "Port: %d, Connections: %d", iPort, iMaxConnections);
			}
			else {
				sprintf_s(sText, sizeof(sText), "Port: %d, Connections: %d, [%s]", iPort, iMaxConnections, sDescription);
			}

			int iItemIndex = (int)SendMessage(hCombo, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)sText);
			SendMessage(hCombo, (UINT)CB_SETITEMDATA, (WPARAM)iItemIndex, (LPARAM)pMem->CloneString(sUID));

			item.Destroy();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK Config_WebSites_Item(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

    static HWND hRoot = NULL;
    static HWND hName = NULL;
    static HWND hDescr = NULL;
	static HWND hHostList = NULL;
	static HWND hSocketPools = NULL;

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
		char sText[MAX_TEXT_SIZE];
		CenterOverOwner(hWnd);

        HMENU hMenu = GetSystemMenu(hWnd, FALSE);
        AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
        AppendMenu(hMenu, MF_STRING, GENERIC_MENU_ABOUT, "About");

        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)));

        hName		 = GetDlgItem(hWnd, IDC_NAME);
        hRoot		 = GetDlgItem(hWnd, IDC_ROOT);
        hDescr		 = GetDlgItem(hWnd, IDC_DESCR);
		hHostList	 = GetDlgItem(hWnd, IDC_HOSTLIST);
		hSocketPools = GetDlgItem(hWnd, IDC_SOCKETPOOLS);

		PopListenSocketsComboBox(hSocketPools);

		if(gCI.iListIndex != INVALID_LIST_INDEX)
		{
			//Editing site from list.
			LVGetItemText(gCI.hList, gCI.iListIndex, WEBSITES_LIST_POS_NAME, sText, sizeof(sText));
			Set_Text(hName, sText);

			LVGetItemText(gCI.hList, gCI.iListIndex, WEBSITES_LIST_POS_DESCR, sText, sizeof(sText));
			Set_Text(hDescr, sText);

			LVGetItemText(gCI.hList, gCI.iListIndex, WEBSITES_LIST_POS_ROOT, sText, sizeof(sText));
			Set_Text(hRoot, sText);

			LPARAM xmlParam = LVGetItemParam(gCI.hList, gCI.iListIndex, 0);
			const char *sXMLText = (char *)xmlParam;
			if(sXMLText)
			{
				XMLReader xml(sXMLText);

				char sSocketPool[SOCKETPOOLS_MAX_UID_LENGTH];

				if(xml.ToString("SocketPoolID", sSocketPool, sizeof(sSocketPool)))
				{
					SelectComboBoxItemByData(hSocketPools, sSocketPool);
				}

				PopHostHeadersList(hHostList, false, &xml);
			}
			else {
				PopHostHeadersList(hHostList, false, NULL); //Pop an empty list (add columns).
			}
		}
		else if(gCI.iSite != INVALID_SITE_INDEX)
		{
			XMLReader xml;
			if(gManage->QueryXMLReader(&xml, "::Configuration:WebSite|%d", gCI.iSite))
			{
				PopHostHeadersList(hHostList, false, &xml);

				char sName[MAX_SITE_NAME];
				char sRoot[MAX_PATH];
				char sDescription[MAX_SITE_DESCR];
				char sSocketPool[SOCKETPOOLS_MAX_UID_LENGTH];

				xml.ToString("Name", sName, sizeof(sName));
				xml.ToString("Description", sDescription, sizeof(sDescription));
				xml.ToString("Root", sRoot, sizeof(sRoot));
				xml.ToString("SocketPoolID", sSocketPool, sizeof(sSocketPool));

				//Editing item from treeview.
				Set_Text(hName, sName);
				Set_Text(hDescr, sDescription);
				Set_Text(hRoot, sRoot);

				SelectComboBoxItemByData(hSocketPools, sSocketPool);
			}
		}
		else {
			//Adding new website.
			PopHostHeadersList(hHostList, false, NULL);
		}

		Set_Text(GetDlgItem(hWnd, IDC_CONFIGTITLE), "Web Site");
        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)"Web Site");

		return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }
    //--------------------------------------------------------------------------
    else if(uMsg == WM_COMMAND)
    {
		if(wParam == IDC_BROWSE)
        {
			char sFolder[MAX_PATH];
			Get_Text(hRoot, sFolder, sizeof(sFolder));
			if(BrowseDirectory(hWnd, "Select directory for the website Root",
				BIF_RETURNONLYFSDIRS | BIF_USENEWUI, sFolder, sizeof(sFolder), sFolder))
			{
				Set_Text(hRoot, sFolder);
			}
		}
		else if(wParam == IDC_EDITSOCKETPOOLS)
        {
			CLocalConfigItemStorage localStorage(&gCI);

			char *sSelectedSocketPoolID = (char *)GetSelectedComboBoxItemData(hSocketPools);
			char *sDuplicatedSocketPoolID = NULL;
			
			if(sSelectedSocketPoolID)
			{
				sDuplicatedSocketPoolID = pMem->CloneString(sSelectedSocketPoolID);
			}

			if(DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_CONFIG_GENERICLIST), hWnd, Config_SocketPools) > 0)
			{
				PopListenSocketsComboBox(hSocketPools);
				if(sDuplicatedSocketPoolID)
				{
					SelectComboBoxItemByData(hSocketPools, sDuplicatedSocketPoolID);
				}
			}
			if(sDuplicatedSocketPoolID)
			{
				pMem->Free(sDuplicatedSocketPoolID);
			}
		}
		else if(wParam == IDC_OK)
        {
			char sName[MAX_SITE_NAME];
			char sRoot[MAX_PATH];
			char sDescription[MAX_SITE_DESCR];
			char *sSelectedSocketPoolID = (char*)GetSelectedComboBoxItemData(hSocketPools);

			Get_Text(hName, sName, sizeof(sName));
			Get_Text(hRoot, sRoot, sizeof(sRoot));
			Get_Text(hDescr, sDescription, sizeof(sDescription));

			if(strlen(sName) <= 0)
			{
				MessageBox(hWnd, "You must specify site name.", gsTitleCaption, MB_ICONINFORMATION);
				return TRUE;
			}
			if(strlen(sRoot) <= 0)
			{
				MessageBox(hWnd, "You must specify a root folder.", gsTitleCaption, MB_ICONINFORMATION);
				return TRUE;
			}

			WEBSITE Item;
			memset(&Item, 0, sizeof(Item));

			Item.Name = sName;
			Item.Root = sRoot;
			Item.Description = sDescription;
			Item.Index = gCI.iSite;
			Item.SocketPoolID = sSelectedSocketPoolID;

			XMLWriter siteXML("Site");
			siteXML.Add("Index", gCI.iSite);
			siteXML.Add("Name", sName);
			siteXML.Add("Description", sDescription);
			siteXML.Add("Root", sRoot);
			siteXML.Add("SocketPoolID", sSelectedSocketPoolID);
					
			XMLWriter xmlHostHeaders;
			LoadNewHostHeadersConfiguration(hWnd, hHostList, &xmlHostHeaders);
			siteXML.Add(&xmlHostHeaders);

			if(gCI.iListIndex != INVALID_LIST_INDEX)
			{
				char *sXMLText = siteXML.Text();
				if(sXMLText)
				{
					//We have a list index, so we must be editing.
					FreeLParamMemory(gCI.hList, gCI.iListIndex);
					LVDeleteItem(gCI.hList, gCI.iListIndex);
					PopWebsitesList_AddItem(gCI.hList, &Item, gCI.iListIndex, sXMLText);
					free(sXMLText);
				}
			}
			else {
				if(gCI.iSite == INVALID_SITE_INDEX)
				{
					//We DO NOT have a list index, so we must be adding.

					char *sXMLText = siteXML.Text();
					if(sXMLText)
					{
						//We have a list index, so we must be editing.
						FreeLParamMemory(gCI.hList, gCI.iListIndex);
						LVDeleteItem(gCI.hList, gCI.iListIndex);
						PopWebsitesList_AddItem(gCI.hList, &Item, sXMLText);
						free(sXMLText);
					}
				}
				else {
					//Editing item from treeview.

					bool bResult = false;
					char *xmlText = siteXML.Text();

					XMLReader xml;
					if(ProcessLengthyCommand(&xml, hWnd,
						"Please wait while the new website configuration is applied. This process can take a long"
						" time depending on the changes which were made and the number of connections to the given site.",
						"::Update:Website|%s", xmlText))
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
			}

			FreeLParamMemory(hHostList);
			FreeComboBoxMemory(hSocketPools);
            EndDialog(hWnd, TRUE);
        }
        else if(wParam == IDC_ADD_HOST)
        {
			CLocalConfigItemStorage localStorage(&gCI);

			gCI.hList = hHostList;
			gCI.iListIndex = INVALID_LIST_INDEX;

			DialogBox(ghAppInstance,
				MAKEINTRESOURCE(IDD_CONFIG_HOSTHEADERS_ITEM),
				hWnd, Config_HostHeaders_Item);
		}
        else if(wParam == IDC_EDIT_HOST)
        {
			CLocalConfigItemStorage localStorage(&gCI);

			gCI.hList = hHostList;

			if((gCI.iListIndex = LVGetFirstSelectedItemIndex(gCI.hList)) >= 0)
			{
				DialogBox(ghAppInstance,
					MAKEINTRESOURCE(IDD_CONFIG_HOSTHEADERS_ITEM),
					hWnd, Config_HostHeaders_Item);
			}
		}
        else if(wParam == IDC_DELETE_HOST)
        {
			LVDeleteItemFriendly(hHostList);
        }
		else if(wParam == IDC_CANCEL || wParam == IDCANCEL)
        {
			FreeLParamMemory(hHostList);
			FreeComboBoxMemory(hSocketPools);
            EndDialog(hWnd, FALSE);
            DestroyWindow(hWnd);
        }
        return FALSE;
    }
	//--------------------------------------------------------------------------
	else if(uMsg == WM_NOTIFY)
    {
        LPNMHDR pHdr = (LPNMHDR)lParam;

		if(pHdr->hwndFrom == hHostList)
        {
			if(pHdr->code == NM_DBLCLK)
			{
				if(ListView_GetItemCount(hHostList) > 0 && ListView_GetSelectedCount(hHostList) == 1)
				{
					CLocalConfigItemStorage localStorage(&gCI);

					gCI.hList = hHostList;

					if((gCI.iListIndex = LVGetFirstSelectedItemIndex(gCI.hList)) >= 0)
					{
						if((gCI.iListIndex = LVGetFirstSelectedItemIndex(gCI.hList)) >= 0)
						{
							DialogBox(ghAppInstance,
								MAKEINTRESOURCE(IDD_CONFIG_HOSTHEADERS_ITEM),
								hWnd, Config_HostHeaders_Item);
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
		FreeLParamMemory(hHostList);
		FreeComboBoxMemory(hSocketPools);
        EndDialog(hWnd, 0);
        DestroyWindow(hWnd);
        return TRUE;
    }
    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
