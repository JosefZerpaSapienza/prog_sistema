/* Chapter 12. clientSK.c										*/
/* Single threaded command line client	WINDOWS SOCKETS VERSION	*/
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

static BOOL SendRequestMessage (REQUEST *, SOCKET);
static BOOL ReceiveResponseMessage (RESPONSE *, SOCKET);

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
		SendRequestMessage (&Request, ClientSock);
		ReceiveResponseMessage (&Response, ClientSock);
	}

	shutdown (ClientSock, 2); /* Disallow sends and receives */
	closesocket (ClientSock);
	WSACleanup();
	_tprintf (_T("\n****Leaving client\n"));
	return 0;
}


BOOL SendRequestMessage (REQUEST *pRequest, SOCKET sd)
{
	/* Send the the request to the server on socket sd */
	BOOL Disconnect = FALSE;
	LONG32 nRemainSend, nXfer;
	LPBYTE pBuffer;

	/* The request is sent in two parts. First the header, then
	   the command string proper. */
	
	nRemainSend = RQ_HEADER_LEN; 
	pRequest->Command = 1;  /* This is the only command type for now */
	pRequest->RqLen = strlen (pRequest->Record) + 1;
	pBuffer = (LPBYTE)pRequest;
	while (nRemainSend > 0 && !Disconnect)  {
		/* send does not guarantee that the entire message is sent */
		nXfer = send (sd, pBuffer, nRemainSend, 0);
		if (nXfer == SOCKET_ERROR) ReportError ("client send() failed", 4, TRUE);
		Disconnect = (nXfer == 0);
		nRemainSend -=nXfer; pBuffer += nXfer;
	}

	nRemainSend = pRequest->RqLen; 
	pBuffer = (LPSTR)pRequest->Record;
	while (nRemainSend > 0 && !Disconnect)  {
		nXfer = send (sd, pBuffer, nRemainSend, 0);
		if (nXfer == SOCKET_ERROR) ReportError (_T("client send() failed"), 5, TRUE);
		Disconnect = (nXfer == 0);
		nRemainSend -=nXfer; pBuffer += nXfer;
	}
	return Disconnect;
}


BOOL ReceiveResponseMessage (RESPONSE *pResponse, SOCKET sd)
{
	BOOL Disconnect = FALSE, LastRecord = FALSE;
	LONG32 nRemainRecv = 0, nXfer;
	LPBYTE pBuffer;

	/*  Read the response records - there may be more than one.
		As each is received, write it to std out. */

	/*	Read each response and send it to std out. 
		First, read the record header, and then
		read the rest of the record.  */
		
	while (!LastRecord) {
		/*  Read the header */
		nRemainRecv = RS_HEADER_LEN; pBuffer = (LPBYTE)pResponse;
		while (nRemainRecv > 0 && !Disconnect) {
			nXfer = recv (sd, pBuffer, nRemainRecv, 0);
			if (nXfer == SOCKET_ERROR) ReportError (_T("client header recv() failed"), 6, TRUE);
			Disconnect = (nXfer == 0);
			nRemainRecv -=nXfer; pBuffer += nXfer;
		}
		/*	Read the response record */
		nRemainRecv = pResponse->RsLen; 
		LastRecord = (nRemainRecv <= 1);  /* The terminating null is counted */
		pBuffer = (LPSTR)pResponse->Record;
		while (nRemainRecv > 0 && !Disconnect) {
			nXfer = recv (sd, pBuffer, nRemainRecv, 0);
			if (nXfer == SOCKET_ERROR) ReportError (_T("client response recv() failed"), 7, TRUE);
			Disconnect = (nXfer == 0);
			nRemainRecv -=nXfer; pBuffer += nXfer;
		}
	
		if (!Disconnect) 
			printf ("%s", pResponse->Record);
	}
	return Disconnect;	
}
