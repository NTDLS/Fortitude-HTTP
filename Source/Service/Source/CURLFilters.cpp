///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CURLFilters_CPP
#define _CURLFilters_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <shlobj.h>

extern HIMAGELIST hEnableDisableImageList; //Declared in MainDialog.cpp
extern HIMAGELIST hOnePixilImageList; //Declared in MainDialog.cpp

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CURLFilters.H"
#include "Entry.H"
#include "CWebSites.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::File;
using namespace NSWFL::ListView;
using namespace NSWFL::Conversion;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CURLFilters::~CURLFilters(void)
{
	this->Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CURLFilters::CURLFilters(void *lpWebSites)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CURLFilters::CURLFilters(void *lpWebSites, XMLReader *xmlConfig, CURLFilters *pDefaults)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
	this->Load(xmlConfig, pDefaults);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CURLFilters::Save(void)
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

bool CURLFilters::ToXML(XMLReader *lpXML)
{
	this->Locks.LockShared();

	XMLWriter xmlConfig("URLFilters");

	xmlConfig.AddBool("Enable", this->Collection.Enabled);

	for(int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		XMLWriter Item("Filter");
		Item.Add("Sequence", this->Collection.Items[iItem].Sequence);
		Item.Add("Description", this->Collection.Items[iItem].Description);
		Item.Add("ScanRequest", this->Collection.Items[iItem].ScanRequest);
		Item.Add("ScanQuery", this->Collection.Items[iItem].ScanQuery);
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
bool CURLFilters::Reload(void)
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

bool CURLFilters::Load(const char *sXMLFileName)
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
		if(xmlConfig.ToReader("URLFilters", &xmlEntity))
		{
			this->Initialized = this->Load(&xmlEntity, NULL);
		}
		xmlEntity.Destroy();
		xmlConfig.Destroy();
	}

	return this->Locks.UnlockExclusive(this->Initialized);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CURLFilters::Load(XMLReader *xmlConfig, CURLFilters *pDefaults)
{
	this->Locks.LockExclusive();
	if(this->Initialized)
	{
		this->Destroy();
	}

	int iLength = 0;
	memset(&this->Collection, 0, sizeof(this->Collection));

	xmlConfig->ProgressiveScan(true);
	XMLReader XPURLFilter;
	
	this->Collection.Enabled = xmlConfig->ToBoolean("Enable", true);

	while(xmlConfig->ToReader("Filter", &XPURLFilter))
	{
		char sSequence[URLFILTERS_MAX_SEQ_LENGTH];
		char sDescr[URLFILTERS_MAX_DESCR_LENGTH];

		this->Collection.Items = (URLFILTER *)
			pMem->ReAllocate(this->Collection.Items, sizeof(URLFILTER), this->Collection.Count + 1);

		URLFILTER *p = &this->Collection.Items[this->Collection.Count++];

		XPURLFilter.ToString("Sequence", sSequence, sizeof(sSequence), &iLength);
		p->Sequence = (char *) pMem->CloneString(sSequence);
		p->SequenceLength = (int) strlen(sSequence);

		XPURLFilter.ToString("Description", sDescr, sizeof(sDescr), &iLength);
		p->Description = (char *) pMem->CloneString(sDescr);

		p->ScanQuery = XPURLFilter.ToBoolean("ScanQuery", true);
		p->ScanRequest = XPURLFilter.ToBoolean("ScanRequest", true);
		p->Enabled = XPURLFilter.ToBoolean("Enable", true);

		XPURLFilter.Destroy();
	}

	return this->Locks.UnlockExclusive((this->Initialized = true));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CURLFilters::Enabled(void)
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->Collection.Enabled && (this->Collection.Count > 0));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CURLFilters::Destroy(void)
{
	this->Locks.LockExclusive();

	if(this->Initialized)
	{
		this->Initialized = false;
		if(this->Collection.Count > 0)
		{
			for(int iPos = 0; iPos < this->Collection.Count; iPos++)
			{
				pMem->Free(this->Collection.Items[iPos].Sequence);
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
	Returns true if the header passes, otherwise false.
*/
bool CURLFilters::ScanHeader(VOID *pHeader)
{
	this->Locks.LockShared();

	HTTPHEADER *pH = (HTTPHEADER *)(pHeader);
	if(this->Collection.Enabled && (this->Collection.Count > 0))
	{
		for(int iPos = 0; iPos < this->Collection.Count; iPos++)
		{
			if(this->Collection.Items[iPos].Enabled)
			{
				if(this->Collection.Items[iPos].ScanQuery)
				{
					if(InStrI(this->Collection.Items[iPos].Sequence,
						this->Collection.Items[iPos].SequenceLength, pH->Query, (int)strlen(pH->Query), 0) >= 0)
					{
						return this->Locks.UnlockShared(false);
					}
				}
				if(this->Collection.Items[iPos].ScanRequest)
				{
					if(InStrI(this->Collection.Items[iPos].Sequence,
						this->Collection.Items[iPos].SequenceLength, pH->Request, (int)strlen(pH->Request), 0) >= 0)
					{
						return this->Locks.UnlockShared(false);
					}
				}
			}
		}
	}

	return this->Locks.UnlockShared(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
