///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _Config_WebsiteAdvanced_CPP
#define _Config_WebsiteAdvanced_CPP
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
#include "../../@Common/WebsiteSettings.H"

#include "MainDlg.H"
#include "NetLogoDlg.H"

#include "Config_WebsiteAdvanced.H"

#include "../../../CTabControl/CTabControl.h"
#include "../../@Common/Cryptography.h"
#include "../../../NASCCL/NASCCL.H"

#include "../../../NSWFL/NSWFL.h"
using namespace NSWFL::Collections;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListView;
using namespace NSWFL::File;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK Config_WebsiteAdvanced_Username_Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	if(uMsg == WM_KEYDOWN)
	{
		Set_Text(GetDlgItem(GetParent(hWnd), IDC_NTPASSWORD), "");
	}
	else if(uMsg == WM_DESTROY)
	{
		RemoveWindowSubclass(hWnd, Config_WebsiteAdvanced_Username_Proc, uIdSubclass);
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK Config_WebsiteAdvanced_Domain_Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	if(uMsg == WM_KEYDOWN)
	{
		Set_Text(GetDlgItem(GetParent(hWnd), IDC_NTPASSWORD), "");
	}
	else if(uMsg == WM_DESTROY)
	{
		RemoveWindowSubclass(hWnd, Config_WebsiteAdvanced_Domain_Proc, uIdSubclass);
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK Config_WebsiteAdvanced(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

	static CTabControl *TabControl;
	static HWND hTabSecurity = NULL;
	static HWND hTabAdvanced = NULL;

	static HWND hMaxConnections = NULL;
    static HWND hScriptTimeout = NULL;
    static HWND hAdminEmail = NULL;
	static HWND hMaxReceiveSize = NULL;
    static HWND hKeepAliveTiemout = NULL;
    static HWND hLogPath = NULL;
    static HWND hNTUser = NULL;
    static HWND hNTDomain = NULL;
    static HWND hNTPass = NULL;

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
			"Security",
			"Advanced",
			NULL
		};

		static LPSTR dlgNames[] = {
			MAKEINTRESOURCE(IDD_CONFIG_ADVANCED_SECURITY),
			MAKEINTRESOURCE(IDD_CONFIG_ADVANCED_ADVANCED),
			NULL
		};

		TabControl = new CTabControl(GetDlgItem(hWnd, IDC_TABCONTROL), tabNames, dlgNames, &Config_WebsiteAdvanced, TRUE);

		hTabSecurity		= TabControl->Handle("Security");
        hNTUser				= GetDlgItem(hTabSecurity, IDC_NTUSERNAME);
        hNTDomain			= GetDlgItem(hTabSecurity, IDC_NTDOMAIN);
        hNTPass				= GetDlgItem(hTabSecurity, IDC_NTPASSWORD);

		hTabAdvanced		= TabControl->Handle("Advanced");
		hScriptTimeout		= GetDlgItem(hTabAdvanced, IDC_SCRIPTTIMEOUT);
		hAdminEmail			= GetDlgItem(hTabAdvanced, IDC_ADMINEMAIL);
		hMaxReceiveSize		= GetDlgItem(hTabAdvanced, IDC_MAXRECEIVESIZE);
		hKeepAliveTiemout	= GetDlgItem(hTabAdvanced, IDC_KEEPALIVETIMEOUT);
		hLogPath			= GetDlgItem(hTabAdvanced, IDC_LOGPATH);

		XMLReader xml;
		if(gManage->QueryXMLReader(&xml, "::Configuration:WebsiteSettings|%d", gCI.iSite))
		{
			char sAdminEmail[MAX_ADMIN_EMAIL];
			char sUsername[MAX_USER_LENGTH];
			char sDomain[MAX_DOMAIN_LENGTH];
			char sEncodedPassword[MAX_PASS_LENGTH];
			char sLogPath[MAX_PATH];

			bool bAllowDirectoryIndexing;
			bool bPreventImageLeaching;
			bool bCacheLogFile;
			bool bAutoStart;
			int iKeepAliveTimeout;
			int iScriptTimeout;
			int iMaxConnections;
			int iBufferSize;
			int iMaxReceiveSize;

			//Read XML.
			bAllowDirectoryIndexing = xml.ToBoolean("AllowDirectoryIndexing", false);
			bPreventImageLeaching = xml.ToBoolean("PreventImageLeaching", false);
			bCacheLogFile = xml.ToBoolean("CacheLogFile", true);
			bAutoStart = xml.ToBoolean("AutoStart", false);
			iKeepAliveTimeout = xml.ToInteger("KeepAliveTimeout", 3000);
			iScriptTimeout = xml.ToInteger("ScriptTimeout", 3000);
			iMaxConnections = xml.ToInteger("MaxConnections", 10);
			iBufferSize = xml.ToInteger("BufferSize", 1024 * 8);
			iMaxReceiveSize = xml.ToInteger("MaxReceiveSize", 1024 * 1024 * 2);
			xml.ToString("LogPath", sLogPath, sizeof(sLogPath));
			xml.ToString("ServerAdminEmail", sAdminEmail, sizeof(sAdminEmail));
			xml.ToString("Username", sUsername, sizeof(sUsername));
			xml.ToString("Domain", sDomain, sizeof(sDomain));
			xml.ToString("Password", sEncodedPassword, sizeof(sEncodedPassword));

			char sInt[64];
			CheckDlgButton(hTabAdvanced, IDC_PREVENTIMAGELEACHING, bPreventImageLeaching);
			CheckDlgButton(hTabAdvanced, IDC_DIRINDEXING, bAllowDirectoryIndexing);
			CheckDlgButton(hTabAdvanced, IDC_AUTOSTART, bAutoStart);
			Set_Text(hLogPath, sLogPath);
			Set_Text(hMaxReceiveSize, FormatInteger(sInt, sizeof(sInt), iMaxReceiveSize));
			CheckDlgButton(hTabAdvanced, IDC_CACHELOGFILE, bCacheLogFile);
			Set_Text(hScriptTimeout, FormatInteger(sInt, sizeof(sInt), iScriptTimeout));
			Set_Text(hKeepAliveTiemout, FormatInteger(sInt, sizeof(sInt), iKeepAliveTimeout));
			Set_Text(hAdminEmail, sAdminEmail);
			Set_Text(hNTUser, sUsername);
			Set_Text(hNTDomain, sDomain);

			if(strlen(sUsername) > 0)
			{
				CheckDlgButton(hTabSecurity, IDC_NTAUTHENABLED, TRUE);

				char sPassword[VIRTUALROOTS_MAX_PASS_LENGTH];

				int iRawPasswordLength = (int)Base64Decode((unsigned char *)sEncodedPassword, (int)strlen(sEncodedPassword), (unsigned char *)sPassword, (int)sizeof(sPassword));
				if (iRawPasswordLength > 0)
				{
					NASCCLStream nasccl;
					char sEncryptionKey[48];
					MakeEncryptionKey(sEncryptionKey, sizeof(sEncryptionKey), "%s\\%s", sDomain, sUsername);
					nasccl.Initialize(sEncryptionKey);
					nasccl.Cipher(sPassword, iRawPasswordLength);
					nasccl.Destroy();

					Set_Text(hNTPass, sPassword);
				}
			}
			SendMessage(hWnd, WM_COMMAND, IDC_NTAUTHENABLED, 0);

			char sText[255];
			sprintf_s(sText, sizeof(sText), "%s%s", gCI.iSite == INVALID_SITE_INDEX ? "Global " : "", "Settings");
			Set_Text(GetDlgItem(hWnd, IDC_CONFIGTITLE), sText);
			SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)sText);

			if(gCI.iSite == INVALID_SITE_INDEX)
			{
				ShowWindow(GetDlgItem(hWnd, IDC_USEGLOBAL), SW_HIDE);
				CheckDlgButton(hWnd, IDC_USEGLOBAL, BST_UNCHECKED);
			}
			else{
				if(gManage->QueryBoolean(&gCI.bGlobal, "::IsGlobal|%d:WebsiteSettings", gCI.iSite))
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

		SetWindowSubclass(hNTUser, Config_WebsiteAdvanced_Username_Proc, 0, 0);
		SetWindowSubclass(hNTDomain, Config_WebsiteAdvanced_Domain_Proc, 0, 0);

        return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }
    //--------------------------------------------------------------------------
    else if(uMsg == WM_COMMAND)
    {
		if(wParam == IDC_NTAUTHENABLED)
        {
			BOOL bEnable = IsDlgButtonChecked(hTabSecurity, IDC_NTAUTHENABLED);
			EnableWindow(hNTUser, bEnable);
			EnableWindow(hNTDomain, bEnable);
			EnableWindow(hNTPass, bEnable);
		}
		else if(wParam == IDC_USEGLOBAL)
		{
			bool bChecked = IsDlgButtonChecked(hWnd, IDC_USEGLOBAL) > 0;
			EnableWindow(GetDlgItem(hTabAdvanced, IDC_DIRINDEXING), !bChecked);
			EnableWindow(GetDlgItem(hTabAdvanced, IDC_PREVENTIMAGELEACHING), !bChecked);
			EnableWindow(GetDlgItem(hTabAdvanced, IDC_AUTOSTART), !bChecked);
			EnableWindow(GetDlgItem(hTabAdvanced, IDC_CACHELOGFILE), !bChecked);
			EnableWindow(GetDlgItem(hTabAdvanced, IDC_BROWSE), !bChecked);

			EnableWindow(hMaxReceiveSize, !bChecked);
			EnableWindow(hScriptTimeout, !bChecked);
			EnableWindow(hAdminEmail, !bChecked);
			EnableWindow(hKeepAliveTiemout, !bChecked);
			EnableWindow(hLogPath, !bChecked);

			EnableWindow(GetDlgItem(hTabSecurity, IDC_NTAUTHENABLED), !bChecked);
			EnableWindow(hNTUser, !bChecked);
			EnableWindow(hNTDomain, !bChecked);
			EnableWindow(hNTPass, !bChecked);

			if(!bChecked)
			{
				SendMessage(hWnd, WM_COMMAND, IDC_NTAUTHENABLED, 0);
			}
		}
		else if (wParam == IDC_BROWSE)
		{
			char sFolder[MAX_PATH];
			Get_Text(hLogPath, sFolder, sizeof(sFolder));
			if(BrowseDirectory(hWnd, "Select a directory to be used for log file storage.",
				BIF_RETURNONLYFSDIRS | BIF_NONEWFOLDERBUTTON, sFolder, sizeof(sFolder), sFolder))
			{
				Set_Text(hLogPath, sFolder);
			}
		}
		else if(wParam == IDC_OK)
        {
			//---(Begin Validation)------------------------------------------------------------------
			if(!IsDlgButtonChecked(hWnd, IDC_USEGLOBAL))
			{
				int iValue = 0;
				char sText[MAX_TEXT_SIZE];

				iValue = Get_Long(hScriptTimeout);
				if(iValue < 5 || iValue > 86400)
				{
					MessageBox(hWnd, "Script timeout must be between 5 and 86,400.", gsTitleCaption, MB_ICONINFORMATION);
					return TRUE;
				}

				iValue = Get_Long(hKeepAliveTiemout);
				if(iValue < 0 || iValue > 86400)
				{
					MessageBox(hWnd, "Keep-alive timeout must be between 0 (disabled) and 86,400.", gsTitleCaption, MB_ICONINFORMATION);
					return TRUE;
				}

				iValue = Get_Long(hMaxReceiveSize);
				if(iValue < 1024 || iValue > 67108864)
				{
					MessageBox(hWnd, "Max receive size must be between 1024 and 67,108,864.", gsTitleCaption, MB_ICONINFORMATION);
					return TRUE;
				}

				iValue = Get_Text(hLogPath, sText, sizeof(sText));
				if(iValue <= 0 || !FileAccess(sText, FExist) || !IsDirectory(sText))
				{
					MessageBox(hWnd, "You must specify a valid log file path.", gsTitleCaption, MB_ICONINFORMATION);
					return TRUE;
				}
			}
			//---(End Validation)------------------------------------------------------------------

			//---(Begin Server Update)------------------------------------------------------------------
			if(IsDlgButtonChecked(hWnd, IDC_USEGLOBAL) == FALSE)
			{
				XMLWriter xml;
				if(LoadNewWebsiteSettingsConfiguration(hWnd, gCI.hList, &xml, TabControl))
				{
					char *xmlText = xml.Text();

					XMLReader xml;
					if(gManage->QueryXMLReader(&xml, "::Update|%d:%s:%s", gCI.iSite, "WebsiteSettings", xmlText))
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
					if(gManage->QueryXMLReader(&xml, "::Update|%d:%s:%s", gCI.iSite, "WebsiteSettings", "~Global"))
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

			EndDialog(hWnd, 0);
			DestroyWindow(hWnd);
			delete TabControl;
        }
        else if(wParam == IDC_CANCEL || wParam == IDCANCEL)
        {
			EndDialog(hWnd, 0);
			DestroyWindow(hWnd);
			delete TabControl;
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
