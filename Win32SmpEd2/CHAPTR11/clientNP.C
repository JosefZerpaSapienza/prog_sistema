/* Chapter 11 - Client/Server system. CLIENT VERSION.
	ClientNP - Connection-oriented client. Named Pipe version */
/* Execute a command line (on the server) and display the response. */

/*  The client creates a long-lived
	connection with the server (consuming a pipe instance) and prompts
	the user for the command to execute. */

/* This program illustrates:
	1. Named pipes from the client side. 
	2. Long-lived connections with a single request but multiple responses.
	3. Reading responses in server messages until and end of response is received.
*/

/* Special commands recognized by the server are:
	1. $Statistics: Give performance statistics.
	2. $ShutDownThread: Terminate a server thread.
	3. $Quit. Exit from the client. */

#include "EvryThng.h"
#include "ClntSrvr.h" /* Defines the resquest and response records */

int _tmain (int argc, LPTSTR argv [])
{
	HANDLE hNamedPipe;
	TCHAR PromptMsg [] = _T ("\nEnter Command: ");
	TCHAR QuitMsg [] = _T ("$Quit");
	TCHAR ServerPipeName [MAX_PATH];
	REQUEST Request;		/* See ClntSrvr.h */
	RESPONSE Response;		/* See ClntSrvr.h */
	DWORD nRead, nWrite;

	LocateServer (ServerPipeName);
	WaitNamedPipe (ServerPipeName, NMPWAIT_WAIT_FOREVER);
	hNamedPipe = CreateFile (ServerPipeName, GENERIC_READ | GENERIC_WRITE, 0, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hNamedPipe == INVALID_HANDLE_VALUE)
		ReportError (_T ("Failure to locate server."), 3, TRUE);

	/* Prompt the user for commands. Terminate on "$quit". */
	Request.Command = 0;
	Request.RqLen = RQ_SIZE;
	while (ConsolePrompt (PromptMsg, Request.Record, MAX_RQRS_LEN, TRUE)
			&& (_tcscmp (Request.Record, QuitMsg) != 0)) {

		if (!WriteFile (hNamedPipe, &Request, RQ_SIZE, &nWrite, NULL))
			ReportError (_T ("Write NP failed"), 0, TRUE);

		/* Read each response and send it to std out
			Response.Status == 0 indicates "end of response". */

		while (ReadFile (hNamedPipe, &Response, RS_SIZE, &nRead, NULL)
				&& (Response.Status == 0))
			_tprintf (_T ("%s"), Response.Record);

	}

	_tprintf (_T("Quit command received. Disconnect."));;

	CloseHandle (hNamedPipe);

	return 0;
}

