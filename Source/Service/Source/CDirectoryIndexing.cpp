///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CDirectoryIndexing_CPP
#define _CDirectoryIndexing_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>

#include "Entry.H"

#include "CDirectoryIndexing.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::File;
using namespace NSWFL::ListView;
using namespace NSWFL::Conversion;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char *sIconsExt[] = {

    ".AIF",    "SOUND.PNG",
    ".AIFC",   "SOUND.PNG",
    ".AIFF",   "SOUND.PNG",
    ".ARC",    "COMPRESSED.PNG",
    ".ARJ",    "COMPRESSED.PNG",
    ".ASF",    "VIDEO.PNG",
    ".ASX",    "VIDEO.PNG",
    ".AU",     "SOUND.PNG",
    ".AVI",    "VIDEO.PNG",
    ".B64",    "UUENCODED.PNG",
    ".BHX",    "BINARY.PNG",
    ".BIN",    "BINARY.PNG",
    ".BMP",    "IMAGE.PNG",
    ".C",      "SOURCE_C.PNG",
    ".CAB",    "COMPRESSED.PNG",
    ".COM",    "BINARY.PNG",
    ".CPP",    "SOURCE_CPP.PNG",
    ".CSS",    "SOURCE_CSS.PNG",
    ".CUR",    "IMAGE.PNG",
    ".DAT",    "BINARY.PNG",
    ".DIB",    "IMAGE.PNG",
    ".DLL",    "BINARY.PNG",
    ".DOC",    "WORDPROCESSING.PNG",
    ".EMF",    "IMAGE.PNG",
    ".EXE",    "BINARY.PNG",
    ".F",      "SOURCE_F.PNG",
    ".GIF",    "IMAGE.PNG",
    ".GZ",     "COMPRESSED.PNG",
    ".H",      "SOURCE_H.PNG",
    ".HPP",    "SOURCE_H.PNG",
    ".HQX",    "COMPRESSED.PNG",
    ".HTM",    "HTML.PNG",
    ".HTM",    "HTML.PNG",
    ".HTML",   "HTML.PNG",
    ".ICO",    "IMAGE.PNG",
    ".INI",    "TXT.PNG",
    ".J",      "SOURCE_J.PNG",
    ".JAR",    "JAVA_JAR.PNG",
    ".JAVA",   "SOURCE_JAVA.PNG",
    ".JPE",    "IMAGE.PNG",
    ".JPEG",   "IMAGE.PNG",
    ".JPG",    "IMAGE.PNG",
    ".JS",     "SOURCE_JAVA.PNG",
    ".L",      "SOURCE_L.PNG",
    ".LNK",    "LINK.PNG",
    ".LZH",    "COMPRESSED.PNG",
    ".M1V",    "VIDEO.PNG",
    ".M3U",    "SOUND.PNG",
    ".MID",    "MIDI.PNG",
    ".MID",    "SOUND.PNG",
    ".MIDI",   "MIDI.PNG",
    ".MIDI",   "SOUND.PNG",
    ".MOC",    "SOURCE_MOC.PNG",
    ".MOV",    "VIDEO.PNG",
    ".MP2",    "VIDEO.PNG",
    ".MP2V",   "VIDEO.PNG",
    ".MP3",    "SOUND.PNG",
    ".MPA",    "VIDEO.PNG",
    ".MPE",    "VIDEO.PNG",
    ".MPEG",   "VIDEO.PNG",
    ".MPG",    "VIDEO.PNG",
    ".MPV2",   "VIDEO.PNG",
    ".O",      "SOURCE_O.PNG",
    ".OCX",    "BINARY.PNG",
    ".P",      "SOURCE_P.PNG",
    ".PCX",    "IMAGE.PNG",
    ".PDF",    "PDF.PNG",
    ".PHP",    "SOURCE_PHP.PNG",
    ".PL",     "SOURCE_PL.PNG",
    ".PNG",    "IMAGE.PNG",
    ".PS",     "POSTSCRIPT.PNG",
    ".PY",     "SOURCE_PY.PNG",
    ".Q3X",    "QUICKTIME.PNG",
    ".QDAT",   "QUICKTIME.PNG",
    ".QFI",    "QUICKTIME.PNG",
    ".QFN",    "QUICKTIME.PNG",
    ".QIF",    "QUICKTIME.PNG",
    ".QPX",    "QUICKTIME.PNG",
    ".QPX",    "QUICKTIME.PNG",
    ".QT",     "QUICKTIME.PNG",
    ".QT",     "SOUND.PNG",
    ".QTC",    "QUICKTIME.PNG",
    ".QTCH",   "QUICKTIME.PNG",
    ".QTI",    "QUICKTIME.PNG",
    ".QTIF",   "QUICKTIME.PNG",
    ".QTL",    "QUICKTIME.PNG",
    ".QTM",    "QUICKTIME.PNG",
    ".QTP",    "QUICKTIME.PNG",
    ".QTPF",   "QUICKTIME.PNG",
    ".QTR",    "QUICKTIME.PNG",
    ".QTS",    "QUICKTIME.PNG",
    ".QTV",    "QUICKTIME.PNG",
    ".QTVR",   "QUICKTIME.PNG",
    ".QTX",    "QUICKTIME.PNG",
    ".QUP",    "QUICKTIME.PNG",
    ".RA",     "REAL.PNG",
    ".RLE",    "IMAGE.PNG",
    ".RMI",    "SOUND.PNG",
    ".RTF",    "WORDPROCESSING.PNG",
    ".S",      "SOURCE_S.PNG",
    ".SHTML",  "HTML.PNG",
    ".SND",    "SOUND.PNG",
    ".SSI",    "HTML.PNG",
    ".SSI",    "HTML.PNG",
    ".TAR",    "TAR.PNG",
    ".TAZ",    "COMPRESSED.PNG",
    ".TGZ",    "TGZ.PNG",
    ".TMP",    "FILE_TEMPORARY.PNG",
    ".TXT",    "TXT.PNG",
    ".TZ",     "COMPRESSED.PNG",
    ".UU",     "UUENCODED.PNG",
    ".VBS",    "SHELLSCRIPT.PNG",
    ".VOC",    "SOUND.PNG",
    ".WAV",    "SOUND.PNG",
    ".WAX",    "SOUND.PNG",
    ".WM",     "VIDEO.PNG",
    ".WMA",    "SOUND.PNG",
    ".WMF",    "IMAGE.PNG",
    ".WMX",    "VIDEO.PNG",
    ".WPS",    "WORDPROCESSING.PNG",
    ".WRI",    "TXT.PNG",
    ".WSH",    "SHELLSCRIPT.PNG",
    ".WVX",    "VIDEO.PNG",
    ".Y",      "SOURCE_Y.PNG",
    ".Z",      "COMPRESSED.PNG",
    ".ZIP",    "COMPRESSED.PNG",
	".BAT",    "SHELLSCRIPT.PNG",
	".BWI",    "CDIMAGE.PNG",
	".CHM",    "HELP.PNG",
	".CIF",    "CDIMAGE.PNG",
	".FON",    "FONT.PNG",
	".HLP",    "HELP.PNG",
	".HTA",    "SHELLSCRIPT.PNG",
	".IMG",    "CDIMAGE.PNG",
	".ISO",    "CDIMAGE.PNG",
	".LOG",    "LOG.PNG",
	".NRG",    "CDIMAGE.PNG",
	".RAM",    "REAL.PNG",
	".RM",     "REAL.PNG",
	".RV",     "REAL.PNG",
	".TTF",    "FONT_TRUETYPE.PNG",
	NULL,     NULL
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDirectoryIndexing::GetFileIcon(const char *sFileNameAndPath, char *sIconURL, int iMaxIconURL)
{
	int iPos = 0;

	char sExt[MAX_PATH];

	strcpy_s(sIconURL, iMaxIconURL, "/IndexImages/");
	GetFileExtension(sFileNameAndPath, sExt, sizeof(sExt));
	if(strlen(sExt) == 0)
	{
		strcat_s(sIconURL, iMaxIconURL, "UNKNOWN.PNG");
		return true;
	}

	while(sIconsExt[iPos])
	{
		if(_strcmpi(sIconsExt[iPos], sExt) == 0)
		{
			strcat_s(sIconURL, iMaxIconURL, sIconsExt[iPos + 1]);
			return true;
		}
		iPos = (iPos + 2);
	}

	strcat_s(sIconURL, iMaxIconURL, "UNKNOWN.PNG");

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDirectoryIndexing::IndexDirectory(const char *sDirectory)
{
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;

	StringBuilder path;

	path.AppendF("%s\\*.*", sDirectory);
	if((hFind = FindFirstFile(path, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		return false; //Path not found
	}

	int iIndex = 0;
	int iAlloc = 0;
	
	memset(&this->Index, 0, sizeof(this->Index));

	do
	{
		if(strcmp(fdFile.cFileName, ".") != 0 && strcmp(fdFile.cFileName, "..") != 0)
		{
			//fdFile.cFileName
			if(fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				iIndex = this->Index.Folders.Count;
				iAlloc = iIndex + 1;

				this->Index.Folders.Object = (INDEXEDOBJECT *)
					pMem->ReAllocate(this->Index.Folders.Object, sizeof(INDEXEDOBJECT), iAlloc);

				iAlloc = (int)strlen(fdFile.cFileName) + 1;
				this->Index.Folders.Object[iIndex].Name =
					(char *) pMem->Allocate(sizeof(char), iAlloc);
				strcpy_s(this->Index.Folders.Object[iIndex].Name, iAlloc, fdFile.cFileName);

				memcpy(&this->Index.Folders.Object[iIndex].LastModified,
					&fdFile.ftLastWriteTime, sizeof(fdFile.ftLastWriteTime));

				this->Index.Folders.Count++;
			}
			else{
				iIndex = this->Index.Files.Count;
				iAlloc = iIndex + 1;

				this->Index.Files.Object = (INDEXEDOBJECT *)
					pMem->ReAllocate(this->Index.Files.Object, sizeof(INDEXEDOBJECT), iAlloc);

				iAlloc = (int)strlen(fdFile.cFileName) + 1;
				this->Index.Files.Object[iIndex].Name =
					(char *) pMem->Allocate(sizeof(char), iAlloc);
				strcpy_s(this->Index.Files.Object[iIndex].Name, iAlloc, fdFile.cFileName);

				memcpy(&this->Index.Files.Object[iIndex].LastModified,
					&fdFile.ftLastWriteTime, sizeof(fdFile.ftLastWriteTime));

				ULARGE_INTEGER ulFileSize;
				ulFileSize.HighPart = fdFile.nFileSizeHigh;
				ulFileSize.LowPart = fdFile.nFileSizeLow;

				this->Index.Files.Object[iIndex].Size = ulFileSize.QuadPart;

				this->Index.Files.Count++;
			}
		}
	}
	while(FindNextFile(hFind, &fdFile));

	FindClose(hFind);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDirectoryIndexing::SendContents(PEER *pC, const char *sDirectory)
{
	pC->PerRequestStore.IsDynamicContent = true;

	bool bResult = true;

	if(!this->IndexDirectory(sDirectory))
	{
		return false;
	}

	StringBuilder HTML;

	char sRequest[MAX_URI_LEN];

	strcpy_s(sRequest, sizeof(sRequest), pC->Header.Request);
	int iLen = (int)strlen(sRequest);
	if(sRequest[iLen - 1] != '/')
	{
		strcat_s(sRequest, sizeof(sRequest), "/");
	}

	CHttp *pHttp = (CHttp *)((CWebSite *)pC->pWebSite)->pHttp;
	CWebSites *pWebSites = (CWebSites *)((CWebSite *)pC->pWebSite)->pWebSites;

	if(this->ToHTML(sRequest, &HTML))
	{
		if(pWebSites->pHttp->SendOKHeader(pC, NULL, "text/html", HTML.Length))
		{
			if((pC->Header.Method != NULL && _strcmpi(pC->Header.Method, "HEAD") != 0) || pC->Header.Method == NULL)
			{
				bResult = pWebSites->pHttp->BufferDataToClient(pC, HTML.Buffer, HTML.Length);
			}
			else {
				bResult = true;
			}
		}
		else{
			bResult = false;
		}
	}
	else{
		bResult = false;
	}

	this->Destroy();

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *URLEncodePath(const char *sRawPath, StringBuilder *lpURLEncoded)
{
	StringBuilder TempString;

	char **sParts = NULL;
	int iParts = 0;
	int iLength = (int)strlen(sRawPath);

	lpURLEncoded->Clear();

	if(sRawPath[0] == '/')
	{
		lpURLEncoded->Append("/");
	}

	if((sParts = Split(sRawPath, iLength, &iParts, "/", 1)))
	{
		for(int i = 0; i < iParts; i++)
		{
			int iPartLength = (int)strlen(sParts[i]);

			int iMinimumSize = URLEncodeSimulate(sParts[i], iPartLength) + 1;

			TempString.Resize(iMinimumSize);

			if(URLEncode(sParts[i], iPartLength, TempString.Buffer, TempString.Alloc) > 0)
			{
				lpURLEncoded->AppendF("%s/", TempString.Buffer);
			}
		}

		FreeArray(sParts, iParts);
	}

	if(sRawPath[iLength - 1] == '/')
	{
		lpURLEncoded->Terminate(lpURLEncoded->Length - 1);
	}
	else {
		lpURLEncoded->Terminate(lpURLEncoded->Length - 1);
	}

	return lpURLEncoded->Buffer;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDirectoryIndexing::ToHTML(const char *sCurrentFolder, StringBuilder *lpStr)
{
	char sImageURL[MAX_URI_LEN];
	char sSize[128];
	char sModifiedDate[64];
	char sModifiedTime[64];
	int iCurrentFolderLen = (int)strlen(sCurrentFolder);
	StringBuilder URLEncoded;
	StringBuilder URL;

	int iLevelIndex = LastIndexOf(sCurrentFolder, iCurrentFolderLen - 1, '/');
	if(iLevelIndex < 0)
	{
		iLevelIndex = 0;
	}
	bool bsRoot = ((iCurrentFolderLen <= 1) && (iLevelIndex <= 0));

	StringBuilder BackLink(sCurrentFolder, iLevelIndex);
	BackLink.Append("/");
	BackLink.Terminate();

	lpStr->Append("<html>");
	lpStr->Append("<head>");
	lpStr->AppendF("<title>%s</title>", sCurrentFolder);
	lpStr->Append("</head>");

	lpStr->Append("<body>");

	lpStr->Append("<table width=\"100%\" border=\"0\" cellpadding=\"1\" cellspacing=\"0\">");

	lpStr->Append("<tr>");
	lpStr->Append("<td colspan=\"4\" align=\"left\"><font size='5'>Index of ");
	lpStr->Append(sCurrentFolder);
	lpStr->Append("</font></td>");
	lpStr->Append("</tr>");

	lpStr->Append("<tr>");
	lpStr->AppendF("<td align=\"center\" width=\"16\">");
	lpStr->AppendF("<a href=\"%s\"><img src=\"/IndexImages/back.gif\" border=\"0\" /></a>", BackLink.Buffer);
	lpStr->Append("</td>");
	lpStr->Append("<td align=\"left\"><b>Name</b></td>");
	lpStr->Append("<td align=\"right\"><b>Modified</b></td>");
	lpStr->Append("<td align=\"right\"><b>Size</b></td>");
	lpStr->Append("</tr>");

	lpStr->Append("<tr>");
	lpStr->Append("<td colspan=\"4\"><hr width=\"100%\" /></td>");
	lpStr->Append("</tr>");

	for(int iIndex = 0; iIndex < this->Index.Folders.Count; iIndex++)
	{
		URL.Clear();

		URL.AppendF("%s/%s", sCurrentFolder, this->Index.Folders.Object[iIndex].Name);
		CorrectForwardPath(URL.Buffer, URL.Alloc);

		lpStr->Append("<tr>");
		lpStr->Append("<td align=\"center\" width=\"16\"><img src=\"/IndexImages/Folder.png\" /></td>");
		lpStr->Append("<td align=\"left\"><a href=\"");

		//URLEncode(sURL, sURLEncoded, sizeof(sURLEncoded));
		//lpStr->Append(sURLEncoded);
		//lpStr->Append(sURL);
		lpStr->Append(URLEncodePath(URL.Buffer, &URLEncoded));

		lpStr->Append("\">");
		lpStr->Append(this->Index.Folders.Object[iIndex].Name);
		lpStr->Append("</a></td>");
		lpStr->Append("<td align=\"right\">");
		
		SYSTEMTIME ST;
		FileTimeToSystemTime(&this->Index.Folders.Object[iIndex].LastModified, &ST);
		GetDateFormat(NULL, NULL, &ST, "MM/dd/yyyy", sModifiedDate, sizeof(sModifiedDate));
		GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &ST, NULL, sModifiedTime, sizeof(sModifiedTime));

		lpStr->Append(sModifiedDate);
		lpStr->Append(" ");
		lpStr->Append(sModifiedTime);

		lpStr->Append("</td>");
		lpStr->Append("<td align=\"right\" width=\"100\"> - </td>");
		lpStr->Append("</tr>");
	}

	for(int iIndex = 0; iIndex < this->Index.Files.Count; iIndex++)
	{
		URL.Clear();

		URL.AppendF("%s/%s", sCurrentFolder, this->Index.Files.Object[iIndex].Name);
		CorrectForwardPath(URL.Buffer, URL.Alloc);
		//URLEncode((unsigned char *)sURL, (unsigned char *)sURLEncoded);

		this->GetFileIcon(this->Index.Files.Object[iIndex].Name, sImageURL, sizeof(sImageURL));
		FileSizeFriendly(this->Index.Files.Object[iIndex].Size, sSize, sizeof(sSize));

		lpStr->Append("<tr>");
		lpStr->Append("<td align=\"center\" width=\"16\"><img src=\"");
		lpStr->Append(sImageURL);
		lpStr->Append("\" /></td>");
		lpStr->Append("<td align=\"left\"><a href=\"");
		//URLEncode(sURL, sURLEncoded, sizeof(sURLEncoded));
		//lpStr->Append(sURLEncoded);
		//lpStr->Append(sURL);
		lpStr->Append(URLEncodePath(URL.Buffer, &URLEncoded));
		lpStr->Append("\">");
		lpStr->Append(this->Index.Files.Object[iIndex].Name);
		lpStr->Append("</a></td>");
		lpStr->Append("<td align=\"right\">");

		SYSTEMTIME ST;
		FileTimeToSystemTime(&this->Index.Files.Object[iIndex].LastModified, &ST);
		GetDateFormat(NULL, NULL, &ST, "MM/dd/yyyy", sModifiedDate, sizeof(sModifiedDate));
		GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &ST, NULL, sModifiedTime, sizeof(sModifiedTime));

		lpStr->Append(sModifiedDate);
		lpStr->Append(" ");
		lpStr->Append(sModifiedTime);

		lpStr->Append("</td>");
		lpStr->Append("<td align=\"right\">");
		lpStr->Append(sSize);
		lpStr->Append("</td>");
		lpStr->Append("</tr>");
	}

	lpStr->Append("<tr>");
	lpStr->Append("<td colspan=\"4\">");
	lpStr->Append("<hr width=\"100%\" />");
	lpStr->Append("</td>");
	lpStr->Append("</tr>");

	lpStr->Append("<tr>");
	lpStr->Append("<td align=\"left\" colspan=\"4\">");
	lpStr->AppendF("<i>%s version %s</i>", gsTitleCaption, gsFileVersion);
	lpStr->Append("</td>");
	lpStr->Append("</tr>");

	lpStr->Append("</table>");

	lpStr->Append("</body>");
	lpStr->Append("</html>");

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDirectoryIndexing::Destroy(void)
{
	if(this->Index.Folders.Count > 0)
	{
		for(int iIndex = 0; iIndex < this->Index.Folders.Count; iIndex++)
		{
			pMem->Free(this->Index.Folders.Object[iIndex].Name);
		}
		pMem->Free(this->Index.Folders.Object);
	}

	if(this->Index.Files.Count > 0)
	{
		for(int iIndex = 0; iIndex < this->Index.Files.Count; iIndex++)
		{
			pMem->Free(this->Index.Files.Object[iIndex].Name);
		}
		pMem->Free(this->Index.Files.Object);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
