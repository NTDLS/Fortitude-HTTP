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

#include "../../../../@Libraries/CXML/CXMLReader.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/CLocks/CLocks.H"
#include "../../../../@Libraries/CStack/CStack.H"

#include "../../@Common/VirtualRoots.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CVirtualRoots{

private:
	bool Initialized;
	void *pWebSites;
	ROOTS Collection;
	char sFileName[MAX_PATH];

	CLocks Locks;

public:
	CVirtualRoots(void *lpWebSites);
	CVirtualRoots(void *lpWebSites, CXMLReader *xmlConfig, CVirtualRoots *pDefaults);
	~CVirtualRoots(void);
	bool Save(void);

	bool ToXML(CXMLReader *lpXML);

	ROOT *IsVirtualRoot(const char *sRequest/*, char *&sFullRequest*/);
	bool Reload(void);
	bool Load(const char *sXMLFileName);
	bool Load(CXMLReader *xmlConfig, CVirtualRoots *pDefaults);
	bool Destroy();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
