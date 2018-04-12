///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CCGIFolders_CPP
#define _CCGIFolders_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <shlobj.h>

extern HIMAGELIST hEnableDisableImageList; //Declared in MainDialog.cpp
extern HIMAGELIST hOnePixilImageList; //Declared in MainDialog.cpp

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CCGIFolders.H"
#include "Entry.H"
#include "CWebSites.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::File;
using namespace NSWFL::ListView;
using namespace NSWFL::Windows;
using namespace NSWFL::Conversion;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CCGIFolders::~CCGIFolders(void)
{
	this->Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CCGIFolders::CCGIFolders(void *lpWebSites)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CCGIFolders::CCGIFolders(void *lpWebSites, XMLReader *xmlConfig, CCGIFolders *pDefaults)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
	this->Load(xmlConfig, pDefaults);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCGIFolders::Save(void)
{
	this->Locks.LockShared();

	XMLReader xmlConfig;
	if(this->ToXML(&xmlConfig))
	{
		bool bResult = xmlConfig.ToFile(this->sFileName);
		if(!bResult)
		{
			char sErrorMsg[2048];
			GetLastError(sErrorMsg, sizeof(sErrorMsg),
				"Failed to save websites configuration file (", ")");
			strcat_s(sErrorMsg, sizeof(sErrorMsg), "\r\n\r\n\"");
			strcat_s(sErrorMsg, sizeof(sErrorMsg), this->sFileName);
			strcat_s(sErrorMsg, sizeof(sErrorMsg), "\"");
			((CWebSites *)this->pWebSites)->Trace(sErrorMsg);
		}
		return this->Locks.UnlockShared(bResult);
	}
	return this->Locks.UnlockShared(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCGIFolders::ToXML(XMLReader *lpXML)
{
	this->Locks.LockShared();

	XMLWriter xmlConfig("CGIFolders");

	xmlConfig.AddBool("Enable", this->Collection.Enabled);

	for(int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		XMLWriter Item("Folder");
		Item.Add("Path", this->Collection.Items[iItem].Path);
		Item.Add("Description", this->Collection.Items[iItem].Description);
		Item.AddBool("Enable", this->Collection.Items[iItem].Enabled);
		xmlConfig.Add(&Item);
	}

	xmlConfig.ToReader(lpXML);

	xmlConfig.Destroy();

	return this->Locks.UnlockShared(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	Reloads the configuration from the file it was originally loaded from.
*/
bool CCGIFolders::Reload(void)
{
	this->Locks.LockExclusive();

	if(this->Initialized)
	{
		this->Destroy();
	}

	this->Load(this->sFileName);

	return this->Locks.UnlockExclusive(this->Initialized);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCGIFolders::Load(const char *sXMLFileName)
{
	this->Locks.LockExclusive();
	if(this->Initialized)
	{
		this->Destroy();
	}

	strcpy_s(this->sFileName, sizeof(this->sFileName), sXMLFileName);

	XMLReader xmlConfig;

	if(xmlConfig.FromFile(sXMLFileName))
	{
		XMLReader xmlEntity;
		if(xmlConfig.ToReader("CGIFolders", &xmlEntity))
		{
			this->Initialized = this->Load(&xmlEntity, NULL);
		}
		xmlEntity.Destroy();
		xmlConfig.Destroy();
	}

	return this->Locks.UnlockExclusive(this->Initialized);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCGIFolders::Load(XMLReader *xmlConfig, CCGIFolders *pDefaults)
{
	this->Locks.LockExclusive();
	if(this->Initialized)
	{
		this->Destroy();
	}

	int iLength = 0;
	memset(&this->Collection, 0, sizeof(this->Collection));

	this->Collection.Enabled = xmlConfig->ToBoolean("Enable", false);

	xmlConfig->ProgressiveScan(true);
	XMLReader XPCGIFolder;

	while(xmlConfig->ToReader("Folder", &XPCGIFolder))
	{
		char sPath[CGIFOLDER_MAX_PATH_LENGTH];
		char sDescr[CGIFOLDER_MAX_DESCR_LENGTH];

		this->Collection.Items = (CGIFOLDER *)
			pMem->ReAllocate(this->Collection.Items, sizeof(CGIFOLDER), this->Collection.Count + 1);

		CGIFOLDER *p = &this->Collection.Items[this->Collection.Count++];

		XPCGIFolder.ToString("Path", sPath, sizeof(sPath), &iLength);
		p->Path = (char *) pMem->CloneString(sPath);
		p->PathLength = (int)strlen(sPath);

		CorrectForwardPath(p->Path, p->PathLength + 1);
		while(p->Path[p->PathLength - 1] == '/')
		{
			p->PathLength--;
			p->Path[p->PathLength] = '\0';
		}

		XPCGIFolder.ToString("Description", sDescr, sizeof(sDescr), &iLength);
		p->Description = (char *) pMem->CloneString(sDescr);

		p->Enabled = XPCGIFolder.ToBoolean("Enable", false);

		XPCGIFolder.Destroy();
	}

	return this->Locks.UnlockExclusive(this->Initialized = true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCGIFolders::Enabled(void)
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->Collection.Enabled && (this->Collection.Count > 0));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCGIFolders::Destroy(void)
{
	this->Locks.LockExclusive();

	if(this->Initialized)
	{
		this->Initialized = false;
		if(this->Collection.Count > 0)
		{
			for(int iPos = 0; iPos < this->Collection.Count; iPos++)
			{
				pMem->Free(this->Collection.Items[iPos].Path);
				pMem->Free(this->Collection.Items[iPos].Description);
			}

			pMem->Free(this->Collection.Items);

			this->Collection.Count = 0;
		}
	}
	return this->Locks.UnlockExclusive(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns true if the request is in a CGI folder, otherwise false.
*/
bool CCGIFolders::IsInCGIFolder(VOID *pClient)
{
	this->Locks.LockShared();

	if(this->Collection.Enabled && (this->Collection.Count > 0))
	{
		PEER *pC = (PEER *)pClient;

		for(int iPos = 0; iPos < this->Collection.Count; iPos++)
		{
			if(this->Collection.Items[iPos].Enabled)
			{
				if(_strnicmp(this->Collection.Items[iPos].Path,
					pC->Header.Request, this->Collection.Items[iPos].PathLength) == 0)
				{
					return this->Locks.UnlockShared(true);
				}
			}
		}
	}
	return this->Locks.UnlockShared(false);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
