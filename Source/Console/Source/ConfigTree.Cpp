///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _ConfigTree_CPP
#define _ConfigTree_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <Stdlib.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Source/Entry.H"
#include "../Source/ConfigTree.H"

#include "../Dialogs/MainDlg.H"
#include "../Dialogs/NetLogoDlg.H"
#include "../Dialogs/CommandWaitDialog.H"

#include "../Dialogs/Config_DefaultPages.H"
#include "../Dialogs/Config_CGIFolders.H"
#include "../Dialogs/Config_URLFilters.H"
#include "../Dialogs/Config_IPFilters.H"
#include "../Dialogs/Config_ExtensionFilters.H"
#include "../Dialogs/Config_SSIFiles.H"
#include "../Dialogs/Config_CustomFilters.H"
#include "../Dialogs/Config_MimeTypes.H"
#include "../Dialogs/Config_VirtualRoots.H"
#include "../Dialogs/Config_ScriptingEngines.H"
#include "../Dialogs/Config_CompressibleFiles.H"
#include "../Dialogs/Config_ErrorPages.H"
#include "../Dialogs/Config_WebsiteAdvanced.H"
#include "../Dialogs/Config_WebSites.H"
#include "../Dialogs/Config_WebSites_Item.H"
#include "../Dialogs/Config_ServerSettings_User_Item.H"
#include "../Dialogs/Config_ServerSettings.H"
#include "../Dialogs/Config_SocketPools.H"

#include "../Dialogs/Config_Tab_Admins.H"
#include "../Dialogs/Config_Tab_Advanced.H"
#include "../Dialogs/Config_Tab_CPUs.H"
#include "../Dialogs/Config_Tab_General.H"

#include "../../@Common/WebSite.h"
#include "../../@Common/DefaultPages.h"
#include "../../@Common/SocketPools.H"

