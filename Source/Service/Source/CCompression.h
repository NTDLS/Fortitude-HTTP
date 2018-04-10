///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CCompression_H
#define _CCompression_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"
#include "CServerSettings.H"
#include "CSocketPools.H"

#include "../../../../@Libraries/CXML/CXMLReader.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/CLocks/CLocks.H"
#include "../../../../@Libraries/CCRC32/CCRC32.H"
#include "../../../../@Libraries/CStack/CStack.H"

#include "../../@Common/Compression.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define GZIP_CHUNK 16384
#define CRC32C(c,d) (c=(c>>8)^crc_c[(c^(d))&0xFF])

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCompression {

private:
	bool Initialized;
	void *pWebSites;
	CCRC32 Crc32;
	COMPRESSIBLEFILES Collection;
	COMPRESSIONCACHE Cache;
	char sFileName[MAX_PATH];

	CLocks Locks;

	bool ResetCacheFileTime(COMPRESSIONCACHEITEM *lpItem);
	bool HasCacheFileChanged(COMPRESSIONCACHEITEM *lpItem);

	int GetLeastHitItem(void);
	bool FreeCacheItem(COMPRESSIONCACHEITEM *lpItem);
	int GetFileCompressionLevel(const char *sFileName);

	CRITICAL_SECTION CS;

public:
	~CCompression(void);
	CCompression(void *lpWebSites);
	CCompression(void *lpWebSites, CXMLReader *xmlConfig, CCompression *pDefaults);

	bool Reload(void);
	bool Destroy(void);
	bool Load(const char *sXMLFileName);
	bool Load(CXMLReader *xmlConfig, CCompression *pDefaults);

	__int64 GetCompressedCacheSize(void);
	__int64 GetNativeCacheSize(void);
	double GetCacheCompressionRatio(void);
	int CachedFiles(void);
	int MaxCachedFiles(void);
	int MaxCompressionSize(void);
	int MinCompressionSize(void);
	int CompressionLevel(void);
	int MemoryLevel(void);
	char *CachePath(void);
	bool Enabled(void);
	bool CompressDynamicContent(void);

	bool Deflate(PEER *pC, const char *sSourceName, const char *sTargetName);
	bool Inflate(const char *sSourceName, const char *sTargetName);

	bool GetUniqueFileName(char *sTarget, int iMaxTargetSz);
	bool GetUniqueFileName(char *sTarget, int iMaxTargetSz, const char *sOriginalFileName);

	bool RemoveFileFromCache(const char *sOriginalFile);
	bool CompressFile(PEER *pC, const char *sSource, char *sTarget, int iMaxTargetSz);
	bool IsFileCompressible(const char *sFileName);
	bool IsCompressibleType(const char *sFileName);
	bool FlushCache(void);

	bool Save(void);
	bool ToXML(CXMLReader *lpXML);
	bool CacheToXML(CXMLReader *lpXML);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
