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

#include "../../../NSWFL/NSWFL.h"

#include "../../@Common/DefaultPages.h"

using namespace NSWFL::Isolation;
using namespace NSWFL::XML;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CDefaultPages{

private:
	bool Initialized;
	void *pWebSites;
	DEFAULTPAGES Collection;
	char sFileName[MAX_PATH];

	IsolationLock Locks;

public:
	~CDefaultPages(void);
	CDefaultPages(void *lpWebSites);
	CDefaultPages(void *lpWebSites, XMLReader *xmlConfig, CDefaultPages *pDefaults);
	bool Save(void);

	bool ToXML(XMLReader *lpXML);

	bool CheckPath(char *&sFullRequest);

	bool Load(const char *sXMLFileName);
	bool Load(XMLReader *xmlConfig, CDefaultPages *pDefaults);

	bool Reload(void);

	bool Destroy();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
