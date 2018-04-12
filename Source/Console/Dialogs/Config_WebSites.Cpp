///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _Config_WebSites_CPP
#define _Config_WebSites_CPP
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
#include "../../@Common/WebSite.H"

#include "MainDlg.H"
#include "NetLogoDlg.H"

#include "Config_WebSites.H"
#include "Config_WebSites_Item.H"
#include "CommandWaitDialog.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK Config_WebSites(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
		if(gManage->QueryXMLReader(&xml, "::Configuration:WebSites|"))
		{
			PopWebsitesList(gCI.hList, false, &xml, true);
		}

		Set_Text(GetDlgItem(hWnd, IDC_CONFIGTITLE), "Web Sites");
        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)"Web Sites");
		ShowWindow(GetDlgItem(hWnd, IDC_USEGLOBAL), SW_HIDE);
		ShowWindow(GetDlgItem(hWnd, IDC_ENABLED), SW_HIDE);
	}
    //--------------------------------------------------------------------------
    else if(uMsg == WM_COMMAND)
    {
        if(wParam == IDC_ADD)
        {
			CLocalConfigItemStorage localStorage(&gCI);

			gCI.iListIndex = INVALID_LIST_INDEX;
			gCI.iSite = INVALID_SITE_INDEX;

			DialogBox(ghAppInstance,
				MAKEINTRESOURCE(IDD_CONFIG_WEBSITES_ITEM),
				hWnd, Config_WebSites_Item);
		}
        else if(wParam == IDC_EDIT)
        {
			CLocalConfigItemStorage localStorage(&gCI);

			if((gCI.iListIndex = LVGetFirstSelectedItemIndex(gCI.hList)) >= 0)
			{
				char sIndex[64];
				LVGetText(gCI.hList, gCI.iListIndex, WEBSITES_LIST_POS_INDEX, sIndex, sizeof(sIndex));
				gCI.iSite = atol(sIndex);

				DialogBox(ghAppInstance,
					MAKEINTRESOURCE(IDD_CONFIG_WEBSITES_ITEM),
					hWnd, Config_WebSites_Item);
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
			//FIXFIX: Doest not except response.

			XMLWriter xml;
			if(LoadNewWebSitessConfiguration(hWnd, gCI.hList, &xml))
			{
				bool bResult = false;
				char *xmlText = xml.Text();

				XMLReader xml;
				if(ProcessLengthyCommand(&xml, hWnd,
					"Please wait while the new websites configuration is applied. This process can take a long time"
					" depending on the changes which were made and the number of connections to any modified sites.",
					"::Update:Websites|%s", xmlText))
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
				if(ListView_GetItemCount(gCI.hList) > 0 && ListView_GetSelectedCount(gCI.hList) == 1)
				{
					CLocalConfigItemStorage localStorage(&gCI);

					if((gCI.iListIndex = LVGetFirstSelectedItemIndex(gCI.hList)) >= 0)
					{
						char sIndex[64];
						LVGetText(gCI.hList, gCI.iListIndex, WEBSITES_LIST_POS_INDEX, sIndex, sizeof(sIndex));
						gCI.iSite = atol(sIndex);

						DialogBox(ghAppInstance,
							MAKEINTRESOURCE(IDD_CONFIG_WEBSITES_ITEM),
							hWnd, Config_WebSites_Item);
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
