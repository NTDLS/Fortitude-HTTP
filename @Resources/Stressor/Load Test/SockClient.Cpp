#ifndef _ENTRY_CPP
#define _ENTRY_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <UrlMon.H>
#include <Wininet.h>

#include "../../../@Classes/CStringBuilder/CStringBuilder.H"
#include "../../../@Libraries/NSWFL/NSWFL.h"

using namespace NSWFL::Conversion;
using namespace NSWFL::File;
using namespace NSWFL::String;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define gsTitleCaption "Fortitude Stressor"
#define gsFileVersion  "1.0.0.0"
#define gsDestination  "C:\\Index\\"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DownloadFile(const char *sURL, char *sFile, int iMaxFileSz, bool *bIsHTML)
{
	char sGUID[64];
	NewGUID(sGUID, sizeof(sGUID));

	sprintf_s(sFile, iMaxFileSz, "%s%s.html", gsDestination, sGUID);

	bool bResult = false;
	const int BUFFER_SIZE = (8 * 1024);
	unsigned long iSize = 0;
	char sBuffer[BUFFER_SIZE+1];
    char sText[1024];
 	FILE *hFile = NULL;
	float fProgress = 0;

	sprintf_s(sText, "%s - %s", gsTitleCaption, gsFileVersion);
	HINTERNET hNet = InternetOpen(sText, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if(!hNet)
	{
        MessageBox(NULL, "Failed to open internet connection.", gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);
		return false;
	}

	HINTERNET hINetFile = InternetOpenUrl(hNet, sURL, NULL, 0,
		INTERNET_FLAG_RELOAD | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_CACHE_WRITE, 0); 
	if(!hINetFile)
	{
        MessageBox(NULL, "Failed to open internet URL.", gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);
		InternetCloseHandle(hNet);
		return false;
	}

	if(bIsHTML)
	{
		char sContentType[2048];
		DWORD dwContentTypeSz = sizeof(sContentType);

		if(HttpQueryInfo(hINetFile, HTTP_QUERY_CONTENT_TYPE, (LPVOID)sContentType, &dwContentTypeSz, NULL))
		{
			if(_strcmpi(sContentType, "text/html") == 0)
			{
				*bIsHTML = true;
			}
		}
	}

	if(fopen_s(&hFile, sFile, "wb") != 0)
	{
        MessageBox(NULL, "Failed to open temp file.", gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);

		InternetCloseHandle(hINetFile);
		InternetCloseHandle(hNet);
		return false;
	}

	while((bResult = InternetReadFile(hINetFile, sBuffer, BUFFER_SIZE, &iSize) > 0))
	{
		if(iSize <= 0)
		{
			bResult = true;
			break;
		}
		fwrite(sBuffer, sizeof(char), iSize, hFile);
		fProgress += iSize;

        printf_s("%.0f", fProgress);
	}

	if(!bResult)
	{
        MessageBox(NULL, "Failed to download file.", gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);
	}

	fclose(hFile);
	InternetCloseHandle(hINetFile);
	InternetCloseHandle(hNet);

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ParseFile(const char *sFileName)
{
	CStringBuilder sText;

	sText.AppendFromFile(sFileName);

	MessageBox(NULL, sText.Buffer, "", 0);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int main(int argc, char *argv[])
{
	char sDestFile[MAX_PATH];
	bool bIsHTML = false;
	if(DownloadFile("http://127.0.0.1/", sDestFile, sizeof(sDestFile), &bIsHTML))
	{
		if(bIsHTML)
		{
			ParseFile(sDestFile);
		}
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
