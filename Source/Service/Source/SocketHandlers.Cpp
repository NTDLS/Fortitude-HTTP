///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _SOCKETS_CPP
#define _SOCKETS_CPP
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::DateTime;
using namespace NSWFL::String;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Socket_SendHandler(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	CWebSite *pWebSite = (CWebSite *)pClient->UserData();
	if(pWebSite)
	{
		pWebSite->Stats.BytesSent += pChunk->iLength;

		return(pWebSite->pCustomFilters->ProcessSendFilters(pClient, pChunk) == FILTER_EVENT_RESULT_CONTINUE);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Socket_ConsumeHandler(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	CWebSite *pWebSite = (CWebSite *)pClient->UserData();
	if(pWebSite)
	{
		pWebSite->Stats.BytesRecv += pChunk->iLength;
		return (pWebSite->pCustomFilters->ProcessRecvFilters(pClient, (const char *)pChunk->pBuffer, pChunk->iLength) == FILTER_EVENT_RESULT_CONTINUE);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Socket_AcceptHandler(CSocketServer *pSock, CSocketClient *pClient)
{
	pClient->UserData(NULL);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Socket_EndClientThreadHandler(CSocketServer *pSock, CSocketClient *pClient)
{
	pClient->UserData(NULL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Highlevel_Process: Each request will receive a worker thread, this is the worker thread proc which handles those connections.
void Socket_ClientHandler(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	CWebSites *pWebSites = gWebSites; //The CWebSites class pointer.

	int iSlot = pClient->Slot();
	PEER *pC = NULL;
	CWebSite *pWebSite = NULL;
	int iKeepAliveTimeout = 10000; //Default timeout, amount of time to wait for data before disconnecting (even if not using keep alive).
	int iTimeoutCount = 0;
	HANDLE hSiteToken = NULL;
	bool bIsInitialConnection = true;

	pWebSites->IncrementConnectionCount();

	SetThreadPriorityBoost(GetCurrentThread(), !pWebSites->pServerSettings->Settings.Processor.ThreadBoost);

	HighPrecisionTimer duration;
	bool bProcessedARequest = false;

	while(pClient->IsConnected())
    {
		if(bProcessedARequest && pWebSite != NULL && pC != NULL)
		{
			pWebSite->pCustomFilters->ProcessRequestConcludedFilters(pC);
		}

		bProcessedARequest = false;

		//Highlevel_Process: Receive the header from the remote peer.
		if(pClient->GetClientData(pChunk))
		{
			bProcessedARequest = true;

			duration.Start();

			unsigned long ulStartTime = GetTickCount();
			LOGFILEENTRY LFE;
			memset(&LFE, 0, sizeof(LFE));

			Get_ShortDate(LFE.sDate, sizeof(LFE.sDate));
			Get_Time(LFE.sTime, sizeof(LFE.sTime), 0);

			StringBuilder header((const char *)pChunk->pBuffer, pChunk->iLength);
			
			if(bIsInitialConnection)
			{
				bIsInitialConnection = false;
				char *sHostHeader = NULL;
				if(pWebSites->pHttp->GetHttpHeaderTag((const char *)pChunk->pBuffer, pChunk->iLength, "Host:", sHostHeader, NULL) > 0)
				{
					ReplaceCharacter(sHostHeader, ':', '\0');
				}

				pC = pWebSites->pSocketPools->InitializePeerStructure(pSock, pClient, sHostHeader);

				if(sHostHeader) pMem->Free(sHostHeader);

				if(!pC)
				{
					//There are no acceptible websites associated with the current socket pool.
					break;
				}

				pWebSite = (CWebSite *)pC->pWebSite;

				//We couldn't track the received data initially because we didnt
				//	have a website when the data was received - so do it now.
				pClient->UserData(pC->pWebSite);
				if(!Socket_ConsumeHandler(pSock, pClient, pChunk))
				{
					break;
				}

				//Default timeout, amount of time to wait for data before disconnecting (even if not using keep alive).
				iKeepAliveTimeout = pWebSite->pWebsiteSettings->KeepAliveTimeout() * 1000;
				iTimeoutCount = 0;

				//---BEGIN---------------(Get the Login Information that will be used to access resources)-------------------
				if(strlen(pWebSite->pWebsiteSettings->Username()) > 0)
				{
					HANDLE hToken = NULL;
					char *sDomain = NULL;
					char sErrorMessage[1024];

					if(strlen(pWebSite->pWebsiteSettings->Domain()) > 0)
					{
						sDomain = pWebSite->pWebsiteSettings->Domain();
					}

					if(!(hSiteToken = NTLogin(pWebSite->pWebsiteSettings->Username(),
						sDomain, pWebSite->pWebsiteSettings->Password(), sErrorMessage, sizeof(sErrorMessage))))
					{
						pWebSite->pErrorPages->SendError(pC, "500", sErrorMessage);
						pClient->WaitOnDataToBeSent();
						return;
					}
				}
				else {
					HANDLE hProcess = GetCurrentProcess();

					if(!OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hSiteToken))
					{
						char sErrorMessage[1024];
						if(GetLastError(sErrorMessage, sizeof(sErrorMessage)))
						{
							pWebSite->pErrorPages->SendError(pC, "500", sErrorMessage);
						}
						else{
							pWebSite->pErrorPages->SendError(pC, "500", "Unknown original logon error.");
						}

						pClient->WaitOnDataToBeSent();
						return;
					}
				}
				//---END---------------(Get the Login Information that will be used to access resources)-------------------
			}
			else {
				//If this is not a new connection, then we need to be sure to clean up any per-request information.
				memset(&pC->PerRequestStore, 0, sizeof(pC->PerRequestStore));
			}

			//If the received header contains a content-length then we need to receive the header in multiple chunks.
			char sHeaderContentLength[16];
			if(pWebSites->pHttp->GetHttpHeaderTag((const char *)pChunk->pBuffer, pChunk->iLength, "Content-Length:", sHeaderContentLength, sizeof(sHeaderContentLength), NULL) > 0)
			{
				char sContentType[15];
				int iHeaderContentTypeSz = pWebSites->pHttp->GetHttpHeaderTag((const char *)pChunk->pBuffer, pChunk->iLength, "Content-Type:", sContentType, sizeof(sContentType), true, NULL);
				//Question: Should we look at other content types other than "multipart/*" when considering receving the header in chunks?
				//Answer: No. The content type "application/x-www-form-urlencoded" is inefficient for sending large quantities of
				//	binary data or text containing non-ASCII characters. The content type "multipart/form-data" should be used
				//	for submitting forms that contain files, non-ASCII data, and binary data.

				int iMaxReceiveSize = pWebSite->pWebsiteSettings->MaxReceiveSize();
				int iHeaderContentLength = atol(sHeaderContentLength);

				if(iHeaderContentLength >= iMaxReceiveSize)
				{
					pWebSite->pErrorPages->SendError(pC, "413");
					break;
				}

				if(iHeaderContentTypeSz > 0 &&
					(_strnicmp(sContentType, "multipart/", 10) == 0 /*|| strnicmp(sContentType, "application/", 12) == 0*/))
				{
					//FIXFIX: Should we be looking for the size of the content as opposed to the full header size?
					//	Example: we have (header.Length < iHeaderContentLength), should it be ("Header content".Length() < iHeaderContentLength)?
					while(header.Length < iHeaderContentLength && header.Length <= iMaxReceiveSize
						&& (GetTickCount() - ulStartTime) < (unsigned int)iKeepAliveTimeout && pClient->IsConnected())
					{
						if(pClient->GetClientData(pChunk))
						{
							header.Append((const char *)pChunk->pBuffer, pChunk->iLength);
						}
						Sleep(1);
					}
				}

				if(header.Length >= iMaxReceiveSize)
				{
					pWebSite->pErrorPages->SendError(pC, "413");
					break;
				}
			}

			//Reset the threads impersonation token, as it may have been changed by a previous request.
			if(hSiteToken)
			{
				if(!ImpersonateLoggedOnUser(hSiteToken))
				{
					char sErrorMessage[1024];
					if(GetLastError(sErrorMessage, sizeof(sErrorMessage)))
					{
						if(pWebSite) pWebSite->pErrorPages->SendError(pC, "500", sErrorMessage);
					}
					else{
						if(pWebSite) pWebSite->pErrorPages->SendError(pC, "500", "Unknown original logon error");
					}

					CloseHandle(hSiteToken);
					break;
				}
			}

			bool bRequestResult = false;
			//Highlevel_Process: Parse the received header, store all results in pC.
			bool bParseHTTPHeaderResult = pWebSites->pHttp->ParseHTTPHeader(pWebSite, pC, header.Buffer, header.Length);

			header.Destroy(); //This data is really not needed anymore, just free it early...

			//-------------------------------------------------------------------------------------------
			//-( Begin Log File Write )------------------------------------------------------------------
			//-------------------------------------------------------------------------------------------
			LFE.iPort = pWebSite->Port();
			LFE.sAgent = pC->Header.UserAgent;
			LFE.sMethod = pC->Header.Method;
			LFE.sQuery = pC->Header.Query;
			LFE.sSiteName = pWebSite->Name;
			strcpy_s(LFE.sSourceIP, sizeof(LFE.sSourceIP), pClient->PeerAddress());
			LFE.sStatus = pC->PerRequestStore.LastResponse.ResponseCode;
			LFE.iContentLength = pC->PerRequestStore.LastResponse.ContentLength;
			//strcpy_s(LFE.sTargetIP, sizeof(LFE.sTargetIP), "*.*.*.*");
			LFE.sURI = pC->Header.Request;
			LFE.sAccept = pC->Header.Accept;
			LFE.sAcceptEncoding = pC->Header.AcceptEncoding;
			LFE.sAcceptLanguage = pC->Header.AcceptLanguage;
			LFE.sCacheControl = pC->Header.CacheControl;
			LFE.sConnection = pC->Header.Connection;
			LFE.sHost = pC->Header.Host;
			LFE.sVersion = pC->Header.Version;
			LFE.Duration = 0;

			if(bParseHTTPHeaderResult)
			{
				//Highlevel_Process: Process the request, sending the appropriate data to the connected remote peer.
				bRequestResult = pWebSites->pHttp->ProcessRequest(pWebSite, pC);

				pClient->WaitOnDataToBeSent();

				duration.Stop();

				//-------------------------------------------------------------------------------------------
				//-( Begin Log File Write - It may have changed )--------------------------------------------
				//-------------------------------------------------------------------------------------------
				LFE.iPort = pWebSite->Port();
				LFE.sAgent = pC->Header.UserAgent;
				LFE.sMethod = pC->Header.Method;
				LFE.sQuery = pC->Header.Query;
				LFE.sSiteName = pWebSite->Name;
				strcpy_s(LFE.sSourceIP, sizeof(LFE.sSourceIP), pClient->PeerAddress());
				LFE.sStatus = pC->PerRequestStore.LastResponse.ResponseCode;
				LFE.iContentLength = pC->PerRequestStore.LastResponse.ContentLength;
				//strcpy_s(LFE.sTargetIP, sizeof(LFE.sTargetIP), "*.*.*.*");
				LFE.sURI = pC->Header.Request;
				LFE.sAccept = pC->Header.Accept;
				LFE.sAcceptEncoding = pC->Header.AcceptEncoding;
				LFE.sAcceptLanguage = pC->Header.AcceptLanguage;
				LFE.sCacheControl = pC->Header.CacheControl;
				LFE.sConnection = pC->Header.Connection;
				LFE.sHost = pC->Header.Host;
				LFE.sVersion = pC->Header.Version;
				LFE.Duration = duration.Calculate();

				FILTERLOGENTRY FLE;
				memset(&FLE, 0, sizeof(FLE));
				FLE.sDate           = LFE.sDate;
				FLE.sTime           = LFE.sTime;
				FLE.sSiteName       = LFE.sSiteName;
				FLE.sSourceIP       = LFE.sSourceIP;
				FLE.sMethod         = LFE.sMethod;
				FLE.sAgent          = LFE.sAgent;
				FLE.sStatus         = LFE.sStatus;
				FLE.sAccept         = LFE.sAccept;
				FLE.sAcceptEncoding = LFE.sAcceptEncoding;
				FLE.sAcceptLanguage = LFE.sAcceptLanguage;
				FLE.sCacheControl   = LFE.sCacheControl;
				FLE.sConnection     = LFE.sConnection;
				FLE.sHost           = LFE.sHost;
				FLE.sVersion        = LFE.sVersion;
				FLE.iContentLength  = LFE.iContentLength;
				FLE.sURI            = LFE.sURI;
				FLE.sQuery          = LFE.sQuery;
				FLE.iPort           = LFE.iPort;
				FLE.Duration		= LFE.Duration;

				pWebSite->pCustomFilters->ProcessLogFilters(pC, &FLE);
				
				//-------------------------------------------------------------------------------------------
				//-( End Log File Write )--------------------------------------------------------------------
				//-------------------------------------------------------------------------------------------
			}

			pWebSite->WriteLog(&LFE);

			pWebSites->pHttp->FreeClientHeader(pC);

			if(!bRequestResult)
			{
				break;
			}
			else{
				if(!pC->KeepAlive)
				{
					break;
				}
				else{
					iTimeoutCount = 0; //Reset the keep-alive timeout.
				}
			}
		}

		if(iTimeoutCount++ >= iKeepAliveTimeout)
		{
			break;
		}

		Sleep(1);
    }

	if(hSiteToken)
	{
		CloseHandle(hSiteToken);
	}

	pClient->WaitOnDataToBeSent();

	if(pC)
	{
		pWebSite->pCustomFilters->ProcessDisconnectFilters(pClient);
		memset(pC, 0, sizeof(PEER));
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Socket_ErrorHandler(CSocketServer *pSock, CSocketClient *pClient, int iErrorNumber, const char *sErrorMsg)
{
	gWebSites->Trace(sErrorMsg);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
