///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CWebSites_CPP
#define _CWebSites_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern HIMAGELIST hEnableDisableImageList; //Declared in MainDialog.cpp
extern HIMAGELIST hOnePixilImageList; //Declared in MainDialog.cpp

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"

#include "CWebSites.H"

#include "SocketHandlers.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Conversion;
using namespace NSWFL::File;
using namespace NSWFL::DateTime;
using namespace NSWFL::ListView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CWebSites *gWebSites;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CWebSites::~CWebSites()
{
	this->Destroy();

	DeleteCriticalSection(&this->InternalTrace.CS);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CWebSites::CWebSites()
{
	gWebSites = this;

	memset(&this->InternalTrace, 0, sizeof(this->InternalTrace));
	memset(&this->Collection, 0, sizeof(this->Collection));
	memset(&this->sFileName, 0, sizeof(this->sFileName));

	this->dwTotalConnections = 0;

	this->pServerSettings = NULL;
	this->pHttp = NULL;
	this->pSocketPools = NULL;

	this->DfltWebsiteSettings = NULL;
	this->DfltMimeTypes = NULL;
	this->DfltExtensionFilters = NULL;
	this->DfltURLFilters = NULL;
	this->DfltDefaultPages = NULL;
	this->DfltVirtualRoots = NULL;
	this->DfltErrorPages = NULL;
	this->DfltSSIFiles = NULL;
	this->DfltIPFilters = NULL;
	this->DfltCGIFolders = NULL;
	this->DfltCompression = NULL;
	this->DfltCustomFilters = NULL;
	this->DfltScriptingEngines = NULL;
	this->DfltAuthentication = NULL;
	this->DfltRealms = NULL;

	memset(&this->InternalTrace.CS, 0, sizeof(this->InternalTrace.CS));
	InitializeCriticalSection(&this->InternalTrace.CS);

	//Trace is "on" to begin with.
	//-------------------------------------------------------------------
	this->StartTrace(); //Must stop the trace after construction.
	//-------------------------------------------------------------------

	this->Initialize();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Should attempt to save ALL server configuration information (sites and server, local and global).
*/
bool CWebSites::SaveAll(void)
{
	bool bResult = true;

	bResult = bResult && this->DfltWebsiteSettings->Save();
	bResult = bResult && this->DfltMimeTypes->Save();
	bResult = bResult && this->DfltExtensionFilters->Save();
	bResult = bResult && this->DfltURLFilters->Save();
	bResult = bResult && this->DfltDefaultPages->Save();
	bResult = bResult && this->DfltVirtualRoots->Save();
	bResult = bResult && this->DfltErrorPages->Save();
	bResult = bResult && this->DfltSSIFiles->Save();
	bResult = bResult && this->DfltIPFilters->Save();
	bResult = bResult && this->DfltCGIFolders->Save();
	bResult = bResult && this->DfltCompression->Save();
	bResult = bResult && this->DfltCustomFilters->Save();
	bResult = bResult && this->DfltScriptingEngines->Save();
	//bResult = bResult && this->DfltAuthentication->Save();
	//bResult = bResult && this->DfltRealms->Save();
	bResult = bResult && this->Save(false);

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWebSites::Save(bool bOnlySaveReferencedSites)
{
	XMLWriter xmlConfig("WebSites");

	if(this->pServerSettings->Save())
	{
		for(int iItem = 0; iItem < this->Collection.Count; iItem++)
		{
			XMLWriter Item;

			if(bOnlySaveReferencedSites)
			{
				if(this->Collection.WebSite[iItem]->bReferencedInUpdate)
				{
					this->Collection.WebSite[iItem]->ToXML(&Item, true, false);
					xmlConfig.Add(&Item);
				}
			}
			else {
				this->Collection.WebSite[iItem]->ToXML(&Item, true, false);
				xmlConfig.Add(&Item);
			}

			Item.Destroy();
		}

		if(strlen(this->sFileName) > 0)
		{
			if(xmlConfig.ToFile(this->sFileName))
			{
				return true;
			}

			char sErrorMsg[2048];
			GetLastError(sErrorMsg, sizeof(sErrorMsg),
				"Failed to save websites configuration file (", ")");
			strcat_s(sErrorMsg, sizeof(sErrorMsg), "\r\n\r\n\"");
			strcat_s(sErrorMsg, sizeof(sErrorMsg), this->sFileName);
			strcat_s(sErrorMsg, sizeof(sErrorMsg), "\"");
			this->Trace(sErrorMsg);
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWebSites::LoadSites(char *sFile)
{
	XMLReader xmlWebSites;
	strcpy_s(this->sFileName, sizeof(this->sFileName), sFile);

	if(xmlWebSites.FromFile(this->sFileName))
	{
		return this->LoadSites(&xmlWebSites);
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWebSites::LoadSites(XMLReader *pWebSites)
{
	pWebSites->ProgressiveScan(true);
	XMLReader xmlSite;

	while(pWebSites->ToReader("Site", &xmlSite))
	{
		this->Add(&xmlSite);
		xmlSite.Destroy();
	}
	
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWebSites::ReloadSites(XMLReader *pWebSites)
{
	this->DestroySites();
	return this->LoadSites(pWebSites);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Reloads all server settings and websites.
*/
bool CWebSites::Reload(void)
{
	if(this->Destroy())
	{
		return this->Initialize();
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Reloads all server websites.
*/
bool CWebSites::ReloadSites(void)
{
	this->DestroySites();
	return this->LoadSites(this->sFileName);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CWebSites::Count(void)
{
	return this->Collection.Count;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CWebSite *CWebSites::Site(int iIndex)
{
	return this->Collection.WebSite[iIndex];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWebSites::Initialize(void)
{
	char sMissingFiles[2048];
	char sConfigFile[MAX_PATH];
	const char *sErrorMsg = "Could not load \"%s\"";

	memset(&this->Collection, 0, sizeof(this->Collection));
	this->dwTotalConnections = 0;

	this->pServerSettings = (CServerSettings *) pMem->TrackMemory(new CServerSettings(this));
	this->pSocketPools = (CSocketPools *) pMem->TrackMemory(new CSocketPools(this));

	this->DfltWebsiteSettings = (CWebsiteSettings *) pMem->TrackMemory(new CWebsiteSettings(this));
	this->DfltMimeTypes = (CMimeTypes *) pMem->TrackMemory(new CMimeTypes(this));
	this->DfltExtensionFilters = (CExtensionFilters *) pMem->TrackMemory(new CExtensionFilters(this));
	this->DfltURLFilters = (CURLFilters *) pMem->TrackMemory(new CURLFilters(this));
	this->DfltDefaultPages = (CDefaultPages *) pMem->TrackMemory(new CDefaultPages(this));
	this->DfltVirtualRoots = (CVirtualRoots *) pMem->TrackMemory(new CVirtualRoots(this));
	this->DfltErrorPages = (CErrorPages *) pMem->TrackMemory(new CErrorPages(this));
	this->DfltSSIFiles = (CSSIFiles *) pMem->TrackMemory(new CSSIFiles(this));
	this->DfltIPFilters = (CIPFilters *) pMem->TrackMemory(new CIPFilters(this));
	this->DfltCGIFolders = (CCGIFolders *) pMem->TrackMemory(new CCGIFolders(this));
	this->DfltCompression = (CCompression *) pMem->TrackMemory(new CCompression(this));
	this->DfltCustomFilters = (CCustomFilters *) pMem->TrackMemory(new CCustomFilters(this));
	this->DfltScriptingEngines = (CScriptingEngines *) pMem->TrackMemory(new CScriptingEngines(this));
	this->DfltAuthentication = (CAuthentication *) pMem->TrackMemory(new CAuthentication(this));
	this->DfltRealms = (CRealms *) pMem->TrackMemory(new CRealms(this));

	sprintf_s(sConfigFile, sizeof(sConfigFile), "%s\\SocketPools.xml", gsDataPath);
	if(!this->pSocketPools->Load(sConfigFile))
	{
		sprintf_s(sMissingFiles, sizeof(sMissingFiles), sErrorMsg, sConfigFile);
		this->Trace(sMissingFiles);
		return false;
	}

	sprintf_s(sConfigFile, sizeof(sConfigFile), "%s\\WebsiteSettings.xml", gsDataPath);
	if(!this->DfltWebsiteSettings->Load(sConfigFile))
	{
		sprintf_s(sMissingFiles, sizeof(sMissingFiles), sErrorMsg, sConfigFile);
		this->Trace(sMissingFiles);
		return false;
	}
	sprintf_s(sConfigFile, sizeof(sConfigFile), "%s\\MimeTypes.xml", gsDataPath);
	if(!this->DfltMimeTypes->Load(sConfigFile))
	{
		sprintf_s(sMissingFiles, sizeof(sMissingFiles), sErrorMsg, sConfigFile);
		this->Trace(sMissingFiles);
		return false;
	}
	sprintf_s(sConfigFile, sizeof(sConfigFile), "%s\\URLFilters.xml", gsDataPath);
	if(!this->DfltURLFilters->Load(sConfigFile))
	{
		sprintf_s(sMissingFiles, sizeof(sMissingFiles), sErrorMsg, sConfigFile);
		this->Trace(sMissingFiles);
		return false;
	}
	sprintf_s(sConfigFile, sizeof(sConfigFile), "%s\\ExtensionFilters.xml", gsDataPath);
	if(!this->DfltExtensionFilters->Load(sConfigFile))
	{
		sprintf_s(sMissingFiles, sizeof(sMissingFiles), sErrorMsg, sConfigFile);
		this->Trace(sMissingFiles);
		return false;
	}
	sprintf_s(sConfigFile, sizeof(sConfigFile), "%s\\DefaultPages.xml", gsDataPath);
	if(!this->DfltDefaultPages->Load(sConfigFile))
	{
		sprintf_s(sMissingFiles, sizeof(sMissingFiles), sErrorMsg, sConfigFile);
		this->Trace(sMissingFiles);
		return false;
	}
	sprintf_s(sConfigFile, sizeof(sConfigFile), "%s\\VirtualRoots.xml", gsDataPath);
	if(!this->DfltVirtualRoots->Load(sConfigFile))
	{
		sprintf_s(sMissingFiles, sizeof(sMissingFiles), sErrorMsg, sConfigFile);
		this->Trace(sMissingFiles);
		return false;
	}
	sprintf_s(sConfigFile, sizeof(sConfigFile), "%s\\SSIFiles.xml", gsDataPath);
	if(!this->DfltSSIFiles->Load(sConfigFile))
	{
		sprintf_s(sMissingFiles, sizeof(sMissingFiles), sErrorMsg, sConfigFile);
		this->Trace(sMissingFiles);
		return false;
	}
	sprintf_s(sConfigFile, sizeof(sConfigFile), "%s\\IPFilters.xml", gsDataPath);
	if(!this->DfltIPFilters->Load(sConfigFile))
	{
		sprintf_s(sMissingFiles, sizeof(sMissingFiles), sErrorMsg, sConfigFile);
		this->Trace(sMissingFiles);
		return false;
	}
	sprintf_s(sConfigFile, sizeof(sConfigFile), "%s\\CGIFolders.xml", gsDataPath);
	if(!this->DfltCGIFolders->Load(sConfigFile))
	{
		sprintf_s(sMissingFiles, sizeof(sMissingFiles), sErrorMsg, sConfigFile);
		this->Trace(sMissingFiles);
		return false;
	}
	sprintf_s(sConfigFile, sizeof(sConfigFile), "%s\\Compression.xml", gsDataPath);
	if(!this->DfltCompression->Load(sConfigFile))
	{
		sprintf_s(sMissingFiles, sizeof(sMissingFiles), sErrorMsg, sConfigFile);
		this->Trace(sMissingFiles);
		return false;
	}
	sprintf_s(sConfigFile, sizeof(sConfigFile), "%s\\CustomFilters.xml", gsDataPath);
	if(!this->DfltCustomFilters->Load(sConfigFile))
	{
		sprintf_s(sMissingFiles, sizeof(sMissingFiles), sErrorMsg, sConfigFile);
		this->Trace(sMissingFiles);
		return false;
	}
	sprintf_s(sConfigFile, sizeof(sConfigFile), "%s\\ScriptingEngines.xml", gsDataPath);
	if(!this->DfltScriptingEngines->Load(sConfigFile))
	{
		sprintf_s(sMissingFiles, sizeof(sMissingFiles), sErrorMsg, sConfigFile);
		this->Trace(sMissingFiles);
		return false;
	}
	sprintf_s(sConfigFile, sizeof(sConfigFile), "%s\\ErrorPages.xml", gsDataPath);
	if(!this->DfltErrorPages->Load(sConfigFile))
	{
		sprintf_s(sMissingFiles, sizeof(sMissingFiles), sErrorMsg, sConfigFile);
		this->Trace(sMissingFiles);
		return false;
	}
	sprintf_s(sConfigFile, sizeof(sConfigFile), "%s\\ServerSettings.xml", gsDataPath);
	if(!this->pServerSettings->Load(sConfigFile))
	{
		sprintf_s(sMissingFiles, sizeof(sMissingFiles), sErrorMsg, sConfigFile);
		this->Trace(sMissingFiles);
		return false;
	}
	sprintf_s(sConfigFile, sizeof(sConfigFile), "%s\\WebSites.xml", gsDataPath);
	if(!this->LoadSites(sConfigFile))
	{
		sprintf_s(sMissingFiles, sizeof(sMissingFiles), sErrorMsg, sConfigFile);
		this->Trace(sMissingFiles);
		return false;
	}

	pHttp = (CHttp *) pMem->TrackMemory(new CHttp(this));

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CWebSites::GetCurrentConenctions(void)
{
	int iConnections = 0;

	for(int i = 0; i < this->pSocketPools->Collection.Count ; i++)
	{
		if(this->pSocketPools->Collection.Items[i].pSocket->IsActive())
		{
			iConnections += this->pSocketPools->Collection.Items[i].pSocket->CurrentConnections();
		}
	}

	return iConnections;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CWebSites::IncrementConnectionCount(void)
{
	this->dwTotalConnections++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CWebSites::GetTotalConnections(void)
{
	return this->dwTotalConnections;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CWebSites::DestroySites(void)
{
	this->StopAllSites();

	if(this->Collection.Count > 0)
	{
		for(int iIndex = 0; iIndex < this->Collection.Count; iIndex++)
		{
			delete (CWebSite *)pMem->UnTrackMemory(this->Collection.WebSite[iIndex]);
		}
		pMem->Free(this->Collection.WebSite);
	}

	this->Collection.Count = 0;
	this->Collection.WebSite = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWebSites::Destroy(void)
{
	this->DestroySites();

	if(this->DfltMimeTypes) delete (CMimeTypes *)pMem->UnTrackMemory(this->DfltMimeTypes);
	if(this->DfltURLFilters) delete (CURLFilters *)pMem->UnTrackMemory(this->DfltURLFilters);
	if(this->DfltExtensionFilters) delete (CExtensionFilters *)pMem->UnTrackMemory(this->DfltExtensionFilters);
	if(this->DfltDefaultPages) delete (CDefaultPages *)pMem->UnTrackMemory(this->DfltDefaultPages);
	if(this->DfltWebsiteSettings) delete (CWebsiteSettings *)pMem->UnTrackMemory(this->DfltWebsiteSettings);
	if(this->DfltVirtualRoots) delete (CVirtualRoots *)pMem->UnTrackMemory(this->DfltVirtualRoots);
	if(this->DfltSSIFiles) delete (CSSIFiles *)pMem->UnTrackMemory(this->DfltSSIFiles);
	if(this->DfltIPFilters) delete (CIPFilters *)pMem->UnTrackMemory(this->DfltIPFilters);
	if(this->DfltCGIFolders) delete (CCGIFolders *)pMem->UnTrackMemory(this->DfltCGIFolders);
	if(this->DfltCompression) delete (CCompression *)pMem->UnTrackMemory(this->DfltCompression);
	if(this->DfltCustomFilters) delete (CCustomFilters *)pMem->UnTrackMemory(this->DfltCustomFilters);
	if(this->DfltScriptingEngines) delete (CScriptingEngines *)pMem->UnTrackMemory(this->DfltScriptingEngines);
	if(this->DfltErrorPages) delete (CErrorPages *)pMem->UnTrackMemory(this->DfltErrorPages);
	if(this->DfltAuthentication) delete (CAuthentication *)pMem->UnTrackMemory(this->DfltAuthentication);
	if(this->DfltRealms) delete (CRealms *)pMem->UnTrackMemory(this->DfltRealms);
	if(this->pServerSettings) delete (CServerSettings *)pMem->UnTrackMemory(this->pServerSettings);
	if(this->pSocketPools) delete (CSocketPools *)pMem->UnTrackMemory(this->pSocketPools);
	if(this->pHttp) delete (CHttp *)pMem->UnTrackMemory(this->pHttp);

	this->StopTrace();

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWebSites::Add(XMLReader *xmlSite, int *iOutIndex)
{
	this->Collection.WebSite = (CWebSite **)
		pMem->ReAllocate(this->Collection.WebSite, sizeof(CWebSite), (this->Collection.Count + 1));

	if(iOutIndex)
	{
		*iOutIndex = this->Collection.Count;
	}

	this->Collection.WebSite[this->Collection.Count++] = (CWebSite *) pMem->TrackMemory(new CWebSite(this, xmlSite));

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWebSites::Add(XMLReader *xmlSite)
{
	return this->Add(xmlSite, NULL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CWebSites::StartTrace(void)
{
	EnterCriticalSection(&this->InternalTrace.CS);
	if(InternalTrace.bTracingEnabled)
	{
		InternalTrace.Stack->Clear();
	}
	else{
		InternalTrace.Stack = (Stack *) pMem->TrackMemory(new Stack());
		InternalTrace.bTracingEnabled = true;
	}
	LeaveCriticalSection(&this->InternalTrace.CS);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CWebSites::Trace(const char *sText)
{
	EnterCriticalSection(&this->InternalTrace.CS);
	if(InternalTrace.bTracingEnabled)
	{
		char sDate[255];
		char sTime[255];

		if(Get_ShortDate(sDate, sizeof(sDate)) && Get_Time(sTime, sizeof(sTime), 0))
		{
			int iAllocate = (int)(strlen(sText) + strlen(sDate) + strlen(sTime) + 10);
			char *sMessage = (char *) pMem->Allocate(iAllocate, sizeof(char));
			if(sMessage)
			{
				sprintf_s(sMessage, iAllocate, "(%s %s) - %s", sDate, sTime, sText);
				this->InternalTrace.Stack->Push(sMessage);
				pMem->Free(sMessage);
			}
			else{
				//Low on memory or the heap is corrupt, either way, I have money on things going badly!
				this->InternalTrace.Stack->Push(sText);
			}
		}
	}
	LeaveCriticalSection(&this->InternalTrace.CS);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CWebSites::StopTrace(void)
{
	return this->StopTrace(NULL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CWebSites::StopTrace(StringBuilder *pOutText)
{
	StringBuilder sLastResort;

	int iErrorCount = 0;
	EnterCriticalSection(&this->InternalTrace.CS);
	if(InternalTrace.bTracingEnabled)
	{
		if((iErrorCount = (int)this->InternalTrace.Stack->StackSize()) > 0)
		{
			char sDate[255];
			if(Get_ShortDate(sDate, sizeof(sDate)))
			{
				char sServicePath[MAX_PATH];
				sprintf_s(sServicePath, sizeof(sServicePath), "ErrorLog\\%s", gsServiceName);
	
				char sPath[MAX_PATH];
				char sFileName[MAX_PATH];
				_makepath_s(sPath, sizeof(sPath), NULL, gsPath, sServicePath, NULL);
				CorrectReversePath(sPath, sizeof(sPath), true);
				CreateFolderStructure(sPath);

				_makepath_s(sFileName, sizeof(sFileName), NULL, sPath, sDate, ".txt");
				CorrectReversePath(sFileName, sizeof(sFileName), true);

				FILE *hLogFile = NULL;
				if(fopen_s(&hLogFile, sFileName, "ab") != 0)
				{
					hLogFile = NULL;
				}

					for(unsigned int iExp = 0; iExp < this->InternalTrace.Stack->StackSize(); iExp++)
					{
						if(pOutText)
						{
							if(pOutText->Length > 0)
							{
								pOutText->Append("\r\n");
							}
							pOutText->Append((char *)this->InternalTrace.Stack->Peek(iExp)->Value);
						}
						if(hLogFile)
						{
							fprintf_s(hLogFile, "%s\r\n", (char *)this->InternalTrace.Stack->Peek(iExp)->Value);
						}
						if(!pOutText && !hLogFile)
						{
							if(sLastResort.Length == 0)
							{
								sLastResort.AppendF("This message is being raised by %s version %s.\r\n"
									"This message is not normal and reflects a case where the service"
									" was unable to write this information to a log file or prompt the"
									" user in any other manner. The messages are as follows:\r\n\r\n",
									gsTitleCaption, gsFileVersion);
							}
							sLastResort.AppendF("%s\r\n", (char *)this->InternalTrace.Stack->Peek(iExp)->Value);
						}
					}

				if(hLogFile)
				{
					fclose(hLogFile);
				}
			}
		}
		InternalTrace.bTracingEnabled = false;
		delete (Stack *)pMem->UnTrackMemory(this->InternalTrace.Stack);
	}
	LeaveCriticalSection(&this->InternalTrace.CS);

	if(sLastResort.Length > 0)
	{
		MessageBox(NULL, sLastResort.Buffer, gsTitleCaption, MB_ICONERROR | MB_TASKMODAL);
	}

	return iErrorCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CWebSites::StopAllSites(void)
{
	for(int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		if(this->Collection.WebSite[iItem]->IsActive())
		{
			this->Collection.WebSite[iItem]->Stop();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWebSites::StartAllSites(void)
{
	bool bResult = true;

	for(int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		if(!this->Collection.WebSite[iItem]->IsActive() && this->Collection.WebSite[iItem]->pWebsiteSettings->AutoStart())
		{
			bResult = bResult && this->Collection.WebSite[iItem]->Start();
		}
	}
	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CWebSites::DeReferenceAllSites(void)
{
	for(int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		this->Collection.WebSite[iItem]->bReferencedInUpdate = false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CWebSites::BindSocketPools(void)
{
	for(int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		this->Collection.WebSite[iItem]->BindSocketPool();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Stack *CWebSites::RunningSites(void)
{
	Stack *runningSites = (Stack *) pMem->TrackMemory(new Stack());

	for(int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		runningSites->Push(this->Collection.WebSite[iItem]->Index());
	}

	return runningSites;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
