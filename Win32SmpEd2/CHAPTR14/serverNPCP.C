/*  Chapter 14. ServerNPCP.
 *	Multi-threaded command line server. Named pipe version, COMPLETION PORT example
 *	Usage:	Server [UserName GroupName]
 *	ONE PIPE INSTANCE FOR EVERY CLIENT. */

#include "EvryThng.h"
#include "ClntSrvr.h" /* Request and Response messages defined here */

typedef struct {				/* Argument to a server thread. */
	HANDLE hCompPort;			/* Completion port handle. */
	DWORD ThreadNo;
	TCHAR TmpFileName [MAX_PATH]; /* Temporary file name. */
} THREAD_ARG;
typedef THREAD_ARG *LPTHREAD_ARG;

typedef struct {	/* Completion port keys point to these structures */
		HANDLE	hNp;	/* which represent outstanding ReadFile  */
		REQUEST	Req;	/* and ConnectNamedPipe operations */
		DWORD Type;		/* 0 for ConnectNamedPipe; 1 for ReadFile */
		OVERLAPPED Ov;
} CP_KEY;

static CP_KEY Key[MAX_CLIENTS_CP]; /* must be available to all threads */


volatile static BOOL ShutDown = FALSE;
static DWORD WINAPI Server (LPTHREAD_ARG);
static DWORD WINAPI ServerBroadcast (LPLONG);
static BOOL  WINAPI Handler (DWORD);
static TCHAR ShutRqst [] = _T ("$ShutDownServer");

_tmain (int argc, LPTSTR argv [])
{
	/* MAX_CLIENTS is defined in ClntSrvr.h. */
	/* Currently limited to MAXIMUM_WAIT_OBJECTS WaitForMultipleObjects */
	/* is used by the main thread to wait for the server threads */

	HANDLE hCp, hMonitor, hSrvrThread [MAX_CLIENTS];
	DWORD iNp, iTh, MonitorId, ThreadId;
	DWORD AceMasks [] =	/* Named pipe access rights */
		{STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0X1FF, 0, 0 };
	LPSECURITY_ATTRIBUTES pNPSA = NULL;
	THREAD_ARG ThArgs [MAX_SERVER_TH];

	/* Console control handler to permit server shutdown */
	if (!SetConsoleCtrlHandler (Handler, TRUE))
		ReportError (_T("Cannot create Ctrl handler"), 1, TRUE);

	/* Pipe security is commented out for simplicity */
//	if (argc == 4)		/* Optional pipe security. */
//		pNPSA = InitializeUnixSA (0440, argv [1], argv [2], AceMasks, &hSecHeap);
			
	/* Create a thread broadcast pipe name periodically. */
	hMonitor = (HANDLE) _beginthreadex (NULL, 0, ServerBroadcast, NULL, 0, &MonitorId);

	hCp = CreateIoCompletionPort (INVALID_HANDLE_VALUE, NULL, 0, MAX_SERVER_TH); 
	if (hCp == NULL) ReportError (_T("Failure to create completion port"), 2, TRUE);

	/*	Create an overlapped named pipe for every potential client, */
	/*	add to the completion port, and wait for a connection */
	/*	The assumption is that the maximum number of clients far exceeds */
	/*	the number of server threads	*/

	for (iNp = 0; iNp < MAX_CLIENTS_CP; iNp++) {
		memset (&Key[iNp], 0, sizeof(CP_KEY));
		Key[iNp].hNp =  CreateNamedPipe ( SERVER_PIPE, 
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_READMODE_MESSAGE | PIPE_TYPE_MESSAGE | PIPE_WAIT,
			MAX_CLIENTS_CP, 0, 0, INFINITE, pNPSA);
		if (Key[iNp].hNp == INVALID_HANDLE_VALUE) 
			ReportError (_T("Error creating named pipe"), 3, TRUE);
		if (CreateIoCompletionPort (Key[iNp].hNp, hCp, iNp, MAX_SERVER_TH+2) == NULL)
			ReportError (_T("Error adding NP handle to CP"), 4, TRUE);
		Key[iNp].Ov.hEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
		if (Key[iNp].Ov.hEvent == NULL) 
			ReportError (_T("Error creating ov event"), 5, TRUE);
		if (!ConnectNamedPipe (Key[iNp].hNp, &Key[iNp].Ov)
			&& GetLastError() != ERROR_IO_PENDING) 
			/* This is a strange value, but it's what's returned */
			ReportError (_T("ConnectNamedPipe error in main thread"), 6, TRUE);
		
	}


	/*	Create server worker threads and a temp file name for each.*/
	for (iTh = 0; iTh < MAX_SERVER_TH; iTh++) {
		ThArgs [iTh].hCompPort = hCp;
		ThArgs [iTh].ThreadNo = iTh;
		GetTempFileName (_T ("."), _T ("CLP"), 0, ThArgs [iTh].TmpFileName);
		hSrvrThread [iTh] = (HANDLE)_beginthreadex (NULL, 0, Server,
				&ThArgs [iTh], 0, &ThreadId);
		if (hSrvrThread [iTh] == NULL)
			ReportError (_T ("Failure to create server thread."), 2, TRUE);
	}

	/* Wait for all the threads to terminate. */

	WaitForMultipleObjects (MAX_SERVER_TH, hSrvrThread, TRUE, INFINITE);
	_tprintf (_T ("All Server worker threads have shut down.\n"));

	WaitForSingleObject (hMonitor, INFINITE);
	_tprintf (_T ("Monitor thread has shut down.\n"));

	CloseHandle (hMonitor);
	for (iTh = 0; iTh < MAX_SERVER_TH; iTh++) { 
		/* Close pipe handles and delete temp files */
		/* Closing temp files is redundant, as the worker threads do it */
		CloseHandle (hSrvrThread [iTh]);
		DeleteFile (ThArgs [iTh].TmpFileName);
	}
	CloseHandle (hCp);

	_tprintf (_T ("Server process will exit.\n"));
	/*	ExitProcess assures a clean exit. All DLL entry points will be 
		called, indicating process detach. Among other reasons that this is
		important is that connection threads may still be blocked on
		ConnectNamedPipe calls. */
	ExitProcess (0);

	return 0;
}

