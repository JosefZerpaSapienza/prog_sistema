/* Test UNIX Permissions functions. */

#include "EvryThng.h"

#define NAME_SIZE 1024

LPSECURITY_ATTRIBUTES InitializeUnixSA (DWORD, LPCTSTR);
DWORD ReadFilePermissions (LPCTSTR, LPTSTR, LPTSTR);
BOOL ChangeFilePermissions (DWORD, LPCTSTR);

VOID _tmain (int argc, LPTSTR argv [])
{
	HANDLE fHandle, hStdOut = GetStdHandle (STD_OUTPUT_HANDLE);
	TCHAR FName [MAX_PATH], GName [MAX_PATH];
	TCHAR UserName [NAME_SIZE], GroupName [NAME_SIZE];
	LPSECURITY_ATTRIBUTES pSA;
	int fPerm = 0;

	while (fPerm >= 0) {
		ConsolePrompt (_T ("File name to create/change: \n"),
				FName, sizeof (FName), TRUE);
		ConsolePrompt (_T ("Group Name: "), 
				GName, sizeof (GName), TRUE);
		PrintMsg (hStdOut, _T ("Permissions: "));
		_tscanf (_T ("%o"), &fPerm); /* Quit when negative. */
		if (fPerm < 0) break;

		/* Test to see if the file already exists. */

		fHandle = CreateFile (FName, 0, 0, NULL, OPEN_EXISTING, 
				FILE_ATTRIBUTE_NORMAL, NULL);
		if (fHandle != INVALID_HANDLE_VALUE) {
			CloseHandle (fHandle);
			if (ChangeFilePermissions (fPerm, FName))
				PrintMsg (hStdOut, _T ("Changed Permissions."));
			else ReportError (_T ("Change Permissions error."), 1, TRUE);
		}
			
		else { /* Create a new file with these permissions. */
			pSA = InitializeUnixSA (fPerm, GName);
			if (pSA == NULL)
				ReportError (_T ("InitUnixSA failed."), 2, TRUE);
			fHandle = CreateFile (FName, GENERIC_READ | GENERIC_WRITE, 
				0, pSA, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			if (fHandle == INVALID_HANDLE_VALUE)
				ReportError (_T ("Failure to open file."), 0, TRUE);
			else
				PrintMsg (hStdOut, _T ("New file created."));
	}

	/* Read back the file permissions. */

	fPerm = ReadFilePermissions (FName, UserName, GroupName);
	if (fPerm == 0xFFFFFFFF)
		ReportError (_T ("Failure Reading file permissions."), 0, TRUE);
	_tprintf (_T ("File Permissions = %o\n"), fPerm);
	_tprintf (_T ("User Name: %s\nGroup Name: %s"), UserName, GroupName);
	}
	ExitProcess (0);
}
