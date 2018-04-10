///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CMIMETYPES_CPP
#define _CMIMETYPES_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>

extern HIMAGELIST hEnableDisableImageList; //Declared in MainDialog.cpp
extern HIMAGELIST hOnePixilImageList; //Declared in MainDialog.cpp

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CMimeTypes.H"

#include "Entry.H"

#include "CWebSites.H"
#include "CWebSite.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::File;
using namespace NSWFL::ListView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMimeTypes::~CMimeTypes(void)
{
	this->Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMimeTypes::CMimeTypes(void *lpWebSites)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMimeTypes::CMimeTypes(void *lpWebSites, CXMLReader *xmlConfig, CMimeTypes *pDefaults)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
	this->Load(xmlConfig, pDefaults);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CMimeTypes::Save(void)
{
	this->Locks.LockShared();

	CXMLReader xmlConfig;
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

bool CMimeTypes::ToXML(CXMLReader *lpXML)
{
	this->Locks.LockShared();

	CXMLWriter xmlConfig("MimeTypes");

	xmlConfig.AddBool("Enable", this->Collection.Enabled);

	for(int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		CXMLWriter Item("Type");
		Item.Add("Extension", this->Collection.Items[iItem].Extension);
		Item.Add("Mime", this->Collection.Items[iItem].Type);
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
bool CMimeTypes::Reload(void)
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

bool CMimeTypes::Load(const char *sXMLFileName)
{
	this->Locks.LockExclusive();
	if(this->Initialized)
	{
		this->Destroy();
	}

	strcpy_s(this->sFileName, sizeof(this->sFileName), sXMLFileName);

	CXMLReader xmlConfig;

	if(xmlConfig.FromFile(sXMLFileName))
	{
		CXMLReader xmlEntity;
		if(xmlConfig.ToReader("MimeTypes", &xmlEntity))
		{
			this->Initialized = this->Load(&xmlEntity, NULL);
		}
		xmlEntity.Destroy();
		xmlConfig.Destroy();
	}

	return this->Locks.UnlockExclusive(this->Initialized);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CMimeTypes::Load(CXMLReader *xmlConfig, CMimeTypes *pDefaults)
{
	this->Locks.LockExclusive();
	if(this->Initialized)
	{
		this->Destroy();
	}

	int iLength = 0;
	memset(&this->Collection, 0, sizeof(this->Collection));

	this->Collection.Enabled = xmlConfig->ToBoolean("Enable", true);

	xmlConfig->ProgressiveScan(true);
	CXMLReader XPMimeType;

	while(xmlConfig->ToReader("Type", &XPMimeType))
	{
		char sExt[MIMETYPES_MAX_EXT_LENGTH];
		char sType[MIMETYPES_MAX_TYPE_LENGTH];
		char sDescr[MIMETYPES_MAX_DESCR_LENGTH];

		this->Collection.Items = (MIMETYPE *)
			pMem->ReAllocate(this->Collection.Items, sizeof(MIMETYPE), this->Collection.Count + 1);

		MIMETYPE *p = &this->Collection.Items[Collection.Count++];

		XPMimeType.ToString("Extension", sExt, sizeof(sExt), &iLength);
		p->Extension = (char *) pMem->StrDup(LCase(sExt, iLength));

		XPMimeType.ToString("Mime", sType, sizeof(sType), &iLength);
		p->Type = (char *) pMem->StrDup(sType);

		XPMimeType.ToString("Description", sDescr, sizeof(sDescr), &iLength);
		p->Description = (char *) pMem->StrDup(sDescr);

		p->Enabled = XPMimeType.ToBoolean("Enable", true);

		XPMimeType.Destroy();
	}

	return this->Locks.UnlockExclusive(this->Initialized = true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CMimeTypes::Destroy(void)
{
	this->Locks.LockExclusive();

	if(this->Initialized)
	{
		this->Initialized = false;
		if(this->Collection.Count > 0)
		{
			for(int iPos = 0; iPos < this->Collection.Count; iPos++)
			{
				pMem->Free(this->Collection.Items[iPos].Extension);
				pMem->Free(this->Collection.Items[iPos].Type);
				pMem->Free(this->Collection.Items[iPos].Description);
			}

			pMem->Free(this->Collection.Items);

			this->Collection.Count = 0;
		}
	}
	return this->Locks.UnlockExclusive(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CMimeTypes::GetType(const char *sFileName, char *sMimeName, int iMaxMimeName)
{
	this->Locks.LockShared();
	if(this->Collection.Enabled && (this->Collection.Count > 0))
	{
		char sExt[MAX_PATH];
		GetFileExtension(sFileName, sExt, sizeof(sExt));

		for(int iPos = 0; iPos < this->Collection.Count; iPos++)
		{
			if(this->Collection.Items[iPos].Enabled)
			{
				if(_strcmpi(sExt, this->Collection.Items[iPos].Extension) == 0)
				{
					strcpy_s(sMimeName, iMaxMimeName, this->Collection.Items[iPos].Type);
					return this->Locks.UnlockShared(true);
				}
			}
		}
	}

	strcpy_s(sMimeName, iMaxMimeName, "application/octet-stream");
	
	return this->Locks.UnlockShared(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CMimeTypes::Export(const char *sFileName)
{
	this->Locks.LockShared();

	FILE *hTarget = NULL;
	if(fopen_s(&hTarget, sFileName, "wb") == 0)
	{
		for(int iPos = 0; iPos < this->Collection.Count; iPos++)
		{
			fprintf(hTarget, "%s\t%s\t%s\t%s\r\n",
				this->Collection.Items[iPos].Extension,
				this->Collection.Items[iPos].Type,
				this->Collection.Items[iPos].Enabled ? "Yes" : "No",
				this->Collection.Items[iPos].Description);
		}
		fclose(hTarget);
		return this->Locks.UnlockShared(true);
	}

	return this->Locks.UnlockShared(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
