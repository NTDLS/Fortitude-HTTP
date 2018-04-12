///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CSSIFiles_CPP
#define _CSSIFiles_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <ShellAPI.H>
#include <Psapi.h>
#include <shlobj.h>

extern HIMAGELIST hEnableDisableImageList; //Declared in MainDialog.cpp
extern HIMAGELIST hOnePixilImageList; //Declared in MainDialog.cpp

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CSSIFiles.H"
#include "Entry.H"
#include "CWebSites.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::File;
using namespace NSWFL::ListView;
using namespace NSWFL::Registry;
using namespace NSWFL::System;
using namespace NSWFL::Conversion;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CSSIFiles::~CSSIFiles(void)
{
	this->Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CSSIFiles::CSSIFiles(void *lpWebSites)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CSSIFiles::CSSIFiles(void *lpWebSites, XMLReader *xmlConfig, CSSIFiles *pDefaults)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
	this->Load(xmlConfig, pDefaults);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSSIFiles::Save(void)
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

bool CSSIFiles::ToXML(XMLReader *lpXML)
{
	this->Locks.LockShared();

	XMLWriter xmlConfig("SSIFiles");

	xmlConfig.AddBool("Enable", this->Collection.Enabled);

	for(int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		XMLWriter Item("File");
		Item.Add("Extension", this->Collection.Items[iItem].Extension);
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

bool CSSIFiles::Reload(void)
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

bool CSSIFiles::Load(const char *sXMLFileName)
{
	this->Locks.LockExclusive();
	if(this->Initialized)
	{
		this->Destroy();
	}

	strcpy_s(this->sFileName, sizeof(this->sFileName), sXMLFileName);

	XMLReader xmlConfig;

	if(xmlConfig.FromFile(sXMLFileName))
	{
		XMLReader xmlEntity;
		if(xmlConfig.ToReader("SSIFiles", &xmlEntity))
		{
			this->Initialized = this->Load(&xmlEntity, NULL);
		}
		xmlEntity.Destroy();
		xmlConfig.Destroy();
	}

	return this->Locks.UnlockExclusive(this->Initialized);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSSIFiles::Load(XMLReader *xmlConfig, CSSIFiles *pDefaults)
{
	this->Locks.LockExclusive();
	if(this->Initialized)
	{
		this->Destroy();
	}

	int iLength = 0;
	memset(&this->Collection, 0, sizeof(this->Collection));

	this->Collection.Enabled = xmlConfig->ToBoolean("Enable", false);

	xmlConfig->ProgressiveScan(true);
	XMLReader XPSSIFile;

	while(xmlConfig->ToReader("File", &XPSSIFile))
	{
		char sExt[SSIFILES_MAX_EXT_LENGTH];
		char sDescr[SSIFILES_MAX_EXT_LENGTH];

		this->Collection.Items = (SSIFILE *)
			pMem->ReAllocate(this->Collection.Items, sizeof(SSIFILE), this->Collection.Count + 1);

		SSIFILE *p = &this->Collection.Items[this->Collection.Count++];

		XPSSIFile.ToString("Extension", sExt, sizeof(sExt), &iLength);
		p->Extension = (char *) pMem->CloneString(LCase(sExt, iLength));

		XPSSIFile.ToString("Description", sDescr, sizeof(sDescr), &iLength);
		p->Description = (char *) pMem->CloneString(sDescr);

		p->Enabled = XPSSIFile.ToBoolean("Enable", false);

		XPSSIFile.Destroy();
	}

	return this->Locks.UnlockExclusive(this->Initialized = true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSSIFiles::Enabled(void)
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->Collection.Enabled && (this->Collection.Count > 0));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSSIFiles::Destroy(void)
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
	Returns true if the file should be process as an SSI file, otherwise returns false.
*/
bool CSSIFiles::IsSSIFile(const char *sFileName)
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
				if(_strcmpi(sExt, this->Collection.Items[iPos].Extension) == 0)
				{
					return this->Locks.UnlockShared(true);
				}
			}
		}
	}
	
	return this->Locks.UnlockShared(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSSIFiles::ProcessServerSideInclude(VOID *pClient, const char *sFileName)
{
	SSIINFO SSII;
	memset(&SSII, 0, sizeof(SSII));
	return this->ProcessServerSideIncludeEx(&SSII, pClient, sFileName);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSSIFiles::ProcessServerSideInclude(VOID *pClient, char *sFileBuf,
				int iFileBufSz, bool bSend, StringBuilder *lpBuf)
{
	SSIINFO SSII;
	memset(&SSII, 0, sizeof(SSII));
	return this->ProcessServerSideIncludeEx(&SSII, pClient, sFileBuf, iFileBufSz, bSend, lpBuf);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSSIFiles::ProcessServerSideInclude(VOID *pClient,
	const char *sFileName, bool bSend, StringBuilder *pBuffer)
{
	SSIINFO SSII;
	memset(&SSII, 0, sizeof(SSII));
	return this->ProcessServerSideIncludeEx(&SSII, pClient, sFileName, bSend, pBuffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSSIFiles::ProcessServerSideIncludeEx(SSIINFO *pSSII, VOID *pClient, const char *sFileName)
{
	this->Locks.LockShared();
	StringBuilder Buffer;
	return this->Locks.UnlockShared(ProcessServerSideIncludeEx(pSSII, pClient, sFileName, true, &Buffer));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSSIFiles::ProcessServerSideIncludeEx(SSIINFO *pSSII, VOID *pClient, const char *sFileName, bool bSend, StringBuilder *pBuffer)
{
	this->Locks.LockShared();
	PEER *pC = (PEER *)pClient;
	CWebSite *pWebSite = (CWebSite *)pC->pWebSite;
	CHttp *pHttp = (CHttp *)pWebSite->pHttp;
	CWebSites *pWebSites = (CWebSites *)pWebSite->pWebSites;

	pC->PerRequestStore.IsDynamicContent = true;

	FILE *fSource = NULL;           // The handle of the RAW SSI file.
	char *sFileBuf = NULL;          // The buffer that contains the RAW SSI file.
	unsigned long ulFileBufSz = 0;         // The size of the buffer that contains the RAW SSI file.

	//Get the size of the RAW SSI file.
	if(!FileSize(sFileName, &ulFileBufSz))
	{
		StringBuilder errorMessage;
		GetLastError(&errorMessage);
		pWebSite->pErrorPages->SendError(pC, "500", "SSI Error->FileSize->%s", errorMessage.Buffer);
		return this->Locks.UnlockShared(false);
	}

	if(ulFileBufSz <= 0)
	{
		return this->Locks.UnlockShared(false);
	}

	//Open the RAW SSI file.
	if(fopen_s(&fSource, sFileName, "rb") != 0)
	{
		StringBuilder errorMessage;
		GetLastError(&errorMessage);
		pWebSite->pErrorPages->SendError(pC, "500", "SSI Error->fopen_s->%s", errorMessage.Buffer);
		return this->Locks.UnlockShared(false);
	}

	//Allocate RAM for our RAW SSI file.
	sFileBuf = (char *) pMem->Allocate(sizeof(char), ulFileBufSz + 1);

	//Read the entire RAW SSI file into the buffer.
	ulFileBufSz = (int)fread(sFileBuf, sizeof(char), ulFileBufSz, fSource);

	fclose(fSource);

	bool bResult = ProcessServerSideIncludeEx(pSSII, pC, sFileBuf, ulFileBufSz, bSend, pBuffer);

	pMem->Free(sFileBuf);

	return this->Locks.UnlockShared(bResult);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSSIFiles::ProcessServerSideIncludeEx(SSIINFO *pSSII, VOID *pClient, char *sFileBuf,
	int iFileBufSz, bool bSend, StringBuilder *lpBuf)
{
	this->Locks.LockShared();
	PEER *pC = (PEER *)pClient;
	CWebSite *pWebSite = (CWebSite *)pC->pWebSite;
	CHttp *pHttp = (CHttp *)pWebSite->pHttp;
	CWebSites *pWebSites = (CWebSites *)pWebSite->pWebSites;

	int iRPos = 0;
	int iWPos = 0;

	//------------------------------------------------------------------------------------
	// Begin SSI Processing
	//------------------------------------------------------------------------------------

	int iSFlagLen = (int)strlen(SSI_BEGIN_FLAG);
	int iEFlagLen = (int)strlen(SSI_END_FLAG);
	
	int iFlagStartPos = 0;
	int iFlagEndPos = 0;
	int iSearchPos = 0;
	int iSSISz = 0;

	char sSSI[1024];
	char sSSIOth[1024];
	char sFileName[MAX_PATH];

	int iRPosA = 0;
	int iWPosA = 0;

	bool bPerformedExecution = false;

	if(iFileBufSz > 1024*1024*10)
	{
		pWebSite->pErrorPages->SendError(pC, "500", "SSI dataset is too large (%d bytes)", lpBuf->Length);
		return this->Locks.UnlockShared(false);
	}

	if(pSSII->NestingDepth++ > 128)
	{
		pWebSite->pErrorPages->SendError(pC, "500", "SSI nesting depth is too great");
		return this->Locks.UnlockShared(false);
	}

	while(true)
	{
		if((iFlagStartPos = InStr(SSI_BEGIN_FLAG, sFileBuf, iFileBufSz, iSearchPos)) < 0)
		{
			iFlagStartPos = iFileBufSz;
			break;
			//No SSI start flags were found.
		}

		lpBuf->Append(sFileBuf + iRPos, iFlagStartPos - iRPos);

		if((iFlagEndPos = InStr(SSI_END_FLAG, sFileBuf, iFileBufSz, iFlagStartPos)) < 0)
		{
			//No SSI end flags were found.
			lpBuf->Append("<!--SSI ERROR(Expected: \"-->\")-->");
			break;
		}

		if(Mid(sFileBuf, iFileBufSz, iFlagStartPos + iSFlagLen, (iFlagEndPos - iFlagStartPos) - iSFlagLen, sSSI, sizeof(sSSI)) < 0)
		{
			lpBuf->Append("<!--SSI ERROR(Expected: \"-->\")-->");
			break;
		}

		iSSISz = (iFlagEndPos - iFlagStartPos) - iSFlagLen;

		if(_strnicmp(sSSI, "Echo ", 5) == 0)
		{
			iRPosA = 5;
			iWPosA = 0;

			//Skip White-Spaces.
			while(IsWhiteSpace(sSSI[iRPosA]) && iRPosA < iSSISz)
				iRPosA++;

			//Parse the second SSI command Word.
			while(!IsWhiteSpace(sSSI[iRPosA]) && sSSI[iRPosA] != '=' && iRPosA < iSSISz)
			{
				if(iWPosA >= sizeof(sSSIOth))
				{
					lpBuf->Append("<!--SSI ERROR(Secondary tag is too long)-->");
					break;
				}
				sSSIOth[iWPosA++] = sSSI[iRPosA++];
			}
			sSSIOth[iWPosA++] = '\0';

			if(sSSI[iRPosA] != '=')
			{
				lpBuf->Append("<!--SSI ERROR(Expected: \"=\")-->");
			}

			iRPosA++; //Skip the equal sign.

			//Skip White-Spaces.
			while(IsWhiteSpace(sSSI[iRPosA]) && iRPosA < iSSISz)
				iRPosA++;

			if(_strcmpi(sSSIOth, "VAR") == 0)
			{
				if(sSSI[iRPosA] != '\"')
				{
					lpBuf->Append("<!--SSI ERROR(Expected: \"\"\")-->");
				}
				else{
					iWPosA = 0;
					iRPosA++; //Skip the double quote.

					//Parse the second SSI command Word.
					while(!IsWhiteSpace(sSSI[iRPosA]) && sSSI[iRPosA] != '"' && iRPosA < iSSISz)
					{
						if(iWPosA >= sizeof(sSSIOth))
						{
							lpBuf->Append("<!--SSI ERROR(Secondary tag is too long)-->");
							break;
						}
						sSSIOth[iWPosA++] = sSSI[iRPosA++];
					}
					sSSIOth[iWPosA++] = '\0';

					EchoSSIVar(pC, lpBuf, sSSIOth);
				}
			}
			else if(_strcmpi(sSSIOth, "ENV") == 0)
			{
				if(sSSI[iRPosA] != '\"')
				{
					lpBuf->Append("<!--SSI ERROR(Expected: \"\"\")-->");
				}
				else{
					iWPosA = 0;
					iRPosA++; //Skip the double quote.

					//Parse the second SSI command Word.
					while(!IsWhiteSpace(sSSI[iRPosA]) && sSSI[iRPosA] != '"' && iRPosA < iSSISz)
					{
						if(iWPosA >= sizeof(sSSIOth))
						{
							lpBuf->Append("<!--SSI ERROR(Secondary tag is too long)-->");
							break;
						}
						sSSIOth[iWPosA++] = sSSI[iRPosA++];
					}
					sSSIOth[iWPosA++] = '\0';

					size_t iReqSz = 0;

					getenv_s(&iReqSz, NULL, 0, sSSIOth);

					if(iReqSz > 0)
					{
						char *sBuf = NULL;

						if((sBuf = (char *) pMem->Allocate(sizeof(char), iReqSz + 1)) != NULL)
						{
							if(getenv_s(&iReqSz, sBuf, iReqSz, sSSIOth) == 0)
							{
								lpBuf->Append(sBuf);
							}
							else{
								lpBuf->Append("");
							}

							pMem->Free(sBuf);
						}
						else{
							lpBuf->Append("");
						}
					}
					else{
						lpBuf->Append("");
					}
				}
			}
			else{
				lpBuf->Append("<!--SSI ERROR(Expected: \"VAR\")-->");
			}
			
		}
		else if(_strnicmp(sSSI, "FlastMod ", 9) == 0)
		{
			char sTime[255];
			
			GetVirtualOrFileName(pC, lpBuf, 9, sSSI, iSSISz, sFileName);
			if(strlen(sFileName) > 0)
			{
				GetLastFileModificationDate(sFileName, sTime, sizeof(sTime));
				lpBuf->Append(sTime);
			}
		}
		else if(_strnicmp(sSSI, "FSize ", 6) == 0)
		{
			GetVirtualOrFileName(pC, lpBuf, 6, sSSI, iSSISz, sFileName);
			if(strlen(sFileName) > 0)
			{
				__int64 i64FileSize = 0;
				if(!FileSize(sFileName, &i64FileSize))
				{
					i64FileSize = 0;
				}

				lpBuf->Append(i64FileSize);
			}
		}
		else if(_strnicmp(sSSI, "Include ", 8) == 0)
		{
			GetVirtualOrFileName(pC, lpBuf, 8, sSSI, iSSISz, sFileName);
			if(strlen(sFileName) > 0)
			{
				if(this->IsSSIFile(sFileName))
				{
					StringBuilder RecursiveBuffer;
					if(this->ProcessServerSideIncludeEx(pSSII, pC, sFileName, false, &RecursiveBuffer))
					{
						lpBuf->Append(&RecursiveBuffer);
					}
					else{
						lpBuf->Append("<!--SSI ERROR(Error parsing recursive SSI)-->");
					}
				}
				else{
					lpBuf->AppendFromFile(sFileName);
				}
			}
		}
		else if(_strnicmp(sSSI, "Exec ", 5) == 0)
		{
			bPerformedExecution = true;

			GetCGIOrCMDName(pC, lpBuf, 5, sSSI, iSSISz, sFileName);
			if(strlen(sFileName) > 0)
			{
				int iResult = 0;
				StringBuilder Str;
				if((iResult = ((CWebSite*)pC->pWebSite)->pScriptingEngines->ProcessScript(pC, &Str, sFileName)) == EXEC_RESULT_OK)
				{
					lpBuf->Append(Str.Buffer, Str.Length);
				}
				else{
					lpBuf->Append("<!--SSI ERROR(");
					lpBuf->Append(((CWebSite*)pC->pWebSite)->pScriptingEngines->ErrorString(iResult));
					lpBuf->Append(")-->");
				}
			}
		}
		else {
			lpBuf->Append("<!--SSI ERROR(Unknown SSI tag)-->");
		}
		
		iSearchPos = (iFlagEndPos + iEFlagLen);
		iRPos = iSearchPos;
	}

	lpBuf->Append(sFileBuf + iRPos, iFlagStartPos - iRPos); //Append the file tail.

	//------------------------------------------------------------------------------------
	// End SSI Processing
	//------------------------------------------------------------------------------------

	if(bSend)
	{
		int iHeaderEndPos = 0;
		int iCRLFsToSkip = 0;

		bool bHasContentType = false;
		
		//If we executed any scripts / programs then we need to check for headers.
		if(bPerformedExecution)
		{
			bHasContentType = pHttp->DoesHeaderContainTag(lpBuf->Buffer, lpBuf->Length, "Content-type:", &iHeaderEndPos);

			//If no content type was specified, then we need to include one. Default to text/html.
			if(bHasContentType)
			{
				while(lpBuf->Buffer[iCRLFsToSkip] == '\r' || lpBuf->Buffer[iCRLFsToSkip] == '\n')
				{
					iCRLFsToSkip++; //We do not want to send the preceeding line-feed because our header will contain a trailing line-feed.
				}
			}
		}

		if(pHttp->DoesValueContain(pC->Header.AcceptEncoding, "gzip"))
		{
			if(((CWebSite*)pC->pWebSite)->pCompression->CompressDynamicContent())
			{
				if(lpBuf->Length <= ((CWebSite*)pC->pWebSite)->pCompression->MaxCompressionSize()
					&& lpBuf->Length >= ((CWebSite*)pC->pWebSite)->pCompression->MinCompressionSize())
				{
					bool bResult = false;
					char sUncompressed[MAX_PATH];
					char sCompressed[MAX_PATH];

					if(!((CWebSite*)pC->pWebSite)->pCompression->GetUniqueFileName(
						sUncompressed, sizeof(sUncompressed), ((PEER*)pC)->Header.FullRequest))
					{
						StringBuilder errorMessage;
						GetLastError(&errorMessage);
						pWebSite->pErrorPages->SendError(pC, "500", "SSI Error->GetUniqueFileName->%s", errorMessage.Buffer);
						return this->Locks.UnlockShared(false);
					}
					if(!((CWebSite*)pC->pWebSite)->pCompression->GetUniqueFileName(
						sCompressed, sizeof(sCompressed)))
					{
						StringBuilder errorMessage;
						GetLastError(&errorMessage);
						pWebSite->pErrorPages->SendError(pC, "500", "SSI Error->GetUniqueFileName->%s", errorMessage.Buffer);
						return this->Locks.UnlockShared(false);
					}

					if(BufferDataToFile(sUncompressed, lpBuf->Buffer + iHeaderEndPos, lpBuf->Length - iHeaderEndPos))
					{
						if(((CWebSite*)pC->pWebSite)->pCompression->Deflate((PEER *)pClient, sUncompressed, sCompressed))
						{
							unsigned long ulCompressedSize = 0;

							//Get the size of the compressed SSI file.
							if(!FileSize(sCompressed, &ulCompressedSize))
							{
								StringBuilder errorMessage;
								GetLastError(&errorMessage);
								pWebSite->pErrorPages->SendError(pC, "500", "SSI Error->FileSize->%s", errorMessage.Buffer);
							}
							else{
								//Send the header - if no content type was specified, then we need to include one. Default to text/html.
								if(pWebSites->pHttp->SendOKHeader(pC, NULL, bHasContentType ? NULL : "text/html", "gzip", ulCompressedSize))
								{
									if((pC->Header.Method != NULL && _strcmpi(pC->Header.Method, "HEAD") != 0) || pC->Header.Method == NULL)
									{
										if(pWebSites->pHttp->SendRawFile(pC, sCompressed))
										{
											bResult = true;
										}
									}
									else{
										bResult = true;
									}
								}

								//If the scripting engine supplied part of a header, send it.
								if(iHeaderEndPos > 0)
								{
									pWebSites->pHttp->BufferDataToClient(pC, lpBuf->Buffer + iCRLFsToSkip, iHeaderEndPos - iCRLFsToSkip);
								}
							}
						}
						else{
							StringBuilder errorMessage;
							GetLastError(&errorMessage);
							pWebSite->pErrorPages->SendError(pC, "500", "SSI Error->Deflate->%s", errorMessage.Buffer);
						}
					}
					else{
						StringBuilder errorMessage;
						GetLastError(&errorMessage);
						pWebSite->pErrorPages->SendError(pC, "500", "SSI Error->BufferDataToFile->%s [%s]", errorMessage.Buffer, sUncompressed);
					}

					DeleteFile(sCompressed);
					DeleteFile(sUncompressed);
					return this->Locks.UnlockShared(bResult);
				}
				else{
					//No compression, continue below...
				}
			}
			else{
				//No compression, continue below...
			}
		}
		else{
			//No compression, continue below...
		}

		//Can't compress, just send the data.
		//Send the header - if no content type was specified, then we need to include one. Default to text/html.
		if(pWebSites->pHttp->SendOKHeader(pC, NULL, bHasContentType ? NULL : "text/html", NULL, lpBuf->Length - iHeaderEndPos, !bHasContentType))
		{
			if((pC->Header.Method != NULL && _strcmpi(pC->Header.Method, "HEAD") != 0) || pC->Header.Method == NULL)
			{
				if(pWebSites->pHttp->BufferDataToClient(pC, lpBuf->Buffer + iCRLFsToSkip, lpBuf->Length - iCRLFsToSkip))
				{
					return this->Locks.UnlockShared(true);
				}
				else{
					//We've already sent a header, so we cant go back on it now.
					//	All we can do is abandon the client...
					return this->Locks.UnlockShared(false);
				}
				return this->Locks.UnlockShared(true);
			}
		}
		else{
			pWebSite->pErrorPages->SendError(pC, "500", "SSI Error->SendOKHeader");
			return this->Locks.UnlockShared(false);
		}
	}

	return this->Locks.UnlockShared(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSSIFiles::GetSSITagEx(VOID *pClient, StringBuilder *lpBuf, int iInitRPos, const char *sSSI,
				 const int iSSISz, char *sOutBuffer, int iOutBufMaxSz,
				 const char *sOpt1, const char *sOpt2)
{
	this->Locks.LockShared();
	PEER *pC = (PEER *)pClient;

	char sSSIOth[1024];

	int iRPosA = iInitRPos;
	int iWPosA = 0;

	strcpy_s(sOutBuffer, iOutBufMaxSz, "");
	
	//Skip White-Spaces.
	while(IsWhiteSpace(sSSI[iRPosA]) && iRPosA < iSSISz)
		iRPosA++;

	//Parse the second SSI command Word.
	while(!IsWhiteSpace(sSSI[iRPosA]) && sSSI[iRPosA] != '=' && iRPosA < iSSISz)
	{
		if(iWPosA >= sizeof(sSSIOth))
		{
			lpBuf->Append("<!--SSI ERROR(Secondary tag is too long)-->");
			return false;
		}
		sSSIOth[iWPosA++] = sSSI[iRPosA++];
	}
	sSSIOth[iWPosA++] = '\0';

	if(sSSI[iRPosA] == '=')
		iRPosA++;

	//Skip White-Spaces.
	while(IsWhiteSpace(sSSI[iRPosA]) && iRPosA < iSSISz)
		iRPosA++;

	if(sSSI[iRPosA] != '\"')
	{
		lpBuf->Append("<!--SSI ERROR(Expected: \"\"\")-->");
		return this->Locks.UnlockShared(false);
	}
	else{
		iRPosA++;

		iWPosA = 0;

		if(_strcmpi(sSSIOth, sOpt1) == 0)
		{
			//Parse the file name.
			while(sSSI[iRPosA] != '"' && iRPosA < iSSISz)
			{
				if(iWPosA >= iOutBufMaxSz)
				{
					lpBuf->Append("<!--SSI ERROR(Tag is too long)-->");
					break;
				}
				sOutBuffer[iWPosA++] = sSSI[iRPosA++];
			}
			sOutBuffer[iWPosA++] = '\0';
		}
		else if(_strcmpi(sSSIOth, sOpt2) == 0)
		{
			//Parse the file name.
			while(sSSI[iRPosA] != '"' && iRPosA < iSSISz)
			{
				if(iWPosA >= iOutBufMaxSz)
				{
					lpBuf->Append("<!--SSI ERROR(Secondary tag is too long)-->");
					break;
				}
				sSSIOth[iWPosA++] = sSSI[iRPosA++];
			}
			sSSIOth[iWPosA++] = '\0';

			if(sSSIOth[0] != '/' && pC->Header.FullRequest)
			{
				char sScriptPath[MAX_PATH];
				if(GetFilePath(pC->Header.FullRequest, sScriptPath, sizeof(sScriptPath)))
				{
					AppendPaths(sOutBuffer, iOutBufMaxSz, sScriptPath, sSSIOth);
				}
			}
			else {
				AppendPaths(sOutBuffer, iOutBufMaxSz, ((CWebSite *)pC->pWebSite)->Root, sSSIOth);
			}
		}
		else {
			lpBuf->Append("<!--SSI ERROR(Expected: \"");
			lpBuf->Append(sOpt1);
			lpBuf->Append("\" or \"");
			lpBuf->Append(sOpt2);
			lpBuf->Append("\")-->");
			return this->Locks.UnlockShared(false);
		}

		if(!FileAccess(sOutBuffer, FExist))
		{
			strcpy_s(sOutBuffer, iOutBufMaxSz, "");
			lpBuf->Append("<!--SSI ERROR(File not found)-->");
			return this->Locks.UnlockShared(false);
		}
	}

	return this->Locks.UnlockShared(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSSIFiles::GetVirtualOrFileName(VOID *pClient, StringBuilder *lpBuf, int iInitRPos,
						   const char *sSSI, const int iSSISz, char *sOutFileName)
{
	this->Locks.LockShared();
	PEER *pC = (PEER *)pClient;
	return this->Locks.UnlockShared(GetSSITagEx(pC, lpBuf, iInitRPos, sSSI, iSSISz, sOutFileName, MAX_PATH, "File", "Virtual"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSSIFiles::GetCGIOrCMDName(VOID *pClient, StringBuilder *lpBuf, int iInitRPos,
					  const char *sSSI, const int iSSISz, char *sOutFileName)
{
	this->Locks.LockShared();
	PEER *pC = (PEER *)pClient;
	return this->Locks.UnlockShared(GetSSITagEx(pC, lpBuf, iInitRPos, sSSI, iSSISz, sOutFileName, MAX_PATH, "CMD", "CGI"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSSIFiles::EchoSSIVar(VOID *pClient, StringBuilder *lpBuf, char *sVarName)
{
	this->Locks.LockShared();
	PEER *pC = (PEER *)pClient;
	CWebSite *pWebSite = (CWebSite *)pC->pWebSite;
	CHttp *pHttp = (CHttp *)pWebSite->pHttp;
	CWebSites *pWebSites = (CWebSites *)pWebSite->pWebSites;

	const char *sTrue = "True";
	const char *sFalse = "False";

	if(_strcmpi(sVarName, "DOCUMENT_NAME") == 0){
		lpBuf->Append(pC->Header.FullRequest);
	}
	else if(_strcmpi(sVarName, "DOCUMENT_URI") == 0){
		lpBuf->Append(pC->Header.Request);
		if(pC->Header.Query[0] != 0)
		{
			lpBuf->AppendF("?%s", pC->Header.Query);
		}
	}
	else if(_strcmpi(sVarName, "QUERY_STRING_UNESCAPED") == 0){
		StringBuilder queryString(pC->Header.Query);
		int iLength = URLDecode(queryString.Buffer);
		if(iLength >= 0)
		{
			lpBuf->Append(queryString.Buffer, iLength);
		}
		else{
			lpBuf->Append("<!--SSI ERROR(URLDecode Failed)-->");
		}
	}
	else if(_strcmpi(sVarName, "DATE_LOCAL") == 0){
		char sTime[255];
		GetLOCALTime(sTime, sizeof(sTime));
		lpBuf->Append(sTime);
	}
	else if(_strcmpi(sVarName, "DATE_GMT") == 0){
		char sGMTTime[255];
		MakeGMTTimeString(sGMTTime, sizeof(sGMTTime));
		lpBuf->Append(sGMTTime);
	}
	else if(_strcmpi(sVarName, "LAST_MODIFIED") == 0){
		char sTime[255];
		GetLastFileModificationDate(pC->Header.FullRequest, sTime, sizeof(sTime));
		lpBuf->Append(sTime);
	}
	else if(_strcmpi(sVarName, "SERVER_SOFTWARE") == 0){
		char sSoftware[255];
		sprintf_s(sSoftware, sizeof(sSoftware), "NetworkDLS - %s", gsTitleCaption);
		lpBuf->Append(sSoftware);
	}
	else if(_strcmpi(sVarName, "SERVER_VERSION") == 0){
		lpBuf->Append(gsFileVersion);
	}
	else if(_strcmpi(sVarName, "SERVER_NAME") == 0){
		char sHostName[255];
		if(Get_MachineName(sHostName, sizeof(sHostName)))
		{
			lpBuf->Append(sHostName);
		}
	}
	else if(_strcmpi(sVarName, "GATEWAY_INTERFACE") == 0){
		lpBuf->Append(CGIINTERFACE);
	}
	else if(_strcmpi(sVarName, "SERVER_PROTOCOL") == 0){
		lpBuf->Append(pC->Header.Version);
	}
	else if(_strcmpi(sVarName, "SERVER_PORT") == 0){
		lpBuf->Append(pWebSite->pSocketPool->pSocket->ListenPort());
	}
	else if(_strcmpi(sVarName, "REQUEST_METHOD") == 0){
		lpBuf->Append(pC->Header.Method);
	}
	else if(_strcmpi(sVarName, "PATH_INFO") == 0){
		lpBuf->Append(pC->Header.PathInfo);
	}
	else if(_strcmpi(sVarName, "PATH_TRANSLATED") == 0){
		lpBuf->Append(pC->Header.PathTranslated);
	}
	else if(_strcmpi(sVarName, "SCRIPT_NAME") == 0){
		lpBuf->Append(pC->Header.ScriptName);
	}
	else if(_strcmpi(sVarName, "QUERY_STRING") == 0){
		lpBuf->Append(pC->Header.Query);
	}
	else if(_strcmpi(sVarName, "REMOTE_HOST") == 0){
		lpBuf->Append(pC->pClient->PeerAddress());
	}
	else if(_strcmpi(sVarName, "REMOTE_ADDR") == 0){
		lpBuf->Append(pC->pClient->PeerAddress());
	}
	else if(_strcmpi(sVarName, "AUTH_TYPE") == 0){
		lpBuf->Append("<!--SSI ERROR(Not yet implemented)-->");
	}
	else if(_strcmpi(sVarName, "REMOTE_USER") == 0){
		lpBuf->Append("<!--SSI ERROR(Not yet implemented)-->");
	}
	else if(_strcmpi(sVarName, "REMOTE_IDENT") == 0){
		lpBuf->Append("<!--SSI ERROR(Not yet implemented)-->");
	}
	else if(_strcmpi(sVarName, "CONTENT_TYPE") == 0){
		lpBuf->Append("*.*");
	}
	else if(_strcmpi(sVarName, "CONTENT_LENGTH") == 0){
		lpBuf->Append("<!--SSI ERROR(Not yet implemented)-->");
	}
	else if(_strcmpi(sVarName, "HTTP_USER_AGENT") == 0){
		lpBuf->Append(pC->Header.UserAgent);
	}
	else if(_strcmpi(sVarName, "ADMIN_EMAIL") == 0){
		lpBuf->Append(((CWebSite *)pC->pWebSite)->pWebsiteSettings->ServerAdminEmail());
	}
	else if(_strcmpi(sVarName, "CONNECTION_ID") == 0){
		lpBuf->Append(pC->pClient->PeerID());
	}
	else if(_strcmpi(sVarName, "OS_VERSION") == 0){
	    char sTemp[255];
		Get_OsVersion(sTemp, sizeof(sTemp));
		lpBuf->Append(sTemp);
	}
	else if(_strcmpi(sVarName, "PROCESSOR_VENDOR") == 0){
		char sBuf[255];
		DWORD dwBufSz = sizeof(sBuf);
		memset(sBuf, 0, dwBufSz);
		if(Get_StringRegistryValue(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", "VendorIdentifier", sBuf, dwBufSz))
		{
			lpBuf->Append(sBuf);
		}
		else lpBuf->Append("n/a");
	}
	else if(_strcmpi(sVarName, "PROCESSOR_COUNT") == 0){
		SYSTEM_INFO SI;
		memset(&SI, 0, sizeof(SI));
	    GetSystemInfo(&SI);
		lpBuf->Append(SI.dwNumberOfProcessors);
	}
	else if(_strcmpi(sVarName, "PROCESSOR_NAME") == 0){
		char sBuf[255];
		DWORD dwBufSz = sizeof(sBuf);
		memset(sBuf, 0, dwBufSz);
		if(Get_StringRegistryValue(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", "ProcessorNameString", sBuf, dwBufSz))
		{
			dwBufSz--;

			ReverseString(sBuf, dwBufSz);

			while(IsWhiteSpace(sBuf[dwBufSz]) && dwBufSz > 0)
			{
				dwBufSz--;
			}

			if(!IsWhiteSpace(sBuf[dwBufSz]))
			{
				dwBufSz++;
			}

			sBuf[dwBufSz] = '\0';

			ReverseString(sBuf, dwBufSz);

			lpBuf->Append(sBuf);
		}
		else lpBuf->Append("n/a");
	}
	else if(_strcmpi(sVarName, "PROCESSOR_IDENTIFIER") == 0){
		char sBuf[255];
		DWORD dwBufSz = sizeof(sBuf);
		memset(sBuf, 0, dwBufSz);
		if(Get_StringRegistryValue(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", "Identifier", sBuf, dwBufSz))
		{
			lpBuf->Append(sBuf);
		}
		else lpBuf->Append("n/a");
	}
	else if(_strcmpi(sVarName, "PROCESSOR_MHZ") == 0){
		DWORD dwBuf = 0;
		if(Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", "~MHz", dwBuf))
		{
			lpBuf->Append(dwBuf);
		}
		else lpBuf->Append("n/a");
	}
	else if(_strcmpi(sVarName, "SERVER_ADMIN_EMAIL") == 0){
		lpBuf->Append(((CWebSite*)pC->pWebSite)->pWebsiteSettings->ServerAdminEmail());
	}
	else if(_strcmpi(sVarName, "SERVER_ALLOW_DIRECTORY_INDEXING") == 0){
		if(((CWebSite*)pC->pWebSite)->pWebsiteSettings->AllowDirectoryIndexing())
		{
			lpBuf->Append(sTrue);
		}
		else{
			lpBuf->Append(sFalse);
		}
	}
	else if(_strcmpi(sVarName, "SERVER_BYTES_RECV") == 0){
		lpBuf->Append(((CWebSite *)pC->pWebSite)->Stats.BytesRecv);
	}
	else if(_strcmpi(sVarName, "SERVER_BYTES_SENT") == 0){
		lpBuf->Append(((CWebSite *)pC->pWebSite)->Stats.BytesSent);
	}
	else if(_strcmpi(sVarName, "SERVER_ACCEPTED_CONNECTIONS") == 0){
		lpBuf->Append(((CWebSite *)pC->pWebSite)->Stats.AcceptedConnections);
	}
	else if(_strcmpi(sVarName, "SERVER_HITS") == 0){
		lpBuf->Append(((CWebSite *)pC->pWebSite)->Stats.Hits);
	}
	else if(_strcmpi(sVarName, "SERVER_CACHE_NATIVE_SIZE") == 0){
		lpBuf->Append(((CWebSite*)pC->pWebSite)->pCompression->GetNativeCacheSize());
	}
	else if(_strcmpi(sVarName, "SERVER_CACHE_COMPRESSED_SIZE") == 0){
		lpBuf->Append(((CWebSite*)pC->pWebSite)->pCompression->GetCompressedCacheSize());
	}
	else if(_strcmpi(sVarName, "SERVER_CACHE_COMPRESSION_RATIO") == 0){
		lpBuf->Append(((CWebSite*)pC->pWebSite)->pCompression->GetCacheCompressionRatio(), 2);
	}
	else if(_strcmpi(sVarName, "SERVER_CACHE_CURRENT_FILES") == 0){
		lpBuf->Append(((CWebSite*)pC->pWebSite)->pCompression->CachedFiles());
	}
	else if(_strcmpi(sVarName, "SERVER_CACHE_MAX_FILES") == 0){
		lpBuf->Append(((CWebSite*)pC->pWebSite)->pCompression->MaxCachedFiles());
	}
	else if(_strcmpi(sVarName, "SERVER_CACHE_PATH") == 0){
		lpBuf->Append(((CWebSite*)pC->pWebSite)->pCompression->CachePath());
	}
	else if(_strcmpi(sVarName, "SERVER_CGI_TIMEOUT") == 0){
		lpBuf->Append(((CWebSite*)pC->pWebSite)->pWebsiteSettings->ScriptTimeout());
	}
	else if(_strcmpi(sVarName, "SERVER_COMPRESS_DYNAMIC_CONTENT") == 0){
		if(((CWebSite*)pC->pWebSite)->pCompression->CompressDynamicContent())
		{
			lpBuf->Append(sTrue);
		}
		else{
			lpBuf->Append(sFalse);
		}
	}
	else if(_strcmpi(sVarName, "SERVER_CONNECTIONS") == 0){
		lpBuf->Append(pWebSites->pSocketPools->Pool(pC->pWebSite)->pSocket->CurrentConnections());
	}
	else if(_strcmpi(sVarName, "SERVER_GLOBAL_CONNECTIONS") == 0){
		lpBuf->Append(pWebSites->GetCurrentConenctions());
	}
	else if(_strcmpi(sVarName, "SERVER_WARNINGS") == 0){
		lpBuf->Append(pWebSite->pErrorPages->WarningCount());
	}
	else if(_strcmpi(sVarName, "SERVER_ERRORS") == 0){
		lpBuf->Append(pWebSite->pErrorPages->ErrorCount());
	}
	else if(_strcmpi(sVarName, "SERVER_KEEP_ALIVE_TIMEOUT") == 0){
		lpBuf->Append(((CWebSite*)pC->pWebSite)->pWebsiteSettings->KeepAliveTimeout());
	}
	else if(_strcmpi(sVarName, "SERVER_LOG_PATH") == 0){
		lpBuf->Append(((CWebSite*)pC->pWebSite)->pWebsiteSettings->LogPath());
	}
	else if(_strcmpi(sVarName, "SERVER_COMPRESS_MAX_SIZE") == 0){
		lpBuf->Append(((CWebSite*)pC->pWebSite)->pCompression->MaxCompressionSize());
	}
	else if(_strcmpi(sVarName, "SERVER_COMPRESS_MIN_SIZE") == 0){
		lpBuf->Append(((CWebSite*)pC->pWebSite)->pCompression->MinCompressionSize());
	}
	else if(_strcmpi(sVarName, "SERVER_PROCESS_EXTERNAL_CGI") == 0){
		if(((CWebSite*)pC->pWebSite)->pScriptingEngines->Enabled())
		{
			lpBuf->Append(sTrue);
		}
		else{
			lpBuf->Append(sFalse);
		}
	}
	else if(_strcmpi(sVarName, "SERVER_PROCESS_SSI_FILES") == 0){
		if(((CWebSite*)pC->pWebSite)->pSSIFiles->Enabled())
		{
			lpBuf->Append(sTrue);
		}
		else{
			lpBuf->Append(sFalse);
		}
	}
	else if(_strcmpi(sVarName, "SERVER_SCAN_EXTS") == 0){
		if(((CWebSite*)pC->pWebSite)->pExtensionFilters->Enabled())
		{
			lpBuf->Append(sTrue);
		}
		else{
			lpBuf->Append(sFalse);
		}	
	}
	else if(_strcmpi(sVarName, "SERVER_SCAN_IPS") == 0){
		if(((CWebSite*)pC->pWebSite)->pIPFilters->Enabled())
		{
			lpBuf->Append(sTrue);
		}
		else{
			lpBuf->Append(sFalse);
		}	
	}
	else if(_strcmpi(sVarName, "SERVER_SCAN_URLS") == 0){
		if(((CWebSite*)pC->pWebSite)->pURLFilters->Enabled())
		{
			lpBuf->Append(sTrue);
		}
		else{
			lpBuf->Append(sFalse);
		}	
	}
	else if(_strcmpi(sVarName, "SERVER_TOTAL_CONNECTIONS") == 0){
		lpBuf->Append(pWebSites->GetTotalConnections());
	}
	else if(_strcmpi(sVarName, "SERVER_USE_CGI_DIRECTORIES") == 0){
		if(((CWebSite*)pC->pWebSite)->pCGIFolders->Enabled())
		{
			lpBuf->Append(sTrue);
		}
		else{
			lpBuf->Append(sFalse);
		}	
	}
	else if(_strcmpi(sVarName, "SERVER_USE_HTTP_COMPRESSION") == 0){
		if(((CWebSite*)pC->pWebSite)->pCompression->Enabled())
		{
			lpBuf->Append(sTrue);
		}
		else{
			lpBuf->Append(sFalse);
		}	
	}
	else if(_strcmpi(sVarName, "SERVER_MEMORY_USED") == 0){
		PROCESS_MEMORY_COUNTERS pmc;
		memset(&pmc, 0, sizeof(pmc));

		if(GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
		{
			lpBuf->Append(pmc.WorkingSetSize);
		}
	}
	else if(_strcmpi(sVarName, "SERVER_MEMORY_PAGEFILEUSAGE") == 0){
		PROCESS_MEMORY_COUNTERS pmc;
		memset(&pmc, 0, sizeof(pmc));

		if(GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
		{
			lpBuf->Append(pmc.PagefileUsage);
		}
	}
	else if(_strcmpi(sVarName, "SERVER_MEMORY_PEAKMEMUSAGE") == 0){
		PROCESS_MEMORY_COUNTERS pmc;
		memset(&pmc, 0, sizeof(pmc));

		if(GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
		{
			lpBuf->Append(pmc.PeakWorkingSetSize);
		}
	}
	else if(_strcmpi(sVarName, "SERVER_MEMORY_PEAKPAGEFILEUSAGE") == 0){
		PROCESS_MEMORY_COUNTERS pmc;
		memset(&pmc, 0, sizeof(pmc));

		if(GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
		{
			lpBuf->Append(pmc.PeakPagefileUsage);
		}
	}
	else if(_strcmpi(sVarName, "SYSTEM_MEMORY_FREE") == 0){
		MEMORYSTATUSEX MS;
		memset(&MS, 0, sizeof(MS));
		MS.dwLength = sizeof(MS);
		GlobalMemoryStatusEx(&MS);

		lpBuf->Append(MS.ullAvailPhys);
	}
	else if(_strcmpi(sVarName, "SYSTEM_MEMORY_TOTAL") == 0){
		MEMORYSTATUSEX MS;
		memset(&MS, 0, sizeof(MS));
		MS.dwLength = sizeof(MS);
		GlobalMemoryStatusEx(&MS);

		lpBuf->Append(MS.ullTotalPhys);
	}
	else if(_strcmpi(sVarName, "SYSTEM_MEMORY_USED") == 0){
		MEMORYSTATUSEX MS;
		memset(&MS, 0, sizeof(MS));
		MS.dwLength = sizeof(MS);
		GlobalMemoryStatusEx(&MS);

		lpBuf->Append(MS.ullTotalPhys - MS.ullAvailPhys);
	}

	else{
		//Used for Custom Error Pages with Scripting (BEGIN)
		if(pC->Header.OriginalRequest)
		{
			if(_strcmpi(sVarName, "ORIGINAL_PATH_INFO") == 0)
			{
				lpBuf->Append(pC->Header.OriginalPathInfo);
			}
			else if(_strcmpi(sVarName, "ORIGINAL_SCRIPT_FILENAME") == 0)
			{
				lpBuf->Append(pC->Header.OriginalFullRequest);
			}
			else if(_strcmpi(sVarName, "ORIGINAL_REQUEST") == 0)
			{
				lpBuf->Append(pC->Header.OriginalRequest);
			}
			else if(_strcmpi(sVarName, "ORIGINAL_PATH_TRANSLATED") == 0)
			{
				lpBuf->Append(pC->Header.OriginalPathTranslated);
			}
			else if(_strcmpi(sVarName, "ORIGINAL_SCRIPT_NAME") == 0)
			{
				lpBuf->Append(pC->Header.OriginalScriptName);
			}
			else if(_strcmpi(sVarName, "ORIGINAL_MIME_TYPE") == 0)
			{
				if(pC->Header.OriginalScriptName)
				{
					char sOriginalMimeType[MIMETYPES_MAX_TYPE_LENGTH];
					if(((CWebSite *)pC->pWebSite)->pMimeTypes->GetType(pC->Header.OriginalScriptName, sOriginalMimeType, sizeof(sOriginalMimeType)))
					{
						lpBuf->Append(sOriginalMimeType);
					}
				}
			}
			else {
				lpBuf->Append("<!--SSI ERROR(Unknown echo variable)-->");
			}
		}
		//Used for Custom Error Pages with Scripting (END)
		else {
			lpBuf->Append("<!--SSI ERROR(Unknown echo variable)-->");
		}
	}

	return this->Locks.UnlockShared(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
