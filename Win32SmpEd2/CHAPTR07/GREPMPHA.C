/* Chapter 7. grepMPha. */
/* Multiple process version of grep command using handles. */
/* grepMPha files.
	Cat one or more files to temporary files. No Options. */

/* This program illustrates:
	1.	Creating processes.
	2.	Setting a child process standard I/O using the process start-up structure.
	3.	Specifying to the child process that the parent's file handles are inheritable.
	4.	Synchronizing with process termination using
		WaitForMultipleObjects and WaitForSingleObject.
	5.	Generating and using temporary files to hold the output of each process.
	6.	Passing handles as numbers on the command line. */

#include "envirmnt.h"
#include <windows.h>
#include <tchar.h>
#include "support.h"
#include <stdio.h>

int _tmain (int argc, LPTSTR argv [])

/* Create a separate process to search each file on the
	command line. Each process is given a temporary file,
	in the current directory, to receive the results. */
{
	LPHANDLE pProcHandles, pFileHandles;
	SECURITY_ATTRIBUTES StdOutSA =
			{sizeof (SECURITY_ATTRIBUTES), NULL, TRUE};

	/* SA for inheritable handle. */

	LPTSTR pTempNames;
	TCHAR CommandLine [MAX_PATH + 100], HandleNum [15];
	STARTUPINFO StartUpSearch, StartUp;
	PROCESS_INFORMATION ProcessInfo;
	DWORD FileCnt, iProc;
	HANDLE hStdOut = GetStdHandle (STD_OUTPUT_HANDLE);

	if (argc < 2)
		ReportError (_T ("Usage: grepMPha files"), 1, FALSE);
	FileCnt = argc - 1;

	/* Start up info for each child search process as well as
		the child process that will display the results. */

	GetStartupInfo (&StartUpSearch);
	GetStartupInfo (&StartUp);

	/* Allocate storage for:
		An array or temporary file names.
		An array of process handles.
		An array of handles for temp files.
		One name & handle for each file name. 
			Given the simplicity, malloc is sufficient. */

	pProcHandles = malloc (FileCnt * sizeof (HANDLE));
	pTempNames = malloc (FileCnt * MAX_PATH * TSIZE);
	pFileHandles = malloc (FileCnt * sizeof (HANDLE));
	
	/* Create a separate "grep" process for each file on
		the command line. Each process also gets a temporary
		file name for the results; the handle is communicated
		through the STARTUPINFO structure.
			argv [1] is the search pattern. */

	for (iProc = 0; iProc < FileCnt; iProc++) {

		/* Create a command line of the form:
			catHA argv [iProc + 1] FileHandleInText */

		lstrcpy (CommandLine, _T ("catHA "));
		lstrcat (CommandLine, SPACE);   /* Search file. */
		lstrcat (CommandLine, argv [iProc + 1]);

		/* Create the temp file name for std output. */

		if (GetTempFileName (_T ("."), _T ("gtm"), 0,
				pTempNames + iProc * MAX_PATH * TSIZE) == 0)
			ReportError (_T ("Temp file failure"), 2, TRUE);

		/* Set the std output for the search process. */

		pFileHandles [iProc] = /* This handle is inheritable. */
				CreateFile ((LPCTSTR) (pTempNames + iProc *
				MAX_PATH * TSIZE), GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE, &StdOutSA,
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (pFileHandles [iProc] == INVALID_HANDLE_VALUE)
			ReportError (_T ("Failure opening temp file."), 3, TRUE);

		_itot ((LONG) pFileHandles [iProc], HandleNum, 10);
		lstrcat (CommandLine, SPACE);
		lstrcat (CommandLine, HandleNum);

		/* Specify that the new process takes its std output
			from the temporary file's handles. */

//		StartUpSearch.dwFlags = STARTF_USESTDHANDLES;
//		StartUpSearch.hStdOutput = pFileHandles [iProc];

		/* Create a process to execute the command line. */

		if (!CreateProcess (NULL, CommandLine, NULL, NULL,
				TRUE, 0, NULL, NULL, &StartUpSearch, &ProcessInfo))
			ReportError (_T ("ProcCreate failed."), 4, TRUE);

		/* Save the process handle. */

		pProcHandles [iProc] = ProcessInfo.hProcess;
	}

	/* Processes are all running. Wait for them to complete,
		then output the results - in the order of the command line file names. */

	WaitForMultipleObjects (FileCnt, pProcHandles, TRUE, INFINITE);

	/* Use the cat utility to list the file.
		Delete each temporary file upon completion. */

	for (iProc = 0; iProc < FileCnt; iProc++) {
		CloseHandle (pFileHandles [iProc]);
		if (FileCnt > 1)	/* Display file name if more than one. */
			PrintStrings (hStdOut, argv [iProc + 2], CRLF, NULL);
		lstrcpy (CommandLine, _T ("cat "));
		lstrcat (CommandLine, pTempNames + iProc * MAX_PATH * TSIZE);

		/* It is necessary to restore the process startup handle. */

		StartUp.dwFlags = STARTF_USESTDHANDLES;
		StartUp.hStdOutput = hStdOut;

		if (!CreateProcess (NULL, CommandLine, NULL, NULL,
				TRUE, 0, NULL, NULL, &StartUp, &ProcessInfo))
			ReportError (_T ("Failure executing cat."), 5, TRUE);
		WaitForSingleObject (ProcessInfo.hProcess, INFINITE);

//		if (!DeleteFile (pTempNames + iProc * MAX_PATH * TSIZE))
//			ReportError (_T ("Cannot delete temp file."), 6, TRUE);
	}
	free (pProcHandles);
	free (pFileHandles);
	free (pTempNames);
	return 0;
}

