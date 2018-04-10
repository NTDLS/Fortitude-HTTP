///////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <WindowsX.H>
#include <Stdio.H>
#include <Stdlib.H>

///////////////////////////////////////////////////////////////////////////////////////////

#define MAXSIZE 1024

///////////////////////////////////////////////////////////////////////////////////////////

DWORD GetEnvVar(const char *sVarName, char *sVarData, int iMaxSize)
{
	memset(sVarData, 0, iMaxSize);
	return GetEnvironmentVariable(sVarName, sVarData, iMaxSize);
}

///////////////////////////////////////////////////////////////////////////////////////////

int main(int iArg, char *sArg[])
{
	char sVarData[MAXSIZE];
	char *sVarName = NULL;

	printf("Content-Type: Text/Html\n\n");

	printf("<html>\r\n");

	printf("<head>\r\n");
	printf("	<title>Console Child Process CGI</title>\r\n");
	printf("</head>\r\n\r\n");

	printf("<body>\r\n");

	printf("<center><h2>Console Child Process CGI</h2></center>\r\n<br />\r\n\r\n");

	printf("<hr />\r\n<br />\r\n");

	sVarName = "REQUEST_METHOD";
	GetEnvVar(sVarName, sVarData, sizeof(sVarData));
	printf("%s = %s<br />\r\n", sVarName, sVarData);

	sVarName = "CONTENT_LENGTH";
	GetEnvVar(sVarName, sVarData, sizeof(sVarData));
	printf("%s = %s<br />\r\n", sVarName, sVarData);

	sVarName = "QUERY_STRING";
	GetEnvVar(sVarName, sVarData, sizeof(sVarData));
	printf("%s = %s<br />\r\n", sVarName, sVarData);

	sVarName = "SERVER_SOFTWARE";
	GetEnvVar(sVarName, sVarData, sizeof(sVarData));
	printf("%s = %s<br />\r\n", sVarName, sVarData);

	sVarName = "SERVER_NAME";
	GetEnvVar(sVarName, sVarData, sizeof(sVarData));
	printf("%s = %s<br />\r\n", sVarName, sVarData);

	sVarName = "GATEWAY_INTERFACE";
	GetEnvVar(sVarName, sVarData, sizeof(sVarData));
	printf("%s = %s<br />\r\n", sVarName, sVarData);

	sVarName = "SERVER_PORT";
	GetEnvVar(sVarName, sVarData, sizeof(sVarData));
	printf("%s = %s<br />\r\n", sVarName, sVarData);

	sVarName = "PATH_TRANSLATED";
	GetEnvVar(sVarName, sVarData, sizeof(sVarData));
	printf("%s = %s<br />\r\n", sVarName, sVarData);

	sVarName = "REMOTE_HOST";
	GetEnvVar(sVarName, sVarData, sizeof(sVarData));
	printf("%s = %s<br />\r\n", sVarName, sVarData);

	sVarName = "REMOTE_ADDR";
	GetEnvVar(sVarName, sVarData, sizeof(sVarData));
	printf("%s = %s<br />\r\n", sVarName, sVarData);

	sVarName = "SCRIPT_NAME";
	GetEnvVar(sVarName, sVarData, sizeof(sVarData));
	printf("%s = %s<br />\r\n", sVarName, sVarData);

	sVarName = "HTTP_ACCEPT";
	GetEnvVar(sVarName, sVarData, sizeof(sVarData));
	printf("%s = %s<br />\r\n", sVarName, sVarData);

	sVarName = "HTTP_ACCEPT_LANGUAGE";
	GetEnvVar(sVarName, sVarData, sizeof(sVarData));
	printf("%s = %s<br />\r\n", sVarName, sVarData);

	sVarName = "CONTENT_TYPE";
	GetEnvVar(sVarName, sVarData, sizeof(sVarData));
	printf("%s = %s<br />\r\n", sVarName, sVarData);

	sVarName = "SERVER_ADMIN";
	GetEnvVar(sVarName, sVarData, sizeof(sVarData));
	printf("%s = %s<br />\r\n", sVarName, sVarData);

	sVarName = "DOCUMENT_ROOT";
	GetEnvVar(sVarName, sVarData, sizeof(sVarData));
	printf("%s = %s<br />\r\n", sVarName, sVarData);

	for(int iCmd = 0; iCmd < iArg; iCmd++)
	{
		printf("Cmd Line[%d] = %s<br />\r\n", iCmd, sArg[iCmd]);
	}

	printf("</body>\r\n\r\n");

	printf("</html>\r\n");

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////

