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

#include "../../../../@Libraries/CSocketServer/CSocketServer.H"
#include "../../../../@Libraries/CXML/CXMLReader.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/CStack/CStack.H"

#include "CWebSite.H"
#include "CHttp.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_WSTrace { //FIXFIX: This needs to be a class with its own critical section.
	CStack *Stack;
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

public:
	CWebSites();
	~CWebSites();

	void StopAllSites(void);
	bool StartAllSites(void);
	void DeReferenceAllSites(void);
	void BindSocketPools(void);

	CStack *RunningSites(void);

	void StartTrace(void);
	int StopTrace(void);
	int StopTrace(CStringBuilder *pOutText);
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
	bool Add(CXMLReader *xmlSite);
	bool Add(CXMLReader *xmlSite, int *iOutIndex);

	bool Reload(void);
	bool ReloadSites(void);
	bool ReloadSites(CXMLReader *pWebSites);
	bool LoadSites(CXMLReader *pWebSites);
	bool LoadSites(char *sFile);

	CWebSite *Site(int iIndex);
	int Count(void);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern CWebSites *gWebSites;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
