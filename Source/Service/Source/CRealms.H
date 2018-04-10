///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CRealms_H
#define _CRealms_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../../@Libraries/CXML/CXMLReader.H"
#include "../../../../@Libraries/CXML/CXMLWriter.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/CLocks/CLocks.H"
#include "../../../../@Libraries/CStack/CStack.H"

#include "../../@Common/Realms.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CRealms{

private:
	bool Initialized;
	void *pWebSites;
	REALMS Collection;
	char sFileName[MAX_PATH];

	CLocks Locks;

public:
	~CRealms(void);
	CRealms(void *lpWebSites);
	CRealms(void *lpWebSites, CXMLReader *xmlConfig, CRealms *pDefaults);
	bool Save(void);

	bool ToXML(CXMLReader *lpXML);

	bool CheckPath(char *&sFullRequest);

	bool Load(const char *sXMLFileName);
	bool Load(CXMLReader *xmlConfig, CRealms *pDefaults);

	bool Reload(void);

	bool Destroy();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
