///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CErrorPages_H
#define _CErrorPages_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../../@Libraries/CXML/CXMLReader.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/CLocks/CLocks.H"
#include "../../../../@Libraries/CStack/CStack.H"

#include "../../@Common/ErrorPages.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CErrorPages{

private:
	bool Initialized;
	void *pWebSites;
	ERRORPAGES Collection;
	char sFileName[MAX_PATH];

	CLocks Locks;

	DWORD dwErrorCount;
	DWORD dwWarningCount;

public:
	CErrorPages(void *lpWebSites);
	~CErrorPages(void);
	CErrorPages(void *lpWebSites, CXMLReader *xmlConfig, CErrorPages *pDefaults);
	bool Save(void);

	bool ToXML(CXMLReader *lpXML);

	bool Reload(void);
	bool Load(const char *sXMLFileName);
	bool Load(CXMLReader *xmlConfig, CErrorPages *pDefaults);
	bool Destroy();

	DWORD WarningCount(void);
	DWORD ErrorCount(void);

	bool AuthorizationRequired(VOID *pClient);

	bool SendError(VOID *pClient, const char *sErrorCode);
	bool SendError(VOID *pClient, const char *sErrorCode, const char *sFormat, ...);
	bool SendErrorEx(VOID *pClient, ERRORPAGE *pErrorPage, const char *sDetails, const char *sExtraHeaders);
	bool SendFailsafeError(VOID *pClient, const char *sErrCode, const char *sErrDesc, const char *sDetails);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
