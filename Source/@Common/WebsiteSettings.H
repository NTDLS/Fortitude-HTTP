///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _WebsiteSettings_Types_H
#define _WebsiteSettings_Types_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SETTINGS_LIST_POS_ATTRIB	0
#define SETTINGS_LIST_POS_VALUE		1

#define MAX_ADMIN_EMAIL				255
#define MAX_USER_LENGTH				1024
#define MAX_DOMAIN_LENGTH			1024
#define MAX_PASS_LENGTH				1024

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_WebsiteSettings {
    int ScriptTimeout;
    bool CacheLogFile;
	bool PreventImageLeaching;
    bool AllowDirectoryIndexing;
	bool AutoStart;
    int KeepAliveTimeout;
	int MaxReceiveSize;
	char LogPath[MAX_PATH];
	char ServerAdminEmail[MAX_ADMIN_EMAIL];

	char Username[MAX_USER_LENGTH];
	char Domain[MAX_DOMAIN_LENGTH];
	char Password[MAX_PASS_LENGTH];
} WEBSITESETTINGS, *LPWEBSITESETTINGS;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
