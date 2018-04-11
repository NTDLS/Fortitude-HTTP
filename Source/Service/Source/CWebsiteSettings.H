///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CWebsiteSettings_H
#define _CWebsiteSettings_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../NSWFL/NSWFL.h"

#include "../../@Common/WebsiteSettings.H"

using namespace NSWFL::Isolation;
using namespace NSWFL::XML;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CWebsiteSettings{

private:
	bool Initialized;
	void *pWebSites;
	WEBSITESETTINGS Collection;
	char sFileName[MAX_PATH];

	IsolationLock Locks;

public:
	CWebsiteSettings(void *lpWebSites);
	~CWebsiteSettings(void);
	CWebsiteSettings(void *lpWebSites, XMLReader *xmlConfig, CWebsiteSettings *pDefaults);
	bool Save(void);

	bool ToXML(XMLReader *lpXML);

	bool AutoStart(void);
	int ScriptTimeout(void);
	bool CacheLogFile(void);
	bool PreventImageLeaching(void);
	bool AllowDirectoryIndexing(void);
	int KeepAliveTimeout(void);
	char *LogPath(void);
	char *ServerAdminEmail(void);
	int MaxReceiveSize(void);
	char *Username(void);
	char *Domain(void);
	char *Password();

	bool Reload(void);
	bool Load(const char *sXMLFileName);
	bool Load(XMLReader *xmlConfig, CWebsiteSettings *pDefaults);
	bool Destroy();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
