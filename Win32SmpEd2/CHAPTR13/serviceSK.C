/*Chapter 13. serviceSK.c
	serverSK (Chapter 12) converted to an NT service */

/* Build as a mutltithreaded console application */

/*	Everything at the beginning is a "service wrapper", 
	the same as in SimpleService.c" */
#define _NOEXCLUSIONS
#include "EvryThng.h"
#include "ClntSrvr.h"
#define UPDATE_TIME 1000	/* One second between udpdates */

VOID LogEvent (LPCTSTR, DWORD, BOOL);
void WINAPI ServiceMain (DWORD argc, LPTSTR argv[]);
VOID WINAPI ServerCtrlHandler(DWORD);
void UpdateStatus (int, int);
int  ServiceSpecific (int, LPTSTR *);

static FILE *hLogFile; /* Text Log file */
static SERVICE_STATUS hServStatus;
static SERVICE_STATUS_HANDLE hSStat; /* Service status handle for setting status */
volatile static ShutFlag = FALSE;
									 
static LPTSTR ServiceName = _T("serviceSK");
static LPTSTR LogFileName = "serviceSKLog.txt";
									 
/*  Main routine that starts the service control dispatcher */
VOID _tmain (int argc, LPTSTR argv [])
{
	SERVICE_TABLE_ENTRY DispatchTable[] =
	{
		{ ServiceName,				ServiceMain	},
		{ NULL,						NULL }
	};

	StartServiceCtrlDispatcher (DispatchTable);

	return;
}


/*	ServiceMain entry point, called when the service is created by
	the main program.  */
void WINAPI ServiceMain (DWORD argc, LPTSTR argv[])
{
	DWORD i;

	/*  Set the current directory and open a log file, appending to
		an existing file */
	if (argc > 2) SetCurrentDirectory (argv[2]);
	hLogFile = fopen (LogFileName, _T("w+"));
	if (hLogFile == NULL) return ;

	LogEvent (_T("Starting service. First log entry."), 0, FALSE);
	fprintf (hLogFile, "\nargc = %d", argc);
	for (i = 0; i < argc; i++) 
		fprintf (hLogFile, "\nargv[%d] = %s", i, argv[i]);
	LogEvent (_T("Entering ServiceMain."), 0, FALSE);

	hServStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	hServStatus.dwCurrentState = SERVICE_START_PENDING;
	hServStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | 
		SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE;
	hServStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
	hServStatus.dwServiceSpecificExitCode = 0;
	hServStatus.dwCheckPoint = 0;
	hServStatus.dwWaitHint = 2*CS_TIMEOUT;

	hSStat = RegisterServiceCtrlHandler( ServiceName, ServerCtrlHandler);
	if (hSStat == 0) 
		LogEvent (_T("Cannot register control handler"), 100, TRUE);

	LogEvent (_T("Control handler registered successfully"), 0, FALSE);
	SetServiceStatus (hSStat, &hServStatus);
	LogEvent (_T("Service status set to SERVICE_START_PENDING"), 0, FALSE);

	/*  Start the service-specific work, now that the generic work is complete */
	if (ServiceSpecific (argc, argv) != 0) {
		hServStatus.dwCurrentState = SERVICE_STOPPED;
		hServStatus.dwServiceSpecificExitCode = 1;  /* Server initilization failed */
		SetServiceStatus (hSStat, &hServStatus);
		return;
	}
	LogEvent (_T("Service threads shut down. Set SERVICE_STOPPED status"), 0, FALSE);
	/*  We will only return here when the ServiceSpecific function
		completes, indicating system shutdown. */
	UpdateStatus (SERVICE_STOPPED, 0);
	LogEvent (_T("Service status set to SERVICE_STOPPED"), 0, FALSE);
	fclose (hLogFile);  /*  Clean up everything, in general */
	return;

}


