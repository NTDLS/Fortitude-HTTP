#ifndef _Common_Constants_H
#define _Common_Constants_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_HTTP_HEADER					1024 * 2
#define MAX_FILE_BUFFER					1048576 //One megabyte
#define MAX_QUERY_STRING				2048
#define MAX_URI_LEN						1024 + MAX_QUERY_STRING
//#define MAX_HTTP_METHOD_NAME			10
//#define MAX_HTTP_VERSION_NAME			15
#define MAX_IP_ADDRESS					16
#define MAX_POST_DATA					10240
#define MAX_RESPONSE_CODE				10
#define DFLT_MANAGEMENT_PORT			35634

//#define MAX_ACCEPT_STRING				1024
//#define MAX_LANGUAGE_STRING			1024
//#define MAX_ACCEPT_ENCODING_STRING	1024
//#define MAX_USER_AGENT_STRING			1024
#define MAX_HOST_STRING					255
//#define MAX_CONNECTION_STRING			100

#define HTTP_VERSION					"HTTP/1.1"
#define CGIINTERFACE					"CGI/1.1"
#define MIMEVERSION						"1.1"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
