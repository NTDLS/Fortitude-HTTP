///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CMIMETYPES_H
#define _CMIMETYPES_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../../@Libraries/CXML/CXMLReader.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/CLocks/CLocks.H"
#include "../../../../@Libraries/CStack/CStack.H"

#include "../../@Common/MimeTypes.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CMimeTypes{

private:
	bool Initialized;
	void *pWebSites;
	MIMETYPES Collection;
	char sFileName[MAX_PATH];

	CLocks Locks;

public:
	~CMimeTypes(void);
	CMimeTypes(void *lpWebSites);
	CMimeTypes(void *lpWebSites, CXMLReader *xmlConfig, CMimeTypes *pDefaults);
	bool Save(void);

	bool ToXML(CXMLReader *lpXML);

	bool Load(const char *sXMLFileName);
	bool Load(CXMLReader *xmlConfig, CMimeTypes *pDefaults);
	bool Destroy();
	bool GetType(const char *sFileName, char *sMimeName, int iMaxMimeName);
	bool Reload(void);
	bool Export(const char *sFileName);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
