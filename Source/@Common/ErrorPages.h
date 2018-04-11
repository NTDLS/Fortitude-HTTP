///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright � NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _ErrorPages_Types_H
#define _ErrorPages_Types_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define ERRORPAGES_LIST_POS_CODE			0
#define ERRORPAGES_LIST_POS_RETURNCODE		1
#define ERRORPAGES_LIST_POS_DESCR			2
#define ERRORPAGES_LIST_POS_FILE			3

#define ERRORPAGES_MAX_CODE_LENGTH			10
#define ERRORPAGES_MAX_RETURNCODE_LENGTH	10
#define ERRORPAGES_MAX_FILE_LENGTH			MAX_PATH
#define ERRORPAGES_MAX_DESCR_LENGTH			1024

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_ErrorPage{
	char *Code;
	char *ReturnCode;
	char *File;
	char *Description;
	bool Enabled;

#ifndef HTTP_SERVICE
	bool ParentEnabled;
#endif

} ERRORPAGE, *LPERRORPAGE;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_ErrorPages{
	int Count;
	ERRORPAGE *Items;
	char DefaultFile[MAX_PATH];
} ERRORPAGES, *LPERRORPAGES;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif