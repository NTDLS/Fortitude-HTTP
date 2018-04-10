///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CRYPTOGRAPHY_CPP
#define _CRYPTOGRAPHY_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>

#ifdef HTTP_SERVICE
#include "../Service/Source/Entry.H"
#else
#include "../Console/Source/Entry.H"
#endif

#include "../../../@Libraries/NSWFL/NSWFL.h"
#include "../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../@Libraries/CSHA1/CSHA1.h"
#include "Cryptography.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::System;
using namespace NSWFL::String;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Makes a new key based on a salt, returns a 40+1 character password (key+NULL);
*/
bool MakeEncryptionKey(char *sNewKey, int iNewKeySz, const char *sFormat, ...)
{
	bool bResult = false;
	va_list ArgList;
	va_start(ArgList, sFormat);

	int iMemoryRequired = _vscprintf(sFormat, ArgList);

	#ifdef _USE_GLOBAL_MEMPOOL
		char *sSuppliedKey = (char *) pMem->Allocate(sizeof(char), iMemoryRequired + 1);
	#else
		char *sSuppliedKey = (char *) calloc(sizeof(char), iMemoryRequired + 1);
	#endif

	int iSz = _vsprintf_s_l(sSuppliedKey, iMemoryRequired + 1, sFormat, NULL, ArgList);
	va_end(ArgList);

	char sHostName[256];
	if(Get_MachineName(sHostName, sizeof(sHostName)))
	{
		UCase(sHostName);
		CStringBuilder localKey;
		localKey.AppendF("%s\\%s\\%s", sSuppliedKey, GLOBAL_CRPYO_KEY, sHostName);
		bResult = SimpleSHA1(localKey.Buffer, localKey.Length, sNewKey, iNewKeySz);
	}

	#ifdef _USE_GLOBAL_MEMPOOL
		pMem->Free(sSuppliedKey);
	#else
		free(sSuppliedKey);
	#endif

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
