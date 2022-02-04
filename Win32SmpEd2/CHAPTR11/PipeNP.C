/*	pipeNP - A reimplementation of "pipe" (Prog 9-1) using
	named pipes in byte mode. Note that the parent process acts
	as both the client and server in creating the named pipe
	handles.
	This example also illustrates the limtations of named
	pipes under Windows 95.   */

#include "EvryThng.h"

int _tmain (int argc, LPTSTR argv [])

/* Pipe together two programs whose names are on the command line:
		pipe command1 = command2
	where the two commands are arbitrary strings.
	command1 uses standard input, and command2 uses standard output.
	Use = so as not to conflict with the DOS pipe.  */

/*	THIS DOES NOT WORK ON WINDOWS 95. The reason is the poorly documented
		fact that CreateNamedPipe is not supported under Windows 95.
		Nonetheless, if you run this program under Windows 95, the
		call will return a valid handle. However, the corresponding
		CreateFile call will fail saying that the "network path was not found."
	WINDOWS 95 is limited to being a named pipe client; it can not be a server.
	Notice, however, that Windows 95 can be a Mailslot server
*/
{
	DWORD i, HostNameLen = MAX_COMPUTERNAME_LENGTH + 1 ;
	HANDLE hReadPipe, hWritePipe;
	TCHAR Command1 [MAX_PATH];
	SECURITY_ATTRIBUTES PipeSA = {sizeof (SECURITY_ATTRIBUTES), NULL, TRUE};
			/* Init for inheritable handles. */
		
	PROCESS_INFORMATION ProcInfo1, ProcInfo2;
	STARTUPINFO StartInfoCh1, StartInfoCh2;
	TCHAR PipeNameServer[] = _T("\\\\.\\pipe\\halfduplex");
	TCHAR PipeNameClient[MAX_PATH];
	TCHAR PipeNameSuffix[] = _T("\\pipe\\halfduplex");
	TCHAR HostName [MAX_COMPUTERNAME_LENGTH + 1 ];

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

	/*	This is not necessary (you could use ".") but illustrates
		the GetComputerName function. */

	if (!GetComputerName(HostName, &HostNameLen))
		ReportError (_T("Error getting computer name"), 3, TRUE);
   
	_stprintf (PipeNameClient, _T ("%s%s%s"),
			_T("\\\\"), HostName/*_T(".")*/, PipeNameSuffix);

	/*  Create an named pipe with default size and one instance.
		Use PIPE_ACCESS_OUTBOUND as data flows from the server (actually,
		a child process of the server) to the client. */

	hWritePipe = CreateNamedPipe (PipeNameServer, PIPE_ACCESS_OUTBOUND, 
 		PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, 0, 0,
		NMPWAIT_USE_DEFAULT_WAIT, &PipeSA);
	
//	_tprintf (_T("CreateNamedPipe handle value %x\n"), hWritePipe);

	if (hWritePipe == INVALID_HANDLE_VALUE)
		ReportError (_T("Error creating NP"), 3, TRUE);

	hReadPipe = CreateFile (PipeNameClient, GENERIC_READ, 0, &PipeSA, OPEN_EXISTING, 0, NULL);
	if (hReadPipe == INVALID_HANDLE_VALUE)
		ReportError (_T("Error opening NP"), 3, TRUE);
	
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
	CloseHandle (hWritePipe);

	/* Repeat (symmetrically) for the second process. */

	StartInfoCh2.hStdInput  = hReadPipe;
	StartInfoCh2.hStdError  = GetStdHandle (STD_ERROR_HANDLE);
	StartInfoCh2.hStdOutput = GetStdHandle (STD_OUTPUT_HANDLE);
	StartInfoCh2.dwFlags = STARTF_USESTDHANDLES;

	if (!CreateProcess (NULL, targv, NULL, NULL,
			TRUE,			/* Inherit handles. */
			0, NULL, NULL, &StartInfoCh2, &ProcInfo2))
		ReportError (_T ("CreateChild helper failed."), 5, TRUE);
	CloseHandle (hReadPipe);

	/* Wait for both processes to complete.
		The first one should finish first, although it really does not matter. */
	
	WaitForSingleObject (ProcInfo1.hProcess, INFINITE);
	WaitForSingleObject (ProcInfo2.hProcess, INFINITE);
	CloseHandle (ProcInfo1.hThread); CloseHandle (ProcInfo1.hProcess); 
	CloseHandle (ProcInfo2.hThread); CloseHandle (ProcInfo2.hProcess);
	return 0;
}
