///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CWEBSITE_CPP
#define _CWEBSITE_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <StdLib.H>
#include <Share.H>
#include <shlobj.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"

#include "CWebSite.H"
#include "CWebSites.H"
#include "CHttp.H"

#include "SocketHandlers.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern HIMAGELIST hEnableDisableImageList; //Declared in MainDialog.cpp
extern HIMAGELIST hOnePixilImageList; //Declared in MainDialog.cpp

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Conversion;
using namespace NSWFL::File;
using namespace NSWFL::DateTime;
using namespace NSWFL::ListView;
using namespace NSWFL::String;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CWebSite::CWebSite()
{
	this->Initialize();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CWebSite::Initialize(void)
{
	this->IsStarted = false;

	memset(this->sLogCycleDate, 0, sizeof(this->sLogCycleDate));
	memset(&this->csLogFile, 0, sizeof(this->csLogFile));
	memset(&this->Stats, 0, sizeof(this->Stats));

	this->pSocketPool = NULL;
	this->hLogFile = NULL;
	this->pHttp = NULL;
	this->pWebSites = NULL;
	this->pHostHeaders = NULL;
	this->pMimeTypes = NULL;
	this->pExtensionFilters = NULL;
	this->pURLFilters = NULL;
	this->pDefaultPages = NULL;
	this->pVirtualRoots = NULL;
	this->pSSIFiles = NULL;
	this->pIPFilters = NULL;
	this->pCGIFolders = NULL;
	this->pCompression = NULL;
	this->pCustomFilters = NULL;
	this->pScriptingEngines = NULL;
	this->pErrorPages = NULL;
	this->pWebsiteSettings = NULL;
	this->pAuthentication = NULL;
	this->pRealms = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CWebSite::CWebSite(void *lpWebSites, XMLReader *pXMLSite)
{
	this->Initialize();

	this->pWebSites = lpWebSites;

	InitializeCriticalSection(&this->csLogFile);

	XMLReader xmlConfig;

	this->Reload(pXMLSite);

	//----------------------------------------------------------------------
	if(pXMLSite->ToReader("WebsiteSettings", &xmlConfig))
	{
		this->pWebsiteSettings = (CWebsiteSettings *) pMem->TrackMemory(new CWebsiteSettings(this->pWebSites, &xmlConfig, ((CWebSites *)pWebSites)->DfltWebsiteSettings));
		xmlConfig.Destroy();
	}
	else{
		this->pWebsiteSettings = ((CWebSites *)pWebSites)->DfltWebsiteSettings;
	}
	//----------------------------------------------------------------------
	if(pXMLSite->ToReader("CGIFolders", &xmlConfig))
	{
		this->pCGIFolders = (CCGIFolders *) pMem->TrackMemory(new CCGIFolders(this->pWebSites, &xmlConfig, ((CWebSites *)pWebSites)->DfltCGIFolders));
		xmlConfig.Destroy();
	}
	else{
		this->pCGIFolders = ((CWebSites *)pWebSites)->DfltCGIFolders;
	}
	//----------------------------------------------------------------------
	if(pXMLSite->ToReader("Compression", &xmlConfig))
	{
		this->pCompression = (CCompression *) pMem->TrackMemory(new CCompression(this->pWebSites, &xmlConfig, ((CWebSites *)pWebSites)->DfltCompression));
		xmlConfig.Destroy();
	}
	else{
		this->pCompression = ((CWebSites *)pWebSites)->DfltCompression;
	}
	//----------------------------------------------------------------------
	if(pXMLSite->ToReader("CustomFilters", &xmlConfig))
	{
		this->pCustomFilters = (CCustomFilters *) pMem->TrackMemory(new CCustomFilters(this->pWebSites, &xmlConfig, ((CWebSites *)pWebSites)->DfltCustomFilters));
		xmlConfig.Destroy();
	}
	else{
		this->pCustomFilters = ((CWebSites *)pWebSites)->DfltCustomFilters;
	}
	//----------------------------------------------------------------------
	if(pXMLSite->ToReader("DefaultPages", &xmlConfig))
	{
		this->pDefaultPages = (CDefaultPages *) pMem->TrackMemory(new CDefaultPages(this->pWebSites, &xmlConfig, ((CWebSites *)pWebSites)->DfltDefaultPages));
		xmlConfig.Destroy();
	}
	else{
		this->pDefaultPages = ((CWebSites *)pWebSites)->DfltDefaultPages;
	}
	//----------------------------------------------------------------------
	if(pXMLSite->ToReader("ExtensionFilters", &xmlConfig))
	{
		this->pExtensionFilters = (CExtensionFilters *) pMem->TrackMemory(new CExtensionFilters(this->pWebSites, &xmlConfig, ((CWebSites *)pWebSites)->DfltExtensionFilters));
		xmlConfig.Destroy();
	}
	else{
		this->pExtensionFilters = ((CWebSites *)pWebSites)->DfltExtensionFilters;
	}
	//----------------------------------------------------------------------
	if(pXMLSite->ToReader("IPFilters", &xmlConfig))
	{
		this->pIPFilters = (CIPFilters *) pMem->TrackMemory(new CIPFilters(this->pWebSites, &xmlConfig, ((CWebSites *)pWebSites)->DfltIPFilters));
		xmlConfig.Destroy();
	}
	else{
		this->pIPFilters = ((CWebSites *)pWebSites)->DfltIPFilters;
	}
	//----------------------------------------------------------------------
	if(pXMLSite->ToReader("MimeTypes", &xmlConfig))
	{
		this->pMimeTypes = (CMimeTypes *) pMem->TrackMemory(new CMimeTypes(this->pWebSites, &xmlConfig, ((CWebSites *)pWebSites)->DfltMimeTypes));
		xmlConfig.Destroy();
	}
	else{
		this->pMimeTypes = ((CWebSites *)pWebSites)->DfltMimeTypes;
	}
	//----------------------------------------------------------------------
	if(pXMLSite->ToReader("ScriptingEngines", &xmlConfig))
	{
		this->pScriptingEngines = (CScriptingEngines *) pMem->TrackMemory(new CScriptingEngines(this->pWebSites, &xmlConfig, ((CWebSites *)pWebSites)->DfltScriptingEngines));
		xmlConfig.Destroy();
	}
	else{
		this->pScriptingEngines = ((CWebSites *)pWebSites)->DfltScriptingEngines;
	}
	//----------------------------------------------------------------------
	if(pXMLSite->ToReader("Realms", &xmlConfig))
	{
		this->pRealms = (CRealms *) pMem->TrackMemory(new CRealms(this->pWebSites, &xmlConfig, ((CWebSites *)pWebSites)->DfltRealms));
		xmlConfig.Destroy();
	}
	else{
		this->pRealms = ((CWebSites *)pWebSites)->DfltRealms;
	}
	//----------------------------------------------------------------------
	if(pXMLSite->ToReader("Authentication", &xmlConfig))
	{
		this->pAuthentication = (CAuthentication *) pMem->TrackMemory(new CAuthentication(this->pWebSites, &xmlConfig, ((CWebSites *)pWebSites)->DfltAuthentication));
		xmlConfig.Destroy();
	}
	else{
		this->pAuthentication = ((CWebSites *)pWebSites)->DfltAuthentication;
	}
	//----------------------------------------------------------------------
	if(pXMLSite->ToReader("SSIFiles", &xmlConfig))
	{
		this->pSSIFiles = (CSSIFiles *) pMem->TrackMemory(new CSSIFiles(this->pWebSites, &xmlConfig, ((CWebSites *)pWebSites)->DfltSSIFiles));
		xmlConfig.Destroy();
	}
	else{
		this->pSSIFiles = ((CWebSites *)pWebSites)->DfltSSIFiles;
	}
	//----------------------------------------------------------------------
	if(pXMLSite->ToReader("URLFilters", &xmlConfig))
	{
		this->pURLFilters = (CURLFilters *) pMem->TrackMemory(new CURLFilters(this->pWebSites, &xmlConfig, ((CWebSites *)pWebSites)->DfltURLFilters));
		xmlConfig.Destroy();
	}
	else{
		this->pURLFilters = ((CWebSites *)pWebSites)->DfltURLFilters;
	}
	//----------------------------------------------------------------------
	if(pXMLSite->ToReader("VirtualRoots", &xmlConfig))
	{
		this->pVirtualRoots = (CVirtualRoots *) pMem->TrackMemory(new CVirtualRoots(this->pWebSites, &xmlConfig, ((CWebSites *)pWebSites)->DfltVirtualRoots));
		xmlConfig.Destroy();
	}
	else{
		this->pVirtualRoots = ((CWebSites *)pWebSites)->DfltVirtualRoots;
	}
	//----------------------------------------------------------------------
	if(pXMLSite->ToReader("ErrorPages", &xmlConfig))
	{
		this->pErrorPages = (CErrorPages *) pMem->TrackMemory(new CErrorPages(this->pWebSites, &xmlConfig, ((CWebSites *)pWebSites)->DfltErrorPages));
		xmlConfig.Destroy();
	}
	else{
		this->pErrorPages = ((CWebSites *)pWebSites)->DfltErrorPages;
	}
	//----------------------------------------------------------------------

	if(this->CycleLog())
	{
		if(this->pSocketPool && this->pWebsiteSettings->AutoStart())
		{
			this->Start();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CWebSite::BindSocketPool(void)
{
	this->pSocketPool = ((CWebSites *)this->pWebSites)->pSocketPools->Pool(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function loads the base site configuration (name, description, host headers etc)
		but does not load the sub items (thse are handled seperatly).
	If the site has already been loaded, then these settings will be reloaded.
*/
bool CWebSite::Reload(XMLReader *xml)
{
	bool bIsSiteActive = this->IsActive();
	if(bIsSiteActive)
	{
		this->Stop();
	}

	if(this->pHostHeaders)
	{
		delete (CHostHeaders *)pMem->UnTrackMemory(this->pHostHeaders);
	}
	
	xml->ToString("Name", this->Name, sizeof(this->Name));
	xml->ToString("Description", this->Description, sizeof(this->Description));
	xml->ToString("Root", this->Root, sizeof(this->Root));
	xml->ToString("SocketPoolID", this->SocketPoolID, sizeof(this->SocketPoolID));
	CorrectReversePath(this->Root, sizeof(this->Root), true);

	this->BindSocketPool();

	XMLReader xmlConfig;
	//----------------------------------------------------------------------
	if(xml->ToReader("HostHeaders", &xmlConfig))
	{
		this->pHostHeaders = (CHostHeaders *) pMem->TrackMemory(new CHostHeaders(this->pWebSites, &xmlConfig));
		xmlConfig.Destroy();
	}
	else {
		this->pHostHeaders = (CHostHeaders *) pMem->TrackMemory(new CHostHeaders(this->pWebSites));
	}

	if(bIsSiteActive)
	{
		this->Start();
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CWebSite::~CWebSite()
{
	CWebSites *pWebSites = ((CWebSites *)this->pWebSites);
	if(pWebSites)
	{
		this->Stop();

		if(this->pCGIFolders && this->pCGIFolders != pWebSites->DfltCGIFolders)
			delete (CCGIFolders *)pMem->UnTrackMemory(this->pCGIFolders);
		if(this->pCompression && this->pCompression != pWebSites->DfltCompression)
			delete (CCompression *)pMem->UnTrackMemory(this->pCompression);
		if(this->pCustomFilters && this->pCustomFilters != pWebSites->DfltCustomFilters) 
			delete (CCustomFilters *)pMem->UnTrackMemory(this->pCustomFilters);
		if(this->pDefaultPages && this->pDefaultPages != pWebSites->DfltDefaultPages)
			delete (CDefaultPages *)pMem->UnTrackMemory(this->pDefaultPages);
		if(this->pExtensionFilters && this->pExtensionFilters != pWebSites->DfltExtensionFilters)
			delete (CExtensionFilters *)pMem->UnTrackMemory(this->pExtensionFilters);
		if(this->pIPFilters && this->pIPFilters != pWebSites->DfltIPFilters)
			delete (CIPFilters *)pMem->UnTrackMemory(this->pIPFilters);
		if(this->pMimeTypes && this->pMimeTypes != pWebSites->DfltMimeTypes)
			delete (CMimeTypes *)pMem->UnTrackMemory(this->pMimeTypes);
		if(this->pScriptingEngines && this->pScriptingEngines != pWebSites->DfltScriptingEngines)
			delete (CScriptingEngines *)pMem->UnTrackMemory(this->pScriptingEngines);
		if(this->pRealms && this->pRealms != pWebSites->DfltRealms)
			delete (CRealms *)pMem->UnTrackMemory(this->pRealms);
		if(this->pAuthentication && this->pAuthentication != pWebSites->DfltAuthentication)
			delete (CAuthentication *)pMem->UnTrackMemory(this->pAuthentication);
		if(this->pWebsiteSettings && this->pWebsiteSettings != pWebSites->DfltWebsiteSettings)
			delete (CWebsiteSettings *)pMem->UnTrackMemory(this->pWebsiteSettings);
		if(this->pSSIFiles && this->pSSIFiles != pWebSites->DfltSSIFiles)
			delete (CSSIFiles *)pMem->UnTrackMemory(this->pSSIFiles);
		if(this->pURLFilters && this->pURLFilters != pWebSites->DfltURLFilters)
			delete (CURLFilters *)pMem->UnTrackMemory(this->pURLFilters);
		if(this->pVirtualRoots && this->pVirtualRoots != pWebSites->DfltVirtualRoots)
			delete (CVirtualRoots *)pMem->UnTrackMemory(this->pVirtualRoots);
		if(this->pErrorPages && this->pErrorPages != pWebSites->DfltErrorPages)
			delete (CErrorPages *)pMem->UnTrackMemory(this->pErrorPages);
		if(this->pHostHeaders)
			delete (CHostHeaders *)pMem->UnTrackMemory(this->pHostHeaders);

		this->CloseLog();
		DeleteCriticalSection(&this->csLogFile);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWebSite::ToXML(XMLWriter *pXMLSite)
{
	return this->ToXML(pXMLSite, false, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWebSite::Bindings(XMLWriter *pXMLSite)
{
	if(pXMLSite->Initialize("Site"))
	{
		pXMLSite->Add("Index", this->Index());
		pXMLSite->Add("IsActive", this->IsActive());

		pXMLSite->Add("Port", this->pSocketPool->Port);

		if(this->pHostHeaders->ActiveCount() > 0)
		{
			for(int i = 0; i < this->pHostHeaders->Collection.Count; i++)
			{
				if(this->pHostHeaders->Collection.Items[i].Enabled)
				{
					StringBuilder binding;

					if(this->pSocketPool->Port == 80)
					{
						binding.AppendF("http://%s/", this->pHostHeaders->Collection.Items[i].Name);
					}
					else {
						binding.AppendF("http://%s:%d/", this->pHostHeaders->Collection.Items[i].Name, this->pSocketPool->Port);
					}

					pXMLSite->Add("Binding", binding.Buffer);
				}
			}
		}
		else if(this->pSocketPool->ListenIPs.ListenOnAll)
		{
			StringBuilder binding;

			if(this->pSocketPool->Port == 80)
			{
				binding.AppendF("http://localhost/");
			}
			else {
				binding.AppendF("http://localhost:%d/", this->pSocketPool->Port);
			}

			pXMLSite->Add("Binding", binding.Buffer);
		}
		else {
			for(int i = 0; i < this->pSocketPool->ListenIPs.Count; i++)
			{
				if(this->pSocketPool->ListenIPs.Items[i].Enabled)
				{
					StringBuilder binding;

					if(this->pSocketPool->Port == 80)
					{
						binding.AppendF("http://%s/", this->pSocketPool->ListenIPs.Items[i].IP);
					}
					else {
						binding.AppendF("http://%s:%d/", this->pSocketPool->ListenIPs.Items[i].IP, this->pSocketPool->Port);
					}

					pXMLSite->Add("Binding", binding.Buffer);
				}
			}
		}

		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWebSite::Overview(XMLWriter *pXMLSite)
{
	if(pXMLSite->Initialize("Site"))
	{
		pXMLSite->Add("Index", this->Index());
		pXMLSite->Add("IsActive", this->IsActive());
		pXMLSite->Add("Name", this->Name);
		pXMLSite->Add("Description", this->Description);
		pXMLSite->Add("Root", this->Root);
		pXMLSite->Add("SocketPoolID", this->SocketPoolID);

		if(this->pSocketPool)
		{
			pXMLSite->Add("Port", this->pSocketPool->Port);
			pXMLSite->Add("MaxConnections", this->pSocketPool->MaxConnections);
		}

		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWebSite::ToXML(XMLWriter *pXMLSite, bool bIncludeSubItems, bool bIncludeStatusInfo)
{
	if(pXMLSite->Initialize("Site"))
	{
		if(bIncludeStatusInfo)
		{
			pXMLSite->Add("Index", this->Index());
			pXMLSite->Add("IsActive", this->IsActive());
		}

		pXMLSite->Add("Name", this->Name);
		pXMLSite->Add("Description", this->Description);
		pXMLSite->Add("Root", this->Root);
		pXMLSite->Add("SocketPoolID", this->SocketPoolID);

		XMLReader xmlSub;
		if(this->pHostHeaders->ToXML(&xmlSub))
		{
			pXMLSite->Add(&xmlSub);
			xmlSub.Destroy();
		}

		if(bIncludeSubItems)
		{
			this->SubItemsToXML(pXMLSite);
		}

		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CWebSite::SubItemsToXML(XMLWriter *xmlConfig)
{
	XMLReader xmlSub;
	int iSubItems = 0;

	if(this->pCGIFolders != ((CWebSites *)pWebSites)->DfltCGIFolders)
	{
		if(this->pCGIFolders->ToXML(&xmlSub))
		{
			xmlConfig->Add(&xmlSub);
			xmlSub.Destroy();
			iSubItems++;
		}
	}
	if(this->pWebsiteSettings != ((CWebSites *)pWebSites)->DfltWebsiteSettings)
	{
		if(this->pWebsiteSettings->ToXML(&xmlSub))
		{
			xmlConfig->Add(&xmlSub);
			xmlSub.Destroy();
			iSubItems++;
		}
	}
	if(this->pMimeTypes != ((CWebSites *)pWebSites)->DfltMimeTypes)
	{
		if(this->pMimeTypes->ToXML(&xmlSub))
		{
			xmlConfig->Add(&xmlSub);
			xmlSub.Destroy();
			iSubItems++;
		}
	}
	if(this->pExtensionFilters != ((CWebSites *)pWebSites)->DfltExtensionFilters)
	{
		if(this->pExtensionFilters->ToXML(&xmlSub))
		{
			xmlConfig->Add(&xmlSub);
			xmlSub.Destroy();
			iSubItems++;
		}
	}
	if(this->pURLFilters != ((CWebSites *)pWebSites)->DfltURLFilters)
	{
		if(this->pURLFilters->ToXML(&xmlSub))
		{
			xmlConfig->Add(&xmlSub);
			xmlSub.Destroy();
			iSubItems++;
		}
	}
	if(this->pDefaultPages != ((CWebSites *)pWebSites)->DfltDefaultPages)
	{
		if(this->pDefaultPages->ToXML(&xmlSub))
		{
			xmlConfig->Add(&xmlSub);
			xmlSub.Destroy();
			iSubItems++;
		}
	}
	if(this->pVirtualRoots != ((CWebSites *)pWebSites)->DfltVirtualRoots)
	{
		if(this->pVirtualRoots->ToXML(&xmlSub))
		{
			xmlConfig->Add(&xmlSub);
			xmlSub.Destroy();
			iSubItems++;
		}
	}
	if(this->pSSIFiles != ((CWebSites *)pWebSites)->DfltSSIFiles)
	{
		if(this->pSSIFiles->ToXML(&xmlSub))
		{
			xmlConfig->Add(&xmlSub);
			xmlSub.Destroy();
			iSubItems++;
		}
	}
	if(this->pIPFilters != ((CWebSites *)pWebSites)->DfltIPFilters)
	{
		if(this->pIPFilters->ToXML(&xmlSub))
		{
			xmlConfig->Add(&xmlSub);
			xmlSub.Destroy();
			iSubItems++;
		}
	}
	if(this->pCompression != ((CWebSites *)pWebSites)->DfltCompression)
	{
		if(this->pCompression->ToXML(&xmlSub))
		{
			xmlConfig->Add(&xmlSub);
			xmlSub.Destroy();
			iSubItems++;
		}
	}
	if(this->pCustomFilters != ((CWebSites *)pWebSites)->DfltCustomFilters)
	{
		if(this->pCustomFilters->ToXML(&xmlSub))
		{
			xmlConfig->Add(&xmlSub);
			xmlSub.Destroy();
			iSubItems++;
		}
	}
	if(this->pScriptingEngines != ((CWebSites *)pWebSites)->DfltScriptingEngines)
	{
		if(this->pScriptingEngines->ToXML(&xmlSub))
		{
			xmlConfig->Add(&xmlSub);
			xmlSub.Destroy();
			iSubItems++;
		}
	}
	if(this->pRealms != ((CWebSites *)pWebSites)->DfltRealms)
	{
		if(this->pRealms->ToXML(&xmlSub))
		{
			xmlConfig->Add(&xmlSub);
			xmlSub.Destroy();
			iSubItems++;
		}
	}
	if(this->pAuthentication != ((CWebSites *)pWebSites)->DfltAuthentication)
	{
		if(this->pAuthentication->ToXML(&xmlSub))
		{
			xmlConfig->Add(&xmlSub);
			xmlSub.Destroy();
			iSubItems++;
		}
	}
	if(this->pErrorPages != ((CWebSites *)pWebSites)->DfltErrorPages)
	{
		if(this->pErrorPages->ToXML(&xmlSub))
		{
			xmlConfig->Add(&xmlSub);
			xmlSub.Destroy();
			iSubItems++;
		}
	}

	return iSubItems;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int CWebSite::Port(void)
{
	return ((CWebSites *)this->pWebSites)->pSocketPools->Port(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWebSite::Stop(void)
{
	if(this->IsStarted)
	{
		if(((CWebSites *)this->pWebSites)->pSocketPools->Stop(this))
		{
			this->IsStarted = false;
		}
	}
	return !this->IsStarted;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWebSite::Start(void)
{
	if(!this->IsStarted)
	{
		if(this->pSocketPool)
		{
			if(this->pSocketPool->pSocket->IsActive())
			{
				this->IsStarted = true;
			}
			else if (((CWebSites *)this->pWebSites)->pSocketPools->Start(this))
			{
				this->IsStarted = true;
			}
		}
		else {
			((CWebSites*)this->pWebSites)->Trace("No socket pool has been defined for this website. Configure the site and select one.");
		}
	}
	return this->IsStarted;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CWebSite::WriteLog(LOGFILEENTRY *pLFE)
{
	EnterCriticalSection(&this->csLogFile);

	char sDate[64];
	Get_ShortDate(sDate, sizeof(sDate));
	if(strcmp(sDate, this->sLogCycleDate) != 0)
	{
		if(!this->CycleLog())
		{
			return;
		}
	}

	fprintf(this->hLogFile, "%s\t%s\t%s\t%s\t%s\t%d\t%s\t"
		"%s\t%s\t%s\t%I64d\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%.0f\r\n",
		pLFE->sDate,pLFE->sTime,pLFE->sSiteName,pLFE->sSourceIP,
		pLFE->sHost,pLFE->iPort,pLFE->sMethod,pLFE->sURI,pLFE->sQuery,
		pLFE->sStatus,pLFE->iContentLength,pLFE->sAgent,
		pLFE->sAccept,pLFE->sAcceptEncoding,pLFE->sAcceptLanguage,
		pLFE->sCacheControl,pLFE->sConnection,pLFE->sVersion,pLFE->Duration);

	LeaveCriticalSection(&this->csLogFile);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CWebSite::WriteLog(const char *sText)
{
	EnterCriticalSection(&this->csLogFile);

	char sDate[64];
	Get_ShortDate(sDate, sizeof(sDate));
	if(strcmp(sDate, this->sLogCycleDate) != 0)
	{
		if(!this->CycleLog())
		{
			return;
		}
	}

	fwrite(sText, sizeof(char), strlen(sText), this->hLogFile);
	LeaveCriticalSection(&this->csLogFile);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CWebSite::CloseLog(void)
{
	EnterCriticalSection(&this->csLogFile);
	if(this->hLogFile) fclose(this->hLogFile);
	this->hLogFile = NULL;
	LeaveCriticalSection(&this->csLogFile);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWebSite::CycleLog(void)
{
	EnterCriticalSection(&this->csLogFile);
	bool bResult = false;

	char sTime[64];
	char sFileName[MAX_PATH];
	memset(sFileName, 0, sizeof(sFileName));

	if(Get_ShortDate(this->sLogCycleDate, sizeof(this->sLogCycleDate)) && Get_Time(sTime, sizeof(sTime), 0))
	{
		char sSiteName[MAX_PATH];

		strcpy_s(sSiteName, sizeof(sSiteName), this->Name);
		CleanFileName(sSiteName, sizeof(sSiteName));

		sprintf_s(sFileName, sizeof(sFileName), "%s\\%s\\%s",
			this->pWebsiteSettings->LogPath(), gsServiceName, sSiteName);
		CorrectReversePath(sFileName, sizeof(sFileName), true);
		if(!IsDirectory(sFileName))
		{
			if(!CreateFolderStructure(sFileName))
			{
				char sErrorMsg[2048];
				GetLastError(sErrorMsg, sizeof(sErrorMsg),
					"Site log file folder creation failed (", ")");
				strcat_s(sErrorMsg, sizeof(sErrorMsg), "\r\n\r\n\"");
				strcat_s(sErrorMsg, sizeof(sErrorMsg), sFileName);
				strcat_s(sErrorMsg, sizeof(sErrorMsg), "\"");
				((CWebSites*)this->pWebSites)->Trace(sErrorMsg);

				this->hLogFile = NULL;

				LeaveCriticalSection(&this->csLogFile);
				return false;
			}
		}

		sprintf_s(sFileName, sizeof(sFileName), "%s\\%s\\%s\\%s.txt",
			this->pWebsiteSettings->LogPath(), gsServiceName, sSiteName, this->sLogCycleDate);
		CorrectReversePath(sFileName, sizeof(sFileName), true);

		if(this->hLogFile)
		{
			fclose(this->hLogFile);
		}
 
		bResult = ((this->hLogFile = _fsopen(sFileName, "ab", _SH_DENYWR)) != NULL);
		if(bResult)
		{
			if(!this->pWebsiteSettings->CacheLogFile())
			{
				setvbuf(this->hLogFile, NULL, _IONBF, 0);
			}

			char sText[1024];
			sprintf_s(sText, sizeof(sText),
				"#Software: NetworkDLS %s\r\n"
				"#Version: %s\r\n"
				"#Date: %s %s\r\n",
				gsTitleCaption, gsFileVersion, this->sLogCycleDate, sTime);
			this->WriteLog(sText);

			this->WriteLog("Date\tTime\tSite Name\tSource IP\t"
				"Host\tPort\tMethod\tURI\tQuery\tStatus\tContent Length\t"
				"Agent\tAccept\tAccept Encoding\tAccept Language\t"
				"Cache Control\tConnection\tVersion\tDuration(ms)\r\n");
		}
	}

	if(!bResult)
	{
		char sErrorMsg[2048];
		GetLastError(sErrorMsg, sizeof(sErrorMsg),
			"Site log file creation failed (", ")");
		strcat_s(sErrorMsg, sizeof(sErrorMsg), "\r\n\r\n\"");
		strcat_s(sErrorMsg, sizeof(sErrorMsg), sFileName);
		strcat_s(sErrorMsg, sizeof(sErrorMsg), "\"");
		((CWebSites*)this->pWebSites)->Trace(sErrorMsg);

		this->hLogFile = NULL;
	}

	LeaveCriticalSection(&this->csLogFile);
	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	Returns the index of this WebSite in the parent collection.
*/
int CWebSite::Index(void)
{
	for(int iIndex = 0; iIndex < ((CWebSites *)pWebSites)->Collection.Count; iIndex++)
	{
		if(this == ((CWebSites *)pWebSites)->Collection.WebSite[iIndex])
		{
			return iIndex;
		}
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWebSite::IsActive(void)
{
	return this->IsStarted && ((CWebSites *)this->pWebSites)->pSocketPools->IsActive(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CWebSite::CurrentConnections(void)
{
	return ((CWebSites*)this->pWebSites)->pSocketPools->CurrentConnections(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
