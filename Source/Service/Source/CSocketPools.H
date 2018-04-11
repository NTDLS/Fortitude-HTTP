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

#include "../../../NSWFL/NSWFL.h"
#include "../../../CSocketServer/CSocketServer.H"

#include "../../@Common/SocketPools.h"

using namespace NSWFL::Isolation;
using namespace NSWFL::XML;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSocketPools {

private:
	bool Initialized;
	void *pWebSites;
	char sFileName[MAX_PATH];

	IsolationLock Locks;

public:
	SOCKETPOOLS Collection;

	~CSocketPools(void);
	CSocketPools(void *lpWebSites);
	CSocketPools(void *lpWebSites, XMLReader *xmlConfig, CServerSettings *pDefaults);
	bool Save(void);

	bool ToXML(XMLReader *lpXML);
	bool ToXML(XMLReader *lpXML, bool bIncludeStats);

	bool Load(const char *sXMLFileName);
	bool Load(XMLReader *xmlConfig);
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

