///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CWebsiteSettings_CPP
#define _CWebsiteSettings_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern HIMAGELIST hEnableDisableImageList; //Declared in MainDialog.cpp
extern HIMAGELIST hOnePixilImageList; //Declared in MainDialog.cpp

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CWebsiteSettings.H"
#include "Entry.H"
#include "CWebSites.H"

#include "../../@Common/Cryptography.h"
#include "../../../NASCCL/NASCCL.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::File;
using namespace NSWFL::ListView;
using namespace NSWFL::Conversion;
using namespace NSWFL::Windows;
using namespace NSWFL::Conversion;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CWebsiteSettings::~CWebsiteSettings(void)
{
	this->Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CWebsiteSettings::CWebsiteSettings(void *lpWebSites)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CWebsiteSettings::CWebsiteSettings(void *lpWebSites, XMLReader *xmlConfig, CWebsiteSettings *pDefaults)
{
	this->Initialized = true;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
	this->Load(xmlConfig, pDefaults);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWebsiteSettings::Save(void)
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

bool CWebsiteSettings::ToXML(XMLReader *lpXML)
{
	this->Locks.LockShared();

	XMLWriter xmlConfig("WebsiteSettings");

	xmlConfig.AddBool("AllowDirectoryIndexing", this->Collection.AllowDirectoryIndexing);
	xmlConfig.AddBool("CacheLogFile", this->Collection.CacheLogFile);
	xmlConfig.AddBool("PreventImageLeaching", this->Collection.PreventImageLeaching);
	xmlConfig.AddBool("AutoStart", this->Collection.AutoStart);
	xmlConfig.Add("KeepAliveTimeout", this->Collection.KeepAliveTimeout);
	xmlConfig.Add("ScriptTimeout", this->Collection.ScriptTimeout);
	xmlConfig.Add("LogPath", this->Collection.LogPath);
	xmlConfig.Add("ServerAdminEmail", this->Collection.ServerAdminEmail);
	xmlConfig.Add("MaxReceiveSize", this->Collection.MaxReceiveSize);
	xmlConfig.Add("Username", this->Collection.Username);
	xmlConfig.Add("Domain", this->Collection.Domain);

	int iPasswordLength = (int)strlen(this->Collection.Password);
	char sEncryptedPassword[VIRTUALROOTS_MAX_PASS_LENGTH];
	strcpy_s(sEncryptedPassword, sizeof(sEncryptedPassword), this->Collection.Password);

	NASCCLStream nasccl;
	char sEncryptionKey[48];
	MakeEncryptionKey(sEncryptionKey, sizeof(sEncryptionKey), "%s\\%s", this->Collection.Domain, this->Collection.Username);
	nasccl.Initialize(sEncryptionKey);
	nasccl.Cipher(sEncryptedPassword, iPasswordLength);
	nasccl.Destroy();

	char sEncodedPassword[VIRTUALROOTS_MAX_PASS_LENGTH];
	if (Base64Encode((unsigned char *)sEncryptedPassword, iPasswordLength, (unsigned char *)sEncodedPassword, sizeof(sEncodedPassword)) < 0)
	{
		return this->Locks.UnlockShared(false);
	}

	xmlConfig.Add("Password", sEncodedPassword);

	xmlConfig.ToReader(lpXML);
	xmlConfig.Destroy();

	return this->Locks.UnlockShared(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	Reloads the configuration from the file it was originally loaded from.
*/
bool CWebsiteSettings::Reload(void)
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

bool CWebsiteSettings::Load(const char *sXMLFileName)
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
		if(xmlConfig.ToReader("WebsiteSettings", &xmlEntity))
		{
			this->Initialized = this->Load(&xmlEntity, NULL);
		}
		xmlEntity.Destroy();
		xmlConfig.Destroy();
	}

	return this->Locks.UnlockExclusive(this->Initialized);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWebsiteSettings::Load(XMLReader *xmlConfig, CWebsiteSettings *pDefaults)
{
	this->Locks.LockExclusive();
	if(this->Initialized)
	{
		this->Destroy();
	}

	memset(&this->Collection, 0, sizeof(this->Collection));

	if(pDefaults)
	{
		this->Collection.AllowDirectoryIndexing = xmlConfig->ToBoolean("AllowDirectoryIndexing", pDefaults->AllowDirectoryIndexing());
		this->Collection.CacheLogFile = xmlConfig->ToBoolean("CacheLogFile", pDefaults->CacheLogFile());
		this->Collection.PreventImageLeaching = xmlConfig->ToBoolean("PreventImageLeaching", pDefaults->PreventImageLeaching());
		this->Collection.AutoStart = xmlConfig->ToBoolean("AutoStart", pDefaults->AutoStart());
		this->Collection.KeepAliveTimeout = xmlConfig->ToInteger("KeepAliveTimeout", pDefaults->KeepAliveTimeout());
		this->Collection.ScriptTimeout = xmlConfig->ToInteger("ScriptTimeout", pDefaults->ScriptTimeout());
		this->Collection.MaxReceiveSize = xmlConfig->ToInteger("MaxReceiveSize", pDefaults->MaxReceiveSize());

		xmlConfig->ToString("LogPath", this->Collection.LogPath, sizeof(this->Collection.LogPath), pDefaults->LogPath());
		xmlConfig->ToString("ServerAdminEmail", this->Collection.ServerAdminEmail, sizeof(this->Collection.ServerAdminEmail), pDefaults->ServerAdminEmail());
		xmlConfig->ToString("Username", this->Collection.Username, sizeof(this->Collection.Username), pDefaults->Username());
		xmlConfig->ToString("Domain", this->Collection.Domain, sizeof(this->Collection.Domain), pDefaults->Domain());

		char sEncryptedPassword[VIRTUALROOTS_MAX_PASS_LENGTH];
		xmlConfig->ToString("Password", sEncryptedPassword, sizeof(sEncryptedPassword));

		int iRawPasswordLength = (int)Base64Decode((unsigned char *)sEncryptedPassword, (int)strlen(sEncryptedPassword), (unsigned char *)this->Collection.Password, (int)sizeof(this->Collection.Password));
		if (iRawPasswordLength > 0)
		{
			NASCCLStream nasccl;
			char sEncryptionKey[48];
			MakeEncryptionKey(sEncryptionKey, sizeof(sEncryptionKey), "%s\\%s", this->Collection.Domain, this->Collection.Username);
			nasccl.Initialize(sEncryptionKey);
			nasccl.Cipher(this->Collection.Password, iRawPasswordLength);
			nasccl.Destroy();
		}
	}
	else{
		this->Collection.AllowDirectoryIndexing = xmlConfig->ToBoolean("AllowDirectoryIndexing", false);
		this->Collection.AutoStart = xmlConfig->ToBoolean("AutoStart", true);
		this->Collection.CacheLogFile = xmlConfig->ToBoolean("CacheLogFile", true);
		this->Collection.PreventImageLeaching = xmlConfig->ToBoolean("PreventImageLeaching", false);
		this->Collection.KeepAliveTimeout = xmlConfig->ToInteger("KeepAliveTimeout", 300);
		this->Collection.ScriptTimeout = xmlConfig->ToInteger("ScriptTimeout", 600);
		this->Collection.MaxReceiveSize = xmlConfig->ToInteger("MaxReceiveSize", 2097152);

		xmlConfig->ToString("LogPath", this->Collection.LogPath, sizeof(this->Collection.LogPath));
		xmlConfig->ToString("ServerAdminEmail", this->Collection.ServerAdminEmail, sizeof(this->Collection.ServerAdminEmail));
		xmlConfig->ToString("Username", this->Collection.Username, sizeof(this->Collection.Username));
		xmlConfig->ToString("Domain", this->Collection.Domain, sizeof(this->Collection.Domain));

		char sEncryptedPassword[VIRTUALROOTS_MAX_PASS_LENGTH];
		xmlConfig->ToString("Password", sEncryptedPassword, sizeof(sEncryptedPassword));

		int iRawPasswordLength = (int)Base64Decode((unsigned char *)sEncryptedPassword, (int)strlen(sEncryptedPassword), (unsigned char *)this->Collection.Password, (int)sizeof(this->Collection.Password));
		if (iRawPasswordLength > 0)
		{
			NASCCLStream nasccl;
			char sEncryptionKey[48];
			MakeEncryptionKey(sEncryptionKey, sizeof(sEncryptionKey), "%s\\%s", this->Collection.Domain, this->Collection.Username);
			nasccl.Initialize(sEncryptionKey);
			nasccl.Cipher(this->Collection.Password, iRawPasswordLength);
			nasccl.Destroy();
		}

	}

	return this->Locks.UnlockExclusive(this->Initialized = true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *CWebsiteSettings::LogPath(void)
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->Collection.LogPath);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CWebsiteSettings::ScriptTimeout(void)
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->Collection.ScriptTimeout);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWebsiteSettings::PreventImageLeaching(void)
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->Collection.PreventImageLeaching);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWebsiteSettings::CacheLogFile(void)
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->Collection.CacheLogFile);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWebsiteSettings::AutoStart(void)
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->Collection.AutoStart);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *CWebsiteSettings::Username(void)
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->Collection.Username);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *CWebsiteSettings::Domain(void)
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->Collection.Domain);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *CWebsiteSettings::Password()
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->Collection.Password);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CWebsiteSettings::MaxReceiveSize(void)
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->Collection.MaxReceiveSize);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *CWebsiteSettings::ServerAdminEmail(void)
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->Collection.ServerAdminEmail);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWebsiteSettings::AllowDirectoryIndexing(void)
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->Collection.AllowDirectoryIndexing);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CWebsiteSettings::KeepAliveTimeout(void)
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->Collection.KeepAliveTimeout);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWebsiteSettings::Destroy(void)
{
	this->Locks.LockExclusive();

	if(this->Initialized)
	{
		this->Initialized = false;
		memset(&this->Collection, 0, sizeof(this->Collection));
	}
	return this->Locks.UnlockExclusive(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
