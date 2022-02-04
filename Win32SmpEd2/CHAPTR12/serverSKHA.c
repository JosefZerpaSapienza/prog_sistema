/* Chapter 12. serverSKHA.c													*/
/* Client/Server. SERVER PROGRAM.  SOCKET, STREAM VERSION					*/
/*		THIS VERSSION ALSO USES A "SOCKET HANDLE" TO PRESERVE STATE			*/
/*		IT IS A SLIGHT VARIATION OF serverSKST.c							*/
/* Execute the command in the request and return a response.				*/
/* Commands will be exeuted in process if a shared library					*/
/* entry point can be located, and out of process otherwise					*/
/* ADDITIONAL FEATURE: argv[1] can be the name of a DLL supporting			*/
/* in process services														*/
/* This implementation differs from serverSK in that it uses a				*/
/* DLL to support socket I/O; the DLL provided uses streams with			*/
/* end of lines to terminate messages, rather than use length headers		*/
/* The differences are all hidden in ReceiveCSMessage and					*/
/* SendCSMessage (renamings from serverSK) so that there are no differences in */
/* the main program. Just the function implementations have been removed	*/
/* and the two message functions are imported */

#define _NOEXCLUSIONS
#include "EvryThng.h"
#include "ClntSrvr.h"	/* Defines the request and response records. */

struct sockaddr_in SrvSAddr;		/* Server's Socket address structure */
struct sockaddr_in ConnectSAddr;	/* Connected socket with client details   */
WSADATA WSStartData;				/* Socket library data structure   */

typedef struct SERVER_ARG_TAG { /* Server thread arguments */
	volatile DWORD	number;
	volatile SOCKET sock;
	volatile DWORD	status; /* 0: Does not exist, 1: Stopped, 2: Running 
				3: Stop entire system */
	volatile HANDLE srv_thd;
	HINSTANCE	 dlhandle; /* Shared libary handle */
} SERVER_ARG;

_declspec (dllimport) PVOID CreateCSSocketHandle (SOCKET);
_declspec (dllimport) BOOL CloseCSSocketHandle (PVOID);
_declspec (dllimport) BOOL ReceiveCSMessage (REQUEST *pRequest, PVOID);
_declspec (dllimport) BOOL SendCSMessage (RESPONSE *pResponse, PVOID);
static DWORD WINAPI Server (SERVER_ARG *);
static DWORD WINAPI AcceptTh (SERVER_ARG *);
static BOOL  WINAPI Handler (DWORD);

static HANDLE srv_thd[MAX_CLIENTS];
volatile static ShutFlag = FALSE;
static SOCKET SrvSock = INVALID_SOCKET, ConnectSock = INVALID_SOCKET;

