///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright � NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CDefaultPages_CPP
#define _CDefaultPages_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <ShellAPI.H>
#include <shlobj.h>

extern HIMAGELIST hEnableDisableImageList; //Declared in MainDialog.cpp
extern HIMAGELIST hOnePixilImageList; //Declared in MainDialog.cpp

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CDefaultPages.H"

#include "Entry.H"

#include "CWebSites.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::File;
using namespace NSWFL::ListView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDefaultPages::~CDefaultPages(void)
{
	this->Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDefaultPages::CDefaultPages(void *lpWebSites)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDefaultPages::CDefaultPages(void *lpWebSites, XMLReader *xmlConfig, CDefaultPages *pDefaults)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
	this->Load(xmlConfig, pDefaults);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDefaultPages::Save(void)
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

bool CDefaultPages::ToXML(XMLReader *lpXML)
{
	this->Locks.LockShared();

	XMLWriter xmlConfig("DefaultPages");

	xmlConfig.AddBool("Enable", this->Collection.Enabled);

	for(int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		XMLWriter Item("Page");
		Item.Add("File", this->Collection.Items[iItem].Page);
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
bool CDefaultPages::Reload(void)
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

bool CDefaultPages::Load(const char *sXMLFileName)
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
		if(xmlConfig.ToReader("DefaultPages", &xmlEntity))
		{
			this->Initialized = this->Load(&xmlEntity, NULL);
		}
		xmlEntity.Destroy();
		xmlConfig.Destroy();
	}

	return this->Locks.UnlockExclusive(this->Initialized);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDefaultPages::Load(XMLReader *xmlConfig, CDefaultPages *pDefaults)
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
	XMLReader XPDefaultPage;

	while(xmlConfig->ToReader("Page", &XPDefaultPage))
	{
		char sPage[DEFAULTPAGES_MAX_PAGE_LENGTH];
		char sDescr[DEFAULTPAGES_MAX_DESCR_LENGTH];

		this->Collection.Items = (DEFAULTPAGE *)
			pMem->ReAllocate(this->Collection.Items, sizeof(DEFAULTPAGE), this->Collection.Count + 1);

		DEFAULTPAGE *p = &this->Collection.Items[this->Collection.Count++];

		XPDefaultPage.ToString("File", sPage, sizeof(sPage), &iLength);
		p->Page = (char *) pMem->CloneString(LCase(sPage, iLength));
		p->PageLength = (int) strlen(sPage);

		XPDefaultPage.ToString("Description", sDescr, sizeof(sDescr), &iLength);
		p->Description = (char *) pMem->CloneString(sDescr);

		p->Enabled = XPDefaultPage.ToBoolean("Enable", true);

		XPDefaultPage.Destroy();
	}

	return this->Locks.UnlockExclusive(this->Initialized = true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDefaultPages::Destroy(void)
{
	this->Locks.LockExclusive();

	if(this->Initialized)
	{
		this->Initialized = false;
		if(this->Collection.Count > 0)
		{
			for(int iPos = 0; iPos < this->Collection.Count; iPos++)
			{
				pMem->Free(this->Collection.Items[iPos].Page);
				pMem->Free(this->Collection.Items[iPos].Description);
				this->Collection.Items[iPos].PageLength = 0;
			}

			pMem->Free(this->Collection.Items);

			this->Collection.Count = 0;
		}
	}
	return this->Locks.UnlockExclusive(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns true if a default page is found, otherwise false.
*/
bool CDefaultPages::CheckPath(char *&sFullRequest)
{
	this->Locks.LockShared();

	if(this->Collection.Enabled && (this->Collection.Count > 0))
	{
		char sRequest[MAX_PATH];
		int iAlloc = 0;

		int iPathSize = (int)strlen(sFullRequest);
		for(int iPos = 0; iPos < this->Collection.Count; iPos++)
		{
			if(this->Collection.Items[iPos].Enabled)
			{
				if((iPathSize + this->Collection.Items[iPos].PageLength + 1) < sizeof(sRequest))
				{
					if((iAlloc = sprintf_s(sRequest, sizeof(sRequest),
						"%s\\%s", sFullRequest, this->Collection.Items[iPos].Page)) > 0)
					{
						CorrectReversePath(sRequest, sizeof(sRequest), true);

						if(FileAccess(sRequest, FExist))
						{
							iAlloc++;
							sFullRequest = (char *) pMem->ReAllocate(sFullRequest, sizeof(char), iAlloc);
							strcpy_s(sFullRequest, iAlloc, sRequest);

							return this->Locks.UnlockShared(true);
						}
					}
				}
			}
		}
	}

	return this->Locks.UnlockShared(false);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
