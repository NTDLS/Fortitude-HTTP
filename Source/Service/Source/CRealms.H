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

#include "../../../NSWFL/NSWFL.h"

#include "../../@Common/Realms.h"

using namespace NSWFL::Isolation;
using namespace NSWFL::XML;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CRealms{

private:
	bool Initialized;
	void *pWebSites;
	REALMS Collection;
	char sFileName[MAX_PATH];

	IsolationLock Locks;

public:
	~CRealms(void);
	CRealms(void *lpWebSites);
	CRealms(void *lpWebSites, XMLReader *xmlConfig, CRealms *pDefaults);
	bool Save(void);

	bool ToXML(XMLReader *lpXML);

	bool CheckPath(char *&sFullRequest);

	bool Load(const char *sXMLFileName);
	bool Load(XMLReader *xmlConfig, CRealms *pDefaults);

	bool Reload(void);

	bool Destroy();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
