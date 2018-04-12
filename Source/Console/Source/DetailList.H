///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _DetailList_H
#define _DetailList_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>

#include "../../../NSWFL/NSWFL.h"

#include "../../@Common/Authentication.h"
#include "../../@Common/CGIFolders.h"
#include "../../@Common/Compression.h"
#include "../../@Common/CustomFilters.h"
#include "../../@Common/DefaultPages.h"
#include "../../@Common/ErrorPages.h"
#include "../../@Common/ExtentionFilters.h"
#include "../../@Common/IPFilters.H"
#include "../../@Common/MimeTypes.h"
#include "../../@Common/Realms.h"
#include "../../@Common/ScriptingEngines.h"
#include "../../@Common/WebsiteSettings.H"
#include "../../@Common/SSIFiles.h"
#include "../../@Common/URLFilters.h"
#include "../../@Common/VirtualRoots.h"
#include "../../@Common/WebSite.h"
#include "../../@Common/ServerSettings.h"
#include "../../@Common/SocketPools.H"
#include "../../@Common/HostHeaders.H"

#include "../../../CTabControl/CTabControl.h"

using namespace NSWFL::XML;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FreeLParamMemory(HWND hList);
void FreeLParamMemory(HWND hList, int iRow);

void FreeComboBoxMemory(HWND hCombo);
LPARAM GetSelectedComboBoxItemData(HWND hCombo);
bool SelectComboBoxItemByData(HWND hCombo, const char *sData);

bool LoadNewHostHeadersConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML);
void PopHostHeadersList(HWND hList, bool bUseIcons, XMLReader *pXML);
void PopHostHeadersList_AddItem(HWND hList, HOSTHEADER *obj, int iInsertionIndex);
void PopHostHeadersList_AddItem(HWND hList, HOSTHEADER *obj);

void PopSocketPoolsList_AddItem(HWND hList, SOCKETPOOL *obj, const char *sXML);
void PopSocketPoolsList_AddItem(HWND hList, SOCKETPOOL *obj, int iInsertionIndex, const char *sXML);
void PopSocketPoolsList(HWND hList, bool bUseIcons, XMLReader *pXML, bool bSaveXMLInLParam);
bool LoadNewSocketPoolsConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML);

void PopListeningIPsList_AddItem(HWND hList, LISTENIP *obj);
void PopListeningIPsList_AddItem(HWND hList, LISTENIP *obj, int iInsertionIndex);
void PopListeningIPsList(HWND hList, bool bUseIcons, XMLReader *pXML);
bool LoadNewListeningIPsConfiguration(HWND hWnd, HWND hGrid, bool bListenOnAll, XMLWriter *pXML);

void PopWebsitesList(HWND hList, bool bUseIcons, XMLReader *pXML, bool bSaveXMLInLParam);
void PopWebsitesList_AddItem(HWND hList, WEBSITE *obj, int iInsertionIndex, const char *sXML);
void PopWebsitesList_AddItem(HWND hList, WEBSITE *obj, const char *sXML);
bool LoadNewWebSitessConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML);

void PopCacheList_AddItem(HWND hList, COMPRESSIONCACHEITEM *obj);
void PopCacheList(HWND hList, XMLReader *pXML);

void PopWebsite_AddItem(HWND hList, const char *sAttribute, const char *sValue);
void PopWebsiteOverviewList(HWND hList, XMLReader *pXML);

void PopWebsiteSettingsList(HWND hList, XMLReader *pXML);
void PopWebsiteSettingsList_AddItem(HWND hList, const char *sAttribute, const char *sValue);
void PopWebsiteSettingsList_AddItem(HWND hList, const char *sAttribute, bool bValue);
void PopWebsiteSettingsList_AddItem(HWND hList, const char *sAttribute, double dValue);
void PopWebsiteSettingsList_AddItem(HWND hList, const char *sAttribute, unsigned int iValue);
void PopWebsiteSettingsList_AddItem(HWND hList, const char *sAttribute, int iValue);
bool LoadNewWebsiteSettingsConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML, CTabControl *TabControl);

void PopCompressionList(HWND hList, bool bUseIcons, XMLReader *pXML);
void PopCompressionList_AddItem(HWND hList, COMPRESSIBLEFILE *obj);
void PopCompressionList_AddItem(HWND hList, COMPRESSIBLEFILE *obj, int iInsertionIndex);
bool LoadNewCompressionConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML);

