///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CDefaultPages_H
#define _CDefaultPages_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../../@Libraries/CXML/CXMLReader.H"
#include "../../../../@Libraries/CXML/CXMLWriter.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/CLocks/CLocks.H"
#include "../../../../@Libraries/CStack/CStack.H"

#include "../../@Common/DefaultPages.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CDefaultPages{

private:
	bool Initialized;
	void *pWebSites;
	DEFAULTPAGES Collection;
	char sFileName[MAX_PATH];

	CLocks Locks;

public:
	~CDefaultPages(void);
	CDefaultPages(void *lpWebSites);
	CDefaultPages(void *lpWebSites, CXMLReader *xmlConfig, CDefaultPages *pDefaults);
	bool Save(void);

	bool ToXML(CXMLReader *lpXML);

	bool CheckPath(char *&sFullRequest);

	bool Load(const char *sXMLFileName);
	bool Load(CXMLReader *xmlConfig, CDefaultPages *pDefaults);

	bool Reload(void);

	bool Destroy();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
