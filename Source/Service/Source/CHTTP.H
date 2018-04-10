///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CHttp_H
#define _CHttp_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CWebSite.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CHttp{

private:
	VOID *pWebSites;

public:
	CHttp(VOID *lpWebSites);
	~CHttp(void);

	int GetHttpHeaderTag(const char *sHeader, const int iHeaderLen, const char *sTag, SYSTEMTIME *pOutST, int *iFurthestExtent);
	int GetHttpHeaderTag(const char *sHeader, const int iHeaderLen, const char *sTag, char *&sBuf, int *iFurthestExtent);
	int GetHttpHeaderTag(const char *sHeader, const int iHeaderLen, const char *sTag, char *sBuf, int iMaxBuf, int *iFurthestExtent);
	int GetHttpHeaderTag(const char *sHeader, const int iHeaderLen, const char *sTag, char *sBuf, int iMaxBuf, bool bPartialOk, int *iFurthestExtent);
	bool DoesHeaderContainTag(const char *sBuf, const int iBufLen, const char *sTag);
	bool DoesHeaderContainTag(const char *sBuf, const int iBufLen, const char *sTag, int *iHeaderEndPos);
	bool IsValueEqual(const char *sValue1, const char *sValue2);
	bool DoesValueContain(const char *sIn, const char *sValue);
	bool SendRawFileEx(PEER *pC, const char *sFileName, char *sBuffer, unsigned int iBufferSz);
	bool SendFileWithHeader(PEER *pC, const char *sFileName);
	bool SendRawFile(PEER *pC, const char *sFileName);
	bool ResponseRedirect(PEER *pC, const char *sURL, bool bPermanent);

	bool SendHttpHeader(PEER *pC, const char *sLastModified, const char *sErrCode, const char *sErrDesc,
		const char *sEncoding, const char *sMimeType, __int64 i64ContentLength,
		const char *sExtraHeaders, bool bTerminateHeader);
	bool BuildHttpHeader(PEER *pC, CStringBuilder *sOutHeader, const char *sLastModified, const char *sErrCode, const char *sErrDesc,
		const char *sEncoding, const char *sMimeType, __int64 i64ContentLength,
		const char *sExtraHeaders, bool bTerminateHeader);

	bool SendOKHeader(PEER *pC, const char *sLastModified, __int64 i64FileSize);
	bool SendOKHeader(PEER *pC, const char *sLastModified, const char *sMimeType, const char *sEncoding, __int64 i64FileSize);
	bool SendOKHeader(PEER *pC, const char *sLastModified, const char *sMimeType, const char *sEncoding, __int64 i64FileSize, bool bTerminateHeader);
	bool SendOKHeader(PEER *pC, const char *sLastModified, const char *sMimeType, __int64 i64FileSize);

	bool BufferDataToClient(PEER *pC, const char *sData, unsigned int iDataLength);
	bool BufferDataToClient(PEER *pC, CStringBuilder *sData);
	bool FreeClientHeader(PEER *pC);
	bool ParseHTTPHeader(CWebSite *pWebSite, PEER *pC, const char *sBuf, const int iBufLen);
	bool GetHeaderRequestRange(CWebSite *pWebSite, PEER *pC, const char *sBuf, const int iBufLen);
	bool AssembleFullRequest(CWebSite *pWebSite, PEER *pC);
	void FillInPathInfoEnvironment(CWebSite *pWebSite, PEER *pC, ROOT *pVirtualRoot);
	bool ProcessRequest(CWebSite *pWebSite, PEER *pC);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

