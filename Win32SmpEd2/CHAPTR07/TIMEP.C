/* Chapter 7. timepp. Simplified version with no special 
	auxiliary functions or header files.  */

/* timeprint: Execute a command line and display
	the time (elapsed, kernel, user) required. */

/* This program illustrates:
	1. Creating processes.
	2. Obtaining the command line.
	3. Obtaining the elapsed times.
	4. Converting file times to system times.
	5. Displaying system times.
		Windows 2000/NT only. */

#include "EvryThng.h"

int _tmain (int argc, LPTSTR argv [])
{
	STARTUPINFO StartUp;
	PROCESS_INFORMATION ProcInfo;
	union {		/* Structure required for file time arithmetic. */
		LONGLONG li;
		FILETIME ft;
	} CreateTime, ExitTime, ElapsedTime;

	FILETIME KernelTime, UserTime;
	SYSTEMTIME ElTiSys, KeTiSys, UsTiSys, StartTimeSys, ExitTimeSys;
	LPTSTR targv = SkipArg (GetCommandLine ());
		/* On Windows CE, you need to access the argv[] elements directly */
	OSVERSIONINFO OSVer;
	BOOL Is2000NT;
	HANDLE hProc;

	/*  Skip past the first blank-space delimited token on the command line
		A more general solution would account for tabs and new lines */
	if (argc <= 1) 
		ReportError (_T("Usage: timep command ..."), 1, FALSE);

	/* Determine is this is Windows 2000 or NT.  */
	OSVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (!GetVersionEx (&OSVer)) 
		ReportError (_T("Can not get OS Version info. %d"), 2, TRUE);

	Is2000NT = (OSVer.dwPlatformId == VER_PLATFORM_WIN32_NT);
	/* No explicit W2000 or CE values defined. This works on W2000, however */

	GetStartupInfo (&StartUp);
	GetSystemTime (&StartTimeSys);

	/* Execute the command line and wait for the process to complete. */

	if (!CreateProcess (NULL, targv, NULL, NULL, TRUE,
		NORMAL_PRIORITY_CLASS, NULL, NULL, &StartUp, &ProcInfo)) 
		ReportError (_T ("\nError starting process. %d"), 3, TRUE);
	
	/*	Assure that we have all REQUIRED access to the process */
	if (!DuplicateHandle (GetCurrentProcess(), ProcInfo.hProcess,
			GetCurrentProcess(), &hProc,  PROCESS_QUERY_INFORMATION | SYNCHRONIZE,
			FALSE, 0)) 
				ReportError (_T("Failure duplicating handle. %d"), 4, TRUE);

	if (WaitForSingleObject (hProc, INFINITE) != WAIT_OBJECT_0) 
		ReportError (_T("Failed waiting for process termination. %d"), 5, TRUE);;

	GetSystemTime (&ExitTimeSys);

	if (Is2000NT) {	/* Windows 2000, NT. Elapsed, Kernel, & User times. */
		if (!GetProcessTimes (hProc, &CreateTime.ft,
			&ExitTime.ft, &KernelTime, &UserTime)) 
				ReportError (_T("Can not get process times. %d"), 6, TRUE);

		ElapsedTime.li = ExitTime.li - CreateTime.li;

		FileTimeToSystemTime (&ElapsedTime.ft, &ElTiSys);
		FileTimeToSystemTime (&KernelTime, &KeTiSys);
		FileTimeToSystemTime (&UserTime, &UsTiSys);
		_tprintf (_T ("Real Time: %02d:%02d:%02d:%03d\n"),
			ElTiSys.wHour, ElTiSys.wMinute, ElTiSys.wSecond,
			ElTiSys.wMilliseconds);
		_tprintf (_T ("User Time: %02d:%02d:%02d:%03d\n"),
			UsTiSys.wHour, UsTiSys.wMinute, UsTiSys.wSecond,
			UsTiSys.wMilliseconds);
		_tprintf (_T ("Sys Time:  %02d:%02d:%02d:%03d\n"),
			KeTiSys.wHour, KeTiSys.wMinute, KeTiSys.wSecond,
			KeTiSys.wMilliseconds);
	} else {

		/* Windows 9x and CE. Elapsed time only. */
		SystemTimeToFileTime (&StartTimeSys, &CreateTime.ft);
		SystemTimeToFileTime (&ExitTimeSys, &ExitTime.ft);
		ElapsedTime.li = ExitTime.li - CreateTime.li;
		FileTimeToSystemTime (&ElapsedTime.ft, &ElTiSys);
		_tprintf (_T ("Real Time: %02d:%02d:%02d:%03d\n"),
			ElTiSys.wHour, ElTiSys.wMinute, ElTiSys.wSecond,
			ElTiSys.wMilliseconds);
	}
	CloseHandle (ProcInfo.hThread); CloseHandle (ProcInfo.hProcess);
	CloseHandle (hProc);	
	return 0;
}
