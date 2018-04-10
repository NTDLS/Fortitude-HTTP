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

#define FILTER_EVENT_RESULT_CONTINUE		1
#define FILTER_EVENT_RESULT_ABORT			2
#define FILTER_EVENT_RESULT_SENDERROR		3
#define FILTER_EVENT_RESULT_REDIRECT_TEMP	4
#define FILTER_EVENT_RESULT_REDIRECT_PERM	5

typedef int FILTERESULT;
typedef void FILTENORESULT;

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

typedef struct _tag_FILTER_ACTION {
	int ErrorCode;
	char *ErrorInformation;
	char *RedirectLocation;
} FILTERACTION, *LPFILTERACTION;

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

typedef unsigned int (*LPOnProcessRawResponseHeader)(unsigned int iPeerId, FILTERHTTPHEADER *pRequestHead, FILTERACTION *pFilterAction, char *sResponseHeader, int iResponseHeaderLength);
typedef unsigned int (*LPOnProcessRequestHeader)(unsigned int iPeerId, FILTERHTTPHEADER *pRequestHead, FILTERACTION *pFilterAction);
typedef unsigned int (*LPOnUrlMap)(unsigned int iPeerId, FILTERHTTPHEADER *pRequestHead, FILTERACTION *pFilterAction);
typedef unsigned int (*LPOnSendRawData)(unsigned int iPeerId, const char *sBuf, unsigned int uiBufSz, char **sModifiedBuf, unsigned int *uiModifiedBufSz);
typedef unsigned int (*LPOnRecvRawData)(unsigned int iPeerId, const char *sBuf, unsigned int uiBufSz);
typedef unsigned int (*LPOnCompressData)(unsigned int iPeerId, const char *sPathTranslated, const char *sBuf, unsigned int uiBufSz, char **sModifiedBuf, unsigned int *uiModifiedBufSz);
typedef unsigned int (*LPOnLog)(unsigned int iPeerId, FILTERLOGENTRY *pLFE);
typedef unsigned int (*LPOnConnect)(unsigned int iPeerId, const char *sIPAddress);
typedef void (*LPOnDisconnect)(unsigned int iPeerId);
typedef void (*LPOnRequestConcluded)(unsigned int iPeerId);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//DllExport unsigned int OnProcessRawResponseHeader(unsigned int iPeerId, FILTERHTTPHEADER *pRequestHead, FILTERACTION *pFilterAction, char *sResponseHeader, int iResponseHeaderLength);
//DllExport unsigned int OnProcessRequestHeader(unsigned int iPeerId, FILTERHTTPHEADER *pRequestHead, FILTERACTION *pFilterAction);
//DllExport unsigned int OnUrlMap(unsigned int iPeerId, FILTERHTTPHEADER *pRequestHead, FILTERACTION *pFilterAction);
//DllExport unsigned int OnSendRawData(unsigned int iPeerId, char *sBuf, unsigned int *uiBufSz);
//DllExport unsigned int OnRecvRawData(unsigned int iPeerId, const char *sBuf, unsigned int uiBufSz);
//DllExport unsigned int OnCompressData(unsigned int iPeerId, const char *sPathTranslated, const char *sBuf, unsigned int uiBufSz, char **sModifiedBuf, unsigned int *uiModifiedBufSz);
//DllExport unsigned int OnLog(unsigned int iPeerId, FILTERLOGENTRY *pLFE);
//DllExport unsigned int OnConnect(unsigned int iPeerId, const char *sIPAddress);
//DllExport void OnDisconnect(unsigned int iPeerId);
//DllExport void OnRequestConcluded(unsigned int iPeerId);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
