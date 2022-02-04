/* Chapter 12. atou Explicit Link version.    ASCII to Unicode file copy. */
/* atouEL [options] file1 file2 DllName
	-i (prompt the user to overwrite existing) is the only option supported.
	The DLL is loaded to provide the asc2un function. */

/* This program illustrates:
	1. Explicit DLL linking. */

#include "EvryThng.h"

int _tmain (int argc, LPTSTR argv [])
{
	BOOL (*Asc2Un) (LPCTSTR, LPCTSTR, BOOL);
	DWORD LocFileIn, LocFileOut, LocDLL;
	HINSTANCE hDLL;
	FARPROC pA2U;
	BOOL DashI;
	HANDLE hOutFile;
	TCHAR YNResp [3] = YES;

	if (argc != 4 && argc != 5)
		ReportError (_T ("Usage: atouEL [options] file1 file2 DllName"), 1, FALSE);
	LocFileIn = Options (argc, argv, _T ("i"), &DashI, NULL);

	LocFileOut = LocFileIn + 1;
	LocDLL = LocFileOut + 1;
	
	/* Test for existence of output file if DashI
		and prompt the user if it does exist. */
	
	if (DashI) {
		hOutFile = CreateFile (argv [3], 0, 0, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hOutFile != INVALID_HANDLE_VALUE && ConsolePrompt 
			(_T ("Overwrite existing file? [y/n]"), YNResp, 3, TRUE)
			&& _tcscmp (CharLower (YNResp), YES) != 0) {
				CloseHandle (hOutFile);
				ReportError (_T ("Will not overwrite."), 2, FALSE);
		}
				/* Get rid of the handle before we forget. */

		CloseHandle (hOutFile);
		if (_tcscmp (CharLower (YNResp), YES) != 0) {
			ReportError (_T ("Will not overwrite."), 3, FALSE);
			return 3;
		}
	}

	/* Load the ASCII to Unicode function. */
	
	hDLL = LoadLibrary (argv [LocDLL]);
	if (hDLL == NULL)
		ReportError (_T ("Failed loading DLL."), 4, TRUE);

	/*  Get the entry point address. */

	pA2U = GetProcAddress (hDLL, "Asc2Un");
	if (pA2U == NULL)
		ReportError (_T ("Failed of find entry point."), 5, TRUE);
	Asc2Un = (BOOL (*)(LPCTSTR, LPCTSTR, BOOL)) pA2U;

	/*  Call the function. */
 
	if (!Asc2Un (argv [LocFileIn], argv [LocFileOut], FALSE)) {
		FreeLibrary (hDLL);
		ReportError (_T ("ASCII to Unicode failed."), 6, TRUE);
	}
	FreeLibrary (hDLL);
	return 0;
}
