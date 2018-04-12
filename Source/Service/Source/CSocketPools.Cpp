///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CSocketPools_Cpp
#define _CSocketPools_Cpp
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <WindowsX.H>
#include <ShellAPI.H>
#include <Stdio.H>
#include <Stdlib.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"
#include "CWebSites.H"
#include "SocketHandlers.H"
#include "CSocketPools.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::File;
using namespace NSWFL::ListView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CSocketPools::~CSocketPools(void)
{
	this->Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CSocketPools::CSocketPools(void *lpWebSites)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSocketPools::Save(void)
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

bool CSocketPools::ToXML(XMLReader *lpXML)
{
	return this->ToXML(lpXML, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSocketPools::ToXML(XMLReader *lpXML, bool bIncludeStats)
{
	this->Locks.LockShared();

	XMLWriter xmlConfig("SocketPools");

	for(int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		XMLWriter Item("SocketPool");
		Item.Add("UID", this->Collection.Items[iItem].UID);
		Item.Add("Port", this->Collection.Items[iItem].Port);
		Item.Add("MaxConnections", this->Collection.Items[iItem].MaxConnections);
		Item.Add("InitialBufferSize", this->Collection.Items[iItem].InitialBufferSize);
		Item.Add("MaxBufferSize", this->Collection.Items[iItem].MaxBufferSize);
		Item.Add("FileBufferSize", this->Collection.Items[iItem].FileBufferSize);
		Item.Add("Description", this->Collection.Items[iItem].Description);

		if(bIncludeStats)
		{
			Item.Add("IsActive", this->Collection.Items[iItem].pSocket->IsActive());
			Item.Add("ReferencedSites", this->ReferencedSiteCount(&this->Collection.Items[iItem]));
		}

		XMLWriter xmlIPs("IPAddresses");
		xmlIPs.AddBool("ListenOnAll", this->Collection.Items[iItem].ListenIPs.ListenOnAll);
		for(int iIP = 0; iIP < this->Collection.Items[iItem].ListenIPs.Count; iIP++)
		{
			XMLWriter Item("IPAddress");
			Item.Add("Address", this->Collection.Items[iItem].ListenIPs.Items[iIP].IP);
			Item.Add("Description", this->Collection.Items[iItem].ListenIPs.Items[iIP].Description);
			Item.Add("Enable", this->Collection.Items[iItem].ListenIPs.Items[iIP].Enabled);
			xmlIPs.Add(&Item);
		}

		Item.Add(&xmlIPs);

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
bool CSocketPools::Reload(void)
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

bool CSocketPools::Load(const char *sXMLFileName)
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
		if(xmlConfig.ToReader("SocketPools", &xmlEntity))
		{
			this->Initialized = this->Load(&xmlEntity);
		}
		xmlEntity.Destroy();
		xmlConfig.Destroy();
	}

	return this->Locks.UnlockExclusive(this->Initialized);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSocketPools::Load(XMLReader *xmlConfig)
{
	if(this->Initialized)
	{
		this->Destroy();
	}

	this->Locks.LockExclusive();

	CWebSites *pWebSites = ((CWebSites *)this->pWebSites);

	int iLength = 0;
	memset(&this->Collection, 0, sizeof(this->Collection));

	XMLReader xml;

	xmlConfig->ProgressiveScan(true);

	while(xmlConfig->ToReader("SocketPool", &xml))
	{
		char sDescription[SOCKETPOOLS_MAX_DESCR_LENGTH];
		char sUID[SOCKETPOOLS_MAX_UID_LENGTH];

		this->Collection.Items = (SOCKETPOOL *)
			pMem->ReAllocate(this->Collection.Items, sizeof(SOCKETPOOL), this->Collection.Count + 1);

		SOCKETPOOL *p = &this->Collection.Items[this->Collection.Count++];
		memset(p, 0, sizeof(SOCKETPOOL));

		xml.ToString("UID", sUID, sizeof(sUID), &iLength);
		if(iLength == 0)
		{
			if(NewGUID(sUID, sizeof(sUID)))
			{
				iLength = (int)strlen(sUID);
			}
		}
		p->UID = (char *) pMem->Allocate(sizeof(char), iLength + 1);
		strcpy_s(p->UID, iLength + 1, sUID);

		p->Port = xml.ToInteger("Port");
		p->MaxConnections = xml.ToInteger("MaxConnections");
		p->InitialBufferSize = xml.ToInteger("InitialBufferSize", 8 * 1024);
		p->MaxBufferSize = xml.ToInteger("MaxBufferSize", 1572864);
		p->FileBufferSize = xml.ToInteger("FileBufferSize", 10240);

		xml.ToString("Description", sDescription, sizeof(sDescription), &iLength);
		p->Description = (char *) pMem->CloneString(sDescription);
		
		//---(Load IP Addresses)--

		XMLReader xmlIPs;
		xml.ToReader("IPAddresses", &xmlIPs);

		p->ListenIPs.ListenOnAll = xmlIPs.ToBoolean("ListenOnAll", true);

		xmlIPs.ProgressiveScan(true);
		
		for(XMLReader xmlIP; xmlIPs.ToReader("IPAddress", &xmlIP);)
		{
			char sIP[SOCKETPOOLS_IPS_MAX_IP_LENGTH];
			char sDescr[SOCKETPOOLS_IPS_MAX_DESCR_LENGTH];

			p->ListenIPs.Items = (LISTENIP *)
				pMem->ReAllocate(p->ListenIPs.Items, sizeof(LISTENIP), p->ListenIPs.Count + 1);

			LISTENIP *pIP = &p->ListenIPs.Items[p->ListenIPs.Count++];
			memset(pIP, 0, sizeof(LISTENIP));

			xmlIP.ToString("Address", sIP, sizeof(sIP), &iLength);
			pIP->IP = (char *) pMem->CloneString(sIP);

			xmlIP.ToString("Description", sDescr, sizeof(sDescr), &iLength);
			pIP->Description = (char *) pMem->CloneString(sDescr);

			pIP->Enabled = xmlIP.ToBoolean("Enable", true);

			xmlIP.Destroy();
		}

		p->pSocket = (CSocketServer *) pMem->TrackMemory(new CSocketServer());

		if(p->pSocket->Initialize(p->Port, p->MaxConnections, Socket_ClientHandler, Socket_ErrorHandler))
		{
			p->pSocket->OptimizeForThroughput(pWebSites->pServerSettings->Settings.Advanced.OptimizeForThroughput);
			p->pSocket->ForceSocketReuse(pWebSites->pServerSettings->Settings.Advanced.ForceSocketReuse);
			p->pSocket->InitialBufferSize(p->InitialBufferSize);
			p->pSocket->MaxBufferSize(p->MaxBufferSize);
			p->pSocket->PacketMode(false);
			p->pSocket->SetOnSend(Socket_SendHandler);
			p->pSocket->SetOnAccept(Socket_AcceptHandler);
			p->pSocket->SetOnConsume(Socket_ConsumeHandler);
			p->pSocket->SetOnEndClientThread(Socket_EndClientThreadHandler);
		}
		else{
			((CWebSites*)this->pWebSites)->Trace("The website's server failed to initialize.");
		}

		p->Peers = (PEER *) pMem->Allocate(sizeof(PEER), p->MaxConnections);

		xml.Destroy();
	}
		
	((CWebSites*)this->pWebSites)->BindSocketPools();

	return this->Locks.UnlockExclusive(this->Initialized = true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSocketPools::Destroy(void)
{
	if(this->Initialized)
	{
		((CWebSites *)this->pWebSites)->StopAllSites();

		this->Locks.LockExclusive();

		this->Initialized = false;
		if(this->Collection.Count > 0)
		{
			for(int iPos = 0; iPos < this->Collection.Count; iPos++)
			{
				pMem->Free(this->Collection.Items[iPos].Description);
				pMem->Free(this->Collection.Items[iPos].UID);
				this->Collection.Items[iPos].pSocket->Destroy();
				delete (CSocketServer *)pMem->UnTrackMemory(this->Collection.Items[iPos].pSocket);

				if(this->Collection.Items[iPos].ListenIPs.Count)
				{
					for(int iIP = 0; iIP < this->Collection.Items[iPos].ListenIPs.Count; iIP++)
					{
						pMem->Free(this->Collection.Items[iPos].ListenIPs.Items[iIP].IP);
						pMem->Free(this->Collection.Items[iPos].ListenIPs.Items[iIP].Description);
					}

					pMem->Free(this->Collection.Items[iPos].ListenIPs.Items);
				}

				if(this->Collection.Items[iPos].Peers)
				{
					pMem->Free(this->Collection.Items[iPos].Peers);
				}
			}

			pMem->Free(this->Collection.Items);

			this->Collection.Count = 0;
		}
	}
	else {
		this->Locks.LockExclusive();
	}

	memset(&this->Collection, 0, sizeof(this->Collection));

	return this->Locks.UnlockExclusive(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CSocketPools::ReferencedSiteCount(SOCKETPOOL *pPool)
{
	this->Locks.LockShared();

	CWebSites *pWebSites = ((CWebSites *)this->pWebSites);

	int iReferences = 0;

	for(int i = 0; i < pWebSites->Collection.Count; i++)
	{
		if(pWebSites->Collection.WebSite[i]->pSocketPool == pPool)
		{
			iReferences++;
		}
	}

	return this->Locks.UnlockShared(iReferences);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CSocketPools::CurrentConnections(void *lpWebSite)
{
	this->Locks.LockShared();

	CWebSite *pWebSite = ((CWebSite *)lpWebSite);

	int iConnections = 0;

	for(int i = 0; i < pWebSite->pSocketPool->MaxConnections; i++)
	{
		if(pWebSite->pSocketPool->Peers && pWebSite->pSocketPool->Peers[i].pWebSite == lpWebSite)
		{
			iConnections++;
		}
	}

	return this->Locks.UnlockShared(iConnections);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Locate the pool which hosts the socket server.
*/
SOCKETPOOL *CSocketPools::Pool(CSocketServer *pSockerServer)
{
	this->Locks.LockShared();

	for(int i = 0; i < this->Collection.Count; i++)
	{
		if(this->Collection.Items[i].pSocket == pSockerServer)
		{
			return (SOCKETPOOL *)this->Locks.UnlockShared(&this->Collection.Items[i]);
		}
	}

	return (SOCKETPOOL *)this->Locks.UnlockShared((size_t)NULL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is called once per connection and matches a request up with a socket pool and website.

	Note: This function is NOT called per-request if the connection used
		keep-alive, but rather is called once per persistent connection.
*/
PEER *CSocketPools::InitializePeerStructure(CSocketServer *pSockerServer, CSocketClient *pClient, const char *sHostHeader)
{
	this->Locks.LockShared();

	CWebSites *pWebSites = ((CWebSites *)this->pWebSites);
	CWebSite *pResultingSite = NULL;
	CWebSite *pCatchAllSite = NULL;

	SOCKETPOOL *pPool = this->Pool(pSockerServer);
	if(!pPool)
	{
		return (PEER *)this->Locks.UnlockShared((size_t)NULL);
	}
	
	for(int s = 0; s < pWebSites->Collection.Count; s++) 
	{
		if(pWebSites->Collection.WebSite[s]->IsActive())
		{
			//Locate the site(s) which use the pool.
			if(pWebSites->Collection.WebSite[s]->pSocketPool == pPool)
			{
				CWebSite *pWebSite = pWebSites->Collection.WebSite[s];

				//If the site has no host headers then we will return it if we do not find a sutible match on the host headers
				if(pWebSite->pHostHeaders->Collection.Count == 0)
				{
					if(!pCatchAllSite)
					{
						pCatchAllSite = pWebSite;
					}
					else {
						//We have more than one catch-all site using the same pool - this is an invalid configuration.
						return (PEER *)this->Locks.UnlockShared((size_t)NULL);
					}
				}
				else if(sHostHeader != NULL)
				{
					//Check all of the host names associated with the site aganst the host which was parsed from the request header.
					for(int h = 0; h < pWebSite->pHostHeaders->Collection.Count; h++)
					{
						//Locate the proper site by host header.
						if(_strcmpi(pWebSite->pHostHeaders->Collection.Items[h].Name, sHostHeader) == 0)
						{
							pResultingSite = pWebSites->Collection.WebSite[s];
							break;
						}
					}
				}
			}
		}
		if(pResultingSite)
		{
			break;
		}
	}

	if(pCatchAllSite && !pResultingSite)
	{
		pResultingSite = pCatchAllSite; //Default site with no host headers.
	}
	
	if(pResultingSite)
	{
		if(!pResultingSite->pIPFilters->IsIPAllowed(pClient->PeerAddress()))
		{
			pResultingSite->pErrorPages->SendError(pClient, "403.6"); //IP Address Rejected
			return (PEER *)this->Locks.UnlockShared((size_t)NULL);
		}

		//Set connected client information.
		PEER *pClientInfo = &pPool->Peers[pClient->Slot()];
		memset(pClientInfo, 0, sizeof(PEER));
		pClientInfo->pWebSite = pResultingSite;
		pClientInfo->pClient = pClient;

		if(!pResultingSite->pCustomFilters->ProcessConnectFilters(pClient, pClient->PeerAddress()))
		{
			memset(pClientInfo, 0, sizeof(PEER));
			return (PEER *)this->Locks.UnlockShared((size_t)NULL);
		}

		pResultingSite->Stats.AcceptedConnections++;

		return (PEER *)this->Locks.UnlockShared(pClientInfo);
	}
	else {
		return (PEER *)this->Locks.UnlockShared((size_t)NULL);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SOCKETPOOL *CSocketPools::Pool(void *lpWebSite)
{
	this->Locks.LockShared();

	for(int i = 0; i < this->Collection.Count; i++)
	{
		if(_strcmpi(this->Collection.Items[i].UID, ((CWebSite*)lpWebSite)->SocketPoolID) == 0)
		{
			return (SOCKETPOOL *)this->Locks.UnlockShared(&this->Collection.Items[i]);
		}
	}

	return (SOCKETPOOL *)this->Locks.UnlockShared((size_t)NULL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSocketPools::Start(void *lpWebSite)
{
	SOCKETPOOL *pPool = this->Pool(lpWebSite);
	if(pPool)
	{
		pPool->pSocket->ClearListenAddresses();

		if(!pPool->ListenIPs.ListenOnAll)
		{
			int iEnabledIPAddresses = 0;
			for(int iIP = 0; iIP < pPool->ListenIPs.Count; iIP++)
			{
				if(pPool->ListenIPs.Items[iIP].Enabled)
				{
					pPool->pSocket->AddListenAddress(pPool->ListenIPs.Items[iIP].IP);
					iEnabledIPAddresses++;
				}
			}
			if(iEnabledIPAddresses > 0)
			{
				return pPool->pSocket->Start(pPool->Port);
			}
			else {
				((CWebSites*)this->pWebSites)->Trace("The website has no associated IP addresses.");
				return false;
			}
		}
		else{
			return pPool->pSocket->Start(pPool->Port);
		}

		return pPool->pSocket->Start();
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSocketPools::Stop(void *lpWebSite)
{
	SOCKETPOOL *pPool = this->Pool(lpWebSite);
	if(pPool)
	{
		int iReferences = 0;

		CWebSites *pWebSites = ((CWebSites *)this->pWebSites);

		//Disconnect all connections to the site.. very, very carefully.....
		pPool->pSocket->Lock();
		pPool->pSocket->PausePumpThread();

		//Pause the client threads for the referenced site. and set the connections to disconnect.
		for(int i = 0; i < pPool->MaxConnections; i++)
		{
			if(pPool->Peers[i].pClient && pPool->Peers[i].pWebSite == lpWebSite)
			{
				pPool->Peers[i].pClient->PauseClientThread();
				pPool->Peers[i].pClient->Disconnect();
			}
		}

		//Resume all of the client threads for the referenced site.
		for(int i = 0; i < pPool->MaxConnections; i++)
		{
			if(pPool->Peers[i].pClient && pPool->Peers[i].pWebSite == lpWebSite)
			{
				pPool->Peers[i].pClient->ResumeClientThread();
			}
		}

		pPool->pSocket->ResumePumpThread();
		pPool->pSocket->Unlock();

		//Look for other references to this socket pool, if we find some then we cannot stop the pool.
		for(int s = 0; s < pWebSites->Collection.Count; s++) 
		{
			if(pWebSites->Collection.WebSite[s] != lpWebSite && pWebSites->Collection.WebSite[s]->IsActive())
			{
				if(pWebSites->Collection.WebSite[s]->pSocketPool == pPool)
				{
					iReferences++;
				}
			}
		}

		if(iReferences == 0)
		{
			return pPool->pSocket->Stop(true);
		}
		else 
		{
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSocketPools::IsActive(void *lpWebSite)
{
	SOCKETPOOL *pPool = this->Pool(lpWebSite);
	if(pPool)
	{
		return pPool->pSocket->IsActive();
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CSocketPools::MaxConnections(void *lpWebSite)
{
	SOCKETPOOL *pPool = this->Pool(lpWebSite);
	if(pPool)
	{
		return pPool->MaxConnections;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CSocketPools::FileBufferSize(void *lpWebSite)
{
	SOCKETPOOL *pPool = this->Pool(lpWebSite);
	if(pPool)
	{
		return pPool->FileBufferSize;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CSocketPools::InitialBufferSize(void *lpWebSite)
{
	SOCKETPOOL *pPool = this->Pool(lpWebSite);
	if(pPool)
	{
		return pPool->InitialBufferSize;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CSocketPools::MaxBufferSize(void *lpWebSite)
{
	SOCKETPOOL *pPool = this->Pool(lpWebSite);
	if(pPool)
	{
		return pPool->MaxBufferSize;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CSocketPools::Port(void *lpWebSite)
{
	SOCKETPOOL *pPool = this->Pool(lpWebSite);
	if(pPool)
	{
		return pPool->Port;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char *CSocketPools::UID(void *lpWebSite)
{
	SOCKETPOOL *pPool = this->Pool(lpWebSite);
	if(pPool)
	{
		return pPool->UID;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char *CSocketPools::Description(void *lpWebSite)
{
	SOCKETPOOL *pPool = this->Pool(lpWebSite);
	if(pPool)
	{
		return pPool->Description;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
