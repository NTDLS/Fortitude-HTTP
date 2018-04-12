///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CHttp_CPP
#define _CHttp_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <WindowsX.H>
#include <ShellAPI.H>
#include <Stdio.H>
#include <Stdlib.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../@Common/Cryptography.h"
#include "../../../NSWFL/NSWFL.h"
#include "../../../NASCCL/NASCCL.H"

#include "Entry.H"
#include "CHttp.H"
#include "CWebSites.H"
#include "CDirectoryIndexing.H"
#include "../../../Setup/Filters/Filter Example/Source/HTTPFilter.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::File;
using namespace NSWFL::ListView;
using namespace NSWFL::DateTime;
using namespace NSWFL::Hashing;
using namespace NSWFL::Conversion;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CHttp::CHttp(VOID *lpWebSites)
{
	pWebSites = lpWebSites;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CHttp::~CHttp(void)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Called by [Socket_ClientHandler()]

	Return TRUE to keep the client connected (if applicable) otherwise FALSE.
*/
bool CHttp::ProcessRequest(CWebSite *pWebSite, PEER *pC)
{
	if(!((CWebSite*)pC->pWebSite)->pURLFilters->ScanHeader(&pC->Header))
	{
		pWebSite->pErrorPages->SendError(pC, "401", "Authorization failed by filter");
		return true;
	}

	if(!pWebSite->pExtensionFilters->CheckFile(pC->Header.Request))
	{
		pWebSite->pErrorPages->SendError(pC, "401", "Authorization failed by filter");
		return true;
	}

	if(this->AssembleFullRequest(pWebSite, pC))
	{
		if(pWebSite->pAuthentication->IsAuthorizationRequired(pC))
		{
			return true; //Do not proccess the request at this time.
		}

		if(IsDirectory(pC->Header.FullRequest))
		{
			CDirectoryIndexing DI;
			if(!DI.SendContents(pC, pC->Header.FullRequest))
			{
				char sErrorMessage[1024];
				if(GetLastError(sErrorMessage, sizeof(sErrorMessage)))
				{
					pWebSite->pErrorPages->SendError(pC, "500", sErrorMessage);
				}
				else{
					pWebSite->pErrorPages->SendError(pC, "500", "Unknown logon error");
				}
			}
		}
		else {
			if(pWebSite->pScriptingEngines->IsScriptFile(pC->Header.FullRequest))
			{
				int iResult = EXEC_RESULT_OK;
				if((iResult = pWebSite->pScriptingEngines->ProcessScript(pC)) != EXEC_RESULT_OK)
				{
					pWebSite->pErrorPages->SendError(pC, "500", pWebSite->pScriptingEngines->ErrorString(iResult));
				}
			}
			else if(pWebSite->pSSIFiles->IsSSIFile(pC->Header.FullRequest))
			{
				if(!pWebSite->pSSIFiles->ProcessServerSideInclude(pC, pC->Header.FullRequest))
				{
					pWebSite->pErrorPages->SendError(pC, "500");
				}
			}
			else if(pWebSite->pCGIFolders->IsInCGIFolder(pC))
			{
				if(pWebSite->pScriptingEngines->ProcessScript(pC) != EXEC_RESULT_OK)
				{
					pWebSite->pErrorPages->SendError(pC, "500");
				}
			}
			else{
				if(!this->SendFileWithHeader(pC, pC->Header.FullRequest))
				{
					//pWebSite->pErrorPages->SendError(pC, "500"); this error is sent by SendFileWithHeader();
				}
			}
		}
	}

	return true; //No reason to force a disconnect.
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHttp::SendRawFileEx(PEER *pC, const char *sFileName, char *sBuffer, unsigned int iBufferSz)
{
	bool bResult = true;
	DWORD dwBytesRead = 0;
    int iBytesRead = 0;
	__int64 i64FileSize = 0;
	__int64 i64BytesRead = 0;
	__int64 i64MaxSend = (pC->Header.ContentEnd - pC->Header.ContentBegin);

	HANDLE hFile = CreateFile(sFileName,
		GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	if(!FileSize(hFile, &i64FileSize))
	{
		return false;
	}

	if(pC->Header.ContentBegin > 0)
	{
		FileSeek(hFile, pC->Header.ContentBegin, FILE_BEGIN);
	}

	if(i64MaxSend > 0)
	{
		i64MaxSend++;
	}

	do{
		if(i64MaxSend > 0 && (i64BytesRead + iBufferSz) > i64MaxSend)
		{
			iBufferSz = (unsigned long)(i64MaxSend - i64BytesRead);
		}

		ReadFile(hFile, sBuffer, iBufferSz, &dwBytesRead, NULL);

		i64BytesRead += dwBytesRead;

		if(!pC->pClient->SetNextSendData(sBuffer, dwBytesRead))
		{
			return false;
		}

		if(i64MaxSend > 0 && i64BytesRead == i64MaxSend)
		{
			break;
		}

	} while(dwBytesRead == iBufferSz && iBufferSz > 0 && pC->pClient->IsConnected());

	CloseHandle(hFile);

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHttp::SendRawFile(PEER *pC, const char *sFileName)
{
	__int64 i64FileSize = 0;

	if(FileSize(sFileName, &i64FileSize))
	{
		unsigned int iBufferSize = ((CWebSite*)pC->pWebSite)->pSocketPool->FileBufferSize;
		if(i64FileSize < iBufferSize)
		{
			iBufferSize = (int) i64FileSize;
		}
		
		char *sBuffer = (char *) pMem->Allocate(sizeof(char), iBufferSize + 1);
		if(sBuffer)
		{
			bool bResult = this->SendRawFileEx(pC, sFileName, sBuffer, iBufferSize);
			pMem->Free(sBuffer);
			return bResult;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHttp::SendFileWithHeader(PEER *pC, const char *sFileName)
{
	CWebSite *pWebSite = (CWebSite *)pC->pWebSite;
	CHttp *pHttp = (CHttp *)pWebSite->pHttp;
	CWebSites *pWebSites = (CWebSites *)pWebSite->pWebSites;

	if(!FileAccess(sFileName, FExist))
	{
		pWebSite->pErrorPages->SendError(pC, "404");
		return false;
	}

	char *sEncoding = NULL;
	char sSendFile[MAX_PATH];

	if(!this->IsValueEqual(pC->Header.CacheControl, "no-cache"))
	{
		if(this->DoesValueContain(pC->Header.AcceptEncoding, "gzip"))
		{
			if(((CWebSite*)pC->pWebSite)->pCompression->IsFileCompressible(sFileName))
			{
				if(((CWebSite*)pC->pWebSite)->pCompression->CompressFile(pC, sFileName, sSendFile, sizeof(sSendFile)))
				{
					sEncoding = "gzip";
				}
			}
		}
	}
	
	if(!sEncoding) //If the file is not compressed.
	{
		strcpy_s(sSendFile, sizeof(sSendFile), sFileName);
	}

	__int64 i64FileSize = 0;

	if(FileSize(sSendFile, &i64FileSize))
	{
		char sMimeType[MIMETYPES_MAX_TYPE_LENGTH];

		if(!((CWebSite*)pC->pWebSite)->pMimeTypes->GetType(sFileName, sMimeType, sizeof(sMimeType)))
		{
			return false;
		}

		char sLastModified[128];
		sLastModified[0] = '\0';

		SYSTEMTIME lastModified;
		if(GetLastFileModificationDateGMT(sFileName, &lastModified))
		{
			MakeGMTTimeString(&lastModified, sLastModified, sizeof(sLastModified));
		}

		bool bNotModified = false;

		if(pC->Header.IfModifiedSince.wYear != 0)
		{
			int iNotModified = CompareDateTime(&pC->Header.IfModifiedSince, &lastModified);
			bNotModified = (iNotModified >= 0);
		}

		if(bNotModified)
		{
			if(this->SendHttpHeader(pC, sLastModified, "304", "Not Modified", NULL, NULL, -1, NULL, true))
			{
				return true;
			}
		}
		else if(this->SendOKHeader(pC, sLastModified, sMimeType, sEncoding, i64FileSize))
		{
			if((pC->Header.Method != NULL && _strcmpi(pC->Header.Method, "HEAD") != 0) || pC->Header.Method == NULL)
			{
				unsigned int iBufferSize = ((CWebSite*)pC->pWebSite)->pSocketPool->FileBufferSize;
				if(i64FileSize < iBufferSize)
				{
					iBufferSize = (int) i64FileSize;
				}
				
				if(iBufferSize > 0)
				{
					char *sBuffer = (char *) pMem->Allocate(sizeof(char), iBufferSize + 1);
					if(sBuffer)
					{
						bool bResult = this->SendRawFileEx(pC, sSendFile, sBuffer, iBufferSize);
						pMem->Free(sBuffer);
						return bResult;
					}
				}
				else{
					return true; //Zero-length file.
				}
			}
			else{
				return true; //Only sending header.
			}
		}
	}

	pWebSite->pErrorPages->SendError(pC, "500");
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHttp::ResponseRedirect(PEER *pC, const char *sURL, bool bPermanent)
{
	StringBuilder body;
	StringBuilder location;

	body.AppendF("<head><title>Document Moved</title></head>"
		"<body><h1>Object have been %s moved</h1>This document may be found <a href=\"%s\">here</a></body>",
		(bPermanent ? "permanently" : "temporarily"),
		sURL);

	location.AppendF("Location: %s\r\n", sURL);

	if(SendHttpHeader(pC, NULL,
			(bPermanent ? "301" : "307"),
			(bPermanent ? "Moved Permanently" : "Temporary Redirect"),
			NULL, "text/html", body.Length, location.Buffer, true))
	{
		return BufferDataToClient(pC, &body);
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHttp::BuildHttpHeader(PEER *pC, StringBuilder *sOutHeader, const char *sLastModified, const char *sErrCode,
						   const char *sErrDesc, const char *sEncoding,
						   const char *sMimeType, __int64 i64ContentLength, //Set i64ContentLength less than 0 to omit from the response header.
						   const char *sExtraHeaders, bool bTerminateHeader)
{
	if(((CWebSite *)pC->pWebSite)->pWebsiteSettings->PreventImageLeaching())
	{
		if(sMimeType && _strnicmp(sMimeType, "image/", 6) == 0)
		{
			if(!pC->Header.Referer || !pC->Header.Host || InStrI(pC->Header.Host, pC->Header.Referer) < 0)
			{
				((CWebSite *)pC->pWebSite)->pErrorPages->SendError(pC, "403", "Direct image linking is not allowed");
				return false;
			}
		}
	}

	char sGMT[128];
	if(!MakeGMTTimeString(sGMT, sizeof(sGMT)))
	{
		return false;
	}

	if(sErrCode && sErrDesc)
	{
		sOutHeader->AppendF("%s %s %s\r\n", HTTP_VERSION, sErrCode, sErrDesc);
	}
	else if(sErrCode)
	{
		sOutHeader->AppendF("%s %s\r\n", HTTP_VERSION, sErrCode);
	}

	if(sMimeType && sMimeType[0] != 0) {
		sOutHeader->AppendF("Content-Type: %s\r\n", sMimeType);
	}
	if(sEncoding && sEncoding[0] != 0) {
		sOutHeader->AppendF("Content-Encoding: %s\r\n", sEncoding);
	}
	if(pC->Header.Cookie && pC->Header.Cookie[0] != 0) {
		sOutHeader->AppendF("Cookie: %s\r\n", pC->Header.Cookie);
	}

	sOutHeader->AppendF("Server: NetworkDLS - %s\r\n", gsTitleCaption);
	
	if(pC->KeepAlive)
	{
		sOutHeader->Append("Connection: Keep-Alive\r\n");
	}
	else{
		sOutHeader->Append("Connection: Close\r\n");
	}

	if(sExtraHeaders)
	{
		sOutHeader->Append(sExtraHeaders);
	}

	//We don't support content ranges on dynamic content.
	if(!pC->PerRequestStore.IsDynamicContent)
	{
		sOutHeader->Append("Accept-Ranges: bytes\r\n");

		if((pC->Header.ContentBegin != 0 || pC->Header.ContentEnd) && !pC->PerRequestStore.IsDynamicContent)
		{
			bool bAddOneByte = false;

			if(pC->Header.ContentEnd <= 0)
			{
				pC->Header.ContentEnd = i64ContentLength - 1;
				bAddOneByte = true;
			}

			sOutHeader->AppendF("Content-Range: bytes %I64d-%I64d/%I64d\r\n",
				pC->Header.ContentBegin, pC->Header.ContentEnd, i64ContentLength);

			i64ContentLength = (pC->Header.ContentEnd - pC->Header.ContentBegin);

			if(bAddOneByte)
			{
				i64ContentLength++;
			}
		}
	}

	if(i64ContentLength >= 0)
	{
		sOutHeader->AppendF("Content-Length: %I64d\r\n", i64ContentLength);
	}

	sOutHeader->AppendF("Date: %s\r\n", sGMT);

	if(sLastModified && strlen(sLastModified) > 0)
	{
		sOutHeader->AppendF("Last-Modified: %s\r\n", sLastModified);
	}

	if(bTerminateHeader)
	{
		sOutHeader->Append("\r\n");
	}

	//strcat_s(sHdrStr, sizeof(sHdrStr), "Transfer-Encoding: chunked\r\n");

	//Save the clients last response information.
	strncpy_s(pC->PerRequestStore.LastResponse.ResponseCode, sizeof(pC->PerRequestStore.LastResponse.ResponseCode), sErrCode, sizeof(pC->PerRequestStore.LastResponse.ResponseCode) - 1);
	pC->PerRequestStore.LastResponse.ContentLength = i64ContentLength;

	int iResponseCodeLength = CharIndex(pC->PerRequestStore.LastResponse.ResponseCode, ' ');
	if(iResponseCodeLength > 0)
	{
		pC->PerRequestStore.LastResponse.ResponseCode[iResponseCodeLength] = '\0';
	}

	if(!IsNumeric(pC->PerRequestStore.LastResponse.ResponseCode))
	{
		return false;
	}
/*
#ifdef _DEBUG
	printf("--------------------------------------------------------------------------\n");
	printf("%s\n", pC->Header.Request);
	printf("--------------------------------------------------------------------------\n");
	printf("%s\n", sOutHeader->Buffer);
#endif
*/
	bool bResult = true;

	FILTERACTION filterAction;
	memset(&filterAction, 0, sizeof(filterAction));

	FILTERESULT filterResult = ((CWebSite *)pC->pWebSite)->pCustomFilters->ProcessRawResponseHeader(pC, sOutHeader, &filterAction);

	if(filterResult == FILTER_EVENT_RESULT_ABORT)
	{
		((CWebSite *)pC->pWebSite)->pErrorPages->SendError(pC, "401", "Authorization failed by filter");
		bResult = false;
	}
	else if(filterResult == FILTER_EVENT_RESULT_SENDERROR)
	{
		if(filterAction.ErrorCode)
		{
			char sErrorCode[64];
			sprintf_s(sErrorCode, sizeof(sErrorCode), "%d", filterAction.ErrorCode);
			((CWebSite *)pC->pWebSite)->pErrorPages->SendError(pC, sErrorCode, filterAction.ErrorInformation);
		}
		else {
			((CWebSite *)pC->pWebSite)->pErrorPages->SendError(pC, "401", "Authorization failed by filter");
		}
		bResult = false;
	}
	else if(filterResult == FILTER_EVENT_RESULT_REDIRECT_TEMP || filterResult == FILTER_EVENT_RESULT_REDIRECT_PERM)
	{
		if(filterAction.RedirectLocation == NULL)
		{
			((CWebSite *)pC->pWebSite)->pErrorPages->SendError(pC, "401", "Authorization failed by filter");
		}
		else {
			this->ResponseRedirect(pC, filterAction.RedirectLocation, filterResult == FILTER_EVENT_RESULT_REDIRECT_PERM);
		}
		bResult = false;
	}

	((CWebSite *)pC->pWebSite)->pCustomFilters->FreeFilterAction(&filterAction);

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHttp::SendHttpHeader(PEER *pC, const char *sLastModified, const char *sErrCode,
						   const char *sErrDesc, const char *sEncoding,
						   const char *sMimeType,
						   __int64 i64ContentLength, //Set i64ContentLength less than 0 to omit from the response header.
						   const char *sExtraHeaders, bool bTerminateHeader)
{
	StringBuilder header;

	if(BuildHttpHeader(pC, &header, sLastModified, sErrCode, sErrDesc, sEncoding, sMimeType, i64ContentLength, sExtraHeaders, bTerminateHeader))
	{
		return pC->pClient->SetNextSendData(header.Buffer, header.Length);
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHttp::SendOKHeader(PEER *pC, const char *sLastModified, __int64 i64FileSize)
{
	return this->SendOKHeader(pC, sLastModified, "text/html", i64FileSize);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHttp::SendOKHeader(PEER *pC, const char *sLastModified, const char *sMimeType, const char *sEncoding, __int64 i64FileSize)
{
	char *sCode = "200";
	char *sDesc = "OK";
	if((pC->Header.ContentBegin != 0 || pC->Header.ContentEnd) && !pC->PerRequestStore.IsDynamicContent)
	{
		sCode = "206";
		sDesc = "Partial content";
	}
	return this->SendHttpHeader(pC, sLastModified, sCode, sDesc, sEncoding, sMimeType, i64FileSize, NULL, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHttp::SendOKHeader(PEER *pC, const char *sLastModified, const char *sMimeType,
						 const char *sEncoding, __int64 i64FileSize, bool bTerminateHeader)
{
	char *sCode = "200";
	char *sDesc = "OK";
	if((pC->Header.ContentBegin != 0 || pC->Header.ContentEnd) && !pC->PerRequestStore.IsDynamicContent)
	{
		sCode = "206";
		sDesc = "Partial content";
	}
	return this->SendHttpHeader(pC, sLastModified, sCode, sDesc, sEncoding, sMimeType, i64FileSize, NULL, bTerminateHeader);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHttp::SendOKHeader(PEER *pC, const char *sLastModified, const char *sMimeType, __int64 i64FileSize)
{
	char *sCode = "200";
	char *sDesc = "OK";
	if((pC->Header.ContentBegin != 0 || pC->Header.ContentEnd) && !pC->PerRequestStore.IsDynamicContent)
	{
		sCode = "206";
		sDesc = "Partial content";
	}
	return this->SendHttpHeader(pC, sLastModified, sCode, sDesc, NULL, sMimeType, i64FileSize, NULL, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHttp::BufferDataToClient(PEER *pC, StringBuilder *sData)
{
	return this->BufferDataToClient(pC, sData->Buffer, sData->Length);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHttp::BufferDataToClient(PEER *pC, const char *sData, unsigned int iDataLength)
{
	int iBytesRemaining = iDataLength;
	int iBytes = 0;
	int iBufferSize = ((CWebSite*)pC->pWebSite)->pSocketPool->FileBufferSize;

	while(iBytesRemaining > 0 && pC->pClient->IsConnected())
	{
		if(iBytesRemaining > iBufferSize)
		{
			iBytes = iBufferSize;
		}
		else{
			iBytes = iBytesRemaining;
		}

		if(!pC->pClient->SetNextSendData(sData + iDataLength - iBytesRemaining, iBytes))
		{
			return false;
		}

		iBytesRemaining -= iBytes;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHttp::FreeClientHeader(PEER *pC)
{
	if(pC->Header.AuthenticationRealm) pMem->Free(pC->Header.AuthenticationRealm);
	if(pC->Header.Method) pMem->Free(pC->Header.Method);
	if(pC->Header.Request) pMem->Free(pC->Header.Request);
	if(pC->Header.Version) pMem->Free(pC->Header.Version);
	if(pC->Header.Accept) pMem->Free(pC->Header.Accept);
	if(pC->Header.AcceptLanguage) pMem->Free(pC->Header.AcceptLanguage);
	if(pC->Header.AcceptEncoding) pMem->Free(pC->Header.AcceptEncoding);
	if(pC->Header.UserAgent) pMem->Free(pC->Header.UserAgent);
	if(pC->Header.Host) pMem->Free(pC->Header.Host);
	if(pC->Header.Connection) pMem->Free(pC->Header.Connection);
	if(pC->Header.Query) pMem->Free(pC->Header.Query);
	if(pC->Header.CacheControl) pMem->Free(pC->Header.CacheControl);
	if(pC->Header.Referer) pMem->Free(pC->Header.Referer);
	if(pC->Header.Cookie) pMem->Free(pC->Header.Cookie);
	if(pC->Header.PostData) pMem->Free(pC->Header.PostData);
	if(pC->Header.FullRequest) pMem->Free(pC->Header.FullRequest);
	if(pC->Header.ContentRange) pMem->Free(pC->Header.ContentRange);
	if(pC->Header.ContentType) pMem->Free(pC->Header.ContentType);
	if(pC->Header.Status) pMem->Free(pC->Header.Status);
	if(pC->Header.ScriptName) pMem->Free(pC->Header.ScriptName);
	if(pC->Header.PathInfo) pMem->Free(pC->Header.PathInfo);
	if(pC->Header.PathTranslated) pMem->Free(pC->Header.PathTranslated);

	memset(&pC->Header, 0, sizeof(pC->Header));

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHttp::GetHeaderRequestRange(CWebSite *pWebSite, PEER *pC, const char *sBuf, const int iBufLen)
{
	bool bResult = false;

	if(iBufLen > 0)
	{
		char sTok[255];
		int iRPos = 0;
		int iTokSz = 0;

		const char *sTokSep = "`~!@#$%^&*()_-+=<,>.?/;:'\"[{]}\\|";
		const int iTokSep = (int)strlen(sTokSep);

		if((iTokSz = GetNextToken(sBuf, iBufLen, sTok, sizeof(sTok), &iRPos, sTokSep, iTokSep)) > 0)
		{
			if(sBuf[iRPos++] == '=')
			{
				if((iTokSz = GetNextToken(sBuf, iBufLen, sTok, sizeof(sTok), &iRPos, sTokSep, iTokSep)) > 0)
				{
					pC->Header.ContentBegin = _atoi64(sTok);

					if(sBuf[iRPos++] == '-')
					{
						bResult = true;
						if((iTokSz = GetNextToken(sBuf, iBufLen, sTok, sizeof(sTok), &iRPos, sTokSep, iTokSep)) > 0)
						{
							pC->Header.ContentEnd = _atoi64(sTok);
						}
					}
				}
			}
		}
	}
	
	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHttp::ParseHTTPHeader(CWebSite *pWebSite, PEER *pC, const char *sBuf, const int iBufLen)
{
#ifdef _DEBUG
	//printf("Request header: %s\n", sBuf);
#endif
	
	pWebSite->Stats.Hits++;

	memset(&pC->Header, 0, sizeof(HTTPHEADER));

	// -- ANYTHING ALLOCATED HERE MUST BE FREED IN FreeClientHeader() --
	int iTokenSz = 0;
	int iRequestLength = 0;
	int iRPos = 0;
	int iFurthestExtent = 0;

	iTokenSz = GetNextToken(sBuf, iBufLen, pC->Header.Method, &iRPos);

	if(_strcmpi(pC->Header.Method, "GET") != 0
		&& _strcmpi(pC->Header.Method, "PUT") != 0
		&& _strcmpi(pC->Header.Method, "POST") != 0
		&& _strcmpi(pC->Header.Method, "HEAD") != 0)
	{
		char *sMethod = "<unknown>";
		if(IsAlNum(pC->Header.Method, iTokenSz))
		{
			sMethod = pC->Header.Method;
		}

		pWebSite->pErrorPages->SendError(pC, "400", "Invalid or non-implemented http method: %s", sMethod);
		return false;
	}

	iRequestLength = GetNextToken(sBuf, iBufLen, pC->Header.Request, &iRPos);

	if(iRequestLength >= MAX_URI_LEN)
	{
		pWebSite->pErrorPages->SendError(pC, "400", "Request URI is too long");
		return false;
	}

	iTokenSz = GetNextToken(sBuf, iBufLen, pC->Header.Version, &iRPos);

	iTokenSz = this->GetHttpHeaderTag(sBuf, iBufLen, "Accept:", pC->Header.Accept, &iFurthestExtent);
	iTokenSz = this->GetHttpHeaderTag(sBuf, iBufLen, "Accept-Language:", pC->Header.AcceptLanguage, &iFurthestExtent);
	iTokenSz = this->GetHttpHeaderTag(sBuf, iBufLen, "Accept-Encoding:", pC->Header.AcceptEncoding, &iFurthestExtent);
	iTokenSz = this->GetHttpHeaderTag(sBuf, iBufLen, "User-Agent:", pC->Header.UserAgent, &iFurthestExtent);
	iTokenSz = this->GetHttpHeaderTag(sBuf, iBufLen, "Cache-Control:", pC->Header.CacheControl, &iFurthestExtent);
	iTokenSz = this->GetHttpHeaderTag(sBuf, iBufLen, "Cookie:", pC->Header.Cookie, &iFurthestExtent);
	iTokenSz = this->GetHttpHeaderTag(sBuf, iBufLen, "Host:", pC->Header.Host, &iFurthestExtent);
	iTokenSz = this->GetHttpHeaderTag(sBuf, iBufLen, "Connection:", pC->Header.Connection, &iFurthestExtent);
	iTokenSz = this->GetHttpHeaderTag(sBuf, iBufLen, "Referer:", pC->Header.Referer, &iFurthestExtent);
	iTokenSz = this->GetHttpHeaderTag(sBuf, iBufLen, "Content-Type:", pC->Header.ContentType, &iFurthestExtent);
	iTokenSz = this->GetHttpHeaderTag(sBuf, iBufLen, "If-Modified-Since:", &pC->Header.IfModifiedSince, &iFurthestExtent);

	//%___|JMP 2012/06/12
	//GetHeaderRequestRange must directly follow the call to GetHttpHeaderTag which parses the "Range:" tag.
	iTokenSz = this->GetHttpHeaderTag(sBuf, iBufLen, "Range:", pC->Header.ContentRange, &iFurthestExtent);
	GetHeaderRequestRange(pWebSite, pC, pC->Header.ContentRange, iTokenSz);

	//Parse query string.
	int iQueryIndex = CharIndex(pC->Header.Request, '?');
	if(iQueryIndex > 0)
	{
		iQueryIndex++; //Skip the question mark;

		pMem->CloneStringNSafe(pC->Header.Query, pC->Header.Request + iQueryIndex, (iRequestLength - iQueryIndex));
		pC->Header.Request[iQueryIndex - 1] = '\0';
	}
	else{
		pMem->CloneStringSafe(pC->Header.Query, "");
	}

	URLDecode(pC->Header.Request);
	//URLDecode(pC->Header.Query); //QUERY_STRING must stay encoded, only QUERY_STRING_UNESCAPED is to be decoded.

	int iPostStartPos = -1;
	int iPostEndPos = iBufLen;

	int iBoundaryStart = InStrI("boundary=", pC->Header.ContentType);
	if(iBoundaryStart >= 0)
	{
		//Parse multi-part form data.

		char sToken[100];
		iBoundaryStart += 9; //strlen("boundary=");
		int iBoundaryTagSz = GetNextToken(pC->Header.ContentType, (int)strlen(pC->Header.ContentType), sToken, sizeof(sToken), &iBoundaryStart, ";\n\r", 4);
		if(iBoundaryTagSz >= sizeof(sToken)) //We'll be appending to this buffer
		{
			pWebSite->pErrorPages->SendError(pC, "400", "Boundary tag is too large");
			return false;
		}
		else if (iBoundaryTagSz <= 0)
		{
			pWebSite->pErrorPages->SendError(pC, "400", "Boundary tag could not be tokenized");
			return false;
		}

		char sBoundaryTag[110];
		//Find beginning of post data:
		sprintf_s(sBoundaryTag, sizeof(sBoundaryTag), "--%s", sToken);
		if((iPostStartPos = InStr(sBoundaryTag, sBuf, iBufLen, iFurthestExtent)) >= 0)
		{
			//Find end of post data:
			sprintf_s(sBoundaryTag, sizeof(sBoundaryTag), "--%s--", sToken);
			if((iPostEndPos = InStr(sBoundaryTag, sBuf, iBufLen, iPostStartPos)) >= 0)
			{
				iPostEndPos += (int)strlen(sBoundaryTag);

				//Preserve any trailing "LF" or "CRLF".
				if(sBuf[iPostEndPos] == '\r') iPostEndPos++;
				if(sBuf[iPostEndPos] == '\n') iPostEndPos++;
				//We have a completely valid chunk of data.
			}
			else {
				pWebSite->pErrorPages->SendError(pC, "400");
				return false;
			}
		}
		else {
			iPostStartPos = -1;
		}
	}
	else
	{
		//Parse generic post data (header data).
		if((iPostStartPos = InStr("\n\n", 2, sBuf, iBufLen, iFurthestExtent)) <= 0)
		{
			if((iPostStartPos = InStr("\r\n\r\n", 4, sBuf, iBufLen, iFurthestExtent)) <= 0)
			{
				iPostStartPos = -1;
			}
			else{
				iPostStartPos += 4;
			}
		}
		else{
			iPostStartPos += 2;
		}
	}

	if(iPostStartPos > 0 && iBufLen > iPostStartPos)
	{
		pC->Header.PostDataSize = (iPostEndPos - iPostStartPos);
		pC->Header.PostData = (char *) pMem->Allocate(sizeof(char), pC->Header.PostDataSize + 1);
		memcpy_s(pC->Header.PostData, pC->Header.PostDataSize + 1, sBuf + iPostStartPos, pC->Header.PostDataSize);
	}
	else{
		pC->Header.PostDataSize = 0;
		pC->Header.PostData = (char *) pMem->Allocate(sizeof(char), 1);
		memcpy_s(pC->Header.PostData, 1, "", 1);
	}

	//Will this connection be using keep-alives?
	if(((CWebSite*)pC->pWebSite)->pWebsiteSettings->KeepAliveTimeout() > 0)
	{
		if(_strcmpi(pC->Header.Connection, "Keep-Alive") == 0) //Keep-Alive vs. Close
		{
			pC->KeepAlive = true;
		}
		else{
			pC->KeepAlive = false;
		}
	}

	/* FIXFIX: Reimplement filter.
	if(!pWebSite->pCustomFilters->ProcessRequestHeader(pC))
	{
		pWebSite->pErrorPages->SendError(pC, "401", "Authorization failed by filter");
		return false;
	}
	*/
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns true if processing should continue, otherwise false.
*/
bool CHttp::AssembleFullRequest(CWebSite *pWebSite, PEER *pC)
{
	ROOT *pVirtualRoot = pWebSite->pVirtualRoots->IsVirtualRoot(pC->Header.Request/*, pC->Header.FullRequest*/);

	StringBuilder FullRequest;

	if(pVirtualRoot)
	{
		if(pVirtualRoot->Username)
		{
			HANDLE hToken = NULL;
			char *sDomain = NULL;
			char sErrorMessage[1024];

			if(strlen(pVirtualRoot->Domain) > 0)
			{
				sDomain = pVirtualRoot->Domain;
			}

			if((hToken = NTLogin(pVirtualRoot->Username, sDomain, pVirtualRoot->Password, sErrorMessage, sizeof(sErrorMessage))))
			{
				if(!ImpersonateLoggedOnUser(hToken))
				{
					char sErrorMessage[1024];
					if(GetLastError(sErrorMessage, sizeof(sErrorMessage)))
					{
						pWebSite->pErrorPages->SendError(pC, "500", sErrorMessage);
					}
					else{
						pWebSite->pErrorPages->SendError(pC, "500", "Unknown logon error.");
					}

					CloseHandle(hToken);
					return false;
				}
				CloseHandle(hToken);
			}
			else{
				pWebSite->pErrorPages->SendError(pC, "500", sErrorMessage);
				return false;
			}
		}

		FullRequest.Append(pVirtualRoot->Path);
	}
	else
	{
		FullRequest.Append(pWebSite->Root);
	}

	if(FullRequest.Buffer[FullRequest.Length - 1] == '\\')
	{
		FullRequest.Terminate(FullRequest.Length - 1);
	}

	char sToken[MAX_PATH];
	int iTokPos = 0;
	int iRequestLength = (int) strlen(pC->Header.Request);

	if(pVirtualRoot)
	{
		iTokPos = pVirtualRoot->RootLength;
	}

	//Skip any string of slashes.
	while(pC->Header.Request[iTokPos] == '/' || pC->Header.Request[iTokPos] == '\\')
	{
		iTokPos++;
	}

	//Check every part of the passed in request to see it it can be matched to a file name.
	//	If it can, then the remainder of the request is "extra path info" (e.g. part of PATH_INFO).
	while(GetNextToken(pC->Header.Request, iRequestLength, sToken, sizeof(sToken), &iTokPos, "\\/", 2))
	{
		FullRequest.AppendF("\\%s", sToken);

		DWORD dwSegmentAttribs = GetFileAttributes(FullRequest.Buffer);
		if(dwSegmentAttribs != INVALID_FILE_ATTRIBUTES && !(dwSegmentAttribs &FILE_ATTRIBUTE_DIRECTORY))
		{
			int iPathInfoLength = iRequestLength - iTokPos;
			if(iPathInfoLength > 0)
			{
				pMem->CloneStringNSafe(pC->Header.PathInfo, pC->Header.Request + iTokPos, iPathInfoLength);

				CorrectForwardPath(pC->Header.PathInfo, iPathInfoLength + 1);
				if(strcmp(pC->Header.PathInfo, "/") == 0)
				{
					pC->Header.PathInfo[0] = '\0';
				}

				iTokPos = iRequestLength;
			}

			break;
		}

		//Skip any string of slashes.
		while(pC->Header.Request[iTokPos] == '/' || pC->Header.Request[iTokPos] == '\\')
		{
			iTokPos++;
		}
	}

	if(iTokPos < iRequestLength)
	{
		FullRequest.AppendF("\\%s", pC->Header.Request + iTokPos);
	}

	pMem->CloneStringNSafe(pC->Header.FullRequest, FullRequest.Buffer, FullRequest.Length);
	CorrectReversePath(pC->Header.FullRequest, FullRequest.Length + 1, true);

	DWORD dwAttributes = GetFileAttributes(pC->Header.FullRequest);
	if(dwAttributes == INVALID_FILE_ATTRIBUTES)
	{
		this->FillInPathInfoEnvironment(pWebSite, pC, pVirtualRoot);

		char sErrorMessage[1024];
		if(GetLastError(sErrorMessage, sizeof(sErrorMessage)))
		{
			if(InStrI("cannot find", sErrorMessage) > 0)
			{
				//If the file nolonger exists, remove it from the cache (if it exists there).
				((CWebSite*)pC->pWebSite)->pCompression->RemoveFileFromCache(pC->Header.FullRequest);

				pWebSite->pErrorPages->SendError(pC, "404", sErrorMessage);
				return false;
			}

			pWebSite->pErrorPages->SendError(pC, "500", sErrorMessage);
		}
		else{
			pWebSite->pErrorPages->SendError(pC, "500", "Unknown logon error.");
		}
		return false;
	}
	else{
		if(dwAttributes &FILE_ATTRIBUTE_DIRECTORY)
		{
			size_t iRequestLength = strlen(pC->Header.Request);

			//Requests for directories musst end with a forward slash.
			if(pC->Header.Request[iRequestLength - 1] != '/' && pC->Header.Request[iRequestLength - 1] != '\\')
			{
				StringBuilder newLocation;
				newLocation.AppendF("%s/%s%s", pC->Header.Request,  (pC->Header.Query[0] == '\0' ? "" : "?"), pC->Header.Query);
				this->ResponseRedirect(pC, newLocation.Buffer, true);
				return false;
			}

			if(pWebSite->pDefaultPages->CheckPath(pC->Header.FullRequest))
			{
				this->FillInPathInfoEnvironment(pWebSite, pC, pVirtualRoot);
				/* FIXFIX: Reimplement filter.
				if(!pWebSite->pCustomFilters->ProcessUrlMap(pC))
				{
					pWebSite->pErrorPages->SendError(pC, "401", "Authorization failed by filter");
					return false;
				}
				*/
				return true;
			}
			else {
				if(pWebSite->pWebsiteSettings->AllowDirectoryIndexing() && !pWebSite->pCGIFolders->IsInCGIFolder(pC))
				{
					//Will be sending directory structure.
					/* FIXFIX: Reimplement filter.
					if(!pWebSite->pCustomFilters->ProcessUrlMap(pC))
					{
						pWebSite->pErrorPages->SendError(pC, "401", "Authorization failed by filter");
						return false;
					}
					*/
					return true;
				}
				else{
					pWebSite->pErrorPages->SendError(pC, "403", "Directory listing denied");
					return false;
				}
			}
		}
		else{
			//pC->Header.FullRequest now contains the full request.
			this->FillInPathInfoEnvironment(pWebSite, pC, pVirtualRoot);
			/* FIXFIX: Reimplement filter.
			if(!pWebSite->pCustomFilters->ProcessUrlMap(pC))
			{
				pWebSite->pErrorPages->SendError(pC, "401", "Authorization failed by filter");
				return false;
			}
			*/
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CHttp::FillInPathInfoEnvironment(CWebSite *pWebSite, PEER *pC, ROOT *pVirtualRoot)
{
	StringBuilder buffer;

	//---------------------------------------------------------------------------------------------------------
	//Fill in the script name.
	//---------------------------------------------------------------------------------------------------------
	if(pVirtualRoot)
	{
		int iRootLength = (int)strlen(pVirtualRoot->Path);

		int iMemoryRequired = iRootLength + ((int) strlen(pC->Header.FullRequest + iRootLength)) + 2;

		pC->Header.ScriptName = (char *) pMem->Allocate(sizeof(char), iMemoryRequired);
		sprintf_s(pC->Header.ScriptName, iMemoryRequired,
			"%s%s", pVirtualRoot->Path, pC->Header.FullRequest + iRootLength);
		CorrectForwardPath(pC->Header.ScriptName, (int)strlen(pC->Header.ScriptName) + 1);
	}
	else {
		int iRootLength = (int)strlen(pWebSite->Root);
		if(pWebSite->Root[iRootLength-1] == '\\')
		{
			iRootLength--;
		}
		pMem->CloneStringSafe(pC->Header.ScriptName, pC->Header.FullRequest + iRootLength);
		CorrectForwardPath(pC->Header.ScriptName, (int)strlen(pC->Header.ScriptName) + 1);
	}

	//---------------------------------------------------------------------------------------------------------
	//Fill in the "Path Translated" (depends on the full request and the previously parsed partial PATH_INFO).
	//---------------------------------------------------------------------------------------------------------
	buffer.Clear();
	buffer.Append(pC->Header.FullRequest);
	if(pC->Header.PathInfo)
	{
		buffer.Append(pC->Header.PathInfo);
	}
	pMem->CloneStringSafe(pC->Header.PathTranslated, buffer.Buffer);
	CorrectReversePath(pC->Header.PathTranslated, (int)strlen(pC->Header.PathTranslated) + 1);

	//---------------------------------------------------------------------------------------------------------
	//Fill in the path info (depends on the SCRIPT_NAME and the previously parsed partial PATH_INFO).
	//---------------------------------------------------------------------------------------------------------
	buffer.Clear();
	buffer.Append(pC->Header.ScriptName);
	if(pC->Header.PathInfo)
	{
		buffer.Append(pC->Header.PathInfo);
		pMem->Free(pC->Header.PathInfo);
	}
	pMem->CloneStringSafe(pC->Header.PathInfo, buffer.Buffer);

	//---------------------------------------------------------------------------------------------------------
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CHttp::GetHttpHeaderTag(const char *sHeader, const int iHeaderLen, const char *sTag, SYSTEMTIME *pOutST, int *iFurthestExtent)
{
	char *sBuffer = NULL;
	int iTokenSz = this->GetHttpHeaderTag(sHeader, iHeaderLen, "If-Modified-Since:", sBuffer, iFurthestExtent);
	if(iTokenSz > 0)
	{
		if(BreakGMTTimeString(sBuffer, pOutST))
		{
			pMem->Free(sBuffer);
			return iTokenSz;
		}
	}

	memset(pOutST, 0, sizeof(SYSTEMTIME));

	pMem->Free(sBuffer);
	return iTokenSz;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CHttp::GetHttpHeaderTag(const char *sHeader, const int iHeaderLen, const char *sTag, char *&sBuf, int *iFurthestExtent)
{
	int iTagLength = (int)strlen(sTag);
	int iRPos = 0;

	if (sBuf)
	{
		pMem->Free(sBuf);
		sBuf = NULL;
	}

	iRPos = InStrI(sTag, iTagLength, sHeader, iHeaderLen, 0);
	//The tag must either be on a line by its-self or the first line.
	while(iRPos > 0 && sHeader[iRPos - 1] != '\n')
	{
		//the tag may be further down the line...
		iRPos = InStrI(sTag, iTagLength, sHeader, iHeaderLen, iRPos + 1);
	}

	if(iRPos >= 0)
	{
		iRPos += iTagLength; //Skip the tag itsself;
		SkipWhiteSpaces(sHeader, iHeaderLen, &iRPos);

		int iEndPos = InStr("\n", 1, sHeader, iHeaderLen, iRPos);
		if(iEndPos > 0 && iEndPos > iRPos)
		{
			if(iEndPos > iRPos)
			{
				if(iFurthestExtent)
				{
					if(iEndPos > ((int)*iFurthestExtent))
					{
						*iFurthestExtent = iEndPos - 1;
					}
				}

				int iLength = iEndPos - iRPos;
				pMem->CloneStringNSafe(sBuf, sHeader + iRPos, iLength);
				return Trim(sBuf, iLength);
			}
		}
	}

	pMem->CloneStringSafe(sBuf, "");

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CHttp::GetHttpHeaderTag(const char *sHeader, const int iHeaderLen,
								const char *sTag, char *sBuf, int iMaxBuf, bool bPartialOk, int *iFurthestExtent)
{
	int iTagLength = (int)strlen(sTag);
	int iRPos = 0;

	if((iRPos = InStrI(sTag, iTagLength, sHeader, iHeaderLen, 0)) >= 0)
	{
		iRPos += iTagLength; //Skip the tag itsself;
		SkipWhiteSpaces(sHeader, iHeaderLen, &iRPos);

		int iEndPos = InStr("\n", 1, sHeader, iHeaderLen, iRPos);
		if(iEndPos > 0 && iEndPos > iRPos)
		{
			if(iEndPos > iRPos)
			{
				int iLength = iEndPos - iRPos;
				if(iLength >= iMaxBuf)
				{
					if(bPartialOk)
					{
						iLength = iMaxBuf - 1;
					}
					else {
						strcpy_s(sBuf, iMaxBuf, "");
						return 0;
					}
				}

				if(iFurthestExtent)
				{
					if(iEndPos > ((int)*iFurthestExtent))
					{
						*iFurthestExtent = iEndPos - 1;
					}
				}

				strncpy_s(sBuf, iMaxBuf, sHeader + iRPos, iLength);
				return Trim(sBuf, iLength);
			}
		}
	}

	strcpy_s(sBuf, iMaxBuf, "");

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CHttp::GetHttpHeaderTag(const char *sHeader, const int iHeaderLen,
								const char *sTag, char *sBuf, int iMaxBuf, int *iFurthestExtent)
{
	return GetHttpHeaderTag(sHeader, iHeaderLen, sTag, sBuf, iMaxBuf, false, iFurthestExtent);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHttp::DoesHeaderContainTag(const char *sBuf, const int iBufLen, const char *sTag)
{
	return this->DoesHeaderContainTag(sBuf, iBufLen, sTag, NULL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHttp::DoesHeaderContainTag(const char *sBuf, const int iBufLen,
								 const char *sTag, int *iHeaderEndPos)
{
	if(iHeaderEndPos)
	{
		*iHeaderEndPos = 0;
	}

	int iTagPos = InStrI(sTag, sBuf, iBufLen, 0);

	//The tag must either be on a line by its-self or the first line.
	while(iTagPos > 0 && sBuf[iTagPos - 1] != '\n')
	{
		//the tag may be further down the line...
		iTagPos = InStrI(sTag, sBuf, iBufLen, iTagPos + 1);
	}

	if(iTagPos >= 0)
	{
		int iLineFeedLen = 0;
		int iLineFeedPos = 0;
		int iSingleLineFeedPos = InStr("\n\n", 2, sBuf, iBufLen, 0);
		int iDoubleLineFeedPos = InStr("\r\n\r\n", 4, sBuf, iBufLen, 0);

		if(iDoubleLineFeedPos > 0 && iSingleLineFeedPos > 0)
		{
			if(iDoubleLineFeedPos < iSingleLineFeedPos) //We want to use the first one.
			{
				iLineFeedLen = 4;
				iLineFeedPos = iDoubleLineFeedPos;
			}
			else {
				iLineFeedLen = 2;
				iLineFeedPos = iSingleLineFeedPos;
			}
		}
		else if(iDoubleLineFeedPos > 0) {
			iLineFeedLen = 4;
			iLineFeedPos = iDoubleLineFeedPos;
		}
		else if(iSingleLineFeedPos > 0) {
			iLineFeedLen = 2;
			iLineFeedPos = iSingleLineFeedPos;
		}

		if(iLineFeedPos > 0 && iTagPos < iLineFeedPos)
		{
			if(iHeaderEndPos)
			{
				*iHeaderEndPos = (iLineFeedPos + iLineFeedLen);
			}
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHttp::IsValueEqual(const char *sValue1, const char *sValue2)
{
	if(sValue1 && sValue2)
	{
		return(_strcmpi(sValue1, sValue2) == 0);
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHttp::DoesValueContain(const char *sIn, const char *sValue)
{
	if(sIn && sValue)
	{
		return(InStrI(sValue, sIn) >= 0);
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
