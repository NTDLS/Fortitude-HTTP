///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright � NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _Realms_Types_H
#define _Realms_Types_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define REALM_LIST_POS_PAGE		0
#define REALM_LIST_POS_DESCR		1

#define REALM_MAX_PAGE_LENGTH	MAX_PATH
#define REALM_MAX_DESCR_LENGTH	1024

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Realm{
	char *Page;
	char *Description;
	bool Enabled;

#ifndef HTTP_SERVICE
	bool ParentEnabled;
#endif

#ifdef HTTP_SERVICE
	int PageLength;
#endif //HTTP_SERVICE

} REALM, *LPREALM;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Realms{
	int Count;
	REALM *Items;
	bool Enabled;
} REALMS, *LPREALMS;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif