///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _ROUTINES_H
#define _ROUTINES_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GetLastError(CStringBuilder *outError);
bool GetLastError(char *sOutErrorMessage, int iMaxOutErrorMessage);
bool GetLastError(char *sOutErrorMessage, int iMaxOutErrorMessage, const char *sPrefix);
bool GetLastError(char *sOutErrorMessage, int iMaxOutErrorMessage, const char *sPrefix, const char *sSuffix);
void ThrowErrors(void *lpStack);

HANDLE NTLogin(const char *sUsername, const char *sDomain,
			   const char *sPassword, char *sOutErrorMessage, int iMaxOutErrorMessage);
bool MakeGMTTimeString(char *sGMT, int iMaxSz);
bool MakeGMTTimeString(SYSTEMTIME *pST, char *sGMT, int iMaxSz);
bool BreakGMTTimeString(char *sGMT, SYSTEMTIME *pST);
bool GetLastFileModificationDate(const char *sFile, char *sLocalTime, int iMaxSz);
bool GetLastFileModificationDateGMT(const char *sFile, SYSTEMTIME *pST);
bool GetLOCALTime(char *sLocalTime, int iMaxSz);
bool GetLOCALTime(LPSYSTEMTIME lpST, char *sLocalTime, int iMaxSz);
int GetNextToken(const char *sBuf, int iBufSz, char *&sOutToken, int *iInOutRPos);
bool IsValidIPAddressV4(const char *sIP);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

