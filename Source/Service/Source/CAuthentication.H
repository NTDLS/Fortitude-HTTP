///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CAuthentication_H
#define _CAuthentication_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../../@Libraries/CXML/CXMLReader.H"
#include "../../../../@Libraries/CXML/CXMLWriter.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/CLocks/CLocks.H"
#include "../../../../@Libraries/CStack/CStack.H"

#include "../../@Common/Authentication.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAuthentication{

private:
	bool Initialized;
	void *pWebSites;
	PROTECTEDPAGES Collection;
	char sFileName[MAX_PATH];

	CLocks Locks;

public:
	~CAuthentication(void);
	CAuthentication(void *lpWebSites);
	CAuthentication(void *lpWebSites, CXMLReader *xmlConfig, CAuthentication *pDefaults);
	bool Save(void);

	bool ToXML(CXMLReader *lpXML);

	bool IsAuthorizationRequired(VOID *pClient);
	bool IsAuthenticated(VOID *pClient);
	bool Authenticate(VOID *pClient);

	bool Load(const char *sXMLFileName);
	bool Load(CXMLReader *xmlConfig, CAuthentication *pDefaults);

	bool Reload(void);

	bool Destroy();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
