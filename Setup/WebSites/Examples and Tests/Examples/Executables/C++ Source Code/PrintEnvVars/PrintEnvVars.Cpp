/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <StdIO.H>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int iArg, char *sArg[])
{

	printf("Content-Type: Text/Html\n\n");

	printf("<html>\r\n");

	printf("<head>\r\n");
	printf("	<title>Environment Strings</title>\r\n");
	printf("</head>\r\n\r\n");

	printf("<body>\r\n");
	printf("<center><h2>Environment Strings</h2></center>\r\n<br />\r\n\r\n");
	printf("<hr />\r\n<br />\r\n");

	char *sEnv = GetEnvironmentStrings();
	for(int iOffset = 0, iCount = 0;
		((char *)(sEnv + iOffset))[0] != '\0';
		iOffset += strlen(((char *)(sEnv + iOffset))) + 1, iCount++)
	{
		printf("<b>%d:</b> [%s]<br />\r\n", iCount, ((char *)sEnv + iOffset));
	}

	printf("</body>\r\n\r\n");

	printf("</html>\r\n");

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
