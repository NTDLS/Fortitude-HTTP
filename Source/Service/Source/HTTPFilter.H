///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _HTTPFILTER_H
#define _HTTPFILTER_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DllImport extern "C" __declspec(dllimport)
#define DllExport extern "C" __declspec(dllexport)

#define FILTER_EVENT_RESULT_OK		1
#define FILTER_EVENT_RESULT_FAIL	2

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_FILTER_LOG_ENTRY {
	char *sDate;
	char *sTime;
	char *sSiteName;
	char *sSourceIP;
	char *sMethod; //GET, PUT, POST
	char *sAgent;
	char *sStatus;
	char *sAccept;
	char *sAcceptEncoding;
	char *sAcceptLanguage;
	char *sCacheControl;
	char *sConnection;
	char *sHost;
	char *sVersion;
	__int64 iContentLength; //Bytes
	char *sURI;
	char *sQuery;
	int iPort;
	double Duration;
} FILTERLOGENTRY, *LPFILTERLOGENTRY;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_FILTER_HTTP_HEADER {
	bool ModifiedByFilter; //The filter process should se this to true if the header has been modified.

	char *Method;
	char *Request; //This is the request made by the remote peer.
	char *FullRequest; //This is the request as translated by the server.
	char *Version;
	char *Query;
	char *ContentType;

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

	__int64 ContentBegin;
	__int64 ContentEnd;

	char *PostData;
	int PostDataSize;
} FILTERHTTPHEADER, *LPFILTERHTTPHEADER;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef unsigned int (*LPOnProcessRawResponseHeader)(unsigned int iPeerId, char *sResponseHeader, int iResponseHeaderLength);
typedef unsigned int (*LPOnProcessRequestHeader)(unsigned int iPeerId, FILTERHTTPHEADER *pHead);
typedef unsigned int (*LPOnUrlMap)(unsigned int iPeerId, FILTERHTTPHEADER *pHead);
typedef unsigned int (*LPOnSendRawData)(unsigned int iPeerId, const char *sBuf, unsigned int uiBufSz);
typedef unsigned int (*LPOnRecvRawData)(unsigned int iPeerId, const char *sBuf, unsigned int uiBufSz);
typedef unsigned int (*LPOnLog)(unsigned int iPeerId, FILTERLOGENTRY *pLFE);
typedef unsigned int (*LPOnConnect)(unsigned int iPeerId, const char *sIPAddress);
typedef unsigned int (*LPOnDisconnect)(unsigned int iPeerId);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DllExport unsigned int OnProcessRawResponseHeader(unsigned int iPeerId, char *sResponseHeader, int iResponseHeaderLength);
DllExport unsigned int OnProcessRequestHeader(unsigned int iPeerId, FILTERHTTPHEADER *pHead);
DllExport unsigned int OnUrlMap(unsigned int iPeerId, FILTERHTTPHEADER *pHead);
DllExport unsigned int OnSendRawData(unsigned int iPeerId, const char *sBuf, unsigned int uiBufSz);
DllExport unsigned int OnRecvRawData(unsigned int iPeerId, const char *sBuf, unsigned int uiBufSz);
DllExport unsigned int OnLog(unsigned int iPeerId, FILTERLOGENTRY *pLFE);
DllExport unsigned int OnConnect(unsigned int iPeerId, const char *sIPAddress);
DllExport unsigned int OnDisconnect(unsigned int iPeerId);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
