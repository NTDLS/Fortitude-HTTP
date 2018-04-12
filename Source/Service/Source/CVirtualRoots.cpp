///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CVirtualRoots_CPP
#define _CVirtualRoots_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <shlobj.h>

extern HIMAGELIST hEnableDisableImageList; //Declared in MainDialog.cpp
extern HIMAGELIST hOnePixilImageList; //Declared in MainDialog.cpp

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CVirtualRoots.H"
#include "Entry.H"
#include "CWebSites.H"
#include "CWebSite.H"

#include "../../@Common/Cryptography.h"
#include "../../../NASCCL/NASCCL.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::File;
using namespace NSWFL::ListView;
using namespace NSWFL::Conversion;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CVirtualRoots::~CVirtualRoots(void)
{
	this->Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CVirtualRoots::CVirtualRoots(void *lpWebSites)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CVirtualRoots::CVirtualRoots(void *lpWebSites, XMLReader *xmlConfig, CVirtualRoots *pDefaults)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
	this->Load(xmlConfig, pDefaults);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CVirtualRoots::Save(void)
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

bool CVirtualRoots::ToXML(XMLReader *lpXML)
{
	this->Locks.LockShared();

	XMLWriter xmlConfig("VirtualRoots");

	xmlConfig.AddBool("Enable", this->Collection.Enabled);

	for(int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		XMLWriter Item("Root");
		Item.Add("URI", this->Collection.Items[iItem].Root);
		Item.Add("Path", this->Collection.Items[iItem].Path);
		Item.Add("Description", this->Collection.Items[iItem].Description);
		Item.AddBool("Enable", this->Collection.Items[iItem].Enabled);

		if(this->Collection.Items[iItem].Username && strlen(this->Collection.Items[iItem].Username) > 0)
		{
			XMLWriter xmlAuth("Login");
			xmlAuth.Add("Username", this->Collection.Items[iItem].Username);
			xmlAuth.Add("Domain", this->Collection.Items[iItem].Domain);

			char sEncodedPassword[VIRTUALROOTS_MAX_PASS_LENGTH];
			memset(sEncodedPassword, 0, sizeof(sEncodedPassword));
			if (this->Collection.Items[iItem].Password)
			{
				char sEncryptedPassword[VIRTUALROOTS_MAX_PASS_LENGTH];
				int iPasswordLength = (int)strlen(this->Collection.Items[iItem].Password);
				strcpy_s(sEncryptedPassword, sizeof(sEncryptedPassword), this->Collection.Items[iItem].Password);

				NASCCLStream nasccl;
				char sEncryptionKey[48];
				MakeEncryptionKey(sEncryptionKey, sizeof(sEncryptionKey), "%s\\%s", this->Collection.Items[iItem].Domain, this->Collection.Items[iItem].Username);
				nasccl.Initialize(sEncryptionKey);
				nasccl.Cipher(sEncryptedPassword, iPasswordLength);
				nasccl.Destroy();

				if (Base64Encode((unsigned char *)sEncryptedPassword, iPasswordLength, (unsigned char *)sEncodedPassword, sizeof(sEncodedPassword)) < 0)
				{
					return this->Locks.UnlockShared(false);
				}
			}

			xmlAuth.Add("Password", sEncodedPassword);
			Item.Add(&xmlAuth);
		}

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
bool CVirtualRoots::Reload(void)
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

bool CVirtualRoots::Load(const char *sXMLFileName)
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
		if(xmlConfig.ToReader("VirtualRoots", &xmlEntity))
		{
			this->Initialized = this->Load(&xmlEntity, NULL);
		}
		xmlEntity.Destroy();
		xmlConfig.Destroy();
	}

	return this->Locks.UnlockExclusive(this->Initialized);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CVirtualRoots::Load(XMLReader *xmlConfig, CVirtualRoots *pDefaults)
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
	XMLReader XPVirtualRoot;
	XMLReader xmlAuth;

	bool bDoesBuiltInErrorsExist = false;

	while(xmlConfig->ToReader("Root", &XPVirtualRoot))
	{
		char sURI[VIRTUALROOTS_MAX_ROOT_LENGTH];
		char sPath[VIRTUALROOTS_MAX_PATH_LENGTH];
		char sDescr[VIRTUALROOTS_MAX_DESCR_LENGTH];
		char sUser[VIRTUALROOTS_MAX_USER_LENGTH];
		char sDomain[VIRTUALROOTS_MAX_DOMAIN_LENGTH];
		char sEncryptedPassword[VIRTUALROOTS_MAX_PASS_LENGTH];
		char sPassword[VIRTUALROOTS_MAX_PASS_LENGTH];

		this->Collection.Items = (ROOT *) pMem->ReAllocate(
			this->Collection.Items, sizeof(ROOT), this->Collection.Count + 1);

		ROOT *p = &this->Collection.Items[this->Collection.Count++];

		XPVirtualRoot.ToString("URI", sURI, sizeof(sURI), &iLength);
		CorrectForwardPath(sURI, sizeof(sURI));
		if(sURI[iLength - 1] != '/')
		{
			strcat_s(sURI, sizeof(sURI), "/");
		}
		if(sURI[0] != '/')
		{
			char sTmp[sizeof(sURI)];
			sprintf_s(sTmp, sizeof(sTmp), "/%s", sURI);
			strcpy_s(sURI, sizeof(sURI), sTmp);
		}

		bDoesBuiltInErrorsExist = bDoesBuiltInErrorsExist || (InStrI("/BuiltInErrors", sURI) >= 0);

		p->Root = (char *) pMem->CloneString(sURI);
		p->RootLength = (int) strlen(sURI);

		XPVirtualRoot.ToString("Path", sPath, sizeof(sPath), &iLength);
		p->Path = (char *) pMem->CloneString(sPath);
		p->PathLength = (int) strlen(sPath);

		XPVirtualRoot.ToString("Description", sDescr, sizeof(sDescr), &iLength);
		p->Description = (char *) pMem->CloneString(sDescr);

		p->Enabled = XPVirtualRoot.ToBoolean("Enable", false);
		
		if(XPVirtualRoot.ToReader("Login", &xmlAuth))
		{
			xmlAuth.ToString("Username", sUser, sizeof(sUser), &iLength);
			p->Username = (char *) pMem->CloneString(sUser);

			xmlAuth.ToString("Domain", sDomain, sizeof(sDomain), &iLength);
			p->Domain = (char *) pMem->CloneString(sDomain);
	
			xmlAuth.ToString("Password", sEncryptedPassword, sizeof(sEncryptedPassword), &iLength);

			int iRawPasswordLength = (int)Base64Decode((unsigned char *)sEncryptedPassword, (int)strlen(sEncryptedPassword), (unsigned char *)sPassword, (int)sizeof(sPassword));
			if (iRawPasswordLength > 0)
			{
				NASCCLStream nasccl;
				char sEncryptionKey[48];
				MakeEncryptionKey(sEncryptionKey, sizeof(sEncryptionKey), "%s\\%s", sDomain, sUser);
				nasccl.Initialize(sEncryptionKey);
				nasccl.Cipher(sPassword, iRawPasswordLength);
				nasccl.Destroy();
				p->Password = (char *)pMem->CloneString(sPassword);
			}
			else
			{
				p->Password = NULL;
			}
		}
		else{
			p->Username = NULL;
			p->Domain = NULL;
			p->Password = NULL;
		}

		xmlAuth.Destroy();

		XPVirtualRoot.Destroy();
	}

	//Version 1.0.3.8 and above require a custom error pages to use relative paths, so for
	//	now we need to force the existence of a virtual root until all users have upgraded.
	if(!bDoesBuiltInErrorsExist)
	{
		this->Collection.Items = (ROOT *) pMem->ReAllocate(
			this->Collection.Items, sizeof(ROOT), this->Collection.Count + 1);

		ROOT *p = &this->Collection.Items[this->Collection.Count++];

		char sPath[VIRTUALROOTS_MAX_PATH_LENGTH];
		sprintf_s(sPath, sizeof(sPath), "%s\\Messages", gsPath);
		CorrectForwardPath(sPath, sizeof(sPath));

		p->Root = (char *) pMem->CloneString("/BuiltInErrors/");
		p->RootLength = (int) strlen(p->Root);
		p->Path = (char *) pMem->CloneString(sPath);
		p->PathLength = (int) strlen(sPath);
		p->Description = (char *) pMem->CloneString("Used built-in global error messages. You must reconfigure your error messages if this virtual root is removed or is not present under each web-site.");
		p->Enabled = true;
		p->Username = NULL;
		p->Domain = NULL;
		p->Password = NULL;
	}

	return this->Locks.UnlockExclusive(this->Initialized = true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CVirtualRoots::Destroy(void)
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
				pMem->Free(this->Collection.Items[iPos].Root);
				pMem->Free(this->Collection.Items[iPos].Description);
				if(this->Collection.Items[iPos].Username)
				{
					pMem->Free(this->Collection.Items[iPos].Username);
				}
				if(this->Collection.Items[iPos].Domain)
				{
					pMem->Free(this->Collection.Items[iPos].Domain);
				}
				if(this->Collection.Items[iPos].Password)
				{
					pMem->Free(this->Collection.Items[iPos].Password);
				}
				this->Collection.Items[iPos].RootLength = 0;
			}

			pMem->Free(this->Collection.Items);

			this->Collection.Count = 0;
		}
	}
	return this->Locks.UnlockExclusive(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns a pointer to a virtual root if the request is a virtual root otherwise returns NULL.
*/
ROOT *CVirtualRoots::IsVirtualRoot(const char *sRequest/*, char *&sFullRequest*/)
{
	this->Locks.LockShared();

	if(this->Collection.Enabled && (this->Collection.Count > 0))
	{
		int isRequestSize = (int)strlen(sRequest);

		for(int iPos = 0; iPos < this->Collection.Count; iPos++)
		{
			if(this->Collection.Items[iPos].Enabled)
			{
				if(InStrI(this->Collection.Items[iPos].Root,
					this->Collection.Items[iPos].RootLength - 1, sRequest, isRequestSize, 0) == 0)
				{
					//Make sure that this the virtual root string isn't just contained in
					//	another path, but is actually terminated along proper boundaries.
					if(sRequest[this->Collection.Items[iPos].RootLength - 1] == '/'
						|| sRequest[this->Collection.Items[iPos].RootLength - 1] == '\\'
						|| sRequest[this->Collection.Items[iPos].RootLength - 1] == '\0')
					{
						/*
						int iAlloc = this->Collection.Items[iPos].PathLength + 2;
						if(isRequestSize > this->Collection.Items[iPos].RootLength)
						{
							iAlloc += isRequestSize - this->Collection.Items[iPos].RootLength;
						}

						sFullRequest = (char *) pMem->ReAllocate(sFullRequest, sizeof(char), iAlloc);

						strcpy_s(sFullRequest, iAlloc, this->Collection.Items[iPos].Path);
						if(isRequestSize > this->Collection.Items[iPos].RootLength)
						{
							strcat_s(sFullRequest, iAlloc, "\\");
							strcat_s(sFullRequest, iAlloc, sRequest + this->Collection.Items[iPos].RootLength);
						}

						CorrectReversePath(sFullRequest, iAlloc, true);
						*/

						return (ROOT *)this->Locks.UnlockShared((void *)&this->Collection.Items[iPos]);
					}
				}
			}
		}
	}
	return (ROOT *)this->Locks.UnlockShared((void *)NULL);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
