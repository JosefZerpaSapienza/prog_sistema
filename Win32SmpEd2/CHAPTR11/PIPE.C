#include "EvryThng.h"

int _tmain (int argc, LPTSTR argv [])

/* Pipe together two programs whose names are on the command line:
		pipe command1 = command2
	where the two commands are arbitrary strings.
	command1 uses standard input, and command2 uses standard output.
	Use = so as not to conflict with the DOS pipe. */
{
	DWORD i;
	HANDLE hReadPipe, hWritePipe;
	TCHAR Command1 [MAX_PATH];
	SECURITY_ATTRIBUTES PipeSA = {sizeof (SECURITY_ATTRIBUTES), NULL, TRUE};
			/* Init for inheritable handles. */
		
	PROCESS_INFORMATION ProcInfo1, ProcInfo2;
	STARTUPINFO StartInfoCh1, StartInfoCh2;
	LPTSTR targv = GetCommandLine ();

	/* Startup info for the two child processes. */

	GetStartupInfo (&StartInfoCh1);
	GetStartupInfo (&StartInfoCh2);

	if (targv == NULL)
		ReportError (_T ("\nCannot read command line."), 1, TRUE);
	targv = SkipArg (targv);
	i = 0;		/* Get the two commands. */
	while (*targv != _T ('=') && *targv != _T ('\0')) {
		Command1 [i] = *targv;
		targv++; i++;
	}
	Command1 [i] = '\0';
	if (*targv == '\0')
		ReportError (_T ("No command separator found."), 2, FALSE);
	targv = SkipArg (targv);

	/* Create an anonymous pipe with default size.
		The handles are inheritable. */

	if (!CreatePipe (&hReadPipe, &hWritePipe, &PipeSA, 0))
		ReportError (_T ("Anon pipe create failed."), 3, TRUE);

	/* Set the output handle to the inheritable pipe handle,
		and create the first processes. */

	StartInfoCh1.hStdInput  = GetStdHandle (STD_INPUT_HANDLE);
	StartInfoCh1.hStdError  = GetStdHandle (STD_ERROR_HANDLE);
	StartInfoCh1.hStdOutput = hWritePipe;
	StartInfoCh1.dwFlags = STARTF_USESTDHANDLES;

	if (!CreateProcess (NULL, (LPTSTR)Command1, NULL, NULL,
			TRUE,			/* Inherit handles. */
			0, NULL, NULL, &StartInfoCh1, &ProcInfo1)) {
		ReportError (_T ("CreateProc1 failed."), 4, TRUE);
	}
	CloseHandle (ProcInfo1.hThread);
	CloseHandle (hWritePipe);

	/* Repeat (symmetrically) for the second process. */

	StartInfoCh2.hStdInput  = hReadPipe;
	StartInfoCh2.hStdError  = GetStdHandle (STD_ERROR_HANDLE);
	StartInfoCh2.hStdOutput = GetStdHandle (STD_OUTPUT_HANDLE);
	StartInfoCh2.dwFlags = STARTF_USESTDHANDLES;

	if (!CreateProcess (NULL, (LPTSTR)targv, NULL, NULL,
			TRUE,			/* Inherit handles. */
			0, NULL, NULL, &StartInfoCh2, &ProcInfo2))
		ReportError (_T ("CreateProc2 failed."), 5, TRUE);
	CloseHandle (ProcInfo2.hThread); 
	CloseHandle (hReadPipe);

	/* Wait for both processes to complete.
		The first one should finish first, although it really does not matter. */

	WaitForSingleObject (ProcInfo1.hProcess, INFINITE);
	WaitForSingleObject (ProcInfo2.hProcess, INFINITE);
	CloseHandle (ProcInfo1.hProcess); 
	CloseHandle (ProcInfo2.hProcess);
	return 0;
}
