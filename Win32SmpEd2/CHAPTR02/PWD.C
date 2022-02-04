/* Chapter 2. pwd. */
/* pwd: Similar to the UNIX command. */
/* This program illustrates:
	1. Win32 GetCurrentDirectory
	2. Testing the length of a returned string */

#include "EvryThng.h"

#define DIRNAME_LEN MAX_PATH + 2

int _tmain (int argc, LPTSTR argv [])
{
	/* Buffer to receive current directory allows for the CR,
		LF at the end of the longest possible path. */

	TCHAR pwdBuffer [DIRNAME_LEN];
	DWORD LenCurDir;
	LenCurDir = GetCurrentDirectory (DIRNAME_LEN, pwdBuffer);
	if (LenCurDir == 0)
		ReportError (_T ("Failure getting pathname."), 1, TRUE);
	if (LenCurDir > DIRNAME_LEN)
		ReportError (_T ("Pathname is too long."), 2, FALSE);
	
	PrintMsg (GetStdHandle (STD_OUTPUT_HANDLE), pwdBuffer);
	return 0;
}
