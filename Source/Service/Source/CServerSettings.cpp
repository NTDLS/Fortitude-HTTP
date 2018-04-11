///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CServerSettings_CPP
#define _CServerSettings_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>

extern HIMAGELIST hEnableDisableImageList; //Declared in MainDialog.cpp
extern HIMAGELIST hOnePixilImageList; //Declared in MainDialog.cpp

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CServerSettings.H"
#include "Entry.H"
#include "CWebSites.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::File;
using namespace NSWFL::ListView;
using namespace NSWFL::Hashing;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CServerSettings::~CServerSettings(void)
{
	this->Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CServerSettings::CServerSettings(void *lpWebSites)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Settings, 0, sizeof(this->Settings));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CServerSettings::CServerSettings(void *lpWebSites, XMLReader *xmlConfig, CServerSettings *pDefaults)
{
	this->Initialized = true;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Settings, 0, sizeof(this->Settings));
	this->Load(xmlConfig, pDefaults);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CServerSettings::Save(void)
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

bool CServerSettings::ToXML(XMLReader *lpXML)
{
	this->Locks.LockShared();

	XMLWriter xmlConfig("ServerSettings");

	XMLWriter xmlUsers("Users");
	xmlUsers.AddBool("AllowRemoteManagement", this->Settings.Users.AllowRemoteManagement);

	for(int iItem = 0; iItem < this->Settings.Users.Count; iItem++)
	{
		XMLWriter Item("User");
		Item.Add("Username", this->Settings.Users.Items[iItem].Username);
		Item.Add("Password", this->Settings.Users.Items[iItem].Password);
		Item.Add("Description", this->Settings.Users.Items[iItem].Description);
		Item.AddBool("Enable", this->Settings.Users.Items[iItem].Enabled);
		xmlUsers.Add(&Item);
	}
	xmlConfig.Add(&xmlUsers);

	XMLWriter xmlProcessors("Processors");
	xmlProcessors.Add("AffinityMask", this->Settings.Processor.AffinityMask);
	xmlProcessors.AddBool("ProcessBoost", this->Settings.Processor.ProcessBoost);
	xmlProcessors.AddBool("ThreadBoost", this->Settings.Processor.ThreadBoost);
	xmlConfig.Add(&xmlProcessors);

	XMLWriter xmlAdvanced("Advanced");
	xmlAdvanced.AddBool("OptimizeForThroughput", this->Settings.Advanced.OptimizeForThroughput);
	xmlAdvanced.AddBool("ForceSocketReuse", this->Settings.Advanced.ForceSocketReuse);
	xmlConfig.Add(&xmlAdvanced);

	xmlConfig.ToReader(lpXML);

	xmlConfig.Destroy();

	return this->Locks.UnlockShared(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	Reloads the configuration from the file it was originally loaded from.
*/
bool CServerSettings::Reload(void)
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

bool CServerSettings::Load(const char *sXMLFileName)
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
		if(xmlConfig.ToReader("ServerSettings", &xmlEntity))
		{
			this->Initialized = this->Load(&xmlEntity, NULL);
		}
		xmlEntity.Destroy();
		xmlConfig.Destroy();
	}

	return this->Locks.UnlockExclusive(this->Initialized);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CServerSettings::Load(XMLReader *xmlConfig, CServerSettings *pDefaults)
{
	this->Locks.LockExclusive();
	if(this->Initialized)
	{
		this->Destroy();
	}

	int iLength = 0;
	memset(&this->Settings, 0, sizeof(this->Settings));

	XMLReader xml;

	XMLReader xmlUsers;
	if(xmlConfig->ToReader("Users", &xmlUsers))
	{
		this->Settings.Users.AllowRemoteManagement = xmlUsers.ToBoolean("AllowRemoteManagement");

		xmlUsers.ProgressiveScan(true);
		while(xmlUsers.ToReader("User", &xml))
		{
			char sUsername[SERVERS_MAX_USERNAME_LENGTH];
			char sPassword[SERVERS_MAX_PASSWORD_LENGTH];
			char sDescription[SERVERS_MAX_DESCR_LENGTH];

			this->Settings.Users.Items = (SERVERUSER *)
				pMem->ReAllocate(this->Settings.Users.Items, sizeof(SERVERUSER), this->Settings.Users.Count + 1);

			SERVERUSER *p = &this->Settings.Users.Items[this->Settings.Users.Count++];

			xml.ToString("Username", sUsername, sizeof(sUsername), &iLength);
			p->Username = (char *) pMem->StrDup(sUsername);

			xml.ToString("Password", sPassword, sizeof(sPassword), &iLength);
			//For backwards compatibility, we need to hash the passwords in the
			//	file - because thats what they look like goring forward from v1.0.3.8.
			if(iLength != 40)
			{
				char sSHA1[48];
				SimpleSHA1(sPassword, sSHA1, sizeof(sSHA1));
				strcpy_s(sPassword, sizeof(sPassword), sSHA1);
			}
			p->Password = (char *) pMem->StrDup(sPassword);

			xml.ToString("Description", sDescription, sizeof(sDescription), &iLength);
			p->Description = (char *) pMem->StrDup(sDescription);

			p->Enabled = xml.ToBoolean("Enable", true);

			xml.Destroy();
		}
		xmlUsers.Destroy();
	}

	XMLReader xmlProcessors;
	if(xmlConfig->ToReader("Processors", &xmlProcessors))
	{
#if defined(WIN64) || defined(_WIN64) || defined(_AMD64)
		this->Settings.Processor.AffinityMask = xmlProcessors.ToU64("AffinityMask");
#else
		this->Settings.Processor.AffinityMask = xmlProcessors.ToUInteger("AffinityMask");
#endif
		this->Settings.Processor.ProcessBoost = xmlProcessors.ToBoolean("ProcessBoost");
		this->Settings.Processor.ThreadBoost = xmlProcessors.ToBoolean("ThreadBoost");

		if(this->Settings.Processor.AffinityMask != 0)  //Initially (after setup), the affinity mask will be 0.
		{
			SetProcessAffinityMask(GetCurrentProcess(), this->Settings.Processor.AffinityMask);
		}
		SetProcessPriorityBoost(GetCurrentProcess(), !this->Settings.Processor.ProcessBoost);

		xmlProcessors.Destroy();
	}

	XMLReader xmlAdvanced;
	if(xmlConfig->ToReader("Advanced", &xmlAdvanced))
	{
		this->Settings.Advanced.OptimizeForThroughput = xmlAdvanced.ToBoolean("OptimizeForThroughput");
		this->Settings.Advanced.ForceSocketReuse = xmlAdvanced.ToBoolean("ForceSocketReuse");

		xmlAdvanced.Destroy();
	}

	return this->Locks.UnlockExclusive(this->Initialized = true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CServerSettings::Destroy(void)
{
	this->Locks.LockExclusive();

	if(this->Initialized)
	{
		this->Initialized = false;
		if(this->Settings.Users.Count > 0)
		{
			for(int iPos = 0; iPos < this->Settings.Users.Count; iPos++)
			{
				pMem->Free(this->Settings.Users.Items[iPos].Username);
				pMem->Free(this->Settings.Users.Items[iPos].Password);
				pMem->Free(this->Settings.Users.Items[iPos].Description);
			}

			pMem->Free(this->Settings.Users.Items);

			this->Settings.Users.Count = 0;
		}
	}

	memset(&this->Settings, 0, sizeof(this->Settings));

	return this->Locks.UnlockExclusive(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns true if the login passes, otherwise false.
*/
bool CServerSettings::CheckLogin(const char *sUsername, const char *sPassword)
{
	this->Locks.LockShared();
	if(this->Settings.Users.Count > 0)
	{
		for(int iPos = 0; iPos < this->Settings.Users.Count; iPos++)
		{
			if(this->Settings.Users.Items[iPos].Enabled)
			{
				if(_strcmpi(this->Settings.Users.Items[iPos].Username, sUsername) == 0
					&& strcmp(this->Settings.Users.Items[iPos].Password, sPassword) == 0)
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
