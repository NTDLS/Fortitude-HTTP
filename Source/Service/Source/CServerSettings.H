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

#include "../../../../@Libraries/CXML/CXMLReader.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/CLocks/CLocks.H"
#include "../../../../@Libraries/CStack/CStack.H"

#include "../../@Common/ServerSettings.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CServerSettings {

private:
	bool Initialized;
	void *pWebSites;
	char sFileName[MAX_PATH];

	CLocks Locks;

public:
	SERVERSETTINGS Settings;

	~CServerSettings(void);
	CServerSettings(void *lpWebSites);
	CServerSettings(void *lpWebSites, CXMLReader *xmlConfig, CServerSettings *pDefaults);
	bool Save(void);

	bool ToXML(CXMLReader *lpXML);

	bool CheckLogin(const char *sUsername, const char *sPassword);
	bool Load(const char *sXMLFileName);
	bool Load(CXMLReader *xmlConfig, CServerSettings *pDefaults);
	bool Reload(void);
	bool Destroy(void);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
