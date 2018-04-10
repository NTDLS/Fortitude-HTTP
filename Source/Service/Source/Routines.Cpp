///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _ROUTINES_CPP
#define _ROUTINES_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <WindowsX.H>
#include <ShellAPI.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <ShlObj.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"
#include "Routines.H"
#include "../../../../@Libraries/CStack/CStack.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::DateTime;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GetLastError(char *sOutErrorMessage, int iMaxOutErrorMessage, const char *sPrefix, const char *sSuffix)
{
	if(GetLastError(sOutErrorMessage, iMaxOutErrorMessage))
	{
		int iAllocate = (int)strlen(sOutErrorMessage) + 1;
		char *sTemp = (char *)pMem->Allocate(sizeof(char), iAllocate);
		strcpy_s(sTemp, iAllocate, sOutErrorMessage);
		Trim(sTemp, iAllocate - 1);

		strcpy_s(sOutErrorMessage, iMaxOutErrorMessage, sPrefix);
		strcat_s(sOutErrorMessage, iMaxOutErrorMessage, sTemp);
		strcat_s(sOutErrorMessage, iMaxOutErrorMessage, sSuffix);
		pMem->Free(sTemp);
		return true;
	}
	else{
		strcpy_s(sOutErrorMessage, iMaxOutErrorMessage, sPrefix);
		strcat_s(sOutErrorMessage, iMaxOutErrorMessage, sSuffix);
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GetLastError(char *sOutErrorMessage, int iMaxOutErrorMessage, const char *sPrefix)
{
	if(GetLastError(sOutErrorMessage, iMaxOutErrorMessage))
	{
		int iAllocate = (int)strlen(sOutErrorMessage) + 1;
		char *sTemp = (char *)pMem->Allocate(sizeof(char), iAllocate);
		strcpy_s(sTemp, iAllocate, sOutErrorMessage);
		Trim(sTemp, iAllocate - 1);

		strcpy_s(sOutErrorMessage, iMaxOutErrorMessage, sPrefix);
		strcat_s(sOutErrorMessage, iMaxOutErrorMessage, sTemp);
		pMem->Free(sTemp);
		return true;
	}
	else{
		strcpy_s(sOutErrorMessage, iMaxOutErrorMessage, sPrefix);
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Retreives a textual error message from a threads GetLastError() call.
	Returns TRUE if error information was received.
*/
bool GetLastError(char *sOutErrorMessage, int iMaxOutErrorMessage)
{
	DWORD dwLastError = GetLastError();
	if(dwLastError)
	{
		char *sErrorMessage = NULL;

		if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, dwLastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&sErrorMessage, 0, NULL))
		{
			strcpy_s(sOutErrorMessage, iMaxOutErrorMessage, sErrorMessage);
			LocalFree(sErrorMessage);
			return true;
		}

		strcpy_s(sOutErrorMessage, iMaxOutErrorMessage, "Unknown error.");
		return true;
	}
	return false;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Retreives a textual error message from a threads GetLastError() call.
	Returns TRUE if error information was received.
*/
bool GetLastError(CStringBuilder *outError)
{
	DWORD dwLastError = GetLastError();
	if(dwLastError)
	{
		char *sErrorMessage = NULL;

		if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, dwLastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&sErrorMessage, 0, NULL))
		{
			outError->Set(sErrorMessage);
			LocalFree(sErrorMessage);
			return true;
		}

		strcpy_s(outError->Buffer, outError->Alloc, "Unknown error.");
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HANDLE NTLogin(const char *sUsername, const char *sDomain, const char *sPassword, char *sOutErrorMessage, int iMaxOutErrorMessage)
{
	HANDLE hToken = NULL;

	/*
	#define LOGON32_LOGON_INTERACTIVE       2
	#define LOGON32_LOGON_NETWORK           3
	#define LOGON32_LOGON_BATCH             4
	#define LOGON32_LOGON_SERVICE           5
	#define LOGON32_LOGON_UNLOCK            7
	#define LOGON32_LOGON_NETWORK_CLEARTEXT 8
	#define LOGON32_LOGON_NEW_CREDENTIALS   9
	*/

	if(!LogonUser(sUsername, sDomain, sPassword,
		LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hToken))
	{
		if(!GetLastError(sOutErrorMessage, iMaxOutErrorMessage))
		{
			strcpy_s(sOutErrorMessage, iMaxOutErrorMessage, "Unknown logon error.");
		}

		return NULL;
	}
	return hToken;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BreakGMTTimeString(char *sGMT, SYSTEMTIME *pST)
{
	/*
	http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html#sec3.3.1
	3.3.1 Full Date

	Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
	Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
	Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format

	HTTP/1.1 clients and servers that parse the date value MUST accept all three formats
	(for compatibility with HTTP/1.0), though they MUST only generate the RFC 1123
	format for representing HTTP-date values in header fields.
	*/
	
	const char *sep = ", :-";
	const int sepSz = (int) strlen(sep);

	bool bIsRFC = true;

	char sToken[64];
	int iGMTSz = (int)strlen(sGMT);
	int iRPos = 0;

	memset(pST, 0, sizeof(SYSTEMTIME));

	//Get weekday:
	if(GetNextToken(sGMT, iGMTSz, sToken, sizeof(sToken), &iRPos, sep, sepSz) > 0)
	{
		if((pST->wDayOfWeek = (WORD)GetWeekdayNumber("sun")) <= 0)
		{
			return false;
		}
	}
	else {
		return false;
	}
	if(IsCharInArrayS(sGMT[iRPos], sep, sepSz)) iRPos++; //Skip seperator.
	SkipWhiteSpaces(sGMT, iGMTSz, &iRPos);

	//Get month-day or month name:
	if(GetNextToken(sGMT, iGMTSz, sToken, sizeof(sToken), &iRPos, sep, sepSz) > 0)
	{
		if(!IsNumeric(sToken))
		{
			if((pST->wMonth = GetMonthNumber(sToken)) <= 0)
			{
				return false;
			}
			bIsRFC = false; //ANSI C's asctime() format
		}
		else {
			if((pST->wDay = (WORD)atol(sToken)) <= 0)
			{
				return false;
			}
		}
	}
	else {
		return false;
	}
	if(IsCharInArrayS(sGMT[iRPos], sep, sepSz)) iRPos++; //Skip seperator.
	SkipWhiteSpaces(sGMT, iGMTSz, &iRPos);

	if(bIsRFC)
	{
		//Get month name:
		if(GetNextToken(sGMT, iGMTSz, sToken, sizeof(sToken), &iRPos, sep, sepSz) > 0)
		{
			if((pST->wMonth = GetMonthNumber(sToken)) <= 0)
			{
				return false;
			}
		}
		else {
			return false;
		}
		if(IsCharInArrayS(sGMT[iRPos], sep, sepSz)) iRPos++; //Skip seperator.
		SkipWhiteSpaces(sGMT, iGMTSz, &iRPos);

		//Get year:
		if(GetNextToken(sGMT, iGMTSz, sToken, sizeof(sToken), &iRPos, sep, sepSz) > 0)
		{
			if((pST->wYear = (WORD)atol(sToken)) <= 0)
			{
				return false;
			}
			//Convert 2 digit year to 4 digit year.
			if(pST->wYear < 100)
			{
				pST->wYear += 1900;
			}
		}
		else {
			return false;
		}
		if(IsCharInArrayS(sGMT[iRPos], sep, sepSz)) iRPos++; //Skip seperator.
		SkipWhiteSpaces(sGMT, iGMTSz, &iRPos);

	}
	else {
		//Get month-day:
		if(GetNextToken(sGMT, iGMTSz, sToken, sizeof(sToken), &iRPos, sep, sepSz) > 0)
		{
			if((pST->wDay = (WORD)atol(sToken)) <= 0)
			{
				return false;
			}
		}
		else {
			return false;
		}
		if(IsCharInArrayS(sGMT[iRPos], sep, sepSz)) iRPos++; //Skip seperator.
		SkipWhiteSpaces(sGMT, iGMTSz, &iRPos);
	}

	sscanf_s(sGMT + iRPos, "%d:%d:%d", (int *)&pST->wHour, (int *)&pST->wMinute, (int *)&pST->wSecond);

	if(!bIsRFC)
	{
		if(IsCharInArrayS(sGMT[iRPos], sep, sepSz)) iRPos++; //Skip seperator.
		SkipWhiteSpaces(sGMT, iGMTSz, &iRPos);
		while(sGMT[iRPos] != ' ' && iRPos < iGMTSz)
		{			
			iRPos++;
		}
		if(IsCharInArrayS(sGMT[iRPos], sep, sepSz)) iRPos++; //Skip seperator.
		SkipWhiteSpaces(sGMT, iGMTSz, &iRPos);

		//Get year:
		if(GetNextToken(sGMT, iGMTSz, sToken, sizeof(sToken), &iRPos, sep, sepSz) > 0)
		{
			if((pST->wYear = (WORD)atol(sToken)) <= 0)
			{
				return false;
			}
			//Convert 2 digit year to 4 digit year.
			if(pST->wYear < 100)
			{
				pST->wYear += 1900;
			}
		}
		else {
			return false;
		}
		if(IsCharInArrayS(sGMT[iRPos], sep, sepSz)) iRPos++; //Skip seperator.
		SkipWhiteSpaces(sGMT, iGMTSz, &iRPos);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MakeGMTTimeString(SYSTEMTIME *pST, char *sGMT, int iMaxSz)
{
	/*
		http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html#sec3.3.1
		3.3.1 Full Date

		Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
		Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
		Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format

		HTTP/1.1 clients and servers that parse the date value MUST accept all three formats
		(for compatibility with HTTP/1.0), though they MUST only generate the RFC 1123
		format for representing HTTP-date values in header fields.	
	*/

	char sTime[64];
	char sDate[64];
	
	GetTimeFormat(LOCALE_SYSTEM_DEFAULT, NULL, pST, "HH':'mm':'ss 'GMT'", sTime, 64);
	GetDateFormat(LOCALE_SYSTEM_DEFAULT, NULL, pST, "ddd\',\' dd MMM yyyy", sDate, 64);
	
	strcpy_s(sGMT, iMaxSz, sDate);
	strcat_s(sGMT, iMaxSz, " ");
	strcat_s(sGMT, iMaxSz, sTime);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MakeGMTTimeString(char *sGMT, int iMaxSz)
{
	SYSTEMTIME st;
	memset(&st, 0, sizeof(st));
	GetSystemTime(&st);
	
	return MakeGMTTimeString(&st, sGMT, iMaxSz);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GetLastFileModificationDateGMT(const char *sFile, SYSTEMTIME *pST)
{
    HANDLE hFile = CreateFile(sFile, 0, 0, NULL, OPEN_EXISTING, 0, NULL);

	FILETIME ftLastModTimeGMT;
	FILETIME ftLastModTimeLocal;

	memset(&ftLastModTimeGMT, 0, sizeof(ftLastModTimeGMT));
	memset(pST, 0, sizeof(SYSTEMTIME));

	if(hFile != INVALID_HANDLE_VALUE)
	{
		if(GetFileTime(hFile, NULL, NULL, &ftLastModTimeGMT))
		{
			CloseHandle(hFile);

			if(FileTimeToLocalFileTime(&ftLastModTimeGMT, &ftLastModTimeLocal))
			{
				if(FileTimeToSystemTime(&ftLastModTimeLocal, pST))
				{
					pST->wMilliseconds = 0;
					return true;
				}
			}
		}
	}

	CloseHandle(hFile);

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GetLastFileModificationDate(const char *sFile, char *sLocalTime, int iMaxSz)
{
    HANDLE hFile = CreateFile(sFile, 0, 0, NULL, OPEN_EXISTING, 0, NULL);

	FILETIME ftLastModTimeGMT;
	FILETIME ftLastModTimeLocal;
	SYSTEMTIME ST;

	memset(&ftLastModTimeGMT, 0, sizeof(ftLastModTimeGMT));
	memset(&ST, 0, sizeof(ST));

	if(hFile != INVALID_HANDLE_VALUE)
	{
		if(GetFileTime(hFile, NULL, NULL, &ftLastModTimeGMT))
		{
			CloseHandle(hFile);

			if(FileTimeToLocalFileTime(&ftLastModTimeGMT, &ftLastModTimeLocal))
			{
				if(FileTimeToSystemTime(&ftLastModTimeLocal, &ST))
				{
					return GetLOCALTime(&ST, sLocalTime, iMaxSz);
				}
			}
		}
	}

	CloseHandle(hFile);

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GetLOCALTime(char *sLocalTime, int iMaxSz)
{
	SYSTEMTIME ST;

	memset(&ST, 0, sizeof(ST));

	GetLocalTime(&ST);

	return GetLOCALTime(&ST, sLocalTime, iMaxSz);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GetLOCALTime(LPSYSTEMTIME lpST, char *sLocalTime, int iMaxSz)
{
	char sTime[64];
	char sDate[64];
	char sStdName[32];
	char sStdAbbreviation[32];

	size_t iConverted = 0;

	TIME_ZONE_INFORMATION TZI;

	GetTimeZoneInformation(&TZI);
	
	GetTimeFormat(LOCALE_SYSTEM_DEFAULT, NULL, lpST, "HH':'mm':'ss", sTime, 64);
	GetDateFormat(LOCALE_SYSTEM_DEFAULT, NULL, lpST, "ddd\',\' dd MMM yyyy", sDate, 64);

	wcstombs_s(&iConverted, sStdName, sizeof(sStdName), TZI.StandardName, 32);

	Abbreviate(sStdName, sStdAbbreviation, sizeof(sStdName));

	strcpy_s(sLocalTime, iMaxSz, sDate);
	strcat_s(sLocalTime, iMaxSz, " ");
	strcat_s(sLocalTime, iMaxSz, sTime);
	strcat_s(sLocalTime, iMaxSz, " ");
	strcat_s(sLocalTime, iMaxSz, sStdAbbreviation);

	//sLocalTime: Thu, 08 Nov 2004 04:04:16 EST

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GetNextToken(const char *sBuf, int iBufSz, char *&sOutToken, int *iInOutRPos)
{
	int iRPos = *iInOutRPos;
	int iWPos = 0;
	int iAlloc = 0;

	SkipWhiteSpaces(sBuf, iBufSz, &iRPos);

	while(!IsWhiteSpace(sBuf[iRPos]) && iRPos < iBufSz)
	{
		iRPos++;
		iAlloc++;
	}

	sOutToken = (char *) pMem->Allocate(sizeof(char), iAlloc + 1);

	iRPos -= iAlloc;

	if(sOutToken)
	{
		for(iWPos = 0; iWPos < iAlloc; iWPos++)
		{
			sOutToken[iWPos] = sBuf[iRPos++];
		}

		sOutToken[iWPos] = '\0';
	}

	SkipWhiteSpaces(sBuf, iBufSz, &iRPos);

	*iInOutRPos = iRPos;

	return iWPos; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsValidIPAddressV4(const char *sIP)
{
	char **sIPs = Split(sIP, ".");

	if(UBound(sIPs) != 4)
	{
		FreeArray(sIPs);
		return false;
	}

	int iOctLen = 0;

	for(unsigned short iOct = 0; iOct < 4; iOct++)
	{
		if((iOctLen = (int)strlen(sIPs[iOct])) > 3)
		{
			FreeArray(sIPs);
			return false;
		}

		for(int iPos = 0; iPos < iOctLen; iPos++)
		{
			if(!IsNumeric(sIPs[iOct][iPos]))
			{
				FreeArray(sIPs);
				return false;
			}
		}

		if(atol(sIPs[iOct]) > 255)
		{
			FreeArray(sIPs);
			return false;
		}
	}

	FreeArray(sIPs);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