int _tmain (DWORD argc, LPCTSTR argv [])
{
	/* Server listening and connected sockets. */
	BOOL Done = FALSE;
	DWORD ith, tstatus, ThId;
	SERVER_ARG srv_arg[MAX_CLIENTS];
	HANDLE hAcceptTh = NULL;
	HINSTANCE hDll = NULL;

	/* Console control handler to permit server shutdown */
	if (!SetConsoleCtrlHandler (Handler, TRUE))
		ReportError (_T("Cannot create Ctrl handler"), 1, TRUE);

	/*	Initialize the WS library. Ver 1.1 */
	if (WSAStartup (MAKEWORD (1, 1), &WSStartData) != 0)
		ReportError (_T("Cannot support sockets"), 1, TRUE);
	
	/* Open the shared command library DLL if it is specifiec on command line */
	if (argc > 1) {
		hDll = LoadLibrary (argv[1]);
		if (hDll == NULL) ReportError (argv[1], 0, TRUE);
	}

	/* Intialize thread arg array */
	for (ith = 0; ith < MAX_CLIENTS; ith++) {
		srv_arg[ith].number = ith;
		srv_arg[ith].status = 0;
		srv_arg[ith].sock = 0;
		srv_arg[ith].dlhandle = hDll;
		srv_arg[ith].srv_thd = NULL;
	}
	/*	Follow the standard server socket/bind/listen/accept sequence */
	SrvSock = socket(PF_INET, SOCK_STREAM, 0);
	if (SrvSock == INVALID_SOCKET) 
		ReportError (_T("Failed server socket() call"), 1, TRUE);
    
	/*	Prepare the socket address structure for binding the
	    	server socket to port number "reserved" for this service.
	    	Accept requests from any client machine.  */

    SrvSAddr.sin_family = AF_INET;	
    SrvSAddr.sin_addr.s_addr = htonl( INADDR_ANY );    
    SrvSAddr.sin_port = htons( SERVER_PORT );	
	if (bind (SrvSock, (struct sockaddr *)&SrvSAddr, sizeof(SrvSAddr)) == SOCKET_ERROR)
		ReportError (_T("Failed server bind() call"), 2, TRUE);
	if (listen (SrvSock, MAX_CLIENTS) != 0) 
		ReportError (_T("Server listen() error"), 3, TRUE);

	/* Main thread becomes listening/connecting/monitoring thread */
	/* Find an empty slot in the server thread arg array */
	/* status values:	0 - Slot is free;	1 - thread stopped; 
						2 - thread running; 3 - Stop entire system */
	while (!ShutFlag) {
		for (ith = 0; ith < MAX_CLIENTS && !ShutFlag; ) {
			if (srv_arg[ith].status == 1 || srv_arg[ith].status == 3) {
				/* This thread stopped, either normally or with a shutdown request */
				tstatus = WaitForSingleObject (srv_thd[ith], INFINITE);
				if (tstatus != WAIT_OBJECT_0) 
					ReportError (_T("Server thread wait error"), 4, TRUE);
				CloseHandle (srv_thd[ith]);
				if (srv_arg[ith].status == 3) ShutFlag = TRUE;
				else srv_arg[ith].status = 0; /* Free thread slot */
			}				
			if (srv_arg[ith].status == 0 || ShutFlag) break;
			ith = (ith+1) % MAX_CLIENTS;
			if (ith == 0) Sleep(1000); /* Break the polling loop */
			/* An alternative would be to use an event to signal a free slot */
		}

		/* Wait for a connection on this socket */
		/* Use a separate thread so that we can poll the ShutFlag flag */
		hAcceptTh = (HANDLE)_beginthreadex (NULL, 0, AcceptTh, &srv_arg[ith], 0, &ThId);
		while (!ShutFlag) {
			tstatus = WaitForSingleObject (hAcceptTh, CS_TIMEOUT);
			if (tstatus == WAIT_OBJECT_0) break; /* the connection was made */
		}
		CloseHandle (hAcceptTh);
		hAcceptTh = NULL; /* prepare for next connection */
	}
	
	_tprintf (_T("Server shutdown in process. Wait for all server threads\n"));
	/* Terminate the accept thread if it is still running */
	if (hAcceptTh != NULL) TerminateThread (hAcceptTh, 0);
	/* Wait for any active server threads to terminate */
	shutdown (SrvSock, 2);
	closesocket (SrvSock);
	WSACleanup();
	for (ith = 0; ith < MAX_CLIENTS; ith++) {
		if (srv_arg[ith].status != 0) WaitForSingleObject (srv_thd[ith], INFINITE);
		CloseHandle (srv_thd[ith]);
	}
	if (hDll != NULL) FreeLibrary (hDll);	
	_tprintf (_T("All server resources deallocated. ExitProcess(0)\n"));
	ExitProcess(0);
	return 0;
}

static DWORD WINAPI AcceptTh (SERVER_ARG * pThArg)
{
	LONG AddrLen, ThId;
	
	AddrLen = sizeof(ConnectSAddr);
	pThArg->sock = 
		 accept (SrvSock, (struct sockaddr *)&ConnectSAddr, &AddrLen);
	if (pThArg->sock == INVALID_SOCKET) ReportError ("accept error", 1, TRUE);
	/* A new connection. Create a server thread */
	pThArg->status = 2; 
	pThArg->srv_thd = (HANDLE)_beginthreadex (NULL, 0, Server, pThArg, 0, &ThId);
	if (pThArg->srv_thd == NULL) 
		ReportError (_T("Failed creating server thread"), 0, TRUE);
	return 0; /* Server thread remains running */
}


static DWORD WINAPI Server (SERVER_ARG * pThArg)

