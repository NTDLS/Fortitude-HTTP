///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2010, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _TABGENERAL_CPP
#define _TABGENERAL_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../Source/Entry.H"
#include "../Source/Routines.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK TabGeneralDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    if(uMsg == WM_INITDIALOG)
    {

		// ------------------------------------(Fill in general information end)

        return TRUE;
    }
    else if(uMsg == WM_COMMAND)
    {
		/*
        if(wParam == IDC_HELPBUTTON)
        {
			OpenHelpFile("Configuration/General.Html");
            return TRUE;
        }
		*/
	}
    else if(uMsg == WM_NOTIFY)
    {
        LPNMHDR pNMH = (LPNMHDR)lParam;
        if(pNMH->code == PSN_KILLACTIVE)
        {
            return FALSE;
        }
        if(pNMH->code == PSN_APPLY) // Ok
        {
            return TRUE;
        }
        if(pNMH->code == PSN_RESET) // Cancel
        {
            return FALSE;
        }
    }

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
