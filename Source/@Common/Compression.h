///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _Compression_Types_H
#define _Compression_Types_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define COMPRESSION_LIST_POS_EXT		0
#define COMPRESSION_LIST_POS_LEVEL		1
#define COMPRESSION_LIST_POS_DESCR		2

#define CACHE_LIST_POS_FILENAME			0
#define CACHE_LIST_POS_ORIGINALSIZE		1
#define CACHE_LIST_POS_CACHEDSIZE		2
#define CACHE_LIST_POS_HITS				3

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define COMPRESSION_MAX_EXT_LENGTH		MAX_PATH
#define COMPRESSION_MAX_DESCR_LENGTH	1024

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Compressible_File{
	char *Extension;
	char *Description;
	int CompressionLevel;
	bool Enabled;

#ifndef HTTP_SERVICE
	bool ParentEnabled;
#endif

} COMPRESSIBLEFILE, *LPCOMPRESSIBLEFILE;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag__tag_Compressible_Files{
	int Count;

	bool Enabled;

    bool CompressDynamicContent;
    int MaxCompressionSize;
    int MinCompressionSize;
	int CompressionLevel;
	int MemoryLevel;

	COMPRESSIBLEFILE *Items;

} COMPRESSIBLEFILES, *LPCOMPRESSIBLEFILES;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Compression_Cache_Item{
	char *OriginalFile;
	__int64 OriginalSize;
	__int64 CachedSize;
	int Hits;

#ifdef HTTP_SERVICE
	char *CachedFile;
#endif //HTTP_SERVICE

} COMPRESSIONCACHEITEM, *LPCOMPRESSIONCACHEITEM;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Compression_Cache{
	int Count;
	int MaxCachedFiles;
	char *CachePath;
	COMPRESSIONCACHEITEM *Items;
} COMPRESSIONCACHE, *LPCOMPRESSIONCACHE;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
