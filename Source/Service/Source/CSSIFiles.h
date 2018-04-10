///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CSSIFiles_H
#define _CSSIFiles_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../../@Libraries/CXML/CXMLReader.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/CLocks/CLocks.H"
#include "../../../../@Libraries/CStack/CStack.H"

#include "../../@Common/SSIFiles.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSSIFiles{

private:
	bool Initialized;
	void *pWebSites;
	SSIFILES Collection;
	char sFileName[MAX_PATH];

	typedef struct _tag_SSI_INFO {
		int NestingDepth;
	} SSIINFO, *LPSSIINFO;

	CLocks Locks;

	bool EchoSSIVar(VOID *pClient, CStringBuilder *lpBuf, char *sVarName);
	bool GetSSITagEx(VOID *pClient, CStringBuilder *lpBuf, int iInitRPos, const char *sSSI,
				 const int iSSISz, char *sOutBuffer, int iOutBufMaxSz,
				 const char *sOpt1, const char *sOpt2);
	bool GetVirtualOrFileName(VOID *pClient, CStringBuilder *lpBuf, int iInitRPos,
						   const char *sSSI, const int iSSISz, char *sOutFileName);
	bool GetCGIOrCMDName(VOID *pClient, CStringBuilder *lpBuf, int iInitRPos,
					  const char *sSSI, const int iSSISz, char *sOutFileName);

	bool ProcessServerSideIncludeEx(SSIINFO *pSSII, VOID *pClient, const char *sFileName);

	bool ProcessServerSideIncludeEx(SSIINFO *pSSII, VOID *pClient, char *sFileBuf,
					int iFileBufSz, bool bSend, CStringBuilder *lpBuf);

	bool ProcessServerSideIncludeEx(SSIINFO *pSSII, VOID *pClient,
		const char *sFileName, bool bSend, CStringBuilder *pBuffer);

public:
	~CSSIFiles();
	CSSIFiles(void *lpWebSites);
	CSSIFiles(void *lpWebSites, CXMLReader *xmlConfig, CSSIFiles *pDefaults);
	bool Save(void);

	bool ToXML(CXMLReader *lpXML);

	bool ProcessServerSideInclude(VOID *pClient, const char *sFileName);

	bool ProcessServerSideInclude(VOID *pClient, char *sFileBuf,
					int iFileBufSz, bool bSend, CStringBuilder *lpBuf);

	bool ProcessServerSideInclude(VOID *pClient,
		const char *sFileName, bool bSend, CStringBuilder *pBuffer);

	bool Load(const char *sXMLFileName);
	bool Reload(void);
	bool Load(CXMLReader *xmlConfig, CSSIFiles *pDefaults);
	bool Destroy();
	bool IsSSIFile(const char *sFileName);
	bool Enabled(void);

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
