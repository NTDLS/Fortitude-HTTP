///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CWebSites_H
#define _CWebSites_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../CSocketServer/CSocketServer.H"
#include "../../../NSWFL/NSWFL.h"

#include "CWebSite.H"
#include "CHttp.H"

using namespace NSWFL::Collections;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_WSTrace { //FIXFIX: This needs to be a class with its own critical section.
	Stack *Stack;
	bool bTracingEnabled;
	CRITICAL_SECTION CS;
} ERRORTRACING, LPERRORTRACING;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CWebSites{

private:
	DWORD dwTotalConnections;
	char sFileName[MAX_PATH];

	ERRORTRACING InternalTrace;

	bool Initialize(void);
	bool Destroy(void);
	bool bInitializeSuccess;

public:
	CWebSites();
	~CWebSites();

	bool InitializeSuccess()
	{
		return bInitializeSuccess;
	}

	void StopAllSites(void);
	bool StartAllSites(void);
	void DeReferenceAllSites(void);
	void BindSocketPools(void);

	Stack *RunningSites(void);

	void StartTrace(void);
	int StopTrace(void);
	int StopTrace(StringBuilder *pOutText);
	void Trace(const char *sText);

	typedef struct _tag_Web_Site_Collection{
		int Count;
		CWebSite **WebSite;
	} WEBSITECOLLECTION;

	WEBSITECOLLECTION Collection;

	CWebsiteSettings *DfltWebsiteSettings;
	CMimeTypes *DfltMimeTypes;
	CExtensionFilters *DfltExtensionFilters;
	CURLFilters *DfltURLFilters;
	CDefaultPages *DfltDefaultPages;
	CVirtualRoots *DfltVirtualRoots;
	CErrorPages *DfltErrorPages;
	CSSIFiles *DfltSSIFiles;
	//CListenIPs *DfltListenIPs; //Has no defaults.
	CIPFilters *DfltIPFilters;
	CCGIFolders *DfltCGIFolders;
	CCompression *DfltCompression;
	CCustomFilters *DfltCustomFilters;
	CScriptingEngines *DfltScriptingEngines;
	CAuthentication *DfltAuthentication;
	CRealms *DfltRealms;

	CServerSettings *pServerSettings;
	CSocketPools *pSocketPools;
	CHttp *pHttp;

	int GetCurrentConenctions(void);
	int GetTotalConnections(void);
	void IncrementConnectionCount(void);

	bool Save(bool bOnlySaveReferencedSites);
	bool SaveAll(void);

	void DestroySites(void);
	bool Add(XMLReader *xmlSite);
	bool Add(XMLReader *xmlSite, int *iOutIndex);

	bool Reload(void);
	bool ReloadSites(void);
	bool ReloadSites(XMLReader *pWebSites);
	bool LoadSites(XMLReader *pWebSites);
	bool LoadSites(char *sFile);

	CWebSite *Site(int iIndex);
	int Count(void);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern CWebSites *gWebSites;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