#include "../../../../@Libraries/CStack/CStack.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HIMAGELIST ghImageList = NULL;
WNDPROC gpOldConfigTreeProc = NULL;
CONFIGITEM gCI;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HTREEITEM ATI(HTREEITEM hParent, char *sINText, int iImage, int lParam);
LRESULT CALLBACK ConfigTreeProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SetConfigItem(void)
{
	gCI.iType = PAGE_TYPE_INVALID;
	gCI.iSite = INVALID_SITE_INDEX;
	gCI.bGlobal = false;
	gCI.iListIndex = INVALID_LIST_INDEX;
	gCI.hList = NULL;
	gCI.hDialog = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SetConfigItem(int iType, int iSite)
{
	gCI.iType = iType;
	gCI.iSite = iSite;
	gCI.bGlobal = false;

	gCI.iListIndex = INVALID_LIST_INDEX;
	gCI.hList = NULL;
	gCI.hDialog = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ConfigureTreeItem(int iSiteIndex, int iPageType)
{
	SetConfigItem(iPageType, iSiteIndex);

	bool bRefresh = false;

	if(iPageType == PAGE_TYPE_SERVER)
	{
		bRefresh = DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_TAB_SERVER_SETTINGS), gMainDialog, Config_ServerSettings) > 0;
	}
	else if(iPageType == PAGE_TYPE_WEBSITES)
	{
		bRefresh = DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_CONFIG_GENERICLIST), gMainDialog, Config_WebSites) > 0;
	}
	else if(iPageType == PAGE_TYPE_WEBSITE && iSiteIndex != INVALID_SITE_INDEX)
	{
		bRefresh = DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_CONFIG_WEBSITES_ITEM), gMainDialog, Config_WebSites_Item) > 0;
	}
	else if(iPageType == PAGE_TYPE_ADVANCED)
	{
		bRefresh = DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_CONFIG_SETTINGSLIST), gMainDialog, Config_WebsiteAdvanced) > 0;
	}
	else if(iPageType == PAGE_TYPE_CGIFOLDERS)
	{
		bRefresh = DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_CONFIG_GENERICLIST), gMainDialog, Config_CGIFolders) > 0;
	}
	else if(iPageType == PAGE_TYPE_DEFAULTPAGES)
	{
		bRefresh = DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_CONFIG_GENERICLIST), gMainDialog, Config_DefaultPages) > 0;
	}
	else if(iPageType == PAGE_TYPE_VIRTUALROOTS)
	{
		bRefresh = DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_CONFIG_GENERICLIST), gMainDialog, Config_VirtualRoots) > 0;
	}
	else if(iPageType == PAGE_TYPE_COMPRESSION)
	{
		bRefresh = DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_CONFIG_COMPRESSIBLEFILESLIST), gMainDialog, Config_CompressibleFiles) > 0;
	}
	else if(iPageType == PAGE_TYPE_MIMETYPES)
	{
		bRefresh = DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_CONFIG_GENERICLIST), gMainDialog, Config_MimeTypes) > 0;
	}
	else if(iPageType == PAGE_TYPE_IPFILTERS)
	{
		bRefresh = DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_CONFIG_GENERICLIST), gMainDialog, Config_IPFilters) > 0;
	}
	else if(iPageType == PAGE_TYPE_URLFILTERS)
	{
		bRefresh = DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_CONFIG_GENERICLIST), gMainDialog, Config_URLFilters) > 0;
	}
	else if(iPageType == PAGE_TYPE_EXTFILTERS)
	{
		bRefresh = DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_CONFIG_GENERICLIST), gMainDialog, Config_ExtensionFilters) > 0;
	}
	else if(iPageType == PAGE_TYPE_CUSTOMFILTERS)
	{
		bRefresh = DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_CONFIG_GENERICLIST), gMainDialog, Config_CustomFilters) > 0;
	}
	else if(iPageType == PAGE_TYPE_SCRIPTINGENGINES)
	{
		bRefresh = DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_CONFIG_GENERICLIST), gMainDialog, Config_ScriptingEngines) > 0;
	}
	else if(iPageType == PAGE_TYPE_SSIFILES)
	{
		bRefresh = DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_CONFIG_GENERICLIST), gMainDialog, Config_SSIFiles) > 0;
	}
	else if(iPageType == PAGE_TYPE_ERRORPAGES)
	{
		bRefresh = DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_CONFIG_ERRORPAGESLIST), gMainDialog, Config_ErrorPages) > 0;
	}
	else if(iPageType == PAGE_TYPE_SOCKETPOOLS)
	{
		bRefresh = DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_CONFIG_GENERICLIST), gMainDialog, Config_SocketPools) > 0;
	}
	else
	{
		SetConfigItem();
		return false;
	}

	if(bRefresh)
	{
		if(iPageType == PAGE_TYPE_WEBSITE || iPageType == PAGE_TYPE_WEBSITES || iPageType == PAGE_TYPE_SOCKETPOOLS)
		{
			PopulateConfigTree();
		}
		else {
			SendMessage(GetParent(gTreeView), WM_REFRESH_SELECTED_CONFIG_ITEM, 0, 0);
		}
	}

	SetConfigItem();
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ProcessXMLResult(HWND hWnd, CXMLReader *pXML, const char *sSuccessText, const char *sFailureText)
{
	if(pXML->ToBoolean("Result"))
	{
		if(sSuccessText)
		{
			MessageBox(hWnd, sSuccessText, gsTitleCaption, MB_ICONINFORMATION);
		}
		return true;
	}
	else {
		char sErrorText[MAX_TEXT_SIZE];

		CStringBuilder errMsg(sFailureText);
		if(pXML->ToString("Text", sErrorText, sizeof(sErrorText)))
		{
			errMsg.AppendF("\r\n\r\n%s", sErrorText);
		}
		errMsg.Terminate();
		MessageBox(hWnd, errMsg.Buffer, gsTitleCaption, MB_ICONWARNING);
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CStack *bindingsStack = NULL;

LRESULT CALLBACK ConfigTreeProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_LBUTTONDBLCLK)
	{
		SendMessage(hWnd, WM_COMMAND, TREE_CMD_CONFIGURE, 0);
		return 0; //Do not expand the tree items on double click.
	}
	/*
	else if (uMsg == WM_LBUTTONUP)
	{
		TV_ITEM SelectedItem;
		memset(&SelectedItem, 0, sizeof(SelectedItem));

		if ((SelectedItem.hItem = TreeView_GetSelection(gTreeView)))
		{
			SelectedItem.mask = TVIF_PARAM | TVIF_IMAGE;
			if (TreeView_GetItem(gTreeView, &SelectedItem))
			{
				WPARAM iPageType = HIWORD(SelectedItem.lParam);
				LPARAM iSiteIndex = LOWORD(SelectedItem.lParam);
				//return true;
			}
		}

		PostMessage(hWnd, TVM_EXPAND, TVE_EXPAND, reinterpret_cast<LPARAM>(SelectedItem.hItem));
		//PostMessage(pnmhdr->hwndFrom, TVM_EXPAND, TVE_COLLAPSE | TVE_COLLAPSERESET, reinterpret_cast<LPARAM>(pnmtv->itemNew.hItem));
	}
	*/

	LRESULT lResult = CallWindowProc(gpOldConfigTreeProc, hWnd, uMsg, wParam, lParam);

	static int iPageType = -1;
	static int iSiteIndex = -1;
	static HMENU hViewMenu = NULL;

	if(uMsg == WM_COMMAND)
	{
		if(GetSelectedConfigItem(&iPageType, &iSiteIndex))
		{
			if(LOWORD(wParam) == TREE_CMD_CONFIGURE)
			{
				ConfigureTreeItem(iSiteIndex, iPageType);
			}
			else if(LOWORD(wParam) == TREE_CMD_RESTARTSITES)
			{
				if(MessageBox(hWnd,
					"Are you sure you want to restart all of the webSites?\r\n\r\n"
					"This will cause the server to:\n"
					"\t1) Stop all sites.\n"
					"\t2) Re-read all website configuration from the disk.\n"
					"\t3) Start sites based on their settings.\n",
					gsTitleCaption, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES)
				{
					CXMLReader xml;
					if(ProcessLengthyCommand(&xml, gMainDialog,
						"Please wait while the websites are restarted. This process can take a long time depending on the number of connections.",
						"::RestartWebSites|", iSiteIndex))
					{
						if(ProcessXMLResult(hWnd, &xml, "The sites were successfully restarted!", "Failed to restart the sites!"))
						{
							PopulateConfigTree();
						}
					}
					else {
						MessageBox(hWnd, "The server did not respond to the requested action!", gsTitleCaption, MB_ICONERROR);
					}
				}
			}
			else if(LOWORD(wParam) == TREE_CMD_CONNECTTO)
			{
				gManage->Disconnect();
			}
			else if(LOWORD(wParam) == TREE_CMD_RESTARTSERVER)
			{
				if(MessageBox(hWnd,
					"Are you sure you want to restart the webserver?\r\n\r\n"
					"This will cause the server to:\n"
					"\t1) Stop all sites.\n"
					"\t2) Re-read all configuration data from the disk.\n"
					"\t3) Start sites based on their settings.\n",
					gsTitleCaption, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES)
				{
					CXMLReader xml;
					if(ProcessLengthyCommand(&xml, gMainDialog,
						"Please wait while the server is restarted. This process can take a long time depending on the number of connections.",
						"::RestartServer|", iSiteIndex))
					{
						if(ProcessXMLResult(hWnd, &xml, "The server was successfully restarted!", "Failed to restart the server!"))
						{
							PopulateConfigTree();
						}
					}
					else {
						MessageBox(hWnd, "The server did not respond to the requested action!", gsTitleCaption, MB_ICONERROR);
					}
				}
			}
			else if(LOWORD(wParam) == TREE_CMD_PERSISTCONFIGURATION)
			{
				if(MessageBox(hWnd,
					"Persisting the configuration will cause all configured setting to be written to the disk. Continue?",
					gsTitleCaption, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES)
				{
					CXMLReader xml;
					if(ProcessLengthyCommand(&xml, gMainDialog,
						"Please wait while the configuration is persisted. This process can take a long time depending on the number of connections.",
						"::PersistConfiguration|", iSiteIndex))
					{
						if(ProcessXMLResult(hWnd, &xml, "The configuration has been persisted!", "Failed to restart the server!"))
						{
							PopulateConfigTree();
						}
					}
					else {
						MessageBox(hWnd, "The server did not respond to the requested action!", gsTitleCaption, MB_ICONERROR);
					}
				}
			}
			else if(LOWORD(wParam) == TREE_CMD_START_SITE)
			{
				CXMLReader xml;
				if(gManage->QueryXMLReader(&xml, "::Start|%d", iSiteIndex))
				{
					if(ProcessXMLResult(hWnd, &xml, NULL, "Failed to start the site!"))
					{
						TV_ITEM TVItem;
						memset(&TVItem, 0, sizeof(TVItem));

						if((TVItem.hItem = TreeView_GetSelection(hWnd)))
						{
							TVItem.mask = TVIF_IMAGE|TVIF_SELECTEDIMAGE;
							TVItem.iImage = TREE_IMAGE_SITE_ONLINE;
							TVItem.iSelectedImage = TREE_IMAGE_SITE_ONLINE;
							TreeView_SetItem(hWnd, &TVItem);
						}
					}
				}
				else {
					MessageBox(hWnd, "The server did not respond to the requested action!", gsTitleCaption, MB_ICONERROR);
				}
			}
			else if(LOWORD(wParam) == TREE_CMD_STOP_SITE)
			{
				if(MessageBox(hWnd,
					"Are you sure you want to stop this site?",
					gsTitleCaption, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES)
				{
					CXMLReader xml;
					if(ProcessLengthyCommand(&xml, gMainDialog,
						"Please wait while the website is stopped. This process can take a long time depending on the number of connections on the given website.",
						"::Stop|%d", iSiteIndex))
					{
						if(ProcessXMLResult(hWnd, &xml, NULL, "Failed to stop the site!"))
						{
							TV_ITEM TVItem;
							memset(&TVItem, 0, sizeof(TVItem));

							if((TVItem.hItem = TreeView_GetSelection(hWnd)))
							{
								TVItem.mask = TVIF_IMAGE|TVIF_SELECTEDIMAGE;
								TVItem.iImage = TREE_IMAGE_SITE_OFFLINE;
								TVItem.iSelectedImage = TREE_IMAGE_SITE_OFFLINE;
								TreeView_SetItem(hWnd, &TVItem);
							}
						}
					}
					else {
						MessageBox(hWnd, "The server did not respond to the requested action!", gsTitleCaption, MB_ICONERROR);
					}
				}
			}
			else if(LOWORD(wParam) == TREE_CMD_RESTART_SITE)
			{
				if(MessageBox(hWnd,
					"Are you sure you want to restart this site?",
					gsTitleCaption, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES)
				{
					CXMLReader xml;
					if(ProcessLengthyCommand(&xml, gMainDialog,
						"Please wait while the website is restarted. This process can take a long time depending on the number of connections on the given website.",
						"::RestartWebSite|%d", iSiteIndex))
					{
						if(ProcessXMLResult(hWnd, &xml, "The site was successfully restarted!", "Failed to restart the site!"))
						{
							TV_ITEM TVItem;
							memset(&TVItem, 0, sizeof(TVItem));

							if((TVItem.hItem = TreeView_GetSelection(hWnd)))
							{
								bool bIsActive = false;
								if(gManage->QueryBoolean(&bIsActive, "::IsActive|%d", iSiteIndex))
								{
									TVItem.iImage = TREE_IMAGE_SITE_ONLINE;
									TVItem.iSelectedImage = TREE_IMAGE_SITE_ONLINE;
								}
								else {
									TVItem.iImage = TREE_IMAGE_SITE_OFFLINE;
									TVItem.iSelectedImage = TREE_IMAGE_SITE_OFFLINE;
								}

								TVItem.mask = TVIF_IMAGE|TVIF_SELECTEDIMAGE;
								TreeView_SetItem(hWnd, &TVItem);
							}
						}
					}
					else {
						MessageBox(hWnd, "The server did not respond to the requested action!", gsTitleCaption, MB_ICONERROR);
					}
				}
			}
			else if(LOWORD(wParam) == TREE_CMD_VIEW_SITE)
			{
				int iClickedIndex = HIWORD(wParam);

				_CSTACKITEM *pStackItem = bindingsStack->Peek(iClickedIndex);
				if(pStackItem)
				{
					ShellExecute(hWnd, "open", (char *)pStackItem->Value, NULL, NULL, SW_NORMAL);
				}

				DestroyMenu(hViewMenu);
				delete bindingsStack;

				hViewMenu = NULL;
				bindingsStack = NULL;
			}
		}
	}
	else if(uMsg == WM_RBUTTONDOWN)
	{
		//Perform a hit-test to find the item that was clicked.
		TVHITTESTINFO tvHit;
		memset(&tvHit, 0, sizeof(tvHit));

		GetCursorPos(&tvHit.pt);
		ScreenToClient(hWnd, &tvHit.pt);

		HTREEITEM hSelectedItem = TreeView_HitTest(hWnd, &tvHit);

		//Select the item that was clicked.
		TreeView_SelectItem(hWnd, hSelectedItem);
		TreeView_EnsureVisible(hWnd, hSelectedItem);

		if(GetSelectedConfigItem(&iPageType, &iSiteIndex))
		{
			if((iSiteIndex != INVALID_SITE_INDEX && iPageType == PAGE_TYPE_WEBSITE)
				|| (iSiteIndex == INVALID_SITE_INDEX &&
				(iPageType == PAGE_TYPE_WEBSITES || iPageType == PAGE_TYPE_SERVER)))
			{
				HMENU hParent = CreateMenu();
				HMENU hChild = CreateMenu();
				HMENU hSelect = CreateMenu();

				if(iPageType == PAGE_TYPE_WEBSITE)
				{
					bool bIsActive = false;
					if(gManage->QueryBoolean(&bIsActive, "::IsActive|%d", iSiteIndex))
					{
						if(bIsActive)
						{
							AppendMenu(hChild, MF_STRING, TREE_CMD_STOP_SITE, "Stop");
							AppendMenu(hChild, MF_STRING, TREE_CMD_RESTART_SITE, "Restart");
							AppendMenu(hChild, MF_SEPARATOR, TREE_CMD_SEPERATOR, "");

							CXMLReader xml;
							if(gManage->QueryXMLReader(&xml, "::Configuration:WebSiteBindings|%d", iSiteIndex))
							{
								if(hViewMenu)
								{
									DestroyMenu(hViewMenu);
								}
								if(bindingsStack)
								{
									delete bindingsStack;
								}

								hViewMenu = CreateMenu();
								bindingsStack = new CStack(BEHAVIOR_FILO);

								xml.ProgressiveScan(true);

								int iMenuPosition = 0;

								for(char sURL[MAX_TEXT_SIZE]; xml.ToString("Binding", sURL, sizeof(sURL)) > 0;)
								{
									bindingsStack->Push(sURL);
									AppendMenu(hViewMenu, MF_STRING, MAKELONG(TREE_CMD_VIEW_SITE, iMenuPosition++), sURL);
								}

								InsertMenu(hChild, 0, MF_POPUP|MF_BYPOSITION, (UINT_PTR)hViewMenu, "View");
							}
						}
						else{
							AppendMenu(hChild, MF_STRING, TREE_CMD_START_SITE, "Start");
							AppendMenu(hChild, MF_SEPARATOR, TREE_CMD_SEPERATOR, "");
						}
					}

					AppendMenu(hChild, MF_STRING, TREE_CMD_CONFIGURE, "Configure");
				}
				else if(iPageType == PAGE_TYPE_SERVER)
				{
					AppendMenu(hChild, MF_STRING, TREE_CMD_CONNECTTO, "Connect to...");
					AppendMenu(hChild, MF_SEPARATOR,0,"Separator");
					AppendMenu(hChild, MF_STRING, TREE_CMD_RESTARTSERVER, "Restart Server");
					AppendMenu(hChild, MF_STRING, TREE_CMD_PERSISTCONFIGURATION, "Persist Configuration");
					AppendMenu(hChild, MF_SEPARATOR,0,"Separator");
					AppendMenu(hChild, MF_STRING, TREE_CMD_CONFIGURE, "Configure");
				}
				else if(iPageType == PAGE_TYPE_WEBSITES)
				{
					AppendMenu(hChild, MF_STRING, TREE_CMD_RESTARTSITES, "Restart Websites");
					AppendMenu(hChild, MF_STRING, TREE_CMD_CONFIGURE, "Configure");
				}

				InsertMenu(hParent, 0, MF_POPUP|MF_BYPOSITION, (UINT_PTR)hChild, "HTTP WebSites");

				POINT xy;
				GetCursorPos(&xy);
				TrackPopupMenuEx(hChild, TPM_LEFTALIGN|TPM_RIGHTBUTTON, (xy.x) + 1, xy.y, hWnd, NULL);
				DestroyMenu(hChild);
				DestroyMenu(hParent);
			}
			else {

				if(iPageType == PAGE_TYPE_ADVANCED
					|| iPageType == PAGE_TYPE_DEFAULTPAGES
					|| iPageType == PAGE_TYPE_VIRTUALROOTS
					|| iPageType == PAGE_TYPE_COMPRESSION
					|| iPageType == PAGE_TYPE_MIMETYPES
					|| iPageType == PAGE_TYPE_IPFILTERS
					|| iPageType == PAGE_TYPE_URLFILTERS
					|| iPageType == PAGE_TYPE_EXTFILTERS
					|| iPageType == PAGE_TYPE_CUSTOMFILTERS
					|| iPageType == PAGE_TYPE_SCRIPTINGENGINES
					|| iPageType == PAGE_TYPE_SSIFILES
					|| iPageType == PAGE_TYPE_CGIFOLDERS
					|| iPageType == PAGE_TYPE_ERRORPAGES
					|| iPageType == PAGE_TYPE_WEBSITES
					|| iPageType == PAGE_TYPE_WEBSITE)
				{
					HMENU hParent = CreateMenu();
					HMENU hChild = CreateMenu();
					HMENU hSelect = CreateMenu();

					AppendMenu(hChild, MF_STRING, TREE_CMD_CONFIGURE, "Configure");
					InsertMenu(hParent, 0, MF_POPUP|MF_BYPOSITION, (UINT_PTR)hChild, "Attribute");

					POINT xy;
					GetCursorPos(&xy);
					TrackPopupMenuEx(hChild, TPM_LEFTALIGN|TPM_RIGHTBUTTON, (xy.x) + 1, xy.y, hWnd, NULL);
					DestroyMenu(hChild);
					DestroyMenu(hParent);
				}
			}
		}
	}

	//--------------------------------------------------------------------------

	return lResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool InitializeConfigTree(void)
{
	TreeView_DeleteAllItems(gTreeView);
	if(!ghImageList)
	{
		ghImageList = ImageList_Create(16, 16, ILC_COLOR24, 10, 1);

		HBITMAP hBitMap = NULL;

		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_SERVER));
		ImageList_Add(ghImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_SITESTARTED));
		ImageList_Add(ghImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_SITESTOPPED));
		ImageList_Add(ghImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_CONFIG_ADVANCED));
		ImageList_Add(ghImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_CONFIG_FILTERS));
		ImageList_Add(ghImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_CONFIG_SCRIPTING));
		ImageList_Add(ghImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_CONFIG_SCRIPTINGENGINES));
		ImageList_Add(ghImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_CONFIG_WEBSITES));
		ImageList_Add(ghImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_CONFIG_CACHE));
		ImageList_Add(ghImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_CONFIG_CGIFOLDERS));
		ImageList_Add(ghImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_CONFIG_CUSTOMFILTERS));
		ImageList_Add(ghImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_CONFIG_DEFAULTPAGES));
		ImageList_Add(ghImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_CONFIG_EXTENSIONFILTERS));
		ImageList_Add(ghImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_CONFIG_IPFILTERS));
		ImageList_Add(ghImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_CONFIG_MIMETYPES));
		ImageList_Add(ghImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_CONFIG_SSIFILES));
		ImageList_Add(ghImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_CONFIG_URLFILTERS));
		ImageList_Add(ghImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_CONFIG_VIRTUALROOTS));
		ImageList_Add(ghImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_CONFIG_COMPRESSION));
		ImageList_Add(ghImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_CONFIG_ERRORPAGES));
		ImageList_Add(ghImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_CONFIG_REALMS));
		ImageList_Add(ghImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_CONFIG_AUTHENTICATION));
		ImageList_Add(ghImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_CONFIG_SETTINGS));
		ImageList_Add(ghImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_CONFIG_SITE));
		ImageList_Add(ghImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_CONFIG_SOCKETPOOLS));
		ImageList_Add(ghImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
	}

	TreeView_SetImageList(gTreeView, ghImageList, TVSIL_NORMAL);

	gpOldConfigTreeProc = (WNDPROC) SetWindowLongPtr(gTreeView, GWLP_WNDPROC, (LRESULT) &ConfigTreeProc);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RefreshConfigTree(void)
{
	SetWindowLongPtr(gTreeView, GWLP_WNDPROC, (LRESULT) gpOldConfigTreeProc);
	gpOldConfigTreeProc = (WNDPROC) SetWindowLongPtr(gTreeView, GWLP_WNDPROC, (LRESULT) &ConfigTreeProc);

	PopulateConfigTree();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//ATI: AddTreeItem
HTREEITEM ATI(HTREEITEM hParent, char *sINText, int iImage, int lParam)
{
    TV_INSERTSTRUCT MyTVIS;
	memset(&MyTVIS, 0, sizeof(MyTVIS));
    MyTVIS.item.hItem          = hParent;
    MyTVIS.item.pszText        = sINText;
    MyTVIS.item.iImage         = iImage;
    MyTVIS.item.iSelectedImage = iImage;
    MyTVIS.item.mask           = TVIF_TEXT|TVIF_SELECTEDIMAGE|TVIF_IMAGE|TVIF_PARAM;

    MyTVIS.hParent             = hParent;
	MyTVIS.item.lParam		   = lParam;
    MyTVIS.item.cchTextMax     = (int)strlen(sINText);

    return TreeView_InsertItem(gTreeView, &MyTVIS);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopulateConfigTree(void)
{
	CXMLReader xml;
	memset(&xml, 0, sizeof(xml));
	if(gManage->QueryXMLReader(&xml, "::Configuration:WebSites|"))
	{
		PopulateConfigTree(&xml);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PopulateConfigTree(CXMLReader *pSites)
{
	TreeView_DeleteAllItems(gTreeView);

	int iIndex;
	char sName[MAX_SITE_NAME];
	bool bIsActive = false;
	HTREEITEM hMajorRoot = NULL;

	HTREEITEM hRoot = ATI(TVI_ROOT, gManage->CurrentConnectioninfo.Host, TREE_IMAGE_SERVER, MAKELONG(INVALID_SITE_INDEX, PAGE_TYPE_SERVER));
	HTREEITEM hSites = ATI(hRoot, "Web Sites", TREE_IMAGE_WEBSITES, MAKELONG(INVALID_SITE_INDEX, PAGE_TYPE_WEBSITES));
	HTREEITEM hTI = NULL;

	//MAKELONG("SITE INDEX", "PAGE TYPE")

	CXMLReader site;
	pSites->ProgressiveScan(true);
	while(pSites->ToReader("Site", &site))
	{
		iIndex = site.ToInteger("Index");
		site.ToString("Name", sName, sizeof(sName));
		bIsActive = site.ToBoolean("IsActive");
		site.Destroy();

		if(bIsActive)
		{
			hMajorRoot = ATI(hSites, sName, TREE_IMAGE_SITE_ONLINE, MAKELONG(iIndex, PAGE_TYPE_WEBSITE));
		}
		else{
			hMajorRoot = ATI(hSites, sName, TREE_IMAGE_SITE_OFFLINE, MAKELONG(iIndex, PAGE_TYPE_WEBSITE));
		}

		//Per site settings.
		hTI = ATI(hMajorRoot, "Settings", TREE_IMAGE_SETTINGS, MAKELONG(iIndex, PAGE_TYPE_SETTINGS));
		ATI(hTI, "Advanced", TREE_IMAGE_ADVANCED, MAKELONG(iIndex, PAGE_TYPE_ADVANCED));
		ATI(hTI, "Default Pages", TREE_IMAGE_DEFAULTPAGES, MAKELONG(iIndex, PAGE_TYPE_DEFAULTPAGES));
		ATI(hTI, "Virtual Roots", TREE_IMAGE_VIRTUALROOTS, MAKELONG(iIndex, PAGE_TYPE_VIRTUALROOTS));
		ATI(hTI, "Compression", TREE_IMAGE_COMPRESSION, MAKELONG(iIndex, PAGE_TYPE_COMPRESSION));
		ATI(hTI, "Mime Types", TREE_IMAGE_MIMETYPES, MAKELONG(iIndex, PAGE_TYPE_MIMETYPES));
		ATI(hTI, "Error Pages", TREE_IMAGE_ERRORPAGES, MAKELONG(iIndex, PAGE_TYPE_ERRORPAGES));
		//ATI(hTI, "Realms", TREE_IMAGE_REALMS, MAKELONG(iIndex, PAGE_TYPE_REALMS));
		//ATI(hTI, "Authentication", TREE_IMAGE_AUTHENTICATION, MAKELONG(iIndex, PAGE_TYPE_AUTHENTICATION));

		//Per site filters.
		hTI = ATI(hMajorRoot, "Filters", TREE_IMAGE_FILTERS, MAKELONG(iIndex, PAGE_TYPE_INVALID));
		ATI(hTI, "IP Filters", TREE_IMAGE_IPFILTERS, MAKELONG(iIndex, PAGE_TYPE_IPFILTERS));
		ATI(hTI, "URL Filters", TREE_IMAGE_URLFILTERS, MAKELONG(iIndex, PAGE_TYPE_URLFILTERS));
		ATI(hTI, "Extension Filters", TREE_IMAGE_EXTENSIONFILTERS, MAKELONG(iIndex, PAGE_TYPE_EXTFILTERS));
		ATI(hTI, "Custom Filters", TREE_IMAGE_CUSTOMFILTERS, MAKELONG(iIndex, PAGE_TYPE_CUSTOMFILTERS));

		//Per site scripting.
		hTI = ATI(hMajorRoot, "Scripting", TREE_IMAGE_SCRIPTING, MAKELONG(iIndex, PAGE_TYPE_INVALID));
		ATI(hTI, "Scripting Engines", TREE_IMAGE_SCRIPTINGENGINES, MAKELONG(iIndex, PAGE_TYPE_SCRIPTINGENGINES));
		ATI(hTI, "SSI Files", TREE_IMAGE_SSIFILES, MAKELONG(iIndex, PAGE_TYPE_SSIFILES));
		ATI(hTI, "CGI Folders", TREE_IMAGE_CGIFOLDERS, MAKELONG(iIndex, PAGE_TYPE_CGIFOLDERS));

		//Per site cache.
		hTI = ATI(hMajorRoot, "Cache", TREE_IMAGE_CACHE, MAKELONG(iIndex, PAGE_TYPE_CACHE));
	}

	hMajorRoot = ATI(hRoot, "Global Settings", TREE_IMAGE_SITE, MAKELONG(INVALID_SITE_INDEX, PAGE_TYPE_INVALID));

	//Global settings.
	hTI = ATI(hMajorRoot, "Settings", TREE_IMAGE_SETTINGS, MAKELONG(INVALID_SITE_INDEX, PAGE_TYPE_SETTINGS));
	ATI(hTI, "Advanced", TREE_IMAGE_ADVANCED, MAKELONG(INVALID_SITE_INDEX, PAGE_TYPE_ADVANCED));
	ATI(hTI, "Default Pages", TREE_IMAGE_DEFAULTPAGES, MAKELONG(INVALID_SITE_INDEX, PAGE_TYPE_DEFAULTPAGES));
	ATI(hTI, "Virtual Roots", TREE_IMAGE_VIRTUALROOTS, MAKELONG(INVALID_SITE_INDEX, PAGE_TYPE_VIRTUALROOTS));
	ATI(hTI, "Compression", TREE_IMAGE_COMPRESSION, MAKELONG(INVALID_SITE_INDEX, PAGE_TYPE_COMPRESSION));
	ATI(hTI, "Mime Types", TREE_IMAGE_MIMETYPES, MAKELONG(INVALID_SITE_INDEX, PAGE_TYPE_MIMETYPES));
	ATI(hTI, "Error Pages", TREE_IMAGE_ERRORPAGES, MAKELONG(INVALID_SITE_INDEX, PAGE_TYPE_ERRORPAGES));
	//ATI(hTI, "Realms", TREE_IMAGE_REALMS, MAKELONG(INVALID_SITE_INDEX, PAGE_TYPE_REALMS));
	//ATI(hTI, "Authentication", TREE_IMAGE_AUTHENTICATION, MAKELONG(INVALID_SITE_INDEX, PAGE_TYPE_AUTHENTICATION));

	//Global filters.
	hTI = ATI(hMajorRoot, "Filters", TREE_IMAGE_FILTERS, MAKELONG(INVALID_SITE_INDEX, PAGE_TYPE_INVALID));
	ATI(hTI, "IP Filters", TREE_IMAGE_IPFILTERS, MAKELONG(INVALID_SITE_INDEX, PAGE_TYPE_IPFILTERS));
	ATI(hTI, "URL Filters", TREE_IMAGE_URLFILTERS, MAKELONG(INVALID_SITE_INDEX, PAGE_TYPE_URLFILTERS));
	ATI(hTI, "Extension Filters", TREE_IMAGE_EXTENSIONFILTERS, MAKELONG(INVALID_SITE_INDEX, PAGE_TYPE_EXTFILTERS));
	ATI(hTI, "Custom Filters", TREE_IMAGE_CUSTOMFILTERS, MAKELONG(INVALID_SITE_INDEX, PAGE_TYPE_CUSTOMFILTERS));

	//Global scripting.
	hTI = ATI(hMajorRoot, "Scripting", TREE_IMAGE_SCRIPTING, MAKELONG(INVALID_SITE_INDEX, PAGE_TYPE_INVALID));
	ATI(hTI, "Scripting Engines", TREE_IMAGE_SCRIPTINGENGINES, MAKELONG(INVALID_SITE_INDEX, PAGE_TYPE_SCRIPTINGENGINES));
	ATI(hTI, "SSI Files", TREE_IMAGE_SSIFILES, MAKELONG(INVALID_SITE_INDEX, PAGE_TYPE_SSIFILES));
	ATI(hTI, "CGI Folders", TREE_IMAGE_CGIFOLDERS, MAKELONG(INVALID_SITE_INDEX, PAGE_TYPE_CGIFOLDERS));

	ATI(hMajorRoot, "Socket Pools", TREE_IMAGE_SOCKETPOOLS, MAKELONG(INVALID_SITE_INDEX, PAGE_TYPE_SOCKETPOOLS));

	SendMessage(gTreeView, TVM_EXPAND, TVE_EXPAND, (LPARAM)hRoot);
	SendMessage(gTreeView, TVM_EXPAND, TVE_EXPAND, (LPARAM)hSites);
	SendMessage(gTreeView, TVM_EXPAND, TVE_EXPAND, (LPARAM)hMajorRoot);

	TreeView_Select(gTreeView, hRoot, TVGN_CARET);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GetSelectedConfigItem(int *iPageType, int *iSiteIndex)
{
	*iPageType = -1;
	*iSiteIndex = -1;

	TV_ITEM SelectedItem;
	memset(&SelectedItem, 0, sizeof(SelectedItem));

	if((SelectedItem.hItem = TreeView_GetSelection(gTreeView)))
	{
		SelectedItem.mask = TVIF_PARAM|TVIF_IMAGE;
		if(TreeView_GetItem(gTreeView, &SelectedItem))
		{
			*iPageType = HIWORD(SelectedItem.lParam);
			*iSiteIndex = LOWORD(SelectedItem.lParam);
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
