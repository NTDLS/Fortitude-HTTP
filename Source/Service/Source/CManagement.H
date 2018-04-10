///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _Management_H
#define _Management_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../../@Libraries/CSocketServer/CSocketServer.H"
#include "../../../../@Libraries/CNASCCL (Stream)/CNASCCL.H"
#include "../../../../@Libraries/CXML/CXML.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_TEXT_SIZE		2048
#define MGMT_INITIAL_KEY	"Vq5qt61oVnUrs5My9ix9RUC56u82OiOW0jsO6WvnNXVNx14Ax624Oqdu6i4v"
#define MGMT_INITIAL_KEY_SZ	20

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CManagement {
	
public:
	~CManagement();
	CManagement();

	bool SendCommand(CSocketClient *pClient, const char *sText, ...);
	bool SendCommand(CSocketClient *pClient, LPBASICHUNK pReturnData, const char *sText, ...);
	bool SendCommand(CSocketClient *pClient, CXMLWriter *pXML);
	bool SendCommand(CSocketClient *pClient, CXMLReader *pXML);

	bool IsInitialized(void)
	{
		return this->bInitialized;
	}

protected:
	CSocketServer *Socket;
	CNASCCL Cytpto;
	bool bIsAuthorized;
	bool bInitialized;

	bool ProcessCommand(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);
	bool ProcessUrgentCommand(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);

	friend void Mgmt_Sock_OnSockError(CSocketServer *pSock, CSocketClient *pClient, int iErrorNumber, const char *sErrorMsg);
	friend void Mgmt_Sock_ClientThread(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);
	friend void MgMt_OnBeginClientThread(CSocketServer *pSock, CSocketClient *pClient, HANDLE hThread);
	friend void MgMt_OnEndClientThread(CSocketServer *pSock, CSocketClient *pClient);
	friend bool MgMt_OnAfterDePacketize(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);
	friend bool MgMt_OnBeforePacketize(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);
	friend bool MgMt_OnAccept(CSocketServer *pSock, CSocketClient *pClient);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

