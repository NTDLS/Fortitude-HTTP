///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _WebSite_Types_H
#define _WebSite_Types_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define INVALID_SITE_INDEX			65535
#define INVALID_LIST_INDEX			65535

#define	MAX_SITE_NAME				512
#define MAX_SITE_DESCR				1024

#define WEBSITES_LIST_POS_ATTRIB	0
#define WEBSITES_LIST_POS_VALUE		1

#define WEBSITES_LIST_POS_NAME		0
#define WEBSITES_LIST_POS_BSENT		1
#define WEBSITES_LIST_POS_BRECV		2
#define WEBSITES_LIST_POS_HITS		3
#define WEBSITES_LIST_POS_CONNECTS	4

#define WEBSITES_LIST_POS_NAME		0
#define WEBSITES_LIST_POS_DESCR		1
#define WEBSITES_LIST_POS_ROOT		2
#define WEBSITES_LIST_POS_INDEX		3

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_WebSite{
	char *Name;
	char *Description;
	char *Root;
	char *SocketPoolID;
	bool IsActive;
	int Index;
} WEBSITE, *LPWEBSITE;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
