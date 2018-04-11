///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CServerSettings_H
#define _CServerSettings_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../NSWFL/NSWFL.h"

#include "../../@Common/ServerSettings.H"

using namespace NSWFL::Isolation;
using namespace NSWFL::XML;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CServerSettings {

private:
	bool Initialized;
	void *pWebSites;
	char sFileName[MAX_PATH];

	IsolationLock Locks;

public:
	SERVERSETTINGS Settings;

	~CServerSettings(void);
	CServerSettings(void *lpWebSites);
	CServerSettings(void *lpWebSites, XMLReader *xmlConfig, CServerSettings *pDefaults);
	bool Save(void);

	bool ToXML(XMLReader *lpXML);

	bool CheckLogin(const char *sUsername, const char *sPassword);
	bool Load(const char *sXMLFileName);
	bool Load(XMLReader *xmlConfig, CServerSettings *pDefaults);
	bool Reload(void);
	bool Destroy(void);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