static DWORD WINAPI Server (LPTHREAD_ARG pThArg)

/* Server thread function. . */
{
	/* Each thread keeps its own request, response,
		and bookkeeping data structures on the stack. */

	HANDLE hCp, hTmpFile = INVALID_HANDLE_VALUE;
	HANDLE hWrEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
	DWORD nXfer, KeyIndex, ServerNumber;
	STARTUPINFO StartInfoCh;
	SECURITY_ATTRIBUTES TempSA = {sizeof (SECURITY_ATTRIBUTES), NULL, TRUE};
	PROCESS_INFORMATION ProcInfo;
	FILE *fp;
	RESPONSE Response;
	BOOL Success, Disconnect, Exit = FALSE;
	LPOVERLAPPED pOv;
	OVERLAPPED ovResp = {0, 0, 0, 0, hWrEvent}; /* Used for responses */

	/*	In order to prevent an overlapped operation from being queued on the 
		CP, the event must have the low order bit set. This is a bit strange,
		but it's the way to do it. */
	ovResp.hEvent = (HANDLE)((DWORD)hWrEvent | 0x1);
	GetStartupInfo (&StartInfoCh);
	hCp = pThArg->hCompPort;
	ServerNumber = pThArg->ThreadNo;

	while (!ShutDown && !Exit) __try {	/* Wait for an outstanding operation to complete */
		Success = FALSE; /* set only when everything has succeeded */
		Disconnect = FALSE;

		if (!GetQueuedCompletionStatus (hCp, &nXfer, &KeyIndex, &pOv, INFINITE)
			&& GetLastError() != ERROR_MORE_DATA) continue;
		/* Contrary to the documentation, this will can return a false indicating more data */

		if (Key[KeyIndex].Type == 0) { /* A connection has completed, read a request */
			/* Open the temporary results file for this connection. */
			hTmpFile = CreateFile (pThArg->TmpFileName, GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE, &TempSA,
				CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
			if (hTmpFile == INVALID_HANDLE_VALUE) continue;
			Key[KeyIndex].Type = 1;
			Disconnect = !ReadFile (Key[KeyIndex].hNp, &Key[KeyIndex].Req, 
				RQ_SIZE, &nXfer, &Key[KeyIndex].Ov)
				&& GetLastError() == ERROR_HANDLE_EOF; /* First read on this connection */
			if (Disconnect) continue;
			Success = TRUE;

		} else { /* A read has completed. process the request */

			ShutDown = ShutDown || (_tcscmp (Key[KeyIndex].Req.Record, ShutRqst) == 0);
			if (ShutDown)  continue;

			/* Main command loop */
			/* Create a process to carry out the command. */
			StartInfoCh.hStdOutput = hTmpFile;
			StartInfoCh.hStdError = hTmpFile;
			StartInfoCh.hStdInput = GetStdHandle (STD_INPUT_HANDLE);
			StartInfoCh.dwFlags = STARTF_USESTDHANDLES;

			if (!CreateProcess (NULL, Key[KeyIndex].Req.Record, NULL,
				NULL, TRUE, /* Inherit handles. */
				0, NULL, NULL, &StartInfoCh, &ProcInfo)) {
					PrintMsg (hTmpFile, _T("ERR: Cannot create process."));
					ProcInfo.hProcess = NULL;
			}

			if (ProcInfo.hProcess != NULL ) { /* Server process is running */
				CloseHandle (ProcInfo.hThread);
				WaitForSingleObject (ProcInfo.hProcess, INFINITE);
				CloseHandle (ProcInfo.hProcess);
			}
			
			/* Respond a line at a time. It is convenient to use
				C library line-oriented routines at this point. */

			fp = _tfopen (pThArg->TmpFileName, _T ("r"));
			if (fp == NULL)
				perror ("Failure to open command output file.");
			Response.Status = 0; 
			/* Responses are not queued on the completion port as the 
			   low order bit of the event is set. */
			while (_fgetts (Response.Record, MAX_RQRS_LEN, fp) != NULL) {
				WriteFile (Key[KeyIndex].hNp, &Response, RS_SIZE, &nXfer, &ovResp);
				WaitForSingleObject (hWrEvent, INFINITE);
			}
			fclose (fp);

			/* Erase temp file contents */
			SetFilePointer (hTmpFile, 0, NULL, FILE_BEGIN);
			SetEndOfFile (hTmpFile);

			/* Send an end of response indicator. */
			Response.Status = 1; strcpy (Response.Record, "");
			/* Responses are not queued on the completion port as the 
			   low order bit of the overlapped event is set. */
			WriteFile (Key[KeyIndex].hNp, &Response, RS_SIZE, &nXfer, &ovResp);
			WaitForSingleObject (hWrEvent, INFINITE);
			/* End of main command loop. Get next command */
			Disconnect = !ReadFile (Key[KeyIndex].hNp, &Key[KeyIndex].Req, 
				RQ_SIZE, &nXfer, &Key[KeyIndex].Ov)
				&& GetLastError() == ERROR_HANDLE_EOF; /* Next read on this connection */

			if (Disconnect) continue;
			Success = TRUE;
		}

	} __finally {
		if (Disconnect) {
			Key[KeyIndex].Type = 0;
			DisconnectNamedPipe (Key[KeyIndex].hNp);
			if (!ConnectNamedPipe (Key[KeyIndex].hNp, &Key[KeyIndex].Ov)
					&& GetLastError() != ERROR_PIPE_CONNECTED)
				ReportError (_T("ConnectNamedPipe error in main thread"), 6, TRUE);
		}
		if (!Success) {
			ReportError (_T("Server failure"), 0, TRUE);
			Exit = TRUE;
		}
	}

		/* Client has disconnected or there has been a shutdown requrest */
		/* Terminate this client connection and then wait for another */
	FlushFileBuffers (Key[KeyIndex].hNp);
	DisconnectNamedPipe (Key[KeyIndex].hNp);
	CloseHandle (hTmpFile); 

	printf ("Thread %d shutting down %d\n", pThArg->ThreadNo);
	/* End of command processing loop. Free resources and exit from the thread. */
	if (hTmpFile != INVALID_HANDLE_VALUE) CloseHandle (hTmpFile);
	DeleteFile (pThArg->TmpFileName);
	_tprintf (_T ("Exiting server thread number %d\n"), pThArg->ThreadNo);
	_endthreadex (0);
	return 0;	/* Suppress a compiler warning message. */
}


static DWORD WINAPI ServerBroadcast (LPLONG pNull)
{
	MS_MESSAGE MsNotify;
	DWORD nXfer;
	HANDLE hMsFile;

	/* Open the mailslot for the MS "client" writer. */
	while (!ShutDown) { /* Run as long as there are server threads */
		/* Wait for another client to open a mailslot. */
		Sleep (CS_TIMEOUT);
		hMsFile = CreateFile (MS_CLTNAME, GENERIC_WRITE | GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hMsFile == INVALID_HANDLE_VALUE) continue;

		/* Send out the message to the mailslot. */

		MsNotify.msStatus = 0;
		MsNotify.msUtilization = 0;
		_tcscpy (MsNotify.msName, SERVER_PIPE);
		if (!WriteFile (hMsFile, &MsNotify, MSM_SIZE, &nXfer, NULL))
			ReportError (_T ("Server MS Write error."), 13, TRUE);
		CloseHandle (hMsFile);
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
