///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _MAINDLG_CPP
#define _MAINDLG_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>

#include "../Source/Entry.H"
#include "../Source/ConfigTree.H"
#include "../Source/DetailList.H"

#include "MainDlg.H"
#include "NetLogoDlg.H"
#include "ConnectDlg.H"

#include "../../@Common/WebSite.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListView;
using namespace NSWFL::File;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK MainDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//---------------------(Variable Declarations)
HWND gMainDialog = NULL;
HWND gTreeView = NULL;
HWND gListView = NULL;

HIMAGELIST hOnePixilImageList = NULL;
HIMAGELIST hEnableDisableImageList = NULL;
HIMAGELIST hPoolActiveInactiveImageList = NULL;
HIMAGELIST hWebsiteStartedStoppedList = NULL;

int iLastPageType = 0;
int iLastSiteIndex = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PopDetailPane(int iPageType, int iSiteIndex)
{
	iLastPageType = iPageType;
	iLastSiteIndex = iSiteIndex;

	LVClear(gListView);

	if(!hEnableDisableImageList)
	{
		HICON hIcon = NULL;

		hEnableDisableImageList = ImageList_Create(
			GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_MASK|ILC_COLOR32, 1, 1);

		hIcon = LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_DISABLED));
		ImageList_AddIcon(hEnableDisableImageList, hIcon);

		hIcon = LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_ENABLED));
		ImageList_AddIcon(hEnableDisableImageList, hIcon);
	}

	if(!hPoolActiveInactiveImageList)
	{
		HBITMAP hBitMap = NULL;

		hPoolActiveInactiveImageList = ImageList_Create(16, 16, ILC_COLOR24, 1, 1);

		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_CONFIG_POOLINACTIVE));
		ImageList_Add(hPoolActiveInactiveImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);

		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_CONFIG_POOLACTIVE));
		ImageList_Add(hPoolActiveInactiveImageList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
	}

	if(!hWebsiteStartedStoppedList)
	{
		HBITMAP hBitMap = NULL;

		hWebsiteStartedStoppedList = ImageList_Create(16, 16, ILC_COLOR24, 1, 1);

		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_SITESTOPPED));
		ImageList_Add(hWebsiteStartedStoppedList, hBitMap, hBitMap);
		DeleteObject(hBitMap);

		hBitMap = LoadBitmap(ghAppInstance, MAKEINTRESOURCE(IDB_TREE_SITESTARTED));
		ImageList_Add(hWebsiteStartedStoppedList, hBitMap, hBitMap);
		DeleteObject(hBitMap);
	}

	if(!hOnePixilImageList)
	{
		hOnePixilImageList = ImageList_Create(1, 1, ILC_MASK|ILC_COLOR32, 1, 1);
	}

	if(iPageType == PAGE_TYPE_INVALID)
	{
		return TRUE;
	}
	else
	{
		CXMLReader xml;
		memset(&xml, 0, sizeof(xml));

		//Site settings.
		if(iPageType == PAGE_TYPE_SERVER)
		{
			if(gManage->QueryXMLReader(&xml, "::Information:Server:Performance|"))
			{
				//if(iLastPageType == PAGE_TYPE_SERVER && iLastSiteIndex == INVALID_SITE_INDEX)
				//{
					//RefreshServerPerformanceList(gListView, &xml);
				//}
				//else {
					PopServerPerformanceList(gListView, &xml);
				//}
			}
		}
		if(iPageType == PAGE_TYPE_WEBSITES)
		{
			if(gManage->QueryXMLReader(&xml, "::Configuration:WebSites|%d", iSiteIndex))
			{
				PopWebsitesList(gListView, true, &xml, false);
			}
		}
		else if(iPageType == PAGE_TYPE_WEBSITE)
		{
			if(iSiteIndex != INVALID_SITE_INDEX)
			{
				if(gManage->QueryXMLReader(&xml, "::Configuration:WebSiteOverview|%d", iSiteIndex))
				{
					PopWebsiteOverviewList(gListView, &xml);
				}
			}
		}
		else if(iPageType == PAGE_TYPE_ADVANCED)
		{
			if(gManage->QueryXMLReader(&xml, "::Configuration:WebsiteSettings|%d", iSiteIndex))
			{
				PopWebsiteSettingsList(gListView, &xml);
			}
		}
		else if(iPageType == PAGE_TYPE_DEFAULTPAGES)
		{
			if(gManage->QueryXMLReader(&xml, "::Configuration:DefaultPages|%d", iSiteIndex))
			{
				PopDefaultPagesList(gListView, true, &xml);
			}
		}
		else if(iPageType == PAGE_TYPE_VIRTUALROOTS)
		{
			if(gManage->QueryXMLReader(&xml, "::Configuration:VirtualRoots|%d", iSiteIndex))
			{
				PopVirtualRootsList(gListView, true, &xml);
			}
		}
		else if(iPageType == PAGE_TYPE_CACHE)
		{
			if(gManage->QueryXMLReader(&xml, "::Configuration:Cache|%d", iSiteIndex))
			{
				PopCacheList(gListView, &xml);
			}
		}
		else if(iPageType == PAGE_TYPE_COMPRESSION)
		{
			if(gManage->QueryXMLReader(&xml, "::Configuration:Compression|%d", iSiteIndex))
			{
				PopCompressionList(gListView, true, &xml);
			}
		}
		else if(iPageType == PAGE_TYPE_MIMETYPES)
		{
			if(gManage->QueryXMLReader(&xml, "::Configuration:MimeTypes|%d", iSiteIndex))
			{
				PopMimeTypesList(gListView, true, &xml);
			}
		}
		else if(iPageType == PAGE_TYPE_IPFILTERS)
		{
			if(gManage->QueryXMLReader(&xml, "::Configuration:IPFilters|%d", iSiteIndex))
			{
				PopIPFiltersList(gListView, true, &xml);
			}
		}
		else if(iPageType == PAGE_TYPE_URLFILTERS)
		{
			if(gManage->QueryXMLReader(&xml, "::Configuration:URLFilters|%d", iSiteIndex))
			{
				PopURLFiltersList(gListView, true, &xml);
			}
		}
		else if(iPageType == PAGE_TYPE_EXTFILTERS)
		{
			if(gManage->QueryXMLReader(&xml, "::Configuration:ExtensionFilters|%d", iSiteIndex))
			{
				PopExtensionFiltersList(gListView, true, &xml);
			}
		}
		else if(iPageType == PAGE_TYPE_CUSTOMFILTERS)
		{
			if(gManage->QueryXMLReader(&xml, "::Configuration:CustomFilters|%d", iSiteIndex))
			{
				PopCustomFiltersList(gListView, true, &xml);
			}
		}
		else if(iPageType == PAGE_TYPE_SCRIPTINGENGINES)
		{
			if(gManage->QueryXMLReader(&xml, "::Configuration:ScriptingEngines|%d", iSiteIndex))
			{
				PopScriptingEnginesList(gListView, true, &xml);
			}
		}
		else if(iPageType == PAGE_TYPE_SSIFILES)
		{
			if(gManage->QueryXMLReader(&xml, "::Configuration:SSIFiles|%d", iSiteIndex))
			{
				PopSSIFilesList(gListView, true, &xml);
			}
		}
		else if(iPageType == PAGE_TYPE_CGIFOLDERS)
		{
			if(gManage->QueryXMLReader(&xml, "::Configuration:CGIFolders|%d", iSiteIndex))
			{
				PopCGIFoldersList(gListView, true, &xml);
			}
		}
		else if(iPageType == PAGE_TYPE_ERRORPAGES)
		{
			if(gManage->QueryXMLReader(&xml, "::Configuration:ErrorPages|%d", iSiteIndex))
			{
				PopErrorPagesList(gListView, true, &xml);
			}
		}
		else if(iPageType == PAGE_TYPE_SOCKETPOOLS)
		{
			if(gManage->QueryXMLReader(&xml, "::Configuration:SocketPoolsOverview|"))
			{
				PopSocketPoolsList(gListView, true, &xml, false);
			}
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam)
{
	if(hWnd != (HWND)lParam)
	{
		if(GetWindowLongPtr(hWnd, GWL_STYLE) &WS_CAPTION)
		{
			EndDialog(hWnd, 0);
		}
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CancelAllChildren(HWND hParent)
{
	DWORD dwThreadId = GetCurrentThreadId();
	EnumThreadWindows(dwThreadId, EnumChildProc, (LPARAM)hParent);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void OpenHelp(HWND hWnd)
{
	if(ShellExecute(hWnd, "OPEN", "http:/FortitudeHTTP.NetworkDLS.com/", NULL, NULL, SW_SHOWDEFAULT) <= (HINSTANCE)32)
	{
		MessageBox(hWnd, "Cannot launch your default web-browser, please browse to FortitudeHTTP.com or NetworkDLS.com for more information.", gsTitleCaption, MB_ICONERROR);
	}

	/*
	char sFileName[MAX_PATH];
	char sPath[MAX_PATH];
	char sDrive[MAX_PATH];
	char sDir[MAX_PATH];

	if(GetModuleFileName(ghAppInstance, sFileName, sizeof(sFileName)))
	{
		_splitpath_s(sFileName, sDrive, sizeof(sDrive), sDir, sizeof(sDir), NULL, 0, NULL, 0);
		_makepath_s(sFileName, sizeof(sFileName), sDrive, sDir, "help", ".chm");
		_makepath_s(sPath, sizeof(sPath), sDrive, sDir, NULL, NULL);
		if(ShellExecute(hWnd, "OPEN", sFileName, NULL, sPath, SW_SHOWDEFAULT) <= (HINSTANCE)32)
		{
			MessageBox(hWnd, "Cannot locate the Help File.", gsTitleCaption, MB_ICONERROR);
		}
	}
	*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK MainDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

    static HWND hStatusBar = NULL;
	static bool bFirstPaint = true;
	static bool bTryingToConnect = false;

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
        gMainDialog = hWnd;

        HMENU hMenu = GetSystemMenu(hWnd, FALSE);
        AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
        AppendMenu(hMenu, MF_STRING, GENERIC_MENU_ABOUT, "About");

        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)gsTitleCaption);
        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)));

        gTreeView = GetDlgItem(hWnd, IDC_SITES);
        gListView = GetDlgItem(hWnd, IDC_DETAIL);
		hStatusBar = CreateStatusWindow(WS_CHILD|WS_VISIBLE, "", hWnd, 1);

        int Widths[] = {200, 400, 500, -1};
        SendMessage(hStatusBar, (UINT)SB_SETPARTS, (WPARAM)4, (LPARAM)&Widths);

		RECT rcWindow;
		memset(&rcWindow, 0, sizeof(rcWindow));
        GetClientRect(hWnd, &rcWindow);
		SendMessage(hWnd, WM_SIZE, 0, MAKELONG(rcWindow.right, rcWindow.bottom));

		CenterOverOwner(hWnd);

		iLastPageType = -1;
		iLastSiteIndex = -1;

		InitializeConfigTree();

		SetTimer(hWnd, 0, 1000 * 30, NULL);

        return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }
    //--------------------------------------------------------------------------
    else if(uMsg == WM_COMMAND)
    {
		if(wParam == ID_HELP_CONTENTS)
		{
			OpenHelp(hWnd);
		}
		else if(wParam == ID_HELP_ABOUT)
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
		}
		else if(wParam == ID_SERVER_RESTARTSERVER)
		{
			SendMessage(gTreeView, WM_COMMAND, TREE_CMD_RESTARTSERVER, 0);
		}
		else if(wParam == ID_SERVER_RESTARTWEBSITES)
		{
			SendMessage(gTreeView, WM_COMMAND, TREE_CMD_RESTARTSITES, 0);
		}
		else if(wParam == ID_SERVER_PERSISTSETTINGS)
		{
			SendMessage(gTreeView, WM_COMMAND, TREE_CMD_PERSISTCONFIGURATION, 0);
		}
		else if(wParam == ID_FILE_EXIT)
		{
			return EndDialog(hWnd, 0);
		}
		else if(wParam == ID_FILE_CONNECTTO)
		{
			gManage->Disconnect();
		}
		else if(wParam == ID_SETTINGS_SERVERSETTINGS)
		{
			ConfigureTreeItem(INVALID_SITE_INDEX, PAGE_TYPE_SERVER);
		}
		else if(wParam == ID_SETTINGS_ADDREMOVE_WEBSITES)
		{
			ConfigureTreeItem(INVALID_SITE_INDEX, PAGE_TYPE_WEBSITES);
		}

        return FALSE;
    }
	//--------------------------------------------------------------------------
	else if(uMsg == UM_MGMT_DISCONNECTED)
	{
		if(!bTryingToConnect)
		{
			CancelAllChildren(hWnd);

			bTryingToConnect = true;

			TreeView_DeleteAllItems(gTreeView);
			LVClear(gListView);

			if(DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_CONNECT), hWnd, ConnectDialog) == 1)
			{
				PopulateConfigTree();
			}
			else {
				EndDialog(hWnd, 0);
			}
			bTryingToConnect = false;
		}
	}
	//--------------------------------------------------------------------------
    else if(uMsg == WM_TIMER)
    {
		if(iLastPageType == PAGE_TYPE_SERVER && iLastSiteIndex == INVALID_SITE_INDEX)
		{
			SendMessage(hWnd, WM_REFRESH_SELECTED_CONFIG_ITEM, 0, 0);
		}
		else if(iLastPageType == PAGE_TYPE_CACHE)
		{
			SendMessage(hWnd, WM_REFRESH_SELECTED_CONFIG_ITEM, 0, 0);
		}
        return FALSE;
    }
    //--------------------------------------------------------------------------
	else if(uMsg == WM_SIZE)
    {
		int nWidth = LOWORD(lParam);
        int nHeight = HIWORD(lParam);

		MoveWindow(gTreeView, 5, 5, 250, nHeight - 30, TRUE);
		MoveWindow(gListView, 260, 5, nWidth - 265, nHeight - 30, TRUE);
		MoveWindow(hStatusBar, nWidth-20, nHeight-20, nWidth, nHeight, TRUE);

        return FALSE;
    }
	//---------------------------------------------------------------------------------------------------------------------
	else if(uMsg == WM_GETMINMAXINFO)
	{
		MINMAXINFO *pMMI = (MINMAXINFO *)lParam;
		pMMI->ptMinTrackSize.x = 550;
		pMMI->ptMinTrackSize.y = 350;
		return 0;
	}
	//--------------------------------------------------------------------------
	else if(uMsg == WM_REFRESH_SELECTED_CONFIG_ITEM)
	{
		int iPageType = -1;
		int iSiteIndex = -1;

		if(GetSelectedConfigItem(&iPageType, &iSiteIndex))
		{
			PopDetailPane(iPageType, iSiteIndex);
		}
	}
	//--------------------------------------------------------------------------
	else if(uMsg == WM_REFRESH_CONFIG_TREE_ITEM)
	{
		RefreshConfigTree();
	}
	//--------------------------------------------------------------------------
	else if(uMsg == WM_NOTIFY)
    {
        LPNMHDR lpnmhdr = (LPNMHDR)lParam;
		if(lpnmhdr->code == TVN_SELCHANGED && lpnmhdr->hwndFrom == gTreeView)
		{
			SendMessage(hWnd, WM_REFRESH_SELECTED_CONFIG_ITEM, 0, 0);
		}
		else if(lpnmhdr->code == NM_DBLCLK && lpnmhdr->hwndFrom == gListView)
		{
			ConfigureTreeItem(iLastSiteIndex, iLastPageType);
		}
	}
    //--------------------------------------------------------------------------
	else if(uMsg == WM_PAINT)
    {
		if(bFirstPaint)
		{
			bFirstPaint = false;
			SendMessage(hWnd, UM_MGMT_DISCONNECTED, 0, 0);
		}

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
		return EndDialog(hWnd, 0);
    }
    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
