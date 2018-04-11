///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CCustomFilters_CPP
#define _CCustomFilters_CPP
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

#include "CCustomFilters.H"
#include "Entry.H"
#include "CWebSites.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::File;
using namespace NSWFL::ListView;
using namespace NSWFL::Windows;
using namespace NSWFL::Conversion;
using namespace NSWFL::Collections;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CCustomFilters::~CCustomFilters(void)
{
	this->Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CCustomFilters::CCustomFilters(void *lpWebSites)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CCustomFilters::CCustomFilters(void *lpWebSites, XMLReader *xmlConfig, CCustomFilters *pDefaults)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
	this->Load(xmlConfig, pDefaults);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCustomFilters::Save(void)
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

bool CCustomFilters::ToXML(XMLReader *lpXML)
{
	this->Locks.LockShared();

	XMLWriter xmlConfig("CustomFilters");

	xmlConfig.AddBool("Enable", this->Collection.Enabled);

	for(int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		XMLWriter Item("Filter");
		Item.Add("DLL", this->Collection.Items[iItem].DLL);
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
bool CCustomFilters::Reload(void)
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

bool CCustomFilters::Load(const char *sXMLFileName)
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
		if(xmlConfig.ToReader("CustomFilters", &xmlEntity))
		{
			this->Initialized = this->Load(&xmlEntity, NULL);
		}
		xmlEntity.Destroy();
		xmlConfig.Destroy();
	}

	return this->Locks.UnlockExclusive(this->Initialized);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCustomFilters::Load(XMLReader *xmlConfig, CCustomFilters *pDefaults)
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
	XMLReader XPCustomFilter;

	while(xmlConfig->ToReader("Filter", &XPCustomFilter))
	{
		char sDLL[CUSTOMFILTERS_MAX_DLL_LENGTH];
		char sDescr[CUSTOMFILTERS_MAX_DESCR_LENGTH];

		this->Collection.Items = (CUSTOMFILTER *)
			pMem->ReAllocate(this->Collection.Items, sizeof(CUSTOMFILTER), this->Collection.Count + 1);

		CUSTOMFILTER *p = &this->Collection.Items[this->Collection.Count++];
		memset(p, 0, sizeof(CUSTOMFILTER));

		XPCustomFilter.ToString("DLL", sDLL, sizeof(sDLL), &iLength);
		p->DLL = (char *) pMem->StrDup(sDLL);

		XPCustomFilter.ToString("Description", sDescr, sizeof(sDescr), &iLength);
		p->Description = (char *) pMem->StrDup(sDescr);

		p->Enabled = XPCustomFilter.ToBoolean("Enable", true);

		if(this->Collection.Enabled && p->Enabled)
		{
			if((p->ModuleHandle = LoadLibrary(p->DLL)))
			{
				p->OnConnect = (LPOnConnect)GetProcAddress(p->ModuleHandle, "OnConnect");
				p->OnDisconnect = (LPOnDisconnect)GetProcAddress(p->ModuleHandle, "OnDisconnect");
				p->OnRequestConcluded = (LPOnRequestConcluded)GetProcAddress(p->ModuleHandle, "OnRequestConcluded");
				p->OnLog = (LPOnLog)GetProcAddress(p->ModuleHandle, "OnLog");
				p->OnProcessRawResponseHeader = (LPOnProcessRawResponseHeader)GetProcAddress(p->ModuleHandle, "OnProcessRawResponseHeader");
				p->OnProcessRequestHeader = (LPOnProcessRequestHeader)GetProcAddress(p->ModuleHandle, "OnProcessRequestHeader");
				p->OnRecvRawData = (LPOnRecvRawData)GetProcAddress(p->ModuleHandle, "OnRecvRawData");
				p->OnSendRawData = (LPOnSendRawData)GetProcAddress(p->ModuleHandle, "OnSendRawData");
				p->OnCompressData = (LPOnCompressData)GetProcAddress(p->ModuleHandle, "OnCompressData");
				p->OnUrlMap = (LPOnUrlMap)GetProcAddress(p->ModuleHandle, "OnUrlMap");
			}
			else {
				char sErrorMsg[2048];
				GetLastError(sErrorMsg, sizeof(sErrorMsg),
					"Failed to load custom filter (", ")");

				strcat_s(sErrorMsg, sizeof(sErrorMsg), "\r\n\r\n\"");
				strcat_s(sErrorMsg, sizeof(sErrorMsg), p->DLL);
				strcat_s(sErrorMsg, sizeof(sErrorMsg), "\"");

				((CWebSites *)this->pWebSites)->Trace(sErrorMsg);

				p->Enabled = false;
			}
		}

		XPCustomFilter.Destroy();
	}

	return this->Locks.UnlockExclusive((this->Initialized = true));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCustomFilters::Destroy(void)
{
	this->Locks.LockExclusive();

	if(this->Initialized)
	{
		this->Initialized = false;
		if(this->Collection.Count > 0)
		{
			for(int iPos = 0; iPos < this->Collection.Count; iPos++)
			{
				pMem->Free(this->Collection.Items[iPos].DLL);
				pMem->Free(this->Collection.Items[iPos].Description);

				if(this->Collection.Items[iPos].ModuleHandle)
				{
					FreeLibrary(this->Collection.Items[iPos].ModuleHandle);
				}
			}

			pMem->Free(this->Collection.Items);

			this->Collection.Count = 0;
		}
	}

	return this->Locks.UnlockExclusive(true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCustomFilters::FreeFilterAction(FILTERACTION *filterAction)
{
	if(filterAction->ErrorInformation) free(filterAction->ErrorInformation);
	if(filterAction->RedirectLocation) free(filterAction->RedirectLocation);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCustomFilters::PushFilterHeader(void *pHTTPRequestHeader, FILTERHTTPHEADER *filterRequestHeader)
{
	HTTPHEADER *httpRequestHeader = (HTTPHEADER *)pHTTPRequestHeader;

	memset(filterRequestHeader, 0, sizeof(FILTERHTTPHEADER));
	if(httpRequestHeader->Method) filterRequestHeader->Method = _strdup(httpRequestHeader->Method);
	if(httpRequestHeader->Request) filterRequestHeader->Request = _strdup(httpRequestHeader->Request);
	if(httpRequestHeader->FullRequest) filterRequestHeader->FullRequest = _strdup(httpRequestHeader->FullRequest);
	if(httpRequestHeader->Version) filterRequestHeader->Version = _strdup(httpRequestHeader->Version);
	if(httpRequestHeader->Query) filterRequestHeader->Query = _strdup(httpRequestHeader->Query);
	if(httpRequestHeader->ContentType) filterRequestHeader->ContentType = _strdup(httpRequestHeader->ContentType);
	if(httpRequestHeader->Accept) filterRequestHeader->Accept = _strdup(httpRequestHeader->Accept);
	if(httpRequestHeader->AcceptLanguage) filterRequestHeader->AcceptLanguage = _strdup(httpRequestHeader->AcceptLanguage);
	if(httpRequestHeader->AcceptEncoding) filterRequestHeader->AcceptEncoding = _strdup(httpRequestHeader->AcceptEncoding);
	if(httpRequestHeader->UserAgent) filterRequestHeader->UserAgent = _strdup(httpRequestHeader->UserAgent);
	if(httpRequestHeader->Host) filterRequestHeader->Host = _strdup(httpRequestHeader->Host);
	if(httpRequestHeader->Connection) filterRequestHeader->Connection = _strdup(httpRequestHeader->Connection);
	if(httpRequestHeader->CacheControl) filterRequestHeader->CacheControl = _strdup(httpRequestHeader->CacheControl);
	if(httpRequestHeader->Referer) filterRequestHeader->Referer = _strdup(httpRequestHeader->Referer);
	if(httpRequestHeader->Cookie) filterRequestHeader->Cookie = _strdup(httpRequestHeader->Cookie);
	if(httpRequestHeader->ContentRange) filterRequestHeader->ContentRange = _strdup(httpRequestHeader->ContentRange);
	filterRequestHeader->ContentBegin = httpRequestHeader->ContentBegin;
	filterRequestHeader->ContentEnd = httpRequestHeader->ContentEnd;
	if(httpRequestHeader->PostData) filterRequestHeader->PostData = _strdup(httpRequestHeader->PostData);
	filterRequestHeader->PostDataSize = httpRequestHeader->PostDataSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCustomFilters::PopFilterHeader(FILTERHTTPHEADER *filterRequestHeader, void *pHTTPRequestHeader)
{
	HTTPHEADER *httpRequestHeader = (HTTPHEADER *)pHTTPRequestHeader;

	if(filterRequestHeader->ModifiedByFilter)
	{
		//Free the old HTTP header
		if(httpRequestHeader->Method) pMem->Free(httpRequestHeader->Method);
		if(httpRequestHeader->Request) pMem->Free(httpRequestHeader->Request);
		if(httpRequestHeader->FullRequest) pMem->Free(httpRequestHeader->FullRequest);
		if(httpRequestHeader->Version) pMem->Free(httpRequestHeader->Version);
		if(httpRequestHeader->Query) pMem->Free(httpRequestHeader->Query);
		if(httpRequestHeader->ContentType) pMem->Free(httpRequestHeader->ContentType);
		if(httpRequestHeader->Accept) pMem->Free(httpRequestHeader->Accept);
		if(httpRequestHeader->AcceptLanguage) pMem->Free(httpRequestHeader->AcceptLanguage);
		if(httpRequestHeader->AcceptEncoding) pMem->Free(httpRequestHeader->AcceptEncoding);
		if(httpRequestHeader->UserAgent) pMem->Free(httpRequestHeader->UserAgent);
		if(httpRequestHeader->Host) pMem->Free(httpRequestHeader->Host);
		if(httpRequestHeader->Connection) pMem->Free(httpRequestHeader->Connection);
		if(httpRequestHeader->CacheControl) pMem->Free(httpRequestHeader->CacheControl);
		if(httpRequestHeader->Referer) pMem->Free(httpRequestHeader->Referer);
		if(httpRequestHeader->Cookie) pMem->Free(httpRequestHeader->Cookie);
		if(httpRequestHeader->ContentRange) pMem->Free(httpRequestHeader->ContentRange);
		if(httpRequestHeader->PostData) pMem->Free(httpRequestHeader->PostData);

		//Replace the old HTTP header with the Filter Header.
		httpRequestHeader->Method = pMem->StrDup(filterRequestHeader->Method);
		httpRequestHeader->Request = pMem->StrDup(filterRequestHeader->Request);
		httpRequestHeader->FullRequest = pMem->StrDup(filterRequestHeader->FullRequest);
		httpRequestHeader->Version = pMem->StrDup(filterRequestHeader->Version);
		httpRequestHeader->Query = pMem->StrDup(filterRequestHeader->Query);
		httpRequestHeader->ContentType = pMem->StrDup(filterRequestHeader->ContentType);
		httpRequestHeader->Accept = pMem->StrDup(filterRequestHeader->Accept);
		httpRequestHeader->AcceptLanguage = pMem->StrDup(filterRequestHeader->AcceptLanguage);
		httpRequestHeader->AcceptEncoding = pMem->StrDup(filterRequestHeader->AcceptEncoding);
		httpRequestHeader->UserAgent = pMem->StrDup(filterRequestHeader->UserAgent);
		httpRequestHeader->Host = pMem->StrDup(filterRequestHeader->Host);
		httpRequestHeader->Connection = pMem->StrDup(filterRequestHeader->Connection);
		httpRequestHeader->CacheControl = pMem->StrDup(filterRequestHeader->CacheControl);
		httpRequestHeader->Referer = pMem->StrDup(filterRequestHeader->Referer);
		httpRequestHeader->Cookie = pMem->StrDup(filterRequestHeader->Cookie);
		httpRequestHeader->ContentRange = pMem->StrDup(filterRequestHeader->ContentRange);
		httpRequestHeader->ContentBegin = filterRequestHeader->ContentBegin;
		httpRequestHeader->ContentEnd = filterRequestHeader->ContentEnd;
		httpRequestHeader->PostData = pMem->StrDup(filterRequestHeader->PostData);
		httpRequestHeader->PostDataSize = filterRequestHeader->PostDataSize;
	}

	//Free the Filter Header.
	if(filterRequestHeader->Method) free(filterRequestHeader->Method);
	if(filterRequestHeader->Request) free(filterRequestHeader->Request);
	if(filterRequestHeader->FullRequest) free(filterRequestHeader->FullRequest);
	if(filterRequestHeader->Version) free(filterRequestHeader->Version);
	if(filterRequestHeader->Query) free(filterRequestHeader->Query);
	if(filterRequestHeader->ContentType) free(filterRequestHeader->ContentType);
	if(filterRequestHeader->Accept) free(filterRequestHeader->Accept);
	if(filterRequestHeader->AcceptLanguage) free(filterRequestHeader->AcceptLanguage);
	if(filterRequestHeader->AcceptEncoding) free(filterRequestHeader->AcceptEncoding);
	if(filterRequestHeader->UserAgent) free(filterRequestHeader->UserAgent);
	if(filterRequestHeader->Host) free(filterRequestHeader->Host);
	if(filterRequestHeader->Connection) free(filterRequestHeader->Connection);
	if(filterRequestHeader->CacheControl) free(filterRequestHeader->CacheControl);
	if(filterRequestHeader->Referer) free(filterRequestHeader->Referer);
	if(filterRequestHeader->Cookie) free(filterRequestHeader->Cookie);
	if(filterRequestHeader->ContentRange) free(filterRequestHeader->ContentRange);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns true if the filters passed, otherwise false.
*/
FILTERESULT CCustomFilters::ProcessRawResponseHeader(void *pvPeer /*PEER *pC*/, StringBuilder *sResponseHeader, FILTERACTION *pFilterAction)
{
	this->Locks.LockShared();
	int iFilterResult = FILTER_EVENT_RESULT_CONTINUE;

	if(this->Collection.Enabled && (this->Collection.Count > 0))
	{
		bool bInitialized = false;
		PEER *pC = (PEER*)pvPeer;
		char *sTempResponseHeader = NULL;
		FILTERHTTPHEADER filterRequestHeader;

		for(int iPos = 0; iPos < this->Collection.Count; iPos++)
		{
			if(this->Collection.Items[iPos].Enabled && this->Collection.Items[iPos].OnProcessRawResponseHeader)
			{
				if(!bInitialized)
				{
					sTempResponseHeader = (char *)pMem->Allocate(sizeof(char), sResponseHeader->Length + 1);
					memcpy_s(sTempResponseHeader, sResponseHeader->Length + 1, sResponseHeader->Buffer, sResponseHeader->Length + 1);

					PushFilterHeader(&pC->Header, &filterRequestHeader);
					bInitialized = true;
				}

				int iFilterResult = this->Collection.Items[iPos].OnProcessRawResponseHeader(
					pC->pClient->PeerID(), &filterRequestHeader, pFilterAction, sTempResponseHeader, sResponseHeader->Length);
				if(iFilterResult != FILTER_EVENT_RESULT_CONTINUE)
				{
					break;
				}
			}
		}

		if(bInitialized)
		{
			if(strcmp(sTempResponseHeader, sResponseHeader->Buffer) != 0)
			{
				sResponseHeader->Set(sTempResponseHeader);
			}
			pMem->Free(sTempResponseHeader);
			PopFilterHeader(&filterRequestHeader, &pC->Header);
		}
	}

	return this->Locks.UnlockShared(iFilterResult);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns true if the filters passed, otherwise false.
*/
FILTERESULT CCustomFilters::ProcessRequestHeader(void *pvPeer /*PEER *pC*/, FILTERACTION *pFilterAction)
{
	this->Locks.LockShared();
	int iFilterResult = FILTER_EVENT_RESULT_CONTINUE;

	if(this->Collection.Enabled && (this->Collection.Count > 0))
	{
		bool bInitialized = false;
		PEER *pC = (PEER*)pvPeer;
		FILTERHTTPHEADER filterRequestHeader;

		for(int iPos = 0; iPos < this->Collection.Count; iPos++)
		{
			if(this->Collection.Items[iPos].Enabled && this->Collection.Items[iPos].OnProcessRequestHeader)
			{
				if(!bInitialized)
				{
					PushFilterHeader(&pC->Header, &filterRequestHeader);
					bInitialized = true;
				}

				int iFilterResult = this->Collection.Items[iPos].OnProcessRequestHeader(pC->pClient->PeerID(), &filterRequestHeader, pFilterAction);
				if(iFilterResult != FILTER_EVENT_RESULT_CONTINUE)
				{
					break;
				}
			}
		}

		if(bInitialized)
		{
			PopFilterHeader(&filterRequestHeader, &pC->Header);
		}
	}

	return this->Locks.UnlockShared(iFilterResult);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns true if the filters passed, otherwise false.
*/
FILTERESULT CCustomFilters::ProcessLogFilters(void *pvPeer /*PEER *pC*/, FILTERLOGENTRY *pLFE)
{
	this->Locks.LockShared();
	int iFilterResult = FILTER_EVENT_RESULT_CONTINUE;

	if(this->Collection.Enabled && (this->Collection.Count > 0))
	{
		PEER *pC = (PEER*)pvPeer;

		for(int iPos = 0; iPos < this->Collection.Count; iPos++)
		{
			if(this->Collection.Items[iPos].Enabled && this->Collection.Items[iPos].OnLog)
			{
				int iFilterResult = this->Collection.Items[iPos].OnLog(pC->pClient->PeerID(), pLFE);
				if(iFilterResult != FILTER_EVENT_RESULT_CONTINUE)
				{
					break;
				}
			}
		}
	}

	return this->Locks.UnlockShared(iFilterResult);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns true if the filters passed, otherwise false.
*/
FILTERESULT CCustomFilters::ProcessConnectFilters(void *pvClient /*CSocketClient *pClient*/, const char *sIPAddress)
{
	this->Locks.LockShared();
	int iFilterResult = FILTER_EVENT_RESULT_CONTINUE;

	if(this->Collection.Enabled && (this->Collection.Count > 0))
	{
		CSocketClient *pClient = (CSocketClient*)pvClient;

		for(int iPos = 0; iPos < this->Collection.Count; iPos++)
		{
			if(this->Collection.Items[iPos].Enabled && this->Collection.Items[iPos].OnConnect)
			{
				int iFilterResult = this->Collection.Items[iPos].OnConnect(pClient->PeerID(), sIPAddress);
				if(iFilterResult != FILTER_EVENT_RESULT_CONTINUE)
				{
					break;
				}
			}
		}
	}

	return this->Locks.UnlockShared(iFilterResult);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FILTENORESULT CCustomFilters::ProcessRequestConcludedFilters(void *pvClient /*CSocketClient *pClient*/)
{
	this->Locks.LockShared();

	if(this->Collection.Enabled && (this->Collection.Count > 0))
	{
		CSocketClient *pClient = (CSocketClient*)pvClient;

		for(int iPos = 0; iPos < this->Collection.Count; iPos++)
		{
			if(this->Collection.Items[iPos].Enabled && this->Collection.Items[iPos].OnRequestConcluded)
			{
				this->Collection.Items[iPos].OnRequestConcluded(pClient->PeerID());
			}
		}
	}

	this->Locks.UnlockShared();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FILTENORESULT CCustomFilters::ProcessDisconnectFilters(void *pvClient /*CSocketClient *pClient*/)
{
	this->Locks.LockShared();

	if(this->Collection.Enabled && (this->Collection.Count > 0))
	{
		CSocketClient *pClient = (CSocketClient*)pvClient;

		for(int iPos = 0; iPos < this->Collection.Count; iPos++)
		{
			if(this->Collection.Items[iPos].Enabled && this->Collection.Items[iPos].OnDisconnect)
			{
				this->Collection.Items[iPos].OnDisconnect(pClient->PeerID());
			}
		}
	}

	this->Locks.UnlockShared();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCustomFilters::ContainsCompressionFilter()
{
	this->Locks.LockShared();
	int iFilterResult = FILTER_EVENT_RESULT_CONTINUE;

	if(this->Collection.Enabled && (this->Collection.Count > 0))
	{
		for(int iPos = 0; iPos < this->Collection.Count; iPos++)
		{
			if(this->Collection.Items[iPos].Enabled && this->Collection.Items[iPos].OnCompressData)
			{
				return this->Locks.UnlockShared(true);
			}
		}
	}

	return this->Locks.UnlockShared(false);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FILTERESULT CCustomFilters::ProcessOnCompressFilters(void *pvPeer /*PEER *pC*/, LPBASICHUNK pChunk)
{
	this->Locks.LockShared();
	int iFilterResult = FILTER_EVENT_RESULT_CONTINUE;

	if(this->Collection.Enabled && (this->Collection.Count > 0))
	{
		PEER *pC = (PEER *) pvPeer;
		CSocketClient *pClient = (CSocketClient*)pC->pClient;

		for(int iPos = 0; iPos < this->Collection.Count; iPos++)
		{
			if(this->Collection.Items[iPos].Enabled && this->Collection.Items[iPos].OnCompressData)
			{
				char *sModifiedBuf = NULL;
				unsigned int uiModifiedBufSz = 0;

				int iFilterResult = this->Collection.Items[iPos].OnCompressData(pClient->PeerID(), pC->Header.PathTranslated,
					(const char *)pChunk->pBuffer, pChunk->iLength, &sModifiedBuf, &uiModifiedBufSz);

				if(sModifiedBuf != NULL)
				{
					((CSocketServer*)pClient->pSockServer)->ReAlloc(pChunk, uiModifiedBufSz + 1);
					memcpy_s(pChunk->pBuffer, uiModifiedBufSz + 1, sModifiedBuf, uiModifiedBufSz);
					pChunk->iLength = uiModifiedBufSz;
					GlobalFree(sModifiedBuf);
				}

				if(iFilterResult != FILTER_EVENT_RESULT_CONTINUE)
				{
					break;
				}
			}
		}
	}

	return this->Locks.UnlockShared(iFilterResult);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FILTERESULT CCustomFilters::ProcessSendFilters(void *pvClient, LPBASICHUNK pChunk)
{
	this->Locks.LockShared();
	int iFilterResult = FILTER_EVENT_RESULT_CONTINUE;

	if(this->Collection.Enabled && (this->Collection.Count > 0))
	{
		CSocketClient *pClient = (CSocketClient*)pvClient;

		for(int iPos = 0; iPos < this->Collection.Count; iPos++)
		{
			if(this->Collection.Items[iPos].Enabled && this->Collection.Items[iPos].OnSendRawData)
			{
				char *sModifiedBuf = NULL;
				unsigned int uiModifiedBufSz = 0;

				int iFilterResult = this->Collection.Items[iPos].OnSendRawData(pClient->PeerID(),
					(const char *)pChunk->pBuffer, pChunk->iLength, &sModifiedBuf, &uiModifiedBufSz);

				if(sModifiedBuf != NULL)
				{
					((CSocketServer*)pClient->pSockServer)->ReAlloc(pChunk, uiModifiedBufSz + 1);
					memcpy_s(pChunk->pBuffer, uiModifiedBufSz + 1, sModifiedBuf, uiModifiedBufSz);
					pChunk->iLength = uiModifiedBufSz;
					GlobalFree(sModifiedBuf);
				}

				if(iFilterResult != FILTER_EVENT_RESULT_CONTINUE)
				{
					break;
				}
			}
		}
	}

	return this->Locks.UnlockShared(iFilterResult);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FILTERESULT CCustomFilters::ProcessRecvFilters(void *pvClient, const char *sBuf, unsigned int uiBufSz)
{
	this->Locks.LockShared();
	int iFilterResult = FILTER_EVENT_RESULT_CONTINUE;

	if(this->Collection.Enabled && (this->Collection.Count > 0))
	{
		CSocketClient *pClient = (CSocketClient*)pvClient;

		for(int iPos = 0; iPos < this->Collection.Count; iPos++)
		{
			if(this->Collection.Items[iPos].Enabled && this->Collection.Items[iPos].OnRecvRawData)
			{
				int iFilterResult = this->Collection.Items[iPos].OnRecvRawData(pClient->PeerID(), sBuf, uiBufSz);
				if(iFilterResult != FILTER_EVENT_RESULT_CONTINUE)
				{
					break;
				}
			}
		}
	}

	return this->Locks.UnlockShared(iFilterResult);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FILTERESULT CCustomFilters::ProcessUrlMap(void *pvPeer /*PEER *pC*/, FILTERACTION *pFilterAction)
{
	this->Locks.LockShared();
	int iFilterResult = FILTER_EVENT_RESULT_CONTINUE;

	if(this->Collection.Enabled && (this->Collection.Count > 0))
	{
		bool bInitialized = false;
		PEER *pC = (PEER*)pvPeer;
		FILTERHTTPHEADER filterRequestHeader;

		for(int iPos = 0; iPos < this->Collection.Count; iPos++)
		{
			if(this->Collection.Items[iPos].Enabled && this->Collection.Items[iPos].OnUrlMap)
			{
				if(!bInitialized)
				{
					PushFilterHeader(&pC->Header, &filterRequestHeader);
					bInitialized = true;
				}
				
				int iFilterResult = this->Collection.Items[iPos].OnUrlMap(pC->pClient->PeerID(), &filterRequestHeader, pFilterAction);
				if(iFilterResult != FILTER_EVENT_RESULT_CONTINUE)
				{
					break;
				}
			}
		}

		if(bInitialized)
		{
			PopFilterHeader(&filterRequestHeader, &pC->Header);
		}
	}

	return this->Locks.UnlockShared(iFilterResult);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
