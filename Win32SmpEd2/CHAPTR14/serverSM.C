/*  Chapter 14. ServerSM.
 *	ALTERNATIVE TO I/O COMPLETION PORTS. LIMIT THE NUMBER 
 *	OF READY THREADS WITH A SEMAPHORE.
 *	Multi-threaded command line server. Named pipe version
 *	Usage:	ServerSM [UserName GroupName]
 *	ONE THREAD AND PIPE INSTANCE FOR EVERY CLIENT. */

#include "EvryThng.h"
#include "ClntSrvr.h" /* Request and Response messages defined here */

#define MAX_READY_THREADS 4

typedef struct {				/* Argument to a server thread. */
	HANDLE hNamedPipe;			/* Named pipe instance. */
	HANDLE hSem;				/* Semaphore to limit number of ready threads */
	DWORD ThreadNo;
	TCHAR TmpFileName [MAX_PATH]; /* Temporary file name. */
} THREAD_ARG;
typedef THREAD_ARG *LPTHREAD_ARG;

volatile static BOOL ShutDown = FALSE;
static DWORD WINAPI Server (LPTHREAD_ARG);
static DWORD WINAPI ServerBroadcast (LPLONG);
static BOOL  WINAPI Handler (DWORD);

_tmain (int argc, LPTSTR argv [])
{
	/* MAX_CLIENTS is defined in ClntSrvr.h. */
	/* Currently limited to MAXIMUM_WAIT_OBJECTS WaitForMultipleObjects */
	/* is used by the main thread to wait for the server threads */

	HANDLE hNp, hMonitor, hSrvrThread [MAX_CLIENTS], hS;
	DWORD iNp, MonitorId, ThreadId;
	DWORD AceMasks [] =	/* Named pipe access rights */
		{STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0X1FF, 0, 0 };
	LPSECURITY_ATTRIBUTES pNPSA = NULL;
	THREAD_ARG ThArgs [MAX_CLIENTS];

	/* Console control handler to permit server shutdown */
	if (!SetConsoleCtrlHandler (Handler, TRUE))
		ReportError (_T("Cannot create Ctrl handler"), 1, TRUE);

	/* Pipe security is commented out for simplicity */
//	if (argc == 4)		/* Optional pipe security. */
//		pNPSA = InitializeUnixSA (0440, argv [1], argv [2], AceMasks, &hSecHeap);
			
	/* Create a thread broadcast pipe name periodically. */
	hMonitor = (HANDLE) _beginthreadex (NULL, 0, ServerBroadcast, NULL, 0, &MonitorId);

	/* Create the thread limiting semaphore */
	hS = CreateSemaphore (NULL, MAX_READY_THREADS, MAX_READY_THREADS, NULL);

	/*	Create a pipe instance for every server thread.
	 *	Create a temp file name for each thread.
	 *	Create a thread to service that pipe. */

	for (iNp = 0; iNp < MAX_CLIENTS; iNp++) {
		hNp = CreateNamedPipe ( SERVER_PIPE, PIPE_ACCESS_DUPLEX,
				PIPE_READMODE_MESSAGE | PIPE_TYPE_MESSAGE | PIPE_WAIT,
				MAX_CLIENTS, 0, 0, INFINITE, pNPSA);

		if (hNp == INVALID_HANDLE_VALUE)
			ReportError (_T ("Failure to open named pipe."), 1, TRUE);
		ThArgs [iNp].hNamedPipe = hNp;
		ThArgs [iNp].ThreadNo = iNp;
		ThArgs [iNp].hSem = hS;
		GetTempFileName (_T ("."), _T ("CLP"), 0, ThArgs [iNp].TmpFileName);
		hSrvrThread [iNp] = (HANDLE)_beginthreadex (NULL, 0, Server,
				&ThArgs [iNp], 0, &ThreadId);
		if (hSrvrThread [iNp] == NULL)
			ReportError (_T ("Failure to create server thread."), 2, TRUE);
	}
	
	/* Wait for all the threads to terminate. */

	WaitForMultipleObjects (MAX_CLIENTS, hSrvrThread, TRUE, INFINITE);
	WaitForSingleObject (hMonitor, INFINITE);
	CloseHandle (hMonitor);
	for (iNp = 0; iNp < MAX_CLIENTS; iNp++) { 
		/* Close pipe handles and delete temp files */
		CloseHandle (hSrvrThread [iNp]);
		if (!DeleteFile (ThArgs [iNp].TmpFileName))
			ReportError (_T("Error deleting temp file"), 1, TRUE);
	}

	_tprintf (_T ("Server process has shut down.\n"));

	return 0;
}

