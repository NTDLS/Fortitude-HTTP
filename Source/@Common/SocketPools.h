///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _SocketPools_Types_H
#define _SocketPools_Types_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../@Common/Constants.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SOCKETPOOLS_IPS_LIST_POS_IP			0
#define SOCKETPOOLS_IPS_LIST_POS_DESCR		1

#define SOCKETPOOLS_LIST_POS_PORT			0
#define SOCKETPOOLS_LIST_POS_MAXCONNECTIONS	1
#define SOCKETPOOLS_LIST_POS_INITBUFFERSIZE	2
#define SOCKETPOOLS_LIST_POS_MAXBUFFERSIZE	3
#define SOCKETPOOLS_LIST_POS_FILEBUFFERSIZE	4
#define SOCKETPOOLS_LIST_POS_DESCR			5
#define SOCKETPOOLS_LIST_POS_UID			6
#define SOCKETPOOLS_LIST_POS_REFERENCES		7

#define SOCKETPOOLS_MAX_DESCR_LENGTH		1024
#define SOCKETPOOLS_MAX_UID_LENGTH			40
#define SOCKETPOOLS_IPS_MAX_IP_LENGTH		16
#define SOCKETPOOLS_IPS_MAX_DESCR_LENGTH	1024

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Socket_Pool_Listen_IP{
	char *IP;
	char *Description;
	bool Enabled;
} LISTENIP, *LPLISTENIP;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Socket_Pool_Listen_IPs{
	int Count;
	LISTENIP *Items;
	bool ListenOnAll;
} LISTENIPS, *LPLISTENIPS;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef HTTP_SERVICE
typedef struct _tag_Last_Response {
	char ResponseCode[MAX_RESPONSE_CODE];
	__int64 ContentLength;
} LASTRESPONSE, *LPLASTRESPONSE;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef HTTP_SERVICE
typedef struct _tag_HTTP_Header {
	char *Method;
	char *Request;
	char *Version;
	char *Query;
	char *ContentType;

	char *ScriptName; //SCRIPT_NAME environment variable.
	char *PathInfo; //PATH_INFO environment variable.

	char *Accept;
	char *AcceptLanguage;
	char *AcceptEncoding;
	char *UserAgent;
	char *Host;
	char *Connection;
	char *CacheControl;
	char *Referer;
	char *Cookie;
	char *ContentRange;
	char *Status;
	SYSTEMTIME IfModifiedSince;

	char *FullRequest;
	char *PathTranslated;

	//Used for Custom Error Pages with Scripting (BEGIN)
	char *OriginalPathInfo;
	char *OriginalFullRequest;
	char *OriginalRequest;
	char *OriginalPathTranslated;
	char *OriginalScriptName;
	//Used for Custom Error Pages with Scripting (END)

	char *AuthenticationRealm;

	__int64 ContentBegin;
	__int64 ContentEnd;

	char *PostData;
	int PostDataSize;
} HTTPHEADER, *LPHTTPHEADER;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef HTTP_SERVICE
typedef struct _tag_PerRequest {
	bool IsDynamicContent;
	bool HTTPErrorOccured;
	LASTRESPONSE LastResponse;
} PERREQUESTSTORE, *LPPERREQUESTSTORE;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef HTTP_SERVICE
typedef struct _tag_Client {
	HTTPHEADER Header; //Cleared in FreeClientHeader(PEER *pC);
	CSocketClient *pClient;
	bool KeepAlive;
	PERREQUESTSTORE PerRequestStore; //This is zeroed in Socket_ClientHandler().
	VOID *pWebSite; //CWebSite*
} PEER, *LPPEER; //This structure is setup via ::InitializePeerStructure().
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Socket_Pool{

	int Port;
	int MaxConnections;
	int InitialBufferSize;
	int MaxBufferSize;
	int FileBufferSize;
	char *Description;
	char *UID; //the ID of the socket pool - used to cross-reference websites.

	LISTENIPS ListenIPs;

#ifndef HTTP_SERVICE
	bool IsActive;
	int ReferencedSites;
#endif

#ifdef HTTP_SERVICE
	CSocketServer *pSocket; //CSocketServer*.
	PEER *Peers; //PEER[]
#endif

} SOCKETPOOL, *LPSOCKETPOOL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Socket_Pools{
	int Count;
	SOCKETPOOL *Items;
} SOCKETPOOLS, *LPSOCKETPOOLS;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
