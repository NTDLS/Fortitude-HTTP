///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CCompression_CPP
#define _CCompression_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>

extern HIMAGELIST hEnableDisableImageList; //Declared in MainDialog.cpp
extern HIMAGELIST hOnePixilImageList; //Declared in MainDialog.cpp

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CCompression.H"
#include "Entry.H"
#include "CWebSites.H"
#include "../../../Compression/zLib/ZLibEncapsulation.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::File;
using namespace NSWFL::ListView;
using namespace NSWFL::Conversion;
using namespace NSWFL::Windows;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CCompression::~CCompression(void)
{
	this->Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CCompression::CCompression(void *lpWebSites)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CCompression::CCompression(void *lpWebSites, XMLReader *xmlConfig, CCompression *pDefaults)
{
	this->Initialized = false;
	this->pWebSites = lpWebSites;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
	this->Load(xmlConfig, pDefaults);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCompression::Save(void)
{
	this->Locks.LockShared();

	XMLReader xmlConfig;
	if(this->ToXML(&xmlConfig))
	{
		bool bResult = xmlConfig.ToFile(this->sFileName);
		if(!bResult)
		{
			char sErrorMsg[2048];
			GetLastError(sErrorMsg, sizeof(sErrorMsg),
				"Failed to save websites configuration file (", ")");
			strcat_s(sErrorMsg, sizeof(sErrorMsg), "\r\n\r\n\"");
			strcat_s(sErrorMsg, sizeof(sErrorMsg), this->sFileName);
			strcat_s(sErrorMsg, sizeof(sErrorMsg), "\"");
			((CWebSites *)this->pWebSites)->Trace(sErrorMsg);
		}
		return this->Locks.UnlockShared(bResult);
	}
	return this->Locks.UnlockShared(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCompression::ToXML(XMLReader *lpXML)
{
	this->Locks.LockShared();

	XMLWriter xmlConfig("Compression");

  	xmlConfig.Add("CachePath", this->Cache.CachePath);
	xmlConfig.Add("MaxCompressionSize", this->Collection.MaxCompressionSize);
	xmlConfig.Add("MinCompressionSize", this->Collection.MinCompressionSize);
	xmlConfig.Add("MaxCompressionCache", this->Cache.MaxCachedFiles);
	xmlConfig.Add("CompressionLevel", this->Collection.CompressionLevel);
	xmlConfig.Add("MemoryLevel", this->Collection.MemoryLevel);
	xmlConfig.AddBool("CompressDynamicContent", this->Collection.CompressDynamicContent);
	xmlConfig.AddBool("Enable", this->Collection.Enabled);

	for(int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		XMLWriter Item("FileType");
		Item.Add("Extension", this->Collection.Items[iItem].Extension);
		Item.Add("CompressionLevel", this->Collection.Items[iItem].CompressionLevel);
		Item.Add("Description", this->Collection.Items[iItem].Description);
		Item.AddBool("Enable", this->Collection.Items[iItem].Enabled);
		xmlConfig.Add(&Item);
	}

	xmlConfig.ToReader(lpXML);

	xmlConfig.Destroy();

	return this->Locks.UnlockShared(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCompression::CacheToXML(XMLReader *lpXML)
{
	this->Locks.LockShared();

	XMLWriter xmlConfig("Cache");

	char sFileName[MAX_PATH];

	for(int iItem = 0; iItem < this->Cache.Count; iItem++)
	{
		GetFileName(this->Cache.Items[iItem].OriginalFile, sFileName, sizeof(sFileName));

		XMLWriter Item("Item");
		Item.Add("File", sFileName);
		Item.Add("OriginalSize", this->Cache.Items[iItem].OriginalSize);
		Item.Add("CachedSize", this->Cache.Items[iItem].CachedSize);
		Item.Add("Hits", this->Cache.Items[iItem].Hits);
		xmlConfig.Add(&Item);
	}

	xmlConfig.ToReader(lpXML);

	xmlConfig.Destroy();

	return this->Locks.UnlockShared(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	Reloads the configuration from the file it was originally loaded from.
*/
bool CCompression::Reload(void)
{
	this->Locks.LockExclusive();

	if(this->Initialized)
	{
		this->Destroy();
	}

	this->Load(this->sFileName);

	return this->Locks.UnlockExclusive(this->Initialized);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCompression::Load(const char *sXMLFileName)
{
	this->Locks.LockExclusive();
	if(this->Initialized)
	{
		this->Destroy();
	}

	strcpy_s(this->sFileName, sizeof(this->sFileName), sXMLFileName);

	XMLReader xmlConfig;

	if(xmlConfig.FromFile(sXMLFileName))
	{
		XMLReader xmlEntity;
		if(xmlConfig.ToReader("Compression", &xmlEntity))
		{
			this->Initialized = this->Load(&xmlEntity, NULL);
		}
		xmlEntity.Destroy();
		xmlConfig.Destroy();
	}

	return this->Locks.UnlockExclusive(this->Initialized);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCompression::Load(XMLReader *xmlConfig, CCompression *pDefaults)
{
	this->Locks.LockExclusive();
	if(this->Initialized)
	{
		this->Destroy();
	}

	InitializeCriticalSection(&this->CS);

	int iLength = 0;
	memset(&this->Collection, 0, sizeof(this->Collection));
	memset(&this->Cache, 0, sizeof(this->Cache));

	char sCachePath[MAX_PATH];
	iLength = xmlConfig->ToString("CachePath", sCachePath, sizeof(sCachePath));
	this->Cache.CachePath = (char *) pMem->StrDup(sCachePath);

	this->Collection.CompressDynamicContent = xmlConfig->ToBoolean("CompressDynamicContent", true);
	this->Collection.Enabled = xmlConfig->ToBoolean("Enable", true);
	this->Collection.MaxCompressionSize = xmlConfig->ToInteger("MaxCompressionSize", 1048576);
	this->Collection.MinCompressionSize = xmlConfig->ToInteger("MinCompressionSize", 25600);
	this->Collection.CompressionLevel = xmlConfig->ToInteger("CompressionLevel", Z_BEST_SPEED);
	this->Collection.MemoryLevel = xmlConfig->ToInteger("MemoryLevel", 8);
	this->Cache.MaxCachedFiles = xmlConfig->ToInteger("MaxCompressionCache", 10000);

	xmlConfig->ProgressiveScan(true);
	XMLReader XPCompType;

	while(xmlConfig->ToReader("FileType", &XPCompType))
	{
		char sExt[COMPRESSION_MAX_EXT_LENGTH];
		char sDescr[COMPRESSION_MAX_DESCR_LENGTH];

		this->Collection.Items = (COMPRESSIBLEFILE *)
			pMem->ReAllocate(this->Collection.Items, sizeof(COMPRESSIBLEFILE),
			this->Collection.Count + 1);

		COMPRESSIBLEFILE *p = &this->Collection.Items[this->Collection.Count++];

		XPCompType.ToString("Extension", sExt, sizeof(sExt), &iLength);
		p->Extension = (char *) pMem->StrDup(LCase(sExt, iLength));

		XPCompType.ToString("Description", sDescr, sizeof(sDescr), &iLength);
		p->Description = (char *) pMem->StrDup(sDescr);

		p->CompressionLevel =
			XPCompType.ToInteger("CompressionLevel", this->Collection.CompressionLevel);

		p->Enabled = XPCompType.ToBoolean("Enable", true);

		XPCompType.Destroy();
	}

	return this->Locks.UnlockExclusive(this->Initialized = true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCompression::FreeCacheItem(COMPRESSIONCACHEITEM *lpItem)
{
	this->Locks.LockShared();

	EnterCriticalSection(&this->CS);

	DeleteFile(lpItem->CachedFile);

	pMem->Free(lpItem->CachedFile);
	pMem->Free(lpItem->OriginalFile);

	lpItem->CachedSize = 0;
	lpItem->OriginalSize = 0;

	LeaveCriticalSection(&this->CS);

	return this->Locks.UnlockShared(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__int64 CCompression::GetCompressedCacheSize(void)
{
	this->Locks.LockShared();

	__int64 i64Size = 0;

	EnterCriticalSection(&this->CS);
	for(int iItem = 0; iItem < this->Cache.Count; iItem++)
	{
		i64Size += this->Cache.Items[iItem].CachedSize;
	}
	LeaveCriticalSection(&this->CS);

	return this->Locks.UnlockShared(i64Size);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__int64 CCompression::GetNativeCacheSize(void)
{
	this->Locks.LockShared();

	__int64 i64Size = 0;

	EnterCriticalSection(&this->CS);
	for(int iItem = 0; iItem < this->Cache.Count; iItem++)
	{
		i64Size += this->Cache.Items[iItem].OriginalSize;
	}
	LeaveCriticalSection(&this->CS);

	return this->Locks.UnlockShared(i64Size);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double CCompression::GetCacheCompressionRatio(void)
{
	this->Locks.LockShared();

	EnterCriticalSection(&this->CS);

	double dRatio = 0;
	__int64 i64Native = this->GetNativeCacheSize();

	if(i64Native > 0)
	{
		__int64 i64Compressed = this->GetCompressedCacheSize();

		dRatio = 100.0f - ((double)(((double)i64Compressed / (double)i64Native) * 100.0f));
	}
	else{
		dRatio = 0;
	}

	LeaveCriticalSection(&this->CS);

	return this->Locks.UnlockShared(dRatio);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CCompression::MaxCompressionSize(void)
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->Collection.MaxCompressionSize);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CCompression::MinCompressionSize(void)
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->Collection.MinCompressionSize);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CCompression::CompressionLevel(void)
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->Collection.CompressionLevel);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CCompression::MemoryLevel(void)
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->Collection.MemoryLevel);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CCompression::MaxCachedFiles(void)
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->Cache.MaxCachedFiles);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CCompression::CachedFiles(void)
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->Cache.Count);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCompression::CompressDynamicContent(void)
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->Enabled() && this->Collection.CompressDynamicContent);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCompression::Enabled(void)
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->Collection.Enabled && (this->Collection.Count > 0));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *CCompression::CachePath(void)
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->Cache.CachePath);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCompression::Destroy(void)
{
	this->Locks.LockExclusive();

	if(this->Initialized)
	{
		this->Initialized = false;
		if(this->Collection.Count > 0)
		{
			for(int iItem = 0; iItem < this->Collection.Count; iItem++)
			{
				pMem->Free(this->Collection.Items[iItem].Extension);
				pMem->Free(this->Collection.Items[iItem].Description);
			}

			pMem->Free(this->Collection.Items);

			this->Collection.Count = 0;
		}

		pMem->Free(this->Cache.CachePath);

		if(this->Cache.Count > 0)
		{
			for(int iItem = 0; iItem < this->Cache.Count; iItem++)
			{
				this->FreeCacheItem(&this->Cache.Items[iItem]);
			}

			pMem->Free(this->Cache.Items);

			this->Cache.Count = 0;
		}
		DeleteCriticalSection(&this->CS);
	}
	return this->Locks.UnlockExclusive(true);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCompression::FlushCache(void)
{
	this->Locks.LockExclusive();

	EnterCriticalSection(&this->CS);

	if(this->Cache.Count > 0)
	{
		for(int iItem = 0; iItem < this->Cache.Count; iItem++)
		{
			this->FreeCacheItem(&this->Cache.Items[iItem]);
		}

		pMem->Free(this->Cache.Items);
		this->Cache.Items = NULL;

		this->Cache.Count = 0;
	}

	LeaveCriticalSection(&this->CS);

	return this->Locks.UnlockExclusive(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCompression::GetUniqueFileName(char *sTarget, int iMaxTargetSz, const char *sOriginalFileName)
{
	this->Locks.LockShared();
	char sGUID[32 + 1];

	if(NewGUID(sGUID, sizeof(sGUID)))
	{
		char sExt[MAX_PATH];
		GetFileExtension(sOriginalFileName, sExt, sizeof(sExt));
		sprintf_s(sTarget, iMaxTargetSz, "%s\\%s%s", this->Cache.CachePath, sGUID, sExt);
		CorrectReversePath(sTarget, iMaxTargetSz, true);
		return this->Locks.UnlockShared(true);
	}

	return this->Locks.UnlockShared(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCompression::GetUniqueFileName(char *sTarget, int iMaxTargetSz)
{
	this->Locks.LockShared();
	return this->Locks.UnlockShared(this->GetUniqueFileName(sTarget, iMaxTargetSz, "Compressed.gz"));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCompression::ResetCacheFileTime(COMPRESSIONCACHEITEM *lpItem)
{
	this->Locks.LockShared();

	bool bResult = true;

	FILETIME lpWriteTime;
	FILETIME lpCreateTime;

	HANDLE hFile = NULL;

	hFile = CreateFile(
		lpItem->OriginalFile,
		0,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		return this->Locks.UnlockShared(false);
	}

	GetFileTime(hFile, &lpCreateTime, NULL, &lpWriteTime);

	CloseHandle(hFile);

	hFile = CreateFile(
		lpItem->CachedFile,
		GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		return this->Locks.UnlockShared(false);
	}

	bResult = (SetFileTime(hFile, &lpCreateTime, NULL, &lpWriteTime) == TRUE);

	CloseHandle(hFile);

	return this->Locks.UnlockShared(bResult);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCompression::HasCacheFileChanged(COMPRESSIONCACHEITEM *lpItem)
{
	this->Locks.LockShared();

	bool bResult = true;

	FILETIME lpFTFile;
	FILETIME lpFTCache;

	HANDLE hFile = NULL;
	HANDLE hCache = NULL;

	hFile = CreateFile(
		lpItem->OriginalFile,
		0,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		return this->Locks.UnlockShared(true);
	}

	hCache = CreateFile(
		lpItem->CachedFile,
		0,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if(hCache == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		return this->Locks.UnlockShared(true);
	}

	GetFileTime(hFile, NULL, NULL, &lpFTFile);
	GetFileTime(hCache, NULL, NULL, &lpFTCache);

	int iDiff = CompareFileTime(&lpFTFile, &lpFTCache);

	CloseHandle(hCache);
	CloseHandle(hFile);

	return this->Locks.UnlockShared(iDiff != 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CCompression::GetLeastHitItem(void)
{
	this->Locks.LockShared();

	int iItem = 0;

	for(int iItem = 0; iItem < this->Cache.Count; iItem++)
	{
		if(this->Cache.Items[iItem].Hits < this->Cache.Items[iItem].Hits)
		{
			iItem = iItem;
		}
	}

	return this->Locks.UnlockShared(iItem);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCompression::CompressFile(PEER *pC, const char *sSource, char *sTarget, int iMaxTargetSz)
{
	this->Locks.LockShared();

	EnterCriticalSection(&this->CS);

	for(int iItem = 0; iItem < this->Cache.Count; iItem++)
	{
		if(_strcmpi(this->Cache.Items[iItem].OriginalFile, sSource) == 0)
		{
			if(this->HasCacheFileChanged(&this->Cache.Items[iItem]))
			{
				if(FileSize(sSource, &this->Cache.Items[iItem].OriginalSize))
				{
					//Recompress the file.
					if(this->Deflate(pC, sSource, this->Cache.Items[iItem].CachedFile))
					{
						if(FileSize(this->Cache.Items[iItem].CachedFile, &this->Cache.Items[iItem].CachedSize))
						{
							if(this->ResetCacheFileTime(&this->Cache.Items[iItem]))
							{
								bool bResult = false;
								if(this->Cache.Items[iItem].CachedSize < this->Cache.Items[iItem].OriginalSize)
								{
									bResult = true;
									strcpy_s(sTarget, iMaxTargetSz, this->Cache.Items[iItem].CachedFile);
								}
								else{
									strcpy_s(sTarget, iMaxTargetSz, sSource);
								}

								LeaveCriticalSection(&this->CS);
								return this->Locks.UnlockShared(bResult);
							}
						}
					}
				}
				LeaveCriticalSection(&this->CS);
				return this->Locks.UnlockShared(false); //Caller needs to raise 500 error.
			}
			else{
				this->Cache.Items[iItem].Hits++;

				bool bResult = false;
				if(this->Cache.Items[iItem].CachedSize < this->Cache.Items[iItem].OriginalSize)
				{
					bResult = true;
					strcpy_s(sTarget, iMaxTargetSz, this->Cache.Items[iItem].CachedFile);
				}
				else{
					strcpy_s(sTarget, iMaxTargetSz, sSource);
				}
				LeaveCriticalSection(&this->CS);
				return this->Locks.UnlockShared(bResult);
			}
		}
	}

	int iItem = -1;
	int iAlloc = 0;

	if(this->Cache.Count >= this->Cache.MaxCachedFiles)
	{
		iItem = this->GetLeastHitItem();
		this->FreeCacheItem(&this->Cache.Items[iItem]);
	}
	else{
		iItem = this->Cache.Count++;

		this->Cache.Items = (COMPRESSIONCACHEITEM *)
			pMem->ReAllocate(this->Cache.Items, sizeof(COMPRESSIONCACHEITEM), iItem + 1);
	}

	if(this->Cache.Items)
	{
		memset(&this->Cache.Items[iItem], 0, sizeof(COMPRESSIONCACHEITEM));

		this->Cache.Items[iItem].Hits = 1;

		char sCacheName[MAX_PATH];
		if(this->GetUniqueFileName(sCacheName, sizeof(sCacheName), sSource))
		{
			this->Cache.Items[iItem].OriginalFile = (char *) pMem->StrDup(sSource);
			this->Cache.Items[iItem].CachedFile = (char *) pMem->StrDup(sCacheName);

			if(FileSize(sSource, &this->Cache.Items[iItem].OriginalSize))
			{
				//Compress the file.
				if(this->Deflate(pC, sSource, sCacheName))
				{
					if(FileSize(sCacheName, &this->Cache.Items[iItem].CachedSize))
					{
						if(this->ResetCacheFileTime(&this->Cache.Items[iItem]))
						{
							bool bResult = false;
							if(this->Cache.Items[iItem].CachedSize < this->Cache.Items[iItem].OriginalSize)
							{
								bResult = true;
								strcpy_s(sTarget, iMaxTargetSz, this->Cache.Items[iItem].CachedFile);
							}
							else{
								strcpy_s(sTarget, iMaxTargetSz, sSource);
							}
							LeaveCriticalSection(&this->CS);
							return this->Locks.UnlockShared(bResult);
						}
					}
				}
			}
		}
	}

	LeaveCriticalSection(&this->CS);
	return this->Locks.UnlockShared(false); //Caller needs to raise 500 error.
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCompression::RemoveFileFromCache(const char *sOriginalFile)
{
	this->Locks.LockShared();
	EnterCriticalSection(&this->CS);

	for(int iItem = 0; iItem < this->Cache.Count; iItem++)
	{
		if(_strcmpi(this->Cache.Items[iItem].OriginalFile, sOriginalFile) == 0)
		{
			this->FreeCacheItem(&this->Cache.Items[iItem]);
			this->Cache.Count--;
		}
	}

	LeaveCriticalSection(&this->CS);

	return this->Locks.UnlockShared(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns the desired compression level for compressible filetype.
*/
int CCompression::GetFileCompressionLevel(const char *sFileName)
{
	this->Locks.LockShared();

	if(this->Collection.Enabled && (this->Collection.Count > 0))
	{
		char sExt[MAX_PATH];
		GetFileExtension(sFileName, sExt, sizeof(sExt));

		for(int iItem = 0; iItem < this->Collection.Count; iItem++)
		{
			if(this->Collection.Items[iItem].Enabled)
			{
				if(_strcmpi(this->Collection.Items[iItem].Extension, sExt) == 0)
				{
					if(this->Collection.Items[iItem].CompressionLevel > 0)
					{
						return this->Locks.UnlockShared(this->Collection.Items[iItem].CompressionLevel);
					}
					else{
						return this->Locks.UnlockShared(this->Collection.CompressionLevel);
					}
				}
			}
		}
	}

	return this->Locks.UnlockShared(this->Collection.CompressionLevel);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns true if the filetype is compressible, otherwise false.
*/
bool CCompression::IsFileCompressible(const char *sFileName)
{
	this->Locks.LockShared();

	if(this->Collection.Enabled && (this->Collection.Count > 0))
	{
		if(this->IsCompressibleType(sFileName))
		{
			__int64 i64FileSize = 0;
			if(!FileSize(sFileName, &i64FileSize))
			{
				return this->Locks.UnlockShared(false);
			}

			if(i64FileSize < this->Collection.MinCompressionSize)
			{
				return this->Locks.UnlockShared(false); //File is too small to compress.
			}

			if(i64FileSize > this->Collection.MaxCompressionSize)
			{
				return this->Locks.UnlockShared(false); //File is too large to compress.
			}
			return this->Locks.UnlockShared(true);
		}
	}
	return this->Locks.UnlockShared(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns true if the filetype is compressible, otherwise false.
*/
bool CCompression::IsCompressibleType(const char *sFileName)
{
	this->Locks.LockShared();

	if(this->Collection.Enabled && (this->Collection.Count > 0))
	{
		char sExt[MAX_PATH];
		GetFileExtension(sFileName, sExt, sizeof(sExt));

		for(int iItem = 0; iItem < this->Collection.Count; iItem++)
		{
			if(this->Collection.Items[iItem].Enabled)
			{
				if(_strcmpi(this->Collection.Items[iItem].Extension, sExt) == 0)
				{
					return this->Locks.UnlockShared(true);
				}
			}
		}
	}
	return this->Locks.UnlockShared(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCompression::Inflate(const char *sSourceName, const char *sTargetName)
{
	this->Locks.LockShared();

	char *sInBuf = NULL;
	char *sOutBuf = NULL;

	FILE *fSource = NULL;
	FILE *hTarget = NULL;

	int iResult = Z_OK;

	unsigned int iHave = 0;

    z_stream zStream;

	memset(&zStream, 0, sizeof(zStream));

	if((iResult = inflateInit(&zStream)) != Z_OK)
	{
        return this->Locks.UnlockShared(false);
	}

	if(fopen_s(&fSource, sSourceName, "rb") != 0)
	{
	    inflateEnd(&zStream);
		return this->Locks.UnlockShared(false);
	}

	if(fopen_s(&hTarget, sTargetName, "wb") != 0)
	{
	    inflateEnd(&zStream);
		fclose(fSource);
		return this->Locks.UnlockShared(false);
	}
	
	if((sInBuf = (char *) pMem->Allocate(sizeof(char), GZIP_CHUNK + 1)) == NULL)
	{
	    inflateEnd(&zStream);
		fclose(fSource);
		fclose(hTarget);
		return this->Locks.UnlockShared(false);
	}

    if((sOutBuf = (char *) pMem->Allocate(sizeof(char), GZIP_CHUNK + 1)) == NULL)
	{
	    inflateEnd(&zStream);
		pMem->Free(sInBuf);
		fclose(fSource);
		fclose(hTarget);
		return this->Locks.UnlockShared(false);
	}

    do {
        zStream.avail_in = (int)fread(sInBuf, 1, GZIP_CHUNK, fSource);

        if (zStream.avail_in == 0)
		{
            break;
		}

        zStream.next_in = (Bytef *) sInBuf;

        do {
            zStream.avail_out = GZIP_CHUNK;
            zStream.next_out = (Bytef *) sOutBuf;

            if((iResult = inflate(&zStream, Z_NO_FLUSH)) != Z_OK)
			{
				if(iResult != Z_STREAM_END)
				{
					break;
				}
			}

			iHave = (GZIP_CHUNK - zStream.avail_out);

			if (fwrite(sOutBuf, 1, iHave, hTarget) != iHave || ferror(hTarget))
			{
                iResult = Z_ERRNO;
				break;
            }

			if(iResult != Z_OK)
			{
				break;
			}
        } while (zStream.avail_out == 0);
    } while (iResult != Z_STREAM_END);

    inflateEnd(&zStream);

	pMem->Free(sInBuf);
    pMem->Free(sOutBuf);

	fclose(fSource);
	fclose(hTarget);

	return this->Locks.UnlockShared((iResult == Z_STREAM_END));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	The CRC must use the official 0x04C11DB7 polynomial.
*/
bool CCompression::Deflate(PEER *pC, const char *sSourceName, const char *sTargetName)
{
	this->Locks.LockShared();

	char *sInBuf = NULL;
	char *sOutBuf = NULL;

	FILE *fSource = NULL;
	FILE *hTarget = NULL;

	int iFlush = 0;
	int iResult = Z_OK;
	int iChunkSize = GZIP_CHUNK;

	unsigned int ulCRC32 = 0xffffffff;

	unsigned int iHave = 0;

	z_stream zStream;

	memset(&zStream, 0, sizeof(zStream));
	
	zStream.zalloc = Z_NULL;
	zStream.zfree = Z_NULL;
	zStream.opaque = Z_NULL;

	if (Z_OK != deflateInit2(&zStream, this->GetFileCompressionLevel(sSourceName),
		Z_DEFLATED, -MAX_WBITS,  // supress zlib-header
		this->Collection.MemoryLevel, Z_DEFAULT_STRATEGY))
	{
		return this->Locks.UnlockShared(false);
	}

	CWebSite *pWebSite = (CWebSite *)pC->pWebSite;
	bool bProcessCustomFilters = pWebSite->pCustomFilters->ContainsCompressionFilter();

	if(bProcessCustomFilters)
	{
		if(!FileSize(sSourceName, &iChunkSize))
		{
			return this->Locks.UnlockShared(false);
		}
	}
	
	if(fopen_s(&fSource, sSourceName, "rb") != 0)
	{
	    deflateEnd(&zStream);
		return this->Locks.UnlockShared(false);
	}

	if(fopen_s(&hTarget, sTargetName, "wb") != 0)
	{
	    deflateEnd(&zStream);
		fclose(fSource);
		return this->Locks.UnlockShared(false);
	}
	
	if((sInBuf = (char *) pMem->Allocate(sizeof(char), iChunkSize + 1)) == NULL)
	{
	    deflateEnd(&zStream);
		fclose(fSource);
		fclose(hTarget);
		return this->Locks.UnlockShared(false);
	}

    if((sOutBuf = (char *) pMem->Allocate(sizeof(char), iChunkSize + 1)) == NULL)
	{
	    deflateEnd(&zStream);
		pMem->Free(sInBuf);
		fclose(fSource);
		fclose(hTarget);
		return this->Locks.UnlockShared(false);
	}

	//Write the GZip header.
	unsigned char gz_header[10];
	unsigned int mtime = 0;
	gz_header[0] = 0x1f;
	gz_header[1] = 0x8b;
	gz_header[2] = Z_DEFLATED;
	gz_header[3] = 0; // options
	gz_header[4] = (mtime >>  0) & 0xff;
	gz_header[5] = (mtime >>  8) & 0xff;
	gz_header[6] = (mtime >> 16) & 0xff;
	gz_header[7] = (mtime >> 24) & 0xff;
	gz_header[8] = 0x00; // extra flags
	gz_header[9] = 0x0b; // OS Code Win32
	fwrite(gz_header, sizeof(unsigned char), sizeof(gz_header), hTarget);

    do {
        zStream.avail_in = (int)fread(sInBuf, 1, iChunkSize, fSource);

		if(bProcessCustomFilters)
		{
			BASICHUNK Chunk;
			pC->pClient->Alloc(&Chunk, zStream.avail_in);
			memcpy_s(Chunk.pBuffer, zStream.avail_in, sInBuf, zStream.avail_in);
			Chunk.iLength = zStream.avail_in;

			pWebSite->pCustomFilters->ProcessOnCompressFilters(pC, &Chunk);

			if(Chunk.iLength != zStream.avail_in)
			{
				sInBuf = (char *)pMem->ReAllocate(sInBuf, Chunk.iLength, 1);
				zStream.avail_in = Chunk.iLength;
			}

			memcpy_s(sInBuf, zStream.avail_in, Chunk.pBuffer, zStream.avail_in);

			pC->pClient->Free(&Chunk);
		}

		Crc32.PartialCRC(&ulCRC32, (const unsigned char *)sInBuf, zStream.avail_in);

		iFlush = feof(fSource) ? Z_FINISH : Z_NO_FLUSH;
        zStream.next_in = (Bytef *)sInBuf;

        do {
            zStream.avail_out = iChunkSize;
            zStream.next_out = (Bytef *) sOutBuf;

			iResult = deflate(&zStream, iFlush);

			iHave = (iChunkSize - zStream.avail_out);
            if(fwrite(sOutBuf, 1, iHave, hTarget) != iHave || ferror(hTarget))
			{
                iResult = Z_ERRNO;
				break;
            }
        } while (zStream.avail_out == 0);

		if(iResult != Z_OK)
		{
			break;
		}
	} while (iFlush != Z_FINISH);

	deflateEnd(&zStream);

	ulCRC32 ^= 0xffffffff;

    //Write the GZip footer.
	unsigned char gz_Footer[8];
	gz_Footer[0] = (ulCRC32 >>  0) & 0xff;
	gz_Footer[1] = (ulCRC32 >>  8) & 0xff;
	gz_Footer[2] = (ulCRC32 >> 16) & 0xff;
	gz_Footer[3] = (ulCRC32 >> 24) & 0xff;
	gz_Footer[4] = (zStream.total_in >>  0) & 0xff;
	gz_Footer[5] = (zStream.total_in >>  8) & 0xff;
	gz_Footer[6] = (zStream.total_in >> 16) & 0xff;
	gz_Footer[7] = (zStream.total_in >> 24) & 0xff;
	fwrite(gz_Footer, sizeof(unsigned char), sizeof(gz_Footer), hTarget);
	
	pMem->Free(sInBuf);
    pMem->Free(sOutBuf);
	
	fclose(fSource);
	fclose(hTarget);

    return this->Locks.UnlockShared((iResult == Z_STREAM_END));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
