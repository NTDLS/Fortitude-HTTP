///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CCustomFilters_H
#define _CCustomFilters_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../../@Libraries/CXML/CXMLReader.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/CLocks/CLocks.H"
#include "../../../../@Libraries/CStack/CStack.H"

#include "../../../Setup/Filters/Filter Example/Source/HTTPFilter.H"
#include "../../../../@Libraries/CSocketServer/CSocketServer.H"

#include "../../@Common/CustomFilters.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCustomFilters{

private:
	bool Initialized;
	void *pWebSites;
	CUSTOMFILTERS Collection;
	char sFileName[MAX_PATH];

	void PopFilterHeader(FILTERHTTPHEADER *filterRequestHeader, void *pHTTPRequestHeader);
	void PushFilterHeader(void *pHTTPRequestHeader, FILTERHTTPHEADER *filterRequestHeader);

	CLocks Locks;

public:
	~CCustomFilters(void);
	CCustomFilters(void *lpWebSites);
	CCustomFilters(void *lpWebSites, CXMLReader *xmlConfig, CCustomFilters *pDefaults);
	bool Save(void);
	
	void FreeFilterAction(FILTERACTION *filterAction);
	bool ToXML(CXMLReader *lpXML);
	FILTERESULT ProcessRawResponseHeader(void *pvPeer /*PEER *pC*/, CStringBuilder *sResponseHeader, FILTERACTION *pFilterAction);
	FILTERESULT ProcessUrlMap(void *pvPeer /*PEER *pC*/, FILTERACTION *pFilterAction);
	FILTERESULT ProcessRequestHeader(void *pvPeer /*PEER *pC*/, FILTERACTION *pFilterAction);
	FILTENORESULT ProcessDisconnectFilters(void *pvClient /*CSocketClient *pClient*/);
	FILTENORESULT ProcessRequestConcludedFilters(void *pvClient /*CSocketClient *pClient*/);
	FILTERESULT ProcessConnectFilters(void *pvClient /*CSocketClient *pClient*/, const char *sIPAddress);
	FILTERESULT ProcessLogFilters(void *pvPeer /*PEER *pC*/, FILTERLOGENTRY *pLFE);
	FILTERESULT ProcessSendFilters(void *pvClient, LPBASICHUNK pChunk);
	FILTERESULT ProcessOnCompressFilters(void *pvPeer, LPBASICHUNK pChunk);
	FILTERESULT ProcessRecvFilters(void *pvClient /*CSocketClient *pClient*/, const char *sBuf, unsigned int uiBufSz);

	bool ContainsCompressionFilter();

	bool Load(const char *sXMLFileName);
	bool Load(CXMLReader *xmlConfig, CCustomFilters *pDefaults);
	bool Reload(void);
	bool Destroy();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
