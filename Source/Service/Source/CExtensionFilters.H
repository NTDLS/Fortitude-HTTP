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

#include "../../../../@Libraries/CXML/CXMLReader.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/CLocks/CLocks.H"
#include "../../../../@Libraries/CStack/CStack.H"

#include "../../@Common/ExtentionFilters.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CExtensionFilters{

private:
	bool Initialized;
	void *pWebSites;
	EXTENSIONFILTERS Collection;
	char sFileName[MAX_PATH];

	CLocks Locks;

public:
	~CExtensionFilters(void);
	CExtensionFilters(void *lpWebSites);
	CExtensionFilters(void *lpWebSites, CXMLReader *xmlConfig, CExtensionFilters *pDefaults);
	bool Save(void);

	bool ToXML(CXMLReader *lpXML);

	bool CheckFile(const char *sString);
	bool Load(const char *sXMLFileName);
	bool Load(CXMLReader *xmlConfig, CExtensionFilters *pDefaults);
	bool Reload(void);

	bool Destroy(void);
	bool Enabled(void);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
