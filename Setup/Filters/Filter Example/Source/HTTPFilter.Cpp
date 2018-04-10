#ifndef _FILTER_CPP
#define _FILTER_CPP
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include "HTTPFilter.H"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	You can "uncomment" a few of the calls to MessageBox() to view some of the information available in each request.
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DllExport unsigned int OnProcessRequestHeader(unsigned int iPeerId, FILTERHTTPHEADER *pRequestHead)
{
	char sText[255];
	sprintf_s(sText, sizeof(sText),
		"PeerId: %d\r\n"
		"Method: %s\r\n"
		"Host: %s\r\n"
		"Request: %s",
		iPeerId, pRequestHead->Method, pRequestHead->Host, pRequestHead->Request);
	//MessageBox(NULL, sText, "OnProcessRequestHeader", 0);

	return FILTER_EVENT_RESULT_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DllExport unsigned int OnProcessRawResponseHeader(unsigned int iPeerId, FILTERHTTPHEADER *pRequestHead, char *sResponseHeader, int iResponseHeaderLength)
{
	char sText[255];
	sprintf_s(sText, sizeof(sText),
		"PeerId: %d\r\n"
		"Header: %s",
		iPeerId, sResponseHeader);
	//MessageBox(NULL, sText, "OnProcessRequestHeader", 0);

	return FILTER_EVENT_RESULT_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Notifies the filter when the server is mapping a logical URL to a physical path.
DllExport unsigned int OnUrlMap(unsigned int iPeerId, FILTERHTTPHEADER *pRequestHead)
{
	char sText[255];
	sprintf_s(sText, sizeof(sText),
		"PeerId: %d\r\n"
		"Request: %s\r\n"
		"FullRequest: %s",
		iPeerId, pRequestHead->Request, pRequestHead->FullRequest);
	//MessageBox(NULL, sText, "OnUrlMap", 0);

	/* //An example of filtering out all requests for a few image types:
	char sFileExtension[MAX_PATH];
	_splitpath_s(pRequestHead->FullRequest, NULL, 0, NULL, 0, NULL, 0, sFileExtension, sizeof(sFileExtension));

	if(_strcmpi(sFileExtension, ".jpg") == 0
		|| _strcmpi(sFileExtension, ".png") == 0
		|| _strcmpi(sFileExtension, ".gif") == 0)
	{
		return FILTER_EVENT_RESULT_ABORT; //Causes the server to send a "401 - Authorization failed by filter" response.
	}
	*/

	return FILTER_EVENT_RESULT_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Notifies the filter before raw data is sent from the server to the client.
DllExport unsigned int OnSendRawData(unsigned int iPeerId, const char *sBuf, unsigned int uiBufSz)
{
	char sText[255];
	sprintf_s(sText, sizeof(sText),
		"PeerId: %d\r\n"
		"%d Bytes Written.",
		iPeerId, uiBufSz);
	//MessageBox(NULL, sText, "OnSendRawData", 0);

	return FILTER_EVENT_RESULT_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Notifies the filter after raw data is sent from the client to the server, but before the server processes it.
DllExport unsigned int OnRecvRawData(unsigned int iPeerId, const char *sBuf, unsigned int uiBufSz)
{
	char sText[255];
	sprintf_s(sText, sizeof(sText),
		"PeerId: %d\r\n"
		"%d Bytes Read.",
		iPeerId, uiBufSz);
	//MessageBox(NULL, sText, "OnReadRawData", 0);

	return FILTER_EVENT_RESULT_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Logs information to a server file.
DllExport unsigned int OnLog(unsigned int iPeerId, FILTERLOGENTRY *pLFE)
{
	char sText[255];
	sprintf_s(sText, sizeof(sText),
		"PeerId: %d\r\n"
		"Method: %s\r\n"
		"Time: %s\r\n"
		"IP: %s",
		iPeerId, pLFE->sMethod, pLFE->sTime, pLFE->sSourceIP);
	//MessageBox(NULL, sText, "OnLog", 0);

	return FILTER_EVENT_RESULT_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Notifies the filter that the session is beginning.
DllExport unsigned int OnConnect(unsigned int iPeerId, const char *sIPAddress)
{
	char sText[255];
	sprintf_s(sText, sizeof(sText),
		"PeerId: %d\r\n"
		"IP Address: %s",
		iPeerId, sIPAddress);
	//MessageBox(NULL, sText, "OnConnect", 0);

	return FILTER_EVENT_RESULT_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Notifies the filter that the session is ending.
DllExport void OnDisconnect(unsigned int iPeerId)
{
	char sText[255];
	sprintf_s(sText, sizeof(sText),
		"PeerId: %d",
		iPeerId);
	//MessageBox(NULL, sText, "OnDisconnect", 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Notifies the filter that the request is ending.
DllExport void OnRequestConcluded(unsigned int iPeerId)
{
	char sText[255];
	sprintf_s(sText, sizeof(sText),
		"PeerId: %d",
		iPeerId);
	//MessageBox(NULL, sText, "OnRequestConcluded", 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
