/*Chapter 13. ServiceServer.c
	Simplest example of an NT service
	All it does is update the checkpoint counter
	and accept basic controls. */

/* Build as a mutltithreaded console application */

#include "EvryThng.h"
#include "ClntSrvr.h"
#define UPDATE_TIME 1000	/* One second between udpdates */

VOID LogEvent (LPCTSTR, DWORD, BOOL);
void WINAPI ServiceMain (DWORD argc, LPTSTR argv[]);
VOID WINAPI ServerCtrlHandler(DWORD);
void UpdateStatus (int, int);
int  ServiceSpecific (int, LPTSTR *);

volatile static BOOL ShutDown = FALSE;
static FILE *hLogFile; /* Text Log file */
static SERVICE_STATUS hServStatus;
static SERVICE_STATUS_HANDLE hSStat; /* Service status handle for setting status */
									 
static LPTSTR ServiceName = _T("SS");
static LPTSTR LogFileName = "SimpleServiceLog.txt";
									 
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


/*	This is the service-specific function, or "main" and is 
	called from the more generic ServiceMain
	In general, you can take a server, such as ServerMT.c
	of Chapter 11 (book) and rename the "main" as "ServiceSpecific"
	and put the code right here. */

int ServiceSpecific (int argc, LPTSTR argv[])
{	

	UpdateStatus (-1, -1); /* Now change to status; increment the checkpoint */
	LogEvent (_T("Starting main service server loop"), 0, FALSE);

	while (!ShutDown) { /* ShutDown is set on a shutdown control */
		Sleep (UPDATE_TIME);
		UpdateStatus (-1, -1);  /* Increment the checkpoint*/
		LogEvent (_T("Service server loop iteration"), 0, FALSE);
	}

	LogEvent (_T ("\nServer process has shut down."), 0, FALSE);

	return 0;
}



/*	Control Handler Function */
VOID WINAPI ServerCtrlHandler( DWORD Control)
 // requested control code 
{
	switch (Control) {
	case SERVICE_CONTROL_SHUTDOWN:
	case SERVICE_CONTROL_STOP:
		ShutDown = TRUE;	/* Set the global shutdown flag */
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


