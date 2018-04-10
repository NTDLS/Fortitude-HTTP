///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _VirtualRoots_Types_H
#define _VirtualRoots_Types_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define VIRTUALROOTS_LIST_POS_ROOT		0
#define VIRTUALROOTS_LIST_POS_PATH		1
#define VIRTUALROOTS_LIST_POS_DESCR		2
#define VIRTUALROOTS_LIST_POS_USER		3
#define VIRTUALROOTS_LIST_POS_DOMAIN	4
#define VIRTUALROOTS_LIST_POS_PASS		5

#define VIRTUALROOTS_MAX_ROOT_LENGTH	MAX_PATH
#define VIRTUALROOTS_MAX_PATH_LENGTH	MAX_PATH
#define VIRTUALROOTS_MAX_DESCR_LENGTH	1024
#define VIRTUALROOTS_MAX_USER_LENGTH	1024
#define VIRTUALROOTS_MAX_DOMAIN_LENGTH	1024
#define VIRTUALROOTS_MAX_PASS_LENGTH	1024

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Root{
	char *Root;
	char *Path;
	char *Description;
	bool Enabled;

	char *Username;
	char *Domain;
	char *Password;

#ifndef HTTP_SERVICE
	bool ParentEnabled;
#endif

#ifdef HTTP_SERVICE
	int PathLength;
	int RootLength;
#endif //HTTP_SERVICE

} ROOT, *LPROOT;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Roots{
	int Count;
	ROOT *Items;
	bool Enabled;
} ROOTS, *LPROOTS;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
