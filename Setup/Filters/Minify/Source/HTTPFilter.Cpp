#ifndef _FILTER_CPP
#define _FILTER_CPP
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include "HTTP.H"
#include "HTTPFilter.H"
#include "../../../../../@Libraries/NSWFL/NSWFL.H"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::Memory;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int RemoveSpaces(char *sBuffer, int iLength)
{
	int iRPos = 0;
	int iWPos = 0;

	while((iRPos = InStr("\r\n", 2, sBuffer, iLength, 0)) >= 0)
	{
		iLength -= 1;
		sBuffer[iRPos] = ' ';
		memcpy(sBuffer + iRPos + 1, sBuffer + iRPos + 2, iLength - iRPos);
	}

	iRPos = 0;

	while(iRPos <= iLength)
	{
		if(sBuffer[iRPos] == ' ')
		{
			int iRunLength = 0;
			int iRunRPos = iRPos;
			while(sBuffer[iRunRPos] == ' ')
			{
				iRunLength++;
				iRunRPos++;
			}
			if(iRunLength > 1)
			{
				iRPos = iRunRPos - 1;
			}
		}
		else if(sBuffer[iRPos] == '\t')
		{
			iRPos++;
			continue;
		}

		sBuffer[iWPos++] = sBuffer[iRPos++];
	}
	sBuffer[iWPos] = '\0';
	return iWPos;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Notifies the filter before raw data compressed at the server.
DllExport unsigned int OnCompressData(unsigned int iPeerId, const char *sPathTranslated, const char *sBuf, unsigned int uiBufSz, char **sModifiedBuf, unsigned int *uiModifiedBufSz)
{
	if(sPathTranslated != NULL)
	{
		char sExtension[MAX_PATH];
		_splitpath_s(sPathTranslated, NULL, 0, NULL, 0, NULL, 0, sExtension, sizeof(sExtension));

		if(_strcmpi(sExtension, ".html") == 0)
		{
			char *sLocalBuffer = StrnDup(sBuf, uiBufSz);
			int iNewLength = RemoveSpaces(sLocalBuffer, uiBufSz);

			(*sModifiedBuf) = (char *)GlobalAlloc(0, iNewLength + 1);

			strcpy_s((*sModifiedBuf), iNewLength + 1, sLocalBuffer);

			(*sModifiedBuf)[iNewLength] = '\0';
			*uiModifiedBufSz = (unsigned int)iNewLength;
		}
	}

	return FILTER_EVENT_RESULT_CONTINUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
