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

#include "../../../NSWFL/NSWFL.h"

#include "../../@Common/SSIFiles.h"

using namespace NSWFL::Isolation;
using namespace NSWFL::XML;
using namespace NSWFL::String;

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

	IsolationLock Locks;

	bool EchoSSIVar(VOID *pClient, StringBuilder *lpBuf, char *sVarName);
	bool GetSSITagEx(VOID *pClient, StringBuilder *lpBuf, int iInitRPos, const char *sSSI,
				 const int iSSISz, char *sOutBuffer, int iOutBufMaxSz,
				 const char *sOpt1, const char *sOpt2);
	bool GetVirtualOrFileName(VOID *pClient, StringBuilder *lpBuf, int iInitRPos,
						   const char *sSSI, const int iSSISz, char *sOutFileName);
	bool GetCGIOrCMDName(VOID *pClient, StringBuilder *lpBuf, int iInitRPos,
					  const char *sSSI, const int iSSISz, char *sOutFileName);

	bool ProcessServerSideIncludeEx(SSIINFO *pSSII, VOID *pClient, const char *sFileName);

	bool ProcessServerSideIncludeEx(SSIINFO *pSSII, VOID *pClient, char *sFileBuf,
					int iFileBufSz, bool bSend, StringBuilder *lpBuf);

	bool ProcessServerSideIncludeEx(SSIINFO *pSSII, VOID *pClient,
		const char *sFileName, bool bSend, StringBuilder *pBuffer);

public:
	~CSSIFiles();
	CSSIFiles(void *lpWebSites);
	CSSIFiles(void *lpWebSites, XMLReader *xmlConfig, CSSIFiles *pDefaults);
	bool Save(void);

	bool ToXML(XMLReader *lpXML);

	bool ProcessServerSideInclude(VOID *pClient, const char *sFileName);

	bool ProcessServerSideInclude(VOID *pClient, char *sFileBuf,
					int iFileBufSz, bool bSend, StringBuilder *lpBuf);

	bool ProcessServerSideInclude(VOID *pClient,
		const char *sFileName, bool bSend, StringBuilder *pBuffer);

	bool Load(const char *sXMLFileName);
	bool Reload(void);
	bool Load(XMLReader *xmlConfig, CSSIFiles *pDefaults);
	bool Destroy();
	bool IsSSIFile(const char *sFileName);
	bool Enabled(void);

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
