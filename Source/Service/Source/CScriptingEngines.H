///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CScriptingEngines_H
#define _CScriptingEngines_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../../@Libraries/CXML/CXMLReader.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/CLocks/CLocks.H"
#include "../../../../@Libraries/CStack/CStack.H"

#include "../../@Common/ScriptingEngines.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CScriptingEngines{

private:
	bool Initialized;
	void *pWebSites;
	SCRIPTENGINES Collection;
	char sFileName[MAX_PATH];

	CLocks Locks;

	bool AddEnvVar(CStringBuilder *lpBuf, const char *sVarName, const int iVarData);
	bool AddEnvVar(CStringBuilder *lpBuf, const char *sVarName, const char *sVarData);
	bool AddEnvVar(CStringBuilder *lpBuf, const char *sVarName, const char *sVarData, int iVarLength);
	bool AddEnvVars(VOID *pClient, CStringBuilder *lpStr, const char *sScriptFileName);
	bool CopyEnvVar(CStringBuilder *lpStr, const char *sVarName);
	bool AddRawEnvVar(CStringBuilder *lpBuf, const char *sVarNameAndValue);
	bool AddRawEnvVar(CStringBuilder *lpBuf, const char *sVarNameAndValue, int iVarNameAndValueLength);

	int ProcessCGI(VOID *pClient, CStringBuilder *lpOutput, const char *sScriptFile,
		const char *sEngine, const char *sParams, bool bIsNativeExecutable, int iSuccessCode, bool bUseSuccessCode);
	int ProcessFastCGI(VOID *pClient, CStringBuilder *lpOutput,
		const char *sScriptFile, const char *sEngine, const char *sParams);

public:
	CScriptingEngines(void *lpWebSites);
	CScriptingEngines(void *lpWebSites, CXMLReader *xmlConfig, CScriptingEngines *pDefaults);
	~CScriptingEngines(void);
	bool Save(void);

	bool ToXML(CXMLReader *lpXML);

	bool GetScriptEngine(const char *sFileName, char *sEngine, int iMaxEngine,
		char *sParams, int iMaxParams, int *iSuccessCode, bool *bUseSuccessCode, bool *bIsFastCGI);
	bool IsNativeExecutable(const char *sFileName);
	bool IsScriptFile(const char *sFileName);
	bool Load(const char *sXMLFileName);
	bool Load(CXMLReader *xmlConfig, CScriptingEngines *pDefaults);
	bool Reload(void);

	bool Destroy(void);
	bool Enabled(void);
	int ProcessScript(VOID *pClient);
	int ProcessScript(VOID *pClient, CStringBuilder *lpOutput, const char *sScriptFile);

	const char *ErrorString(int iErrorCode);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
