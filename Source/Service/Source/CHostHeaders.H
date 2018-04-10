///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CHostHeaders_H
#define _CHostHeaders_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../../@Libraries/CXML/CXMLReader.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/CLocks/CLocks.H"
#include "../../../../@Libraries/CStack/CStack.H"

#include "../../@Common/HostHeaders.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CHostHeaders{

private:
	bool Initialized;
	void *pWebSites;
	CLocks Locks;

public:
	HOSTHEADERS Collection;

	~CHostHeaders(void);
	CHostHeaders(void *lpWebSites, CXMLReader *xmlConfig);
	CHostHeaders(void *lpWebSites);

	bool ToXML(CXMLReader *lpXML);

	int ActiveCount(void);

	bool Load(CXMLReader *xmlConfig);
	bool Destroy(void);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
