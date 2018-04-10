///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CustomFilters_Types_H
#define _CustomFilters_Types_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CUSTOMFILTERS_LIST_POS_DESCR	0
#define CUSTOMFILTERS_LIST_POS_DLL		1

#define CUSTOMFILTERS_MAX_DLL_LENGTH	MAX_PATH
#define CUSTOMFILTERS_MAX_DESCR_LENGTH	1024

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Custom_Filter{
	char *DLL;
	char *Description;
	bool Enabled;

#ifndef HTTP_SERVICE
	bool ParentEnabled;
#endif

#ifdef HTTP_SERVICE
	HMODULE ModuleHandle;

	LPOnProcessRawResponseHeader OnProcessRawResponseHeader;
	LPOnProcessRequestHeader OnProcessRequestHeader;
	LPOnUrlMap OnUrlMap;
	LPOnSendRawData OnSendRawData;
	LPOnRecvRawData OnRecvRawData;
	LPOnCompressData OnCompressData;
	LPOnLog OnLog;
	LPOnConnect OnConnect;
	LPOnDisconnect OnDisconnect;
	LPOnRequestConcluded OnRequestConcluded;
#endif //HTTP_SERVICE

} CUSTOMFILTER, *LPCUSTOMFILTER;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Custom_Filters{
	int Count;
	CUSTOMFILTER *Items;
	bool Enabled;
} CUSTOMFILTERS, *LPCUSTOMFILTERS;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
