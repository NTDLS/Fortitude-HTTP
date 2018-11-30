///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _Management_Cpp
#define _Management_Cpp
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <IO.H>
#include <fcntl.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"
#include "Routines.H"
#include "CManagement.H"

#include "CWebSites.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::KeyGeneration;
using namespace NSWFL::XML;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CManagement::CManagement()
{
	this->bIsAuthorized = false;
	this->bInitialized = false;

	this->Socket = (CSocketServer *) pMem->TrackMemory(new CSocketServer());

	this->Cytpto.Initialize(MGMT_INITIAL_KEY, MGMT_INITIAL_KEY_SZ, true);
	
	if(this->Socket->Initialize(gdwManagementPort, 2, Mgmt_Sock_ClientThread, Mgmt_Sock_OnSockError))
	{
		this->Socket->pUserData = this;
		this->Socket->SetOnBeginClientThread(MgMt_OnBeginClientThread);
		this->Socket->SetOnEndClientThread(MgMt_OnEndClientThread);
		this->Socket->SetOnAfterDePacketize(MgMt_OnAfterDePacketize);
		this->Socket->SetOnBeforePacketize(MgMt_OnBeforePacketize);
		this->Socket->SetOnAccept(MgMt_OnAccept);

		this->bInitialized = this->Socket->Start();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CManagement::~CManagement()
{
	if(this->Socket)
	{
		if(this->Socket->IsActive())
		{
			this->Socket->Stop(true);
		}
		this->Socket->Destroy();
		delete (CSocketServer *)pMem->UnTrackMemory(this->Socket);
	}

	this->Cytpto.Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CManagement::ProcessUrgentCommand(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	BASICHUNK CmdData;

	if(!this->bIsAuthorized)
	{
		if(pSock->CompareCommandAndBreakData(pChunk, "::Login|", &CmdData))
		{
			XMLReader xml((char *)CmdData.pBuffer, CmdData.iLength);
			pSock->Free(&CmdData);

			bool bResult = false;
			char sUsername[512];
			char sPassword[512];
			char sVersion[16];

			xml.ToString("Username", sUsername, sizeof(sUsername));
			xml.ToString("Password", sPassword, sizeof(sPassword));
			xml.ToString("Version", sVersion, sizeof(sVersion));

			if(strcmp(sVersion, gsFileVersion) != 0)
			{
				char sText[1024];
				sprintf_s(sText, sizeof(sText), "This server must be managed with a Management"
					" Console version %s but you are running version %s.", gsFileVersion, sVersion);

				SendCommand(pClient, sText);
				return true;
			}
		
			this->bIsAuthorized = gWebSites->pServerSettings->CheckLogin(sUsername, sPassword);

			SendCommand(pClient, bIsAuthorized ? "Yes" : "No");
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CManagement::ProcessCommand(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	BASICHUNK CmdData;
	int iCmdLen = 0;

	if(!this->bIsAuthorized)
	{
		return false;
	}

	gWebSites->StartTrace();

	//---------------------------------------------------------------------------------------------------------------------
	if(pSock->CompareCommandAndBreakData(pChunk, "::Information:Server:Performance|", &CmdData))
	{
		XMLWriter xml("Performance");
		for(int iSite = 0; iSite < gWebSites->Count(); iSite++)
		{
			XMLWriter site("Site");
			CWebSite *pSite = gWebSites->Site(iSite);
			site.Add("Index", pSite->Index());
			site.Add("Name", pSite->Name);
			site.Add("BytesSent", pSite->Stats.BytesSent);
			site.Add("BytesReceived", pSite->Stats.BytesRecv);
			site.Add("Hits", pSite->Stats.Hits);
						
			SOCKETPOOL *pSocketPool = gWebSites->pSocketPools->Pool(pSite);
			if(pSocketPool)
			{
				site.Add("CurrentConnections", pSite->CurrentConnections());
				site.Add("MaxConnections", pSocketPool->MaxConnections);
			}
			site.Add("IsActive", pSite->IsActive());
			xml.Add(&site);
		}
		SendCommand(pClient, &xml);

		pSock->Free(&CmdData);
	} //::Information:Server:Performance|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::RestartWebSites|", &CmdData))
	{
		int iSiteIndex = atoi((char *)CmdData.pBuffer);
		pSock->Free(&CmdData);

		StringBuilder errMsg;
		bool bResult = gWebSites->ReloadSites();
		bResult = gWebSites->StopTrace(&errMsg) == 0 && bResult;
		gWebSites->StartTrace();

		XMLWriter resultXML("ResultInformation");
		resultXML.Add("Result", bResult);
		resultXML.Add("Text", errMsg.Buffer);
		SendCommand(pClient, &resultXML);

	} //::RestartSites|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::RestartServer|", &CmdData))
	{
		int iSiteIndex = atoi((char *)CmdData.pBuffer);
		pSock->Free(&CmdData);

		StringBuilder errMsg;
		bool bResult = gWebSites->Reload();
		bResult = gWebSites->StopTrace(&errMsg) == 0 && bResult;
		gWebSites->StartTrace();

		XMLWriter resultXML("ResultInformation");
		resultXML.Add("Result", bResult);
		resultXML.Add("Text", errMsg.Buffer);
		SendCommand(pClient, &resultXML);

	} //::RestartServer|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::PersistConfiguration|", &CmdData))
	{
		int iSiteIndex = atoi((char *)CmdData.pBuffer);
		pSock->Free(&CmdData);

		StringBuilder errMsg;
		bool bResult = gWebSites->SaveAll();
		bResult = gWebSites->StopTrace(&errMsg) == 0 && bResult;
		gWebSites->StartTrace();

		XMLWriter resultXML("ResultInformation");
		resultXML.Add("Result", bResult);
		resultXML.Add("Text", errMsg.Buffer);
		SendCommand(pClient, &resultXML);

	} //::FlushConfiguration|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Start|", &CmdData))
	{
		int iSiteIndex = atoi((char *)CmdData.pBuffer);
		CWebSite *pSite = gWebSites->Site(iSiteIndex);

		StringBuilder errMsg;
		bool bResult = false;

		if(pSite)
		{
			bResult = pSite->Start();
			bResult = gWebSites->StopTrace(&errMsg) == 0 && bResult;
			gWebSites->StartTrace();
		}
		else {
			errMsg.Append("The site does not exist on the server.");
		}

		XMLWriter resultXML("ResultInformation");
		resultXML.Add("Result", bResult);
		resultXML.Add("Text", errMsg.Buffer);
		SendCommand(pClient, &resultXML);

		pSock->Free(&CmdData);
	} //::Start|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Stop|", &CmdData))
	{
		int iSiteIndex = atoi((char *)CmdData.pBuffer);
		CWebSite *pSite = gWebSites->Site(iSiteIndex);

		StringBuilder errMsg;
		bool bResult = false;

		if(pSite)
		{
			bResult = pSite->Stop();
			bResult = gWebSites->StopTrace(&errMsg) == 0 && bResult;
			gWebSites->StartTrace();
		}
		else {
			errMsg.Append("The site does not exist on the server.");
		}

		XMLWriter resultXML("ResultInformation");
		resultXML.Add("Result", bResult);
		resultXML.Add("Text", errMsg.Buffer);
		SendCommand(pClient, &resultXML);

		pSock->Free(&CmdData);
	} //::Stop|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::RestartWebSite|", &CmdData))
	{
		int iSiteIndex = atoi((char *)CmdData.pBuffer);
		CWebSite *pSite = gWebSites->Site(iSiteIndex);

		StringBuilder errMsg;
		bool bResult = false;

		if(pSite)
		{
			if((bResult = pSite->Stop()))
			{
				bResult = pSite->Start();
			}
			bResult = gWebSites->StopTrace(&errMsg) == 0 && bResult;
			gWebSites->StartTrace();
		}
		else {
			errMsg.Append("The site does not exist on the server.");
		}

		XMLWriter resultXML("ResultInformation");
		resultXML.Add("Result", bResult);
		resultXML.Add("Text", errMsg.Buffer);
		SendCommand(pClient, &resultXML);

		pSock->Free(&CmdData);
	} //::Restart|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::IsActive|", &CmdData))
	{
		int iSiteIndex = atoi((char *)CmdData.pBuffer);
		CWebSite *pSite = gWebSites->Site(iSiteIndex);
		if(pSite)
		{
			SendCommand(pClient, pSite->IsActive() ? "Yes" : "No");
		}
		else {
			SendCommand(pClient, "No");
		}
		pSock->Free(&CmdData);
	} //::IsActive|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Configuration:SocketPoolsOverview|", &CmdData))
	{
		XMLReader xml;
		if(gWebSites->pSocketPools->ToXML(&xml, true))
		{
			SendCommand(pClient, &xml);
		}
		pSock->Free(&CmdData);
	} //::Configuration:SocketPoolsOverview|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Configuration:SocketPools|", &CmdData))
	{
		XMLReader xml;
		if(gWebSites->pSocketPools->ToXML(&xml))
		{
			SendCommand(pClient, &xml);
		}
		pSock->Free(&CmdData);
	} //::Configuration:SocketPools|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Configuration:WebSites|", &CmdData))
	{
		XMLWriter xml("WebSites");
		for(int iSite = 0; iSite < gWebSites->Count(); iSite++)
		{
			XMLWriter xmlSite;
			if(gWebSites->Site(iSite)->ToXML(&xmlSite, false, true))
			{
				xml.Add(&xmlSite);
			}
		}
		SendCommand(pClient, &xml);

		pSock->Free(&CmdData);
	} //::Configuration:WebSites|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Configuration:WebSiteOverview|", &CmdData))
	{
		int iSiteIndex = atoi((char *)CmdData.pBuffer);
		CWebSite *pSite = gWebSites->Site(iSiteIndex);
		if(pSite)
		{
			XMLWriter xml;
			if(pSite->Overview(&xml))
			{
				SendCommand(pClient, &xml);
			}
		}
		pSock->Free(&CmdData);
	} //::Configuration:WebSiteOverview|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Configuration:WebSiteBindings|", &CmdData))
	{
		int iSiteIndex = atoi((char *)CmdData.pBuffer);
		CWebSite *pSite = gWebSites->Site(iSiteIndex);
		if(pSite)
		{
			XMLWriter xml;
			if(pSite->Bindings(&xml))
			{
				SendCommand(pClient, &xml);
			}
		}
		pSock->Free(&CmdData);
	} //::Configuration:WebSiteOverview|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Configuration:WebSite|", &CmdData))
	{
		int iSiteIndex = atoi((char *)CmdData.pBuffer);
		CWebSite *pSite = gWebSites->Site(iSiteIndex);
		if(pSite)
		{
			XMLWriter xml;
			if(pSite->ToXML(&xml))
			{
				SendCommand(pClient, &xml);
			}
		}
		pSock->Free(&CmdData);
	} //::Configuration:WebSite|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Configuration:WebsiteSettings|", &CmdData))
	{
		int iSiteIndex = atoi((char *)CmdData.pBuffer);
		if(iSiteIndex == INVALID_SITE_INDEX)
		{
			XMLReader xml;
			if(gWebSites->DfltWebsiteSettings->ToXML(&xml))
			{
				SendCommand(pClient, &xml);
			}
		}
		else {
			CWebSite *pSite = gWebSites->Site(iSiteIndex);
			if(pSite)
			{
				XMLReader xml;
				if(pSite->pWebsiteSettings->ToXML(&xml))
				{
					SendCommand(pClient, &xml);
				}
			}
		}
		pSock->Free(&CmdData);
	} //::Configuration:WebsiteSettings|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Configuration:ServerSettings|", &CmdData))
	{
		XMLReader xml;
		if(gWebSites->pServerSettings->ToXML(&xml))
		{
			SendCommand(pClient, &xml);
		}
		pSock->Free(&CmdData);
	} //::Configuration:ServerSettings|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Configuration:DefaultPages|", &CmdData))
	{
		int iSiteIndex = atoi((char *)CmdData.pBuffer);
		if(iSiteIndex == INVALID_SITE_INDEX)
		{
			XMLReader xml;
			if(gWebSites->DfltDefaultPages->ToXML(&xml))
			{
				SendCommand(pClient, &xml);
			}
		}
		else {
			CWebSite *pSite = gWebSites->Site(iSiteIndex);
			if(pSite)
			{
				XMLReader xml;
				if(pSite->pDefaultPages->ToXML(&xml))
				{
					SendCommand(pClient, &xml);
				}
			}
		}
		pSock->Free(&CmdData);
	} //::Configuration:DefaultPages|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Configuration:VirtualRoots|", &CmdData))
	{
		int iSiteIndex = atoi((char *)CmdData.pBuffer);
		if(iSiteIndex == INVALID_SITE_INDEX)
		{
			XMLReader xml;
			if(gWebSites->DfltVirtualRoots->ToXML(&xml))
			{
				SendCommand(pClient, &xml);
			}
		}
		else {
			CWebSite *pSite = gWebSites->Site(iSiteIndex);
			if(pSite)
			{
				XMLReader xml;
				if(pSite->pVirtualRoots->ToXML(&xml))
				{
					SendCommand(pClient, &xml);
				}
			}
		}
		pSock->Free(&CmdData);
	} //::Configuration:VirtualRoots|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Configuration:Cache|", &CmdData))
	{
		int iSiteIndex = atoi((char *)CmdData.pBuffer);
		CWebSite *pSite = gWebSites->Site(iSiteIndex);
		if(pSite)
		{
			XMLReader xml;
			if(pSite->pCompression->CacheToXML(&xml))
			{
				SendCommand(pClient, &xml);
			}
		}
		pSock->Free(&CmdData);
	} //::Configuration:Cache|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Configuration:Compression|", &CmdData))
	{
		int iSiteIndex = atoi((char *)CmdData.pBuffer);
		if(iSiteIndex == INVALID_SITE_INDEX)
		{
			XMLReader xml;
			if(gWebSites->DfltCompression->ToXML(&xml))
			{
				SendCommand(pClient, &xml);
			}
		}
		else {
			CWebSite *pSite = gWebSites->Site(iSiteIndex);
			if(pSite)
			{
				XMLReader xml;
				if(pSite->pCompression->ToXML(&xml))
				{
					SendCommand(pClient, &xml);
				}
			}
		}
		pSock->Free(&CmdData);
	} //::Configuration:Compression|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Configuration:MimeTypes|", &CmdData))
	{
		int iSiteIndex = atoi((char *)CmdData.pBuffer);
		if(iSiteIndex == INVALID_SITE_INDEX)
		{
			XMLReader xml;
			if(gWebSites->DfltMimeTypes->ToXML(&xml))
			{
				SendCommand(pClient, &xml);
			}
		}
		else {
			CWebSite *pSite = gWebSites->Site(iSiteIndex);
			if(pSite)
			{
				XMLReader xml;
				if(pSite->pMimeTypes->ToXML(&xml))
				{
					SendCommand(pClient, &xml);
				}
			}
		}
		pSock->Free(&CmdData);
	} //::Configuration:MimeTypes|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Configuration:IPFilters|", &CmdData))
	{
		int iSiteIndex = atoi((char *)CmdData.pBuffer);
		if(iSiteIndex == INVALID_SITE_INDEX)
		{
			XMLReader xml;
			if(gWebSites->DfltIPFilters->ToXML(&xml))
			{
				SendCommand(pClient, &xml);
			}
		}
		else {
			CWebSite *pSite = gWebSites->Site(iSiteIndex);
			if(pSite)
			{
				XMLReader xml;
				if(pSite->pIPFilters->ToXML(&xml))
				{
					SendCommand(pClient, &xml);
				}
			}
		}
		pSock->Free(&CmdData);
	} //::Configuration:IPFilters|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Configuration:URLFilters|", &CmdData))
	{
		int iSiteIndex = atoi((char *)CmdData.pBuffer);
		if(iSiteIndex == INVALID_SITE_INDEX)
		{
			XMLReader xml;
			if(gWebSites->DfltURLFilters->ToXML(&xml))
			{
				SendCommand(pClient, &xml);
			}
		}
		else {
			CWebSite *pSite = gWebSites->Site(iSiteIndex);
			if(pSite)
			{
				XMLReader xml;
				if(pSite->pURLFilters->ToXML(&xml))
				{
					SendCommand(pClient, &xml);
				}
			}
		}
		pSock->Free(&CmdData);
	} //::Configuration:URLFilters|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Configuration:ExtensionFilters|", &CmdData))
	{
		int iSiteIndex = atoi((char *)CmdData.pBuffer);
		if(iSiteIndex == INVALID_SITE_INDEX)
		{
			XMLReader xml;
			if(gWebSites->DfltExtensionFilters->ToXML(&xml))
			{
				SendCommand(pClient, &xml);
			}
		}
		else {
			CWebSite *pSite = gWebSites->Site(iSiteIndex);
			if(pSite)
			{
				XMLReader xml;
				if(pSite->pExtensionFilters->ToXML(&xml))
				{
					SendCommand(pClient, &xml);
				}
			}
		}
		pSock->Free(&CmdData);
	} //::Configuration:ExtensionFilters|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Configuration:CustomFilters|", &CmdData))
	{
		int iSiteIndex = atoi((char *)CmdData.pBuffer);
		if(iSiteIndex == INVALID_SITE_INDEX)
		{
			XMLReader xml;
			if(gWebSites->DfltCustomFilters->ToXML(&xml))
			{
				SendCommand(pClient, &xml);
			}
		}
		else {
			CWebSite *pSite = gWebSites->Site(iSiteIndex);
			if(pSite)
			{
				XMLReader xml;
				if(pSite->pCustomFilters->ToXML(&xml))
				{
					SendCommand(pClient, &xml);
				}
			}
		}
		pSock->Free(&CmdData);
	} //::Configuration:CustomFilters|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Configuration:ScriptingEngines|", &CmdData))
	{
		int iSiteIndex = atoi((char *)CmdData.pBuffer);
		if(iSiteIndex == INVALID_SITE_INDEX)
		{
			XMLReader xml;
			if(gWebSites->DfltScriptingEngines->ToXML(&xml))
			{
				SendCommand(pClient, &xml);
			}
		}
		else {
			CWebSite *pSite = gWebSites->Site(iSiteIndex);
			if(pSite)
			{
				XMLReader xml;
				if(pSite->pScriptingEngines->ToXML(&xml))
				{
					SendCommand(pClient, &xml);
				}
			}
		}
		pSock->Free(&CmdData);
	} //::Configuration:ScriptingEngines|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Configuration:SSIFiles|", &CmdData))
	{
		int iSiteIndex = atoi((char *)CmdData.pBuffer);
		if(iSiteIndex == INVALID_SITE_INDEX)
		{
			XMLReader xml;
			if(gWebSites->DfltSSIFiles->ToXML(&xml))
			{
				SendCommand(pClient, &xml);
			}
		}
		else {
			CWebSite *pSite = gWebSites->Site(iSiteIndex);
			if(pSite)
			{
				XMLReader xml;
				if(pSite->pSSIFiles->ToXML(&xml))
				{
					SendCommand(pClient, &xml);
				}
			}
		}
		pSock->Free(&CmdData);
	} //::Configuration:SSIFiles|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Configuration:CGIFolders|", &CmdData))
	{
		int iSiteIndex = atoi((char *)CmdData.pBuffer);
		if(iSiteIndex == INVALID_SITE_INDEX)
		{
			XMLReader xml;
			if(gWebSites->DfltCGIFolders->ToXML(&xml))
			{
				SendCommand(pClient, &xml);
			}
		}
		else {
			CWebSite *pSite = gWebSites->Site(iSiteIndex);
			if(pSite)
			{
				XMLReader xml;
				if(pSite->pCGIFolders->ToXML(&xml))
				{
					SendCommand(pClient, &xml);
				}
			}
		}
		pSock->Free(&CmdData);
	} //::Configuration:CGIFolders|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Configuration:ErrorPages|", &CmdData))
	{
		int iSiteIndex = atoi((char *)CmdData.pBuffer);
		if(iSiteIndex == INVALID_SITE_INDEX)
		{
			XMLReader xml;
			if(gWebSites->DfltErrorPages->ToXML(&xml))
			{
				SendCommand(pClient, &xml);
			}
		}
		else {
			CWebSite *pSite = gWebSites->Site(iSiteIndex);
			if(pSite)
			{
				XMLReader xml;
				if(pSite->pErrorPages->ToXML(&xml))
				{
					SendCommand(pClient, &xml);
				}
			}
		}
		pSock->Free(&CmdData);
	} //::Configuration:ErrorPages|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::IsGlobal|", &CmdData))
	{
		char sTok[MAX_TEXT_SIZE];
		int iRPos = 0;
		int iTokLen = 0;
		int iSiteIndex = INVALID_SITE_INDEX;
		bool bResult = false;

		iTokLen = GetNextToken(((char *)CmdData.pBuffer),  CmdData.iLength, sTok, sizeof(sTok), &iRPos, ":", 1);
		if(((char *)CmdData.pBuffer)[iRPos] == ':')
		{
			iRPos++;
		}

		if((iSiteIndex = atoi(sTok)) != INVALID_SITE_INDEX)
		{
			CWebSite *pSite = gWebSites->Site(iSiteIndex);
			if(pSite)
			{
				GetNextToken(((char *)CmdData.pBuffer),  CmdData.iLength, sTok, sizeof(sTok), &iRPos, ":", 1);

				if(_strcmpi(sTok, "CGIFolders") == 0) {
					SendCommand(pClient, pSite->pCGIFolders == gWebSites->DfltCGIFolders ? "Yes" : "No");
				}
				else if(_strcmpi(sTok, "Compression") == 0) {
					SendCommand(pClient, pSite->pCompression == gWebSites->DfltCompression ? "Yes" : "No");
				}
				else if(_strcmpi(sTok, "CustomFilters") == 0) {
					SendCommand(pClient, pSite->pCustomFilters == gWebSites->DfltCustomFilters ? "Yes" : "No");
				}
				else if(_strcmpi(sTok, "DefaultPages") == 0) {
					SendCommand(pClient, pSite->pDefaultPages == gWebSites->DfltDefaultPages ? "Yes" : "No");
				}
				else if(_strcmpi(sTok, "ErrorPages") == 0) {
					SendCommand(pClient, pSite->pErrorPages == gWebSites->DfltErrorPages ? "Yes" : "No");
				}
				else if(_strcmpi(sTok, "ExtensionFilters") == 0) {
					SendCommand(pClient, pSite->pExtensionFilters == gWebSites->DfltExtensionFilters ? "Yes" : "No");
				}
				else if(_strcmpi(sTok, "IPFilters") == 0) {
					SendCommand(pClient, pSite->pIPFilters == gWebSites->DfltIPFilters ? "Yes" : "No");
				}
				else if(_strcmpi(sTok, "MimeTypes") == 0) {
					SendCommand(pClient, pSite->pMimeTypes == gWebSites->DfltMimeTypes ? "Yes" : "No");
				}
				else if(_strcmpi(sTok, "ScriptingEngines") == 0) {
					SendCommand(pClient, pSite->pScriptingEngines == gWebSites->DfltScriptingEngines ? "Yes" : "No");
				}
				else if(_strcmpi(sTok, "SSIFiles") == 0) {
					SendCommand(pClient, pSite->pSSIFiles == gWebSites->DfltSSIFiles ? "Yes" : "No");
				}
				else if(_strcmpi(sTok, "URLFilters") == 0) {
					SendCommand(pClient, pSite->pURLFilters == gWebSites->DfltURLFilters ? "Yes" : "No");
				}
				else if(_strcmpi(sTok, "VirtualRoots") == 0) {
					SendCommand(pClient, pSite->pVirtualRoots == gWebSites->DfltVirtualRoots ? "Yes" : "No");
				}
				else if(_strcmpi(sTok, "WebsiteSettings") == 0) {
					SendCommand(pClient, pSite->pWebsiteSettings == gWebSites->DfltWebsiteSettings ? "Yes" : "No");
				}
				else {
					SendCommand(pClient, "Unknown");
				}
			}
		}
		pSock->Free(&CmdData);
	} //::IsGlobal|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Update:Website|", &CmdData))
	{
		bool bResult = false;
		XMLReader xml((char *)CmdData.pBuffer, CmdData.iLength);

		int iSiteIndex = xml.ToInteger("Index");

		CWebSite *pSite = gWebSites->Site(iSiteIndex);
		if(pSite)
		{
			if(pSite->Reload(&xml))
			{
				bResult = gWebSites->Save(false);
			}
		}
		else{
			bResult = false;
		}

		StringBuilder errMsg;
		bResult = gWebSites->StopTrace(&errMsg) == 0 && bResult;
		gWebSites->StartTrace();

		XMLWriter resultXML("ResultInformation");
		resultXML.Add("Result", bResult);
		resultXML.Add("Text", errMsg.Buffer);
		SendCommand(pClient, &resultXML);

		pSock->Free(&CmdData);
	} //::Update:Website|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Update:Websites|", &CmdData))
	{
		XMLReader xml((char *)CmdData.pBuffer, CmdData.iLength);

		bool bResult = true;

		char sName[MAX_SITE_NAME];
		char sRoot[MAX_PATH];
		char sDescription[MAX_SITE_DESCR];
		char sSocketPoolID[SOCKETPOOLS_MAX_UID_LENGTH];
		int iSiteIndex = INVALID_SITE_INDEX;

		xml.ProgressiveScan(true);

		gWebSites->StopAllSites();
		gWebSites->DeReferenceAllSites();

		XMLReader item;
		while(xml.ToReader("Site", &item))
		{
			item.ToString("Name", sName, sizeof(sName));
			item.ToString("Description", sDescription, sizeof(sDescription));
			item.ToString("Root", sRoot, sizeof(sRoot));
			item.ToString("SocketPoolID", sSocketPoolID, sizeof(sSocketPoolID));
			iSiteIndex = item.ToInteger("Index");

			if(iSiteIndex == INVALID_SITE_INDEX)
			{
				bResult = bResult && gWebSites->Add(&item, &iSiteIndex);
			}
			else{
				CWebSite *pSite = gWebSites->Site(iSiteIndex);
				if(pSite)
				{
					strcpy_s(pSite->Name, sizeof(pSite->Name), sName);
					strcpy_s(pSite->Root, sizeof(pSite->Root), sRoot);
					strcpy_s(pSite->Description, sizeof(pSite->Description), sDescription);
					strcpy_s(pSite->SocketPoolID, sizeof(pSite->SocketPoolID), sSocketPoolID);
				}
			}

			CWebSite *pSite = gWebSites->Site(iSiteIndex);
			if(pSite)
			{
				pSite->bReferencedInUpdate = true;
			}

			item.Destroy();
		}

		bResult = bResult && gWebSites->Save(true);

		gWebSites->ReloadSites();

		StringBuilder errMsg;
		bResult = gWebSites->StopTrace(&errMsg) == 0 && bResult;
		gWebSites->StartTrace();

		XMLWriter resultXML("ResultInformation");
		resultXML.Add("Result", bResult);
		resultXML.Add("Text", errMsg.Buffer);
		SendCommand(pClient, &resultXML);

		pSock->Free(&CmdData);
	} //::Update:Websites|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Update|", &CmdData))
	{
		char sTok[MAX_TEXT_SIZE];
		int iRPos = 0;
		int iTokLen = 0;
		int iSiteIndex = INVALID_SITE_INDEX;
		bool bResult = false;

		iTokLen = GetNextToken(((char *)CmdData.pBuffer),  CmdData.iLength, sTok, sizeof(sTok), &iRPos, ":", 1);
		if(((char *)CmdData.pBuffer)[iRPos] == ':')
		{
			iRPos++;
		}

		iSiteIndex = atoi(sTok);

		iTokLen = GetNextToken(((char *)CmdData.pBuffer),  CmdData.iLength, sTok, sizeof(sTok), &iRPos, ":", 2);
		if(((char *)CmdData.pBuffer)[iRPos] == ':')
		{
			iRPos++;
		}

		XMLReader xmlReader(((char *)CmdData.pBuffer) + iRPos, CmdData.iLength - iRPos);

		if(iSiteIndex == INVALID_SITE_INDEX)
		{
			//-------------------------------------------------------------------------
			//Apply globally defined values. ------------------------------------------
			//-------------------------------------------------------------------------

			if(_strcmpi(sTok, "SocketPools") == 0)
			{
				if((bResult = gWebSites->pSocketPools->Load(&xmlReader)))
				{
					bResult = gWebSites->pSocketPools->Save();
				}
				gWebSites->ReloadSites();
			}
			else if(_strcmpi(sTok, "ServerSettings") == 0)
			{
				if((bResult = gWebSites->pServerSettings->Load(&xmlReader, NULL)))
				{
					bResult = gWebSites->pServerSettings->Save();
				}
			}
			else if(_strcmpi(sTok, "CGIFolders") == 0)
			{
				if((bResult = gWebSites->DfltCGIFolders->Load(&xmlReader, NULL)))
				{
					bResult = gWebSites->DfltCGIFolders->Save();
				}
			}
			else if(_strcmpi(sTok, "Compression") == 0)
			{
				if((bResult = gWebSites->DfltCompression->Load(&xmlReader, NULL)))
				{
					bResult = gWebSites->DfltCompression->Save();
				}
			}
			else if(_strcmpi(sTok, "CustomFilters") == 0)
			{
				if((bResult = gWebSites->DfltCustomFilters->Load(&xmlReader, NULL)))
				{
					bResult = gWebSites->DfltCustomFilters->Save();
				}
			}
			else if(_strcmpi(sTok, "DefaultPages") == 0)
			{
				if((bResult = gWebSites->DfltDefaultPages->Load(&xmlReader, NULL)))
				{
					bResult = gWebSites->DfltDefaultPages->Save();
				}
			}
			else if(_strcmpi(sTok, "ErrorPages") == 0)
			{
				if((bResult = gWebSites->DfltErrorPages->Load(&xmlReader, NULL)))
				{
					bResult = gWebSites->DfltErrorPages->Save();
				}
			}
			else if(_strcmpi(sTok, "ExtensionFilters") == 0)
			{
				if((bResult = gWebSites->DfltExtensionFilters->Load(&xmlReader, NULL)))
				{
					bResult = gWebSites->DfltExtensionFilters->Save();
				}
			}
			else if(_strcmpi(sTok, "IPFilters") == 0)
			{
				if((bResult = gWebSites->DfltIPFilters->Load(&xmlReader, NULL)))
				{
					bResult = gWebSites->DfltIPFilters->Save();
				}
			}
			else if(_strcmpi(sTok, "MimeTypes") == 0)
			{
				if((bResult = gWebSites->DfltMimeTypes->Load(&xmlReader, NULL)))
				{
					bResult = gWebSites->DfltMimeTypes->Save();
				}
			}
			else if(_strcmpi(sTok, "ScriptingEngines") == 0)
			{
				if((bResult = gWebSites->DfltScriptingEngines->Load(&xmlReader, NULL)))
				{
					bResult = gWebSites->DfltScriptingEngines->Save();
				}
			}
			else if(_strcmpi(sTok, "WebsiteSettings") == 0)
			{
				if((bResult = gWebSites->DfltWebsiteSettings->Load(&xmlReader, NULL)))
				{
					bResult = gWebSites->DfltWebsiteSettings->Save();
				}
			}
			else if(_strcmpi(sTok, "SSIFiles") == 0)
			{
				if((bResult = gWebSites->DfltSSIFiles->Load(&xmlReader, NULL)))
				{
					bResult = gWebSites->DfltSSIFiles->Save();
				}
			}
			else if(_strcmpi(sTok, "URLFilters") == 0)
			{
				if((bResult = gWebSites->DfltURLFilters->Load(&xmlReader, NULL)))
				{
					bResult = gWebSites->DfltURLFilters->Save();
				}
			}
			else if(_strcmpi(sTok, "VirtualRoots") == 0)
			{
				if((bResult = gWebSites->DfltVirtualRoots->Load(&xmlReader, NULL)))
				{
					bResult = gWebSites->DfltVirtualRoots->Save();
				}
			}
		} //if(iSiteIndex == INVALID_SITE_INDEX)
		else {
			//-------------------------------------------------------------------------
			//Apply site level defined values. ----------------------------------------
			//-------------------------------------------------------------------------

			CWebSite *pSite = gWebSites->Site(iSiteIndex);
			if(pSite)
			{
				bool bIsSiteActive = pSite->IsActive();
				if(bIsSiteActive)
				{
					pSite->Stop();
				}

				if(_strcmpi(sTok, "CGIFolders") == 0)
				{
					if(_strcmpi(xmlReader.Text(), "~Global") == 0)
					{
						if(pSite->pCGIFolders != gWebSites->DfltCGIFolders)
						{
							delete (CCGIFolders *)pMem->UnTrackMemory(pSite->pCGIFolders);
						}
						pSite->pCGIFolders = gWebSites->DfltCGIFolders;
						bResult = true;
					}
					else {
						if(pSite->pCGIFolders == gWebSites->DfltCGIFolders)
						{
							pSite->pCGIFolders = (CCGIFolders *) pMem->TrackMemory(new CCGIFolders(gWebSites));
						}
						bResult = pSite->pCGIFolders->Load(&xmlReader, NULL);
					}
				}
				else if(_strcmpi(sTok, "Compression") == 0)
				{
					if(_strcmpi(xmlReader.Text(), "~Global") == 0)
					{
						if(pSite->pCompression != gWebSites->DfltCompression)
						{
							delete (CCompression *)pMem->UnTrackMemory(pSite->pCompression);
						}
						pSite->pCompression = gWebSites->DfltCompression;
						bResult = true;
					}
					else {
						if(pSite->pCompression == gWebSites->DfltCompression)
						{
							pSite->pCompression = (CCompression *) pMem->TrackMemory(new CCompression(gWebSites));
						}
						bResult = pSite->pCompression->Load(&xmlReader, NULL);
					}
				}
				else if(_strcmpi(sTok, "CustomFilters") == 0)
				{
					if(_strcmpi(xmlReader.Text(), "~Global") == 0)
					{
						if(pSite->pCustomFilters != gWebSites->DfltCustomFilters)
						{
							delete (CCustomFilters *)pMem->UnTrackMemory(pSite->pCustomFilters);
						}
						pSite->pCustomFilters = gWebSites->DfltCustomFilters;
						bResult = true;
					}
					else {
						if(pSite->pCustomFilters == gWebSites->DfltCustomFilters)
						{
							pSite->pCustomFilters = (CCustomFilters *) pMem->TrackMemory(new CCustomFilters(gWebSites));
						}
						bResult = pSite->pCustomFilters->Load(&xmlReader, NULL);
					}
				}
				else if(_strcmpi(sTok, "DefaultPages") == 0)
				{
					if(_strcmpi(xmlReader.Text(), "~Global") == 0)
					{
						if(pSite->pDefaultPages != gWebSites->DfltDefaultPages)
						{
							delete (CDefaultPages *)pMem->UnTrackMemory(pSite->pDefaultPages);
						}
						pSite->pDefaultPages = gWebSites->DfltDefaultPages;
						bResult = true;
					}
					else {
						if(pSite->pDefaultPages == gWebSites->DfltDefaultPages)
						{
							pSite->pDefaultPages = (CDefaultPages *) pMem->TrackMemory(new CDefaultPages(gWebSites));
						}
						bResult = pSite->pDefaultPages->Load(&xmlReader, NULL);
					}
				}
				else if(_strcmpi(sTok, "ErrorPages") == 0)
				{
					if(_strcmpi(xmlReader.Text(), "~Global") == 0)
					{
						if(pSite->pErrorPages != gWebSites->DfltErrorPages)
						{
							delete (CErrorPages *)pMem->UnTrackMemory(pSite->pErrorPages);
						}
						pSite->pErrorPages = gWebSites->DfltErrorPages;
						bResult = true;
					}
					else {
						if(pSite->pErrorPages == gWebSites->DfltErrorPages)
						{
							pSite->pErrorPages = (CErrorPages *) pMem->TrackMemory(new CErrorPages(gWebSites));
						}
						bResult = pSite->pErrorPages->Load(&xmlReader, NULL);
					}
				}
				else if(_strcmpi(sTok, "ExtensionFilters") == 0)
				{
					if(_strcmpi(xmlReader.Text(), "~Global") == 0)
					{
						if(pSite->pExtensionFilters != gWebSites->DfltExtensionFilters)
						{
							delete (CExtensionFilters *)pMem->UnTrackMemory(pSite->pExtensionFilters);
						}
						pSite->pExtensionFilters = gWebSites->DfltExtensionFilters;
						bResult = true;
					}
					else {
						if(pSite->pExtensionFilters == gWebSites->DfltExtensionFilters)
						{
							pSite->pExtensionFilters = (CExtensionFilters *) pMem->TrackMemory(new CExtensionFilters(gWebSites));
						}
						bResult = pSite->pExtensionFilters->Load(&xmlReader, NULL);
					}
				}
				else if(_strcmpi(sTok, "IPFilters") == 0)
				{
					if(_strcmpi(xmlReader.Text(), "~Global") == 0)
					{
						if(pSite->pIPFilters != gWebSites->DfltIPFilters)
						{
							delete (CIPFilters *)pMem->UnTrackMemory(pSite->pIPFilters);
						}
						pSite->pIPFilters = gWebSites->DfltIPFilters;
						bResult = true;
					}
					else {
						if(pSite->pIPFilters == gWebSites->DfltIPFilters)
						{
							pSite->pIPFilters = (CIPFilters *) pMem->TrackMemory(new CIPFilters(gWebSites));
						}
						bResult = pSite->pIPFilters->Load(&xmlReader, NULL);
					}
				}
				else if(_strcmpi(sTok, "MimeTypes") == 0)
				{
					if(_strcmpi(xmlReader.Text(), "~Global") == 0)
					{
						if(pSite->pMimeTypes != gWebSites->DfltMimeTypes)
						{
							delete (CMimeTypes *)pMem->UnTrackMemory(pSite->pMimeTypes);
						}
						pSite->pMimeTypes = gWebSites->DfltMimeTypes;
						bResult = true;
					}
					else {
						if(pSite->pMimeTypes == gWebSites->DfltMimeTypes)
						{
							pSite->pMimeTypes = (CMimeTypes *) pMem->TrackMemory(new CMimeTypes(gWebSites));
						}
						bResult = pSite->pMimeTypes->Load(&xmlReader, NULL);
					}
				}
				else if(_strcmpi(sTok, "ScriptingEngines") == 0)
				{
					if(_strcmpi(xmlReader.Text(), "~Global") == 0)
					{
						if(pSite->pScriptingEngines != gWebSites->DfltScriptingEngines)
						{
							delete (CScriptingEngines *)pMem->UnTrackMemory(pSite->pScriptingEngines);
						}
						pSite->pScriptingEngines = gWebSites->DfltScriptingEngines;
						bResult = true;
					}
					else {
						if(pSite->pScriptingEngines == gWebSites->DfltScriptingEngines)
						{
							pSite->pScriptingEngines = (CScriptingEngines *) pMem->TrackMemory(new CScriptingEngines(gWebSites));
						}
						bResult = pSite->pScriptingEngines->Load(&xmlReader, NULL);
					}
				}
				else if(_strcmpi(sTok, "WebsiteSettings") == 0)
				{
					if(_strcmpi(xmlReader.Text(), "~Global") == 0)
					{
						if(pSite->pWebsiteSettings != gWebSites->DfltWebsiteSettings)
						{
							delete (CWebsiteSettings *)pMem->UnTrackMemory(pSite->pWebsiteSettings);
						}
						pSite->pWebsiteSettings = gWebSites->DfltWebsiteSettings;
						bResult = true;
					}
					else {
						if(pSite->pWebsiteSettings == gWebSites->DfltWebsiteSettings)
						{
							pSite->pWebsiteSettings = (CWebsiteSettings *) pMem->TrackMemory(new CWebsiteSettings(gWebSites));
						}
						bResult = pSite->pWebsiteSettings->Load(&xmlReader, NULL);
					}
				}
				else if(_strcmpi(sTok, "SSIFiles") == 0)
				{
					if(_strcmpi(xmlReader.Text(), "~Global") == 0)
					{
						if(pSite->pSSIFiles != gWebSites->DfltSSIFiles)
						{
							delete (CSSIFiles *)pMem->UnTrackMemory(pSite->pSSIFiles);
						}
						pSite->pSSIFiles = gWebSites->DfltSSIFiles;
						bResult = true;
					}
					else {
						if(pSite->pSSIFiles == gWebSites->DfltSSIFiles)
						{
							pSite->pSSIFiles = (CSSIFiles *) pMem->TrackMemory(new CSSIFiles(gWebSites));
						}
						bResult = pSite->pSSIFiles->Load(&xmlReader, NULL);
					}
				}
				else if(_strcmpi(sTok, "URLFilters") == 0)
				{
					if(_strcmpi(xmlReader.Text(), "~Global") == 0)
					{
						if(pSite->pURLFilters != gWebSites->DfltURLFilters)
						{
							delete (CURLFilters *)pMem->UnTrackMemory(pSite->pURLFilters);
						}
						pSite->pURLFilters = gWebSites->DfltURLFilters;
						bResult = true;
					}
					else {
						if(pSite->pURLFilters == gWebSites->DfltURLFilters)
						{
							pSite->pURLFilters = (CURLFilters *) pMem->TrackMemory(new CURLFilters(gWebSites));
						}
						bResult = pSite->pURLFilters->Load(&xmlReader, NULL);
					}
				}
				else if(_strcmpi(sTok, "VirtualRoots") == 0)
				{
					if(_strcmpi(xmlReader.Text(), "~Global") == 0)
					{
						if(pSite->pVirtualRoots != gWebSites->DfltVirtualRoots)
						{
							delete (CVirtualRoots *)pMem->UnTrackMemory(pSite->pVirtualRoots);
						}
						pSite->pVirtualRoots = gWebSites->DfltVirtualRoots;
						bResult = true;
					}
					else {
						if(pSite->pVirtualRoots == gWebSites->DfltVirtualRoots)
						{
							pSite->pVirtualRoots = (CVirtualRoots *) pMem->TrackMemory(new CVirtualRoots(gWebSites));
						}
						bResult = pSite->pVirtualRoots->Load(&xmlReader, NULL);
					}
				}

				if(bResult)
				{
					bResult = gWebSites->Save(false);

					if(bIsSiteActive && bResult)
					{
						bResult = pSite->Start();
					}
				}
			}
		} //if(iSiteIndex != INVALID_SITE_INDEX)

		StringBuilder errMsg;
		XMLWriter resultXML("ResultInformation");
		resultXML.Add("Result", gWebSites->StopTrace(&errMsg) == 0 && bResult);
		resultXML.Add("Text", errMsg.Buffer);
		SendCommand(pClient, &resultXML);

		pSock->Free(&CmdData);
	} //::Update|
	//---------------------------------------------------------------------------------------------------------------------
	else{
		//printf("Unknown command: \"%s\"\n", pChunk->pBuffer);
		SendCommand(pClient, "Unknown command");
	}
	//---------------------------------------------------------------------------------------------------------------------

	gWebSites->StopTrace();

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CManagement::SendCommand(CSocketClient *pClient, const char *sText, ...)
{
	bool bResult = false;

	va_list ArgList;
	va_start(ArgList, sText);

	int iMemoryRequired = _vscprintf(sText, ArgList);
	char *sBuf = (char *) pMem->Allocate(sizeof(char), iMemoryRequired + 1);
	int iSz = _vsprintf_s_l(sBuf, iMemoryRequired + 1, sText, NULL, ArgList);
	va_end(ArgList);
	if(iSz > 0)
	{
		bResult = pClient->SetNextSendData(sBuf, iSz);
	}
	pMem->Free(sBuf);

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CManagement::SendCommand(CSocketClient *pClient, XMLReader *pXML)
{
	const char *sText = pXML->Text();
	if(sText)
	{
		SendCommand(pClient, "%s", pXML->Text());
	}
	else{
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CManagement::SendCommand(CSocketClient *pClient, XMLWriter *pXML)
{
	char *sText = pXML->Text();
	if(sText)
	{
		SendCommand(pClient, "%s", sText);
		#ifdef _USE_GLOBAL_MEMPOOL
			pMem->Free(sText);
		#else
			free(sText);
		#endif
	}
	else{
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CManagement::SendCommand(CSocketClient *pClient, LPBASICHUNK pReturnData, const char *sText, ...)
{
	bool bResult = false;

	va_list ArgList;
	va_start(ArgList, sText);

	int iMemoryRequired = _vscprintf(sText, ArgList);
	char *sBuf = (char *) pMem->Allocate(sizeof(char), iMemoryRequired + 1);
	int iSz = _vsprintf_s_l(sBuf, iMemoryRequired + 1, sText, NULL, ArgList);
	va_end(ArgList);
	if(iSz > 0)
	{
		if(pClient->SetNextSendData(sBuf, iSz))
		{
			bResult = pClient->GetNextClientData(pReturnData);
		}
	}
	pMem->Free(sBuf);

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Mgmt_Sock_OnSockError(CSocketServer *pSock, CSocketClient *pClient, int iErrorNumber, const char *sErrorMsg)
{
	//FIXFIX: Log these errors.
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Mgmt_Sock_ClientThread(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	while(pClient->IsConnected())
	{
		if(pClient->GetClientData(pChunk))
		{
			if(!((CManagement*)pSock->pUserData)->ProcessCommand(pSock, pClient, pChunk))
			{
				return;
			}
		}
		Sleep(1);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MgMt_OnAfterDePacketize(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	CManagement *pMgmt  = ((CManagement*)pSock->pUserData);
	pMgmt->Cytpto.Cipher(pChunk->pBuffer, pChunk->iLength);
	//printf("Recv: %s\n", pChunk->pBuffer);

	if(((CManagement*)pSock->pUserData)->ProcessUrgentCommand(pSock, pClient, pChunk))
	{
		return false; //This packet was handled and should be removed from the queue.
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MgMt_OnBeforePacketize(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	CManagement *pMgmt  = ((CManagement*)pSock->pUserData);
	//printf("Send: %s\n", pChunk->pBuffer);
	pMgmt->Cytpto.Cipher(pChunk->pBuffer, pChunk->iLength);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MgMt_OnBeginClientThread(CSocketServer *pSock, CSocketClient *pClient, HANDLE hThread)
{
	CManagement *pMgmt  = ((CManagement*)pSock->pUserData);

	if(pSock->CurrentConnections() > 1)
	{
		pMgmt->SendCommand(pClient, "This server is already being administered.");
		pClient->WaitOnDataToBeSent();
		pClient->Disconnect();
	}
	else if(pClient->iSlot == 0) //Only slot zero it ever valid.
	{
		((CManagement*)pSock->pUserData)->bIsAuthorized = false;

		/*
		#ifdef _DEBUG
		AllocConsole();

		//Redirect stdout to the newly allocated console.
		int hCrt, i;
		FILE *hf;
		hCrt = _open_osfhandle((long) GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
		hf = _fdopen( hCrt, "w" );
		*stdout = *hf;
		i = setvbuf( stdout, NULL, _IONBF, 0);

		printf("%s Service %s\n\n", gsTitleCaption, gsFileVersion);
		#endif
		*/

		pMgmt->SendCommand(pClient, "Ok");
	}
	else {
		pMgmt->SendCommand(pClient, "The server rejected your connection.");
		pClient->WaitOnDataToBeSent();
		pClient->Disconnect();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MgMt_OnEndClientThread(CSocketServer *pSock, CSocketClient *pClient)
{
	if(pClient->iSlot == 0) //Only slot zero is ever valid.
	{
		((CManagement*)pSock->pUserData)->bIsAuthorized = false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MgMt_OnAccept(CSocketServer *pSock, CSocketClient *pClient)
{
	if(gWebSites && gWebSites->pServerSettings)
	{
		if(gWebSites->pServerSettings->Settings.Users.AllowRemoteManagement)
		{
			return true;
		}
		else {
			return(strcmp(pClient->PeerAddress(), "127.0.0.1") == 0);
		}
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
