///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _ScriptingEngines_Types_H
#define _ScriptingEngines_Types_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SCRIPTINGENGINES_MAX_EXT_LEGTH				MAX_PATH
#define SCRIPTINGENGINES_MAX_PARAMS_LEGTH			MAX_PATH
#define SCRIPTINGENGINES_MAX_ENGINE_LEGTH			MAX_PATH
#define SCRIPTINGENGINES_MAX_DESCR_LEGTH			1024
#define SCRIPTINGENGINES_MAX_STDIO_BUFFER_SIZE		1024

#define EXEC_RESULT_OK						1
#define EXEC_RESULT_ERROR					-1
#define EXEC_RESULT_TIMEOUT					-2
#define EXEC_RESULT_SCRIPT_NOT_FOUND		-3
#define EXEC_RESULT_ENGINE_NOT_FOUND		-4
#define EXEC_RESULT_EXTENSION_NOT_CONFIG	-5
#define EXEC_RESULT_INVALID_ISAPI			-6
#define EXEC_RESULT_NOT_IMPLEMENTED			-7
#define EXEC_RESULT_SSI_FAILED				-8
#define EXEC_RESULT_BAD_RETURN_CODE			-9
#define EXEC_RESULT_NONBLOCK_FAILED			-10
#define EXEC_RESULT_CREATE_PROCESS_FAILED	-11
#define EXEC_RESULT_FAILED_TO_CREATE_PIPE	-12

#define SCRIPTENGINES_LIST_POS_EXT				0
#define SCRIPTENGINES_LIST_POS_DESCR			1
#define SCRIPTENGINES_LIST_POS_PARAMS			2
#define SCRIPTENGINES_LIST_POS_ENGINE			3
#define SCRIPTENGINES_LIST_POS_SUCCESSCODE		4
#define SCRIPTENGINES_LIST_POS_USESUCCESSCODE	5

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Script_Engine{
	char *Extension;
	char *Parameters;
	char *Engine;
	char *Description;
	bool Enabled;
	int SuccessCode;
	bool UseSuccessCode;
#ifndef HTTP_SERVICE
	bool ParentEnabled;
#endif

} SCRIPTENGINE, *LPSCRIPTENGINE;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Script_Engines{
	int Count;
	bool Enabled;
	SCRIPTENGINE *Items;
} SCRIPTENGINES, *LPSCRIPTENGINES;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