void PopErrorPagesList(HWND hList, bool bUseIcons, XMLReader *pXML);
void PopErrorPagesList_AddItem(HWND hList, ERRORPAGE *obj);
void PopErrorPagesList_AddItem(HWND hList, ERRORPAGE *obj, int iInsertionIndex);
bool LoadNewErrorPagesConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML);

void PopCGIFoldersList(HWND hList, bool bUseIcons, XMLReader *pXML);
void PopCGIFoldersList_AddItem(HWND hList, CGIFOLDER *obj);
void PopCGIFoldersList_AddItem(HWND hList, CGIFOLDER *obj, int iInsertionIndex);
bool LoadNewCGIFoldersConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML);

void PopSSIFilesList(HWND hList, bool bUseIcons, XMLReader *pXML);
void PopSSIFilesList_AddItem(HWND hList, SSIFILE *obj);
void PopSSIFilesList_AddItem(HWND hList, SSIFILE *obj, int iInsertionIndex);
bool LoadNewSSIFilesConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML);

void PopScriptingEnginesList(HWND hList, bool bUseIcons, XMLReader *pXML);
void PopScriptingEnginesList_AddItem(HWND hList, SCRIPTENGINE *obj);
void PopScriptingEnginesList_AddItem(HWND hList, SCRIPTENGINE *obj, int iInsertionIndex);
bool LoadNewScriptingEnginesConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML);

void PopCustomFiltersList(HWND hList, bool bUseIcons, XMLReader *pXML);
void PopCustomFiltersList_AddItem(HWND hList, CUSTOMFILTER *obj);
void PopCustomFiltersList_AddItem(HWND hList, CUSTOMFILTER *obj, int iInsertionIndex);
bool LoadNewCustomFiltersConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML);

void PopExtensionFiltersList(HWND hList, bool bUseIcons, XMLReader *pXML);
void PopExtensionFiltersList_AddItem(HWND hList, EXTENSIONFILTER *obj);
void PopExtensionFiltersList_AddItem(HWND hList, EXTENSIONFILTER *obj, int iInsertionIndex);
bool LoadNewExtensionFiltersConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML);

void PopURLFiltersList(HWND hList, bool bUseIcons, XMLReader *pXML);
void PopURLFiltersList_AddItem(HWND hList, URLFILTER *obj);
void PopURLFiltersList_AddItem(HWND hList, URLFILTER *obj, int iInsertionIndex);
bool LoadNewURLFiltersConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML);

void PopIPFiltersList(HWND hList, bool bUseIcons, XMLReader *pXML);
void PopIPFiltersList_AddItem(HWND hList, IPFILTER *obj);
void PopIPFiltersList_AddItem(HWND hList, IPFILTER *obj, int iInsertionIndex);
bool LoadNewIPFiltersConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML);

void PopMimeTypesList(HWND hList, bool bUseIcons, XMLReader *pXML);
void PopMimeTypesList_AddItem(HWND hList, MIMETYPE *obj);
void PopMimeTypesList_AddItem(HWND hList, MIMETYPE *obj, int iInsertionIndex);
bool LoadNewMimeTypesConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML);

void PopVirtualRootsList(HWND hList, bool bUseIcons, XMLReader *pXML);
void PopVirtualRootsList_AddItem(HWND hList, ROOT *obj, bool bSaveCopyOfPlainTextPassword);
void PopVirtualRootsList_AddItem(HWND hList, ROOT *obj, bool bSaveCopyOfPlainTextPassword, int iInsertionIndex);
bool LoadNewVirtualRootsConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML);

void PopDefaultPagesList(HWND hList, bool bUseIcons, XMLReader *pXML);
void PopDefaultPagesList_AddItem(HWND hList, DEFAULTPAGE *obj);
void PopDefaultPagesList_AddItem(HWND hList, DEFAULTPAGE *obj, int iInsertionIndex);
bool LoadNewDefaultPagesConfiguration(HWND hWnd, HWND hGrid, XMLWriter *pXML);

void PopServerSettingsList_User(HWND hList, bool bUseIcons, XMLReader *pXML);
void PopServerSettingsList_User_AddItem(HWND hList, SERVERUSER *obj, bool bSaveCopyOfPlainTextPassword);
void PopServerSettingsList_User_AddItem(HWND hList, SERVERUSER *obj, bool bSaveCopyOfPlainTextPassword, int iInsertionIndex);
bool LoadNewServerSettings_User_Configuration(HWND hWnd, HWND hGrid, XMLWriter *pXML);

void RefreshServerPerformanceList(HWND hList, XMLReader *pXML);
void PopServerPerformanceList(HWND hList, XMLReader *pXML);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

