///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _Config_ScriptingEngines_Item_CPP
#define _Config_ScriptingEngines_Item_CPP
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
#include "../../@Common/ScriptingEngines.H"

#include "MainDlg.H"
#include "NetLogoDlg.H"

#include "Config_ScriptingEngines.H"
#include "Config_ScriptingEngines_Item.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListView;
using namespace NSWFL::File;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK Config_ScriptingEngines_Item(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

    static HWND hExtension = NULL;
    static HWND hExecutable = NULL;
	static HWND hSuccessCode = NULL;
    static HWND hParameters = NULL;
    static HWND hDescr = NULL;

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
		char sText[MAX_TEXT_SIZE];
		CenterOverOwner(hWnd);

        HMENU hMenu = GetSystemMenu(hWnd, FALSE);
        AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
        AppendMenu(hMenu, MF_STRING, GENERIC_MENU_ABOUT, "About");

        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)));

        hExtension		= GetDlgItem(hWnd, IDC_EXTENSION);
        hExecutable		= GetDlgItem(hWnd, IDC_EXECUTABLE);
		hSuccessCode	= GetDlgItem(hWnd, IDC_SUCCESSCODE);
        hParameters		= GetDlgItem(hWnd, IDC_PARAMETERS);
        hDescr			= GetDlgItem(hWnd, IDC_DESCR);

		if(gCI.iListIndex != INVALID_LIST_INDEX) //Editing an item.
		{
			//We have a list index, so we must be editing.

			LVGetItemText(gCI.hList, gCI.iListIndex, SCRIPTENGINES_LIST_POS_EXT, sText, sizeof(sText));
			Set_Text(hExtension, sText);

			LVGetItemText(gCI.hList, gCI.iListIndex, SCRIPTENGINES_LIST_POS_ENGINE, sText, sizeof(sText));
			Set_Text(hExecutable, sText);

			LVGetItemText(gCI.hList, gCI.iListIndex, SCRIPTENGINES_LIST_POS_SUCCESSCODE, sText, sizeof(sText));
			Set_Text(hSuccessCode, sText);

			LVGetItemText(gCI.hList, gCI.iListIndex, SCRIPTENGINES_LIST_POS_USESUCCESSCODE, sText, sizeof(sText));
			CheckDlgButton(hWnd, IDC_USESUCCESSCODE, ParseBoolean(sText));

			LVGetItemText(gCI.hList, gCI.iListIndex, SCRIPTENGINES_LIST_POS_PARAMS, sText, sizeof(sText));
			Set_Text(hParameters, sText);

			LVGetItemText(gCI.hList, gCI.iListIndex, SCRIPTENGINES_LIST_POS_DESCR, sText, sizeof(sText));
			Set_Text(hDescr, sText);

			CheckDlgButton(hWnd, IDC_ENABLED, LVChecked(gCI.hList, gCI.iListIndex));
		}
		else{
			//Adding an item.
			CheckDlgButton(hWnd, IDC_ENABLED, BST_CHECKED);
			CheckDlgButton(hWnd, IDC_USESUCCESSCODE, BST_CHECKED);
			Set_Text(hSuccessCode, "0");
			Set_Text(hParameters, "\"%1\"");
		}

		sprintf_s(sText, sizeof(sText), "%s%s", gCI.iSite == INVALID_SITE_INDEX ? "Global " : "", "Scripting Engine");
		Set_Text(GetDlgItem(hWnd, IDC_CONFIGTITLE), sText);
        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)sText);

		SendMessage(hWnd, WM_COMMAND, IDC_USESUCCESSCODE, 0);

		return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }
    //--------------------------------------------------------------------------
    else if(uMsg == WM_COMMAND)
    {
		if(wParam == IDC_USESUCCESSCODE)
		{
			bool bChecked = IsDlgButtonChecked(hWnd, IDC_USESUCCESSCODE) > 0;
			EnableWindow(hSuccessCode, bChecked);
		}
		else if(wParam == IDC_BROWSE)
        {
			char sCurDir[MAX_PATH];
			if(Get_CurrentDirectory(sCurDir, sizeof(sCurDir)))
			{
				char sFileName[MAX_PATH];
				Get_Text(hExecutable, sFileName, sizeof(sFileName));
				if(OSFileDialog(hWnd, sFileName, sizeof(sFileName), "exe",
						"Select a scripting engine.", "Script Engines\0*.exe\0\0",
						sCurDir, OFD, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST))
				{
					Set_Text(hExecutable, sFileName);
				}
			}
		}
		else if(wParam == IDC_OK)
        {
			SCRIPTENGINE Item;

			char sExt[SCRIPTINGENGINES_MAX_EXT_LEGTH];
			char sEngine[SCRIPTINGENGINES_MAX_ENGINE_LEGTH];
			char sParams[SCRIPTINGENGINES_MAX_PARAMS_LEGTH];
			char sDescr[SCRIPTINGENGINES_MAX_DESCR_LEGTH];
			int iSuccessCode = 0;

			Get_Text(hExtension, sExt, sizeof(sExt));
			Get_Text(hExecutable, sEngine, sizeof(sEngine));
			Get_Text(hParameters, sParams, sizeof(sParams));
			Get_Text(hDescr, sDescr, sizeof(sDescr));
			iSuccessCode = Get_Long(hSuccessCode);

			if(strlen(sExt) == 0)
			{
				MessageBox(hWnd, "You must specify a file extension.", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}
			if(strlen(sEngine) == 0)
			{
				MessageBox(hWnd, "You must specify a scripting engine.", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}

			if(sExt[0] != '.')
			{
				char sTmp[sizeof(sExt)];
				sprintf_s(sTmp, sizeof(sTmp), ".%s", sExt);
				strcpy_s(sExt, sizeof(sExt), sTmp);
			}

			Item.Extension = sExt;
			Item.Parameters = sParams;
			Item.Engine = sEngine;
			Item.Description = sDescr;
			Item.Enabled =  (IsDlgButtonChecked(hWnd, IDC_ENABLED) > 0);
			Item.SuccessCode = iSuccessCode;
			Item.UseSuccessCode = (IsDlgButtonChecked(hWnd, IDC_USESUCCESSCODE) > 0);

			if(gCI.iListIndex != INVALID_LIST_INDEX) //Editing / Adding
			{
				//We have a list index, so we must be editing.
				LVDeleteItem(gCI.hList, gCI.iListIndex);
				PopScriptingEnginesList_AddItem(gCI.hList, &Item, gCI.iListIndex);
			}
			else {
				//We DO NOT have a list index, so we must be adding.
				PopScriptingEnginesList_AddItem(gCI.hList, &Item);
			}

			LVAutoSizeColumns(gCI.hList, 500);
            EndDialog(hWnd, TRUE);
            DestroyWindow(hWnd);
        }
        else if(wParam == IDC_CANCEL || wParam == IDCANCEL)
        {
            EndDialog(hWnd, FALSE);
            DestroyWindow(hWnd);
        }
        return FALSE;
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
        return TRUE;
    }
    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
