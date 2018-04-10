#ifndef _HTTP_CPP
#define _HTTP_CPP
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include "../../../../../@Libraries/NSWFL/NSWFL.h"

#include "HTTP.H"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::Memory;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
int GetHttpHeaderTag(const char *sHeader, const int iHeaderLen, const char *sTag, char *&sBuf)
{
	int iTagLength = (int)strlen(sTag);
	int iRPos = 0;

	iRPos = InStrI(sTag, iTagLength, sHeader, iHeaderLen, 0);
	//The tag must either be on a line by its-self or the first line.
	while(iRPos > 0 && sHeader[iRPos - 1] != '\n')
	{
		//the tag may be further down the line...
		iRPos = InStrI(sTag, iTagLength, sHeader, iHeaderLen, iRPos + 1);
	}

	if(iRPos >= 0)
	{
		iRPos += iTagLength; //Skip the tag itsself;
		SkipWhiteSpaces(sHeader, iHeaderLen, &iRPos);

		int iEndPos = InStr("\n", 1, sHeader, iHeaderLen, iRPos);
		if(iEndPos > 0 && iEndPos > iRPos)
		{
			if(iEndPos > iRPos)
			{
				int iLength = iEndPos - iRPos;
				sBuf = (char*)StrnDup(sHeader + iRPos, iLength);
				return Trim(sBuf, iLength);
			}
		}
	}

	sBuf = _strdup("");

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *ReplaceHttpHeaderTag(const char *sHeader, const int iHeaderLen, const char *sTag, int iNewValue)
{
	char sNewValue[64];
	sprintf_s(sNewValue, sizeof(sNewValue), "%d", iNewValue);
	return ReplaceHttpHeaderTag(sHeader, iHeaderLen, sTag, sNewValue);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *ReplaceHttpHeaderTag(const char *sHeader, const int iHeaderLen, const char *sTag, const char *sNewValue)
{
	int iTagLength = (int)strlen(sTag);
	int iRPos = 0;

	iRPos = InStrI(sTag, iTagLength, sHeader, iHeaderLen, 0);
	//The tag must either be on a line by its-self or the first line.
	while(iRPos > 0 && sHeader[iRPos - 1] != '\n')
	{
		//the tag may be further down the line...
		iRPos = InStrI(sTag, iTagLength, sHeader, iHeaderLen, iRPos + 1);
	}

	if(iRPos >= 0)
	{
		int iStartPos = iRPos;
		iRPos += iTagLength; //Skip the tag itsself;
		SkipWhiteSpaces(sHeader, iHeaderLen, &iRPos);

		int iEndPos = InStr("\n", 1, sHeader, iHeaderLen, iRPos);
		if(iEndPos > 0 && iEndPos > iRPos)
		{
			if(iEndPos > iRPos)
			{
				size_t iLength = iEndPos - iRPos;
				size_t iNewTagLength = strlen(sTag) + strlen(sNewValue) + 2; //"t: v"
				size_t iNewTotalSize = iHeaderLen + iNewTagLength - iLength;

				char *sLocalHeader = (char *) calloc(iNewTotalSize + 1, sizeof(char));
				char *sLocalTag = (char *)calloc(iNewTagLength + 1, sizeof(char));
				sprintf_s(sLocalTag, iNewTagLength + 1, "%s: %s", sTag, sNewValue);

				strncpy_s(sLocalHeader, iNewTotalSize + 1, sHeader, iStartPos);
				sLocalHeader[iStartPos] = '\0';
				strcat_s(sLocalHeader, (iNewTotalSize + 1), sLocalTag);
				strncat_s(sLocalHeader, (iNewTotalSize + 1), sHeader + iEndPos, iHeaderLen - iEndPos);

				free(sLocalTag);

				return sLocalHeader;
			}
		}
	}

	return _strdup(sHeader);
}
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