/* Server thread function. There is a thread for every potential client. */
{
	/* Each thread keeps its own request, response,
		and bookkeeping data structures on the stack. */

	BOOL Done = FALSE;
	STARTUPINFO StartInfoCh;
	SECURITY_ATTRIBUTES TempSA = {sizeof (SECURITY_ATTRIBUTES), NULL, TRUE};
	PROCESS_INFORMATION ProcInfo;
	SOCKET ConnectSck;
	PVOID sh; /* Socket handle pointer */
	int Disconnect = 0, i;
	REQUEST Request;	/* Defined in ClntSrvr.h */
	RESPONSE Response;	/* Defined in ClntSrvr.h.*/
	char sys_command[MAX_RQRS_LEN], TempFile[100];
	HANDLE hTmpFile;
	FILE *fp = NULL;
	int (*dl_addr)(char *, char *);
	char *ws = " \0\t\n"; /* white space */

	GetStartupInfo (&StartInfoCh);
	
	ConnectSck = pThArg->sock;
	/* Create a streaming socket "handle" */
	sh = CreateCSSocketHandle (ConnectSck);
	/* Create a temp file name */
	sprintf (TempFile, "%s%d%s", "ServerTemp", pThArg->number, ".tmp");

	while (!Done && !ShutFlag) { 	/* Main Command Loop. */

		Disconnect = ReceiveCSMessage (&Request, sh);
		Done = Disconnect || (strcmp (Request.Record, "$Quit") == 0)
			|| (strcmp (Request.Record, "$ShutFlagServer") == 0);
		if (Done) continue;	
		/* Stop this thread on "$Quit" or "$ShutDownServer" command. */

		/* Open the temporary results file. */
		hTmpFile = CreateFile (TempFile, GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE, &TempSA,
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hTmpFile == INVALID_HANDLE_VALUE)
			ReportError (_T("Cannot create temp file"), 1, TRUE);

		/* Check for a shared library command. For simplicity, shared 	*/
		/* library commands take precedence over process commands 	*/
		/* First, extract the command name				*/

		i = strcspn (Request.Record, ws); /* length of token */
		memcpy (sys_command, Request.Record, i);
		sys_command[i] = '\0';

		dl_addr = NULL; /* will be set if GetProcAddress succeeds */
		if (pThArg->dlhandle != NULL) { /* Try Server "In process" */
			dl_addr = (int (*)(char *, char *))GetProcAddress (pThArg->dlhandle, sys_command);
			if (dl_addr != NULL) __try { /* Protect server process from exceptions in DLL */
				(*dl_addr)(Request.Record, TempFile);
			}
			__except (EXCEPTION_EXECUTE_HANDLER) { /* Exception in the DLL */
				ReportError (_T("Exception in DLL."), 0, FALSE);
			}
		}
			

		if (dl_addr == NULL) { /* No inprocess support */
			/* Create a process to carry out the command. */
			StartInfoCh.hStdOutput = hTmpFile;
			StartInfoCh.hStdError = hTmpFile;
			StartInfoCh.hStdInput = GetStdHandle (STD_INPUT_HANDLE);
			StartInfoCh.dwFlags = STARTF_USESTDHANDLES;

			if (!CreateProcess (NULL, Request.Record, NULL,
					NULL, TRUE, /* Inherit handles. */
					0, NULL, NULL, &StartInfoCh, &ProcInfo)) {
				PrintMsg (hTmpFile, _T("ERR: Cannot create process."));
				ProcInfo.hProcess = NULL;
			}
			CloseHandle (hTmpFile);
			if (ProcInfo.hProcess != NULL ) {
				CloseHandle (ProcInfo.hThread);
				WaitForSingleObject (ProcInfo.hProcess, INFINITE);
				CloseHandle (ProcInfo.hProcess);
			}
		}
			
		/* Respond a line at a time. It is convenient to use
			C library line-oriented routines at this point. */

		/* Send the temp file, one line at a time, with header, to the client. */

		fp = fopen (TempFile, "r");
		
		if (fp != NULL) {
			Response.RsLen = MAX_RQRS_LEN;
			while ((fgets (Response.Record, MAX_RQRS_LEN, fp) != NULL)) {
				SendCSMessage (&Response, sh);
			}
		}
		/* Send an end message message */
		/* Arbitrarily defined as "$$$$$$$" */
		strcpy (Response.Record, "$$$$$$$");
		SendCSMessage (&Response, sh);
		fclose (fp); fp = NULL;
		DeleteFile (TempFile); 

	}   /* End of main command loop. Get next command */

	/* End of command processing loop. Free resources and exit from the thread. */

	_tprintf (_T ("Shuting down server thread number %d\n"), pThArg->number);
	CloseCSSocketHandle (sh);
	closesocket (ConnectSock);
	shutdown (ConnectSock, 2);
	pThArg->status = 1;
	if (strcmp (Request.Record, "$ShutDownServer") == 0)	{
		pThArg->status = 3;
		ShutFlag = TRUE;
	}

	return pThArg->status;	
}


BOOL WINAPI Handler (DWORD CtrlEvent)
{
	/* Shutdown the system */
	ShutFlag = TRUE;
	return TRUE;
}
