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

#include "../../../../@Libraries/CXML/CXMLReader.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/CLocks/CLocks.H"
#include "../../../../@Libraries/CStack/CStack.H"

#include "../../@Common/URLFilters.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CURLFilters{

private:
	bool Initialized;
	void *pWebSites;
	URLFILTERS Collection;
	char sFileName[MAX_PATH];

	CLocks Locks;

public:
	CURLFilters(void *lpWebSites);
	~CURLFilters(void);
	CURLFilters(void *lpWebSites, CXMLReader *xmlConfig, CURLFilters *pDefaults);
	bool Save(void);

	bool ToXML(CXMLReader *lpXML);

	bool ScanHeader(VOID *pHeader);
	bool Reload(void);
	bool Load(const char *sXMLFileName);
	bool Load(CXMLReader *xmlConfig, CURLFilters *pDefaults);
	bool Destroy(void);
	bool Enabled(void);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
