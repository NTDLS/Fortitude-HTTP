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
using namespace NSWFL::System; 
using namespace NSWFL::File; 
using namespace NSWFL::Registry; 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK Config_ServerSettings(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//General Tab:
	static HWND hGeneralTab;
    static HWND hProductText;
    static HWND hVersionText;
    static HWND hBuildDateText;
	static HWND hOSText;
    static HWND hMemoryText;
    static HWND hProcesorsText;

	//Processors Tab:
	static HWND hProcessorsTab;
	static HWND hProcessorsGrid;

	//Users Tab:
	static HWND hUsersTab;
	static HWND hUsersList;

	//Advanced Tab:
	static HWND hAdvancedTab;

	//Non-Tab-Specific:
	static CTabControl *TabControl;

	//--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
		gCI.hDialog = hWnd;

		CenterOverOwner(hWnd);

        HMENU hMenu = GetSystemMenu(hWnd, FALSE);
        AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
        AppendMenu(hMenu, MF_STRING, GENERIC_MENU_ABOUT, "About");

        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)));

		static LPSTR tabNames[]= {
			"General",
			"Processors",
			"Users",
			"Advanced",
			NULL
		};

		static LPSTR dlgNames[] = {
			MAKEINTRESOURCE(IDD_TAB_SERVER_GENERAL),
			MAKEINTRESOURCE(IDD_TAB_SERVER_PROCESSORS),
			MAKEINTRESOURCE(IDD_TAB_SERVER_SETTINGS_USERS),
			MAKEINTRESOURCE(IDD_TAB_SERVER_ADVANCED),
			NULL
		};

		TabControl = new CTabControl(GetDlgItem(hWnd, IDC_TABCONTROL), tabNames, dlgNames, &Config_ServerSettings, TRUE);

		XMLReader xmlServerSettings;
		if(gManage->QueryXMLReader(&xmlServerSettings, "::Configuration:ServerSettings|"))
		{
			//--------------------------------------------------------------------------------------------------------------------------------------
			//General Tab:
			hGeneralTab = TabControl->Handle("General");

			hProductText    = GetDlgItem(hGeneralTab, IDC_PRODUCTTEXT);
			hVersionText    = GetDlgItem(hGeneralTab, IDC_VERSIONTEXT);
			hBuildDateText  = GetDlgItem(hGeneralTab, IDC_BUILDDATETEXT);
			hOSText         = GetDlgItem(hGeneralTab, IDC_OSTEXT);
			hMemoryText     = GetDlgItem(hGeneralTab, IDC_MEMORYTEXT);
			hProcesorsText  = GetDlgItem(hGeneralTab, IDC_PROCESORSTEXT);

			SYSTEM_INFO SI;
			GetSystemInfo(&SI);

			MEMORYSTATUSEX MS;
			memset(&MS, 0, sizeof(MS));
			MS.dwLength = sizeof(MS);
			GlobalMemoryStatusEx(&MS);
 
			char sTempText[1024];

			Set_Text(hProductText, gsTitleCaption);

			sprintf_s(sTempText, sizeof(sTempText), "%s at %s", __DATE__, __TIME__);
			Set_Text(hBuildDateText, sTempText);

			Set_Text(hVersionText, gsFileVersion);

			Get_OsVersion(sTempText, sizeof(sTempText));
			Set_Text(hOSText, sTempText);

			Set_Text(hMemoryText, FileSizeFriendly((__int64)MS.ullTotalPhys, 2, sTempText, sizeof(sTempText)));

			sprintf_s(sTempText, sizeof(sTempText), "%d", SI.dwNumberOfProcessors);
			Set_Text(hProcesorsText, sTempText);

			//--------------------------------------------------------------------------------------------------------------------------------------
			//Processors Tab:
			hProcessorsTab = TabControl->Handle("Processors");

			XMLReader xmlProcessors;
			if(xmlServerSettings.ToReader("Processors", &xmlProcessors))
			{
				char sRegBuf[255];
				char sTemp[255];

				DWORD dwRegBufSz = 0;
				float fCPUSpeed = 0;

				LV_COLUMN lvColumn;
				LV_ITEM lvItem;
				HIMAGELIST hImageList;

				memset(&lvColumn, 0, sizeof(lvColumn));
				memset(&lvItem, 0, sizeof(lvItem));
				memset(&hImageList, 0, sizeof(hImageList));

				hProcessorsGrid = GetDlgItem(hProcessorsTab, IDC_CPUS);

				if(xmlProcessors.ToBoolean("ProcessBoost")) CheckDlgButton(hProcessorsTab, IDC_PROCESSBOOST, BST_CHECKED);
				if(xmlProcessors.ToBoolean("ThreadBoost"))  CheckDlgButton(hProcessorsTab, IDC_THREADBOOST, BST_CHECKED);

#if defined(WIN64) || defined(_WIN64) || defined(_AMD64)
				DWORD_PTR dwCPUAffinityMask = xmlProcessors.ToU64("AffinityMask");
#else
				DWORD_PTR dwCPUAffinityMask = xmlProcessors.ToUInteger("AffinityMask");
#endif

				int iLVStyle = LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES;
				SendMessage(hProcessorsGrid, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)iLVStyle);

				// Create ImageList begin
				hImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_MASK|ILC_COLOR32, 1, 1);

				HICON hIcon = NULL;

				hIcon = LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_SMALL_CPU));
				ImageList_AddIcon(hImageList, (HICON)hIcon);
				DeleteObject((HICON)hIcon);

				ListView_SetImageList(hProcessorsGrid, hImageList, LVSIL_SMALL);

				// Create Listview columns
				lvColumn.cx        = 180;
				lvColumn.pszText   = "CPU";
				lvColumn.iSubItem  = 0;
				lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
				lvColumn.fmt       = LVCFMT_LEFT;
				ListView_InsertColumn(hProcessorsGrid, 0, &lvColumn);

				lvColumn.cx        = 45;
				lvColumn.pszText   = "Model";
				lvColumn.iSubItem  = 1;
				lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
				lvColumn.fmt       = LVCFMT_CENTER;
				ListView_InsertColumn(hProcessorsGrid, 1, &lvColumn);

				lvColumn.cx        = 55;
				lvColumn.pszText   = "Rev.";
				lvColumn.iSubItem  = 2;
				lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
				lvColumn.fmt       = LVCFMT_CENTER;
				ListView_InsertColumn(hProcessorsGrid, 2, &lvColumn);

				lvColumn.cx        = 65;
				lvColumn.pszText   = "Granularity";
				lvColumn.iSubItem  = 3;
				lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
				lvColumn.fmt       = LVCFMT_CENTER;
				ListView_InsertColumn(hProcessorsGrid, 3, &lvColumn);

				dwRegBufSz = sizeof(sRegBuf);
				memset(sRegBuf, 0, dwRegBufSz);
				if(Get_StringRegistryValue(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", "ProcessorNameString", sRegBuf, dwRegBufSz))
				{
					Trim(sRegBuf);
					Set_Text(GetDlgItem(hProcessorsTab, IDC_CPUDESCR), sRegBuf);
				}
			
				for(DWORD_PTR dwItem = 0; dwItem < SI.dwNumberOfProcessors; dwItem++)
				{
					char sKey[1024];
					sprintf_s(sKey, sizeof(sKey), "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\%d", (int)dwItem);

					if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, sKey, "~MHz", dwRegBufSz))
					{
						dwRegBufSz = 0;
					}
					fCPUSpeed = (float)dwRegBufSz;

					dwRegBufSz = sizeof(sRegBuf);
					memset(sRegBuf, 0, dwRegBufSz);
					if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, sKey, "VendorIdentifier", sRegBuf, dwRegBufSz))
					{
						strcpy_s(sRegBuf, sizeof(sRegBuf), "");
					}

					Trim(sRegBuf);

					if(fCPUSpeed > 1000)
					{
						sprintf_s(sTemp, sizeof(sTemp), "%s %.1f0 GHz", sRegBuf, fCPUSpeed / 1000.0f);
					}
					else {
						if(fCPUSpeed > 0)
						{
							sprintf_s(sTemp, sizeof(sTemp), "%s %.2f MHz", sRegBuf, fCPUSpeed);
						}
						else{
							strcpy_s(sTemp, sizeof(sTemp), "");
						}
					}
			
					lvItem.pszText = sTemp;
					lvItem.mask = LVIF_TEXT|LVIF_IMAGE;
					lvItem.iImage = 0;
					lvItem.iItem  = (int)dwItem;
					lvItem.iSubItem = 0;
					ListView_InsertItem(hProcessorsGrid, &lvItem);

					sprintf_s(sTemp, sizeof(sTemp), "%d", HIWORD(SI.wProcessorRevision));
					lvItem.pszText = sTemp;
					lvItem.mask = LVIF_TEXT|LVIF_IMAGE;
					lvItem.iImage = 0;
					lvItem.iItem  = (int)dwItem;
					lvItem.iSubItem = 1;
					ListView_SetItem(hProcessorsGrid, &lvItem);

					sprintf_s(sTemp, sizeof(sTemp), "%d", LOWORD(SI.wProcessorRevision));
					lvItem.pszText = sTemp;
					lvItem.mask = LVIF_TEXT|LVIF_IMAGE;
					lvItem.iImage = 0;
					lvItem.iItem  = (int)dwItem;
					lvItem.iSubItem = 2;
					ListView_SetItem(hProcessorsGrid, &lvItem);

					sprintf_s(sTemp, sizeof(sTemp), "%0.2f K", ((float)SI.dwAllocationGranularity / 1024.0f));
					lvItem.pszText = sTemp;
					lvItem.mask = LVIF_TEXT|LVIF_IMAGE;
					lvItem.iImage = 0;
					lvItem.iItem  = (int)dwItem;
					lvItem.iSubItem = 3;
					ListView_SetItem(hProcessorsGrid, &lvItem);

					if(IsBitSet(dwCPUAffinityMask, dwItem) || dwCPUAffinityMask == 0) //Initially (after setup), the affinity mask will be 0.
					{
						ListView_SetCheckState(hProcessorsGrid, dwItem, TRUE);
					}
					else {
						ListView_SetCheckState(hProcessorsGrid, dwItem, FALSE);
					}
				}
				xmlProcessors.Destroy();
			}

			//--------------------------------------------------------------------------------------------------------------------------------------
			//Users Tab:
			hUsersTab = TabControl->Handle("Users");
			hUsersList = GetDlgItem(hUsersTab, IDC_LIST);

			XMLReader xmlUsers;
			if(xmlServerSettings.ToReader("Users", &xmlUsers))
			{
				if(xmlUsers.ToBoolean("AllowRemoteManagement"))
				{
					CheckDlgButton(hUsersTab, IDC_ALLOWREMOTEMGMT, BST_CHECKED);
				}
				PopServerSettingsList_User(hUsersList, false, &xmlUsers);
				xmlUsers.Destroy();
			}
			//--------------------------------------------------------------------------------------------------------------------------------------
			//Advanced Tab:
			hAdvancedTab = TabControl->Handle("Advanced");

			XMLReader xmlAdvanced;
			if(xmlServerSettings.ToReader("Advanced", &xmlAdvanced))
			{
				if(xmlAdvanced.ToBoolean("OptimizeForThroughput")) CheckDlgButton(hAdvancedTab, IDC_OPTIMIZEFORTHROUGHPUT, BST_CHECKED);
				if(xmlAdvanced.ToBoolean("ForceSocketReuse"))  CheckDlgButton(hAdvancedTab, IDX_FORCESOCKETREUSE, BST_CHECKED);
				xmlAdvanced.Destroy();
			}
			//--------------------------------------------------------------------------------------------------------------------------------------
		}

        return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }
    //--------------------------------------------------------------------------
    else if(uMsg == WM_COMMAND)
    {
        if(wParam == IDC_ADD)
        {
			CLocalConfigItemStorage localStorage(&gCI);

			gCI.hList = hUsersList;
			gCI.iListIndex = INVALID_LIST_INDEX;

			DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_TAB_SERVER_SETTINGS_USER), hWnd, Config_ServerSettings_User_Item);
		}
        else if(wParam == IDC_EDIT)
        {
			CLocalConfigItemStorage localStorage(&gCI);

			gCI.hList = hUsersList;
			gCI.iListIndex = INVALID_LIST_INDEX;

			if((gCI.iListIndex = LVGetFirstSelectedItemIndex(gCI.hList)) >= 0)
			{
				DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_TAB_SERVER_SETTINGS_USER), hWnd, Config_ServerSettings_User_Item);
			}
        }
        else if(wParam == IDC_DELETE)
        {
			int iItem = LVGetFirstSelectedItemIndex(hUsersList);
			FreeLParamMemory(hUsersList, iItem);
			LVDeleteItemFriendly(hUsersList, iItem);
        }
        else if(wParam == IDC_MOVEUP)
        {
			LVMoveItemUp(hUsersList);
        }
        else if(wParam == IDC_MOVEDOWN)
        {
			LVMoveItemDown(hUsersList);
        }
		else if(wParam == IDC_OK)
        {
			if(LVItemCount(hUsersList) == 0)
			{
				MessageBox(hWnd, "You must define at least one user or you will not be able to manage the server.", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}

			//---(Begin Server Update)------------------------------------------------------------------
			XMLWriter xmlServerSettings("ServerSettings");
			//-----------------------------------------------------------------------------------------------
			//General Tab:

			//-----------------------------------------------------------------------------------------------
			//Processors Tab:

			XMLWriter xmlProcessors("Processors");
			xmlProcessors.AddBool("ProcessBoost", IsDlgButtonChecked(hProcessorsTab, IDC_PROCESSBOOST) == BST_CHECKED);
			xmlProcessors.AddBool("ThreadBoost", IsDlgButtonChecked(hProcessorsTab, IDC_THREADBOOST) == BST_CHECKED);

			DWORD_PTR dwCPUAffinityMask = 0;
			DWORD_PTR iItems = ListView_GetItemCount(hProcessorsGrid);
			for(DWORD_PTR dwItem = 0; dwItem < iItems; dwItem++)
			{
				if(ListView_GetCheckState(hProcessorsGrid, dwItem))
				{
					SetBit(dwCPUAffinityMask, dwItem);
				}
			}

			xmlProcessors.Add("AffinityMask", dwCPUAffinityMask);

			if(dwCPUAffinityMask == 0)
			{
                MessageBox(hWnd, "You must select at least one processor.", gsTitleCaption, MB_ICONASTERISK);
                return TRUE;
			}

			xmlServerSettings.Add(&xmlProcessors);
			//-----------------------------------------------------------------------------------------------
			//Users Tab:
			XMLWriter xmlUsers("Users");
			LoadNewServerSettings_User_Configuration(hUsersTab, hUsersList, &xmlUsers);
			xmlServerSettings.Add(&xmlUsers);

			//-----------------------------------------------------------------------------------------------
			//Advanced Tab:
			XMLWriter xmlAdvanced("Advanced");
			xmlAdvanced.AddBool("OptimizeForThroughput", IsDlgButtonChecked(hAdvancedTab, IDC_OPTIMIZEFORTHROUGHPUT) == BST_CHECKED);
			xmlAdvanced.AddBool("ForceSocketReuse", IsDlgButtonChecked(hAdvancedTab, IDX_FORCESOCKETREUSE) == BST_CHECKED);
			xmlServerSettings.Add(&xmlAdvanced);

			//-----------------------------------------------------------------------------------------------
			StringBuilder xmlText;
			xmlServerSettings.Text(&xmlText);

			XMLReader xmlResult;
			if(gManage->QueryXMLReader(&xmlResult, "::Update|%d:%s:%s", gCI.iSite, "ServerSettings", xmlText.Buffer))
			{
				ProcessXMLResult(hWnd, &xmlResult, NULL, "Failed to apply configuration!");
			}
			else {
				MessageBox(hWnd, "Failed to post configuration to server.", gsTitleCaption, MB_ICONERROR);
			}

			//---(End Server Update)------------------------------------------------------------------

			FreeLParamMemory(hUsersList);
			EndDialog(hWnd, 0);
			DestroyWindow(hWnd);
			delete TabControl;
        }
        else if(wParam == IDC_CANCEL || wParam == IDCANCEL)
        {
			FreeLParamMemory(hUsersList);
			EndDialog(hWnd, 0);
			DestroyWindow(hWnd);
			delete TabControl;
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

				gCI.hList = hUsersList;

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
		FreeLParamMemory(hUsersList);
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
