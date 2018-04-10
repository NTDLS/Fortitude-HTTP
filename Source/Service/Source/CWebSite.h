///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CWEBSITE_H
#define _CWEBSITE_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../../@Libraries/CSocketServer/CSocketServer.H"
#include "../../../../@Libraries/CXML/CXMLReader.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/CStack/CStack.H"

#include "../../@Common/WebSite.h"
#include "../../@Common/Constants.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Site_Stats {
	__int64 AcceptedConnections;
	__int64 Hits;
	__int64 BytesRecv;
	__int64 BytesSent;
} SITESTATS, *LPSITESTATS;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_LOG_FILE_ENTRY {
	char sDate[64];
	char sTime[64];
	char *sSiteName;
	char sSourceIP[20];
	//char sTargetIP[20]; //Not yet implemented.
	char *sMethod; //GET, PUT, POST
	char *sAgent;
	char *sStatus;
	char *sAccept;
	char *sAcceptEncoding;
	char *sAcceptLanguage;
	char *sCacheControl;
	char *sConnection;
	char *sHost;
	char *sVersion;
	__int64 iContentLength; //Bytes
	char *sURI;
	char *sQuery;
	int iPort;
	double Duration;
	//char sUserName[...]; //Not yet implemented.
} LOGFILEENTRY, *LPLOGFILEENTRY;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CMimeTypes.h"
#include "CURLFilters.h"
#include "CExtensionFilters.h"
#include "CDefaultPages.H"
#include "CWebsiteSettings.H"
#include "CVirtualRoots.H"
#include "CSSIFiles.H"
#include "CIPFilters.H"
#include "CCGIFolders.H"
#include "CCompression.H"
#include "CCustomFilters.H"
#include "CScriptingEngines.H"
#include "CSSIFiles.H"
#include "CURLFilters.H"
#include "CCGIFolders.H"
#include "CScriptingEngines.H"
#include "CErrorPages.H"
#include "CAuthentication.H"
#include "CRealms.H"
#include "CServerSettings.H"
#include "CSocketPools.h"
#include "CHostHeaders.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CWebSite{

private:
	FILE *hLogFile;
	char sLogCycleDate[64];
	CRITICAL_SECTION csLogFile;

	void Initialize(void);

public:
	CWebSite();
	CWebSite(void *pWebSites, CXMLReader *pXMLSite);
	~CWebSite();

	friend void Socket_ClientHandler(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);

	int Index(void);

	bool IsActive(void);
	int SubItemsToXML(CXMLWriter *xmlConfig);
	bool ToXML(CXMLWriter *pXMLSite);
	bool ToXML(CXMLWriter *pXMLSite, bool bIncludeSubItems, bool bIncludeStatusInfo);
	bool Overview(CXMLWriter *pXMLSite);
	bool Bindings(CXMLWriter *pXMLSite);
	bool Start(void);
	bool Stop(void);
	bool Reload(CXMLReader *xml);
	void BindSocketPool(void);
	int CurrentConnections(void);

	bool CycleLog(void);
	void CloseLog(void);
	void WriteLog(LOGFILEENTRY *pLFE);
	void WriteLog(const char *sText);

	char Name[MAX_SITE_NAME];
	char Description[MAX_SITE_DESCR];
	char Root[MAX_PATH];
	char SocketPoolID[SOCKETPOOLS_MAX_UID_LENGTH];
	unsigned int Port(void);
	bool IsStarted;

	SITESTATS Stats;
	bool bReferencedInUpdate;

	VOID *pWebSites;		//CWebSites*
	VOID *pHttp;			//CHttp*
	SOCKETPOOL *pSocketPool;

	CHostHeaders *pHostHeaders;
	CMimeTypes *pMimeTypes;
	CExtensionFilters *pExtensionFilters;
	CURLFilters *pURLFilters;
	CDefaultPages *pDefaultPages;
	CVirtualRoots *pVirtualRoots;
	CSSIFiles *pSSIFiles;
	CIPFilters *pIPFilters;
	//CListenIPs *pListenIPs;
	CCGIFolders *pCGIFolders;
	CCompression *pCompression;
	CCustomFilters *pCustomFilters;
	CScriptingEngines *pScriptingEngines;
	CErrorPages *pErrorPages;
	CWebsiteSettings *pWebsiteSettings;
	CAuthentication *pAuthentication;
	CRealms *pRealms;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