static DWORD WINAPI Server (LPTHREAD_ARG pThArg)

/* Server thread function. There is a thread for every potential client. */
{
	/* Each thread keeps its own request, response,
		and bookkeeping data structures on the stack. */

	HANDLE hNamedPipe, hTmpFile;
	DWORD nXfer, PrevCount;
	TCHAR ShutRqst [] = _T ("$ShutDownServer");
	STARTUPINFO StartInfoCh;
	SECURITY_ATTRIBUTES TempSA = {sizeof (SECURITY_ATTRIBUTES), NULL, TRUE};
	PROCESS_INFORMATION ProcInfo;
	FILE *fp;
	REQUEST Request;
	RESPONSE Response;

	GetStartupInfo (&StartInfoCh);
	hNamedPipe = pThArg->hNamedPipe;

				/* Wait for a connection. */
	ConnectNamedPipe (hNamedPipe, NULL);

	while (!ShutDown) { 	/* Main Command Loop. */

		/* Open the temporary results file. */
		hTmpFile = CreateFile (pThArg->TmpFileName, GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE, &TempSA,
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hTmpFile == INVALID_HANDLE_VALUE)
			ReportException (_T("Cannot create temp file"), 1);

		/* Wait for a client request. Reconnect if the client has terminated. */
		while (!ReadFile (hNamedPipe, &Request, RQ_SIZE, &nXfer, NULL)) {
			DisconnectNamedPipe (hNamedPipe);
			ConnectNamedPipe (hNamedPipe, NULL);
		}

		if (_tcscmp (Request.Record, ShutRqst) == 0) {		/* Terminate the system. */
			ShutDown = TRUE;
			continue;
		}
		/* Limit the number of ready threads */
		WaitForSingleObject (pThArg->hSem, INFINITE);
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
			
		/* Release the semaphore */
		ReleaseSemaphore (pThArg->hSem, 1, &PrevCount);
		/* Respond a line at a time. It is convenient to use
			C library line-oriented routines at this point. */

		fp = _tfopen (pThArg->TmpFileName, _T ("r"));
		if (fp == NULL)
			perror ("Failure to open command output file.");
		Response.Status = 0; 
		while (_fgetts (Response.Record, MAX_RQRS_LEN, fp) != NULL) 
			WriteFile (hNamedPipe, &Response, RS_SIZE, &nXfer, NULL); 
		fclose (fp);

		/* Send an end of response indicator. */

		Response.Status = 1; strcpy (Response.Record, "");
		WriteFile (hNamedPipe, &Response, RS_SIZE, &nXfer, NULL);
		FlushFileBuffers (hNamedPipe);
	}   /* End of main command loop. Get next command */

	/* End of command processing loop. Free resources and exit from the thread. */

	_tprintf (_T ("Shuting down server thread number %d\n"), pThArg->ThreadNo);
	if (!DisconnectNamedPipe (hNamedPipe) || !CloseHandle (hNamedPipe))
		ReportError (_T ("Error Stopping server thread."), 0, TRUE);
	_endthreadex (0);
	return 0;	/* Suppress a compiler warning message. */
}


static DWORD WINAPI ServerBroadcast (LPLONG pNull)
{
	BOOL exit = FALSE;
	MS_MESSAGE MsNotify;
	DWORD nXfer;
	HANDLE hMsFile;

	/* Open the mailslot for the MS "client" writer. */
	while (!ShutDown) { /* Run as long as there are server threads */
		exit = FALSE;
		while (!exit) { /* Wait for a client to create a MS. */
			hMsFile = CreateFile (MS_CLTNAME, GENERIC_WRITE | GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hMsFile == INVALID_HANDLE_VALUE) {
				Sleep (CS_TIMEOUT);
			}
			else exit = TRUE;
	  }
		/* Send out the message to the mailslot. */

		MsNotify.msStatus = 0;
		MsNotify.msUtilization = 0;
		_tcscpy (MsNotify.msName, SERVER_PIPE);
		if (!WriteFile (hMsFile, &MsNotify, MSM_SIZE, &nXfer, NULL))
			ReportError (_T ("Server MS Write error."), 13, TRUE);
		CloseHandle (hMsFile);

		/* Wait for another client to open a mailslot. */
		Sleep (CS_TIMEOUT);

	}
	_tprintf (_T ("Shuting down monitor thread.\n"));

	_endthreadex (0);
	return 0;
}

BOOL WINAPI Handler (DWORD CtrlEvent)
{
	/* Shutdown the system */
	printf ("In console control handler\n");
	ShutDown = TRUE;
	return TRUE;
}
