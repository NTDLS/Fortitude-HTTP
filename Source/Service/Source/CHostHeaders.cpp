///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CHostHeaders_CPP
#define _CHostHeaders_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <shlobj.h>

extern HIMAGELIST hEnableDisableImageList; //Declared in MainDialog.cpp
extern HIMAGELIST hOnePixilImageList; //Declared in MainDialog.cpp

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CHostHeaders.H"
#include "Entry.H"
#include "CWebSites.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::File;
using namespace NSWFL::ListView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CHostHeaders::~CHostHeaders(void)
{
	this->Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CHostHeaders::CHostHeaders(void *lpWebSites)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CHostHeaders::CHostHeaders(void *lpWebSites, XMLReader *xmlConfig)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	memset(&this->Collection, 0, sizeof(this->Collection));
	this->Load(xmlConfig);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHostHeaders::ToXML(XMLReader *lpXML)
{
	this->Locks.LockShared();

	XMLWriter xmlConfig("HostHeaders");

	for(int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		XMLWriter Item("HostHeader");
		Item.Add("Name", this->Collection.Items[iItem].Name);
		Item.Add("Description", this->Collection.Items[iItem].Description);
		Item.Add("Enable", this->Collection.Items[iItem].Enabled);
		xmlConfig.Add(&Item);
	}

	xmlConfig.ToReader(lpXML);

	xmlConfig.Destroy();

	return this->Locks.UnlockShared(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHostHeaders::Load(XMLReader *xmlConfig)
{
	this->Locks.LockExclusive();
	if(this->Initialized)
	{
		this->Destroy();
	}

	int iLength = 0;
	memset(&this->Collection, 0, sizeof(this->Collection));

	xmlConfig->ProgressiveScan(true);
	XMLReader xmlItem;

	while(xmlConfig->ToReader("HostHeader", &xmlItem))
	{
		char sName[HOSTHEADERS_MAX_NAME_LENGTH];
		char sDescr[HOSTHEADERS_MAX_DESCR_LENGTH];

		this->Collection.Items = (HOSTHEADER *)
			pMem->ReAllocate(this->Collection.Items, sizeof(HOSTHEADER), this->Collection.Count + 1);

		HOSTHEADER *p = &this->Collection.Items[this->Collection.Count++];

		xmlItem.ToString("Name", sName, sizeof(sName), &iLength);
		p->Name = pMem->CloneString(sName);

		xmlItem.ToString("Description", sDescr, sizeof(sDescr), &iLength);
		p->Description = pMem->CloneString(sDescr);

		p->Enabled = xmlItem.ToBoolean("Enable", true);

		xmlItem.Destroy();
	}

	return this->Locks.UnlockExclusive(this->Initialized = true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHostHeaders::Destroy(void)
{
	this->Locks.LockExclusive();

	if(this->Initialized)
	{
		this->Initialized = false;
		if(this->Collection.Count > 0)
		{
			for(int iPos = 0; iPos < this->Collection.Count; iPos++)
			{
				pMem->Free(this->Collection.Items[iPos].Name);
				pMem->Free(this->Collection.Items[iPos].Description);
			}

			pMem->Free(this->Collection.Items);

			this->Collection.Count = 0;
		}
	}
	return this->Locks.UnlockExclusive(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CHostHeaders::ActiveCount(void)
{
	int iActiveCount = 0;

	for(int i = 0; i < this->Collection.Count; i++)
	{
		if(this->Collection.Items[i].Enabled)
		{
			iActiveCount++;
		}
	}
	return iActiveCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
