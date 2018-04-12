///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _Config_CompressibleFiles_CPP
#define _Config_CompressibleFiles_CPP
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
#include "../../@Common/Compression.h"

#include "MainDlg.H"
#include "NetLogoDlg.H"

#include "Config_CompressibleFiles.H"
#include "Config_CompressibleFiles_Item.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK Config_CompressibleFiles(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

    static HWND hEnabled = NULL;
    static HWND hMinFileSize = NULL;
    static HWND hMaxFileSize = NULL;
    static HWND hCompressLevel = NULL;
    static HWND hMemoryLevel = NULL;
    static HWND hCachePath = NULL;
    static HWND hCacheSize = NULL;

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

        hMinFileSize = GetDlgItem(hWnd, IDC_MINFILESIZE);
        hMaxFileSize = GetDlgItem(hWnd, IDC_MAXFILESIZE);
        hCompressLevel = GetDlgItem(hWnd, IDC_COMPRESSLEVEL);
        hMemoryLevel = GetDlgItem(hWnd, IDC_MEMORYLEVEL);
        hCachePath = GetDlgItem(hWnd, IDC_CACHEPATH);
        hCacheSize = GetDlgItem(hWnd, IDC_CACHESIZE);

		XMLReader xml;
		if(gManage->QueryXMLReader(&xml, "::Configuration:Compression|%d", gCI.iSite))
		{
			char sInt[64];
			char sPath[MAX_PATH];
			xml.ToString("CachePath", sPath, sizeof(sPath));

			int iMaxCompressionSize = xml.ToInteger("MaxCompressionSize", 1024 * 1024);
			int iMinCompressionSize = xml.ToInteger("MinCompressionSize", 1024);
			int iMaxCompressionCache = xml.ToInteger("MaxCompressionCache", 10000);
			int iCompressionLevel = xml.ToInteger("CompressionLevel", 9);
			int iMemoryLevel = xml.ToInteger("MemoryLevel", 8);
			bool bCompressDynamicContent = xml.ToBoolean("CompressDynamicContent", true);
			bool bEnable = xml.ToBoolean("Enable", true);

			Set_Text(hCachePath, sPath);
			Set_Text(hMaxFileSize, FormatInteger(sInt, sizeof(sInt), iMaxCompressionSize));
			Set_Text(hMinFileSize, FormatInteger(sInt, sizeof(sInt), iMinCompressionSize));
			Set_Text(hCompressLevel, FormatInteger(sInt, sizeof(sInt), iCompressionLevel));
			Set_Text(hMemoryLevel, FormatInteger(sInt, sizeof(sInt), iMemoryLevel));
			Set_Text(hCacheSize, FormatInteger(sInt, sizeof(sInt), iMaxCompressionCache));
			CheckDlgButton(hWnd, IDC_COMPRESSDYNAMIC, bCompressDynamicContent);

			if(bEnable)
			{
				CheckDlgButton(hWnd, IDC_ENABLED, BST_CHECKED);
			}

			PopCompressionList(gCI.hList, false, &xml);

			char sText[255];
			sprintf_s(sText, sizeof(sText), "%s%s", gCI.iSite == INVALID_SITE_INDEX ? "Global " : "", "Compression");
			Set_Text(GetDlgItem(hWnd, IDC_CONFIGTITLE), sText);
			SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)sText);

			if(gCI.iSite == INVALID_SITE_INDEX)
			{
				ShowWindow(GetDlgItem(hWnd, IDC_USEGLOBAL), SW_HIDE);
				CheckDlgButton(hWnd, IDC_USEGLOBAL, BST_UNCHECKED);
			}
			else{
				if(gManage->QueryBoolean(&gCI.bGlobal, "::IsGlobal|%d:Compression", gCI.iSite))
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
		if(wParam == IDC_BROWSE)
		{
			char sFolder[MAX_PATH];
			Get_Text(hCachePath, sFolder, sizeof(sFolder));
			if(BrowseDirectory(hWnd, "Select a compression cache directory.",
				BIF_RETURNONLYFSDIRS | BIF_NONEWFOLDERBUTTON, sFolder, sizeof(sFolder), sFolder))
			{
				Set_Text(hCachePath, sFolder);
			}
		}
		else if(wParam == IDC_USEGLOBAL)
		{
			bool bChecked = IsDlgButtonChecked(hWnd, IDC_USEGLOBAL) > 0;
			EnableWindow(gCI.hList, !bChecked);
			EnableWindow(GetDlgItem(hWnd, IDC_ENABLED), !bChecked);
			EnableWindow(GetDlgItem(hWnd, IDC_ADD), !bChecked);
			EnableWindow(GetDlgItem(hWnd, IDC_EDIT), !bChecked);
			EnableWindow(GetDlgItem(hWnd, IDC_DELETE), !bChecked);
			EnableWindow(GetDlgItem(hWnd, IDC_MOVEUP), !bChecked);
			EnableWindow(GetDlgItem(hWnd, IDC_MOVEDOWN), !bChecked);
			EnableWindow(GetDlgItem(hWnd, IDC_COMPRESSDYNAMIC), !bChecked);
			EnableWindow(GetDlgItem(hWnd, IDC_BROWSE), !bChecked);

			EnableWindow(hMinFileSize, !bChecked);
			EnableWindow(hMaxFileSize, !bChecked);
			EnableWindow(hCompressLevel, !bChecked);
			EnableWindow(hMemoryLevel, !bChecked);
			EnableWindow(hCachePath, !bChecked);
			EnableWindow(hCacheSize, !bChecked);
		}
        else if(wParam == IDC_ADD)
        {
			CLocalConfigItemStorage localStorage(&gCI);

			gCI.iListIndex = INVALID_LIST_INDEX;

			DialogBox(ghAppInstance,
				MAKEINTRESOURCE(IDD_CONFIG_COMPRESSIBLEFILES_ITEM),
				hWnd, Config_CompressibleFiles_Item);
		}
        else if(wParam == IDC_EDIT)
        {
			CLocalConfigItemStorage localStorage(&gCI);

			if((gCI.iListIndex = LVGetFirstSelectedItemIndex(gCI.hList)) >= 0)
			{
				DialogBox(ghAppInstance,
					MAKEINTRESOURCE(IDD_CONFIG_COMPRESSIBLEFILES_ITEM),
					hWnd, Config_CompressibleFiles_Item);
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
			int iMax = Get_Long(GetDlgItem(hWnd, IDC_MAXFILESIZE));
			if(iMax <= 0){
				MessageBox(hWnd, "You must specify a maximum file size (over 0 MB).", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}
			int iMin = Get_Long(GetDlgItem(hWnd, IDC_MINFILESIZE));
			if(iMin >= iMax){
				MessageBox(hWnd, "You must specify a minimum file size (lower then maximum size).", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}
			int iCacheSize = Get_Long(GetDlgItem(hWnd, IDC_CACHESIZE));
			if(iCacheSize <= 0){
				MessageBox(hWnd, "You must specify a maximum cahce size (greater than 1).", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}
			int iCompressLevel = Get_Long(GetDlgItem(hWnd, IDC_COMPRESSLEVEL));
			if(iCompressLevel < 1 || iCompressLevel > 9){
				MessageBox(hWnd, "You must specify a compression level (between 1 and 9).", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}
			int iMemLevel = Get_Long(GetDlgItem(hWnd, IDC_MEMORYLEVEL));
			if(iMemLevel < 1 || iMemLevel > 9){
				MessageBox(hWnd, "You must specify a memory level (between 1 and 9).", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}

			//---(Begin Server Update)------------------------------------------------------------------
			if(IsDlgButtonChecked(hWnd, IDC_USEGLOBAL) == FALSE)
			{
				XMLWriter xml;
				if(LoadNewCompressionConfiguration(hWnd, gCI.hList, &xml))
				{
					char *xmlText = xml.Text();

					XMLReader xml;
					if(gManage->QueryXMLReader(&xml, "::Update|%d:%s:%s", gCI.iSite, "Compression", xmlText))
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
					XMLReader xml;
					if(gManage->QueryXMLReader(&xml, "::Update|%d:%s:%s", gCI.iSite, "Compression", "~Global"))
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
							MAKEINTRESOURCE(IDD_CONFIG_COMPRESSIBLEFILES_ITEM),
							hWnd, Config_CompressibleFiles_Item);
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
