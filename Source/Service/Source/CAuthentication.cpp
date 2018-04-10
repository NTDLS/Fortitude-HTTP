///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CAuthentication_CPP
#define _CAuthentication_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>
#include <ShellAPI.H>

extern HIMAGELIST hEnableDisableImageList; //Declared in MainDialog.cpp
extern HIMAGELIST hOnePixilImageList; //Declared in MainDialog.cpp

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CAuthentication.H"

#include "Entry.H"

#include "CWebSites.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::File;
using namespace NSWFL::ListView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CAuthentication::~CAuthentication(void)
{
	this->Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CAuthentication::CAuthentication(void *lpWebSites)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CAuthentication::CAuthentication(void *lpWebSites, CXMLReader *xmlConfig, CAuthentication *pDefaults)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
	this->Load(xmlConfig, pDefaults);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CAuthentication::Save(void)
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

bool CAuthentication::ToXML(CXMLReader *lpXML)
{
	this->Locks.LockShared();

	CXMLWriter xmlConfig("Authentication");

	xmlConfig.AddBool("Enable", this->Collection.Enabled);

	for(int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		CXMLWriter Item("Page");
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
bool CAuthentication::Reload(void)
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

bool CAuthentication::Load(const char *sXMLFileName)
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
		if(xmlConfig.ToReader("Authentication", &xmlEntity))
		{
			this->Initialized = this->Load(&xmlEntity, NULL);
		}
		xmlEntity.Destroy();
		xmlConfig.Destroy();
	}

	return this->Locks.UnlockExclusive(this->Initialized);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CAuthentication::Load(CXMLReader *xmlConfig, CAuthentication *pDefaults)
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
	CXMLReader XPDefaultPage;

	while(xmlConfig->ToReader("Page", &XPDefaultPage))
	{
		char sPage[AUTHENTICATION_MAX_PAGE_LENGTH];
		char sDescr[AUTHENTICATION_MAX_DESCR_LENGTH];

		this->Collection.Items = (PROTECTEDPAGE *)
			pMem->ReAllocate(this->Collection.Items, sizeof(PROTECTEDPAGE), this->Collection.Count + 1);

		PROTECTEDPAGE *p = &this->Collection.Items[this->Collection.Count++];

		XPDefaultPage.ToString("File", sPage, sizeof(sPage), &iLength);
		p->Page = (char *) pMem->StrDup(LCase(sPage, iLength));
		p->PageLength = (int) strlen(p->Page);

		XPDefaultPage.ToString("Description", sDescr, sizeof(sDescr), &iLength);
		p->Description = (char *) pMem->StrDup(sDescr);

		p->Enabled = XPDefaultPage.ToBoolean("Enable", true);

		XPDefaultPage.Destroy();
	}

	return this->Locks.UnlockExclusive(this->Initialized = true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CAuthentication::Destroy(void)
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

bool CAuthentication::Authenticate(VOID *pClient)
{
	PEER *pC = (PEER *)pClient;

	this->Locks.LockShared();

	//FIXFIX: Implement!

	return this->Locks.UnlockShared(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CAuthentication::IsAuthenticated(VOID *pClient)
{
	PEER *pC = (PEER *)pClient;

	this->Locks.LockShared();

	//FIXFIX: Implement!

	return this->Locks.UnlockShared(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns true if a default page is found, otherwise false.
*/
bool CAuthentication::IsAuthorizationRequired(VOID *pClient)
{
	PEER *pC = (PEER *)pClient;
	CWebSite *pWebSite = (CWebSite *)pC->pWebSite;

	this->Locks.LockShared();

	if(this->Collection.Enabled && (this->Collection.Count > 0))
	{
		for(int iPos = 0; iPos < this->Collection.Count; iPos++)
		{
			if(this->Collection.Items[iPos].Enabled)
			{
				if(_strcmpi(this->Collection.Items[iPos].Page, pC->Header.Request) == 0)
				{
					return true;
				}
			}
		}
	}

	return this->Locks.UnlockShared(false);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