/*	Control Handler Function */
VOID WINAPI ServerCtrlHandler( DWORD Control)
 // requested control code 
{
	switch (Control) {
	case SERVICE_CONTROL_SHUTDOWN:
	case SERVICE_CONTROL_STOP:
		ShutFlag = TRUE;	/* Set the global ShutFlag flag */
		UpdateStatus (SERVICE_STOP_PENDING, -1);
		break;
	case SERVICE_CONTROL_PAUSE:
		break;
	case SERVICE_CONTROL_CONTINUE:
		break;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	default:
		if (Control > 127 && Control < 256) /* User Defined */
		break;
	}
	UpdateStatus (-1, -1);
	return;
}

void UpdateStatus (int NewStatus, int Check)
/*  Set a new service status and checkpoint (either specific value or increment) */
{
	if (Check < 0 ) hServStatus.dwCheckPoint++;
	else			hServStatus.dwCheckPoint = Check;
	if (NewStatus >= 0) hServStatus.dwCurrentState = NewStatus;
	if (!SetServiceStatus (hSStat, &hServStatus))
		LogEvent (_T("Cannot set service status"), 101, TRUE);
	return;
}

/*	LogEvent is similar to the ReportError function used elsewhere
	For a service, however, we ReportEvent rather than write to standard
	error. Eventually, this function should go into the utility
	library.  */

VOID LogEvent (LPCTSTR UserMessage, DWORD ExitCode, BOOL PrintErrorMsg)

/*  General-purpose function for reporting system errors.
	Obtain the error number and turn it into the system error message.
	Display this information and the user-specified message to the open log FILE
	UserMessage:		Message to be displayed to standard error device.
	ExitCode:			0 - Return.
						> 0 - ExitProcess with this code.
	PrintErrorMessage:	Display the last system error message if this flag is set. */
{
	DWORD eMsgLen, ErrNum = GetLastError ();
	LPTSTR lpvSysMsg;
	TCHAR MessageBuffer[512];

//	ALTERNATIVE: Use a registry event log, as shown, or
//		OutputDebugString.
//	HANDLE hEventSource;

//	hEventSource = RegisterEventSource (NULL, ServiceName);
	/*  Not much to do if this fails but to keep trying. */

//	if (hEventSource != NULL) {
//		ReportEvent (hEventSource, EVENTLOG_WARNING_TYPE,
//			0,0, NULL, 1, 0, &UserMessage, NULL);
//	}

	if (PrintErrorMsg) {
		eMsgLen = FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM, NULL,
			ErrNum, MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpvSysMsg, 0, NULL);

		_stprintf (MessageBuffer, _T("\n%s %s ErrNum = %d. ExitCode = %d."),
			UserMessage, lpvSysMsg, ErrNum, ExitCode);
		HeapFree (GetProcessHeap (), 0, lpvSysMsg);
				/* Explained in Chapter 6. */
	} else {
		_stprintf (MessageBuffer, _T("\n%s ExitCode = %d."),
			UserMessage, ExitCode);
	}

	fputs (MessageBuffer, hLogFile);

//		ReportEvent (hEventSource, 
//			ExitCode > 0 ? EVENTLOG_ERROR_TYPE : EVENTLOG_WARNING_TYPE,
//			0, 0, NULL, 1, 0, (LPCTSTR*)&lpvSysMsg, NULL); 

	
	if (ExitCode > 0)
		ExitProcess (ExitCode);
	else
		return;
}


/*	This is the service-specific function, or "main" and is 
	called from the more generic ServiceMain
	It calls a renamed version of serverSK from
	Chapter 12.
	In general, you could use a separate source file, or even
	put this in a DLL.  */

 /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Chapter 12. Client/Server. SERVER PROGRAM.  SOCKET VERSION	*/
/* MODIFIED TO BE A SERVICE										*/
/* Execute the command in the request and return a response.	*/
/* Commands will be exeuted in process if a shared library 	*/
/* entry point can be located, and out of process otherwise	*/
/* ADDITIONAL FEATURE: argv[1] can be the name of a DLL supporting */
/* in process services */

struct sockaddr_in SrvSAddr;		/* Server's Socket address structure */
struct sockaddr_in ConnectSAddr;	/* Connected socket with client details   */
WSADATA WSStartData;				/* Socket library data structure   */

