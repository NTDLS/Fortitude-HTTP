///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright � NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _SSIFiles_Types_H
#define _SSIFiles_Types_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SSIFILES_LIST_POS_EXT	0
#define SSIFILES_LIST_POS_DESCR	1

#define SSIFILES_MAX_EXT_LENGTH		MAX_PATH
#define SSIFILES_MAX_DESCR_LENGTH	1024

#define SSI_BEGIN_FLAG			"<!--#"
#define SSI_END_FLAG			"-->"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_SSI_File{
	char *Extension;
	char *Description;
	bool Enabled;

#ifndef HTTP_SERVICE
	bool ParentEnabled;
#endif

} SSIFILE, *LPSSIFILE;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_SSI_Files{
	int Count;
	SSIFILE *Items;
	bool Enabled;
} SSIFILES, *LPSSIFILES;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif