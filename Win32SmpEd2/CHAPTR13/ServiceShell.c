/*	Chapter 13 */
/*	ServiceShell.c  NT Service Management shell program.
	This program modifies Chapter 7's Job Management program,
	managing services rather than jobs. */
/*	Commands supported are:
		create		Create a service
		delete		Delete a service
		start		Start a service
		control		Control a service */

#include "EvryThng.h"

static int Create   (int, LPTSTR *, LPTSTR);
static int Delete   (int, LPTSTR *, LPTSTR);
static int Start    (int, LPTSTR *, LPTSTR);
static int Control  (int, LPTSTR *, LPTSTR);

static SC_HANDLE hScm;
static BOOL Debug;

int _tmain (int argc, LPTSTR argv [])
{
	BOOL Exit = FALSE;
	TCHAR Command [MAX_COMMAND_LINE+10], *pc;
	DWORD i, LocArgc; /* Local argc */
	TCHAR argstr [MAX_ARG] [MAX_COMMAND_LINE];
	LPTSTR pArgs [MAX_ARG];

	Debug = (argc > 1); /* simple debug flag */
	/*  Prepare the local "argv" array as pointers to strings */
	for (i = 0; i < MAX_ARG; i++) pArgs[i] = argstr[i];

	/*  Open the SC Control Manager on the local machine,
		with the default database, and all access. */
	hScm = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hScm == NULL) ReportError (_T("Cannot open SC Manager"), 1, TRUE);

	/*  Main command proceesing loop  */
	_tprintf (_T("\nNT Service Mangement"));
	while (!Exit) {
		_tprintf (_T("\nSM$"));
		_fgetts (Command, MAX_COMMAND_LINE, stdin);
		/*  Replace the new line character with a string end. */
		pc = strchr (Command, '\n'); *pc = '\0';

		if (Debug) printf ("%s\n", Command); 
		/*  Convert the command to "argc, argv" form. */
		GetArgs (Command, &LocArgc, pArgs);
		CharLower (argstr [0]);  /* The command is case-insensitive */

		if (Debug) printf ("\n%s %s %s %s", argstr[0], argstr[1],
			argstr[2], argstr[3]);

		if (_tcscmp (argstr[0], _T("create")) == 0) {
			Create (LocArgc, pArgs, Command);
		}
		else if (_tcscmp (argstr[0], _T("delete")) == 0) {
			Delete (LocArgc, pArgs, Command);
		}
		else if (_tcscmp (argstr[0], _T("start")) == 0) {
			Start (LocArgc, pArgs, Command);
		}
		else if (_tcscmp (argstr[0], _T("control")) == 0) {
			Control (LocArgc, pArgs, Command);
		}
		else if (_tcscmp (argstr[0], _T("quit")) == 0) {
			Exit = TRUE;
		}
		else _tprintf (_T("\nCommand not recognized"));
	}

	CloseServiceHandle (hScm);
	return 0;
}


int Create (int argc, LPTSTR argv [], LPTSTR Command)
{
	/*  Create a new service as a "demand start" service:
		argv[1]: Service Name
		argv[2]: Display Name
		argv[3]: binary executable */
	SC_HANDLE hSc;
	TCHAR CurrentDir [MAX_PATH+1], Executable [MAX_PATH+1];

	if (argc < 4) {
		_tprintf (_T("\nUsage: create ServiceName, DisplayName, .exe"));
		return 1;
	}

	/*  Append the current working directory if the executable name
		is a relative name. Insert a \ if necessary. */
	if (argv[3][1] != _T(':')) {
		GetCurrentDirectory (MAX_PATH, CurrentDir);
		if (argv[3][0] == _T('\\'))
			_stprintf (Executable, _T("%s%s"), CurrentDir, argv[3]);
		else
			_stprintf (Executable, _T("%s\\%s"), CurrentDir, argv[3]);
	}
	else _stprintf (Executable, _T("%s"), argv[3]);

	if (Debug) _tprintf (_T("\nService Full Path Name: %s"), Executable);

	hSc = CreateService (hScm, argv[1], argv[2],
		SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, 
		SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
		Executable, NULL, NULL, NULL, NULL, NULL);
	if (hSc == NULL) ReportError (_T("Cannot create service"), 0, TRUE);
	else CloseServiceHandle (hSc); /* No need to retain the handle as
								   OpenService will query the service DB */
	return 0;

}

/*  Delete a service 
		argv[1]: ServiceName to delete  */

