///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright � NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CGIFolders_Types_H
#define _CGIFolders_Types_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CGIFOLDERS_LIST_POS_PATH		0
#define CGIFOLDER_LIST_POS_DESCR	1

#define CGIFOLDER_MAX_PATH_LENGTH	MAX_PATH
#define CGIFOLDER_MAX_DESCR_LENGTH	1024

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_CGI_Folder{
	char *Path; //relative to root (e.g. /cgi-bin)
	bool Enabled;
	char *Description;

#ifndef HTTP_SERVICE
	bool ParentEnabled;
#endif

#ifdef HTTP_SERVICE
	int PathLength;
#endif //HTTP_SERVICE

} CGIFOLDER, *LPCGIFOLDER;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_CGI_Folders{
	int Count;
	CGIFOLDER *Items;
	bool Enabled;
} CGIFOLDERS, *LPCGIFOLDERS;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
