///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CSocketPools_H
#define _CSocketPools_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../../@Libraries/CXML/CXMLReader.H"
#include "../../../../@Libraries/CSocketServer/CSocketServer.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/CLocks/CLocks.H"
#include "../../../../@Libraries/CStack/CStack.H"

#include "../../@Common/SocketPools.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSocketPools {

private:
	bool Initialized;
	void *pWebSites;
	char sFileName[MAX_PATH];

	CLocks Locks;

public:
	SOCKETPOOLS Collection;

	~CSocketPools(void);
	CSocketPools(void *lpWebSites);
	CSocketPools(void *lpWebSites, CXMLReader *xmlConfig, CServerSettings *pDefaults);
	bool Save(void);

	bool ToXML(CXMLReader *lpXML);
	bool ToXML(CXMLReader *lpXML, bool bIncludeStats);

	bool Load(const char *sXMLFileName);
	bool Load(CXMLReader *xmlConfig);
	bool Reload(void);
	bool Destroy(void);

	PEER *InitializePeerStructure(CSocketServer *pSockerServer, CSocketClient *pClient, const char *sHostHeader);
	int ReferencedSiteCount(SOCKETPOOL *pPool);
	SOCKETPOOL *Pool(void *lpWebSite);
	SOCKETPOOL *Pool(CSocketServer *pSockerServer);
	bool Start(void *lpWebSite);
	bool Stop(void *lpWebSite);
	int MaxConnections(void *lpWebSite);
	int CurrentConnections(void *lpWebSite);
	int InitialBufferSize(void *lpWebSite);
	int MaxBufferSize(void *lpWebSite);
	int FileBufferSize(void *lpWebSite);
	bool IsActive(void *lpWebSite);
	int Port(void *lpWebSite);
	const char *UID(void *lpWebSite);
	const char *Description(void *lpWebSite);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

