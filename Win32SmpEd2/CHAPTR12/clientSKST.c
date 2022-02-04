/* Chapter 12. clientSKST.c										*/
/* Single threaded command line client	WINDOWS SOCKETS VERSION	*/
/* THIS DIFFERS FROM clientSK.c ONLY IN THAT THE MESSAGE SEND	*/
/* and receive functions are now in a DLL, and the functions	*/
/* are renamed.													*/
/* Reads a sequence of commands to sent to a server process 	*/
/* over a socket connection. Wait for the response and display	*/
/* it.								*/

/* This program illustrates:
	1. Windows sockets from the client side.
	2. Short-lived connections with a single request but multiple responses.
	3. Reading responses in server messages until the server disconnects. */

#define _NOEXCLUSIONS

#include "EvryThng.h"
#include "ClntSrvr.h"	/* Defines the request and response records. */

static BOOL ReceiveResponseMessage (RESPONSE *, SOCKET);

_declspec (dllimport) BOOL SendCSMessage (REQUEST *, SOCKET);
_declspec (dllimport) BOOL ReceiveCSMessage (RESPONSE *, SOCKET);

struct sockaddr_in ClientSAddr;		/* Clients's Socket address structure */

int _tmain (DWORD argc, LPTSTR argv [])
{
	SOCKET ClientSock = INVALID_SOCKET;
	REQUEST Request;	/* See clntcrvr.h */
	RESPONSE Response;	/* See clntcrvr.h */
	WSADATA WSStartData;				/* Socket library data structure   */

	BOOL Quit = FALSE;
	DWORD ConVal, j;
	TCHAR PromptMsg [] = _T("\nEnter Command> "), Req[MAX_RQRS_LEN];
	TCHAR QuitMsg [] = _T("$Quit"); /* Request to shut down client */
	TCHAR ShutMsg [] = _T("$ShutDownServer"); /* Stop all threads */
	CHAR DefaultIPAddr[] = "127.0.0.1";

	/*	Initialize the WS library. Ver 2.0 */
	if (WSAStartup (MAKEWORD (2, 0), &WSStartData) != 0)
		ReportError (_T("Cannot support sockets"), 1, TRUE);
		
	/* Connect to the server */
	/* Follow the standard client socket/connect sequence */
	ClientSock = socket(AF_INET, SOCK_STREAM, 0);
	if (ClientSock == INVALID_SOCKET)
		ReportError (_T("Failed client socket() call"), 2, TRUE);

	memset (&ClientSAddr, sizeof(ClientSAddr), 0);    
	ClientSAddr.sin_family = AF_INET;	
	if (argc >= 2) 
		ClientSAddr.sin_addr.s_addr = inet_addr (argv[1]); 
	else
		ClientSAddr.sin_addr.s_addr = inet_addr (DefaultIPAddr);

	ClientSAddr.sin_port = htons(SERVER_PORT);

	ConVal = connect (ClientSock, (struct sockaddr *)&ClientSAddr, sizeof(ClientSAddr));
	if (ConVal == SOCKET_ERROR) ReportError ("Failed client connect() call", 3, TRUE);

	/*  Main loop to prompt user, send request, receive response */
	while (!Quit) {
		_tprintf (_T("%s"), PromptMsg); 
		/* Generic input, but command to server must be ASCII */
		_fgetts (Req, MAX_RQRS_LEN-1, stdin);
		for (j = 0; j <= _tcslen(Req); j++) Request.Record[j] = Req[j];
		/* Get rid of the new line at the end */
		Request.Record[strlen(Request.Record)-1] = '\0';
		if (strcmp (Request.Record, QuitMsg) == 0 ||
			strcmp (Request.Record, ShutMsg) == 0) Quit = TRUE;
		SendCSMessage (&Request, ClientSock);
		ReceiveResponseMessage (&Response, ClientSock);
	}

	shutdown (ClientSock, 2); /* Disallow sends and receives */
	closesocket (ClientSock);
	WSACleanup();
	_tprintf (_T("\n****Leaving client\n"));
	return 0;
}

BOOL ReceiveResponseMessage (RESPONSE *pResponse, SOCKET sd)
{
	BOOL Disconnect = FALSE, LastRecord = FALSE;

	/*  Read the response records - there may be more than one.
		As each is received, write it to std out. */
		
	while (!LastRecord) {
		Disconnect = ReceiveCSMessage (pResponse, sd);
		/* Detect an end message message */
		/* Arbitrarily defined as "$$$$$$$" */
		LastRecord = (strcmp (pResponse->Record, "$$$$$$$") == 0);
		if (!Disconnect && !LastRecord) 
			printf ("%s", pResponse->Record);
	}
	return Disconnect;	
}