typedef struct SERVER_ARG_TAG { /* Server thread arguments */
	volatile DWORD	number;
	volatile SOCKET	sock;
	volatile DWORD	status; /* 0: Does not exist, 1: Stopped, 2: Running 
				3: Stop entire system */
	volatile HANDLE srv_thd;
	HINSTANCE	 dlhandle; /* Shared libary handle */
} SERVER_ARG;

static BOOL ReceiveRequestMessage (REQUEST *pRequest, SOCKET);
static BOOL SendResponseMessage (RESPONSE *pResponse, SOCKET);
static DWORD WINAPI Server (SERVER_ARG *);
static DWORD WINAPI AcceptTh (SERVER_ARG *);

static HANDLE srv_thd[MAX_CLIENTS];
static SOCKET SrvSock = INVALID_SOCKET, ConnectSock = INVALID_SOCKET;

int ServiceSpecific (int argc, LPTSTR argv[])
/* int _tmain (DWORD argc, LPCTSTR argv []) */
{
	/* Server listening and connected sockets. */
	BOOL Done = FALSE;
	DWORD ith, tstatus, ThId;
	SERVER_ARG srv_arg[MAX_CLIENTS];
	HANDLE hAcceptTh = NULL;
	HINSTANCE hDll = NULL;

	UpdateStatus (-1, -1); /* Now change to status; increment the checkpoint */
	LogEvent (_T("Starting ServiceSpecific"), 0, FALSE);

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
	SrvSock = socket(AF_INET, SOCK_STREAM, 0);
	if (SrvSock == INVALID_SOCKET) 
		ReportError (_T("Failed server socket() call"), 1, TRUE);
    
	/*	Prepare the socket address structure for binding the
	    	server socket to port number "reserved" for this service.
	    	Accept requests from any client machine.  */

    SrvSAddr.sin_family = AF_INET;	
    SrvSAddr.sin_addr.s_addr = htonl( INADDR_ANY );    
    SrvSAddr.sin_port = htons( SERVER_PORT );	
	if (bind (SrvSock, (struct sockaddr *)&SrvSAddr, sizeof SrvSAddr) == SOCKET_ERROR)
		ReportError (_T("Failed server bind() call"), 2, TRUE);
	if (listen (SrvSock, MAX_CLIENTS) != 0) 
		ReportError (_T("Server listen() error"), 3, TRUE);

	/* Main thread becomes listening/connecting/monitoring thread */
	/* Find an empty slot in the server thread arg array */
	/* status values:	0 - Slot is free;	1 - thread stopped; 
						2 - thread running; 3 - Stop entire system */
	while (!ShutFlag) {
		/* The next two lines are added to support a service */
		UpdateStatus (-1, -1);  /* Increment the checkpoint*/
		LogEvent (_T("Service server loop iteration"), 0, FALSE);

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
			UpdateStatus (-1, -1);  /* ADDED TO SERVICE */
			tstatus = WaitForSingleObject (hAcceptTh, CS_TIMEOUT);
			if (tstatus == WAIT_OBJECT_0) break; /* the connection was made */
		}
		CloseHandle (hAcceptTh);
		hAcceptTh = NULL; /* prepare for next connection */
	}
	
	LogEvent (_T("Server shutdown in process. Wait for all server threads\n"), 0, FALSE);
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
	LogEvent (_T ("\nServer process has shut down."), 0, FALSE);

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
	SOCKET ConnectSock;
	int Disconnect = 0, i;
	REQUEST Request;	/* Defined in ClntSrvr.h */
	RESPONSE Response;	/* Defined in ClntSrvr.h.*/
	char sys_command[MAX_RQRS_LEN], TempFile[100];
	HANDLE hTmpFile;
	FILE *fp = NULL;
	int (*dl_addr)(char *, char *);
	char *ws = " \0\t\n"; /* white space */

	GetStartupInfo (&StartInfoCh);
	
	ConnectSock = pThArg->sock;
	/* Create a temp file name */
	sprintf (TempFile, "%s%d%s", "ServerTemp", pThArg->number, ".tmp");

	while (!Done && !ShutFlag) { 	/* Main Command Loop. */

		Disconnect = ReceiveRequestMessage (&Request, ConnectSock);
		UpdateStatus (-1, -1);   /* ADDED TO SERVICE */
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
		if (pThArg->dlhandle != NULL) {/* Try Server "In process" */
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
			while ((fgets (Response.Record, MAX_RQRS_LEN, fp) != NULL)) 
				SendResponseMessage (&Response, ConnectSock);
		}
			/* Send a zero length message */
		strcpy (Response.Record, "");
		SendResponseMessage (&Response, ConnectSock);
		fclose (fp); fp = NULL;
		DeleteFile (TempFile);
		UpdateStatus (-1, -1);	/* ADDED TO SERVICE */

	}   /* End of main command loop. Get next command */

	/* End of command processing loop. Free resources and exit from the thread. */

	_tprintf (_T ("Shuting down server thread number %d\n"), pThArg->number);
	closesocket (ConnectSock);
	shutdown (ConnectSock, 2);
	pThArg->status = 1;
	if (strcmp (Request.Record, "$ShutDownServer") == 0)	{
		pThArg->status = 3;
		ShutFlag = TRUE;
	}
	return pThArg->status;	
}

