///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CVirtualRoots_H
#define _CVirtualRoots_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../NSWFL/NSWFL.h"

#include "../../@Common/VirtualRoots.h"

using namespace NSWFL::Isolation;
using namespace NSWFL::XML;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CVirtualRoots{

private:
	bool Initialized;
	void *pWebSites;
	ROOTS Collection;
	char sFileName[MAX_PATH];

	IsolationLock Locks;

public:
	CVirtualRoots(void *lpWebSites);
	CVirtualRoots(void *lpWebSites, XMLReader *xmlConfig, CVirtualRoots *pDefaults);
	~CVirtualRoots(void);
	bool Save(void);

	bool ToXML(XMLReader *lpXML);

	ROOT *IsVirtualRoot(const char *sRequest/*, char *&sFullRequest*/);
	bool Reload(void);
	bool Load(const char *sXMLFileName);
	bool Load(XMLReader *xmlConfig, CVirtualRoots *pDefaults);
	bool Destroy();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
