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

#include "../../../NSWFL/NSWFL.h"

#include "../../@Common/Authentication.h"

using namespace NSWFL::Isolation;
using namespace NSWFL::XML;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CAuthentication{

private:
	bool Initialized;
	void *pWebSites;
	PROTECTEDPAGES Collection;
	char sFileName[MAX_PATH];

	IsolationLock Locks;

public:
	~CAuthentication(void);
	CAuthentication(void *lpWebSites);
	CAuthentication(void *lpWebSites, XMLReader *xmlConfig, CAuthentication *pDefaults);
	bool Save(void);

	bool ToXML(XMLReader *lpXML);

	bool IsAuthorizationRequired(VOID *pClient);
	bool IsAuthenticated(VOID *pClient);
	bool Authenticate(VOID *pClient);

	bool Load(const char *sXMLFileName);
	bool Load(XMLReader *xmlConfig, CAuthentication *pDefaults);

	bool Reload(void);

	bool Destroy();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
