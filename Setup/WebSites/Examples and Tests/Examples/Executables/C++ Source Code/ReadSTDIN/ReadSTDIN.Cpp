#ifndef _ENTRY_CPP
#define _ENTRY_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    char *buffer, *contentLength = getenv("CONTENT_LENGTH"); 
    int length; 
     
    if(!contentLength)
	{
		printf("Nothing was posted.");
		return 0;
    }else{ 
        length = atoi(contentLength); 
    } 
     
    buffer = (char *) calloc(length + 1, 1); 
     
	printf("Content-Type: Text/Html\n\n");

	if(fread(buffer, 1, length, stdin) != length)
	{
		printf("Failed to read post data.");
		return 0;
	}

	printf("Read %d bytes: \"%s\"", length, buffer);

    free(buffer); 

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
