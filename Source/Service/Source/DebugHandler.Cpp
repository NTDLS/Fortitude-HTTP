///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#ifndef _DebugHandler_Cpp
#define _DebugHandler_Cpp
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <WindowsX.H>
#include <ShellAPI.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <ConIO.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"
#include "CHttp.H"
#include "CWebSites.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::File;
using namespace NSWFL::ListView;
using namespace NSWFL::DateTime;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DebugConsoleAsk(void)
{
	printf("Are you sure (y/n)?");
	char sBuf[1000];
	gets_s(sBuf, sizeof(sBuf));
	return (tolower(sBuf[0]) == 'y');
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int DebugConsoleAsk(char *sQuestion)
{
	printf(sQuestion);
	char sBuf[1000];
	gets_s(sBuf, sizeof(sBuf));
	return tolower(sBuf[0]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DebugConsoleMore(void)
{
	printf("More? (y/n)");
	char sBuf[1000];
	gets_s(sBuf, sizeof(sBuf));
	printf("\n");
	return tolower(sBuf[0]) != 'n';
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Fortitude_DebugConsoleHandler(const char *sText, int iTextLength)
{
	if(_strcmpi(sText, "?") == 0 || _strcmpi(sText, "Help") == 0)
	{
		printf("Sockets         Lists all socket pools.\n");
		printf("Sites           Lists all sites.\n");
		printf("Stats           Shows basic real-time statistics.\n");
		printf("Stop            Stops all running sites.\n");
		printf("Restart         Restarts all sites, starts ones configured to auto start.\n");
		printf("Start           Starts all sites configured to auto start.\n");
		printf("FlushBuffers    Flushes all file buffers to disk.\n");
		printf("Memory          Displays internal memory manager state.\n");
		printf("  .Allocations  Details each memory allocation. Can be saved to file.\n");
		printf("Cls             Clears the console window.\n");
		printf("Exit            Gracefully shuts down the server.\n");
	}
	else if (_strcmpi(sText, "Stats") == 0)
	{
		unsigned __int64 i64HitsLast = 0;
		unsigned __int64 i64BytesSentLast = 0;
		unsigned __int64 i64BytesRecvLast = 0;
		unsigned __int64 i64TickCount = 0;
		unsigned __int64 i64LastTickCount = 0;

		printf("Press any key to stop monitoring.\n");

		int iKBHit = 0;
		while (iKBHit == 0)
		{
			unsigned __int64 i64BytesSent = 0;
			unsigned __int64 i64BytesRecv = 0;
			unsigned __int64 i64Hits = 0;

			i64TickCount = GetTickCount64();

			for (int i = 0; i < gWebSites->Collection.Count; i++)
			{
				CWebSite *pSite = gWebSites->Collection.WebSite[i];

				i64BytesSent += pSite->Stats.BytesSent;
				i64BytesRecv += pSite->Stats.BytesRecv;
				i64Hits += pSite->Stats.Hits;
			}

			iKBHit = _kbhit();

			double i64Time = ((double)(i64TickCount - i64LastTickCount)) / 1000.0;
			double i64HitsPerSec = ((double)(i64Hits - i64HitsLast)) / i64Time;
			double i64BytesSentPerSec = ((double)(i64BytesSent - i64BytesSentLast)) / i64Time;
			double i64BytesRecvPerSec = ((double)(i64BytesRecv - i64BytesRecvLast)) / i64Time;

			printf("Connections: %.1f/s, Bytes: [In: %.1f/s, Out: %.1f/s]                            \r",
				i64HitsPerSec, i64BytesRecvPerSec, i64BytesSentPerSec);

			i64BytesSentLast = i64BytesSent;
			i64BytesRecvLast = i64BytesRecv;
			i64HitsLast = i64Hits;

			i64LastTickCount = i64TickCount;

			Sleep(1000);
		}

		getch(); // Read the hit key off the console stack and discard it.

	}
	else if(_strcmpi(sText, "Stop") == 0)
	{
		gWebSites->StopAllSites();
		printf("Complete.");
	}
	else if(_strcmpi(sText, "Restart") == 0)
	{
		gWebSites->Reload();
	}
	else if(_strcmpi(sText, "Start") == 0)
	{
		printf("%s", gWebSites->StartAllSites() ? "Success." : "Fail.");
	}
	else if(_strcmpi(sText, "FlushBuffers") == 0)
	{
		printf("Flushed %d open file streams.", _flushall());
	}
	else if(_strcmpi(sText, "Sockets") == 0)
	{
		for(int i = 0; i < gWebSites->pSocketPools->Collection.Count; i++)
		{
			SOCKETPOOL *pSockPool = &gWebSites->pSocketPools->Collection.Items[i];

			printf("Port: %d, Max Connections: %d\n", pSockPool->Port, pSockPool->MaxConnections);
			if(pSockPool->ListenIPs.ListenOnAll)
			{
				printf("\t(All IPs)\n");
			}
			else {
				for(int y = 0; y < pSockPool->ListenIPs.Count; y++)
				{
					LISTENIP *pListenIP = &pSockPool->ListenIPs.Items[y];
					printf("\t%s\n", pListenIP->IP);
				}
			}
		}
	}
	else if(_strcmpi(sText, "Sites") == 0)
	{
		for(int i = 0; i < gWebSites->Collection.Count; i++)
		{
			CWebSite *pWebSite = gWebSites->Collection.WebSite[i];

			printf("%s\n", pWebSite->Name);
			SOCKETPOOL *pSockPool = pWebSite->pSocketPool;
			printf("Port: %d, Max Connections: %d\n", pSockPool->Port, pSockPool->MaxConnections);
			if(pSockPool->ListenIPs.ListenOnAll)
			{
				printf("\t(All IPs)\n");
			}
			else {
				for(int y = 0; y < pSockPool->ListenIPs.Count; y++)
				{
					LISTENIP *pListenIP = &pSockPool->ListenIPs.Items[y];
					printf("\t%s\n", pListenIP->IP);
				}
			}
		}
	}
	else if(_strcmpi(sText, "Memory") == 0)
	{
		printf("Total Allocated Buffers: %zd\n", pMem->GetSlotAllocationCount());
		printf("Total Allocated Buffers (Used): %zd\n", pMem->GetMemAllocationCount());
		printf("Total Allocated Buffers (free): %zd\n", pMem->GetMemFreeCount());
		printf("Total bytes allocated: %zd\n", pMem->ReservedBytes());
	}
	else if(_strcmpi(sText, "Memory.Allocations") == 0)
	{
		FILE *hFile = NULL;

		if(DebugConsoleAsk("Write to file? (y/n)\n") == 'y')
		{
			char sFileName[MAX_PATH];
			memset(sFileName, 0, sizeof(sFileName));
			do
			{
				if(strlen(sFileName) > 0)
				{
					if(DebugConsoleAsk("Access denied, try again?") != 'y')
					{
						hFile = NULL;
						break;
					}
				}

				memset(sFileName, 0, sizeof(sFileName));
				printf("File path: ");
				gets_s(sFileName, sizeof(sFileName));
			} while(fopen_s(&hFile, sFileName, "wb") != 0);
		}

		pMem->Lock();

		for(unsigned int i = 0; i < pMem->Items.iAllocated; i++)
		{
			if(pMem->Items.Array[i].iAddress != 0)
			{
				if(hFile)
				{
					fprintf(hFile, "%zd\t0x%zX\t\"%s\"\t%zd\r\n",
						pMem->Items.Array[i].iSize, pMem->Items.Array[i].iAddress,
						pMem->Items.Array[i].sCodeFile, 	pMem->Items.Array[i].iLineOfCode);
				}
				else {
					printf("%zd bytes at 0x%zX for [%s] on line %zd\n",
						pMem->Items.Array[i].iSize, pMem->Items.Array[i].iAddress,
						pMem->Items.Array[i].sCodeFile, 	pMem->Items.Array[i].iLineOfCode);
					if(i != 0 && (i % 10) == 0)
					{
						if(!DebugConsoleMore())
						{
							break;
						}
					}
				}
			}
		}

		pMem->Unlock();

		if(hFile)
		{
			fclose(hFile);
		}

	}
	else if(_strcmpi(sText, "Cls") == 0)
	{
		system("cls");
	}
	else if(_strcmpi(sText, "Exit") == 0)
	{
		if(DebugConsoleAsk())
		{
			if(gService.RunningAsService())
			{
				gService.Stop();
			}
			else {
				gService.StopRunning();
			}
			return false;
		}
	}
	else {
		printf("Unknown command: [%s]\n", sText);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
#endif
