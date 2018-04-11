///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _DetailList_CPP
#define _DetailList_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <Stdlib.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../@Common/Cryptography.h"

#include "../Source/Entry.H"
#include "../Source/ConfigTree.H"
#include "../Source/DetailList.H"

#include "../Dialogs/MainDlg.H"
#include "../Dialogs/NetLogoDlg.H"

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
#include "../Dialogs/Config_ServerSettings.H"
#include "../Dialogs/Config_ServerSettings_User_Item.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::File;
using namespace NSWFL::ListView;
using namespace NSWFL::Windows;
using namespace NSWFL::Conversion;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern HIMAGELIST hEnableDisableImageList; //Declared in MainDialog.cpp
extern HIMAGELIST hOnePixilImageList; //Declared in MainDialog.cpp
extern HIMAGELIST hPoolActiveInactiveImageList; //Declared in MainDialog.cpp
extern HIMAGELIST hWebsiteStartedStoppedList; //Declared in MainDialog.cpp

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FreeLParamMemory(HWND hList)
{
	int iGridRows = LVItemCount(hList);
	for(int iRow = 0; iRow < iGridRows; iRow++)
	{
		FreeLParamMemory(hList, iRow);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FreeLParamMemory(HWND hList, int iRow)
{
	LPARAM lAddress = LVGetItemParam(hList, iRow, 0);
	if((char *)lAddress)
	{
		//MsgBox((char *)lAddress);
		pMem->Free((char *)lAddress);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LPARAM GetSelectedComboBoxItemData(HWND hCombo)
{
	int iSelectedItem = (int)SendMessage(hCombo, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	if(iSelectedItem >= 0)
	{
		return SendMessage(hCombo, (UINT)CB_GETITEMDATA, (WPARAM)iSelectedItem, (LPARAM)NULL);
	}
	else
	{
		return NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SelectComboBoxItemByData(HWND hCombo, const char *sData)
{
	int iItemCount = (int)SendMessage(hCombo, (UINT)CB_GETCOUNT, (WPARAM)0, (LPARAM)0);

	for(int i = 0; i < iItemCount; i++)
	{
		char *sItemData = (char *)SendMessage(hCombo, (UINT)CB_GETITEMDATA, (WPARAM)i, (LPARAM)NULL);
		if(sItemData)
		{
			if(_strcmpi(sItemData, sData) == 0)
			{
				SendMessage(hCombo, (UINT)CB_SETCURSEL, (WPARAM)i, (LPARAM)NULL);
				return true;
			}
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FreeComboBoxMemory(HWND hCombo)
{
	int iItemCount = (int)SendMessage(hCombo, (UINT)CB_GETCOUNT, (WPARAM)0, (LPARAM)0);

	for(int i = 0; i < iItemCount; i++)
	{
		char *sItemData = (char *)SendMessage(hCombo, (UINT)CB_GETITEMDATA, (WPARAM)i, (LPARAM)NULL);
		if(sItemData)
		{
			pMem->Free(sItemData);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopListeningIPsList_AddItem(HWND hList, LISTENIP *obj)
{
	PopListeningIPsList_AddItem(hList, obj, ListView_GetItemCount(hList));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopListeningIPsList_AddItem(HWND hList, LISTENIP *obj, int iInsertionIndex)
{
	LV_ITEM ListItem;
    memset(&ListItem, 0, sizeof(ListItem));

	int iImage = obj->Enabled;

	ListItem.pszText = (char *) obj->IP;
    ListItem.mask = LVIF_TEXT|LVIF_IMAGE;
	ListItem.iImage = iImage;
    ListItem.iSubItem = SOCKETPOOLS_IPS_LIST_POS_IP;
	ListItem.iItem = iInsertionIndex;
    int iItem = ListView_InsertItem(hList, &ListItem);

    ListItem.pszText = (char *) obj->Description;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = SOCKETPOOLS_IPS_LIST_POS_DESCR;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListView_SetCheckState(hList, iItem, obj->Enabled);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopListeningIPsList(HWND hList, bool bUseIcons, XMLReader *pXML)
{
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)FALSE, (LPARAM)0);

    LV_COLUMN lvColumn;
    memset(&lvColumn, 0, sizeof(LV_COLUMN));

	lvColumn.cx        = 100;
    lvColumn.pszText   = "IP Address";
    lvColumn.iSubItem  = SOCKETPOOLS_IPS_LIST_POS_IP;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, SOCKETPOOLS_IPS_LIST_POS_IP, &lvColumn);

	lvColumn.cx        = 300;
    lvColumn.pszText   = "Description";
    lvColumn.iSubItem  = SOCKETPOOLS_IPS_LIST_POS_DESCR;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, SOCKETPOOLS_IPS_LIST_POS_DESCR, &lvColumn);

	DWORD dwFlags = LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP;

	if(!bUseIcons)
	{
		dwFlags |= LVS_EX_CHECKBOXES;
	}

    SendMessage(hList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)dwFlags);

	if(bUseIcons)
	{
		ListView_SetImageList(hList, hEnableDisableImageList, LVSIL_SMALL);
	}

	ListView_SetTextColor(hList, RGB(0,0,100));

	if(pXML)
	{
		char sIP[SOCKETPOOLS_IPS_MAX_IP_LENGTH];
		char sDescription[SOCKETPOOLS_IPS_MAX_DESCR_LENGTH];
		bool bEnable = false;

		XMLReader xmlSubItem;
		pXML->ToReader("IPAddresses", &xmlSubItem);

		xmlSubItem.ScanPosition(0);
		xmlSubItem.ProgressiveScan(true);

		//Pop the list-view.
		XMLReader xml;
		for(XMLReader xml; xmlSubItem.ToReader("IPAddress", &xml);)
		{
			//Read XML.
			xml.ToString("Address", sIP, sizeof(sIP));
			xml.ToString("Description", sDescription, sizeof(sDescription));
			bEnable = xml.ToBoolean("Enable");

			xml.Destroy();

			//Build an object
			LISTENIP obj;
			obj.IP = sIP;
			obj.Description = sDescription;
			obj.Enabled = bEnable;

			//Add the object to the list.
			PopListeningIPsList_AddItem(hList, &obj);
		}
	}

	LVAutoSizeColumns(hList, 500);
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)TRUE, (LPARAM)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LoadNewListeningIPsConfiguration(HWND hWnd, HWND hGrid, bool bListenOnAll, XMLWriter *pXML)
{
	int iGridItems = LVItemCount(hGrid);

	char sIP[SOCKETPOOLS_IPS_MAX_IP_LENGTH];
	char sDescr[SOCKETPOOLS_IPS_MAX_DESCR_LENGTH];

	pXML->Initialize("IPAddresses");

	pXML->AddBool("ListenOnAll", bListenOnAll);

	for(int iItem = 0; iItem < iGridItems; iItem++)
	{
		XMLWriter Item("IPAddress");

		LVGetText(hGrid, iItem, SOCKETPOOLS_IPS_LIST_POS_IP, sIP, sizeof(sIP));
		Item.Add("Address", sIP);

		LVGetText(hGrid, iItem, SOCKETPOOLS_IPS_LIST_POS_DESCR, sDescr, sizeof(sDescr));
		Item.Add("Description", sDescr);

		Item.AddBool("Enable", ParseBoolean(LVChecked(hGrid, iItem)));

		pXML->Add(&Item);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopCacheList_AddItem(HWND hList, COMPRESSIONCACHEITEM *obj)
{
	LV_ITEM ListItem;
    memset(&ListItem, 0, sizeof(ListItem));
	char sNumber[64];

	int iInsertionIndex = ListView_GetItemCount(hList);

    ListItem.pszText = (char *) obj->OriginalFile;
    ListItem.mask = LVIF_TEXT|LVIF_IMAGE;
    ListItem.iSubItem = CACHE_LIST_POS_FILENAME;
	ListItem.iItem = iInsertionIndex;
    int iItem = ListView_InsertItem(hList, &ListItem);

    ListItem.pszText = (char *) FileSizeFriendly(obj->OriginalSize, sNumber, sizeof(sNumber));
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = CACHE_LIST_POS_ORIGINALSIZE;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

    ListItem.pszText = (char *) FileSizeFriendly(obj->CachedSize, sNumber, sizeof(sNumber));
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = CACHE_LIST_POS_CACHEDSIZE;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

    ListItem.pszText = (char *) FormatInteger(sNumber, sizeof(sNumber), obj->Hits);
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = CACHE_LIST_POS_HITS;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopCacheList(HWND hList, XMLReader *pXML)
{
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)FALSE, (LPARAM)0);

    LV_COLUMN lvColumn;
    memset(&lvColumn, 0, sizeof(LV_COLUMN));

	lvColumn.cx        = 300;
    lvColumn.pszText   = "File Name";
    lvColumn.iSubItem  = CACHE_LIST_POS_FILENAME;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, CACHE_LIST_POS_FILENAME, &lvColumn);

    lvColumn.cx        = 100;
    lvColumn.pszText   = "Original Size";
    lvColumn.iSubItem  = CACHE_LIST_POS_ORIGINALSIZE;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, CACHE_LIST_POS_ORIGINALSIZE, &lvColumn);

    lvColumn.cx        = 100;
    lvColumn.pszText   = "Cached Size";
    lvColumn.iSubItem  = CACHE_LIST_POS_CACHEDSIZE;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, CACHE_LIST_POS_CACHEDSIZE, &lvColumn);

    lvColumn.cx        = 50;
    lvColumn.pszText   = "Hits";
    lvColumn.iSubItem  = CACHE_LIST_POS_HITS;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, CACHE_LIST_POS_HITS, &lvColumn);

	DWORD dwFlags = LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP;

    SendMessage(hList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)dwFlags);

	ListView_SetImageList(hList, hOnePixilImageList, LVSIL_SMALL);

	ListView_SetTextColor(hList, RGB(0,0,100));
	
	char sFile[MAX_PATH];
	int iOriginalSize;
	int iCachedSize;
	int iHits;

	pXML->ProgressiveScan(true);

	//Pop the list-view.
	for(XMLReader xml; pXML->ToReader("Item", &xml);)
	{
		//Read XML.
		xml.ToString("File", sFile, sizeof(sFile));
		iOriginalSize = xml.ToInteger("OriginalSize");
		iCachedSize = xml.ToInteger("CachedSize");
		iHits = xml.ToInteger("Hits");

		xml.Destroy();

		//Build an object
		COMPRESSIONCACHEITEM obj;
		obj.OriginalFile = sFile;
		obj.OriginalSize = iOriginalSize;
		obj.CachedSize = iCachedSize;
		obj.Hits = iHits;

		//Add the object to the list.
		PopCacheList_AddItem(hList, &obj);
	}

	LVAutoSizeColumns(hList, 500);
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)TRUE, (LPARAM)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopWebsite_AddItem(HWND hList, WEBSITE *obj, int iSiteIndex, int iInsertionIndex)
{
	LV_ITEM LVItem;
    memset(&LVItem, 0, sizeof(LVItem));

	char sInt[64];

	int iImage = 0;
	
	if(iSiteIndex != INVALID_SITE_INDEX)
	{
		//iImage = this->Site(iSiteIndex)->pSocket->IsActive();
	}
	else{
		//Newly added site.
	}

    LVItem.pszText = (char *) obj->Name;
    LVItem.mask = LVIF_TEXT|LVIF_IMAGE;
	LVItem.iImage = iImage;
    LVItem.iSubItem = WEBSITES_LIST_POS_NAME;
	LVItem.iItem = iInsertionIndex;
    ListView_InsertItem(hList, &LVItem);

    LVItem.pszText = (char *) obj->Description;
    LVItem.mask = LVIF_TEXT;
    LVItem.iSubItem = WEBSITES_LIST_POS_DESCR;
	LVItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &LVItem);

    LVItem.pszText = (char *) obj->Root;
    LVItem.mask = LVIF_TEXT;
    LVItem.iSubItem = WEBSITES_LIST_POS_ROOT;
	LVItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &LVItem);

	sprintf_s(sInt, sizeof(sInt), "%d", iSiteIndex);
    LVItem.pszText = (char *) sInt;
    LVItem.mask = LVIF_TEXT;
    LVItem.iSubItem = WEBSITES_LIST_POS_INDEX;
	LVItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &LVItem);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopWebsite_AddItem(HWND hList, const char *sAttribute, const char *sValue)

{
	LV_ITEM LVItem;
    memset(&LVItem, 0, sizeof(LVItem));

	int iIndex = ListView_GetItemCount(hList);

    LVItem.pszText = (char *) sAttribute;
    LVItem.mask = LVIF_TEXT;
    LVItem.iSubItem = WEBSITES_LIST_POS_ATTRIB;
	LVItem.iItem = iIndex;
    ListView_InsertItem(hList, &LVItem);

    LVItem.pszText = (char *) sValue;
    LVItem.mask = LVIF_TEXT;
    LVItem.iSubItem = WEBSITES_LIST_POS_VALUE;
	LVItem.iItem = iIndex;
    ListView_SetItem(hList, &LVItem);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopWebsiteOverviewList(HWND hList, XMLReader *pXML)
{
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)FALSE, (LPARAM)0);

    LV_COLUMN lvColumn;
    memset(&lvColumn, 0, sizeof(LV_COLUMN));

	lvColumn.cx        = 100;
    lvColumn.pszText   = "Attribute";
    lvColumn.iSubItem  = WEBSITES_LIST_POS_ATTRIB;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, WEBSITES_LIST_POS_ATTRIB, &lvColumn);

	lvColumn.cx        = 100;
    lvColumn.pszText   = "Value";
    lvColumn.iSubItem  = WEBSITES_LIST_POS_VALUE;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, WEBSITES_LIST_POS_VALUE, &lvColumn);

	DWORD dwFlags = LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP;

    SendMessage(hList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)dwFlags);

	ListView_SetImageList(hList, hOnePixilImageList, LVSIL_SMALL);

	ListView_SetTextColor(hList, RGB(0,0,100));
	
	char sName[MAX_SITE_NAME];
	char sDescription[MAX_SITE_DESCR];
	char sRoot[MAX_PATH];
	int iMaxConnections;
	int iPort;

	//Pop the list-view.
	XMLReader xml;
	if(pXML->ToReader("Site", &xml))
	{
		//Read XML.
		xml.ToString("Name", sName, sizeof(sName));
		xml.ToString("Description", sDescription, sizeof(sDescription));
		xml.ToString("Root", sRoot, sizeof(sRoot));

		//Add the object to the list.
		char sInt[64];
		PopWebsite_AddItem(hList, "Name", sName);
		PopWebsite_AddItem(hList, "Description", sDescription);
		PopWebsite_AddItem(hList, "Root", sRoot);

		if(xml.ToInteger("Port", &iPort))
		{
			PopWebsite_AddItem(hList, "Port", FormatInteger(sInt, sizeof(sInt), iPort));
		}
		
		if(xml.ToInteger("MaxConnections", &iMaxConnections))
		{
			PopWebsite_AddItem(hList, "Max Connections", FormatInteger(sInt, sizeof(sInt), iMaxConnections));
		}

		xml.Destroy();
	}

	LVAutoSizeColumns(hList, 500);
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)TRUE, (LPARAM)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopWebsitesList_AddItem(HWND hList, WEBSITE *obj, const char *sXML)
{
	PopWebsitesList_AddItem(hList, obj, ListView_GetItemCount(hList), sXML);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopWebsitesList_AddItem(HWND hList, WEBSITE *obj, int iInsertionIndex, const char *sXML)
{
	LV_ITEM ListItem;
    memset(&ListItem, 0, sizeof(ListItem));

	char sInt[64];

	int iImage = 0;
	
	if(obj->Index != INVALID_SITE_INDEX)
	{
		iImage = obj->IsActive;
	}
	else{
		//Newly added site.
	}

	if(sXML)
	{
		size_t iXMLLength = strlen(sXML);
		ListItem.lParam = (LPARAM) pMem->Allocate(iXMLLength + 1, sizeof(char));
		strcpy_s((char *)ListItem.lParam, iXMLLength + 1, sXML); //Allocate RAM to LPARAM. MUST CLEAN UP!!
	}
	
    ListItem.pszText = (char *) obj->Name;
    ListItem.mask = LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
	ListItem.iImage = iImage;
    ListItem.iSubItem = WEBSITES_LIST_POS_NAME;
	ListItem.iItem = iInsertionIndex;
    ListView_InsertItem(hList, &ListItem);

    ListItem.pszText = (char *) obj->Description;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = WEBSITES_LIST_POS_DESCR;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

    ListItem.pszText = (char *) obj->Root;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = WEBSITES_LIST_POS_ROOT;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	sprintf_s(sInt, sizeof(sInt), "%d", obj->Index);
    ListItem.pszText = (char *) sInt;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = WEBSITES_LIST_POS_INDEX;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LoadNewWebSitessConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML)
{
	int iGridItems = ListView_GetItemCount(hGrid);

	char sName[MAX_SITE_NAME];
	char sDescription[MAX_SITE_DESCR];
	char sRoot[MAX_PATH];
	char sIndex[32];

	pXML->Initialize("WebSites");

	for(int iItem = 0; iItem < iGridItems; iItem++)
	{
		XMLWriter Item("Site");

		LVGetText(hGrid, iItem, WEBSITES_LIST_POS_INDEX, sIndex, sizeof(sIndex));
		Item.Add("Index", sIndex);
		LVGetText(hGrid, iItem, WEBSITES_LIST_POS_NAME, sName, sizeof(sName));
		Item.Add("Name", sName);
		LVGetText(hGrid, iItem, WEBSITES_LIST_POS_DESCR, sDescription, sizeof(sDescription));
		Item.Add("Description", sDescription);
		LVGetText(hGrid, iItem, WEBSITES_LIST_POS_ROOT, sRoot, sizeof(sRoot));
		Item.Add("Root", sRoot);

		LPARAM xmlParam = LVGetItemParam(hGrid, iItem, 0);
		const char *sXMLText = (char *)xmlParam;
		if(sXMLText)
		{
			XMLReader xmlWhole(sXMLText);

			XMLReader xmlIPAddresses(sXMLText);
			xmlIPAddresses.ToReader("IPAddresses", &xmlIPAddresses);

			Item.Add("IPAddresses", &xmlIPAddresses);
		}

		pXML->Add(&Item);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopWebsitesList(HWND hList, bool bUseIcons, XMLReader *pXML, bool bSaveXMLInLParam)
{
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)FALSE, (LPARAM)0);

    LV_COLUMN lvColumn;
    memset(&lvColumn, 0, sizeof(LV_COLUMN));

	lvColumn.cx        = 100;
    lvColumn.pszText   = "Name";
    lvColumn.iSubItem  = WEBSITES_LIST_POS_ATTRIB;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, WEBSITES_LIST_POS_NAME, &lvColumn);

	lvColumn.cx        = 500;
    lvColumn.pszText   = "Description";
    lvColumn.iSubItem  = WEBSITES_LIST_POS_VALUE;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, WEBSITES_LIST_POS_DESCR, &lvColumn);

	lvColumn.cx        = 500;
    lvColumn.pszText   = "Root";
    lvColumn.iSubItem  = WEBSITES_LIST_POS_VALUE;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, WEBSITES_LIST_POS_ROOT, &lvColumn);

	lvColumn.cx        = 0;
    lvColumn.pszText   = "Index";
    lvColumn.iSubItem  = WEBSITES_LIST_POS_INDEX;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, WEBSITES_LIST_POS_INDEX, &lvColumn);

	DWORD dwFlags = LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP;

	if(!bUseIcons)
	{
		//dwFlags |= LVS_EX_CHECKBOXES; //This grid does not have use for checkboxes.
	}

    SendMessage(hList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)dwFlags);

	if(bUseIcons)
	{
		ListView_SetImageList(hList, hWebsiteStartedStoppedList, LVSIL_SMALL);
	}

	ListView_SetTextColor(hList, RGB(0,0,100));
	
	char sName[MAX_SITE_NAME];
	char sDescription[MAX_SITE_DESCR];
	char sRoot[MAX_PATH];
	int iIndex;
	bool bIsActive;

	pXML->ProgressiveScan(true);

	//Pop the list-view.
	for(XMLReader xml; pXML->ToReader("Site", &xml);)
	{
		//Read XML.
		xml.ToString("Name", sName, sizeof(sName));
		xml.ToString("Description", sDescription, sizeof(sDescription));
		xml.ToString("Root", sRoot, sizeof(sRoot));
		iIndex = xml.ToInteger("Index");
		bIsActive = xml.ToBoolean("IsActive");

		WEBSITE obj;
		obj.Name = sName;
		obj.Description = sDescription;
		obj.Root = sRoot;
		obj.Index = iIndex;
		obj.IsActive = bIsActive;

		const char *sXMLText = NULL;
		if(bSaveXMLInLParam)
		{
			 sXMLText = xml.Text();
		}

		PopWebsitesList_AddItem(hList, &obj, sXMLText);

		xml.Destroy();
	}

	LVAutoSizeColumns(hList, 500);
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)TRUE, (LPARAM)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopWebsiteSettingsList_AddItem(HWND hList, const char *sAttribute, const char *sValue)
{
	LV_ITEM ListItem;
    memset(&ListItem, 0, sizeof(ListItem));

	int iIndex = ListView_GetItemCount(hList);

    ListItem.pszText = (char *) sAttribute;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = SETTINGS_LIST_POS_ATTRIB;
	ListItem.iItem = iIndex;
    ListView_InsertItem(hList, &ListItem);

    ListItem.pszText = (char *) sValue;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = SETTINGS_LIST_POS_VALUE;
	ListItem.iItem = iIndex;
    ListView_SetItem(hList, &ListItem);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopWebsiteSettingsList_AddItem(HWND hList, const char *sAttribute, bool bValue)
{
	if(bValue)
	{
		PopWebsiteSettingsList_AddItem(hList, sAttribute, "Yes");
	}
	else{
		PopWebsiteSettingsList_AddItem(hList, sAttribute, "No");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopWebsiteSettingsList_AddItem(HWND hList, const char *sAttribute, double dValue)
{
	char sValue[64];
	sprintf_s(sValue, sizeof(sValue), "%f", dValue);
	PopWebsiteSettingsList_AddItem(hList, sAttribute, sValue);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopWebsiteSettingsList_AddItem(HWND hList, const char *sAttribute, unsigned int iValue)
{
	char sValue[64];
	sprintf_s(sValue, sizeof(sValue), "%d", iValue);
	PopWebsiteSettingsList_AddItem(hList, sAttribute, sValue);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopWebsiteSettingsList_AddItem(HWND hList, const char *sAttribute, int iValue)
{
	char sValue[64];
	sprintf_s(sValue, sizeof(sValue), "%d", iValue);
	PopWebsiteSettingsList_AddItem(hList, sAttribute, sValue);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopWebsiteSettingsList(HWND hList, XMLReader *pXML)
{
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)FALSE, (LPARAM)0);

    LV_COLUMN lvColumn;
    memset(&lvColumn, 0, sizeof(LV_COLUMN));

	lvColumn.cx        = 100;
    lvColumn.pszText   = "Attribute";
    lvColumn.iSubItem  = SETTINGS_LIST_POS_ATTRIB;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, SETTINGS_LIST_POS_ATTRIB, &lvColumn);

	lvColumn.cx        = 100;
    lvColumn.pszText   = "Value";
    lvColumn.iSubItem  = SETTINGS_LIST_POS_VALUE;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, SETTINGS_LIST_POS_VALUE, &lvColumn);

	DWORD dwFlags = LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP;

    SendMessage(hList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)dwFlags);

	ListView_SetImageList(hList, hOnePixilImageList, LVSIL_SMALL);

	ListView_SetTextColor(hList, RGB(0,0,100));
	
	char sAdminEmail[MAX_ADMIN_EMAIL];
	char sUsername[MAX_USER_LENGTH];
	char sDomain[MAX_DOMAIN_LENGTH];
	char sPassword[MAX_PASS_LENGTH];
	char sLogPath[MAX_PATH];

	bool bAllowDirectoryIndexing;
	bool bPreventImageLeaching;
	bool bCacheLogFile;
	bool bAutoStart;
	int iKeepAliveTimeout;
	int iScriptTimeout;
	int iMaxReceiveSize;

	//Pop the list-view.
	XMLReader xml;
	if(pXML->ToReader("WebsiteSettings", &xml))
	{
		//Read XML.
		bAllowDirectoryIndexing = xml.ToBoolean("AllowDirectoryIndexing", false);
		bPreventImageLeaching = xml.ToBoolean("PreventImageLeaching", false);
		bCacheLogFile = xml.ToBoolean("CacheLogFile", true);
		bAutoStart = xml.ToBoolean("AutoStart", false);
		iKeepAliveTimeout = xml.ToInteger("KeepAliveTimeout", 3000);
		iScriptTimeout = xml.ToInteger("ScriptTimeout", 3000);
		iMaxReceiveSize = xml.ToInteger("MaxReceiveSize", 1024 * 1024 * 2);
		xml.ToString("LogPath", sLogPath, sizeof(sLogPath));
		xml.ToString("ServerAdminEmail", sAdminEmail, sizeof(sAdminEmail));
		xml.ToString("Username", sUsername, sizeof(sUsername));
		xml.ToString("Domain", sDomain, sizeof(sDomain));
		xml.ToString("Password", sPassword, sizeof(sPassword));
		xml.Destroy();

		//Add the object to the list.
		char sInt[64];
		PopWebsiteSettingsList_AddItem(hList, "Allow Directory Indexing", bAllowDirectoryIndexing);
		PopWebsiteSettingsList_AddItem(hList, "Prevent Image Leaching", bPreventImageLeaching);
		PopWebsiteSettingsList_AddItem(hList, "Auto Start", bAutoStart);
		PopWebsiteSettingsList_AddItem(hList, "Log Path", sLogPath);
		PopWebsiteSettingsList_AddItem(hList, "Max Receive Size", FileSizeFriendly(iMaxReceiveSize, 2, sInt, sizeof(sInt)));
		PopWebsiteSettingsList_AddItem(hList, "Cache Log File", bCacheLogFile);
		PopWebsiteSettingsList_AddItem(hList, "Script Timeout",  FormatInteger(sInt, sizeof(sInt), iScriptTimeout));
		PopWebsiteSettingsList_AddItem(hList, "Keep Alive Timeout",  FormatInteger(sInt, sizeof(sInt), iKeepAliveTimeout));
		PopWebsiteSettingsList_AddItem(hList, "Server Admin Email", sAdminEmail);
		PopWebsiteSettingsList_AddItem(hList, "Root Username",  sUsername);
		PopWebsiteSettingsList_AddItem(hList, "Root Domain",  sDomain);
		//thisList(hList, "Root Password", this->Collection.Password); //Not added to the list, no need to obsfucate!
	}

	LVAutoSizeColumns(hList, 500);
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)TRUE, (LPARAM)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LoadNewWebsiteSettingsConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML, CTabControl *TabControl)
{
	int iGridItems = LVItemCount(hGrid);
	int iLength = 0;

	HWND hTabSecurity = TabControl->Handle("Security");
	HWND hTabAdvanced = TabControl->Handle("Advanced");

	char sText[MAX_TEXT_SIZE];

	pXML->Initialize("WebsiteSettings");

	pXML->AddBool("AllowDirectoryIndexing", IsDlgButtonChecked(hTabAdvanced, IDC_DIRINDEXING) == BST_CHECKED);
	pXML->AddBool("PreventImageLeaching", IsDlgButtonChecked(hTabAdvanced, IDC_PREVENTIMAGELEACHING) == BST_CHECKED);
	pXML->AddBool("CacheLogFile", IsDlgButtonChecked(hTabAdvanced, IDC_CACHELOGFILE) == BST_CHECKED);
	pXML->AddBool("AutoStart", IsDlgButtonChecked(hTabAdvanced, IDC_AUTOSTART) == BST_CHECKED);
	pXML->Add("KeepAliveTimeout", Get_Long(GetDlgItem(hTabAdvanced, IDC_KEEPALIVETIMEOUT)));
	pXML->Add("ScriptTimeout", Get_Long(GetDlgItem(hTabAdvanced, IDC_SCRIPTTIMEOUT)));
	pXML->Add("MaxReceiveSize", Get_Long(GetDlgItem(hTabAdvanced, IDC_MAXRECEIVESIZE)));

	if(IsDlgButtonChecked(hTabSecurity, IDC_NTAUTHENABLED))
	{
		char sUsername[VIRTUALROOTS_MAX_USER_LENGTH];
		char sDomain[VIRTUALROOTS_MAX_DOMAIN_LENGTH];
		char sPassword[VIRTUALROOTS_MAX_PASS_LENGTH];

		if ((iLength = Get_Text(GetDlgItem(hTabSecurity, IDC_NTUSERNAME), sUsername, sizeof(sUsername))) > 0)
		{
			XMLWriter xmlAuth("Login");
			xmlAuth.Add("Username", sUsername);
			Get_Text(GetDlgItem(hTabSecurity, IDC_NTDOMAIN), sDomain, sizeof(sDomain));
			xmlAuth.Add("Domain", sDomain);
			int iPasswordLength = Get_Text(GetDlgItem(hTabSecurity, IDC_NTPASSWORD), sPassword, sizeof(sPassword)); //Not in a listview - will not be obfucated.

			NASCCLStream nasccl;
			char sEncryptionKey[48];
			MakeEncryptionKey(sEncryptionKey, sizeof(sEncryptionKey), "%s\\%s", sDomain, sUsername);
			nasccl.Initialize(sEncryptionKey);
			nasccl.Cipher(sPassword, iPasswordLength);
			nasccl.Destroy();

			char sEncodedPassword[VIRTUALROOTS_MAX_PASS_LENGTH];

			if (Base64Encode((unsigned char *)sPassword, iPasswordLength, (unsigned char *)sEncodedPassword, sizeof(sEncodedPassword)) < 0)
			{
				MessageBox(NULL, "Your password is to long to encrypt.", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}

			xmlAuth.Add("Password", sEncodedPassword);

			pXML->Add(&xmlAuth);
			xmlAuth.Destroy();
		}
	}

	Get_Text(GetDlgItem(hTabAdvanced, IDC_LOGPATH), sText, sizeof(sText));
	pXML->Add("LogPath", sText);

	Get_Text(GetDlgItem(hTabAdvanced, IDC_ADMINEMAIL), sText, sizeof(sText));
	pXML->Add("ServerAdminEmail", sText);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopCompressionList_AddItem(HWND hList, COMPRESSIBLEFILE *obj)
{
	PopCompressionList_AddItem(hList, obj, ListView_GetItemCount(hList));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopCompressionList_AddItem(HWND hList, COMPRESSIBLEFILE *obj, int iInsertionIndex)
{
	LV_ITEM ListItem;
    memset(&ListItem, 0, sizeof(ListItem));
	char sInt[64];

	int iImage = obj->Enabled && obj->ParentEnabled;

    ListItem.pszText = (char *) obj->Extension;
    ListItem.mask = LVIF_TEXT|LVIF_IMAGE;
	ListItem.iImage = iImage;
    ListItem.iSubItem = COMPRESSION_LIST_POS_EXT;
	ListItem.iItem = iInsertionIndex;
    int iItem = ListView_InsertItem(hList, &ListItem);

	if(obj->CompressionLevel >= 0)
	{
		FormatInteger(sInt, sizeof(sInt), obj->CompressionLevel);
	}
	else{
		strcpy_s(sInt, sizeof(sInt), "");
	}
    ListItem.pszText = (char *) sInt;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = COMPRESSION_LIST_POS_LEVEL;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

    ListItem.pszText = (char *) obj->Description;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = COMPRESSION_LIST_POS_DESCR;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListView_SetCheckState(hList, iItem, obj->Enabled);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopCompressionList(HWND hList, bool bUseIcons, XMLReader *pXML)
{
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)FALSE, (LPARAM)0);

    LV_COLUMN lvColumn;
    memset(&lvColumn, 0, sizeof(LV_COLUMN));

	lvColumn.cx        = 50;
    lvColumn.pszText   = "Extension";
    lvColumn.iSubItem  = COMPRESSION_LIST_POS_EXT;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, COMPRESSION_LIST_POS_EXT, &lvColumn);

    lvColumn.cx        = 50;
    lvColumn.pszText   = "Level";
    lvColumn.iSubItem  = COMPRESSION_LIST_POS_LEVEL;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, COMPRESSION_LIST_POS_LEVEL, &lvColumn);

    lvColumn.cx        = 300;
    lvColumn.pszText   = "Description";
    lvColumn.iSubItem  = COMPRESSION_LIST_POS_DESCR;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, COMPRESSION_LIST_POS_DESCR, &lvColumn);

	DWORD dwFlags = LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP;

	if(!bUseIcons)
	{
		dwFlags |= LVS_EX_CHECKBOXES;
	}

    SendMessage(hList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)dwFlags);

	if(bUseIcons)
	{
		ListView_SetImageList(hList, hEnableDisableImageList, LVSIL_SMALL);
	}

	ListView_SetTextColor(hList, RGB(0,0,100));
	
	char sExtension[COMPRESSION_MAX_EXT_LENGTH];
	char sDescription[COMPRESSION_MAX_DESCR_LENGTH];
	bool bEnable = false;
	int iCompressionLevel = 0;

	pXML->ScanPosition(0);
	bool bParentEnabled = pXML->ToBoolean("Enable");
	pXML->ProgressiveScan(true);

	//Pop the list-view.
	for(XMLReader xml; pXML->ToReader("FileType", &xml);)
	{
		//Read XML.
		xml.ToString("Extension", sExtension, sizeof(sExtension));
		xml.ToString("Description", sDescription, sizeof(sDescription));
		iCompressionLevel = xml.ToInteger("CompressionLevel");
		bEnable = xml.ToBoolean("Enable");

		xml.Destroy();

		//Build an object
		COMPRESSIBLEFILE obj;
		obj.Extension = sExtension;
		obj.CompressionLevel = iCompressionLevel;
		obj.Description = sDescription;
		obj.Enabled = bEnable;
		obj.ParentEnabled = bParentEnabled;

		//Add the object to the list.
		PopCompressionList_AddItem(hList, &obj);
	}

	LVAutoSizeColumns(hList, 500);
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)TRUE, (LPARAM)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LoadNewCompressionConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML)
{
	int iGridItems = LVItemCount(hGrid);

	char sExt[COMPRESSION_MAX_EXT_LENGTH];
	char sDescr[COMPRESSION_MAX_DESCR_LENGTH];
	char sPath[MAX_PATH];

	pXML->Initialize("Compression");

	Get_Text(GetDlgItem(hWnd, IDC_CACHEPATH), sPath, sizeof(sPath));
  	pXML->Add("CachePath", sPath);
	pXML->Add("MaxCompressionSize", Get_Long(GetDlgItem(hWnd, IDC_MAXFILESIZE)));
	pXML->Add("MinCompressionSize", Get_Long(GetDlgItem(hWnd, IDC_MINFILESIZE)));
	pXML->Add("MaxCompressionCache", Get_Long(GetDlgItem(hWnd, IDC_CACHESIZE)));
	pXML->Add("CompressionLevel", Get_Long(GetDlgItem(hWnd, IDC_COMPRESSLEVEL)));
	pXML->Add("MemoryLevel", Get_Long(GetDlgItem(hWnd, IDC_MEMORYLEVEL)));
	pXML->AddBool("CompressDynamicContent", IsDlgButtonChecked(hWnd, IDC_COMPRESSDYNAMIC) == BST_CHECKED);
	pXML->AddBool("Enable", IsDlgButtonChecked(hWnd, IDC_ENABLED) == BST_CHECKED);

	for(int iItem = 0; iItem < iGridItems; iItem++)
	{
		XMLWriter Item("FileType");

		LVGetText(hGrid, iItem, COMPRESSION_LIST_POS_EXT, sExt, sizeof(sExt));
		Item.Add("Extension", sExt);

		LVGetText(hGrid, iItem, COMPRESSION_LIST_POS_LEVEL, sDescr, sizeof(sDescr));
		Item.Add("CompressionLevel", iFormattedValue(sDescr));

		LVGetText(hGrid, iItem, COMPRESSION_LIST_POS_DESCR, sDescr, sizeof(sDescr));
		Item.Add("Description", sDescr);

		Item.AddBool("Enable", ParseBoolean(LVChecked(hGrid, iItem)));
		pXML->Add(&Item);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopErrorPagesList_AddItem(HWND hList, ERRORPAGE *obj)
{
	PopErrorPagesList_AddItem(hList, obj, ListView_GetItemCount(hList));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopErrorPagesList_AddItem(HWND hList, ERRORPAGE *obj, int iInsertionIndex)
{
	LV_ITEM ListItem;
    memset(&ListItem, 0, sizeof(ListItem));

	int iImage = obj->Enabled && obj->ParentEnabled;

    ListItem.pszText = (char *) obj->Code;
    ListItem.mask = LVIF_TEXT|LVIF_IMAGE;
	ListItem.iImage = iImage;
    ListItem.iSubItem = ERRORPAGES_LIST_POS_CODE;
	ListItem.iItem = iInsertionIndex;
    int iItem = ListView_InsertItem(hList, &ListItem);

    ListItem.pszText = (char *) obj->ReturnCode;
    ListItem.mask = LVIF_TEXT|LVIF_IMAGE;
	ListItem.iImage = iImage;
    ListItem.iSubItem = ERRORPAGES_LIST_POS_RETURNCODE;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

    ListItem.pszText = (char *) obj->Description;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = ERRORPAGES_LIST_POS_DESCR;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

    ListItem.pszText = (char *) obj->File;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = ERRORPAGES_LIST_POS_FILE;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListView_SetCheckState(hList, iItem, obj->Enabled);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopErrorPagesList(HWND hList, bool bUseIcons, XMLReader *pXML)
{
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)FALSE, (LPARAM)0);

    LV_COLUMN lvColumn;
    memset(&lvColumn, 0, sizeof(LV_COLUMN));

	lvColumn.cx        = 100;
    lvColumn.pszText   = "Code";
    lvColumn.iSubItem  = ERRORPAGES_LIST_POS_CODE;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, ERRORPAGES_LIST_POS_CODE, &lvColumn);

	lvColumn.cx        = 100;
    lvColumn.pszText   = "Return Code";
    lvColumn.iSubItem  = ERRORPAGES_LIST_POS_RETURNCODE;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, ERRORPAGES_LIST_POS_RETURNCODE, &lvColumn);

	lvColumn.cx        = 300;
    lvColumn.pszText   = "Description";
    lvColumn.iSubItem  = ERRORPAGES_LIST_POS_DESCR;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, ERRORPAGES_LIST_POS_DESCR, &lvColumn);

	lvColumn.cx        = 300;
    lvColumn.pszText   = "File";
    lvColumn.iSubItem  = ERRORPAGES_LIST_POS_FILE;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, ERRORPAGES_LIST_POS_FILE, &lvColumn);

	DWORD dwFlags = LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP;

	if(!bUseIcons)
	{
		dwFlags |= LVS_EX_CHECKBOXES;
	}

    SendMessage(hList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)dwFlags);

	if(bUseIcons)
	{
		ListView_SetImageList(hList, hEnableDisableImageList, LVSIL_SMALL);
	}

	ListView_SetTextColor(hList, RGB(0,0,100));
	
	char sCode[ERRORPAGES_MAX_CODE_LENGTH];
	char sReturnCode[ERRORPAGES_MAX_RETURNCODE_LENGTH];
	char sFile[ERRORPAGES_MAX_FILE_LENGTH];
	char sDescription[ERRORPAGES_MAX_DESCR_LENGTH];
	bool bEnable = false;

	pXML->ScanPosition(0);
	bool bParentEnabled = pXML->ToBoolean("Enable");
	pXML->ProgressiveScan(true);

	//Pop the list-view.
	for(XMLReader xml; pXML->ToReader("Page", &xml);)
	{
		//Read XML.
		xml.ToString("Code", sCode, sizeof(sCode));
		xml.ToString("ReturnCode", sReturnCode, sizeof(sReturnCode), sCode);
		xml.ToString("File", sFile, sizeof(sFile));
		xml.ToString("Description", sDescription, sizeof(sDescription));
		bEnable = xml.ToBoolean("Enable");

		xml.Destroy();

		//Build an object
		ERRORPAGE obj;
		obj.Code = sCode;
		obj.ReturnCode = sReturnCode;
		obj.File = sFile;
		obj.Description = sDescription;
		obj.Enabled = bEnable;
		obj.ParentEnabled = bParentEnabled;

		//Add the object to the list.
		PopErrorPagesList_AddItem(hList, &obj);
	}

	LVAutoSizeColumns(hList, 500);
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)TRUE, (LPARAM)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LoadNewErrorPagesConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML)
{
	int iGridItems = LVItemCount(hGrid);

	char sCode[ERRORPAGES_MAX_CODE_LENGTH];
	char sReturnCode[ERRORPAGES_MAX_RETURNCODE_LENGTH];
	char sFile[ERRORPAGES_MAX_FILE_LENGTH];
	char sDescr[ERRORPAGES_MAX_DESCR_LENGTH];

	pXML->Initialize("ErrorPages");

	Get_Text(GetDlgItem(hWnd, IDC_DEFAULTFILE), sFile, sizeof(sFile));
	if(strlen(sCode) == 0)
	{
		strcpy_s(sFile, sizeof(sFile), "GenericError.html");
	}
	pXML->Add("DefaultFile", sFile);

	for(int iItem = 0; iItem < iGridItems; iItem++)
	{
		XMLWriter Item("Page");

		LVGetText(hGrid, iItem, ERRORPAGES_LIST_POS_CODE, sCode, sizeof(sCode));
		Item.Add("Code", sCode);

		LVGetText(hGrid, iItem, ERRORPAGES_LIST_POS_RETURNCODE, sReturnCode, sizeof(sReturnCode));
		Item.Add("ReturnCode", sReturnCode);

		LVGetText(hGrid, iItem, ERRORPAGES_LIST_POS_DESCR, sDescr, sizeof(sDescr));
		Item.Add("Description", sDescr);

		LVGetText(hGrid, iItem, ERRORPAGES_LIST_POS_FILE, sFile, sizeof(sFile));
		Item.Add("File", sFile);

		Item.AddBool("Enable", LVChecked(hGrid, iItem));
		pXML->Add(&Item);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopCGIFoldersList_AddItem(HWND hList, CGIFOLDER *obj)
{
	PopCGIFoldersList_AddItem(hList, obj, ListView_GetItemCount(hList));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopCGIFoldersList_AddItem(HWND hList, CGIFOLDER *obj, int iInsertionIndex)
{
	LV_ITEM ListItem;
    memset(&ListItem, 0, sizeof(ListItem));

	int iImage = obj->Enabled && obj->ParentEnabled;

    ListItem.pszText = (char *) obj->Path;
    ListItem.mask = LVIF_TEXT|LVIF_IMAGE;
	ListItem.iImage = iImage;
    ListItem.iSubItem = CGIFOLDERS_LIST_POS_PATH;
	ListItem.iItem = iInsertionIndex;
    int iItem = ListView_InsertItem(hList, &ListItem);

    ListItem.pszText = (char *) obj->Description;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = CGIFOLDER_LIST_POS_DESCR;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListView_SetCheckState(hList, iItem, obj->Enabled);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopCGIFoldersList(HWND hList, bool bUseIcons, XMLReader *pXML)
{
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)FALSE, (LPARAM)0);

    LV_COLUMN lvColumn;
    memset(&lvColumn, 0, sizeof(LV_COLUMN));

	lvColumn.cx        = 100;
    lvColumn.pszText   = "Path";
    lvColumn.iSubItem  = VIRTUALROOTS_LIST_POS_ROOT;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, CGIFOLDERS_LIST_POS_PATH, &lvColumn);

	lvColumn.cx        = 300;
    lvColumn.pszText   = "Description";
    lvColumn.iSubItem  = VIRTUALROOTS_LIST_POS_DESCR;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, CGIFOLDER_LIST_POS_DESCR, &lvColumn);

	DWORD dwFlags = LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP;

	if(!bUseIcons)
	{
		dwFlags |= LVS_EX_CHECKBOXES;
	}

    SendMessage(hList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)dwFlags);

	if(bUseIcons)
	{
		ListView_SetImageList(hList, hEnableDisableImageList, LVSIL_SMALL);
	}

	ListView_SetTextColor(hList, RGB(0,0,100));
	
	char sPath[CGIFOLDER_MAX_PATH_LENGTH];
	char sDescription[CGIFOLDER_MAX_DESCR_LENGTH];
	bool bEnable = false;

	pXML->ScanPosition(0);
	bool bParentEnabled = pXML->ToBoolean("Enable");
	pXML->ProgressiveScan(true);

	//Pop the list-view.
	for(XMLReader xml; pXML->ToReader("Folder", &xml);)
	{
		//Read XML.
		xml.ToString("Path", sPath, sizeof(sPath));
		xml.ToString("Description", sDescription, sizeof(sDescription));
		bEnable = xml.ToBoolean("Enable");

		xml.Destroy();

		//Build an object
		CGIFOLDER obj;
		obj.Path = sPath;
		obj.Description = sDescription;
		obj.Enabled = bEnable;
		obj.ParentEnabled = bParentEnabled;

		//Add the object to the list.
		PopCGIFoldersList_AddItem(hList, &obj);
	}

	LVAutoSizeColumns(hList, 500);
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)TRUE, (LPARAM)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LoadNewCGIFoldersConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML)
{
	int iGridItems = LVItemCount(hGrid);

	char sPath[CGIFOLDER_MAX_PATH_LENGTH];
	char sDescr[CGIFOLDER_MAX_DESCR_LENGTH];

	pXML->Initialize("CGIFolders");

	pXML->AddBool("Enable", ParseBoolean(IsDlgButtonChecked(hWnd, IDC_ENABLED) == BST_CHECKED));

	for(int iItem = 0; iItem < iGridItems; iItem++)
	{
		XMLWriter Item("Folder");

		LVGetText(hGrid, iItem, CGIFOLDERS_LIST_POS_PATH, sPath, sizeof(sPath));
		Item.Add("Path", sPath);

		LVGetText(hGrid, iItem, CGIFOLDER_LIST_POS_DESCR, sDescr, sizeof(sDescr));
		Item.Add("Description", sDescr);

		Item.AddBool("Enable", ParseBoolean(LVChecked(hGrid, iItem)));
		pXML->Add(&Item);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopSSIFilesList_AddItem(HWND hList, SSIFILE *obj)
{
	PopSSIFilesList_AddItem(hList, obj, ListView_GetItemCount(hList));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopSSIFilesList_AddItem(HWND hList, SSIFILE *obj, int iInsertionIndex)
{
	LV_ITEM ListItem;
    memset(&ListItem, 0, sizeof(ListItem));

	int iImage = obj->Enabled && obj->ParentEnabled;

    ListItem.pszText = (char *) obj->Extension;
    ListItem.mask = LVIF_TEXT|LVIF_IMAGE;
	ListItem.iImage = iImage;
    ListItem.iSubItem = SSIFILES_LIST_POS_EXT;
	ListItem.iItem = iInsertionIndex;
    int iItem = ListView_InsertItem(hList, &ListItem);

    ListItem.pszText = (char *) obj->Description;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = SSIFILES_LIST_POS_DESCR;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListView_SetCheckState(hList, iItem, obj->Enabled);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopSSIFilesList(HWND hList, bool bUseIcons, XMLReader *pXML)
{
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)FALSE, (LPARAM)0);

    LV_COLUMN lvColumn;
    memset(&lvColumn, 0, sizeof(LV_COLUMN));

	lvColumn.cx        = 100;
    lvColumn.pszText   = "Extension";
    lvColumn.iSubItem  = SSIFILES_LIST_POS_EXT;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, SSIFILES_LIST_POS_EXT, &lvColumn);

	lvColumn.cx        = 300;
    lvColumn.pszText   = "Description";
    lvColumn.iSubItem  = SSIFILES_LIST_POS_DESCR;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, SSIFILES_LIST_POS_DESCR, &lvColumn);

	DWORD dwFlags = LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP;

	if(!bUseIcons)
	{
		dwFlags |= LVS_EX_CHECKBOXES;
	}

    SendMessage(hList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)dwFlags);

	if(bUseIcons)
	{
		ListView_SetImageList(hList, hEnableDisableImageList, LVSIL_SMALL);
	}

	ListView_SetTextColor(hList, RGB(0,0,100));
	
	char sExtension[SSIFILES_MAX_EXT_LENGTH];
	char sDescription[SSIFILES_MAX_EXT_LENGTH];
	bool bEnable = false;

	pXML->ScanPosition(0);
	bool bParentEnabled = pXML->ToBoolean("Enable");
	pXML->ProgressiveScan(true);

	//Pop the list-view.
	for(XMLReader xml; pXML->ToReader("File", &xml);)
	{
		//Read XML.
		xml.ToString("Extension", sExtension, sizeof(sExtension));
		xml.ToString("Description", sDescription, sizeof(sDescription));
		bEnable = xml.ToBoolean("Enable");

		xml.Destroy();

		//Build an object
		SSIFILE obj;
		obj.Extension = sExtension;
		obj.Description = sDescription;
		obj.Enabled = bEnable;
		obj.ParentEnabled = bParentEnabled;

		//Add the object to the list.
		PopSSIFilesList_AddItem(hList, &obj);
	}

	LVAutoSizeColumns(hList, 500);
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)TRUE, (LPARAM)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LoadNewSSIFilesConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML)
{
	int iGridItems = LVItemCount(hGrid);

	char sExt[SSIFILES_MAX_EXT_LENGTH];
	char sDescr[SSIFILES_MAX_EXT_LENGTH];

	pXML->Initialize("SSIFiles");

	pXML->AddBool("Enable", IsDlgButtonChecked(hWnd, IDC_ENABLED) == BST_CHECKED);

	for(int iItem = 0; iItem < iGridItems; iItem++)
	{
		XMLWriter Item("File");

		LVGetText(hGrid, iItem, SSIFILES_LIST_POS_EXT, sExt, sizeof(sExt));
		Item.Add("Extension", sExt);

		LVGetText(hGrid, iItem, SSIFILES_LIST_POS_DESCR, sDescr, sizeof(sDescr));
		Item.Add("Description", sDescr);

		Item.AddBool("Enable", LVChecked(hGrid, iItem));
		pXML->Add(&Item);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopScriptingEnginesList_AddItem(HWND hList, SCRIPTENGINE *obj)
{
	PopScriptingEnginesList_AddItem(hList, obj, ListView_GetItemCount(hList));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopScriptingEnginesList_AddItem(HWND hList, SCRIPTENGINE *obj, int iInsertionIndex)
{
	char sNumber[64];
	LV_ITEM ListItem;
    memset(&ListItem, 0, sizeof(ListItem));

	int iImage = obj->Enabled && obj->ParentEnabled;

    ListItem.pszText = (char *) obj->Extension;
    ListItem.mask = LVIF_TEXT|LVIF_IMAGE;
	ListItem.iImage = iImage;
    ListItem.iSubItem = SCRIPTENGINES_LIST_POS_EXT;
	ListItem.iItem = iInsertionIndex;
    int iItem = ListView_InsertItem(hList, &ListItem);

	ListItem.pszText = (char *) obj->Description;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = SCRIPTENGINES_LIST_POS_DESCR;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

    ListItem.pszText = (char *) obj->Parameters;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = SCRIPTENGINES_LIST_POS_PARAMS;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

    ListItem.pszText = (char *) obj->Engine;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = SCRIPTENGINES_LIST_POS_ENGINE;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListItem.pszText = (char *) FormatInteger(sNumber, sizeof(sNumber), obj->SuccessCode);
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = SCRIPTENGINES_LIST_POS_SUCCESSCODE;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListItem.pszText = obj->UseSuccessCode ? "Yes" : "No";
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = SCRIPTENGINES_LIST_POS_USESUCCESSCODE;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListView_SetCheckState(hList, iItem, obj->Enabled);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopScriptingEnginesList(HWND hList, bool bUseIcons, XMLReader *pXML)
{
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)FALSE, (LPARAM)0);

    LV_COLUMN lvColumn;
    memset(&lvColumn, 0, sizeof(LV_COLUMN));

	lvColumn.cx        = 100;
    lvColumn.pszText   = "Extension";
    lvColumn.iSubItem  = SCRIPTENGINES_LIST_POS_EXT;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, SCRIPTENGINES_LIST_POS_EXT, &lvColumn);

	lvColumn.cx        = 300;
    lvColumn.pszText   = "Description";
    lvColumn.iSubItem  = SCRIPTENGINES_LIST_POS_DESCR;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, SCRIPTENGINES_LIST_POS_DESCR, &lvColumn);

    lvColumn.cx        = 300;
    lvColumn.pszText   = "Parameters";
    lvColumn.iSubItem  = SCRIPTENGINES_LIST_POS_PARAMS;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, SCRIPTENGINES_LIST_POS_PARAMS, &lvColumn);

	lvColumn.cx        = 100;
    lvColumn.pszText   = "Engine";
    lvColumn.iSubItem  = SCRIPTENGINES_LIST_POS_ENGINE;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, SCRIPTENGINES_LIST_POS_ENGINE, &lvColumn);

	lvColumn.cx        = 100;
    lvColumn.pszText   = "Success Code";
    lvColumn.iSubItem  = SCRIPTENGINES_LIST_POS_SUCCESSCODE;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, SCRIPTENGINES_LIST_POS_SUCCESSCODE, &lvColumn);

	lvColumn.cx        = 100;
    lvColumn.pszText   = "Use Success Code";
    lvColumn.iSubItem  = SCRIPTENGINES_LIST_POS_USESUCCESSCODE;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, SCRIPTENGINES_LIST_POS_USESUCCESSCODE, &lvColumn);


	DWORD dwFlags = LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP;

	if(!bUseIcons)
	{
		dwFlags |= LVS_EX_CHECKBOXES;
	}

    SendMessage(hList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)dwFlags);

	if(bUseIcons)
	{
		ListView_SetImageList(hList, hEnableDisableImageList, LVSIL_SMALL);
	}

	ListView_SetTextColor(hList, RGB(0,0,100));
	
	char sExtension[SCRIPTINGENGINES_MAX_EXT_LEGTH];
	char sEngine[SCRIPTINGENGINES_MAX_ENGINE_LEGTH];
	char sParams[SCRIPTINGENGINES_MAX_PARAMS_LEGTH];
	char sDescription[SCRIPTINGENGINES_MAX_DESCR_LEGTH];
	bool bEnable = false;
	bool bUseSuccessCode = false;
	int iSuccessCode = 0;

	pXML->ScanPosition(0);
	bool bParentEnabled = pXML->ToBoolean("Enable");
	pXML->ProgressiveScan(true);

	//Pop the list-view.
	for(XMLReader xml; pXML->ToReader("Engine", &xml);)
	{
		//Read XML.
		xml.ToString("Extension", sExtension, sizeof(sExtension));
		xml.ToString("Executable", sEngine, sizeof(sEngine));
		xml.ToString("Parameters", sParams, sizeof(sParams));
		xml.ToString("Description", sDescription, sizeof(sDescription));
		iSuccessCode = xml.ToInteger("SuccessCode", 0);
		bUseSuccessCode = xml.ToBoolean("UseSuccessCode");
		bEnable = xml.ToBoolean("Enable");

		xml.Destroy();

		//Build an object
		SCRIPTENGINE obj;
		obj.Extension = sExtension;
		obj.Engine = sEngine;
		obj.SuccessCode = iSuccessCode;
		obj.UseSuccessCode = bUseSuccessCode;
		obj.Parameters = sParams;
		obj.Description = sDescription;
		obj.Enabled = bEnable;
		obj.ParentEnabled = bParentEnabled;

		//Add the object to the list.
		PopScriptingEnginesList_AddItem(hList, &obj);
	}

	LVAutoSizeColumns(hList, 500);
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)TRUE, (LPARAM)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LoadNewScriptingEnginesConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML)
{
	int iGridItems = LVItemCount(hGrid);

	char sExt[SCRIPTINGENGINES_MAX_EXT_LEGTH];
	char sEngine[SCRIPTINGENGINES_MAX_ENGINE_LEGTH];
	char sParams[SCRIPTINGENGINES_MAX_PARAMS_LEGTH];
	char sDescr[SCRIPTINGENGINES_MAX_DESCR_LEGTH];

	pXML->Initialize("ScriptingEngines");

	pXML->AddBool("Enable", IsDlgButtonChecked(hWnd, IDC_ENABLED) == BST_CHECKED);

	for(int iItem = 0; iItem < iGridItems; iItem++)
	{
		XMLWriter Item("Engine");

		LVGetText(hGrid, iItem, SCRIPTENGINES_LIST_POS_EXT, sExt, sizeof(sExt));
		Item.Add("Extension", sExt);

		LVGetText(hGrid, iItem, SCRIPTENGINES_LIST_POS_ENGINE, sEngine, sizeof(sEngine));
		Item.Add("Executable", sEngine);

		LVGetText(hGrid, iItem, SCRIPTENGINES_LIST_POS_SUCCESSCODE, sDescr, sizeof(sDescr));
		Item.Add("SuccessCode", iFormattedValue(sDescr));

		LVGetText(hGrid, iItem, SCRIPTENGINES_LIST_POS_USESUCCESSCODE, sDescr, sizeof(sDescr));
		Item.Add("UseSuccessCode", ParseBoolean(sDescr));

		LVGetText(hGrid, iItem, SCRIPTENGINES_LIST_POS_PARAMS, sParams, sizeof(sParams));
		Item.Add("Parameters", sParams);

		LVGetText(hGrid, iItem, SCRIPTENGINES_LIST_POS_DESCR, sDescr, sizeof(sDescr));
		Item.Add("Description", sDescr);

		Item.AddBool("Enable", LVChecked(hGrid, iItem));
		pXML->Add(&Item);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopCustomFiltersList_AddItem(HWND hList, CUSTOMFILTER *obj)
{
	PopCustomFiltersList_AddItem(hList, obj, ListView_GetItemCount(hList));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopCustomFiltersList_AddItem(HWND hList, CUSTOMFILTER *obj, int iInsertionIndex)
{
	LV_ITEM ListItem;
    memset(&ListItem, 0, sizeof(ListItem));

	int iImage = obj->Enabled && obj->ParentEnabled;

    ListItem.pszText = (char *) obj->Description;
    ListItem.mask = LVIF_TEXT|LVIF_IMAGE;
	ListItem.iImage = iImage;
    ListItem.iSubItem = CUSTOMFILTERS_LIST_POS_DESCR;
	ListItem.iItem = iInsertionIndex;
    int iItem = ListView_InsertItem(hList, &ListItem);

    ListItem.pszText = (char *) obj->DLL;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = CUSTOMFILTERS_LIST_POS_DLL;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListView_SetCheckState(hList, iItem, obj->Enabled);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopCustomFiltersList(HWND hList, bool bUseIcons, XMLReader *pXML)
{
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)FALSE, (LPARAM)0);

    LV_COLUMN lvColumn;
    memset(&lvColumn, 0, sizeof(LV_COLUMN));

	lvColumn.cx        = 300;
    lvColumn.pszText   = "Description";
    lvColumn.iSubItem  = CUSTOMFILTERS_LIST_POS_DESCR;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, CUSTOMFILTERS_LIST_POS_DESCR, &lvColumn);

	lvColumn.cx        = 100;
    lvColumn.pszText   = "DLL";
    lvColumn.iSubItem  = CUSTOMFILTERS_LIST_POS_DLL;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, CUSTOMFILTERS_LIST_POS_DLL, &lvColumn);

	DWORD dwFlags = LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP;

	if(!bUseIcons)
	{
		dwFlags |= LVS_EX_CHECKBOXES;
	}

    SendMessage(hList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)dwFlags);

	if(bUseIcons)
	{
		ListView_SetImageList(hList, hEnableDisableImageList, LVSIL_SMALL);
	}

	ListView_SetTextColor(hList, RGB(0,0,100));
	
	char sDLL[CUSTOMFILTERS_MAX_DLL_LENGTH];
	char sDescription[CUSTOMFILTERS_MAX_DESCR_LENGTH];
	bool bEnable = false;

	pXML->ScanPosition(0);
	bool bParentEnabled = pXML->ToBoolean("Enable");
	pXML->ProgressiveScan(true);

	//Pop the list-view.
	for(XMLReader xml; pXML->ToReader("Filter", &xml);)
	{
		//Read XML.
		xml.ToString("DLL", sDLL, sizeof(sDLL));
		xml.ToString("Description", sDescription, sizeof(sDescription));
		bEnable = xml.ToBoolean("Enable");

		xml.Destroy();

		//Build an object
		CUSTOMFILTER obj;
		obj.DLL = sDLL;
		obj.Description = sDescription;
		obj.Enabled = bEnable;
		obj.ParentEnabled = bParentEnabled;

		//Add the object to the list.
		PopCustomFiltersList_AddItem(hList, &obj);
	}

	LVAutoSizeColumns(hList, 500);
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)TRUE, (LPARAM)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LoadNewCustomFiltersConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML)
{
	int iGridItems = LVItemCount(hGrid);

	char sDLL[CUSTOMFILTERS_MAX_DLL_LENGTH];
	char sDescr[CUSTOMFILTERS_MAX_DESCR_LENGTH];

	pXML->Initialize("CustomFilters");

	pXML->AddBool("Enable", ParseBoolean(IsDlgButtonChecked(hWnd, IDC_ENABLED) == BST_CHECKED));

	for(int iItem = 0; iItem < iGridItems; iItem++)
	{
		XMLWriter Item("Filter");

		LVGetText(hGrid, iItem, CUSTOMFILTERS_LIST_POS_DLL, sDLL, sizeof(sDLL));
		Item.Add("DLL", sDLL);

		LVGetText(hGrid, iItem, CUSTOMFILTERS_LIST_POS_DESCR, sDescr, sizeof(sDescr));
		Item.Add("Description", sDescr);

		Item.AddBool("Enable", ParseBoolean(LVChecked(hGrid, iItem)));
		pXML->Add(&Item);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopExtensionFiltersList_AddItem(HWND hList, EXTENSIONFILTER *obj)
{
	PopExtensionFiltersList_AddItem(hList, obj, ListView_GetItemCount(hList));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopExtensionFiltersList_AddItem(HWND hList, EXTENSIONFILTER *obj, int iInsertionIndex)
{
	LV_ITEM ListItem;
    memset(&ListItem, 0, sizeof(ListItem));

	int iImage = obj->Enabled && obj->ParentEnabled;

    ListItem.pszText = (char *) obj->Extension;
    ListItem.mask = LVIF_TEXT|LVIF_IMAGE;
	ListItem.iImage = iImage;
    ListItem.iSubItem = EXTENSIONFILTERS_LIST_POS_EXT;
	ListItem.iItem = iInsertionIndex;
    int iItem = ListView_InsertItem(hList, &ListItem);

    ListItem.pszText = (char *) obj->Description;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = EXTENSIONFILTER_LIST_POS_DESCR;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListView_SetCheckState(hList, iItem, obj->Enabled);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopExtensionFiltersList(HWND hList, bool bUseIcons, XMLReader *pXML)
{
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)FALSE, (LPARAM)0);

    LV_COLUMN lvColumn;
    memset(&lvColumn, 0, sizeof(LV_COLUMN));

	lvColumn.cx        = 100;
    lvColumn.pszText   = "Extension";
    lvColumn.iSubItem  = EXTENSIONFILTERS_LIST_POS_EXT;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, EXTENSIONFILTERS_LIST_POS_EXT, &lvColumn);

	lvColumn.cx        = 300;
    lvColumn.pszText   = "Description";
    lvColumn.iSubItem  = EXTENSIONFILTER_LIST_POS_DESCR;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, EXTENSIONFILTER_LIST_POS_DESCR, &lvColumn);

	DWORD dwFlags = LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP;

	if(!bUseIcons)
	{
		dwFlags |= LVS_EX_CHECKBOXES;
	}

    SendMessage(hList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)dwFlags);

	if(bUseIcons)
	{
		ListView_SetImageList(hList, hEnableDisableImageList, LVSIL_SMALL);
	}

	ListView_SetTextColor(hList, RGB(0,0,100));
	
	char sExtension[EXTFILTERS_MAX_EXT_LENGTH];
	char sDescription[EXTFILTERS_MAX_DESCR_LENGTH];
	bool bEnable = false;

	pXML->ScanPosition(0);
	bool bParentEnabled = pXML->ToBoolean("Enable");
	pXML->ProgressiveScan(true);

	//Pop the list-view.
	for(XMLReader xml; pXML->ToReader("Filter", &xml);)
	{
		//Read XML.
		xml.ToString("Extension", sExtension, sizeof(sExtension));
		xml.ToString("Description", sDescription, sizeof(sDescription));
		bEnable = xml.ToBoolean("Enable");

		xml.Destroy();

		//Build an object
		EXTENSIONFILTER obj;
		obj.Extension = sExtension;
		obj.Description = sDescription;
		obj.Enabled = bEnable;
		obj.ParentEnabled = bParentEnabled;

		//Add the object to the list.
		PopExtensionFiltersList_AddItem(hList, &obj);
	}

	LVAutoSizeColumns(hList, 500);
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)TRUE, (LPARAM)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LoadNewExtensionFiltersConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML)
{
	int iGridItems = LVItemCount(hGrid);

	char sExt[EXTFILTERS_MAX_EXT_LENGTH];
	char sDescr[EXTFILTERS_MAX_DESCR_LENGTH];

	pXML->Initialize("ExtensionFilters");

	pXML->AddBool("Enable", IsDlgButtonChecked(hWnd, IDC_ENABLED) == BST_CHECKED);

	for(int iItem = 0; iItem < iGridItems; iItem++)
	{
		XMLWriter Item("Filter");

		LVGetText(hGrid, iItem, EXTENSIONFILTERS_LIST_POS_EXT, sExt, sizeof(sExt));
		Item.Add("Extension", sExt);

		LVGetText(hGrid, iItem, EXTENSIONFILTER_LIST_POS_DESCR, sDescr, sizeof(sDescr));
		Item.Add("Description", sDescr);

		Item.AddBool("Enable", LVChecked(hGrid, iItem));
		pXML->Add(&Item);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopIPFiltersList_AddItem(HWND hList, IPFILTER *obj)
{
	PopIPFiltersList_AddItem(hList, obj, ListView_GetItemCount(hList));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopIPFiltersList_AddItem(HWND hList, IPFILTER *obj, int iInsertionIndex)
{
	LV_ITEM ListItem;
    memset(&ListItem, 0, sizeof(ListItem));

	int iImage = obj->Enabled && obj->ParentEnabled;

	ListItem.pszText = (char *) obj->IP;
    ListItem.mask = LVIF_TEXT|LVIF_IMAGE;
	ListItem.iImage = iImage;
    ListItem.iSubItem = IPFILTERS_LIST_POS_IP;
	ListItem.iItem = iInsertionIndex;
    int iItem = ListView_InsertItem(hList, &ListItem);

    ListItem.pszText = (char *) obj->Description;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = IPFILTERS_LIST_POS_DESCR;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListView_SetCheckState(hList, iItem, obj->Enabled);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopIPFiltersList(HWND hList, bool bUseIcons, XMLReader *pXML)
{
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)FALSE, (LPARAM)0);

    LV_COLUMN lvColumn;
    memset(&lvColumn, 0, sizeof(LV_COLUMN));

	lvColumn.cx        = 100;
    lvColumn.pszText   = "IP Address";
    lvColumn.iSubItem  = IPFILTERS_LIST_POS_IP;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, IPFILTERS_LIST_POS_IP, &lvColumn);

	lvColumn.cx        = 300;
    lvColumn.pszText   = "Description";
    lvColumn.iSubItem  = IPFILTERS_LIST_POS_DESCR;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, IPFILTERS_LIST_POS_DESCR, &lvColumn);

	DWORD dwFlags = LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP;

	if(!bUseIcons)
	{
		dwFlags |= LVS_EX_CHECKBOXES;
	}

    SendMessage(hList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)dwFlags);

	if(bUseIcons)
	{
		ListView_SetImageList(hList, hEnableDisableImageList, LVSIL_SMALL);
	}

	ListView_SetTextColor(hList, RGB(0,0,100));
	
	char sIP[IPFILTERS_MAX_IP_LENGTH];
	char sDescription[IPFILTERS_MAX_DESCR_LENGTH];
	bool bEnable = false;

	pXML->ScanPosition(0);
	bool bParentEnabled = pXML->ToBoolean("Enable");
	pXML->ProgressiveScan(true);

	//Pop the list-view.
	for(XMLReader xml; pXML->ToReader("Filter", &xml);)
	{
		//Read XML.
		xml.ToString("Address", sIP, sizeof(sIP));
		xml.ToString("Description", sDescription, sizeof(sDescription));
		bEnable = xml.ToBoolean("Enable");

		xml.Destroy();

		//Build an object
		IPFILTER obj;
		obj.IP = sIP;
		obj.Description = sDescription;
		obj.Enabled = bEnable;
		obj.ParentEnabled = bParentEnabled;

		//Add the object to the list.
		PopIPFiltersList_AddItem(hList, &obj);
	}

	LVAutoSizeColumns(hList, 500);
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)TRUE, (LPARAM)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LoadNewIPFiltersConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML)
{
	int iGridItems = LVItemCount(hGrid);

	char sIP[IPFILTERS_MAX_IP_LENGTH];
	char sDescr[IPFILTERS_MAX_DESCR_LENGTH];

	pXML->Initialize("IPFilters");

	pXML->AddBool("Enable", IsDlgButtonChecked(hWnd, IDC_ENABLED) == BST_CHECKED);

	for(int iItem = 0; iItem < iGridItems; iItem++)
	{
		XMLWriter Item("Filter");

		LVGetText(hGrid, iItem, IPFILTERS_LIST_POS_IP, sIP, sizeof(sIP));
		Item.Add("Address", sIP);

		LVGetText(hGrid, iItem, IPFILTERS_LIST_POS_DESCR, sDescr, sizeof(sDescr));
		Item.Add("Description", sDescr);

		Item.AddBool("Enable", LVChecked(hGrid, iItem));
		pXML->Add(&Item);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopURLFiltersList_AddItem(HWND hList, URLFILTER *obj)
{
	PopURLFiltersList_AddItem(hList, obj, ListView_GetItemCount(hList));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopURLFiltersList_AddItem(HWND hList, URLFILTER *obj, int iInsertionIndex)
{
	char sTemp[32];
	LV_ITEM ListItem;
    memset(&ListItem, 0, sizeof(ListItem));

	int iImage = obj->Enabled && obj->ParentEnabled;

    ListItem.pszText = (char *) obj->Sequence;
    ListItem.mask = LVIF_TEXT|LVIF_IMAGE;
	ListItem.iImage = iImage;
    ListItem.iSubItem = URLFILTERS_LIST_POS_SEQ;
	ListItem.iItem = iInsertionIndex;
    int iItem = ListView_InsertItem(hList, &ListItem);

	sprintf_s(sTemp, sizeof(sTemp), "%s", obj->ScanRequest ? "Yes" : "No");
    ListItem.pszText = (char *) sTemp;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = URLFILTERS_LIST_POS_SCANREQUEST;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	sprintf_s(sTemp, sizeof(sTemp), "%s", obj->ScanQuery ? "Yes" : "No");
    ListItem.pszText = (char *) sTemp;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = URLFILTERS_LIST_POS_SCANQUERY;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

    ListItem.pszText = (char *) obj->Description;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = URLFILTERS_LIST_POS_DESCR;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListView_SetCheckState(hList, iItem, obj->Enabled);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopURLFiltersList(HWND hList, bool bUseIcons, XMLReader *pXML)
{
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)FALSE, (LPARAM)0);

    LV_COLUMN lvColumn;
    memset(&lvColumn, 0, sizeof(LV_COLUMN));

	lvColumn.cx        = 100;
    lvColumn.pszText   = "Sequence";
    lvColumn.iSubItem  = URLFILTERS_LIST_POS_SEQ;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, URLFILTERS_LIST_POS_SEQ, &lvColumn);

	lvColumn.cx        = 300;
    lvColumn.pszText   = "Scan Request";
    lvColumn.iSubItem  = URLFILTERS_LIST_POS_SCANREQUEST;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, URLFILTERS_LIST_POS_SCANREQUEST, &lvColumn);

	lvColumn.cx        = 300;
    lvColumn.pszText   = "Scan Query";
    lvColumn.iSubItem  = URLFILTERS_LIST_POS_SCANQUERY;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, URLFILTERS_LIST_POS_SCANQUERY, &lvColumn);

	lvColumn.cx        = 300;
    lvColumn.pszText   = "Description";
    lvColumn.iSubItem  = URLFILTERS_LIST_POS_DESCR;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, URLFILTERS_LIST_POS_DESCR, &lvColumn);

	DWORD dwFlags = LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP;

	if(!bUseIcons)
	{
		dwFlags |= LVS_EX_CHECKBOXES;
	}

    SendMessage(hList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)dwFlags);

	if(bUseIcons)
	{
		ListView_SetImageList(hList, hEnableDisableImageList, LVSIL_SMALL);
	}

	ListView_SetTextColor(hList, RGB(0,0,100));
	
	char sSequence[URLFILTERS_MAX_SEQ_LENGTH];
	char sDescription[URLFILTERS_MAX_DESCR_LENGTH];
	bool bEnable = false;
	bool bScanRequest = false;
	bool bScanQuery = false;

	pXML->ScanPosition(0);
	bool bParentEnabled = pXML->ToBoolean("Enable");
	pXML->ProgressiveScan(true);

	//Pop the list-view.
	for(XMLReader xml; pXML->ToReader("Filter", &xml);)
	{
		//Read XML.
		xml.ToString("Sequence", sSequence, sizeof(sSequence));
		xml.ToString("Description", sDescription, sizeof(sDescription));
		bScanRequest = xml.ToBoolean("ScanRequest");
		bScanQuery = xml.ToBoolean("ScanQuery");
		bEnable = xml.ToBoolean("Enable");

		xml.Destroy();

		//Build an object
		URLFILTER obj;
		obj.Sequence = sSequence;
		obj.Description = sDescription;
		obj.ScanQuery = bScanQuery;
		obj.ScanRequest = bScanRequest;
		obj.Enabled = bEnable;
		obj.ParentEnabled = bParentEnabled;

		//Add the object to the list.
		PopURLFiltersList_AddItem(hList, &obj);
	}

	LVAutoSizeColumns(hList, 500);
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)TRUE, (LPARAM)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LoadNewURLFiltersConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML)
{
	int iGridItems = LVItemCount(hGrid);

	char sSequence[URLFILTERS_MAX_SEQ_LENGTH];
	char sDescr[URLFILTERS_MAX_DESCR_LENGTH];

	pXML->Initialize("URLFilters");

	pXML->AddBool("Enable", IsDlgButtonChecked(hWnd, IDC_ENABLED) == BST_CHECKED);

	for(int iItem = 0; iItem < iGridItems; iItem++)
	{
		XMLWriter Item("Filter");

		LVGetText(hGrid, iItem, URLFILTERS_LIST_POS_SEQ, sSequence, sizeof(sSequence));
		Item.Add("Sequence", sSequence);

		LVGetText(hGrid, iItem, URLFILTERS_LIST_POS_DESCR, sDescr, sizeof(sDescr));
		Item.Add("Description", sDescr);

		LVGetText(hGrid, iItem, URLFILTERS_LIST_POS_SCANREQUEST, sDescr, sizeof(sDescr));
		Item.AddBool("ScanRequest", ParseBoolean(sDescr));

		LVGetText(hGrid, iItem, URLFILTERS_LIST_POS_SCANQUERY, sDescr, sizeof(sDescr));
		Item.AddBool("ScanQuery", ParseBoolean(sDescr));

		Item.AddBool("Enable", LVChecked(hGrid, iItem));
		pXML->Add(&Item);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopMimeTypesList_AddItem(HWND hList, MIMETYPE *obj)
{
	PopMimeTypesList_AddItem(hList, obj, ListView_GetItemCount(hList));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopMimeTypesList_AddItem(HWND hList, MIMETYPE *obj, int iInsertionIndex)
{
	LV_ITEM ListItem;
    memset(&ListItem, 0, sizeof(ListItem));

	int iImage = obj->Enabled && obj->ParentEnabled;

	ListItem.pszText = (char *) obj->Extension;
    ListItem.mask = LVIF_TEXT|LVIF_IMAGE;
	ListItem.iImage = iImage;
    ListItem.iSubItem = MIMETYPES_LIST_POS_EXTENSION;
	ListItem.iItem = iInsertionIndex;
    int iItem = ListView_InsertItem(hList, &ListItem);

    ListItem.pszText = (char *) obj->Type;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = MIMETYPES_LIST_POS_TYPE;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

    ListItem.pszText = (char *) obj->Description;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = MIMETYPES_LIST_POS_DESCR;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListView_SetCheckState(hList, iItem, obj->Enabled);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopMimeTypesList(HWND hList, bool bUseIcons, XMLReader *pXML)
{
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)FALSE, (LPARAM)0);

    LV_COLUMN lvColumn;
    memset(&lvColumn, 0, sizeof(LV_COLUMN));

	lvColumn.cx        = 100;
    lvColumn.pszText   = "Extension";
    lvColumn.iSubItem  = MIMETYPES_LIST_POS_EXTENSION;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, MIMETYPES_LIST_POS_EXTENSION, &lvColumn);

	lvColumn.cx        = 300;
    lvColumn.pszText   = "Mime Type";
    lvColumn.iSubItem  = MIMETYPES_LIST_POS_TYPE;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, MIMETYPES_LIST_POS_TYPE, &lvColumn);

	lvColumn.cx        = 300;
    lvColumn.pszText   = "Description";
    lvColumn.iSubItem  = MIMETYPES_LIST_POS_DESCR;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, MIMETYPES_LIST_POS_DESCR, &lvColumn);

	DWORD dwFlags = LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP;

	if(!bUseIcons)
	{
		dwFlags |= LVS_EX_CHECKBOXES;
	}

    SendMessage(hList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)dwFlags);

	if(bUseIcons)
	{
		ListView_SetImageList(hList, hEnableDisableImageList, LVSIL_SMALL);
	}

	ListView_SetTextColor(hList, RGB(0,0,100));
	
	char sExtension[MIMETYPES_MAX_EXT_LENGTH];
	char sType[MIMETYPES_MAX_TYPE_LENGTH];
	char sDescription[MIMETYPES_MAX_DESCR_LENGTH];
	bool bEnable = false;

	pXML->ScanPosition(0);
	bool bParentEnabled = pXML->ToBoolean("Enable");
	pXML->ProgressiveScan(true);

	//Pop the list-view.
	for(XMLReader xml; pXML->ToReader("Type", &xml);)
	{
		//Read XML.
		xml.ToString("Extension", sExtension, sizeof(sExtension));
		xml.ToString("Mime", sType, sizeof(sType));
		xml.ToString("Description", sDescription, sizeof(sDescription));
		bEnable = xml.ToBoolean("Enable");

		xml.Destroy();

		//Build an object
		MIMETYPE obj;
		obj.Extension = sExtension;
		obj.Type = sType;
		obj.Description = sDescription;
		obj.Enabled = bEnable;
		obj.ParentEnabled = bParentEnabled;

		//Add the object to the list.
		PopMimeTypesList_AddItem(hList, &obj);
	}

	LVAutoSizeColumns(hList, 500);
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)TRUE, (LPARAM)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LoadNewMimeTypesConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML)
{
	int iGridItems = LVItemCount(hGrid);

	char sExt[MIMETYPES_MAX_EXT_LENGTH];
	char sType[MIMETYPES_MAX_TYPE_LENGTH];
	char sDescr[MIMETYPES_MAX_DESCR_LENGTH];

	pXML->Initialize("MimeTypes");

	pXML->AddBool("Enable", IsDlgButtonChecked(hWnd, IDC_ENABLED) == BST_CHECKED);

	for(int iItem = 0; iItem < iGridItems; iItem++)
	{
		XMLWriter Item("Type");

		LVGetText(hGrid, iItem, MIMETYPES_LIST_POS_EXTENSION, sExt, sizeof(sExt));
		Item.Add("Extension", sExt);

		LVGetText(hGrid, iItem, MIMETYPES_LIST_POS_TYPE, sType, sizeof(sType));
		Item.Add("Mime", sType);

		LVGetText(hGrid, iItem, MIMETYPES_LIST_POS_DESCR, sDescr, sizeof(sDescr));
		Item.Add("Description", sDescr);

		Item.AddBool("Enable", LVChecked(hGrid, iItem));
		pXML->Add(&Item);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopVirtualRootsList_AddItem(HWND hList, ROOT *obj, bool bSaveCopyOfPlainTextPassword)
{
	PopVirtualRootsList_AddItem(hList, obj, bSaveCopyOfPlainTextPassword, ListView_GetItemCount(hList));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopVirtualRootsList_AddItem(HWND hList, ROOT *obj, bool bSaveCopyOfPlainTextPassword, int iInsertionIndex)
{
	LV_ITEM ListItem;
    memset(&ListItem, 0, sizeof(ListItem));

	int iImage = obj->Enabled && obj->ParentEnabled;

	size_t iPasswordLength = strlen(obj->Password);
	if(bSaveCopyOfPlainTextPassword)
	{
		ListItem.lParam = (LPARAM) pMem->Allocate(iPasswordLength + 1, sizeof(char));
		strcpy_s((char *)ListItem.lParam, iPasswordLength + 1, obj->Password); //Allocate RAM to LPARAM. MUST CLEAN UP!!
	}

	ListItem.pszText = (char *) obj->Root;
    ListItem.mask = LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
	ListItem.iImage = iImage;
    ListItem.iSubItem = VIRTUALROOTS_LIST_POS_ROOT;
	ListItem.iItem = iInsertionIndex;
	int iItem = ListView_InsertItem(hList, &ListItem);

    ListItem.pszText = (char *) obj->Path;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = VIRTUALROOTS_LIST_POS_PATH;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

    ListItem.pszText = (char *) obj->Description;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = VIRTUALROOTS_LIST_POS_DESCR;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	if(obj->Username)
	{
		ListItem.pszText = (char *) obj->Username;
		ListItem.mask = LVIF_TEXT;
		ListItem.iSubItem = VIRTUALROOTS_LIST_POS_USER;
		ListItem.iItem = iInsertionIndex;
		ListView_SetItem(hList, &ListItem);

		ListItem.pszText = (char *) obj->Domain;
		ListItem.mask = LVIF_TEXT;
		ListItem.iSubItem = VIRTUALROOTS_LIST_POS_DOMAIN;
		ListItem.iItem = iInsertionIndex;
		ListView_SetItem(hList, &ListItem);

		int iPwdLength = (int)strlen(obj->Password);
		char *sStars = Space(iPwdLength);
		ReplaceCharacter(sStars, ' ', '*');
		
		ListItem.pszText = (char *) sStars; //obj->Password;
		ListItem.mask = LVIF_TEXT;
		ListItem.iSubItem = VIRTUALROOTS_LIST_POS_PASS;
		ListItem.iItem = iInsertionIndex;
		ListView_SetItem(hList, &ListItem);

		free(sStars);
	}

	ListView_SetCheckState(hList, iItem, obj->Enabled);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopVirtualRootsList(HWND hList, bool bUseIcons, XMLReader *pXML)
{
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)FALSE, (LPARAM)0);

    LV_COLUMN lvColumn;
    memset(&lvColumn, 0, sizeof(LV_COLUMN));

	lvColumn.cx        = 100;
    lvColumn.pszText   = "Root";
    lvColumn.iSubItem  = VIRTUALROOTS_LIST_POS_ROOT;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, VIRTUALROOTS_LIST_POS_ROOT, &lvColumn);

	lvColumn.cx        = 300;
    lvColumn.pszText   = "Path";
    lvColumn.iSubItem  = VIRTUALROOTS_LIST_POS_PATH;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, VIRTUALROOTS_LIST_POS_PATH, &lvColumn);

	lvColumn.cx        = 300;
    lvColumn.pszText   = "Description";
    lvColumn.iSubItem  = VIRTUALROOTS_LIST_POS_DESCR;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, VIRTUALROOTS_LIST_POS_DESCR, &lvColumn);

	lvColumn.cx        = 300;
    lvColumn.pszText   = "Username";
    lvColumn.iSubItem  = VIRTUALROOTS_LIST_POS_USER;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, VIRTUALROOTS_LIST_POS_USER, &lvColumn);

	lvColumn.cx        = 300;
    lvColumn.pszText   = "Domain";
    lvColumn.iSubItem  = VIRTUALROOTS_LIST_POS_DOMAIN;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, VIRTUALROOTS_LIST_POS_DOMAIN, &lvColumn);

	lvColumn.cx        = 300;
    lvColumn.pszText   = "Password";
    lvColumn.iSubItem  = VIRTUALROOTS_LIST_POS_PASS;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, VIRTUALROOTS_LIST_POS_PASS, &lvColumn);

	DWORD dwFlags = LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP;

	if(!bUseIcons)
	{
		dwFlags |= LVS_EX_CHECKBOXES;
	}

    SendMessage(hList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)dwFlags);

	if(bUseIcons)
	{
		ListView_SetImageList(hList, hEnableDisableImageList, LVSIL_SMALL);
	}

	ListView_SetTextColor(hList, RGB(0,0,100));
	
	char sURI[VIRTUALROOTS_MAX_ROOT_LENGTH];
	char sPath[VIRTUALROOTS_MAX_PATH_LENGTH];
	char sDescription[VIRTUALROOTS_MAX_DESCR_LENGTH];
	char sUsername[VIRTUALROOTS_MAX_USER_LENGTH];
	char sDomain[VIRTUALROOTS_MAX_DOMAIN_LENGTH];
	char sPassword[VIRTUALROOTS_MAX_PASS_LENGTH];
	bool bEnable = false;

	pXML->ScanPosition(0);
	bool bParentEnabled = pXML->ToBoolean("Enable");
	pXML->ProgressiveScan(true);

	//Pop the list-view.
	for(XMLReader xml; pXML->ToReader("Root", &xml);)
	{
		//Read XML.
		xml.ToString("URI", sURI, sizeof(sURI));
		xml.ToString("Path", sPath, sizeof(sPath));
		xml.ToString("Description", sDescription, sizeof(sDescription));
		bEnable = xml.ToBoolean("Enable");

		XMLReader xmlLogin;
		if(xml.ToReader("Login", &xmlLogin))
		{
			xmlLogin.ToString("Username", sUsername, sizeof(sUsername));
			xmlLogin.ToString("Domain", sDomain, sizeof(sDomain));
			xmlLogin.ToString("Password", sPassword, sizeof(sPassword));
			xmlLogin.Destroy();
		}
		else{
			memset(sUsername, 0, sizeof(sUsername));
			memset(sDomain, 0, sizeof(sDomain));
			memset(sPassword, 0, sizeof(sPassword));
		}

		xml.Destroy();

		//Build an object
		ROOT obj;
		obj.Root = sURI;
		obj.Path = sPath;
		obj.Description = sDescription;
		obj.Username = sUsername;
		obj.Domain = sDomain;
		obj.Password = sPassword;
		obj.Enabled = bEnable;
		obj.ParentEnabled = bParentEnabled;

		//Add the object to the list.
		PopVirtualRootsList_AddItem(hList, &obj, !bUseIcons);
	}

	LVAutoSizeColumns(hList, 500);
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)TRUE, (LPARAM)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LoadNewVirtualRootsConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML)
{
	int iGridItems = LVItemCount(hGrid);

	char sURI[VIRTUALROOTS_MAX_ROOT_LENGTH];
	char sPath[VIRTUALROOTS_MAX_PATH_LENGTH];
	char sDescr[VIRTUALROOTS_MAX_DESCR_LENGTH];
	char sUsername[VIRTUALROOTS_MAX_USER_LENGTH];
	char sDomain[VIRTUALROOTS_MAX_DOMAIN_LENGTH];
	char sPassword[VIRTUALROOTS_MAX_PASS_LENGTH];

	pXML->Initialize("VirtualRoots");

	pXML->AddBool("Enable", IsDlgButtonChecked(hWnd, IDC_ENABLED) == BST_CHECKED);

	for(int iItem = 0; iItem < iGridItems; iItem++)
	{
		XMLWriter Item("Root");

		LVGetText(hGrid, iItem, VIRTUALROOTS_LIST_POS_ROOT, sURI, sizeof(sURI));
		Item.Add("URI", sURI);

		LVGetText(hGrid, iItem, VIRTUALROOTS_LIST_POS_PATH, sPath, sizeof(sPath));
		Item.Add("Path", sPath);

		LVGetText(hGrid, iItem, VIRTUALROOTS_LIST_POS_DESCR, sDescr, sizeof(sDescr));
		Item.Add("Description", sDescr);

		Item.AddBool("Enable", LVChecked(hGrid, iItem));

		LVGetText(hGrid, iItem, VIRTUALROOTS_LIST_POS_USER, sUsername, sizeof(sUsername));
		LVGetText(hGrid, iItem, VIRTUALROOTS_LIST_POS_DOMAIN, sDomain, sizeof(sDomain));

		LPARAM lAddress = LVGetItemParam(hGrid, iItem, 0);
		strcpy_s(sPassword, sizeof(sPassword), (char *)lAddress);

		int iPasswordLength = (int)strlen(sPassword);

		NASCCLStream nasccl;
		char sEncryptionKey[48];
		MakeEncryptionKey(sEncryptionKey, sizeof(sEncryptionKey), "%s\\%s", sDomain, sUsername);
		nasccl.Initialize(sEncryptionKey);
		nasccl.Cipher(sPassword, iPasswordLength);
		nasccl.Destroy();

		char sEncodedPassword[VIRTUALROOTS_MAX_PASS_LENGTH];
		if (Base64Encode((unsigned char *)sPassword, iPasswordLength, (unsigned char *)sEncodedPassword, sizeof(sEncodedPassword)) < 0)
		{
			MessageBox(NULL, "Your password is to long to encrypt.", gsTitleCaption, MB_ICONINFORMATION);
			return FALSE;
		}

		if(strlen(sUsername) > 0)
		{
			XMLWriter xmlAuth("Login");
			xmlAuth.Add("Username", sUsername);
			xmlAuth.Add("Domain", sDomain);
			xmlAuth.Add("Password", sEncodedPassword);
			Item.Add(&xmlAuth);
			xmlAuth.Destroy();
		}

		pXML->Add(&Item);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopDefaultPagesList_AddItem(HWND hList, DEFAULTPAGE *obj)
{
	PopDefaultPagesList_AddItem(hList, obj, ListView_GetItemCount(hList));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopDefaultPagesList_AddItem(HWND hList, DEFAULTPAGE *obj, int iInsertionIndex)
{
	LV_ITEM ListItem;
    memset(&ListItem, 0, sizeof(ListItem));

	int iImage = obj->Enabled && obj->ParentEnabled;

    ListItem.pszText = (char *) obj->Page;
    ListItem.mask = LVIF_TEXT|LVIF_IMAGE;
	ListItem.iImage = iImage;
    ListItem.iSubItem = DEFAULTPAGES_LIST_POS_PAGE;
	ListItem.iItem = iInsertionIndex;
    int iItem = ListView_InsertItem(hList, &ListItem);

    ListItem.pszText = (char *) obj->Description;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = DEFAULTPAGES_LIST_POS_DESCR;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListView_SetCheckState(hList, iItem, obj->Enabled);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopDefaultPagesList(HWND hList, bool bUseIcons, XMLReader *pXML)
{
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)FALSE, (LPARAM)0);

    LV_COLUMN lvColumn;
    memset(&lvColumn, 0, sizeof(LV_COLUMN));

	lvColumn.cx        = 100;
    lvColumn.pszText   = "Page";
    lvColumn.iSubItem  = DEFAULTPAGES_LIST_POS_PAGE;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, DEFAULTPAGES_LIST_POS_PAGE, &lvColumn);

	lvColumn.cx        = 300;
    lvColumn.pszText   = "Description";
    lvColumn.iSubItem  = DEFAULTPAGES_LIST_POS_DESCR;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, DEFAULTPAGES_LIST_POS_DESCR, &lvColumn);

	DWORD dwFlags = LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP;

	if(!bUseIcons)
	{
		dwFlags |= LVS_EX_CHECKBOXES;
	}

    SendMessage(hList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)dwFlags);

	if(bUseIcons)
	{
		ListView_SetImageList(hList, hEnableDisableImageList, LVSIL_SMALL);
	}

	ListView_SetTextColor(hList, RGB(0,0,100));

	char sFile[DEFAULTPAGES_MAX_PAGE_LENGTH];
	char sDescription[DEFAULTPAGES_MAX_DESCR_LENGTH];
	bool bEnable = false;

	pXML->ScanPosition(0);
	bool bParentEnabled = pXML->ToBoolean("Enable");
	pXML->ProgressiveScan(true);

	//Pop the list-view.
	for(XMLReader xml; pXML->ToReader("Page", &xml);)
	{
		//Read XML.
		xml.ToString("File", sFile, sizeof(sFile));
		xml.ToString("Description", sDescription, sizeof(sDescription));
		bEnable = xml.ToBoolean("Enable");
		xml.Destroy();

		//Build an object
		DEFAULTPAGE obj;
		obj.Page = sFile;
		obj.Description = sDescription;
		obj.Enabled = bEnable;
		obj.ParentEnabled = bParentEnabled;

		//Add the object to the list.
		PopDefaultPagesList_AddItem(hList, &obj);
	}

	LVAutoSizeColumns(hList, 500);
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)TRUE, (LPARAM)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LoadNewDefaultPagesConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML)
{
	int iGridItems = LVItemCount(hGrid);

	char sPage[DEFAULTPAGES_MAX_PAGE_LENGTH];
	char sDescr[DEFAULTPAGES_MAX_DESCR_LENGTH];

	pXML->Initialize("DefaultPages");

	pXML->AddBool("Enable", IsDlgButtonChecked(hWnd, IDC_ENABLED) == BST_CHECKED);

	for(int iItem = 0; iItem < iGridItems; iItem++)
	{
		XMLWriter Item("Page");

		LVGetText(hGrid, iItem, DEFAULTPAGES_LIST_POS_PAGE, sPage, sizeof(sPage));
		Item.Add("File", sPage);

		LVGetText(hGrid, iItem, DEFAULTPAGES_LIST_POS_DESCR, sDescr, sizeof(sDescr));
		Item.Add("Description", sDescr);

		Item.AddBool("Enable", LVChecked(hGrid, iItem));
		pXML->Add(&Item);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopServerSettingsList_User_AddItem(HWND hList, SERVERUSER *obj, bool bSaveCopyOfPlainTextPassword)
{
	PopServerSettingsList_User_AddItem(hList, obj, bSaveCopyOfPlainTextPassword, ListView_GetItemCount(hList));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopServerSettingsList_User_AddItem(HWND hList, SERVERUSER *obj, bool bSaveCopyOfPlainTextPassword, int iInsertionIndex)
{
	LV_ITEM ListItem;
	memset(&ListItem, 0, sizeof(ListItem));

	int iImage = obj->Enabled && obj->ParentEnabled;

	size_t iPasswordLength = strlen(obj->Password);
	if(bSaveCopyOfPlainTextPassword)
	{
		ListItem.lParam = (LPARAM) pMem->Allocate(iPasswordLength + 1, sizeof(char));
		strcpy_s((char *)ListItem.lParam, iPasswordLength + 1, obj->Password); //Allocate RAM to LPARAM. MUST CLEAN UP!!
	}	

	ListItem.pszText = (char *) obj->Username;
    ListItem.mask = LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
	ListItem.iImage = iImage;
    ListItem.iSubItem = SERVERS_LIST_POS_USERNAME;
	ListItem.iItem = iInsertionIndex;
	int iItem = ListView_InsertItem(hList, &ListItem);

	char *sStars = Space(iPasswordLength);
	ReplaceCharacter(sStars, ' ', '*');
    ListItem.pszText = (char *) sStars;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = SERVERS_LIST_POS_PASSWORD;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);
	free(sStars);

    ListItem.pszText = (char *) obj->Description;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = SERVERS_LIST_POS_DESCR;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListView_SetCheckState(hList, iItem, obj->Enabled);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopServerSettingsList_User(HWND hList, bool bUseIcons, XMLReader *pXML)
{
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)FALSE, (LPARAM)0);

    LV_COLUMN lvColumn;
    memset(&lvColumn, 0, sizeof(LV_COLUMN));

	lvColumn.cx        = 100;
    lvColumn.pszText   = "Username";
    lvColumn.iSubItem  = SERVERS_LIST_POS_USERNAME;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, SERVERS_LIST_POS_USERNAME, &lvColumn);

	lvColumn.cx        = 75;
    lvColumn.pszText   = "Password";
    lvColumn.iSubItem  = SERVERS_LIST_POS_PASSWORD;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, SERVERS_LIST_POS_PASSWORD, &lvColumn);

	lvColumn.cx        = 300;
    lvColumn.pszText   = "Description";
    lvColumn.iSubItem  = SERVERS_LIST_POS_DESCR;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, SERVERS_LIST_POS_DESCR, &lvColumn);

	DWORD dwFlags = LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP;

	if(!bUseIcons)
	{
		dwFlags |= LVS_EX_CHECKBOXES;
	}

    SendMessage(hList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)dwFlags);

	if(bUseIcons)
	{
		ListView_SetImageList(hList, hEnableDisableImageList, LVSIL_SMALL);
	}

	ListView_SetTextColor(hList, RGB(0,0,100));
	
	char sUsername[SERVERS_MAX_USERNAME_LENGTH];
	char sPassword[SERVERS_MAX_PASSWORD_LENGTH];
	char sDescription[SERVERS_MAX_DESCR_LENGTH];
	bool bEnable = false;

	pXML->ScanPosition(0);
	bool bParentEnabled = pXML->ToBoolean("Enable");
	pXML->ProgressiveScan(true);

	//Pop the list-view.
	for(XMLReader xml; pXML->ToReader("User", &xml);)
	{
		//Read XML.
		xml.ToString("Username", sUsername, sizeof(sUsername));
		xml.ToString("Password", sPassword, sizeof(sPassword));
		xml.ToString("Description", sDescription, sizeof(sDescription));
		bEnable = xml.ToBoolean("Enable");
		xml.Destroy();

		//Build an object
		SERVERUSER obj;
		obj.Username = sUsername;
		obj.Password = sPassword;
		obj.Description = sDescription;
		obj.Enabled = bEnable;
		obj.ParentEnabled = bParentEnabled;

		//Add the object to the list.
		PopServerSettingsList_User_AddItem(hList, &obj, !bUseIcons);
	}

	LVAutoSizeColumns(hList, 500);
	LVSetColumnWidth(hList, SERVERS_LIST_POS_PASSWORD, LVSCW_AUTOSIZE, 75);
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)TRUE, (LPARAM)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LoadNewServerSettings_User_Configuration(HWND hWnd, HWND hGrid, XMLWriter *pXML)
{
	int iGridItems = LVItemCount(hGrid);

	char sUsername[SERVERS_MAX_USERNAME_LENGTH];
	//char sPassword[SERVERS_MAX_PASSWORD_LENGTH];
	char sDescription[SERVERS_MAX_DESCR_LENGTH];

	pXML->AddBool("AllowRemoteManagement", IsDlgButtonChecked(hWnd, IDC_ALLOWREMOTEMGMT) != 0);

	for(int iItem = 0; iItem < iGridItems; iItem++)
	{
		XMLWriter Item("User");

		LVGetText(hGrid, iItem, SERVERS_LIST_POS_USERNAME, sUsername, sizeof(sUsername));
		Item.Add("Username", sUsername);

		LPARAM lAddress = LVGetItemParam(hGrid, iItem, 0);
		//LVGetText(hGrid, iItem, SERVERS_LIST_POS_PASSWORD, sPassword, sizeof(sPassword));
		Item.Add("Password", (char *)lAddress);

		LVGetText(hGrid, iItem, SERVERS_LIST_POS_DESCR, sDescription, sizeof(sDescription));
		Item.Add("Description", sDescription);

		Item.AddBool("Enable", LVChecked(hGrid, iItem));
		pXML->Add(&Item);
	}


	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RefreshServerPerformanceList(HWND hList, XMLReader *pXML)
{
	char sName[MAX_SITE_NAME];
	char sText[255];
	char sInt[64];

	//Pop the list-view.
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)FALSE, (LPARAM)0);

	int iItemCount = ListView_GetItemCount(hList);

	pXML->ProgressiveScan(true);

	for(XMLReader xml; pXML->ToReader("Site", &xml);)
	{
		if(xml.ToString("Name", sName, sizeof(sName)))
		{
			int iSiteIndex = xml.ToInteger("Index");
			__int64 i64BytesSent = xml.ToI64("BytesSent");
			__int64 i64BytesRecv = xml.ToI64("BytesReceived");
			__int64 i64Hits = xml.ToI64("Hits");
			int iCurrentConnections = xml.ToInteger("CurrentConnections");
			int iMaxConnections = xml.ToInteger("MaxConnections");
			bool bIsActive = xml.ToBoolean("IsActive");

			LV_ITEM LVItem;
			memset(&LVItem, 0, sizeof(LVItem));

			int iImage = bIsActive;

			LVItem.pszText = (char *) sName;
			LVItem.mask = LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE;
			LVItem.iSubItem = WEBSITES_LIST_POS_NAME;
			LVItem.iItem = iSiteIndex;
			LVItem.lParam = iSiteIndex;
			LVItem.iImage = iImage;

			if(iItemCount > 0)
			{
				ListView_SetItem(hList, &LVItem);
			}
			else{
				ListView_InsertItem(hList, &LVItem);
			}

			LVItem.pszText = (char *) FileSizeFriendly(i64BytesSent, 2, sText, sizeof(sText));
			LVItem.mask = LVIF_TEXT;
			LVItem.iSubItem = WEBSITES_LIST_POS_BSENT;
			LVItem.iItem = iSiteIndex;
			ListView_SetItem(hList, &LVItem);

			LVItem.pszText = (char *) FileSizeFriendly(i64BytesRecv, 2, sText, sizeof(sText));
			LVItem.mask = LVIF_TEXT;
			LVItem.iSubItem = WEBSITES_LIST_POS_BRECV;
			LVItem.iItem = iSiteIndex;
			ListView_SetItem(hList, &LVItem);

			LVItem.pszText = (char *) I64FormatInteger(sInt, sizeof(sInt), i64Hits);
			LVItem.mask = LVIF_TEXT;
			LVItem.iSubItem = WEBSITES_LIST_POS_HITS;
			LVItem.iItem = iSiteIndex;
			ListView_SetItem(hList, &LVItem);

			strcpy_s(sText, sizeof(sText), FormatInteger(sInt, sizeof(sInt), iCurrentConnections));
			strcat_s(sText, sizeof(sText), " of ");
			strcat_s(sText, sizeof(sText), FormatInteger(sInt, sizeof(sInt), iMaxConnections));
			LVItem.pszText = (char *) sText;
			LVItem.mask = LVIF_TEXT;
			LVItem.iSubItem = WEBSITES_LIST_POS_CONNECTS;
			LVItem.iItem = iSiteIndex;
			ListView_SetItem(hList, &LVItem);

		}

		xml.Destroy();
	}

	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)TRUE, (LPARAM)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopServerPerformanceList(HWND hList, XMLReader *pXML)
{
	LV_COLUMN lvColumn;
    memset(&lvColumn, 0, sizeof(LV_COLUMN));

	lvColumn.cx        = 100;
    lvColumn.pszText   = "Name";
    lvColumn.iSubItem  = WEBSITES_LIST_POS_NAME;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, WEBSITES_LIST_POS_NAME, &lvColumn);

	lvColumn.cx        = 500;
    lvColumn.pszText   = "Bytes Sent";
    lvColumn.iSubItem  = WEBSITES_LIST_POS_BSENT;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_RIGHT;
    ListView_InsertColumn(hList, WEBSITES_LIST_POS_BSENT, &lvColumn);

	lvColumn.cx        = 500;
    lvColumn.pszText   = "Bytes Recv";
    lvColumn.iSubItem  = WEBSITES_LIST_POS_BRECV;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_RIGHT;
    ListView_InsertColumn(hList, WEBSITES_LIST_POS_BRECV, &lvColumn);

	lvColumn.cx        = 500;
    lvColumn.pszText   = "Hits";
    lvColumn.iSubItem  = WEBSITES_LIST_POS_HITS;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_RIGHT;
    ListView_InsertColumn(hList, WEBSITES_LIST_POS_HITS, &lvColumn);

	lvColumn.cx        = 500;
    lvColumn.pszText   = "Connections";
    lvColumn.iSubItem  = WEBSITES_LIST_POS_CONNECTS;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_RIGHT;
    ListView_InsertColumn(hList, WEBSITES_LIST_POS_CONNECTS, &lvColumn);

	SendMessage(hList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0,
		(LPARAM)LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP);

	ListView_SetImageList(hList, hWebsiteStartedStoppedList, LVSIL_SMALL);
	
	ListView_SetTextColor(hList, RGB(0,0,100));

	RefreshServerPerformanceList(hList, pXML);

	LVAutoSizeColumns(hList, 500);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopSocketPoolsList_AddItem(HWND hList, SOCKETPOOL *obj, const char *sXML)
{
	PopSocketPoolsList_AddItem(hList, obj, ListView_GetItemCount(hList), sXML);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopSocketPoolsList_AddItem(HWND hList, SOCKETPOOL *obj, int iInsertionIndex, const char *sXML)
{
	LV_ITEM ListItem;
    memset(&ListItem, 0, sizeof(ListItem));

	char sNumber[64];

	if(sXML)
	{
		ListItem.lParam = (LPARAM)pMem->StrDup(sXML); //Allocate RAM to LPARAM. MUST CLEAN UP!!
	}

	ListItem.pszText = (char *) FormatInteger(sNumber, sizeof(sNumber), obj->Port);
    ListItem.mask = LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
    ListItem.iSubItem = SOCKETPOOLS_LIST_POS_PORT;
	ListItem.iItem = iInsertionIndex;
	ListItem.iImage = obj->IsActive;
    int iItem = ListView_InsertItem(hList, &ListItem);

	ListItem.pszText = (char *) FormatInteger(sNumber, sizeof(sNumber), obj->MaxConnections);
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = SOCKETPOOLS_LIST_POS_MAXCONNECTIONS;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListItem.pszText = (char *) FormatInteger(sNumber, sizeof(sNumber), obj->InitialBufferSize);
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = SOCKETPOOLS_LIST_POS_INITBUFFERSIZE;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListItem.pszText = (char *) FormatInteger(sNumber, sizeof(sNumber), obj->MaxBufferSize);
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = SOCKETPOOLS_LIST_POS_MAXBUFFERSIZE;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListItem.pszText = (char *) FormatInteger(sNumber, sizeof(sNumber), obj->FileBufferSize);
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = SOCKETPOOLS_LIST_POS_FILEBUFFERSIZE;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListItem.pszText = (char *) obj->Description;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = SOCKETPOOLS_LIST_POS_DESCR;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListItem.pszText = (char *) obj->UID;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = SOCKETPOOLS_LIST_POS_UID;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	if(obj->ReferencedSites >= 0)
	{
		ListItem.pszText = (char *) FormatInteger(sNumber, sizeof(sNumber), obj->ReferencedSites);
		ListItem.mask = LVIF_TEXT;
		ListItem.iSubItem = SOCKETPOOLS_LIST_POS_REFERENCES;
		ListItem.iItem = iInsertionIndex;
		ListView_SetItem(hList, &ListItem);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopSocketPoolsList(HWND hList, bool bUseIcons, XMLReader *pXML, bool bSaveXMLInLParam)
{
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)FALSE, (LPARAM)0);

    LV_COLUMN lvColumn;
    memset(&lvColumn, 0, sizeof(LV_COLUMN));

	lvColumn.cx        = 100;
    lvColumn.pszText   = "Port";
    lvColumn.iSubItem  = SOCKETPOOLS_LIST_POS_PORT;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, SOCKETPOOLS_LIST_POS_PORT, &lvColumn);

	lvColumn.cx        = 100;
    lvColumn.pszText   = "Max Connections";
    lvColumn.iSubItem  = SOCKETPOOLS_LIST_POS_MAXCONNECTIONS;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, SOCKETPOOLS_LIST_POS_MAXCONNECTIONS, &lvColumn);

	lvColumn.cx        = 100;
    lvColumn.pszText   = "Buffer Size";
    lvColumn.iSubItem  = SOCKETPOOLS_LIST_POS_INITBUFFERSIZE;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, SOCKETPOOLS_LIST_POS_INITBUFFERSIZE, &lvColumn);

	lvColumn.cx        = 100;
    lvColumn.pszText   = "Max Buffer Size";
    lvColumn.iSubItem  = SOCKETPOOLS_LIST_POS_MAXBUFFERSIZE;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, SOCKETPOOLS_LIST_POS_MAXBUFFERSIZE, &lvColumn);

	lvColumn.cx        = 100;
    lvColumn.pszText   = "File Buffer Size";
    lvColumn.iSubItem  = SOCKETPOOLS_LIST_POS_FILEBUFFERSIZE;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, SOCKETPOOLS_LIST_POS_FILEBUFFERSIZE, &lvColumn);

	lvColumn.cx        = 300;
    lvColumn.pszText   = "Description";
    lvColumn.iSubItem  = SOCKETPOOLS_LIST_POS_DESCR;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, SOCKETPOOLS_LIST_POS_DESCR, &lvColumn);

	lvColumn.cx        = 0;
    lvColumn.pszText   = "UID";
    lvColumn.iSubItem  = SOCKETPOOLS_LIST_POS_UID;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, SOCKETPOOLS_LIST_POS_UID, &lvColumn);

	if(bUseIcons)
	{
		lvColumn.cx        = 50;
		lvColumn.pszText   = "Sites";
		lvColumn.iSubItem  = SOCKETPOOLS_LIST_POS_REFERENCES;
		lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
		lvColumn.fmt       = LVCFMT_LEFT;
		ListView_InsertColumn(hList, SOCKETPOOLS_LIST_POS_REFERENCES, &lvColumn);
	}

	DWORD dwFlags = LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP;

    SendMessage(hList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)dwFlags);

	if(bUseIcons)
	{
		ListView_SetImageList(hList, hPoolActiveInactiveImageList, LVSIL_SMALL);
	}

	ListView_SetTextColor(hList, RGB(0,0,100));
	
	int iPort = 0;
	int iMaxConnections = 0;
	int iInitialBufferSize = 0;
	int iMaxBufferSize = 0;
	int iFileBufferSize = 0;
	char sUID[SOCKETPOOLS_MAX_UID_LENGTH];
	char sDescription[SOCKETPOOLS_MAX_DESCR_LENGTH];
	bool bIsActive = false;
	int iReferencedSites = 0;

	pXML->ScanPosition(0);
	pXML->ProgressiveScan(true);

	//Pop the list-view.
	for(XMLReader xml; pXML->ToReader("SocketPool", &xml);)
	{
		//Read XML.
		iPort = xml.ToInteger("Port");
		iMaxConnections = xml.ToInteger("MaxConnections");
		iInitialBufferSize = xml.ToInteger("InitialBufferSize");
		iMaxBufferSize = xml.ToInteger("MaxBufferSize");
		iFileBufferSize = xml.ToInteger("FileBufferSize");
		xml.ToString("Description", sDescription, sizeof(sDescription));
		xml.ToString("UID", sUID, sizeof(sUID));
		bIsActive = xml.ToBoolean("IsActive");
		iReferencedSites = xml.ToInteger("ReferencedSites", -1);

		//Build an object
		SOCKETPOOL obj;
		obj.UID = sUID;
		obj.Port = iPort;
		obj.InitialBufferSize = iInitialBufferSize;
		obj.MaxBufferSize = iMaxBufferSize;
		obj.FileBufferSize = iFileBufferSize;
		obj.Description = sDescription;
		obj.MaxConnections = iMaxConnections;
		obj.IsActive = bIsActive;
		obj.ReferencedSites = iReferencedSites;

		const char *sXMLText = NULL;
		if(bSaveXMLInLParam)
		{
			 sXMLText = xml.Text();
		}

		//Add the object to the list.
		PopSocketPoolsList_AddItem(hList, &obj, sXMLText);

		xml.Destroy();
	}

	LVAutoSizeColumns(hList, 500);
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)TRUE, (LPARAM)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LoadNewSocketPoolsConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML)
{
	int iGridItems = LVItemCount(hGrid);

	char sText[MAX_TEXT_SIZE];
	char sDescr[SOCKETPOOLS_MAX_DESCR_LENGTH];

	pXML->Initialize("SocketPools");

	for(int iItem = 0; iItem < iGridItems; iItem++)
	{
		XMLWriter Item("SocketPool");

		LVGetText(hGrid, iItem, SOCKETPOOLS_LIST_POS_UID, sText, sizeof(sText));
		Item.Add("UID", sText);

		LVGetText(hGrid, iItem, SOCKETPOOLS_LIST_POS_PORT, sText, sizeof(sText));
		Item.Add("Port", sText);

		LVGetText(hGrid, iItem, SOCKETPOOLS_LIST_POS_MAXCONNECTIONS, sText, sizeof(sText));
		Item.Add("MaxConnections", sText);

		LVGetText(hGrid, iItem, SOCKETPOOLS_LIST_POS_INITBUFFERSIZE, sText, sizeof(sText));
		Item.Add("InitialBufferSize", sText);

		LVGetText(hGrid, iItem, SOCKETPOOLS_LIST_POS_MAXBUFFERSIZE, sText, sizeof(sText));
		Item.Add("MaxBufferSize", sText);

		LVGetText(hGrid, iItem, SOCKETPOOLS_LIST_POS_FILEBUFFERSIZE, sText, sizeof(sText));
		Item.Add("FileBufferSize", sText);

		LVGetText(hGrid, iItem, SOCKETPOOLS_LIST_POS_DESCR, sDescr, sizeof(sDescr));
		Item.Add("Description", sDescr);

		Item.AddBool("Enable", ParseBoolean(LVChecked(hGrid, iItem)));

		LPARAM xmlParam = LVGetItemParam(hGrid, iItem, 0);
		const char *sXMLText = (char *)xmlParam;
		if(sXMLText)
		{
			XMLReader xmlIPAddresses(sXMLText);
			xmlIPAddresses.ToReader("IPAddresses", &xmlIPAddresses);

			Item.Add("IPAddresses", &xmlIPAddresses);
		}

		pXML->Add(&Item);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopHostHeadersList_AddItem(HWND hList, HOSTHEADER *obj)
{
	PopHostHeadersList_AddItem(hList, obj, ListView_GetItemCount(hList));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopHostHeadersList_AddItem(HWND hList, HOSTHEADER *obj, int iInsertionIndex)
{
	LV_ITEM ListItem;
    memset(&ListItem, 0, sizeof(ListItem));

	int iImage = obj->Enabled;

	ListItem.pszText = (char *) obj->Name;
    ListItem.mask = LVIF_TEXT|LVIF_IMAGE;
	ListItem.iImage = iImage;
    ListItem.iSubItem = SOCKETPOOLS_IPS_LIST_POS_IP;
	ListItem.iItem = iInsertionIndex;
    int iItem = ListView_InsertItem(hList, &ListItem);

    ListItem.pszText = (char *) obj->Description;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = HOSTHEADERS_LIST_POS_DESCR;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListView_SetCheckState(hList, iItem, obj->Enabled);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopHostHeadersList(HWND hList, bool bUseIcons, XMLReader *pXML)
{
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)FALSE, (LPARAM)0);

    LV_COLUMN lvColumn;
    memset(&lvColumn, 0, sizeof(LV_COLUMN));

	lvColumn.cx        = 100;
    lvColumn.pszText   = "Name";
    lvColumn.iSubItem  = HOSTHEADERS_LIST_POS_NAME;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, HOSTHEADERS_LIST_POS_NAME, &lvColumn);

	lvColumn.cx        = 300;
    lvColumn.pszText   = "Description";
    lvColumn.iSubItem  = HOSTHEADERS_LIST_POS_DESCR;
    lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
    lvColumn.fmt       = LVCFMT_LEFT;
    ListView_InsertColumn(hList, HOSTHEADERS_LIST_POS_DESCR, &lvColumn);

	DWORD dwFlags = LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP;

	if(!bUseIcons)
	{
		dwFlags |= LVS_EX_CHECKBOXES;
	}

    SendMessage(hList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0, (LPARAM)dwFlags);

	if(bUseIcons)
	{
		ListView_SetImageList(hList, hEnableDisableImageList, LVSIL_SMALL);
	}

	ListView_SetTextColor(hList, RGB(0,0,100));

	if(pXML)
	{
		char sName[HOSTHEADERS_MAX_NAME_LENGTH];
		char sDescription[HOSTHEADERS_MAX_DESCR_LENGTH];
		bool bEnable = false;

		XMLReader xmlSubItem;
		pXML->ToReader("HostHeaders", &xmlSubItem);

		xmlSubItem.ScanPosition(0);
		xmlSubItem.ProgressiveScan(true);

		//Pop the list-view.
		XMLReader xml;
		for(XMLReader xml; xmlSubItem.ToReader("HostHeader", &xml);)
		{
			//Read XML.
			xml.ToString("Name", sName, sizeof(sName));
			xml.ToString("Description", sDescription, sizeof(sDescription));
			bEnable = xml.ToBoolean("Enable");

			xml.Destroy();

			//Build an object
			HOSTHEADER obj;
			obj.Name = sName;
			obj.Description = sDescription;
			obj.Enabled = bEnable;

			//Add the object to the list.
			PopHostHeadersList_AddItem(hList, &obj);
		}
	}

	LVAutoSizeColumns(hList, 500);
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)TRUE, (LPARAM)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LoadNewHostHeadersConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML)
{
	int iGridItems = LVItemCount(hGrid);

	char sName[HOSTHEADERS_MAX_NAME_LENGTH];
	char sDescr[HOSTHEADERS_MAX_DESCR_LENGTH];

	pXML->Initialize("HostHeaders");

	for(int iItem = 0; iItem < iGridItems; iItem++)
	{
		XMLWriter Item("HostHeader");

		LVGetText(hGrid, iItem, HOSTHEADERS_LIST_POS_NAME, sName, sizeof(sName));
		Item.Add("Name", sName);

		LVGetText(hGrid, iItem, HOSTHEADERS_LIST_POS_DESCR, sDescr, sizeof(sDescr));
		Item.Add("Description", sDescr);

		Item.AddBool("Enable", ParseBoolean(LVChecked(hGrid, iItem)));

		pXML->Add(&Item);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
