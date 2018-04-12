///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CErrorPages_CPP
#define _CErrorPages_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <shlobj.h>

extern HIMAGELIST hEnableDisableImageList; //Declared in MainDialog.cpp
extern HIMAGELIST hOnePixilImageList; //Declared in MainDialog.cpp

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../@Common/Cryptography.h"
#include "../../../NASCCL/NASCCL.H"

#include "CErrorPages.H"
#include "Entry.H"
#include "CWebSites.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::File;
using namespace NSWFL::ListView;
using namespace NSWFL::Windows;
using namespace NSWFL::Conversion;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CErrorPages::~CErrorPages(void)
{
	this->Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CErrorPages::CErrorPages(void *lpWebSites)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CErrorPages::CErrorPages(void *lpWebSites, XMLReader *xmlConfig, CErrorPages *pDefaults)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	this->Load(xmlConfig, pDefaults);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CErrorPages::Save(void)
{
	this->Locks.LockShared();

	XMLReader xmlConfig;
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

bool CErrorPages::ToXML(XMLReader *lpXML)
{
	this->Locks.LockShared();

	XMLWriter xmlConfig("ErrorPages");

	if(strlen(this->Collection.DefaultFile) == 0)
	{
		strcpy_s(this->Collection.DefaultFile,
			sizeof(this->Collection.DefaultFile), "/BuiltInErrors/GenericError.html");
	}
	xmlConfig.Add("DefaultFile", this->Collection.DefaultFile);

	for(int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		XMLWriter Item("Page");
		Item.Add("Code", this->Collection.Items[iItem].Code);
		Item.Add("ReturnCode", this->Collection.Items[iItem].ReturnCode);
		Item.Add("File", this->Collection.Items[iItem].File);
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
bool CErrorPages::Reload(void)
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

bool CErrorPages::Load(const char *sXMLFileName)
{
	this->Locks.LockExclusive();
	if(this->Initialized)
	{
		this->Destroy();
	}

	strcpy_s(this->sFileName, sizeof(this->sFileName), sXMLFileName);
	strcpy_s(this->Collection.DefaultFile, sizeof(this->Collection.DefaultFile), "");

	XMLReader xmlConfig;

	if(xmlConfig.FromFile(sXMLFileName))
	{
		XMLReader xmlEntity;
		if(xmlConfig.ToReader("ErrorPages", &xmlEntity))
		{
			this->Initialized = this->Load(&xmlEntity, NULL);
		}
		xmlEntity.Destroy();
		xmlConfig.Destroy();
	}

	return this->Locks.UnlockExclusive(this->Initialized);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CErrorPages::Load(XMLReader *xmlConfig, CErrorPages *pDefaults)
{
	this->Locks.LockExclusive();
	if(this->Initialized)
	{
		this->Destroy();
	}

	this->dwErrorCount = 0;
	this->dwWarningCount = 0;

	int iLength = 0;
	memset(&this->Collection, 0, sizeof(this->Collection));

	xmlConfig->ToString("DefaultFile", this->Collection.DefaultFile, sizeof(this->Collection.DefaultFile), &iLength);
	if(iLength == 0)
	{
		strcpy_s(this->Collection.DefaultFile,
			sizeof(this->Collection.DefaultFile), "/BuiltInErrors/GenericError.html");
	}

	//The pre version 1.0.3.8 used physical paths but the new version uses realitive - so do some backwards compatibility magic:
	const char *sBackawardCompatStr = "\\NetworkDLS\\Fortitude HTTP\\Messages\\";
	int iBackawardCompatPos = InStrI(sBackawardCompatStr, this->Collection.DefaultFile);
	if(iBackawardCompatPos > 0)
	{
		sprintf_s(this->Collection.DefaultFile, iLength + 1, "/BuiltInErrors/%s", this->Collection.DefaultFile + iBackawardCompatPos + strlen(sBackawardCompatStr));
	}

	xmlConfig->ProgressiveScan(true);
	XMLReader XPVirtualPage;

	while(xmlConfig->ToReader("Page", &XPVirtualPage))
	{
		char sCode[ERRORPAGES_MAX_CODE_LENGTH];
		char sReturnCode[ERRORPAGES_MAX_RETURNCODE_LENGTH];
		char sFile[ERRORPAGES_MAX_FILE_LENGTH];
		char sDescr[ERRORPAGES_MAX_DESCR_LENGTH];

		this->Collection.Items = (ERRORPAGE *) pMem->ReAllocate(
			this->Collection.Items, sizeof(ERRORPAGE), this->Collection.Count + 1);

		ERRORPAGE *p = &this->Collection.Items[this->Collection.Count++];

		XPVirtualPage.ToString("Code", sCode, sizeof(sCode), &iLength);
		p->Code = (char *) pMem->CloneString(sCode);

		XPVirtualPage.ToString("ReturnCode", sReturnCode, sizeof(sReturnCode), sCode, &iLength);
		p->ReturnCode = (char *) pMem->CloneString(sReturnCode);

		XPVirtualPage.ToString("File", sFile, sizeof(sFile), &iLength);
		p->File = (char *) pMem->CloneString(sFile);

		//The pre version 1.0.3.8 used physical paths but the new version uses realitive - so do some backwards compatibility magic:
		const char *sBackawardCompatStr = "\\NetworkDLS\\Fortitude HTTP\\Messages\\";
		int iBackawardCompatPos = InStrI(sBackawardCompatStr, p->File);
		if(iBackawardCompatPos > 0)
		{
			sprintf_s(p->File, iLength + 1, "/BuiltInErrors/%s", p->File + iBackawardCompatPos + strlen(sBackawardCompatStr));
		}

		XPVirtualPage.ToString("Description", sDescr, sizeof(sDescr), &iLength);
		p->Description = (char *) pMem->CloneString(sDescr);

		p->Enabled = XPVirtualPage.ToBoolean("Enable", false);

		XPVirtualPage.Destroy();
	}

	return this->Locks.UnlockExclusive(this->Initialized = true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CErrorPages::Destroy(void)
{
	this->dwErrorCount = 0;
	this->dwWarningCount = 0;

	this->Locks.LockExclusive();

	if(this->Initialized)
	{
		this->Initialized = false;
		if(this->Collection.Count > 0)
		{
			for(int iPos = 0; iPos < this->Collection.Count; iPos++)
			{
				pMem->Free(this->Collection.Items[iPos].Code);
				pMem->Free(this->Collection.Items[iPos].File);
				pMem->Free(this->Collection.Items[iPos].Description);
				pMem->Free(this->Collection.Items[iPos].ReturnCode);
			}

			pMem->Free(this->Collection.Items);

			this->Collection.Count = 0;
		}
	}
	return this->Locks.UnlockExclusive(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD CErrorPages::WarningCount(void)
{
	return this->dwWarningCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD CErrorPages::ErrorCount(void)
{
	return this->dwErrorCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CErrorPages::AuthorizationRequired(VOID *pClient)
{
	/*
	StringBuilder XtraHdrs;

	XtraHdrs.Append("WWW-Authenticate: Basic");
	XtraHdrs.Append(" realm=Secure Area #1");
	XtraHdrs.Append("\r\n");

	for(int iPos = 0; iPos < this->Collection.Count; iPos++)
	{
		if(_strcmpi(this->Collection.Items[iPos].Code, "401") == 0)
		{
			return this->SendErrorEx(pClient, "401",
				this->Collection.Items[iPos].Description,
				"", this->Collection.Items[iPos].File, XtraHdrs.Buffer);
		}
	}

	return this->SendErrorEx(pClient, "401",
		"</i>Unauthorised</i>", "", this->Collection.DefaultFile, XtraHdrs.Buffer);
	*/
	return false; //FIXFIX: Not Implemented.
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CErrorPages::SendError(VOID *pClient, const char *sErrorCode)
{
	return this->SendError(pClient, sErrorCode, "");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CErrorPages::SendError(VOID *pClient, const char *sErrorCode, const char *sFormat, ...)
{
	va_list ArgList;
	va_start(ArgList, sFormat);

	bool bResult = false;

	int iMemoryRequired = _vscprintf(sFormat, ArgList);

	char *sBuf = (char *) pMem->Allocate(sizeof(char), iMemoryRequired + 1);

	int iSz = _vsprintf_s_l(sBuf, iMemoryRequired + 1, sFormat, NULL, ArgList);
	va_end(ArgList);

	for(int iPos = 0; iPos < this->Collection.Count; iPos++)
	{
		if(_strcmpi(sErrorCode, this->Collection.Items[iPos].Code) == 0)
		{
			bResult = this->SendErrorEx(pClient, &this->Collection.Items[iPos], sBuf, NULL);

			pMem->Free(sBuf);

			return bResult;
		}
	}

	bResult = SendFailsafeError(pClient, sErrorCode, "Unknown Error.", sBuf);

	pMem->Free(sBuf);

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CErrorPages::SendErrorEx(VOID *pClient, ERRORPAGE *pErrorPage, const char *sDetails, const char *sExtraHeaders)
{
	PEER *pC = (PEER *)pClient;
	CWebSite *pWebSite = (CWebSite *)pC->pWebSite;
	CHttp *pHttp = (CHttp *)pWebSite->pHttp;

	if(pC->PerRequestStore.HTTPErrorOccured)
	{
		return true;
	}
	pC->PerRequestStore.HTTPErrorOccured = true;

	StringBuilder errorFileName;

	ROOT *pVirtualRoot = pWebSite->pVirtualRoots->IsVirtualRoot(pErrorPage->File);

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
					bool bResult = false;
					char sErrorMessage[1024];
					if(GetLastError(sErrorMessage, sizeof(sErrorMessage)))
					{
						bResult = this->SendFailsafeError(pClient, pErrorPage->Code, pErrorPage->Description, sDetails);
					}
					else{
						bResult = this->SendFailsafeError(pClient, pErrorPage->Code, pErrorPage->Description, sDetails);
					}

					CloseHandle(hToken);
					return bResult;
				}
				CloseHandle(hToken);
			}
			else{
				return this->SendFailsafeError(pClient, pErrorPage->Code, pErrorPage->Description, sDetails);
			}
		}

		errorFileName.Append(pVirtualRoot->Path);
	}
	else
	{
		errorFileName.Append(pWebSite->Root);
	}

	if(errorFileName.Buffer[errorFileName.Length - 1] == '\\')
	{
		errorFileName.Terminate(errorFileName.Length - 1);
	}

	int iTokPos = 0;
	int iRequestLength = (int) strlen(pErrorPage->File);
	if(pVirtualRoot)
	{
		iTokPos = pVirtualRoot->RootLength;
	}

	//Skip any string of slashes.
	while(pErrorPage->File[iTokPos] == '/' || pErrorPage->File[iTokPos] == '\\')
	{
		iTokPos++;
	}

	//NOTE: We do not support "extra path info" for error files... it could be easily implemented, but is has not yet been done.

	if(iTokPos < iRequestLength)
	{
		errorFileName.AppendF("\\%s", pErrorPage->File + iTokPos);
	}
	CorrectReversePath(errorFileName.Buffer, errorFileName.Length + 1, true);

	//At this point, we have the full physical path to our error file. Verify
	//	the file exists, if it doesnt then we just want to send a generic header.
	if(!FileAccess(errorFileName.Buffer, FExist))
	{
		return this->SendFailsafeError(pClient, pErrorPage->Code, pErrorPage->Description, sDetails);
	}

	//Keep track of warnings and errors. Errors are all 500 (Internal Server) errors
	//	and warnings are all other errors except 404 (file not found) errors.
	if(strncmp(pErrorPage->Code, "404", 3) != 0)
	{
		if(strncmp(pErrorPage->Code, "500", 3) == 0)
		{
			this->dwErrorCount++;
		}
		else {
			this->dwWarningCount++;
		}
	}

	StringBuilder Str(1000);

	if(pWebSite->pScriptingEngines->IsScriptFile(errorFileName.Buffer))
	{
		pC->Header.OriginalPathInfo = pC->Header.PathInfo;
		pC->Header.OriginalFullRequest = pC->Header.FullRequest;
		pC->Header.OriginalRequest = pC->Header.Request;
		pC->Header.OriginalPathTranslated = pC->Header.PathTranslated;
		pC->Header.OriginalScriptName = pC->Header.ScriptName;

		//Temporarily pretend we are handling a totally different request.
		pC->Header.PathTranslated = errorFileName.Buffer;
		pC->Header.FullRequest = errorFileName.Buffer;
		pC->Header.ScriptName = (char*)pErrorPage->File;
		pC->Header.Request = (char*)pErrorPage->File;
		pC->Header.PathInfo = (char*)pErrorPage->File;

		int iResult = ((CWebSite*)pC->pWebSite)->pScriptingEngines->ProcessScript(pC, &Str, errorFileName.Buffer);

		//Reset the session back to how it was before we commandeered the request.
		pC->Header.PathInfo = pC->Header.OriginalPathInfo;
		pC->Header.FullRequest = pC->Header.OriginalFullRequest;
		pC->Header.Request = pC->Header.OriginalRequest;
		pC->Header.PathTranslated = pC->Header.OriginalPathTranslated;
		pC->Header.ScriptName = pC->Header.OriginalScriptName;

		if(iResult != EXEC_RESULT_OK)
		{
			return this->SendFailsafeError(pClient, pErrorPage->Code, pErrorPage->Description, sDetails);
		}
	}
	else if(pWebSite->pSSIFiles->IsSSIFile(errorFileName.Buffer))
	{
		if(!pWebSite->pSSIFiles->ProcessServerSideInclude(pC, errorFileName.Buffer, false, &Str))
		{
			return this->SendFailsafeError(pClient, pErrorPage->Code, pErrorPage->Description, sDetails);
		}
	}
	else {
		if(!Str.AppendFromFile(errorFileName.Buffer))
		{
			return this->SendFailsafeError(pClient, pErrorPage->Code, pErrorPage->Description, sDetails);
		}
	}

	Str.Replace("###Code###", pErrorPage->Code);
	Str.Replace("###Description###", pErrorPage->Description);
	Str.Replace("###Details###", sDetails);

	int iHeaderEndPos = 0;

	//A content type is REQUIRED, so if there is a content type, we will assume that the script also terminated the HTTP header.
	bool bHasContentType = pHttp->DoesHeaderContainTag(Str.Buffer, Str.Length, "Content-type:", &iHeaderEndPos);
	//%%___|JMP 2012/05/24
	//What is the "Status:" field. It appears as though the server is looking for a redirect status, but that wouldnt be a field would it?
	int iStatusTagLength = pHttp->GetHttpHeaderTag(Str.Buffer, Str.Length, "Status:", pC->Header.Status, NULL);

	char sDefaultMimeType[MIMETYPES_MAX_TYPE_LENGTH];

	//If the scripting engine did not add a valid http content-type, then we will use the
	//	content-type which was established for the requested file (eg: .php, .pl. ect).
	if(!bHasContentType)
	{
		//If no content-type has been established for the requested file, then default to a
		//	very safe content type which wont freak out any browsers.... tell the browser, it's binary,
		if(!((CWebSite*)pC->pWebSite)->pMimeTypes->GetType(errorFileName.Buffer, sDefaultMimeType, sizeof(sDefaultMimeType)))
		{
			strcpy_s(sDefaultMimeType, sizeof(sDefaultMimeType), "application/octet-stream");
		}
	}

	char *sErrorCode = pErrorPage->Code;
	char *sErrorCodeDesc = pErrorPage->Description;

	//Find the description for the return code in the error pages collection.
	for(int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		if(_strcmpi(this->Collection.Items[iItem].Code, pErrorPage->ReturnCode) == 0)
		{
			sErrorCode = pErrorPage->ReturnCode;
			sErrorCodeDesc = this->Collection.Items[iItem].Description;
			break;
		}
	}

	if(pHttp->SendHttpHeader(pC, NULL, sErrorCode, sErrorCodeDesc, NULL,
		bHasContentType ? NULL : sDefaultMimeType, Str.Length - iHeaderEndPos, sExtraHeaders, !bHasContentType))
	{
		if((pC->Header.Method != NULL && _strcmpi(pC->Header.Method, "HEAD") != 0) || pC->Header.Method == NULL)
		{
			return pHttp->BufferDataToClient(pC, &Str);
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	This error reporting function is used to send the peer an error message in the event that 
		all other attempts have failed (such as when an error occurs while sending an error).
*/
bool CErrorPages::SendFailsafeError(VOID *pClient, const char *sErrCode, const char *sErrDesc, const char *sDetails)
{
	PEER *pC = (PEER *)pClient;
	CWebSite *pWebSite = (CWebSite *)pC->pWebSite;
	CHttp *pHttp = (CHttp *)pWebSite->pHttp;

	StringBuilder genericError;

	genericError.AppendF(
			"<html><head><title>An error has occured</title></head><body><strong>Error %s: %s</strong><br /><br />%s<br /><br /><i>NetworkDLS - %s v%s</i></body></html>",
			sErrCode == NULL ? "" : sErrCode,
			sErrDesc == NULL ? "" : sErrDesc,
			sDetails == NULL ? "" : sDetails,
			gsTitleCaption, gsFileVersion
		);

	if(pHttp->SendHttpHeader(pC, NULL, sErrCode, sErrDesc, NULL, "text/html", genericError.Length, NULL, true))
	{
		if((pC->Header.Method != NULL && _strcmpi(pC->Header.Method, "HEAD") != 0) || pC->Header.Method == NULL)
		{
			return pHttp->BufferDataToClient(pC, &genericError);
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
