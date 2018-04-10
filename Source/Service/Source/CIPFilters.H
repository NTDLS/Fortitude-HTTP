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

#include "../../../../@Libraries/CXML/CXMLReader.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/CLocks/CLocks.H"
#include "../../../../@Libraries/CStack/CStack.H"

#include "../../@Common/IPFilters.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CIPFilters{

private:
	bool Initialized;
	void *pWebSites;
	IPFILTERS Collection;
	char sFileName[MAX_PATH];

	CLocks Locks;

public:
	~CIPFilters(void);
	CIPFilters(void *lpWebSites);
	CIPFilters(void *lpWebSites, CXMLReader *xmlConfig, CIPFilters *pDefaults);
	bool Save(void);

	bool ToXML(CXMLReader *lpXML);

	bool IsIPAllowed(const char *sIP);
	bool Load(const char *sXMLFileName);
	bool Load(CXMLReader *xmlConfig, CIPFilters *pDefaults);
	bool Reload(void);
	bool Destroy(void);
	bool Enabled(void);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
