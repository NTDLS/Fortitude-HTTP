///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _ConfigTree_H
#define _ConfigTree_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <ShlObj.H>

#include "../../../../@Libraries/CXML/CXML.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TREE_IMAGE_SERVER				0
#define TREE_IMAGE_SITE_ONLINE			1
#define TREE_IMAGE_SITE_OFFLINE			2
#define TREE_IMAGE_ADVANCED				3
#define TREE_IMAGE_FILTERS				4
#define TREE_IMAGE_SCRIPTING			5
#define TREE_IMAGE_SCRIPTINGENGINES		6
#define TREE_IMAGE_WEBSITES				7
#define TREE_IMAGE_CACHE				8
#define TREE_IMAGE_CGIFOLDERS			9
#define TREE_IMAGE_CUSTOMFILTERS		10
#define TREE_IMAGE_DEFAULTPAGES			11
#define TREE_IMAGE_EXTENSIONFILTERS		12
#define TREE_IMAGE_IPFILTERS			13
#define TREE_IMAGE_MIMETYPES			14
#define TREE_IMAGE_SSIFILES				15
#define TREE_IMAGE_URLFILTERS			16
#define TREE_IMAGE_VIRTUALROOTS			17
#define TREE_IMAGE_COMPRESSION			18
#define TREE_IMAGE_ERRORPAGES			19
#define TREE_IMAGE_REALMS				20
#define TREE_IMAGE_AUTHENTICATION		21
#define TREE_IMAGE_SETTINGS				22
#define TREE_IMAGE_SITE					23
#define TREE_IMAGE_SOCKETPOOLS			24

#define PAGE_TYPE_INVALID				0
#define PAGE_TYPE_ADVANCED				1
#define PAGE_TYPE_DEFAULTPAGES			2
#define PAGE_TYPE_VIRTUALROOTS			3
#define PAGE_TYPE_COMPRESSION			4
#define PAGE_TYPE_MIMETYPES				5
#define PAGE_TYPE_IPFILTERS				6
#define PAGE_TYPE_URLFILTERS			7
#define PAGE_TYPE_EXTFILTERS			8
#define PAGE_TYPE_CUSTOMFILTERS			9
#define PAGE_TYPE_SCRIPTINGENGINES		10
#define PAGE_TYPE_SSIFILES				11
#define PAGE_TYPE_CGIFOLDERS			12
#define PAGE_TYPE_WEBSITES				13
#define PAGE_TYPE_WEBSITE				14
#define PAGE_TYPE_CACHE					15
#define PAGE_TYPE_SERVER				16
#define PAGE_TYPE_ERRORPAGES			17
#define PAGE_TYPE_REALMS				18
#define PAGE_TYPE_AUTHENTICATION		19
#define PAGE_TYPE_SETTINGS				20
#define PAGE_TYPE_SOCKETPOOLS			21

#define TREE_CMD_SEPERATOR				WM_USER + 1000
#define TREE_CMD_STOP_SITE				WM_USER + 1001
#define TREE_CMD_START_SITE				WM_USER + 1002
#define TREE_CMD_RESTART_SITE			WM_USER + 1003
#define TREE_CMD_VIEW_SITE				WM_USER + 1004
#define TREE_CMD_CONFIGURE				WM_USER + 1005
#define TREE_CMD_RESTARTSITES			WM_USER + 1006
#define TREE_CMD_RESTARTSERVER			WM_USER + 1007
#define TREE_CMD_PERSISTCONFIGURATION	WM_USER + 1008
#define TREE_CMD_CONNECTTO				WM_USER + 1009

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PopulateConfigTree(CXMLReader *pSites);
void PopulateConfigTree(void);

bool InitializeConfigTree(void);
void RefreshConfigTree(void);
bool GetSelectedConfigItem(int *iPageType, int *iSiteIndex);
bool ConfigureTreeItem(int iSiteIndex, int iPageType);
void SetConfigItem(int iType, int iSite, void *pClass, int iListIndex);
void SetConfigItem(int iType, int iSite, void *pClass);
void SetConfigItem(int iType, void *pClass);
void SetConfigItem(void);
bool ProcessXMLResult(HWND hWnd, CXMLReader *pXML, const char *sSuccessText, const char *sFailureText);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Config_Item {
	//For the configure dialog:
	int iSite;
	int iType;
	bool bGlobal;

	//For the item configure dialog:
	int iListIndex;
	HWND hList;
	HWND hDialog;
} CONFIGITEM, *LPCONFIGITEM;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CLocalConfigItemStorage {
	/*
		This class provides a way to store the original values of the global
			CONFIGITEM and have them restored when the scope is destroyed.
	*/
protected:
	CONFIGITEM *_Pointer;
	CONFIGITEM _PreviousValues;

public:
	CLocalConfigItemStorage(CONFIGITEM *lpCI)
	{
		this->_Pointer = lpCI;
		memcpy_s(&this->_PreviousValues, sizeof(this->_PreviousValues), lpCI, sizeof(CONFIGITEM));
	}
	~CLocalConfigItemStorage()
	{
		memcpy_s(this->_Pointer, sizeof(CONFIGITEM), &this->_PreviousValues, sizeof(CONFIGITEM));
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern CONFIGITEM gCI;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

