///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CIPFilters_H
#define _CIPFilters_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../NSWFL/NSWFL.h"

#include "../../@Common/IPFilters.H"

using namespace NSWFL::Isolation;
using namespace NSWFL::XML;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CIPFilters{

private:
	bool Initialized;
	void *pWebSites;
	IPFILTERS Collection;
	char sFileName[MAX_PATH];

	IsolationLock Locks;

public:
	~CIPFilters(void);
	CIPFilters(void *lpWebSites);
	CIPFilters(void *lpWebSites, XMLReader *xmlConfig, CIPFilters *pDefaults);
	bool Save(void);

	bool ToXML(XMLReader *lpXML);

	bool IsIPAllowed(const char *sIP);
	bool Load(const char *sXMLFileName);
	bool Load(XMLReader *xmlConfig, CIPFilters *pDefaults);
	bool Reload(void);
	bool Destroy(void);
	bool Enabled(void);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
