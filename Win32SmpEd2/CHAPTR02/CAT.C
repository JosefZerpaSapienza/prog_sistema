/* Chapter 2. cat. */
/* cat [options] [files]
	Only the -s option is used. Others are ignored.
	-s suppresses error report when a file does not exist */

#include "EvryThng.h"

#define BUF_SIZE 0x200

static VOID CatFile (HANDLE, HANDLE);
int _tmain (int argc, LPTSTR argv [])
{
	HANDLE hInFile, hStdIn = GetStdHandle (STD_INPUT_HANDLE);
	HANDLE hStdOut = GetStdHandle (STD_OUTPUT_HANDLE);
	BOOL DashS;
	int iArg, iFirstFile;

	iFirstFile = Options (argc, argv, _T ("s"), &DashS, NULL);

	if (iFirstFile == argc) { /* No files in arg list. */
		CatFile (hStdIn, hStdOut);
		return 0;
	} 
	for (iArg = iFirstFile; iArg < argc; iArg++) {
		_tprintf (_T("FileName %s\n"), argv[iArg]);
		hInFile = CreateFile (argv [iArg], GENERIC_READ,
				0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hInFile == INVALID_HANDLE_VALUE && !DashS)
			ReportError (_T ("Cat Error: File does not exist."), 1, TRUE);
		CatFile (hInFile, hStdOut);
		CloseHandle (hInFile);
	}
	return 0;
}

static VOID CatFile (HANDLE hInFile, HANDLE hOutFile)
{
	DWORD nIn, nOut;
	BYTE Buffer [BUF_SIZE];

	while (ReadFile (hInFile, Buffer, BUF_SIZE, &nIn, NULL) && (nIn != 0)
			&& WriteFile (hOutFile, Buffer, nIn, &nOut, NULL));

	return;
}
