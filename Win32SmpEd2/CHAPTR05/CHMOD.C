/* Chapter 5. chmod command. */

/* chmod [options] mode file [GroupName]
	Update access rights of the named file.
	Options:
		-f  Force - do not complain if unable to change.
		-c  Create the file if it does not exist.
			NOT AN OPTION FOR THE STANDARD UNIX COMMAND. 
			The group name is after the file name! 
			If the group name is not given, it is taken from the
			process's token.  */

/* This program illustrates:
	1.  Setting the file security attributes.
	2.  Changing a security descriptor. */

#include "EvryThng.h"

int _tmain (int argc, LPTSTR argv [])
{
	HANDLE hFile, hSecHeap;
	BOOL Force, CreateNew, Change, Exists;
	DWORD Mode, DecMode, UsrCnt = ACCT_NAME_SIZE;
	TCHAR UsrNam [ACCT_NAME_SIZE];
	int FileIndex, GrpIndex, ModeIndex;
		/* Array of rights settings in "UNIX order". */
	DWORD AceMasks [] =
		{GENERIC_READ, GENERIC_WRITE, GENERIC_EXECUTE};
	LPSECURITY_ATTRIBUTES pSa;
	LPCTSTR GroupName = NULL;

	if (argc < 3) ReportError (_T("Usage: chmod [options] mode file [GroupName]"), 1, FALSE);
	ModeIndex = Options (argc, argv, _T ("fc"), &Force, &CreateNew, NULL);
	GrpIndex = ModeIndex + 2;
	FileIndex = ModeIndex + 1;
	DecMode = _ttoi (argv [ModeIndex]);

	/* Decimal - Assume every digit is between 0 and 7 and convert. */

	Mode = ((DecMode / 100) % 10) * 64 
			 + ((DecMode / 10) % 10) * 8 + (DecMode % 10);
	Exists = (_taccess (argv [FileIndex], 0) == 0);

	if (!Exists && CreateNew) {
				/* File does not exist; create a new one. */
		if (argc < GrpIndex) 
			ReportError (_T ("Usage: Chmod -c Perm file [GroupName]."), 1, FALSE);
		if (argc == GrpIndex) { /* Find the primary group name */
			GroupName = NULL;
		} else
			GroupName = argv[GrpIndex];

		if (!GetUserName (UsrNam, &UsrCnt))
			ReportError (_T ("Failed to get user name."), 2, TRUE);

		pSa = InitializeUnixSA (Mode, UsrNam, argv [GrpIndex], AceMasks, &hSecHeap);

		if (pSa == NULL)
			ReportError (_T ("Failure setting security attributes."), 3, TRUE);
		hFile = CreateFile (argv [FileIndex], 0,
				0, pSa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			ReportError (_T ("Failure creating secure file."), 4, TRUE);
		CloseHandle (hFile);
		HeapDestroy (hSecHeap); /* Release the security structures */
	}
	else if (Exists) {
				/* File exists; change permissions. */
		Change = ChangeFilePermissions (Mode, argv [FileIndex], AceMasks);
		if (!Change && !Force)
			ReportError (_T ("Change Permissions error."), 5, TRUE);
	}
	else /* Error - File does not exist and no -c option. */
		ReportError (_T ("File to change does not exist."), 6, 0);
	
	return 0;
}
