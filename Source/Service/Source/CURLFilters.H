///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CURLFilters_H
#define _CURLFilters_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../NSWFL/NSWFL.h"

#include "../../@Common/URLFilters.h"

using namespace NSWFL::Isolation;
using namespace NSWFL::XML;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CURLFilters{

private:
	bool Initialized;
	void *pWebSites;
	URLFILTERS Collection;
	char sFileName[MAX_PATH];

	IsolationLock Locks;

public:
	CURLFilters(void *lpWebSites);
	~CURLFilters(void);
	CURLFilters(void *lpWebSites, XMLReader *xmlConfig, CURLFilters *pDefaults);
	bool Save(void);

	bool ToXML(XMLReader *lpXML);

	bool ScanHeader(VOID *pHeader);
	bool Reload(void);
	bool Load(const char *sXMLFileName);
	bool Load(XMLReader *xmlConfig, CURLFilters *pDefaults);
	bool Destroy(void);
	bool Enabled(void);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
