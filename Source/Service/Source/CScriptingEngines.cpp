///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CScriptingEngines_CPP
#define _CScriptingEngines_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>

#include "../../../../@Libraries/CDuration/CDuration.h"

extern HIMAGELIST hEnableDisableImageList; //Declared in MainDialog.cpp
extern HIMAGELIST hOnePixilImageList; //Declared in MainDialog.cpp

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CScriptingEngines.H"

#include "Entry.H"

#include "CWebSites.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::File;
using namespace NSWFL::ListView;
using namespace NSWFL::System;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CScriptingEngines::~CScriptingEngines(void)
{
	this->Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CScriptingEngines::CScriptingEngines(void *lpWebSites)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CScriptingEngines::CScriptingEngines(void *lpWebSites, CXMLReader *xmlConfig, CScriptingEngines *pDefaults)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
	this->Load(xmlConfig, pDefaults);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CScriptingEngines::Save(void)
{
	this->Locks.LockShared();

	CXMLReader xmlConfig;
	if(this->ToXML(&xmlConfig))
	{
		bool bResult = xmlConfig.ToFile(this->sFileName);
		if(!bResult)
		{
			char sErrorMsg[2048];
			GetLastError(sErrorMsg, sizeof(sErrorMsg),
				"Failed to save websites configuration file (", ")");
			strcat_s(sErrorMsg, sizeof(sErrorMsg), "\r\n\r\n\"");
			strcat_s(sErrorMsg, sizeof(sErrorMsg), this->sFileName);
			strcat_s(sErrorMsg, sizeof(sErrorMsg), "\"");
			((CWebSites *)this->pWebSites)->Trace(sErrorMsg);
		}
		return this->Locks.UnlockShared(bResult);
	}
	return this->Locks.UnlockShared(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CScriptingEngines::ToXML(CXMLReader *lpXML)
{
	this->Locks.LockShared();

	CXMLWriter xmlConfig("ScriptingEngines");

	xmlConfig.AddBool("Enable", this->Collection.Enabled);

	for(int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		CXMLWriter Item("Engine");
		Item.Add("Extension", this->Collection.Items[iItem].Extension);
		Item.Add("Executable", this->Collection.Items[iItem].Engine);
		Item.Add("SuccessCode", this->Collection.Items[iItem].SuccessCode);
		Item.AddBool("UseSuccessCode", this->Collection.Items[iItem].UseSuccessCode);
		Item.Add("Parameters", this->Collection.Items[iItem].Parameters);
		Item.Add("Description", this->Collection.Items[iItem].Description);
		Item.AddBool("Enable", this->Collection.Items[iItem].Enabled);
		xmlConfig.Add(&Item);
	}

	xmlConfig.ToReader(lpXML);

	xmlConfig.Destroy();

	return this->Locks.UnlockShared(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	Reloads the configuration from the file it was originally loaded from.
*/
bool CScriptingEngines::Reload(void)
{
	this->Locks.LockExclusive();

	if(this->Initialized)
	{
		this->Destroy();
	}

	this->Load(this->sFileName);

	return this->Locks.UnlockExclusive(this->Initialized);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CScriptingEngines::Load(const char *sXMLFileName)
{
	this->Locks.LockExclusive();
	if(this->Initialized)
	{
		this->Destroy();
	}

	strcpy_s(this->sFileName, sizeof(this->sFileName), sXMLFileName);

	CXMLReader xmlConfig;

	if(xmlConfig.FromFile(sXMLFileName))
	{
		CXMLReader xmlEntity;
		if(xmlConfig.ToReader("ScriptingEngines", &xmlEntity))
		{
			this->Initialized = this->Load(&xmlEntity, NULL);
		}
		xmlEntity.Destroy();
		xmlConfig.Destroy();
	}

	return this->Locks.UnlockExclusive(this->Initialized);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CScriptingEngines::Load(CXMLReader *xmlConfig, CScriptingEngines *pDefaults)
{
	this->Locks.LockExclusive();
	if(this->Initialized)
	{
		this->Destroy();
	}

	int iLength = 0;
	memset(&this->Collection, 0, sizeof(this->Collection));

	this->Collection.Enabled = xmlConfig->ToBoolean("Enable", true);

	xmlConfig->ProgressiveScan(true);
	CXMLReader XPEngine;

	while(xmlConfig->ToReader("Engine", &XPEngine))
	{
		char sExt[SCRIPTINGENGINES_MAX_EXT_LEGTH];
		char sEngine[SCRIPTINGENGINES_MAX_ENGINE_LEGTH];
		char sParams[SCRIPTINGENGINES_MAX_PARAMS_LEGTH];
		char sParams2[SCRIPTINGENGINES_MAX_PARAMS_LEGTH];
		char sDescr[SCRIPTINGENGINES_MAX_DESCR_LEGTH];

		this->Collection.Items = (SCRIPTENGINE *)
			pMem->ReAllocate(this->Collection.Items, sizeof(SCRIPTENGINE), this->Collection.Count + 1);

		SCRIPTENGINE *p = &this->Collection.Items[this->Collection.Count++];

		XPEngine.ToString("Extension", sExt, sizeof(sExt), &iLength);
		p->Extension = (char *) pMem->StrDup(LCase(sExt, iLength));

		XPEngine.ToString("Executable", sEngine, sizeof(sEngine), &iLength);
		p->Engine = (char *) pMem->StrDup(sEngine);

		p->SuccessCode = XPEngine.ToInteger("SuccessCode", 0);
		p->UseSuccessCode = XPEngine.ToBoolean("UseSuccessCode", true);

		XPEngine.ToString("Parameters", sParams, sizeof(sParams), &iLength);
		ReplaceStrings(sParams, "%s", "%1", sParams2, sizeof(sParams2)); //For backwards compatibility.
		p->Parameters = (char *) pMem->StrDup(sParams2);

		XPEngine.ToString("Description", sDescr, sizeof(sDescr), &iLength);
		p->Description = (char *) pMem->StrDup(sDescr);

		p->Enabled = XPEngine.ToBoolean("Enable", true);

		XPEngine.Destroy();
	}

	return this->Locks.UnlockExclusive(this->Initialized = true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CScriptingEngines::Enabled(void)
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->Collection.Enabled && (this->Collection.Count > 0));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CScriptingEngines::Destroy(void)
{
	this->Locks.LockExclusive();

	if(this->Initialized)
	{
		this->Initialized = false;
		if(this->Collection.Count > 0)
		{
			for(int iPos = 0; iPos < this->Collection.Count; iPos++)
			{
				pMem->Free(this->Collection.Items[iPos].Extension);
				pMem->Free(this->Collection.Items[iPos].Engine);
				pMem->Free(this->Collection.Items[iPos].Parameters);
				pMem->Free(this->Collection.Items[iPos].Description);
			}

			pMem->Free(this->Collection.Items);

			this->Collection.Count = 0;
		}
	}
	return this->Locks.UnlockExclusive(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns true if the file is a scripted file, otherwise false.
*/
bool CScriptingEngines::GetScriptEngine(const char *sFileName,
										char *sEngine, int iMaxEngine,
										char *sParams, int iMaxParams,
										int *iSuccessCode, bool *bUseSuccessCode,
										bool *bIsFastCGI)
{
	this->Locks.LockShared();
	if(this->Collection.Enabled && (this->Collection.Count > 0))
	{
		char sExt[MAX_PATH];
		GetFileExtension(sFileName, sExt, sizeof(sExt));
		int iExtLength = (int) strlen(sExt);

		for(int iPos = 0; iPos < this->Collection.Count; iPos++)
		{
			if(this->Collection.Items[iPos].Enabled)
			{
				if(_strcmpi(this->Collection.Items[iPos].Extension, sExt) == 0 || (iExtLength == 0 && strcmp(this->Collection.Items[iPos].Extension, ".") == 0))
				{
					if(bIsFastCGI)
					{
						char sEngineExt[MAX_PATH];
						if(GetFileExtension(this->Collection.Items[iPos].Engine, sEngineExt, sizeof(sEngineExt)))
						{
							*bIsFastCGI = (_strcmpi(".dll", sEngineExt) == 0);
						}
						else{
							*bIsFastCGI = false;
						}
					}
					strcpy_s(sEngine, iMaxEngine, this->Collection.Items[iPos].Engine);
					strcpy_s(sParams, iMaxParams, this->Collection.Items[iPos].Parameters);
					*iSuccessCode = this->Collection.Items[iPos].SuccessCode;
					*bUseSuccessCode = this->Collection.Items[iPos].UseSuccessCode;
					return this->Locks.UnlockShared(true);
				}
			}
		}
	}
	return this->Locks.UnlockShared(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns true if the file is a scripted file, otherwise false.
*/
bool CScriptingEngines::IsScriptFile(const char *sFileName)
{
	this->Locks.LockShared();
	if(this->Collection.Enabled && (this->Collection.Count > 0))
	{
		char sExt[MAX_PATH];
		GetFileExtension(sFileName, sExt, sizeof(sExt));

		for(int iPos = 0; iPos < this->Collection.Count; iPos++)
		{
			if(this->Collection.Items[iPos].Enabled)
			{
				if(_strcmpi(this->Collection.Items[iPos].Extension, sExt) == 0)
				{
					return this->Locks.UnlockShared(true);
				}
			}
		}
	}
	return this->Locks.UnlockShared(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CScriptingEngines::AddEnvVar(CStringBuilder *lpBuf, const char *sVarName, const int iVarData)
{
	this->Locks.LockShared();
	lpBuf->Append(sVarName);
	lpBuf->Append("=");
	lpBuf->Append(iVarData);
	lpBuf->Append("\0", 1);
	return this->Locks.UnlockShared(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CScriptingEngines::AddEnvVar(CStringBuilder *lpBuf, const char *sVarName, const char *sVarData, int iVarLength)
{
	this->Locks.LockShared();
	lpBuf->Append(sVarName);
	lpBuf->Append("=");
	lpBuf->Append(sVarData, iVarLength);
	lpBuf->Append("\0", 1);
	return this->Locks.UnlockShared(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CScriptingEngines::AddRawEnvVar(CStringBuilder *lpBuf, const char *sVarNameAndValue)
{
	this->Locks.LockShared();
	lpBuf->Append(sVarNameAndValue);
	lpBuf->Append("\0", 1);
	return this->Locks.UnlockShared(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CScriptingEngines::AddRawEnvVar(CStringBuilder *lpBuf, const char *sVarNameAndValue, int iVarNameAndValueLength)
{
	this->Locks.LockShared();
	lpBuf->Append(sVarNameAndValue, iVarNameAndValueLength);
	lpBuf->Append("\0", 1);
	return this->Locks.UnlockShared(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CScriptingEngines::AddEnvVar(CStringBuilder *lpBuf, const char *sVarName, const char *sVarData)
{
//#if _DEBUG
//	printf("%s = %s\n", sVarName, sVarData);
//#endif
	this->Locks.LockShared();
	lpBuf->Append(sVarName);
	lpBuf->Append("=");
	lpBuf->Append(sVarData);
	lpBuf->Append("\0", 1);
	return this->Locks.UnlockShared(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CScriptingEngines::CopyEnvVar(CStringBuilder *lpStr, const char *sVarName)
{
	this->Locks.LockShared();
	DWORD dwAlloc = GetEnvironmentVariable(sVarName, NULL, 0);
	char *sTemp = (char *)pMem->Allocate(dwAlloc, sizeof(char));
	bool bResult = false;
	if(sTemp)
	{
		if(GetEnvironmentVariable(sVarName, sTemp, dwAlloc) == dwAlloc - 1)
		{
			bResult = this->AddEnvVar(lpStr, sVarName, sTemp);
		}
		pMem->Free(sTemp);
	}
	return this->Locks.UnlockShared(bResult);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CScriptingEngines::AddEnvVars(VOID *pClient, CStringBuilder *lpStr, const char *sScriptFileName)
{
	this->Locks.LockShared();
	char sTemp[MAX_URI_LEN];

	PEER *pC = (PEER *)pClient;
	CHttp *pHttp = (CHttp *)((CWebSite *)pC->pWebSite)->pHttp;
	CWebSites *pWebSites = (CWebSites *)((CWebSite *)pC->pWebSite)->pWebSites;

	if(pC->Header.PostDataSize > 0)
	{
		if(pC->Header.PostDataSize > 0)
		{
			this->AddEnvVar(lpStr, "CONTENT_LENGTH", pC->Header.PostDataSize);
			this->AddEnvVar(lpStr, "HTTP_CONTENT_LENGTH", pC->Header.PostDataSize);
			//this->AddEnvVar(lpStr, "HTTP_RAW_POST_DATA", pC->Header.PostData, pC->Header.PostDataSize);
		}
		if(strlen(pC->Header.ContentType) > 0)
		{
			this->AddEnvVar(lpStr, "CONTENT_TYPE", pC->Header.ContentType);
			this->AddEnvVar(lpStr, "HTTP_CONTENT_TYPE", pC->Header.ContentType);
		}
		else {
			this->AddEnvVar(lpStr, "CONTENT_TYPE", "application/x-www-form-urlencoded");
			this->AddEnvVar(lpStr, "HTTP_CONTENT_TYPE", "application/x-www-form-urlencoded");
		}
	}

	//Used for Custom Error Pages with Scripting (BEGIN)
	if(pC->Header.OriginalRequest)
	{
		this->AddEnvVar(lpStr, "ORIGINAL_PATH_INFO", pC->Header.OriginalPathInfo);
		this->AddEnvVar(lpStr, "ORIGINAL_SCRIPT_FILENAME", pC->Header.OriginalFullRequest);
		this->AddEnvVar(lpStr, "ORIGINAL_REQUEST", pC->Header.OriginalRequest);
		this->AddEnvVar(lpStr, "ORIGINAL_PATH_TRANSLATED", pC->Header.OriginalPathTranslated);
		this->AddEnvVar(lpStr, "ORIGINAL_SCRIPT_NAME", pC->Header.OriginalScriptName);

		if(pC->Header.OriginalScriptName)
		{
			char sOriginalMimeType[MIMETYPES_MAX_TYPE_LENGTH];
			if(((CWebSite *)pC->pWebSite)->pMimeTypes->GetType(pC->Header.OriginalScriptName, sOriginalMimeType, sizeof(sOriginalMimeType)))
			{
				this->AddEnvVar(lpStr, "ORIGINAL_MIME_TYPE", sOriginalMimeType);
			}
		}
	}
	//Used for Custom Error Pages with Scripting (END)

	this->AddEnvVar(lpStr, "PATH_INFO", pC->Header.PathInfo);

	if(sScriptFileName)
	{
		this->AddEnvVar(lpStr, "SCRIPT_FILENAME", sScriptFileName);
	}
	else {
		this->AddEnvVar(lpStr, "SCRIPT_FILENAME", pC->Header.FullRequest);
	}

	CStringBuilder requestURI(pC->Header.Request);
	if(pC->Header.Query[0] != 0)
	{
		requestURI.AppendF("?%s", pC->Header.Query);
	}

	this->AddEnvVar(lpStr, "REQUEST_URI", requestURI.Buffer);
	this->AddEnvVar(lpStr, "REQUEST_METHOD", pC->Header.Method);
	this->AddEnvVar(lpStr, "QUERY_STRING", pC->Header.Query);
	sprintf_s(sTemp, sizeof(sTemp), "NetworkDLS - %s", gsTitleCaption);

	if(pC->Header.Referer && pC->Header.Referer[0] != 0)
	{
		this->AddEnvVar(lpStr, "HTTP_REFERER", pC->Header.Referer);
	}

	if(pC->Header.Cookie && pC->Header.Cookie[0] != 0)
	{
		this->AddEnvVar(lpStr, "HTTP_COOKIE", pC->Header.Cookie);
	}

	if(_strcmpi(pC->Header.Connection, "Keep-Alive") == 0)
	{
		this->AddEnvVar(lpStr, "HTTP_CONNECTION", "Keep-Alive");
	}
		
	this->AddEnvVar(lpStr, "SERVER_SOFTWARE", sTemp);
	this->AddEnvVar(lpStr, "SERVER_PROTOCOL", HTTP_VERSION);
	this->AddEnvVar(lpStr, "GATEWAY_INTERFACE", CGIINTERFACE);
	this->AddEnvVar(lpStr, "HTTP_HOST", pC->Header.Host);
	this->AddEnvVar(lpStr, "SERVER_PORT", pWebSites->pSocketPools->Port(pC->pWebSite));

	int iPortIndex = CharIndex(pC->Header.Host, ':');
	if(iPortIndex < 0)
	{
		iPortIndex = (int)strlen(pC->Header.Host);
	}
	if(iPortIndex > 0)
	{
		char sHost[MAX_HOST_STRING + 100];
		strncpy_s(sHost, sizeof(sHost), pC->Header.Host, iPortIndex);
		this->AddEnvVar(lpStr, "SERVER_NAME", sHost);
	}
	else{
		this->AddEnvVar(lpStr, "SERVER_NAME", pC->Header.Host);
	}

	GetFilePath(pC->Header.FullRequest, sTemp, sizeof(sTemp));

	this->AddEnvVar(lpStr, "PATH_TRANSLATED", pC->Header.PathTranslated);


	this->AddEnvVar(lpStr, "REMOTE_ADDR", pC->pClient->PeerAddress());
	this->AddEnvVar(lpStr, "SCRIPT_NAME", pC->Header.ScriptName);
	this->AddEnvVar(lpStr, "HTTP_ACCEPT", pC->Header.Accept);
	this->AddEnvVar(lpStr, "HTTP_ACCEPT_LANGUAGE", pC->Header.AcceptLanguage);
	this->AddEnvVar(lpStr, "HTTP_USER_AGENT", pC->Header.UserAgent);
	//this->AddEnvVar(lpStr, "CONTENT_TYPE", "*.*");
	this->AddEnvVar(lpStr, "DOCUMENT_ROOT", ((CWebSite *)pC->pWebSite)->Root);
	//this->AddEnvVar(lpStr, "REDIRECT_STATUS", ((CWebSite *)pC->pWebSite)->Root); //Non-Standard!

	/*
	this->AddEnvVar(lpStr, "PP_POOL_ID", "DefaultAppPool");
	this->AddEnvVar(lpStr, "AUTH_PASSWORD", "");
	this->AddEnvVar(lpStr, "AUTH_TYPE", "");
	this->AddEnvVar(lpStr, "AUTH_USER", "");
	this->AddEnvVar(lpStr, "CERT_COOKIE", "");
	this->AddEnvVar(lpStr, "CERT_FLAGS", "");
	this->AddEnvVar(lpStr, "CERT_ISSUER", "");
	this->AddEnvVar(lpStr, "CERT_SERIALNUMBER", "");
	this->AddEnvVar(lpStr, "CERT_SUBJECT", "");
	this->AddEnvVar(lpStr, "HTTP_ACCEPT_ENCODING", "gzip, deflate");
	this->AddEnvVar(lpStr, "HTTP_CACHE_CONTROL", "no-cache");
	this->AddEnvVar(lpStr, "HTTPS_KEYSIZE", "");
	this->AddEnvVar(lpStr, "HTTPS_SECRETKEYSIZE", "");
	this->AddEnvVar(lpStr, "HTTPS_SERVER_ISSUER", "");
	this->AddEnvVar(lpStr, "HTTPS_SERVER_SUBJECT", "");
	this->AddEnvVar(lpStr, "HTTPS", "off");
	this->AddEnvVar(lpStr, "INSTANCE_ID", "1");
	this->AddEnvVar(lpStr, "LOCAL_ADDR", "192.168.1.5");
	this->AddEnvVar(lpStr, "LOGON_USER", "");
	this->AddEnvVar(lpStr, "REMOTE_HOST", "192.168.1.5");
	this->AddEnvVar(lpStr, "REMOTE_USER", "");
	this->AddEnvVar(lpStr, "SERVER_PORT_SECURE", "0");
	this->AddEnvVar(lpStr, "UNMAPPED_REMOTE_USER", "");
	*/

	/*
	this->CopyEnvVar(lpStr, "ComSpec");
	this->CopyEnvVar(lpStr, "PATH");
	this->CopyEnvVar(lpStr, "pathext");
	this->CopyEnvVar(lpStr, "systemroot");
	this->CopyEnvVar(lpStr, "TEMP");
	this->CopyEnvVar(lpStr, "TMP");
	this->CopyEnvVar(lpStr, "windir");
	this->CopyEnvVar(lpStr, "PATHEXT");
	this->CopyEnvVar(lpStr, "NUMBER_OF_PROCESSORS");
	this->CopyEnvVar(lpStr, "PROCESSOR_ARCHITECTURE");
	this->CopyEnvVar(lpStr, "PROCESSOR_IDENTIFIER");
	this->CopyEnvVar(lpStr, "PROCESSOR_LEVEL");
	this->CopyEnvVar(lpStr, "PROCESSOR_REVISION");
	this->CopyEnvVar(lpStr, "OS");
	*/

	//Copy all environment variables:
	char *first= GetEnvironmentStrings();
	char *next = first;
	while (*next) 
	{
		if(next[0] != '=')
		{
			this->AddRawEnvVar(lpStr, next);
		}

		while(*next)
		{
			next++; // next points to the null at the end.
		}

		next++; // advance one more to get to the next string.
	}

	FreeEnvironmentStrings(first); //2012-06-21 - Resolved memory leak.

	lpStr->Append("\0", 1); //Terminate the environment.

	return this->Locks.UnlockShared(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CScriptingEngines::IsNativeExecutable(const char *sFileName)
{
	this->Locks.LockShared();
	char sExt[MAX_PATH];

	if(GetFileExtension(sFileName, sExt, sizeof(sExt)))
	{
		if(_strcmpi(sExt, ".EXE") == 0 || _strcmpi(sExt, ".CMD") == 0
			||_strcmpi(sExt, ".BAT") == 0 || _strcmpi(sExt, ".CMD") == 0)
		{
			return this->Locks.UnlockShared(true);
		}
	}

	return this->Locks.UnlockShared(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BufferDataToCGI(HANDLE hFile, const char *sBuf, unsigned long ulBufSz)
{
	char sBuffer[FILECHUNKSIZE + 1];

	unsigned long ulRPos = 0;
	unsigned long ulWPos = 0;
	unsigned long ulWritten = 0;

	FILE *hTarget = NULL;

	while(ulRPos < ulBufSz)
	{
		ulWPos = 0;

		//Fill our write buffer.
		while(ulWPos < FILECHUNKSIZE && ulRPos < ulBufSz)
		{
			sBuffer[ulWPos++] = sBuf[ulRPos++];
		}
		
		//If we have data.
		if(ulWPos > 0)
		{
			if(!WriteFile(hFile, sBuffer, ulWPos, &ulWritten, NULL))
			{
				return false;
			}
			if(ulWritten != ulWPos)
			{
				return false;
			}
		}
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Process script and send data directly to the client.
	NOTE: DO NOT not send errors from here, just return an error code.
*/
int CScriptingEngines::ProcessScript(VOID *pClient)
{
	this->Locks.LockShared();
	PEER *pC = (PEER *)pClient;
	CWebSite *pWebSite = (CWebSite *)pC->pWebSite;
	CHttp *pHttp = (CHttp *)pWebSite->pHttp;
	CWebSites *pWebSites = (CWebSites *)pWebSite->pWebSites;
	CStringBuilder Content(SCRIPTINGENGINES_MAX_STDIO_BUFFER_SIZE);

	pC->PerRequestStore.IsDynamicContent = true;

	int iResult = ProcessScript(pC, &Content, pC->Header.FullRequest);

	if(iResult == EXEC_RESULT_OK)
	{
		iResult = EXEC_RESULT_ERROR;

		int iHeaderEndPos = 0;

		//A content type is REQUIRED, so if there is a content type, we will assume that the script also terminated the HTTP header.
		bool bHasContentType = pHttp->DoesHeaderContainTag(Content.Buffer, Content.Length, "Content-type:", &iHeaderEndPos);
		//%%___|JMP 2012/05/24
		//What is the "Status:" field. It appears as though the server is looking for a redirect status, but that wouldnt be a field would it?
		int iStatusTagLength = pHttp->GetHttpHeaderTag(Content.Buffer, Content.Length, "Status:", pC->Header.Status, NULL);

		char sDefaultMimeType[MIMETYPES_MAX_TYPE_LENGTH];

		//If the scripting engine did not add a valid http content-type, then we will use the
		//	content-type which was established for the requested file (eg: .php, .pl. ect).
		if(!bHasContentType)
		{
			//If no content-type has been established for the requested file, then default to a
			//	very safe content type which wont freak out any browsers.... tell the browser, it's binary,
			if(!((CWebSite*)pC->pWebSite)->pMimeTypes->GetType(pC->Header.FullRequest, sDefaultMimeType, sizeof(sDefaultMimeType)))
			{
				strcpy_s(sDefaultMimeType, sizeof(sDefaultMimeType), "application/octet-stream");
			}
		}

		//No need to compress if we have no content? or if we are just redirecting.
		if(Content.Length > iHeaderEndPos && iStatusTagLength == 0)
		{
			if(pHttp->DoesValueContain(pC->Header.AcceptEncoding, "gzip"))
			{
				//If the script didnt return a "Content-Encoding" then we are free to enforce our own.
				if(!pHttp->DoesHeaderContainTag(Content.Buffer, iHeaderEndPos, "Content-Encoding:"))
				{
					if(((CWebSite*)pC->pWebSite)->pCompression->CompressDynamicContent() &&
						((CWebSite*)pC->pWebSite)->pCompression->IsCompressibleType(pC->Header.FullRequest))
					{
						if(Content.Length <= ((CWebSite*)pC->pWebSite)->pCompression->MaxCompressionSize()
							&& Content.Length >= ((CWebSite*)pC->pWebSite)->pCompression->MinCompressionSize())
						{
							char sUncompressed[MAX_PATH];
							char sCompressed[MAX_PATH];

							if(!((CWebSite*)pC->pWebSite)->pCompression->GetUniqueFileName(sUncompressed, sizeof(sUncompressed), ((PEER*)pC)->Header.FullRequest))
							{
								return this->Locks.UnlockShared(EXEC_RESULT_ERROR);
							}
							if(!((CWebSite*)pC->pWebSite)->pCompression->GetUniqueFileName(sCompressed, sizeof(sCompressed)))
							{
								return this->Locks.UnlockShared(EXEC_RESULT_ERROR);
							}

							//Compress the output from the scripting engine., excluding the header part (if any).
							iResult = (BufferDataToFile(sUncompressed, Content.Buffer + iHeaderEndPos, Content.Length - iHeaderEndPos) ? EXEC_RESULT_OK : EXEC_RESULT_ERROR);
							if(iResult == EXEC_RESULT_OK)
							{
								iResult = (((CWebSite*)pC->pWebSite)->pCompression->Deflate((PEER *)pClient, sUncompressed, sCompressed) ? EXEC_RESULT_OK : EXEC_RESULT_ERROR);
								if(iResult == EXEC_RESULT_OK)
								{
									unsigned long ulCompressedSize = 0;

									iResult = (FileSize(sCompressed, &ulCompressedSize) ? EXEC_RESULT_OK : EXEC_RESULT_ERROR);
									if(iResult == EXEC_RESULT_OK)
									{
										if(pWebSites->pHttp->SendOKHeader(pC, NULL, bHasContentType ? NULL : sDefaultMimeType, "gzip", ulCompressedSize, !bHasContentType))
										{
											//If the scripting engine supplied part of a header, send it.
											if(iHeaderEndPos > 0)
											{
												pWebSites->pHttp->BufferDataToClient(pC, Content.Buffer, iHeaderEndPos);
											}

											if((pC->Header.Method != NULL && _strcmpi(pC->Header.Method, "HEAD") != 0) || pC->Header.Method == NULL)
											{
												iResult = (pWebSites->pHttp->SendRawFile(pC, sCompressed) ? EXEC_RESULT_OK : EXEC_RESULT_ERROR); //Send the compressed file.
											}
											else{
												iResult = EXEC_RESULT_OK;
											}
										}
									}
								}
							}

							DeleteFile(sCompressed);
							DeleteFile(sUncompressed);

							return this->Locks.UnlockShared(iResult);
						}
					}
					else{/*No compression, continue below...*/}
				}
				else{/*No compression, continue below...*/}
			}
			else{/*No compression, continue below...*/}
		}
		else{/*No compression, continue below...*/}

		//We already have an HTTP status code.
		if(iStatusTagLength > 0)
		{
			if(pHttp->SendHttpHeader(pC, NULL, pC->Header.Status, NULL, NULL, bHasContentType ? NULL : sDefaultMimeType, Content.Length - iHeaderEndPos, NULL, !bHasContentType))
			{
				if((pC->Header.Method != NULL && _strcmpi(pC->Header.Method, "HEAD") != 0) || pC->Header.Method == NULL)
				{
					iResult = (pWebSites->pHttp->BufferDataToClient(pC, Content.Buffer, Content.Length) ? EXEC_RESULT_OK : EXEC_RESULT_ERROR);
				}
				else {
					iResult = EXEC_RESULT_OK;
				}
			}
		}
		//Can't compress, just send the data.
		else if(pWebSites->pHttp->SendOKHeader(pC, NULL, bHasContentType ? NULL : sDefaultMimeType, NULL, Content.Length - iHeaderEndPos, !bHasContentType))
		{
			if((pC->Header.Method != NULL && _strcmpi(pC->Header.Method, "HEAD") != 0) || pC->Header.Method == NULL)
			{
				iResult = (pWebSites->pHttp->BufferDataToClient(pC, Content.Buffer, Content.Length) ? EXEC_RESULT_OK : EXEC_RESULT_ERROR);
			}
			else{
				iResult = EXEC_RESULT_OK;
			}
		}
	}

	return this->Locks.UnlockShared(iResult);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Process script and return all data through lpOutput.
	NOTE: DO NOT not send errors from here, just return an error code.
*/
int CScriptingEngines::ProcessScript(VOID *pClient, CStringBuilder *lpOutput, const char *sScriptFile)
{
	this->Locks.LockShared();
	if(!FileAccess(sScriptFile, FRead))
	{
		return this->Locks.UnlockShared(EXEC_RESULT_SCRIPT_NOT_FOUND); //The script file was not found.
	}

	bool bIsNativeExecutable = this->IsNativeExecutable(sScriptFile);
	if(bIsNativeExecutable)
	{
		return this->Locks.UnlockShared(ProcessCGI(pClient, lpOutput, sScriptFile, NULL, NULL, true, 0, false));
	}
	else {
		bool bIsFastCGI = false;
		char sEngine[MAX_PATH];
		char sParams[SCRIPTINGENGINES_MAX_PARAMS_LEGTH];
		int iSuccessCode = 0;
		bool bUseSuccessCode = false;

		if(!this->GetScriptEngine(sScriptFile, sEngine, sizeof(sEngine), sParams, sizeof(sParams), &iSuccessCode, &bUseSuccessCode, &bIsFastCGI))
		{
			return this->Locks.UnlockShared(EXEC_RESULT_EXTENSION_NOT_CONFIG); //As far as we are concerned, this is not an executable file.
		}

		if(!FileAccess(sEngine, FRead))
		{
			return this->Locks.UnlockShared(EXEC_RESULT_ENGINE_NOT_FOUND); //The scripting engine executable was not found.
		}

		if(bIsFastCGI)
		{
			return this->Locks.UnlockShared(ProcessFastCGI(pClient, lpOutput, sScriptFile, sEngine, sParams));
		}
		else{
			return this->Locks.UnlockShared(ProcessCGI(pClient, lpOutput, sScriptFile, sEngine, sParams, false, iSuccessCode, bUseSuccessCode));
		}
	}

	return this->Locks.UnlockShared(EXEC_RESULT_ERROR); //Should never get here!
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
#ifdef _DEBUG
void DumpCGIRequestInfo(VOID *pClient, const char *sCommandLine, DWORD dwExitCode, CStringBuilder *Environment, const char *sPostData, int PostDataSize, CStringBuilder *stdOut)
{
	PEER *pC = (PEER *)pClient;
	CHttp *pHttp = (CHttp *)((CWebSite *)pC->pWebSite)->pHttp;
	CWebSites *pWebSites = (CWebSites *)((CWebSite *)pC->pWebSite)->pWebSites;
	CWebSite *pWebSite = (CWebSite *)pC->pWebSite;

	SYSTEMTIME st;
	memset(&st, 0, sizeof(SYSTEMTIME));

	GetSystemTime(&st);

	char sScriptFileName[MAX_PATH];
	GetFileName(pC->Header.ScriptName, sScriptFileName, sizeof(sScriptFileName));

	char sDumpPath[MAX_PATH];
	sprintf_s(sDumpPath, sizeof(sDumpPath), "%s\\ErrorLog\\CGIDumps\\%d_%d_%d-%d_%d_%d.%d (%s) (%d)\\",
		gsPath, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, sScriptFileName, dwExitCode);
	NSWFL::File::CreateFolderStructure(sDumpPath);

	char sFileName[MAX_PATH];
	FILE *hFile = NULL;

	sprintf_s(sFileName, sizeof(sFileName), "%s\\Information.txt", sDumpPath);
	if(fopen_s(&hFile, sFileName, "wb") == 0)
	{
		fprintf(hFile, "CGI Command Line: %s\r\n", sCommandLine);
		fprintf(hFile, "CGI Exit Code: %d\r\n", dwExitCode);
		fprintf(hFile, "Environment Size: %d\r\n", Environment->Length);
		fprintf(hFile, "STDIN Data Size: %d\r\n", pC->Header.PostDataSize);
		fprintf(hFile, "STDOUT Data Size: %d\r\n", stdOut->Length);
		fprintf(hFile, "-------------------------------------------------------------------\r\n");
		fprintf(hFile, "Method: %s\r\n", pC->Header.Method);
		fprintf(hFile, "Request: %s\r\n", pC->Header.Request);
		fprintf(hFile, "Version: %s\r\n", pC->Header.Version);
		fprintf(hFile, "Query: %s\r\n", pC->Header.Query);
		fprintf(hFile, "Content Type: %s\r\n", pC->Header.ContentType);
		fprintf(hFile, "Script Name: %s\r\n", pC->Header.ScriptName);
		fprintf(hFile, "Path Info: %s\r\n", pC->Header.PathInfo);
		fprintf(hFile, "Accept: %s\r\n", pC->Header.Accept);
		fprintf(hFile, "Accept Language: %s\r\n", pC->Header.AcceptLanguage);
		fprintf(hFile, "Accept Encoding: %s\r\n", pC->Header.AcceptEncoding);
		fprintf(hFile, "User Agent: %s\r\n", pC->Header.UserAgent);
		fprintf(hFile, "Host: %s\r\n", pC->Header.Host);
		fprintf(hFile, "Connection: %s\r\n", pC->Header.Connection);
		fprintf(hFile, "Cache Control: %s\r\n", pC->Header.CacheControl);
		fprintf(hFile, "Referer: %s\r\n", pC->Header.Referer);
		fprintf(hFile, "Cookie: %s\r\n", pC->Header.Cookie);
		fprintf(hFile, "Content Range: %s\r\n", pC->Header.ContentRange);
		fprintf(hFile, "Status: %s\r\n", pC->Header.Status);
		fprintf(hFile, "Full Request: %s\r\n", pC->Header.FullRequest);
		fprintf(hFile, "Path Translated: %s\r\n", pC->Header.PathTranslated);
		fprintf(hFile, "Original Path Info: %s\r\n", pC->Header.OriginalPathInfo);
		fprintf(hFile, "Original Full Request: %s\r\n", pC->Header.OriginalFullRequest);
		fprintf(hFile, "Original Request: %s\r\n", pC->Header.OriginalRequest);
		fprintf(hFile, "Original Path Translated: %s\r\n", pC->Header.OriginalPathTranslated);
		fprintf(hFile, "Original Script Name: %s\r\n", pC->Header.OriginalScriptName);
		fprintf(hFile, "Authentication Realm: %s\r\n", pC->Header.AuthenticationRealm);
		fclose(hFile);
	}

	sprintf_s(sFileName, sizeof(sFileName), "%s\\Environment.txt", sDumpPath);
	if(fopen_s(&hFile, sFileName, "wb") == 0)
	{
		fwrite(Environment->Buffer, sizeof(char), Environment->Length, hFile);
		fclose(hFile);
	}

	sprintf_s(sFileName, sizeof(sFileName), "%s\\STD_IN.txt", sDumpPath);
	if(fopen_s(&hFile, sFileName, "wb") == 0)
	{
		fwrite(sPostData, sizeof(char), PostDataSize, hFile);
		fclose(hFile);
	}

	sprintf_s(sFileName, sizeof(sFileName), "%s\\STD_OUT.txt", sDumpPath);
	if(fopen_s(&hFile, sFileName, "wb") == 0)
	{
		fwrite(stdOut->Buffer, sizeof(char), stdOut->Length, hFile);
		fclose(hFile);
	}
}
#endif
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	NOTE: DO NOT not send errors from here, just return an error code.
*/
int CScriptingEngines::ProcessCGI(VOID *pClient, CStringBuilder *lpOutput,  const char *sScriptFile,
								  const char *sEngine, const char *sParams, bool bIsNativeExecutable, int iSuccessCode, bool bUseSuccessCode)
{
	PEER *pC = (PEER *)pClient;
	CHttp *pHttp = (CHttp *)((CWebSite *)pC->pWebSite)->pHttp;
	CWebSites *pWebSites = (CWebSites *)((CWebSite *)pC->pWebSite)->pWebSites;
	CWebSite *pWebSite = (CWebSite *)pC->pWebSite;

	HANDLE hNewSTDIN, hNewSTDOut, hReadSTDOut, hWriteSTDIn;

	STARTUPINFO SI;
	memset(&SI, 0, sizeof(SI));

	PROCESS_INFORMATION PI;
	memset(&PI, 0, sizeof(PI));

	SECURITY_ATTRIBUTES SA;
	memset(&SA, 0, sizeof(SA));

	SECURITY_DESCRIPTOR SD;
	memset(&SD, 0, sizeof(SD));

	//if(IsWinNT()) //As of 1/23/2018 - we'll just assume we're not running on windows 95/98. :)
	//{
		InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION);
		SetSecurityDescriptorDacl(&SD, TRUE, NULL, FALSE);
		SA.lpSecurityDescriptor = &SD;
	//}
	//else SA.lpSecurityDescriptor = NULL;

	SA.nLength = sizeof(SECURITY_ATTRIBUTES);
	SA.bInheritHandle = TRUE; // Allow inheritable handles.

	//Create STD-IN pipe.
	if(!CreatePipe(&hNewSTDIN, &hWriteSTDIn, &SA, pWebSite->pSocketPool->FileBufferSize) )
	{
		return EXEC_RESULT_FAILED_TO_CREATE_PIPE;
	}

	//Create STD-OUT pipe.
	if(!CreatePipe(&hReadSTDOut, &hNewSTDOut, &SA, pWebSite->pSocketPool->FileBufferSize))
	{
		CloseHandle(hNewSTDIN);
		CloseHandle(hWriteSTDIn);
		return EXEC_RESULT_FAILED_TO_CREATE_PIPE;
	}

	//Set StartupInfo for the spawned process.
	SI.cb			= sizeof(STARTUPINFO);
	SI.dwFlags		= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	SI.wShowWindow	= SW_HIDE;
	SI.hStdOutput	= hNewSTDOut; //Set the new handles for the child process.
	SI.hStdError	= hNewSTDOut; //Set the new handles for the child process.
	SI.hStdInput	= hNewSTDIN;  //Set the new handles for the child process.

	CStringBuilder Environment;
	this->AddEnvVars(pC, &Environment, sScriptFile);

	int iResult = EXEC_RESULT_OK;
	char sCurrentDir[MAX_PATH];
	GetFilePath(sScriptFile, sCurrentDir, sizeof(sCurrentDir));

	CStringBuilder commandLine;

	if(bIsNativeExecutable)
	{
		commandLine.Set(sScriptFile);
		iResult = (CreateProcess(commandLine, NULL, NULL, NULL, TRUE, 0, Environment, sCurrentDir, &SI, &PI) > 0);
	}
	else{
		commandLine.Set(sParams);

		if(strlen(sParams) > 0)
		{
			commandLine.Prepend(" ");
			commandLine.Replace("%%", "[%|%]"); //Escape the %%, so that users can explicitly enter percent times. 
			commandLine.Replace("%0", sEngine);
			commandLine.Replace("%1", sScriptFile);
			commandLine.Replace("%2", pC->Header.ScriptName);
			commandLine.Replace("%3", sCurrentDir);
			commandLine.Replace("[%|%]", "%"); //Un-Escape the %%, so that users can explicitly enter percent times. 
		}

		commandLine.PrependF("\"%s\"", sEngine);
		commandLine.Terminate();

		iResult = (CreateProcess(NULL, commandLine, NULL, NULL, TRUE, DETACHED_PROCESS, Environment, sCurrentDir, &SI, &PI) > 0);
	}

	//Environment.Destroy(); //This data is really not needed anymore, just free it early...

	if(iResult != EXEC_RESULT_OK)
	{
		CloseHandle(hNewSTDIN);
		CloseHandle(hNewSTDOut);
		CloseHandle(hReadSTDOut);
		CloseHandle(hWriteSTDIn);
		return EXEC_RESULT_CREATE_PROCESS_FAILED;
	}

	//If we have post/put/get data, write it to the CGI's input.
	if(pC->Header.PostDataSize > 0)
	{
		unsigned long ulBytesWritten = 0; // Bytes written.
		WriteFile(hWriteSTDIn, pC->Header.PostData, pC->Header.PostDataSize, &ulBytesWritten, NULL);
	}

	//----------- Begin reading STDIO -------------------------------------

	DWORD dwExitCode = STILL_ACTIVE; // Process exit code.

	DWORD dwPipeMode = /*PIPE_READMODE_BYTE|*/PIPE_NOWAIT;
	if(SetNamedPipeHandleState(hReadSTDOut, &dwPipeMode, NULL, NULL))
	{
		CStringBuilder stdCollector(pWebSite->pSocketPool->FileBufferSize);
		DWORD dwStartTime = GetTickCount();
		DWORD dwTimeout = (DWORD)((CWebSite*)pC->pWebSite)->pWebsiteSettings->ScriptTimeout() * 1000;

		 while(dwExitCode == STILL_ACTIVE)
		 {
			GetExitCodeProcess(PI.hProcess, &dwExitCode);

			do{
				ReadFile(hReadSTDOut, stdCollector.Buffer, stdCollector.Alloc, (DWORD*)&stdCollector.Length, NULL);
				if(stdCollector.Length > 0)
				{
					lpOutput->Append(&stdCollector);
				}
			} while(stdCollector.Length != 0);

			if((GetTickCount() - dwStartTime) > dwTimeout || !pC->pClient->IsConnected())
			{
				TerminateProcess(PI.hProcess, iSuccessCode);
				iResult = EXEC_RESULT_TIMEOUT;
				break;
			}

			Sleep(1);
		}

		if(bUseSuccessCode && dwExitCode != iSuccessCode)
		{
			iResult = EXEC_RESULT_BAD_RETURN_CODE;
			//pWebSite->pErrorPages->SendError(pC, "500", "The scripting engine returned error code (%d) while the expected success code was (%d)", dwExitCode, iSuccessCode);
		}

		//#ifdef _DEBUG
		//DumpCGIRequestInfo(pC, commandLine.Buffer, dwExitCode, &Environment, pC->Header.PostData, pC->Header.PostDataSize, lpOutput);
		//#endif
	}
	else {
		iResult = EXEC_RESULT_NONBLOCK_FAILED;
		//pWebSite->pErrorPages->SendError(pC, "500", "The CGI engine could not be placed into non-blocking mode");
	}

	if(dwExitCode == STILL_ACTIVE)
	{
		TerminateProcess(PI.hProcess, iSuccessCode);
	}

	if(iResult == EXEC_RESULT_OK)
	{
		if(pWebSite->pSSIFiles->IsSSIFile(sScriptFile))
		{
			CStringBuilder SSIData(lpOutput);
			lpOutput->Clear();
			if(!pWebSite->pSSIFiles->ProcessServerSideInclude(pClient, SSIData.Buffer, SSIData.Length, false, lpOutput))
			{
				iResult = EXEC_RESULT_SSI_FAILED;
			}
		}
	}

	CloseHandle(hNewSTDIN);
	CloseHandle(hWriteSTDIn);
	CloseHandle(hNewSTDOut);
	CloseHandle(hReadSTDOut);

	CloseHandle(PI.hProcess);
	CloseHandle(PI.hThread);

	return iResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#include "httpext.h"

//BOOL WINAPI GetExtensionVersion(HSE_VERSION_INFO* pVer);
//typedef BOOL (__stdcall *LPGetExtensionVersion)(HSE_VERSION_INFO* pVer);
//BOOL WINAPI TerminateExtension(DWORD dwFlags);
//typedef BOOL (__stdcall *LPTerminateExtension)(DWORD dwFlags);

int CScriptingEngines::ProcessFastCGI(VOID *pClient, CStringBuilder *lpOutput,
									  const char *sScriptFile, const char *sEngine, const char *sParams)
{
	//FIXFIX: Not implemented!
/*
	HMODULE hLibrary = LoadLibrary(sEngine);
	if(!hLibrary)
	{
		return EXEC_RESULT_INVALID_ISAPI;
	}

	LPGetExtensionVersion pGetExtensionVersion = (LPGetExtensionVersion)GetProcAddress(hLibrary, "GetExtensionVersion");
	if(pGetExtensionVersion)
	{
		HSE_VERSION_INFO VI;
		memset(&VI, 0, sizeof(VI));
		pGetExtensionVersion(&VI);
		printf("pGetExtensionVersion\n");
	}
	
	LPTerminateExtension pTerminateExtension = (LPTerminateExtension)GetProcAddress(hLibrary, "TerminateExtension");
	if(pTerminateExtension)
	{
		pTerminateExtension(HSE_TERM_MUST_UNLOAD //HSE_TERM_ADVISORY_UNLOAD);
		printf("pTerminateExtension\n");
	}

	FreeLibrary(hLibrary);
*/
	return EXEC_RESULT_NOT_IMPLEMENTED;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char *CScriptingEngines::ErrorString(int iErrorCode)
{
	switch(iErrorCode)
	{
		case EXEC_RESULT_ERROR:
			return "GCI processing failed with a generic error code";
		case EXEC_RESULT_TIMEOUT:
			return "Execution timed-out";
		case EXEC_RESULT_SCRIPT_NOT_FOUND:
			return "The specified script was not found";
		case EXEC_RESULT_ENGINE_NOT_FOUND:
			return "The required scripting engine was not found";
		case EXEC_RESULT_EXTENSION_NOT_CONFIG:
			return "The file extension is not configured as executable";
		case EXEC_RESULT_INVALID_ISAPI:
			return "The SAPI module is invalid";
		case EXEC_RESULT_NOT_IMPLEMENTED:
			return "GCI feature has not been implemented";
		case EXEC_RESULT_SSI_FAILED:
			return "Failed to post process SSI";
		case EXEC_RESULT_BAD_RETURN_CODE:
			return "The CGI process returned an error code";
		case EXEC_RESULT_NONBLOCK_FAILED:
			return "The CGI process pipe could not be placed into non-blocking mode";
		case EXEC_RESULT_CREATE_PROCESS_FAILED:
			return "Failed to create the CGI process";
		case EXEC_RESULT_FAILED_TO_CREATE_PIPE:
			return "Failed to create the std pipe to the CGI process";
	}

	return "CGI Processing Error";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