BOOL ReceiveRequestMessage (REQUEST *pRequest, SOCKET sd)
{
	BOOL Disconnect = FALSE;
	LONG32 nRemainRecv = 0, nXfer;
	LPBYTE pBuffer;

	/*	Read the request. First the header, then the request text. */
	nRemainRecv = RQ_HEADER_LEN; 
	pBuffer = (LPBYTE)pRequest;

	while (nRemainRecv > 0 && !Disconnect) {
		nXfer = recv (sd, pBuffer, nRemainRecv, 0);
		if (nXfer == SOCKET_ERROR) 
			ReportError (_T("server request recv() failed"), 11, TRUE);
		Disconnect = (nXfer == 0);
		nRemainRecv -=nXfer; pBuffer += nXfer;
	}
	
	/*	Read the request record */
	nRemainRecv = pRequest->RqLen;

	pBuffer = (LPSTR)pRequest->Record;
	while (nRemainRecv > 0 && !Disconnect) {
		nXfer = recv (sd, pBuffer, nRemainRecv, 0);
		if (nXfer == SOCKET_ERROR) 
			ReportError (_T("server request recv() failed"), 12, TRUE);
		Disconnect = (nXfer == 0);
		nRemainRecv -=nXfer; pBuffer += nXfer;
	}

	return Disconnect;
}

BOOL SendResponseMessage (RESPONSE *pResponse, SOCKET sd)
{
	BOOL Disconnect = FALSE;
	LONG32 nRemainRecv = 0, nXfer, nRemainSend;
	LPBYTE pBuffer;

	/*	Send the response up to the string end. Send in 
		two parts - header, then the response string. */
	nRemainSend = RS_HEADER_LEN; 
	pResponse->RsLen = strlen (pResponse->Record)+1;
	pBuffer = (LPBYTE)pResponse;
	while (nRemainSend > 0 && !Disconnect) {
		nXfer = send (sd, pBuffer, nRemainSend, 0);
		if (nXfer == SOCKET_ERROR) ReportError (_T("server send() failed"), 13, TRUE);
		Disconnect = (nXfer == 0);
		nRemainSend -=nXfer; pBuffer += nXfer;
	}

	nRemainSend = pResponse->RsLen;
	pBuffer = (LPSTR)pResponse->Record;
	while (nRemainSend > 0 && !Disconnect) {
		nXfer = send (sd, pBuffer, nRemainSend, 0);
		if (nXfer == SOCKET_ERROR) ReportError (_T("server send() failed"), 14, TRUE);
		Disconnect = (nXfer == 0);
		nRemainSend -=nXfer; pBuffer += nXfer;
	}
	return Disconnect;
}

/* REMOVED AS THE SERVICE CONTROL HANDLER TAKES CARE OF THIS
BOOL WINAPI Handler (DWORD CtrlEvent)
*/
