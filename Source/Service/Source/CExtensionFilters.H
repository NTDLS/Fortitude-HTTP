///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CExtensionFilters_H
#define _CExtensionFilters_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../NSWFL/NSWFL.h"

#include "../../@Common/ExtentionFilters.h"

using namespace NSWFL::Isolation;
using namespace NSWFL::XML;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CExtensionFilters{

private:
	bool Initialized;
	void *pWebSites;
	EXTENSIONFILTERS Collection;
	char sFileName[MAX_PATH];

	IsolationLock Locks;

public:
	~CExtensionFilters(void);
	CExtensionFilters(void *lpWebSites);
	CExtensionFilters(void *lpWebSites, XMLReader *xmlConfig, CExtensionFilters *pDefaults);
	bool Save(void);

	bool ToXML(XMLReader *lpXML);

	bool CheckFile(const char *sString);
	bool Load(const char *sXMLFileName);
	bool Load(XMLReader *xmlConfig, CExtensionFilters *pDefaults);
	bool Reload(void);

	bool Destroy(void);
	bool Enabled(void);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
