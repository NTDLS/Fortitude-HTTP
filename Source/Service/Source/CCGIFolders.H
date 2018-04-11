///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CCGIFolders_H
#define _CCGIFolders_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../NSWFL/NSWFL.h"

#include "../../@Common/CGIFolders.h"

using namespace NSWFL::Isolation;
using namespace NSWFL::XML;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCGIFolders{

private:
	bool Initialized;
	void *pWebSites;
	CGIFOLDERS Collection;
	char sFileName[MAX_PATH];

	IsolationLock Locks;

public:
	~CCGIFolders(void);
	CCGIFolders(void *lpWebSites);
	CCGIFolders(void *lpWebSites, XMLReader *xmlConfig, CCGIFolders *pDefaults);
	bool Save(void);
	
	bool IsInCGIFolder(VOID *pClient);
	bool Load(const char *sXMLFileName);
	bool Load(XMLReader *xmlConfig, CCGIFolders *pDefaults);
	bool Reload(void);
	bool Destroy(void);
	bool Enabled(void);
	bool ToXML(XMLReader *lpXML);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
