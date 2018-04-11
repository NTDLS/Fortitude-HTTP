///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _SOCKET_H
#define _SOCKET_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../CService/CService.H"
#include "../../../NSWFL/NSWFL.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Socket_ClientHandler(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);
void Socket_ErrorHandler(CSocketServer *lpCSocketServer, CSocketClient *pClient, int iErrorNumber, const char *sErrorMsg);
bool Socket_SendHandler(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);
bool Socket_ConsumeHandler(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);
void Socket_EndClientThreadHandler(CSocketServer *pSock, CSocketClient *pClient);
bool Socket_AcceptHandler(CSocketServer *pSock, CSocketClient *pClient);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