int Delete (int argc, LPTSTR argv [], LPTSTR Command)
{
	SC_HANDLE hSc;

	if (Debug) _tprintf (_T("\nAbout to delete service: %s"), argv[1]);

	hSc = OpenService(hScm,  argv[1], DELETE);
	if (hSc == NULL) {
		_tprintf (_T("\nService Management Error"));
		ReportError (_T("\nCannot open named service for deletion"), 0, TRUE);
	}
	else {
		if (!DeleteService (hSc)) 
			ReportError (_T("\nCannot delete service"), 0, TRUE);
		CloseServiceHandle (hSc);
		if (Debug) _tprintf (_T("\nService %s deleted if no error msg"), argv[1]);
	}

	return 0;
}

/*  Start a named service.
		argv[1]: Service name to start */
int Start (int argc, LPTSTR argv [], LPTSTR Command)
{
	SC_HANDLE hSc;
	TCHAR WorkingDir [MAX_PATH+1];
	LPTSTR pWorkingDir = WorkingDir;
	LPTSTR argvStart[] = {argv[1], WorkingDir, NULL};

	GetCurrentDirectory (MAX_PATH+1, WorkingDir);
	if (Debug) _tprintf (_T("\nAbout to start service: %s in directory: %s"),
		argv[1], WorkingDir);
	

	hSc = OpenService(hScm,  argv[1], SERVICE_ALL_ACCESS);
	if (hSc == NULL) {
		_tprintf (_T("\nService Management Error"));
		ReportError (_T("\nCannot open named service for startup"), 0, TRUE);
	}
	else {
		/*  Start the service with one arg, the working directory */
		if (!StartService (hSc, 2, argvStart)) {
			ReportError (_T("\nCannot start service"), 0, TRUE);
		}
		CloseServiceHandle (hSc);
		if (Debug) _tprintf (_T("\nService %s started if no error msg"), argv[1]);
	}

	return 0;
}

/*  Control a named service.
		argv[1]:  Service name to control 
		argv[2]:  Control command (case insenstive):
					stop
					pause
					resume
					interrogate
					user  user defined
		argv[3]:    a number from 128 to 255, if the control command is "user"
					*/
static LPCTSTR Commands [] = 
	{"stop", "pause", "resume", "interrogate", "user" };
static DWORD Controls [] = {
	SERVICE_CONTROL_STOP, SERVICE_CONTROL_PAUSE,
	SERVICE_CONTROL_CONTINUE, SERVICE_CONTROL_INTERROGATE, 128 };

int Control (int argc, LPTSTR argv [], LPTSTR Command)
{

	SC_HANDLE hSc;
	SERVICE_STATUS ServiceStatus;
	DWORD dwControl, i;
	BOOL Found = FALSE;

	if (Debug) _tprintf (_T("\nAbout to control service: %s"), argv[1]);

	for (i= 0; i < sizeof(Controls)/sizeof(DWORD) && !Found; i++)
		Found = (_tcscmp (Commands[i], argv[2]) == 0);
	if (!Found) {
		_tprintf (_T("\nIllegal Control Command %s"), argv[1]);
		return 1;
	}
	dwControl = Controls[i-1];
	if (dwControl == 128) dwControl = _ttoi (argv[3]);
	if (Debug) _tprintf (_T("\ndwControl = %d"), dwControl);

	hSc = OpenService(hScm,  argv[1],
		SERVICE_INTERROGATE | SERVICE_PAUSE_CONTINUE | 
		SERVICE_STOP | SERVICE_USER_DEFINED_CONTROL | 
		SERVICE_QUERY_STATUS );
	if (hSc == NULL) {
		_tprintf (_T("\nService Management Error"));
		ReportError (_T("\nCannot open named service for control"), 0, TRUE);
	}
	else {
		if (!ControlService (hSc, dwControl, &ServiceStatus))
			ReportError (_T("\nCannot control service"), 0, TRUE);
		if (Debug) _tprintf (_T("\nService %s controlled if no error msg"), argv[1]);
	}

	if (dwControl == SERVICE_CONTROL_INTERROGATE) {
		if (!QueryServiceStatus (hSc, &ServiceStatus))
			ReportError (_T("\nCannot query service status"), 0, TRUE);
		printf (_T("\nStatus from QueryServiceStatus"));
		printf (_T("\nSerice Status"));
		printf (_T("\ndwServiceType: %d"),             ServiceStatus.dwServiceType);
		printf (_T("\ndwCurrentState: %d"),            ServiceStatus.dwCurrentState);
		printf (_T("\ndwControlsAccepted: %d"),        ServiceStatus.dwControlsAccepted);
		printf (_T("\ndwWin32ExitCode: %d"),           ServiceStatus.dwWin32ExitCode);
		printf (_T("\ndwServiceSpecificExitCode: %d"), ServiceStatus.dwServiceSpecificExitCode);
		printf (_T("\ndwCheckPoint: %d"),              ServiceStatus.dwCheckPoint);
		printf (_T("\ndwWaitHint: %d"),                ServiceStatus.dwWaitHint);

	}
	if (hSc != NULL) CloseServiceHandle (hSc);
	return 0;
}
 
 







