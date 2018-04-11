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

#include "../../../NSWFL/NSWFL.h"

#include "../../@Common/MimeTypes.h"

using namespace NSWFL::Isolation;
using namespace NSWFL::XML;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CMimeTypes{

private:
	bool Initialized;
	void *pWebSites;
	MIMETYPES Collection;
	char sFileName[MAX_PATH];

	IsolationLock Locks;

public:
	~CMimeTypes(void);
	CMimeTypes(void *lpWebSites);
	CMimeTypes(void *lpWebSites, XMLReader *xmlConfig, CMimeTypes *pDefaults);
	bool Save(void);

	bool ToXML(XMLReader *lpXML);

	bool Load(const char *sXMLFileName);
	bool Load(XMLReader *xmlConfig, CMimeTypes *pDefaults);
	bool Destroy();
	bool GetType(const char *sFileName, char *sMimeName, int iMaxMimeName);
	bool Reload(void);
	bool Export(const char *sFileName);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
