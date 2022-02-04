/* Chapter 2. atou Version 1. ASCII to Unicode file copy. */
/* Main program, which can be linked to different implementations */
/* of the Asc2Un function, which is modelled on the CopyFile function */

/* atou [options] file1 file2
 *		-i (prompt the user to overwrite existing) is the only option supported.
 *	Otherwise, this program is like cp but there is no direct UNIX equivalent. */

/* This program illustrates:
 *		1. How to change ASCII characters to UNICODE.
 *		2. Boilerplate code to process the command line.
 *	All 256 character values are allowed; there is no attempt to type file1. */

#include "EvryThng.h"
#include <io.h>

BOOL Asc2Un (LPCTSTR, LPCTSTR, BOOL);

int _tmain (int argc, LPTSTR argv [])
{
	DWORD LocFileIn, LocFileOut;
	BOOL DashI = FALSE;
	TCHAR YNResp [3] = _T ("y");

	if (argc != 3 && argc != 4)
		ReportError  (_T ("Usage: atou [options] file1 file2"), 1, FALSE);

	LocFileIn = Options (argc, argv, _T ("i"), &DashI, NULL);
	LocFileOut = LocFileIn + 1;

	if (DashI && lstrcmp (argv [1], _T ("-i")) != 0) 
		ReportError (_T ("Only supported option is -i"), 2, FALSE);
	
	/* Test for existence of output file if DashI and prompt the user if it does exist. 
		Use _taccess, which is not ANSI standard but is provided by Microsoft. */
	
	if (DashI) {
		if (_taccess (argv [LocFileOut], 0) == 0) {
			_tprintf (_T ("Overwrite existing file? [y/n]"));
			_tscanf (_T ("%s"), &YNResp);

			if (lstrcmp (CharLower (YNResp), YES) != 0) 
				ReportError (_T ("Will not overwrite."), 3, FALSE);
		}
	}

	if (!Asc2Un (argv [LocFileIn], argv [LocFileOut], FALSE))
		ReportError (_T ("ASCII to Unicode failed."), 4, TRUE);
	return 0;
}
